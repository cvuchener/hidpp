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

#include <hidpp/SimpleDispatcher.h>
#include <hidpp/Device.h>
#include <hidpp10/Error.h>
#include <misc/Log.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "device_path";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;

	std::vector<Option> options = {
		DeviceIndexOption (device_index),
		VerboseOption (),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	if (argc-first_arg != 1) {
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];

	try {
		unsigned int major, minor;
		HIDPP::SimpleDispatcher dispatcher (path);
		try {
			HIDPP::Device dev (&dispatcher, device_index);
			std::tie (major, minor) = dev.protocolVersion ();
			printf ("%d.%d\n", major, minor);
			Log::info ().printf ("Device is %s (%04hx:%04hx)\n",
					     dev.name ().c_str (),
					     dispatcher.hidraw ().vendorID (), dev.productID ());
		}
		catch (std::exception &e) {
			fprintf (stderr, "Error accessing device: %s\n", e.what ());
			return EXIT_FAILURE;
		}
	}
	catch (HIDPP::Dispatcher::NoHIDPPReportException &e) {
		Log::info () << "Device is not a HID++ device" << std::endl;
		return EXIT_FAILURE;
	}
	catch (std::system_error &e) {
		fprintf (stderr, "Failed to open %s: %s\n", path, e.what ());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

