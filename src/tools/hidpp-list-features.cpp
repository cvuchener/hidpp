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

#include <hidpp/Device.h>
#include <hidpp10/Device.h>
#include <hidpp10/Error.h>
#include <hidpp20/Device.h>
#include <hidpp20/IFeatureSet.h>
#include <misc/SysCallError.h>

extern "C" {
#include <getopt.h>
}

void testRegister (HIDPP10::Device *dev, std::size_t register_size, uint8_t address, bool test_write = false)
{
	HIDPP::Parameters values (register_size);
	bool readable = false;
	try {
		dev->getRegister (address, nullptr, values);
		readable = true;
		printf ("Register 0x%02hhx read  %2lu:", address, register_size);
		for (uint8_t value: values)
			printf (" %02hhx", value);
		printf ("\n");
	}
	catch (HIDPP10::Error e) {
		if (e.errorCode () != HIDPP10::Error::InvalidSubID &&
		    e.errorCode () != HIDPP10::Error::InvalidAddress) {
			printf ("Register 0x%02hhx read  %2lu: %s (0x%02hhx)\n",
				address, register_size, e.what (), e.errorCode ());
		}
	}
	if (test_write) {
		try {
			HIDPP::Parameters results;
			if (readable) {
				dev->setRegister (address, values, &results);
			}
			else {
				HIDPP::Parameters params (register_size);
				dev->setRegister (address, params, &results);
			}
			printf ("Register 0x%02hhx write %2lu:", address, register_size);
			for (uint8_t value: results)
				printf (" %02hhx", value);
			printf ("\n");
		}
		catch (HIDPP10::Error e) {
			if (e.errorCode () != HIDPP10::Error::InvalidSubID &&
			    e.errorCode () != HIDPP10::Error::InvalidAddress) {
				printf ("Register 0x%02hhx write %2lu: %s (0x%02hhx)\n",
					address, register_size, e.what (), e.errorCode ());
			}
		}
	}
}

const char *usage =
	"Usage: %s [options] /dev/hidrawX\n"
	"Options are:\n"
	"	-w,--write	Also do write tests with HID++ 1.0 devices.\n"
	"	-d,--device index	Use wireless device index.\n"
	"	-h,--help	Print this help.\n"
	;

int main (int argc, char *argv[])
{
	enum Options {
		WriteTestOpt = 256,
		DeviceIndexOpt,
		HelpOpt,
	};
	struct option longopts[] = {
		{ "write", no_argument, nullptr, WriteTestOpt },
		{ "device", required_argument, nullptr, DeviceIndexOpt },
		{ "help", no_argument, nullptr, HelpOpt },
		{ },
	};

	HIDPP::DeviceIndex device_index = HIDPP::WiredDevice;
	bool do_write_tests = false;

	int opt;
	while (-1 != (opt = getopt_long (argc, argv, "wd:h", longopts, nullptr))) {
		switch (opt) {
		case 'w':
		case WriteTestOpt:
			do_write_tests = true;
			break;

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
			fprintf (stderr, usage, argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (argc-optind != 1) {
		fprintf (stderr, usage, argv[0]);
		return EXIT_FAILURE;
	}

	const char *path = argv[optind];

	/*
	 * Check protocol version
	 */
	unsigned int major, minor;
	try {
		HIDPP::Device dev (path, device_index);
		dev.getProtocolVersion (major, minor);
		printf ("%s (%04hx:%04hx) is a HID++ %d.%d device\n",
			dev.name ().c_str (),
			dev.vendorID (), dev.productID (),
			major, minor);
	
	}
	catch (HIDPP::Device::NoHIDPPReportException e) {
		printf ("%s is not a HID++ device\n", path);
		return EXIT_FAILURE;
	}
	catch (SysCallError e) {
		fprintf (stderr, "Failed to open %s: %s\n", path, e.what ());
		return EXIT_FAILURE;
	}
	/*
	 * HID++ 1.0
	 */
	if (major == 1 && minor == 0) {
		HIDPP10::Device dev (path, device_index);

		for (unsigned int address = 0; address < 256; ++address) {
			testRegister (&dev, HIDPP::ShortParamLength, static_cast<uint8_t> (address), do_write_tests);
			testRegister (&dev, HIDPP::LongParamLength, static_cast<uint8_t> (address), do_write_tests);
		}
	}
	/*
	 * HID++ 2.0
	 */
	else if (major == 2 && minor == 0) {
		HIDPP20::Device dev (path, device_index);
		HIDPP20::IFeatureSet ifeatureset (&dev);

		unsigned int feature_count = ifeatureset.getCount ();
		for (unsigned int i = 1; i <= feature_count; ++i) {
			uint8_t feature_index = i;
			uint16_t feature_id;
			bool obsolete, hidden;

			feature_id = ifeatureset.getFeatureID (feature_index, &obsolete, &hidden);
			printf ("Feature 0x%02hhx: [0x%04hx]", feature_index, feature_id);
			if (obsolete)
				printf (" obsolete");
			if (hidden)
				printf (" hidden");
			printf ("\n");
		}
	}
	else {
		fprintf (stderr, "Unsupported HID++ protocol version.\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

