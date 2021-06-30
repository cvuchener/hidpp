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

#include "SimpleDispatcher.h"

#include <hidpp10/Error.h>
#include <hidpp20/Error.h>
#include <misc/Log.h>
#include <memory>
#include <algorithm>

using namespace HIDPP;

SimpleDispatcher::SimpleDispatcher (const char *path):
	_dev (path)
{
	checkReportDescriptor (_dev.getReportDescriptor ());
}

SimpleDispatcher::~SimpleDispatcher ()
{
}

const HID::RawDevice &SimpleDispatcher::hidraw () const
{
	return _dev;
}

uint16_t SimpleDispatcher::vendorID () const
{
	return _dev.vendorID ();
}

uint16_t SimpleDispatcher::productID () const
{
	return _dev.productID ();
}

std::string SimpleDispatcher::name () const
{
	return _dev.name ();
}

void SimpleDispatcher::sendCommandWithoutResponse (const Report &report)
{
	_dev.writeReport (report.rawReport ());
}

std::unique_ptr<Dispatcher::AsyncReport> SimpleDispatcher::sendCommand (Report &&report)
{
	_dev.writeReport (report.rawReport ());
	return std::make_unique<CommandResponse> (this, std::move (report));
}

std::unique_ptr<Dispatcher::AsyncReport> SimpleDispatcher::getNotification (DeviceIndex index, uint8_t sub_id)
{
	return std::make_unique<Notification> (this, index, sub_id);
}

void SimpleDispatcher::listen ()
{
	auto debug = Log::debug ("dispatcher");
	try {
		while (true) {
			Report report = getReport ();
			debug << "Ignored report while listening for events." << std::endl;
		}
	}
	catch (Dispatcher::TimeoutError &e) {
		// return when getReport is interrupted.
	}
}

void SimpleDispatcher::stop ()
{
	_dev.interruptRead ();
}

Report SimpleDispatcher::getReport (int timeout)
{
	while (true) {
		std::vector<uint8_t> raw_report (MaxReportLength);
		if (0 == _dev.readReport (raw_report, timeout))
			throw Dispatcher::TimeoutError ();
		try {
			HIDPP::Report report (std::move (raw_report));
			if (report.checkErrorMessage10 (nullptr, nullptr, nullptr)) {
				return report;
			}
			if (report.checkErrorMessage20 (nullptr, nullptr, nullptr, nullptr)) {
				return report;
			}
			processEvent (report);
			return report;
		}
		catch (Report::InvalidReportID &e) {
			// There may be other reports on this device, just ignore them.
		}
		catch (Report::InvalidReportLength &e) {
			Log::error () << "Ignored report with invalid length" << std::endl;
		}
	}
}

SimpleDispatcher::CommandResponse::CommandResponse (SimpleDispatcher *dispatcher, Report &&report):
	dispatcher (dispatcher), report (std::move (report))
{
}

Report SimpleDispatcher::CommandResponse::get ()
{
	return get (-1);
}

Report SimpleDispatcher::CommandResponse::get (int timeout)
{
	auto debug = Log::debug ("dispatcher");
	while (true) {
		Report response = dispatcher->getReport (timeout);
		if (response.deviceIndex () != report.deviceIndex ()) {
			debug << "Ignored response because of different device index." << std::endl;
			continue;
		}
		unsigned int function, swid;
		uint8_t sub_id, address, feature, error_code;
		if (response.checkErrorMessage10 (&sub_id, &address, &error_code)) {
			if (sub_id == report.subID () && address == report.address ())
				throw HIDPP10::Error (error_code);
			else {
				debug << "Ignored HID++1.0 error response." << std::endl;
				continue;
			}
		}
		if (response.checkErrorMessage20 (&feature, &function, &swid, &error_code)) {
			if (feature == report.featureIndex () && function == report.function () && swid == report.softwareID ())
				throw HIDPP20::Error (error_code);
			else {
				debug << "Ignored HID++2.0 error response." << std::endl;
				continue;
			}
		}
		if (report.subID () == response.subID () && report.address () == response.address ())
			return response;
	}
}

SimpleDispatcher::Notification::Notification (SimpleDispatcher *dispatcher, DeviceIndex index, uint8_t sub_id):
	dispatcher (dispatcher), index (index), sub_id (sub_id)
{
}

Report SimpleDispatcher::Notification::get ()
{
	return get (-1);
}

Report SimpleDispatcher::Notification::get (int timeout)
{
	auto debug = Log::debug ("dispatcher");
	while (true) {
		Report report = dispatcher->getReport (timeout);
		if (report.deviceIndex () == index && report.subID () == sub_id) {
			return report;
		}
		debug << "Ignored report while waiting for notification." << std::endl;
	}
}

