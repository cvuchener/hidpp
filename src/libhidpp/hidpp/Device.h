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

class Device: public HIDRaw
{
public:
	class NoHIDPPReportException: public std::exception
	{
	public:
		NoHIDPPReportException ();
		virtual const char *what () const noexcept;
	};

	Device (const std::string &path, DeviceIndex device_index = WiredDevice);

	DeviceIndex deviceIndex () const;

	void getProtocolVersion (unsigned int &major, unsigned int &minor);

	int sendReport (const Report &report);
	Report getReport ();

private:
	DeviceIndex _device_index;
};

}

#endif
