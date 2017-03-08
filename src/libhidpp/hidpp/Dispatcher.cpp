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

#include "Dispatcher.h"

#include <hidpp10/Error.h>
#include <hidpp20/Error.h>
#include <hidpp20/IRoot.h>
#include <misc/Log.h>
#include <memory>
#include <algorithm>

static constexpr std::chrono::time_point<std::chrono::system_clock> notimeout;

using namespace HIDPP;

Dispatcher::NoHIDPPReportException::NoHIDPPReportException ()
{
}

const char *Dispatcher::NoHIDPPReportException::what () const noexcept
{
	return "No HID++ report";
}

const char *Dispatcher::TimeoutError::what () const noexcept
{
	return "readReport timed out";
}

static const std::array<uint8_t, 27> ShortReportDesc = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x01,		// Usage (0001 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x10,		//   Report ID (16)
	0x75, 0x08,		//   Report Size (8)
	0x95, 0x06,		//   Report Count (6)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

static const std::array<uint8_t, 27> LongReportDesc = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x02,		// Usage (0002 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x11,		//   Report ID (17)
	0x75, 0x08,		//   Report Size (8)
	0x95, 0x13,		//   Report Count (19)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

/* Alternative versions from the G602 */
static const std::array<uint8_t, 27> ShortReportDesc2 = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x01,		// Usage (0001 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x10,		//   Report ID (16)
	0x95, 0x06,		//   Report Count (6)
	0x75, 0x08,		//   Report Size (8)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

static const std::array<uint8_t, 27> LongReportDesc2 = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x02,		// Usage (0002 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x11,		//   Report ID (17)
	0x95, 0x13,		//   Report Count (19)
	0x75, 0x08,		//   Report Size (8)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

Dispatcher::Dispatcher (const char *path):
	_dev (path),
	_stop (false)
{
	const HIDRaw::ReportDescriptor &rdesc = _dev.getReportDescriptor ();
	if (rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 ShortReportDesc.begin (),
					 ShortReportDesc.end ()) &&
	    rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 ShortReportDesc2.begin (),
					 ShortReportDesc2.end ()))
		throw NoHIDPPReportException ();
	if (rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 LongReportDesc.begin (),
					 LongReportDesc.end ()) &&
	    rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 LongReportDesc2.begin (),
					 LongReportDesc2.end ()))
		throw NoHIDPPReportException ();
	_thread = std::thread (&Dispatcher::run, this);
}

Dispatcher::~Dispatcher ()
{
	_stop = true;
	_dev.interruptRead ();
	_thread.join ();
}

const HIDRaw &Dispatcher::hidraw () const
{
	return _dev;
}

std::tuple<unsigned int, unsigned int> Dispatcher::getVersion (DeviceIndex index)
{
	static constexpr unsigned int software_id = 1;
	Report request (Report::Short, index, HIDPP20::IRoot::index, HIDPP20::IRoot::Ping, software_id);
	command_iterator it;
	std::future<Report> answer;
	{
		std::unique_lock<std::mutex> lock (_mutex);
		_dev.writeReport (request.rawReport ());
		it =_commands.insert (_commands.end (), Command { std::move (request) });
		answer = it->promised_report.get_future ();
	}
	try {
		answer.wait_for (std::chrono::duration<int> (1));
		{
			std::unique_lock<std::mutex> lock (_mutex);
			// make sure there was no race before the lock.
			auto status = answer.wait_for (std::chrono::duration<int> (0));
			if (status != std::future_status::ready) {
				// cancel the command
				_commands.erase (it);
				throw TimeoutError ();
			}
		}
		auto report = answer.get ();
		auto params = report.parameterBegin ();
		return std::make_tuple (params[0], params[1]);
	}
	catch (HIDPP10::Error e) {
		if (e.errorCode () == HIDPP10::Error::InvalidSubID) // Expected error from a HID++ 1.0 device
			return std::make_tuple (1, 0);
		else
			throw e;
	}
}

void Dispatcher::sendCommandWithoutResponse (const Report &report)
{
	_dev.writeReport (report.rawReport ());
}

std::future<Report> Dispatcher::sendCommand (Report &&report)
{
	std::unique_lock<std::mutex> lock (_mutex);
	_dev.writeReport (report.rawReport ());
	auto it = _commands.insert (_commands.end (), Command { std::move (report) });
	return it->promised_report.get_future ();
}

std::future<Report> Dispatcher::getNotification (DeviceIndex index, uint8_t sub_id)
{
	std::unique_lock<std::mutex> lock (_mutex);
	auto promise = std::make_shared<std::promise<Report>> ();
	std::future<Report> future = promise->get_future ();
	_listeners.emplace (std::make_tuple (index, sub_id),
		Listener ([promise] (const Report &report) { promise->set_value (report); }, true));
	return future;
}

Dispatcher::listener_iterator Dispatcher::registerEventQueue (DeviceIndex index, uint8_t sub_id, EventQueue<Report> *queue)
{
	std::unique_lock<std::mutex> lock (_mutex);
	return _listeners.emplace (std::make_tuple (index, sub_id),
		Listener (std::bind (&EventQueue<Report>::push, queue, std::placeholders::_1), false));
}

void Dispatcher::unregisterEventQueue (listener_iterator it)
{
	std::unique_lock<std::mutex> lock (_mutex);
	_listeners.erase (it);
}

void Dispatcher::run ()
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

void Dispatcher::processReport (std::vector<uint8_t> &&raw_report)
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

Dispatcher::Listener::Listener (const std::function<void (const Report &)> fn, bool only_once):
	fn (fn),
	only_once (only_once)
{
}
