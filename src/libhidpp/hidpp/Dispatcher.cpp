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

#include <misc/Log.h>

using namespace HIDPP;

const char *Dispatcher::NoHIDPPReportException::what () const noexcept
{
	return "No HID++ report";
}

const char *Dispatcher::TimeoutError::what () const noexcept
{
	return "readReport timed out";
}

Dispatcher::AsyncReport::~AsyncReport ()
{
}

Dispatcher::~Dispatcher ()
{
}

Dispatcher::listener_iterator Dispatcher::registerEventHandler (DeviceIndex index, uint8_t sub_id, const event_handler &handler)
{
	return _listeners.emplace (std::make_tuple (index, sub_id), handler);
}

void Dispatcher::unregisterEventHandler (listener_iterator it)
{
	_listeners.erase (it);
}

void Dispatcher::processEvent (const Report &report)
{
	auto range = _listeners.equal_range (std::make_tuple (report.deviceIndex (), report.subID ()));
	for (auto it = range.first; it != range.second;) {
		if (it->second (report))
			++it;
		else
			it = _listeners.erase (it);
	}
}

static bool hasReport(const HID::ReportCollection &collection, HID::ReportID::Type type, uint8_t id, HID::Usage usage, unsigned int count)
{
	using namespace HID;
	auto it = collection.reports.find (ReportID {type, id});
	if (it == collection.reports.end ())
		return false;
	const auto &fields = it->second;
	if (fields.size () != 1)
		return false;
	const auto &field = fields.front ();
	if (auto usages = std::get_if<std::vector<Usage>> (&field.usages)) {
		return field.flags.Data () && field.flags.Array () &&
			field.count == count && field.size == 8 &&
			usages->size () == 1 && usages->front () == usage;
	}
	else
		return false;
}

void Dispatcher::checkReportDescriptor (const HID::ReportDescriptor &rdesc)
{
	enum {
		Unknown,
		Legacy,
		Modern,
	} scheme = Unknown;
	_report_info.flags = 0;
	uint8_t expected_reports = 0;
	for (const auto &collection: rdesc.collections) {
		auto collection_usage_msb = static_cast<uint8_t> (collection.usage.usage >> 8);
		auto collection_usage_lsb = static_cast<uint8_t> (collection.usage.usage);
		if (collection.usage.usage_page == 0xFF43) { // Modern scheme usage page
			if (scheme == Unknown) {
				scheme = Modern;
				expected_reports = collection_usage_msb;
			}
			else if (scheme != Modern) {
				Log::warning () << "Ignoring HID++ collection with unexpected usage page." << std::endl;
				continue;
			}
			else if (expected_reports != collection_usage_msb) {
				Log::warning () << "Ignoring HID++ collection with mismatched usage report flags." << std::endl;
				continue;
			}
		}
		else if (collection.usage.usage_page == 0xFF00) { // Legacy scheme usage page
			if (scheme == Unknown)
				scheme = Legacy;
			else if (scheme != Legacy) {
				Log::warning () << "Ignoring HID++ collection with unexpected usage page." << std::endl;
				continue;
			}
			else if (collection_usage_msb != 0) {
				Log::warning () << "Ignoring HID++ collection with invalid usage." << std::endl;
				continue;
			}
		}
		else // Not a HID++ collection
			continue;
		for (auto [type, flag]: {
				std::make_tuple (Report::Short, ReportInfo::HasShortReport),
				std::make_tuple (Report::Long, ReportInfo::HasLongReport),
				std::make_tuple (Report::VeryLong, ReportInfo::HasVeryLongReport) }) {
			if (collection_usage_lsb == flag) {
				auto usage = HID::Usage (collection.usage.usage_page, flag);
				bool has_input = hasReport (
						collection,
						HID::ReportID::Type::Input,
						static_cast<uint8_t> (type),
						usage,
						Report::reportLength (type)-1);
				if (!has_input)
					Log::warning () << "Missing input report for report " << type << std::endl;
				bool has_output = hasReport (
						collection,
						HID::ReportID::Type::Output,
						static_cast<uint8_t> (type),
						usage,
						Report::reportLength(type)-1);
				if (!has_output)
					Log::warning () << "Missing output report for report " << type << std::endl;
				if (has_input && has_output)
					_report_info.flags |= flag;
			}
		}
	}
	if (scheme == Modern && expected_reports != _report_info.flags)
		Log::warning () << "Expected HID++ reports were not found." << std::endl;
	if (_report_info.flags == 0)
		throw Dispatcher::NoHIDPPReportException ();
}

