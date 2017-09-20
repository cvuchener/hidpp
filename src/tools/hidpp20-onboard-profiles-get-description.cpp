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
#include <memory>

#include <hidpp/SimpleDispatcher.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/IOnboardProfiles.h>
#include <cstdio>

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

	if (argc-first_arg < 1) {
		fprintf (stderr, "Too few arguments.\n");
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (argv[first_arg]);
	}
	catch (std::exception &e) {
		fprintf (stderr, "Failed to open device: %s.\n", e.what ());
		return EXIT_FAILURE;
	}

	HIDPP20::Device dev (dispatcher.get (), device_index);
	try {
		HIDPP20::IOnboardProfiles iop (&dev);
		auto desc = iop.getDescription ();
		printf ("Memory model:\t%hhu\n", static_cast<uint8_t> (desc.memory_model));
		printf ("Profile format:\t%hhu\n", static_cast<uint8_t> (desc.profile_format));
		printf ("Macro format:\t%hhu\n", static_cast<uint8_t> (desc.macro_format));
		printf ("Profile count:\t%hhu\n", desc.profile_count);
		printf ("Profile count OOB:\t%hhu\n", desc.profile_count_oob);
		printf ("Button count:\t%hhu\n", desc.button_count);
		printf ("Sector count:\t%hhu\n", desc.sector_count);
		printf ("Sector size:\t%hu\n", desc.sector_size);
		printf ("Mechanical layout:\t0x%hhx (", desc.mechanical_layout);
		bool first = true;
		if ((desc.mechanical_layout & 0x03) == 2) {
			if (first)
				first = false;
			else
				printf  (", ");
			printf ("G-shift");
		}
		if ((desc.mechanical_layout & 0x0c) >> 2 == 2) {
			if (first)
				first = false;
			else
				printf  (", ");
			printf ("DPI shift");
		}
		printf (")\n");
		printf ("Various info:\t0x%hhx (", desc.various_info);
		switch (desc.various_info & 0x07) {
		case 1:
			printf ("Corded");
			break;
		case 2:
			printf ("Wireless");
			break;
		case 4:
			printf ("Corded + Wireless");
			break;
		}
		printf (")\n");
	}
	catch (HIDPP20::Error e) {
		fprintf (stderr, "Error code %d: %s\n", e.errorCode (), e.what ());
		return e.errorCode ();
	}

	return EXIT_SUCCESS;
}

