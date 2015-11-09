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

#include <cstdio>

#include "hidpp/Device.h"

int main (int argc, char *argv[])
{
	for (int i = 1; i < argc; ++i) {
		try {
			HIDPP::Device dev (argv[i]);
			unsigned int major, minor;
			dev.getProtocolVersion (major, minor);
			printf ("%s: %s (%04hx:%04hx) HID++ %d.%d\n",
				argv[i], dev.name ().c_str (),
				dev.vendorID (), dev.productID (),
				major, minor);
		}
		catch (HIDPP::Device::NoHIDPPReportException e) {
			printf ("%s: Not a HID++ device\n", argv[i]);
		}
		catch (std::runtime_error e) {
			fprintf (stderr, "Failed to open %s: %s\n", argv[i], e.what ());
		}
	}
	return 0;
}

