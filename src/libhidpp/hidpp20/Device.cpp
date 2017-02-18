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

#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <misc/Log.h>

using namespace HIDPP20;

unsigned int Device::softwareID = 1;

Device::Device (const std::string &path, HIDPP::DeviceIndex device_index):
	HIDPP::Device (path, device_index)
{
	// TODO: check version
}

Device::Device (HIDPP::Device &&device):
	HIDPP::Device (std::move (device))
{
	// TODO: check version
}

std::vector<uint8_t> Device::callFunction (uint8_t feature_index,
					   unsigned int function,
					   std::vector<uint8_t>::const_iterator param_begin,
					   std::vector<uint8_t>::const_iterator param_end)
{
	Log::debug ().printf ("Calling feature 0x%02hhx/function %u\n",
			      feature_index, function);
	Log::debug ().printBytes ("Parameters:", param_begin, param_end);

	std::size_t len = std::distance (param_begin, param_end);
	HIDPP::Report::Type type;
	if (len <= HIDPP::ShortParamLength)
		type = HIDPP::Report::Short;
	else if (len <= HIDPP::LongParamLength)
		type = HIDPP::Report::Long;
	else {
		throw std::logic_error ("Parameters too long");
	}
	HIDPP::Report request (type, deviceIndex (), feature_index, function, softwareID);
	std::copy (param_begin, param_end, request.parameterBegin ());
	sendReport (request);
	while (true) {
		HIDPP::Report response = getReport ();

		if (response.deviceIndex () != deviceIndex ()) {
				Log::debug () << __FUNCTION__ << ": "
					      << "Ignored report with wrong device index"
					      << std::endl;
			continue;
		}

		uint8_t r_feature_index, error_code;
		unsigned int r_function, r_sw_id;
		if (response.checkErrorMessage20 (&r_feature_index,
						  &r_function,
						  &r_sw_id,
						  &error_code)) {
			if (r_feature_index != feature_index ||
			    r_function != function ||
			    r_sw_id != softwareID) {
				Log::debug () << __FUNCTION__ << ": "
					<< "Ignored error message with wrong feature/function/softwareID"
					<< std::endl;
				continue;
			}

			Log::debug ().printf ("Received error message with code 0x%02hhx\n", error_code);
			throw Error (static_cast<Error::ErrorCode> (error_code));
		}
		if (response.featureIndex () == feature_index &&
		    response.function () == function &&
		    response.softwareID () == softwareID) {
			Log::debug ().printBytes ("Results:", response.parameterBegin (), response.parameterEnd ());
			return std::vector<uint8_t> (response.parameterBegin (), response.parameterEnd ());
		}
		Log::debug () << __FUNCTION__ << ": "
			<< "Ignored report with wrong feature/function/softwareID"
			<< std::endl;
	}
}
