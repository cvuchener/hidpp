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

#ifndef LIBHIDPP_HIDPP_DEVICE_H
#define LIBHIDPP_HIDPP_DEVICE_H

#include <hidpp/defs.h>
#include <hidpp/Report.h>
#include <tuple>

namespace HIDPP
{

class Dispatcher;

/**
 * A generic HID++ Device
 *
 * \ingroup hidpp
 */
class Device
{
public:
	class InvalidProtocolVersion: public std::exception
	{
	public:
		InvalidProtocolVersion (const std::tuple<unsigned int, unsigned int> &version);
		const char *what () const noexcept;

	private:
		std::string _msg;
	};

	/**
	 * HID++ device constructor.
	 *
	 * For receivers and wireless devices, multiple devices use the same hidraw
	 * node, \p device_index is needed to select a particular device.
	 *
	 * The constructor ask the receiver for information for wireless devices and
	 * check the protocol version. If something goes wrong it may throw HID++ errors
	 * (HIDPP10::Error or HIDPP20::Error) or other exception (e.g. std::system_error
	 * for errors with the HID node).
	 */
	Device (Dispatcher *dispatcher, DeviceIndex device_index = DefaultDevice);

	Dispatcher *dispatcher () const;

	/**
	 * Access the device index.
	 */
	DeviceIndex deviceIndex () const;

	/**
	 * Get the product ID of the device.
	 *
	 *  - Use HID product ID for wired device or receivers.
	 *  - Use wireless PID given by the receiver for wireless devices.
	 */
	uint16_t productID () const;

	/**
	 * Get the product name of the device.
	 *
	 *  - Use HID product name for wired device or receivers.
	 *  - Use the name given by the receiver for wireless devices.
	 */
	std::string name () const;

	/**
	 * Get the version for the device with the given index.
	 *
	 * Some devices fail to answer with a valid error message
	 * when the device index is not supported. This methods throws
	 * TimeourError if an answer is not received fast enough.
	 *
	 * \return Major and minor version number.
	 */
	std::tuple<unsigned int, unsigned int> protocolVersion ();

private:
	Dispatcher *_dispatcher;
	DeviceIndex _device_index;
	uint16_t _product_id;
	std::string _name;
	std::tuple<unsigned int, unsigned int> _version;
};

}

#endif
