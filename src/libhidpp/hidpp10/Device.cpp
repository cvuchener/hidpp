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

#include <hidpp10/Device.h>
#include <hidpp10/Error.h>
#include <hidpp10/WriteError.h>
#include <misc/Log.h>

#include <cassert>

using namespace HIDPP10;

Device::Device (const std::string &path, HIDPP::DeviceIndex device_index):
	HIDPP::Device (path, device_index)
{
	// TODO: check version
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
	sendReport (request);

	while (true) {
		HIDPP::Report response = getReport ();

		uint8_t r_sub_id, r_address, error_code;
		if (response.checkErrorMessage10 (&r_sub_id, &r_address, &error_code)) {
			if (r_sub_id != sub_id || r_address != address) {
				Log::debug () << __FUNCTION__ << ": "
					      << "Ignored error message with wrong subID or address"
					      << std::endl;
				continue;
			}

			Log::debug ().printf ("Received error message with code 0x%02hhx\n", error_code);
			throw Error (static_cast<Error::ErrorCode> (error_code));
		}

		if (response.subID () != sub_id ||
		    response.address () != address) {
			Log::debug () << __FUNCTION__ << ": "
				      << "Ignored report with wrong subID or address"
				      << std::endl;
			continue;
		}

		if (response.type () != result_type)
			throw std::runtime_error ("Invalid result length");

		if (results)
			results->assign (response.parameterBegin (), response.parameterEnd ());
		return;
	}
}

void Device::setRegister (uint8_t address,
			  const std::vector<uint8_t> &params,
			  std::vector<uint8_t> *results)
{
	if (params.size () <= HIDPP::ShortParamLength) {
		Log::debug ().printf ("Setting short register 0x%02hhx\n", address);
		Log::debug ().printBytes ("Parameters:",
					  params.begin (), params.end ());

		accessRegister<SetRegisterShort,
			       HIDPP::Report::Short, HIDPP::Report::Short>
			      (address, &params, results);

		if (results)
			Log::debug ().printBytes ("Results:",
						  results->begin (), results->end ());
	}
	else if (params.size () <= HIDPP::LongParamLength) {
		Log::debug ().printf ("Setting long register 0x%02hhx\n", address);
		Log::debug ().printBytes ("Parameters:",
					  params.begin (), params.end ());

		accessRegister<SetRegisterLong,
			       HIDPP::Report::Long, HIDPP::Report::Short>
			      (address, &params, results);

		if (results)
			Log::debug ().printBytes ("Results:",
						  results->begin (), results->end ());
	}
	else
		throw std::logic_error ("Register too long");

}

void Device::getRegister (uint8_t address,
			  const std::vector<uint8_t> *params,
			  std::vector<uint8_t> &results)
{
	if (results.size () <= HIDPP::ShortParamLength) {
		Log::debug ().printf ("Getting short register 0x%02hhx\n", address);
		if (params)
			Log::debug ().printBytes ("Parameters:",
						  params->begin (), params->end ());

		accessRegister<GetRegisterShort,
			       HIDPP::Report::Short, HIDPP::Report::Short>
			      (address, params, &results);

		Log::debug ().printBytes ("Results:",
				 results.begin (), results.end ());
	}
	else if (results.size () <= HIDPP::LongParamLength) {
		Log::debug ().printf ("Getting long register 0x%02hhx\n", address);
		if (params)
			Log::debug ().printBytes ("Parameters:",
						  params->begin (), params->end ());

		accessRegister<GetRegisterLong,
			       HIDPP::Report::Short, HIDPP::Report::Long>
			      (address, params, &results);

		Log::debug ().printBytes ("Results:",
					  results.begin (), results.end ());
	}
	else
		throw std::logic_error ("Register too long");
}

void Device::sendDataPacket (uint8_t sub_id, uint8_t seq_num,
			     std::vector<uint8_t>::const_iterator param_begin,
			     std::vector<uint8_t>::const_iterator param_end,
			     bool wait_for_ack)
{
	Log::debug ().printf ("Sending data packet %hhu\n", seq_num);
	Log::debug ().printBytes ("Data packet", param_begin, param_end);

	assert (std::distance (param_begin, param_end) <= (int) HIDPP::LongParamLength);
	HIDPP::Report packet (HIDPP::Report::Long, deviceIndex (), sub_id, seq_num);
	std::copy (param_begin, param_end, packet.parameterBegin ());
	sendReport (packet);

	if (!wait_for_ack)
		return;

	while (true) {
		HIDPP::Report response = getReport ();

		if (response.deviceIndex () != deviceIndex () ||
		    response.subID () != SendDataAcknowledgement) {
			Log::debug () << __FUNCTION__ << ": "
				      << "Ignored notification with wrong deviceIndex or subID"
				      << std::endl;
			continue;
		}

		auto response_params = response.parameterBegin ();
		if (response.address () == 1 && response_params[0] == seq_num) {
			/* Expected notification */
			Log::debug ().printf ("Data packet %hhu acknowledged\n", seq_num);
			return;
		}
		else {
			/* The notification is an error message */
			Log::debug ().printf ("Data packet %hhu: error 0x%02hhx\n",
					      seq_num, response.address ());
			throw WriteError (response.address ());
		}
	}
}

