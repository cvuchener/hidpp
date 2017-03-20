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
#include <hidpp10/Error.h>
#include <hidpp10/WriteError.h>
#include <misc/Log.h>

#include <cassert>

using namespace HIDPP10;

Device::Device (HIDPP::Dispatcher *dispatcher, HIDPP::DeviceIndex device_index):
	HIDPP::Device (dispatcher, device_index)
{
	auto version = protocolVersion ();
	if (version != std::make_tuple (1, 0))
		throw HIDPP::Device::InvalidProtocolVersion (version);
}

Device::Device (HIDPP::Device &&device):
	HIDPP::Device (std::move (device))
{
	auto version = protocolVersion ();
	if (version != std::make_tuple (1, 0))
		throw HIDPP::Device::InvalidProtocolVersion (version);
}

template<uint8_t sub_id, HIDPP::Report::Type request_type, HIDPP::Report::Type result_type>
void Device::accessRegister (uint8_t address,
			     const std::vector<uint8_t> *params,
			     std::vector<uint8_t> *results)
{
	HIDPP::Report request (request_type, deviceIndex (), sub_id, address);
	if (params) {
		assert (params->size () <= request.parameterLength ());
		std::copy (params->begin (), params->end (), request.parameterBegin ());
	}

	auto response = dispatcher ()->sendCommand (std::move (request))->get ();

	if (response.type () != result_type)
		throw std::runtime_error ("Invalid result length");

	if (results)
		results->assign (response.parameterBegin (), response.parameterEnd ());
}

void Device::setRegister (uint8_t address,
			  const std::vector<uint8_t> &params,
			  std::vector<uint8_t> *results)
{
	auto debug = Log::debug ("register");
	if (params.size () <= HIDPP::ShortParamLength) {
		debug.printf ("Setting short register 0x%02hhx\n", address);
		debug.printBytes ("Parameters:", params.begin (), params.end ());

		accessRegister<SetRegisterShort,
			       HIDPP::Report::Short, HIDPP::Report::Short>
			      (address, &params, results);

		if (results)
			debug.printBytes ("Results:", results->begin (), results->end ());
	}
	else if (params.size () <= HIDPP::LongParamLength) {
		debug.printf ("Setting long register 0x%02hhx\n", address);
		debug.printBytes ("Parameters:", params.begin (), params.end ());

		accessRegister<SetRegisterLong,
			       HIDPP::Report::Long, HIDPP::Report::Short>
			      (address, &params, results);

		if (results)
			debug.printBytes ("Results:", results->begin (), results->end ());
	}
	else
		throw std::logic_error ("Register too long");

}

void Device::getRegister (uint8_t address,
			  const std::vector<uint8_t> *params,
			  std::vector<uint8_t> &results)
{
	auto debug = Log::debug ("register");
	if (results.size () <= HIDPP::ShortParamLength) {
		debug.printf ("Getting short register 0x%02hhx\n", address);
		if (params)
			debug.printBytes ("Parameters:", params->begin (), params->end ());

		accessRegister<GetRegisterShort,
			       HIDPP::Report::Short, HIDPP::Report::Short>
			      (address, params, &results);

		debug.printBytes ("Results:", results.begin (), results.end ());
	}
	else if (results.size () <= HIDPP::LongParamLength) {
		debug.printf ("Getting long register 0x%02hhx\n", address);
		if (params)
			debug.printBytes ("Parameters:", params->begin (), params->end ());

		accessRegister<GetRegisterLong,
			       HIDPP::Report::Short, HIDPP::Report::Long>
			      (address, params, &results);

		debug.printBytes ("Results:", results.begin (), results.end ());
	}
	else
		throw std::logic_error ("Register too long");
}

void Device::sendDataPacket (uint8_t sub_id, uint8_t seq_num,
			     std::vector<uint8_t>::const_iterator param_begin,
			     std::vector<uint8_t>::const_iterator param_end,
			     bool wait_for_ack)
{
	auto debug = Log::debug ("data");
	debug.printf ("Sending data packet %hhu\n", seq_num);
	debug.printBytes ("Data packet", param_begin, param_end);

	assert (std::distance (param_begin, param_end) <= (int) HIDPP::LongParamLength);
	HIDPP::Report packet (HIDPP::Report::Long, deviceIndex (), sub_id, seq_num);
	std::copy (param_begin, param_end, packet.parameterBegin ());

	std::unique_ptr<HIDPP::Dispatcher::AsyncReport> notification;

	if (wait_for_ack) {
		notification = dispatcher ()->getNotification (deviceIndex (), SendDataAcknowledgement);
	}

	dispatcher ()->sendCommandWithoutResponse (packet);

	if (wait_for_ack) {
		auto response = notification->get ();
		auto response_params = response.parameterBegin ();
		if (response.address () == 1 && response_params[0] == seq_num) {
			/* Expected notification */
			debug.printf ("Data packet %hhu acknowledged\n", seq_num);
			return;
		}
		else {
			/* The notification is an error message */
			debug.printf ("Data packet %hhu: error 0x%02hhx\n",
					      seq_num, response.address ());
			throw WriteError (response.address ());
		}
	}
}

