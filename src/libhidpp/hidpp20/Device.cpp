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

#include "Device.h"

#include <hidpp/Dispatcher.h>
#include <misc/Log.h>

using namespace HIDPP20;

unsigned int Device::softwareID = 1;

Device::Device (HIDPP::Dispatcher *dispatcher, HIDPP::DeviceIndex device_index):
	HIDPP::Device (dispatcher, device_index)
{
	auto version = protocolVersion ();
	if (std::get<0> (version) < 2)
		throw HIDPP::Device::InvalidProtocolVersion (version);
}

Device::Device (HIDPP::Device &&device):
	HIDPP::Device (std::move (device))
{
	auto version = protocolVersion ();
	if (std::get<0> (version) < 2)
		throw HIDPP::Device::InvalidProtocolVersion (version);
}

std::vector<uint8_t> Device::callFunction (uint8_t feature_index,
					   unsigned int function,
					   std::vector<uint8_t>::const_iterator param_begin,
					   std::vector<uint8_t>::const_iterator param_end)
{
	auto debug = Log::debug ("call");
	debug.printf ("Calling feature 0x%02hhx/function %u\n", feature_index, function);
	debug.printBytes ("Parameters:", param_begin, param_end);

	std::size_t len = std::distance (param_begin, param_end);
	auto type = dispatcher ()->reportInfo ().findReport (len);
	if (!type)
		throw std::logic_error ("Parameters too long");
	HIDPP::Report request (*type, deviceIndex (), feature_index, function, softwareID);
	std::copy (param_begin, param_end, request.parameterBegin ());

	auto response = dispatcher ()->sendCommand (std::move (request))->get ();

	debug.printBytes ("Results:", response.parameterBegin (), response.parameterEnd ());
	return std::vector<uint8_t> (response.parameterBegin (), response.parameterEnd ());
}
