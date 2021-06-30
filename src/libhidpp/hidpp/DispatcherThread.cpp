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

#include "DispatcherThread.h"

#include <hidpp10/Error.h>
#include <hidpp20/Error.h>
#include <misc/Log.h>
#include <memory>
#include <algorithm>

using namespace HIDPP;

template<typename Iterator,
	 void (DispatcherThread::*cancel) (Iterator),
	 std::mutex DispatcherThread::*mutex>
class DispatcherThread::AsyncReport: public Dispatcher::AsyncReport
{
	DispatcherThread *dispatcher;
	std::future<Report> report;
	Iterator it;
public:
	AsyncReport (DispatcherThread *dispatcher, std::future<Report> &&report, Iterator it):
		dispatcher (dispatcher), report (std::move (report)), it (it)
	{
	}

	virtual Report get ()
	{
		return report.get ();
	}

	virtual Report get (int timeout)
	{
		report.wait_for (std::chrono::milliseconds (timeout));
		{
			std::unique_lock<std::mutex> lock (dispatcher->*mutex);
			// make sure there was no race before the lock.
			auto status = report.wait_for (std::chrono::milliseconds (0));
			if (status != std::future_status::ready) {
				// cancel the command
				(dispatcher->*cancel) (it);
				throw Dispatcher::TimeoutError ();
			}
		}
		return report.get ();
	}
};

DispatcherThread::DispatcherThread (const char *path):
	_dev (path),
	_stopped (false)
{
	checkReportDescriptor (_dev.getReportDescriptor ());
}

DispatcherThread::~DispatcherThread ()
{
}

const HID::RawDevice &DispatcherThread::hidraw () const
{
	return _dev;
}

uint16_t DispatcherThread::vendorID () const
{
	return _dev.vendorID ();
}

uint16_t DispatcherThread::productID () const
{
	return _dev.productID ();
}

std::string DispatcherThread::name () const
{
	return _dev.name ();
}

void DispatcherThread::sendCommandWithoutResponse (const Report &report)
{
	_dev.writeReport (report.rawReport ());
}

std::unique_ptr<Dispatcher::AsyncReport> DispatcherThread::sendCommand (Report &&report)
{
	std::unique_lock<std::mutex> lock (_command_mutex);
	if (_stopped)
		throw _exception;
	_dev.writeReport (report.rawReport ());
	auto it = _commands.insert (_commands.end (), Command { std::move (report) });
	return std::make_unique<AsyncCommandResponse> (this, it->response.get_future (), it);
}

std::unique_ptr<Dispatcher::AsyncReport> DispatcherThread::getNotification (DeviceIndex index, uint8_t sub_id)
{
	std::unique_lock<std::mutex> lock (_listener_mutex);
	if (_stopped)
		throw _exception;
	auto it = _notifications.emplace (_notifications.end ());
	it->listener = Dispatcher::registerEventHandler (index, sub_id, [this, it] (const Report &report) {
		it->notification.set_value (report);
		_notifications.erase (it);
		return false;
	});
	return std::make_unique<AsyncNotification> (this, it->notification.get_future (), it);
}

DispatcherThread::listener_iterator DispatcherThread::registerEventHandler (DeviceIndex index, uint8_t sub_id, const event_handler &handler)
{
	std::unique_lock<std::mutex> lock (_listener_mutex);
	return Dispatcher::registerEventHandler (index, sub_id, handler);
}

void DispatcherThread::unregisterEventHandler (listener_iterator it)
{
	std::unique_lock<std::mutex> lock (_listener_mutex);
	Dispatcher::unregisterEventHandler (it);
}

void DispatcherThread::cancelCommand (command_iterator it)
{
	_commands.erase (it);
}

void DispatcherThread::cancelNotification (notification_iterator it)
{
	Dispatcher::unregisterEventHandler (it->listener);
	_notifications.erase (it);
}

void DispatcherThread::run ()
{
	while (!_stopped) {
		try {
			std::vector<uint8_t> raw_report (MaxReportLength);
			if (0 != _dev.readReport (raw_report))
				processReport (std::move (raw_report));
		}
		catch (Report::InvalidReportID &e) {
			// There may be other reports on this device, just ignore them.
		}
		catch (Report::InvalidReportLength &e) {
			Log::error () << "Ignored report with invalid length" << std::endl;
		}
		catch (std::exception &e) {
			Log::error () << "Failed to read HID report: " << e.what () << std::endl;
			_exception = std::current_exception ();
			goto stop;
		}
	}
	_exception = std::make_exception_ptr (NotRunning ());
stop:
	_stopped = true;
	{
		std::unique_lock<std::mutex> lock (_command_mutex);
		if (!_commands.empty ()) {
			Log::warning () << "Unfinished commands while stopping dispatcher." << std::endl;
			for (auto &cmd: _commands) {
				cmd.response.set_exception (_exception);
			}
		}
	}
	{
		std::unique_lock<std::mutex> lock (_listener_mutex);
		if (!_notifications.empty ()) {
			Log::warning () << "Unreceived notifications while stopping dispatcher." << std::endl;
			for (auto &n: _notifications) {
				n.notification.set_exception (_exception);
			}
		}
	}
}

void DispatcherThread::stop ()
{
	_stopped = true;
	_dev.interruptRead ();
}

void DispatcherThread::processReport (std::vector<uint8_t> &&raw_report)
{

	Report report (std::move (raw_report));
	DeviceIndex index = report.deviceIndex ();

	uint8_t sub_id, address, feature, error_code;
	unsigned int function, sw_id;

	if (report.checkErrorMessage10 (&sub_id, &address, &error_code)) {
		std::unique_lock<std::mutex> lock (_command_mutex);
		auto it = std::find_if (_commands.begin (), _commands.end (),
			[index, sub_id, address] (const Command &cmd) {
				return index == cmd.request.deviceIndex () &&
					sub_id == cmd.request.subID () &&
					address == cmd.request.address ();
			});
		if (it != _commands.end ()) {
			it->response.set_exception (std::make_exception_ptr (HIDPP10::Error (error_code)));
			_commands.erase (it);
		}
		else
			Log::warning () << "HID++1.0 error message was not matched with any command." << std::endl;
	}
	else if (report.checkErrorMessage20 (&feature, &function, &sw_id, &error_code)) {
		std::unique_lock<std::mutex> lock (_command_mutex);
		auto it = std::find_if (_commands.begin (), _commands.end (),
			[index, feature, function, sw_id] (const Command &cmd) {
				return index == cmd.request.deviceIndex () &&
					feature == cmd.request.featureIndex () &&
					function == cmd.request.function () &&
					sw_id == cmd.request.softwareID ();
			});
		if (it != _commands.end ()) {
			it->response.set_exception (std::make_exception_ptr (HIDPP20::Error (error_code)));
			_commands.erase (it);
		}
		else
			Log::warning () << "HID++2.0 error message was not matched with any command." << std::endl;
	}
	else {
		std::unique_lock<std::mutex> cmd_lock (_command_mutex);
		auto it = std::find_if (_commands.begin (), _commands.end (),
			[&report] (const Command &cmd) {
				return report.deviceIndex () == cmd.request.deviceIndex () &&
					report.subID () == cmd.request.subID () &&
					report.address () == cmd.request.address ();
			});
		if (it != _commands.end ()) {
			it->response.set_value (std::move (report));
			_commands.erase (it);
		}
		else if (report.softwareID () == 0 || report.subID () < 0x80) { // is an event
			// TODO: fix this test, HID++2.0 answers could
			// be mistaken for HID++1.0 notifications:
			// feature index/subID is usually below 0x80.
			// But the lowest known HID++1.0 notification is 0x40,
			// if no HID++2.0 device has more than 64 features,
			// there should be no confusion in practice.
			cmd_lock.unlock ();
			std::unique_lock<std::mutex> lock (_listener_mutex);
			processEvent (report);
		}
		else {
			Log::warning () << "Answer was not matched with any command." << std::endl;
		}
	}
}

const char *DispatcherThread::NotRunning::what () const noexcept
{
	return "Dispatcher thread is not running";
}
