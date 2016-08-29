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

#ifndef HIDPP_DEVICE_H
#define HIDPP_DEVICE_H

#include <misc/HIDRaw.h>
#include <hidpp/defs.h>
#include <hidpp/Report.h>

namespace HIDPP
{

/**
 * A generic HID++ Device
 *
 * \ingroup hidpp
 */
class Device: public HIDRaw
{
public:
	/**
	 * Exception when no HID++ report is found in the report descriptor.
	 */
	class NoHIDPPReportException: public std::exception
	{
	public:
		NoHIDPPReportException ();
		virtual const char *what () const noexcept;
	};

	/**
	 * HID++ device constructor.
	 *
	 * Open the hidraw device node at \p path.
	 *
	 * For receivers and wireless devices, multiple devices use the same hidraw
	 * node, \p device_index is needed to select a particular device.
	 *
	 * \throws SysCallError
	 * \throws NoHIDPPReportException
	 * \throws HIDPP10::Error Only for wireless devices, if there is an error
	 *                        while reading device information.
	 */
	Device (const std::string &path, DeviceIndex device_index = DefaultDevice);

	/**
	 * Access the device index.
	 */
	DeviceIndex deviceIndex () const;

	/**
	 * Check the HID++ protocol version.
	 *
	 * \param major	Major number of the protocol version.
	 * \param minor Minor number of the protocol version.
	 */
	void getProtocolVersion (unsigned int &major, unsigned int &minor);

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
	 * Send a HID++ report to this device.
	 */
	int sendReport (const Report &report);
	/**
	 * Read a HID++ report from this device.
	 *
	 * It discards any non-HID++ report.
	 *
	 * \param timeout If true, try to read a report with a time out, throw HIDRaw::TimeoutError if no valid report is read fast enough.
	 */
	Report getReport (bool timeout = false);

private:
	DeviceIndex _device_index;
	uint16_t _product_id;
	std::string _name;
};

}

#endif
