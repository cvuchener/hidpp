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

template<typename Container, Container DispatcherThread::*container>
class DispatcherThread::AsyncReport: public Dispatcher::AsyncReport
{
	DispatcherThread *dispatcher;
	std::future<Report> report;
	typename Container::iterator it;
public:
	AsyncReport (DispatcherThread *dispatcher, std::future<Report> &&report, typename Container::iterator it):
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
			std::unique_lock<std::mutex> lock (dispatcher->_mutex);
			// make sure there was no race before the lock.
			auto status = report.wait_for (std::chrono::milliseconds (0));
			if (status != std::future_status::ready) {
				// cancel the command
				(dispatcher->*container).erase (it);
				throw Dispatcher::TimeoutError ();
			}
		}
		return report.get ();
	}
};

DispatcherThread::DispatcherThread (const char *path):
	_dev (path),
	_stop (false)
{
	const HIDRaw::ReportDescriptor &rdesc = _dev.getReportDescriptor ();
	if (!checkReportDescriptor (rdesc))
		throw Dispatcher::NoHIDPPReportException ();
	_thread = std::thread (&DispatcherThread::run, this);
}

DispatcherThread::~DispatcherThread ()
{
	_stop = true;
	_dev.interruptRead ();
	_thread.join ();
}

const HIDRaw &DispatcherThread::hidraw () const
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
	std::unique_lock<std::mutex> lock (_mutex);
	_dev.writeReport (report.rawReport ());
	auto it = _commands.insert (_commands.end (), Command { std::move (report) });
	return std::make_unique<CommandResponse> (this, it->promised_report.get_future (), it);
}

std::unique_ptr<Dispatcher::AsyncReport> DispatcherThread::getNotification (DeviceIndex index, uint8_t sub_id)
{
	std::unique_lock<std::mutex> lock (_mutex);
	auto promise = std::make_shared<std::promise<Report>> ();
	std::future<Report> future = promise->get_future ();
	auto it = _listeners.emplace (std::make_tuple (index, sub_id),
		Listener ([promise] (const Report &report) { promise->set_value (report); }, true));
	return std::make_unique<Notification> (this, std::move (future), it);
}

DispatcherThread::listener_iterator DispatcherThread::registerEventQueue (DeviceIndex index, uint8_t sub_id, EventQueue<Report> *queue)
{
	std::unique_lock<std::mutex> lock (_mutex);
	return _listeners.emplace (std::make_tuple (index, sub_id),
		Listener (std::bind (&EventQueue<Report>::push, queue, std::placeholders::_1), false));
}

void DispatcherThread::unregisterEventQueue (listener_iterator it)
{
	std::unique_lock<std::mutex> lock (_mutex);
	_listeners.erase (it);
}

void DispatcherThread::run ()
{
	while (!_stop) {
		try {
			std::vector<uint8_t> raw_report (Report::MaxDataLength+1);
			if (0 != _dev.readReport (raw_report))
				processReport (std::move (raw_report));
		}
		catch (Report::InvalidReportID e) {
			// There may be other reports on this device, just ignore them.
		}
		catch (Report::InvalidReportLength e) {
			Log::error () << "Ignored report with invalid length" << std::endl;
		}
		catch (std::exception &e) {
			std::unique_lock<std::mutex> lock (_mutex);
			Log::error () << "Failed to read HID report: " << e.what () << std::endl;
			for (auto &cmd: _commands) {
				cmd.promised_report.set_exception (std::current_exception ());
			}
			return;
		}
	}

	{
		std::unique_lock<std::mutex> lock (_mutex);
		if (!_commands.empty ()) {
			Log::warning () << "Unfinished commands while terminating dispatcher." << std::endl;
			for (auto &cmd: _commands) {
				cmd.promised_report.set_exception (std::make_exception_ptr (std::runtime_error ("Dispatcher terminated")));
			}
		}
	}
}

void DispatcherThread::processReport (std::vector<uint8_t> &&raw_report)
{
	std::unique_lock<std::mutex> lock (_mutex);

	Report report (std::move (raw_report));
	DeviceIndex index = report.deviceIndex ();

	uint8_t sub_id, address, feature, error_code;
	unsigned int function, sw_id;

	if (report.checkErrorMessage10 (&sub_id, &address, &error_code)) {
		auto it = std::find_if (_commands.begin (), _commands.end (),
			[index, sub_id, address] (const Command &cmd) {
				return index == cmd.report.deviceIndex () &&
					sub_id == cmd.report.subID () &&
					address == cmd.report.address ();
			});
		if (it != _commands.end ()) {
			it->promised_report.set_exception (std::make_exception_ptr (HIDPP10::Error (error_code)));
			_commands.erase (it);
		}
		else
			Log::warning () << "HID++1.0 error message was not matched with any command." << std::endl;
	}
	else if (report.checkErrorMessage20 (&feature, &function, &sw_id, &error_code)) {
		auto it = std::find_if (_commands.begin (), _commands.end (),
			[index, feature, function, sw_id] (const Command &cmd) {
				return index == cmd.report.deviceIndex () &&
					feature == cmd.report.featureIndex () &&
					function == cmd.report.function () &&
					sw_id == cmd.report.softwareID ();
			});
		if (it != _commands.end ()) {
			it->promised_report.set_exception (std::make_exception_ptr (HIDPP20::Error (error_code)));
			_commands.erase (it);
		}
		else
			Log::warning () << "HID++2.0 error message was not matched with any command." << std::endl;
	}
	else {
		auto it = std::find_if (_commands.begin (), _commands.end (),
			[&report] (const Command &cmd) {
				return report.deviceIndex () == cmd.report.deviceIndex () &&
					report.subID () == cmd.report.subID () &&
					report.address () == cmd.report.address ();
			});
		if (it != _commands.end ()) {
			it->promised_report.set_value (std::move (report));
			_commands.erase (it);
		}
		else if (report.softwareID () == 0 || report.subID () < 0x80) { // is an event
			// TODO: fix this test, HID++2.0 answers could
			// be mistaken for HID++1.0 notifications:
			// feature index/subID is usually below 0x80.
			// But the lowest known HID++1.0 notification is 0x40,
			// if no HID++2.0 device has more than 64 features,
			// there should be no confusion in practice.
			auto range = _listeners.equal_range (std::make_tuple (report.deviceIndex (), report.subID ()));
			for (auto it = range.first; it != range.second;) {
				it->second.fn (report);
				if (it->second.only_once)
					it = _listeners.erase (it);
				else
					++it;
			}
		}
		else {
			Log::warning () << "Answer was not matched with any command." << std::endl;
		}
	}
}

DispatcherThread::Listener::Listener (const std::function<void (const Report &)> fn, bool only_once):
	fn (fn),
	only_once (only_once)
{
}
