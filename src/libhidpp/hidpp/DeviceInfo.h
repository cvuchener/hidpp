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

#ifndef LIBHIDPP_DEVICE_INFO_H
#define LIBHIDPP_DEVICE_INFO_H

#include <cstdint>

namespace HIDPP
{
	struct DeviceInfo
	{
		enum Type {
			Receiver,
			Device,
		} type;

		DeviceInfo (Type type): type (type) {}
		virtual ~DeviceInfo () {} // Make the structure polymorphic
	};

	const DeviceInfo *getDeviceInfo (uint16_t product_id);
}

#endif

