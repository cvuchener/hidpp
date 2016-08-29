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
#include <hidpp10/DeviceInfo.h>
#include <hidpp10/Sensor.h>
#include <hidpp10/IResolution.h>
#include <cstdio>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

int main (int argc, char *argv[])
{
	static const char *args = "/dev/hidrawX get|set dpi [y_dpi]";
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

	if (argc-first_arg < 2) {
		fprintf (stderr, "Too few arguments.\n");
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	char *endptr;
	
	const char *path = argv[first_arg];

	HIDPP10::Device dev (path, device_index);
	const HIDPP10::MouseInfo *info = HIDPP10::getMouseInfo (dev.productID ());
	if (!info) {
		fprintf (stderr, "Unsupported mice.\n");
		return EXIT_FAILURE;
	}

	std::string op = argv[first_arg+1];
	first_arg += 2;
	if (op == "get") {
		switch (info->iresolution_type) {
		case HIDPP10::IResolutionType0: {
			HIDPP10::IResolution0 iresolution (&dev, info->sensor);
			unsigned int dpi = iresolution.getCurrentResolution ();
			printf ("%u dpi\n", dpi);
			break;
		}

		case HIDPP10::IResolutionType3: {
			HIDPP10::IResolution3 iresolution (&dev, info->sensor);
			unsigned int x_dpi, y_dpi;
			iresolution.getCurrentResolution (x_dpi, y_dpi);
			printf ("X: %u dpi, Y: %u dpi\n", x_dpi, y_dpi);
			break;
		}
		default:
			fprintf (stderr, "Unsupported resolution type.\n");
			return EXIT_FAILURE;
		}
	}
	else if (op == "set") {
		switch (info->iresolution_type) {
		case HIDPP10::IResolutionType0: {
			if (argc - first_arg != 1) {
				fprintf (stderr, "This mouse need exactly one resolution.\n");
				return EXIT_FAILURE;
			}
			unsigned int dpi = strtol (argv[first_arg], &endptr, 10);
			if (*endptr != '\0') {
				fprintf (stderr, "Invalid resolution value.\n");
				return EXIT_FAILURE;
			}
			HIDPP10::IResolution0 iresolution (&dev, info->sensor);
			iresolution.setCurrentResolution (dpi);
			break;
		}

		case HIDPP10::IResolutionType3: {
			unsigned int x_dpi, y_dpi;
			if (argc - first_arg == 1) {
				x_dpi = strtol (argv[first_arg], &endptr, 10);
				if (*endptr != '\0') {
					fprintf (stderr, "Invalid resolution value.\n");
					return EXIT_FAILURE;
				}
				y_dpi = x_dpi;
			}
			else if (argc - first_arg == 2) {
				x_dpi = strtol (argv[first_arg], &endptr, 10);
				if (*endptr != '\0') {
					fprintf (stderr, "Invalid X resolution value.\n");
					return EXIT_FAILURE;
				}
				y_dpi = strtol (argv[first_arg+1], &endptr, 10);
				if (*endptr != '\0') {
					fprintf (stderr, "Invalid Y resolution value.\n");
					return EXIT_FAILURE;
				}
			}
			else {
				fprintf (stderr, "This mouse need one or two resolutions.\n");
				return EXIT_FAILURE;
			}

			HIDPP10::IResolution3 iresolution (&dev, info->sensor);
			iresolution.setCurrentResolution (x_dpi, y_dpi);
			break;
		}
		default:
			fprintf (stderr, "Unsupported resolution type.\n");
			return EXIT_FAILURE;
		}
	}
	else {
		fprintf (stderr, "Invalid operation: %s\n", op.c_str ());
		return EXIT_FAILURE;
	}
				
	return EXIT_SUCCESS;
}

