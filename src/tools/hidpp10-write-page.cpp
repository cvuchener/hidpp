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

extern "C" {
#include <unistd.h>
#include <getopt.h>
}

#include <hidpp10/Device.h>
#include <hidpp10/IMemory.h>

const char *usage = 
	"Usage: %s [options] /dev/hidrawX page\n"
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

	if (argc-optind != 2) {
		fprintf (stderr, usage, argv[0]);
		return EXIT_FAILURE;
	}

	const char *path = argv[optind];
	unsigned int page = atoi (argv[optind+1]);

	HIDPP10::Device dev (path, device_index);
	static constexpr std::size_t PageSize = 512;
	std::vector<uint8_t> data (PageSize);

	std::size_t r = 0;
	while (r < PageSize) {
		int ret = read (0, &data[r], PageSize - r);
		if (ret == -1) {
			perror ("read");
			return EXIT_FAILURE;
		}
		if (ret == 0) {
			break;
		}
		r += ret;
	}

	HIDPP10::IMemory (&dev).writePage (page, data);

	return EXIT_SUCCESS;
}
