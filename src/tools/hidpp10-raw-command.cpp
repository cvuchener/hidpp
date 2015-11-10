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

#include <hidpp10/Device.h>
#include <hidpp10/Error.h>
#include <cstdio>

extern "C" {
#include <getopt.h>
}

const char *usage =
	"Usage: %s [options] /dev/hidrawX command read|write short|long parameters...\n"
	"Options are:\n"
	"	-d,--device index	Use wireless device index.\n"
	"	-h,--help	Print this message.\n"
	;

int main (int argc, char *argv[])
{
	enum Options {
		DeviceIndexOpt = 256,
		HelpOpt,
	};
	struct option longopts[] = {
		{ "device", required_argument, nullptr, DeviceIndexOpt },
		{ "help", no_argument, nullptr, HelpOpt },
		{ }
	};

	HIDPP::DeviceIndex device_index = HIDPP::WiredDevice;

	int opt;
	while (-1 != (opt = getopt_long (argc, argv, "d:h", longopts, nullptr))) {
		switch (opt) {
		case 'd':
		case DeviceIndexOpt: {
                        int index = atoi (optarg);
                        switch (index) {
                        case 1:
                                device_index = HIDPP::WirelessDevice1;
                                break;
                        case 2:
                                device_index = HIDPP::WirelessDevice2;
                                break;
                        case 3:
                                device_index = HIDPP::WirelessDevice3;
                                break;
                        case 4:
                                device_index = HIDPP::WirelessDevice4;
                                break;
                        case 5:
                                device_index = HIDPP::WirelessDevice5;
                                break;
                        case 6:
                                device_index = HIDPP::WirelessDevice6;
                                break;
                        default:
                                fprintf (stderr, "Invalid device index: %s\n", optarg);
                                return EXIT_SUCCESS;
                        }
                        break;
                }
		
		case 'h':
		case HelpOpt:
			fprintf (stderr, usage, argv[0]);
			return EXIT_SUCCESS;

		default:
			return EXIT_FAILURE;
		}
	}

	if (argc-optind < 4) {
		fprintf (stderr, "Too few arguments.\n");
		fprintf (stderr, usage, argv[0]);
		return EXIT_FAILURE;
	}

	char *endptr;
	
	const char *path = argv[optind];
	int address = strtol (argv[optind+1], &endptr, 0);
	if (*endptr != '\0' || address < 0 || address > 255) {
		fprintf (stderr, "Invalid register address.\n");
		return EXIT_FAILURE;
	}
	std::string type = argv[optind+2];
	std::string size_string = argv[optind+3];
	std::size_t register_size;
	if (size_string == "short")
		register_size = HIDPP::ShortParamLength;
	else if (size_string == "long")
		register_size = HIDPP::LongParamLength;
	else {
		fprintf (stderr, "Invalid length option (must be short or long).\n");
		return EXIT_FAILURE;
	}

	HIDPP10::Device dev (path, device_index);
	HIDPP::Parameters params, results;
	for (int i = 0; optind+4+i < argc; ++i) {
		int value = strtol (argv[optind+4+i], &endptr, 16);
		if (*endptr != '\0' || value < 0 || value > 255) {
			fprintf (stderr, "Invalid parameter %d value.\n", i);
			return EXIT_FAILURE;
		}
		params[i] = static_cast<uint8_t> (value);
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
	catch (HIDPP10::Error e) {
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

