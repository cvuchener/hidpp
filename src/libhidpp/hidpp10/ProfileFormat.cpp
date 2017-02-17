/*
 * Copyright 2016 Clément Vuchener
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

#include "ProfileFormat.h"

#include <hidpp10/Device.h>
#include <hidpp10/DeviceInfo.h>

#include <hidpp10/ProfileFormatG9.h>
#include <hidpp10/ProfileFormatG500.h>
#include <hidpp10/ProfileFormatG700.h>

using namespace HIDPP;
using namespace HIDPP10;

std::unique_ptr<AbstractProfileFormat> HIDPP10::getProfileFormat (HIDPP10::Device *device)
{
	auto info = getMouseInfo (device->productID ());
	switch (info->profile_type) {
	case G9ProfileType:
		return std::unique_ptr<AbstractProfileFormat> (new ProfileFormatG9 (*info->sensor));
	case G500ProfileType:
		return std::unique_ptr<AbstractProfileFormat> (new ProfileFormatG500 (*info->sensor));
	case G700ProfileType:
		return std::unique_ptr<AbstractProfileFormat> (new ProfileFormatG700 (*info->sensor));
	default:
		throw std::runtime_error ("Unsupported device");
	}
}

