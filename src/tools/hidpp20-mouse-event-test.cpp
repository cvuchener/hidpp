/*
 * Copyright 2017 Clément Vuchener
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

#include <hidpp/SimpleDispatcher.h>
#include <hidpp/DispatcherThread.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/IMouseButtonSpy.h>
#include <hidpp20/IOnboardProfiles.h>
#include <hidpp20/UnsupportedFeature.h>
#include <cstdio>
#include <memory>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"
#include "common/EventQueue.h"

extern "C" {
#include <unistd.h>
#include <signal.h>
#include <string.h>
}

using namespace HIDPP20;

class EventHandler
{
public:
	virtual const HIDPP20::FeatureInterface *feature () const = 0;
	virtual void handleEvent (const HIDPP::Report &event) = 0;
};

class ButtonHandler: public EventHandler
{
	HIDPP20::IMouseButtonSpy _imbs;
	unsigned int _button_count;
	uint16_t _button_state;
public:
	ButtonHandler (HIDPP20::Device *dev):
		_imbs (dev),
		_button_count (_imbs.getMouseButtonCount ()),
		_button_state (0)
	{
		printf ("The mouse has %d buttons.\n", _button_count);
		_imbs.startMouseButtonSpy ();
	}

	~ButtonHandler ()
	{
		_imbs.stopMouseButtonSpy ();
	}

	const HIDPP20::FeatureInterface *feature () const
	{
		return &_imbs;
	}

	void handleEvent (const HIDPP::Report &event)
	{
		uint16_t new_state;
		switch (event.function ()) {
		case IMouseButtonSpy::MouseButtonEvent:
			new_state = IMouseButtonSpy::mouseButtonEvent (event);
			for (unsigned int i = 0; i < _button_count; ++i) {
				if ((_button_state ^ new_state) & (1<<i))
					printf ("Button %d: %s\n", i, (new_state & (1<<i) ? "pressed" : "released"));
			}
			_button_state = new_state;
			break;
		}
	}
};

class ProfileHandler: public EventHandler
{
	HIDPP20::IOnboardProfiles _iop;
	HIDPP20::IOnboardProfiles::Mode _old_mode;
public:
	ProfileHandler (HIDPP20::Device *dev):
		_iop (dev),
		_old_mode (_iop.getMode ())
	{
		_iop.setMode (IOnboardProfiles::Mode::Onboard);
	}

	~ProfileHandler ()
	{
		_iop.setMode (_old_mode);
	}

	const HIDPP20::FeatureInterface *feature () const
	{
		return &_iop;
	}

	void handleEvent (const HIDPP::Report &event)
	{
		switch (event.function ()) {
		case IOnboardProfiles::CurrentProfileChanged:
			printf ("Current profile changed: %u\n",
				std::get<1> (IOnboardProfiles::currentProfileChanged (event)));
			break;
		case IOnboardProfiles::CurrentDPIIndexChanged:
			printf ("Current dpi index changed: %u\n",
			IOnboardProfiles::currentDPIIndexChanged (event));
			break;
		}
	}
};

class EventListener
{
	HIDPP::Dispatcher *dispatcher;
	HIDPP::DeviceIndex index;
	std::map<uint8_t, std::unique_ptr<EventHandler>> handlers;
	std::map<uint8_t, HIDPP::Dispatcher::listener_iterator> iterators;
public:
	EventListener (HIDPP::Dispatcher *dispatcher, HIDPP::DeviceIndex index):
		dispatcher (dispatcher),
		index (index)
	{
	}

	virtual ~EventListener ()
	{
		removeEventHandlers ();
	}

	virtual void addEventHandler (std::unique_ptr<EventHandler> &&handler)
	{
		uint8_t feature = handler->feature ()->index ();
		EventHandler *ptr = handler.get ();
		handlers.emplace (feature, std::move (handler));
		auto it = dispatcher->registerEventHandler (index, feature, [ptr] (const HIDPP::Report &report) {
			ptr->handleEvent (report);
			return true;
		});
		iterators.emplace (feature, it);
	}

	virtual void removeEventHandlers ()
	{
		for (const auto &p: iterators)
			dispatcher->unregisterEventHandler (p.second);
		handlers.clear ();
		iterators.clear ();
	}

	virtual void start () = 0;
	virtual void stop () = 0;

protected:
	virtual bool event (EventHandler *handler, const HIDPP::Report &report) = 0;
};

class ThreadListener: public EventListener
{
	HIDPP::DispatcherThread *dispatcher;
	EventQueue<std::pair<EventHandler *, HIDPP::Report>> queue;
	std::thread thread;
public:
	ThreadListener (HIDPP::DispatcherThread *dispatcher, HIDPP::DeviceIndex index):
		EventListener (dispatcher, index),
		dispatcher (dispatcher),
		thread (std::bind (&HIDPP::DispatcherThread::run, dispatcher))
	{
	}

	virtual void start ()
	{
		while (auto event = queue.pop ()) {
			auto pair = event.value ();
			pair.first->handleEvent (pair.second);
		}
		dispatcher->stop ();
		thread.join ();
	}

	virtual void stop ()
	{
		queue.interrupt ();
	}

protected:
	virtual bool event (EventHandler *handler, const HIDPP::Report &report)
	{
		queue.push (std::make_pair (handler, report));
		return true;
	}
};

class SimpleListener: public EventListener
{
	HIDPP::SimpleDispatcher *dispatcher;
public:
	SimpleListener (HIDPP::SimpleDispatcher *dispatcher, HIDPP::DeviceIndex index):
		EventListener (dispatcher, index),
		dispatcher (dispatcher)
	{
	}

	virtual void start ()
	{
		dispatcher->listen ();
	}

	virtual void stop ()
	{
		dispatcher->stop ();
	}

protected:
	virtual bool event (EventHandler *handler, const HIDPP::Report &report)
	{
		handler->handleEvent (report);
		return true;
	}
};

EventListener *listener;

void sigint (int)
{
	listener->stop ();
}

int main (int argc, char *argv[])
{
	static const char *args = "/dev/hidrawX";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;
	bool thread = false;

	std::vector<Option> options = {
		Option ('t', "thread",
			Option::NoArgument, "",
			"Use threaded dispatcher",
			[&thread] (const char *optarg) {
				thread = true;
				return true;
			}),
		DeviceIndexOption (device_index),
		VerboseOption (),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	if (argc-first_arg < 1) {
		fprintf (stderr, "Too few arguments.\n");
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];
	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	std::unique_ptr<HIDPP20::Device> dev;
	try {
		if (thread) {
			HIDPP::DispatcherThread *d = new HIDPP::DispatcherThread (path);
			listener = new ThreadListener (d, device_index);
			dev = std::make_unique<HIDPP20::Device> (d, device_index);
			dispatcher.reset (d);
		}
		else {
			HIDPP::SimpleDispatcher *d = new HIDPP::SimpleDispatcher (path);
			listener = new SimpleListener (d, device_index);
			dev = std::make_unique<HIDPP20::Device> (d, device_index);
			dispatcher.reset (d);
		}
	}
	catch (std::exception &e) {
		fprintf (stderr, "Initialization failed: %s\n", e.what ());
		delete listener;
		return EXIT_FAILURE;
	}

	struct sigaction sa, oldsa;
	memset (&sa, 0, sizeof (struct sigaction));
	sa.sa_handler = sigint;
	sigaction (SIGINT, &sa, &oldsa);

	try {
		listener->addEventHandler (std::make_unique<ButtonHandler> (dev.get ()));
	}
	catch (HIDPP20::UnsupportedFeature e) {
		printf ("%s\n", e.what ());
	}
	try {
		listener->addEventHandler (std::make_unique<ProfileHandler> (dev.get ()));
	}
	catch (HIDPP20::UnsupportedFeature e) {
		printf ("%s\n", e.what ());
	}
	listener->start();
	listener->removeEventHandlers ();
	sigaction (SIGINT, &oldsa, nullptr);
	delete listener;

	return EXIT_SUCCESS;
}

