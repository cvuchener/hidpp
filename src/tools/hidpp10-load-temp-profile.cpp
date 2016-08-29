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
#include <iostream>
#include <fstream>

extern "C" {
#include <unistd.h>
}

#include <hidpp10/Device.h>
#include <hidpp10/DeviceInfo.h>
#include <hidpp10/defs.h>
#include <hidpp10/IMemory.h>
#include <hidpp10/IProfile.h>
#include <hidpp10/Profile.h>
#include <hidpp10/Macro.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

#include "profile/ProfileXML.h"

using namespace HIDPP10;
using namespace tinyxml2;

int main (int argc, char *argv[])
{
	static const char *args = "/dev/hidrawX [file]";
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

	if (argc-first_arg < 1 || argc-first_arg > 2) {
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];

	Device dev (path, device_index);
	const MouseInfo *info = getMouseInfo (dev.productID ());
	IMemory imemory (&dev);
	IProfile iprofile (&dev);

	std::function<Profile * ()> new_profile;
	XMLToProfile xml_to_profile;

	switch (info->profile_type) {
	case G500ProfileType:
		new_profile = [&info] () {
			return new G500Profile (info->sensor);
		};
		xml_to_profile = XMLToG500Profile;
		break;

	default:
		fprintf (stderr, "Profile format not supported\n");
		return EXIT_FAILURE;
	}

	// Read XML input
	std::string xml;
	std::ifstream file;
	std::istream *input;
	if (argc-first_arg == 2) {
		file.open (argv[first_arg+1]);
		input = &file;
	}
	else {
		input = &std::cin;
	}
	while (*input) {
		char buffer[4096];
		input->read (buffer, sizeof (buffer));
		xml.append (buffer, input->gcount ());
	}

	tinyxml2::XMLDocument doc;
	doc.Parse (xml.c_str ());
	if (doc.Error ()) {
		fprintf (stderr, "Error parsing XML:\n%s\n", doc.GetErrorStr2 ());
		return EXIT_FAILURE;
	}

	Profile *profile= new_profile ();
	std::vector<Macro> macros (profile->buttonCount ());

	xml_to_profile (doc.RootElement (), profile, macros);

	ByteArray data (RAMSize);
	unsigned int profile_len = profile->profileLength ();
	ByteArray::iterator next_pos = data.begin () + profile_len + profile_len%2;
	for (unsigned int i = 0; i < profile->buttonCount (); ++i) {
		Profile::Button &button = profile->button (i);
		if (button.type () == Profile::Button::Macro) {
			Address addr = {
				0,
				static_cast<uint8_t> ((next_pos - data.begin ()) / 2)
			};
			button.setMacro (addr);
			next_pos = macros[i].write (next_pos, addr);
		}
	}
	profile->write (data.begin ());
	delete profile;

	Address profile_addr = {0, 0};
	imemory.writeMem (profile_addr, data);
	iprofile.loadProfileFromAddress (profile_addr);

	return EXIT_SUCCESS;
}
