/*
 * Copyright 2015 Clément Vuchener
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdio>
#include <vector>
#include <map>
#include <cassert>

#include "common/common.h"
#include "common/CommonOptions.h"
#include "common/EventQueue.h"

extern "C" {
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <linux/uinput.h>
}

#include <misc/Log.h>
#include <hid/DeviceMonitor.h>
#include <hidpp/DispatcherThread.h>
#include <hidpp10/Device.h>
#include <hidpp10/defs.h>
#include <hidpp10/IReceiver.h>
#include <hidpp20/Device.h>
#include <hidpp20/ITouchpadRawXY.h>

EventQueue<std::function<void ()>> task_queue;

class Driver
{
	HIDPP::Dispatcher *_dispatcher;
	EventQueue<HIDPP::Report> _queue;
	std::vector<HIDPP::Dispatcher::listener_iterator> _iterators;
public:
	Driver (HIDPP::Dispatcher *dispatcher):
		_dispatcher (dispatcher)
	{
	}

	virtual ~Driver ()
	{
		for (auto it: _iterators)
			_dispatcher->unregisterEventHandler (it);
	}

protected:
	HIDPP::Dispatcher *dispatcher ()
	{
		return _dispatcher;
	}

	void addEvent (HIDPP::DeviceIndex index, uint8_t sub_id, bool direct = false)
	{
		auto it = _dispatcher->registerEventHandler (index, sub_id, (direct ?
			(HIDPP::Dispatcher::event_handler) [this] (const HIDPP::Report &report) {
				event (report);
				return true;
			}
			:
			(HIDPP::Dispatcher::event_handler) [this] (const HIDPP::Report &report) {
				task_queue.push (std::bind (&Driver::event, this, report));
				return true;
			}
		));
		_iterators.push_back (it);
	}

	virtual void event (const HIDPP::Report &report) = 0;
};

static void setBit (int fd, int request, int code)
{
	if (-1 == ioctl (fd, request, code))
		throw std::system_error (errno, std::system_category (), "ioctl");
}

static void sendEvent (int fd, int type, int code, int value)
{
	struct input_event ev;
	memset (&ev, 0, sizeof (struct input_event));
	ev.type = type;
	ev.code = code;
	ev.value = value;
	if (-1 == write (fd, &ev, sizeof (struct input_event)))
		throw std::system_error (errno, std::system_category (), "write");
}

class TouchpadDriver: public Driver
{
	HIDPP20::Device _dev;
	HIDPP20::ITouchpadRawXY _itrxy;
	HIDPP20::ITouchpadRawXY::TouchpadInfo _info;
	int _uinput;
	struct MTState {
		int id[2];
		uint16_t next_id;
		int count;

		MTState ():
			id {-1, -1},
			next_id (1),
			count (0)
		{
		}

		void event (int uinput, const HIDPP20::ITouchpadRawXY::TouchpadRawData::Point points[], unsigned int point_count)
		{
			assert (point_count == 2);
			bool touching[2] = { false, false };
			int new_count = 0;
			for (unsigned int i = 0; i < 2; ++i) {
				const auto &point = points[i];
				if (point.id != 0) {
					touching[point.id-1] = true;
					new_count++;
					if (id[point.id-1] == -1)
						id[point.id-1] = next_id++;
					sendEvent (uinput, EV_ABS, ABS_MT_SLOT, point.id-1);
					sendEvent (uinput, EV_ABS, ABS_MT_TRACKING_ID, id[point.id-1]);
					sendEvent (uinput, EV_ABS, ABS_MT_POSITION_X, point.x);
					sendEvent (uinput, EV_ABS, ABS_MT_POSITION_Y, point.y);
				}
			}
			for (unsigned int i = 0; i < 2; ++i) {
				if (!touching[i] && id[i] != -1) {
					sendEvent (uinput, EV_ABS, ABS_MT_SLOT, i);
					sendEvent (uinput, EV_ABS, ABS_MT_TRACKING_ID, -1);
					id[i] = -1;
				}
			}
			if (new_count != count) {
				if (new_count == 1)
					sendEvent (uinput, EV_KEY, BTN_TOOL_FINGER, 1);
				else if (new_count == 2)
					sendEvent (uinput, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
				if (count == 1)
					sendEvent (uinput, EV_KEY, BTN_TOOL_FINGER, 0);
				else if (count == 2)
					sendEvent (uinput, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
				count = new_count;
			}
		}
	} mt_state;
	struct STState {
		bool touching;
		STState ():
			touching (false)
		{
		}
		void event (int uinput, const HIDPP20::ITouchpadRawXY::TouchpadRawData::Point &point)
		{
			if (point.id != 0) {
				if (!touching)
					sendEvent (uinput, EV_KEY, BTN_TOUCH, 1);
				sendEvent (uinput, EV_ABS, ABS_X, point.x);
				sendEvent (uinput, EV_ABS, ABS_Y, point.y);
			}
			else if (touching) {
				sendEvent (uinput, EV_KEY, BTN_TOUCH, 0);
			}
			touching = point.id != 0;
		}
	} st_state;
public:
	TouchpadDriver (HIDPP::Dispatcher *dispatcher, HIDPP::DeviceIndex index):
		Driver (dispatcher),
		_dev (dispatcher, index),
		_itrxy (&_dev),
		_info (_itrxy.getTouchpadInfo ())
	{
		if (-1 == (_uinput = open ("/dev/uinput", O_RDWR)))
			throw std::system_error (errno, std::system_category (), "open uinput");
		struct uinput_user_dev uidev;
		memset (&uidev, 0, sizeof (struct uinput_user_dev));
		strncpy (uidev.name, _dev.name ().c_str (), UINPUT_MAX_NAME_SIZE);
		uidev.id.bustype = BUS_VIRTUAL;
		uidev.id.vendor = dispatcher->vendorID ();
		uidev.id.product = _dev.productID ();
		uidev.id.version = 0;
		try {
			setBit (_uinput, UI_SET_EVBIT, EV_ABS);
			setBit (_uinput, UI_SET_ABSBIT, ABS_MT_SLOT);
			uidev.absmin[ABS_MT_SLOT] = 0;
			uidev.absmax[ABS_MT_SLOT] = 1;
			setBit (_uinput, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
			uidev.absmin[ABS_MT_TRACKING_ID] = 0;
			uidev.absmax[ABS_MT_TRACKING_ID] = std::numeric_limits<uint16_t>::max ();
			setBit (_uinput, UI_SET_ABSBIT, ABS_X);
			uidev.absmin[ABS_X] = 0;
			uidev.absmax[ABS_X] = _info.x_max;
			setBit (_uinput, UI_SET_ABSBIT, ABS_Y);
			uidev.absmin[ABS_Y] = 0;
			uidev.absmax[ABS_Y] = _info.y_max;
			setBit (_uinput, UI_SET_ABSBIT, ABS_MT_POSITION_X);
			uidev.absmin[ABS_MT_POSITION_X] = 0;
			uidev.absmax[ABS_MT_POSITION_X] = _info.x_max;;
			setBit (_uinput, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
			uidev.absmin[ABS_MT_POSITION_Y] = 0;
			uidev.absmax[ABS_MT_POSITION_Y] = _info.y_max;
			setBit (_uinput, UI_SET_EVBIT, EV_KEY);
			setBit (_uinput, UI_SET_KEYBIT, BTN_TOUCH);
			setBit (_uinput, UI_SET_KEYBIT, BTN_TOOL_FINGER);
			setBit (_uinput, UI_SET_KEYBIT, BTN_TOOL_DOUBLETAP);
			if (-1 == write (_uinput, &uidev, sizeof (struct uinput_user_dev)))
				throw std::system_error (errno, std::system_category (), "write");
			if (-1 == ioctl (_uinput, UI_DEV_CREATE))
				throw std::system_error (errno, std::system_category (), "ioctl UI_DEV_CREATE");
		}
		catch (std::exception &e) {
			Log::error () << "Failed to initialize uinput: " << e.what () << std::endl;
			close (_uinput);
			throw;
		}

		printf ("Added touchpad device: %s\n", _dev.name ().c_str ());
		addEvent (index, _itrxy.index (), true);
		_itrxy.setTouchpadRawMode (true);
	}

	~TouchpadDriver ()
	{
		try {
			_itrxy.setTouchpadRawMode (false);
		}
		catch (std::exception &e) {
			Log::debug () << "Could not disable raw mode: " << e.what () << std::endl;
		}
	}

protected:
	void event (const HIDPP::Report &report)
	{
		if (report.function () != HIDPP20::ITouchpadRawXY::TouchpadRawEvent)
			return;
		auto data = HIDPP20::ITouchpadRawXY::touchpadRawEvent (report);
		for (unsigned int i = 0; i < 2; ++i)
			if (data.points[i].id != 0)
				data.points[i].y = 0x4000+_info.y_max-data.points[i].y;
		try {
			st_state.event (_uinput, data.points[0]);
			mt_state.event (_uinput, data.points, 2);
			sendEvent (_uinput, EV_SYN, SYN_REPORT, 0);
		}
		catch (std::exception &e) {
			Log::error () << "Failed to send uinput event: " << e.what () << std::endl;
		}
	}
};

class ReceiverDriver: public Driver
{
	HIDPP10::Device _dev;
	std::map<HIDPP::DeviceIndex, TouchpadDriver> _drivers;
public:
	ReceiverDriver (HIDPP::Dispatcher *dispatcher):
		Driver (dispatcher),
		_dev (dispatcher)
	{
		Log::info () << "Added receiver device" << std::endl;
		for (int i = 1; i <= 6; ++i) {
			HIDPP::DeviceIndex index = static_cast<HIDPP::DeviceIndex> (i);
			addEvent (index, HIDPP10::DeviceDisconnection);
			addEvent (index, HIDPP10::DeviceConnection);
			addDevice (index);
		}
	}

protected:
	void event (const HIDPP::Report &report)
	{
		auto params = report.parameterBegin ();
		switch (report.subID ()) {
		case HIDPP10::DeviceConnection:
			if (params[0] & (1<<6))
				_drivers.erase (report.deviceIndex ());
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds (10));
				addDevice (report.deviceIndex ());
			}
			break;
		case HIDPP10::DeviceDisconnection:
			_drivers.erase (report.deviceIndex ());
			break;
		}
	}

private:
	void addDevice (HIDPP::DeviceIndex index)
	{
		try {
			_drivers.emplace (std::piecewise_construct_t (),
					  std::make_tuple (index),
					  std::make_tuple (dispatcher (), index));
		}
		catch (std::exception &e) {
			Log::debug () << "Ignoring wireless device " << index << ": " << e.what () << std::endl;
		}
	}
};

class MyMonitor: public HID::DeviceMonitor
{
	struct node
	{
		HIDPP::DispatcherThread dispatcher;
		std::thread thread;
		std::unique_ptr<Driver> driver;

		node (const char *path):
			dispatcher (path),
			thread (std::bind (&HIDPP::DispatcherThread::run, &dispatcher))
		{
		}

		~node ()
		{
			driver.reset ();
			dispatcher.stop ();
			thread.join ();
		}
	};
	std::map<std::string, node> _nodes;
public:
	void addDevice (const char *path)
	{
		std::map<std::string, node>::iterator it;
		try {
			it = _nodes.emplace (path, path).first;
		}
		catch (std::exception &e) {
			Log::debug () << "Ignored device " << path << ": " << e.what () << std::endl;
			return;
		}
		auto &node = it->second;
		try {
			node.driver = std::make_unique<ReceiverDriver> (&node.dispatcher);
			return;
		}
		catch (std::exception &e) {
			Log::debug () << "Device " << path << " is not a receiver: " << e.what () << std::endl;
		}
		for (HIDPP::DeviceIndex index: { HIDPP::DefaultDevice, HIDPP::CordedDevice }) {
			try {
				node.driver = std::make_unique<TouchpadDriver> (&node.dispatcher, index);
				return;
			}
			catch (std::exception &e) {
				Log::debug () << "Device " << path << "/" << index << " is not a touchpad device: " << e.what () << std::endl;
			}
		}
		_nodes.erase (it);
	}

	void removeDevice (const char *path)
	{
		_nodes.erase (path);
	}
};

void sigint (int)
{
	task_queue.interrupt ();
}

int main (int argc, char *argv[])
{
	std::vector<Option> options = {
		VerboseOption (),
	};
	Option help = HelpOption (argv[0], "", &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	MyMonitor monitor;
	std::thread monitor_thread (std::bind (&HID::DeviceMonitor::run, &monitor));

	struct sigaction sa, oldsa;
	memset (&sa, 0, sizeof (struct sigaction));
	sa.sa_handler = sigint;
	sigaction (SIGINT, &sa, &oldsa);

	while (auto task = task_queue.pop ())
		task.value () ();

	sigaction (SIGINT, &oldsa, nullptr);

	monitor.stop ();
	monitor_thread.join ();

	return EXIT_SUCCESS;
}
