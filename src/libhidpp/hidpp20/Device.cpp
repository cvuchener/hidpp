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

using namespace HIDPP20;

unsigned int Device::softwareID = 1;

Device::Device (const std::string &path, HIDPP::DeviceIndex device_index):
	HIDPP::Device (path, device_index)
{
	// TODO: check version
}

HIDPP::Parameters Device::callFunction (uint8_t feature_index,
					unsigned int function,
					const HIDPP::Parameters &params)
{
	HIDPP::Parameters in (params);
	if (in.size () <= HIDPP::ShortParamLength)
		in.resize (HIDPP::ShortParamLength, 0);
	else if (in.size () <= HIDPP::LongParamLength)
		in.resize (HIDPP::LongParamLength, 0);
	else {
		// TODO: error
	}
	HIDPP::Report request (deviceIndex (), feature_index, function, softwareID, in);
	sendReport (request);
	while (true) {
		HIDPP::Report response = getReport ();
		
		if (response.deviceIndex () != deviceIndex ())
			continue;

		uint8_t r_feature_index, error_code;
		unsigned int r_function, r_sw_id;
		if (response.checkErrorMessage20 (&r_feature_index,
						  &r_function,
						  &r_sw_id,
						  &error_code)) {
			if (r_feature_index != feature_index ||
			    r_function != function ||
			    r_sw_id != softwareID)
				continue;
			throw Error (static_cast<Error::ErrorCode> (error_code));
		}
		if (response.featureIndex () == feature_index &&
		    response.function () == function &&
		    response.softwareID () == softwareID)
			return response.params ();
	}
}
