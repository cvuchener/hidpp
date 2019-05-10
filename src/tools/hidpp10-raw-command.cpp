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

#include <hidpp/SimpleDispatcher.h>
#include <hidpp10/Device.h>
#include <hidpp10/Error.h>
#include <cstdio>
#include <memory>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "device_path command read|write short|long [parameters...]";
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

	if (argc-first_arg < 4) {
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

	char *endptr;

	int address = strtol (argv[first_arg+1], &endptr, 0);
	if (*endptr != '\0' || address < 0 || address > 255) {
		fprintf (stderr, "Invalid register address.\n");
		return EXIT_FAILURE;
	}
	std::string type = argv[first_arg+2];
	std::string size_string = argv[first_arg+3];
	std::size_t register_size;
	if (size_string == "short")
		register_size = HIDPP::ShortParamLength;
	else if (size_string == "long")
		register_size = HIDPP::LongParamLength;
	else {
		fprintf (stderr, "Invalid length option (must be short or long).\n");
		return EXIT_FAILURE;
	}

	HIDPP10::Device dev (dispatcher.get (), device_index);
	std::vector<uint8_t> params, results;
	for (int i = 0; first_arg+4+i < argc; ++i) {
		int value = strtol (argv[first_arg+4+i], &endptr, 16);
		if (*endptr != '\0' || value < 0 || value > 255) {
			fprintf (stderr, "Invalid parameter %d value.\n", i);
			return EXIT_FAILURE;
		}
		params.push_back (static_cast<uint8_t> (value));
	}

	try {
		if (type == "read") {
			if (params.size () > HIDPP::ShortParamLength) {
				fprintf (stderr, "Too many parameters.\n");
				return EXIT_FAILURE;
			}
			params.resize (HIDPP::ShortParamLength, 0);
			results.resize (register_size);
			dev.getRegister (static_cast<uint8_t> (address),
					 &params, results);
		}
		else if (type == "write") {
			if (params.size () > register_size) {
				fprintf (stderr, "Too many parameters.\n");
				return EXIT_FAILURE;
			}
			params.resize (register_size, 0);
			dev.setRegister (static_cast<uint8_t> (address),
					 params, &results);
		}
	}
	catch (HIDPP10::Error &e) {
		fprintf (stderr, "%s\n", e.what ());
		return e.errorCode ();
	}

	bool first = true;
	for (uint8_t value: results) {
		if (first)
			first = false;
		else 
			printf (" ");
		printf ("%02hhx", value);
	}
	printf ("\n");

	return EXIT_SUCCESS;
}

