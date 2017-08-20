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

#ifndef LIBHIDPP_HIDPP10_DEVICE_INFO_H
#define LIBHIDPP_HIDPP10_DEVICE_INFO_H

#include <hidpp/DeviceInfo.h>

#include <hidpp10/Sensor.h>
#include <hidpp10/IResolution.h>
#include <hidpp10/ProfileFormat.h>

namespace HIDPP10
{
	struct MouseInfo: HIDPP::DeviceInfo
	{
		const Sensor *sensor;
		IResolutionType iresolution_type;
		ProfileType profile_type;
		unsigned int default_profile_page;

		MouseInfo (const Sensor *sensor,
			   IResolutionType iresolution_type,
			   ProfileType profile_type,
			   unsigned int default_profile_page = 2):
			HIDPP::DeviceInfo (HIDPP::DeviceInfo::Device),
			sensor (sensor),
			iresolution_type (iresolution_type),
			profile_type (profile_type),
			default_profile_page (default_profile_page)
		{
		}
	};

	inline const MouseInfo *getMouseInfo (uint16_t product_id)
	{
		return dynamic_cast<const MouseInfo *> (HIDPP::getDeviceInfo (product_id));
	}
}

#endif
