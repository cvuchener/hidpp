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
#include <misc/CRC.h>
#include <misc/Endian.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "device_path page offset";
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

	if (argc-first_arg != 3) {
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];
	char *end;
	int page = strtol (argv[first_arg+1], &end, 0);
	if (*end != '\0' || page < 0) {
		fprintf (stderr, "Invalid page index.\n");
		return EXIT_FAILURE;
	}
	int offset = strtol (argv[first_arg+2], &end, 0);
	if (*end != '\0' || offset < 0) {
		fprintf (stderr, "Invalid offset.\n");
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

	std::vector<uint8_t> data;
	uint8_t buffer[256];
	int ret;
	while ((ret = read (0, buffer, sizeof (buffer))) > 0) {
		data.insert (data.end (), buffer, buffer+ret);
	}
	if (ret == -1) {
		perror ("read");
		return EXIT_FAILURE;
	}

	try {
		HIDPP20::IOnboardProfiles iop (&dev);
		iop.memoryAddrWrite (page, offset, data.size ());

		constexpr auto LineSize = HIDPP20::IOnboardProfiles::LineSize;
		for (unsigned int i = 0; i < data.size (); i += LineSize)
			iop.memoryWrite (data.begin () + i, data.begin () + i + LineSize);

		iop.memoryWriteEnd ();
	}
	catch (HIDPP20::Error e) {
		fprintf (stderr, "Error while writing data: %s (%d).\n", e.what (), e.errorCode ());
		return e.errorCode ();
	}

	return EXIT_SUCCESS;
}
