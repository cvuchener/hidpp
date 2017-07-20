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
