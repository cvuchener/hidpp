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

extern "C" {
#include <unistd.h>
}

#include <hidpp/SimpleDispatcher.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/IOnboardProfiles.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "device_path page";
	auto mem_type = HIDPP20::IOnboardProfiles::MemoryType::Writeable;
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;

	std::vector<Option> options = {
		DeviceIndexOption (device_index),
		VerboseOption (),
		Option ('r', "rom",
			Option::NoArgument, "",
			"Read data from ROM",
			[&mem_type] (const char *) -> bool {
				mem_type = HIDPP20::IOnboardProfiles::MemoryType::ROM;
				return true;
			}),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	if (argc-first_arg != 2) {
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];
	char *endptr;
	unsigned int page = strtol (argv[first_arg+1], &endptr, 0);
	if (*endptr != '\0') {
		fprintf (stderr, "Invalid page number.\n");
		return EXIT_FAILURE;
	}

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (path);
	}
	catch (std::exception &e) {
		fprintf (stderr, "Failed to open device: %s.\n", e.what ());
		return EXIT_FAILURE;
	}
	HIDPP20::Device dev (dispatcher.get (), device_index);
	try {
		HIDPP20::IOnboardProfiles iop (&dev);
		auto desc = iop.getDescription ();
		unsigned int i = 0;
		while (i < desc.sector_size) {
			auto data = iop.memoryRead (mem_type, page, i);
			write (1, data.data (), data.size ());
			i += data.size ();
		}
	}
	catch (HIDPP20::Error e) {
		fprintf (stderr, "HID++2 error %d: %s\n", e.errorCode (), e.what ());
		return e.errorCode ();
	}

	return EXIT_SUCCESS;
}
