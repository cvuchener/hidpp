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

#ifndef LIBHIDPP_HIDPP_DEFS_H
#define LIBHIDPP_HIDPP_DEFS_H

#include <cstdint>
#include <array>

namespace HIDPP
{
	/**
	 * \defgroup hidpp HID++
	 * @{
	 */

	/**
	 * HID++ device index.
	 *
	 * Receiver and wireless devices share the same
	 * hidraw device. The device index is used to direct
	 * the HID++ report to a particular device.
	 */
	enum DeviceIndex: uint8_t {
		DefaultDevice = 0xff, // used by receiver or corded or bluetooth devices
		CordedDevice = 0, // used by older corded devices
		WirelessDevice1 = 1,
		WirelessDevice2 = 2,
		WirelessDevice3 = 3,
		WirelessDevice4 = 4,
		WirelessDevice5 = 5,
		WirelessDevice6 = 6,
	};

	/**@}*/
}

#endif
