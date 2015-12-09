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

extern "C" {
#include <unistd.h>
}

#include <hidpp10/Device.h>
#include <hidpp10/DeviceInfo.h>
#include <hidpp10/MemoryMapping.h>
#include <hidpp10/ProfileDirectory.h>
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
	static const char *args = "/dev/hidrawX";
	HIDPP::DeviceIndex device_index = HIDPP::WiredDevice;

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

	Device dev (path, device_index);
	const MouseInfo *info = getMouseInfo (dev.productID ());
	MemoryMapping mem (&dev);

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
	
	std::string xml;
	while (std::cin) {
		char buffer[4096];
		std::cin.read (buffer, sizeof (buffer));
		xml.append (buffer, std::cin.gcount ());
	}

	tinyxml2::XMLDocument doc;
	doc.Parse (xml.c_str ());
	if (doc.Error ()) {
		fprintf (stderr, "Error parsing XML:\n%s\n", doc.GetErrorStr2 ());
		return EXIT_FAILURE;
	}

	ProfileDirectory profile_dir;
	std::vector<std::pair<Profile *, std::vector<Macro>>> profiles;
	uint8_t next_page = 2;

	const XMLElement *root = doc.RootElement ();
	const XMLElement *element = root->FirstChildElement ("profile");
	while (element) {
		profiles.emplace_back (new_profile (), std::vector<Macro> ());

		Profile *profile = profiles.back ().first;
		std::vector<Macro> &macros = profiles.back ().second;

		macros.resize (profile->buttonCount ());
		profile_dir.push_back (ProfileEntry ({{next_page++, 0}, 0}));

		xml_to_profile (element, profile, macros);

		element = element->NextSiblingElement ("profile");
	}

	Address next_address = { next_page, 0 };
	auto profile_entry = profile_dir.begin ();
	for (auto &pair: profiles) {
		Profile *profile = pair.first;
		std::vector<Macro> &macros = pair.second;

		for (unsigned int i = 0; i < profile->buttonCount (); ++i) {
			Profile::Button &button = profile->button (i);
			if (button.type () == Profile::Button::Macro) {
				button.setMacro (next_address);
				next_address = macros[i].write (mem, next_address);
			}
		}
		ByteArray &page = mem.getWritablePage (profile_entry->address.page);
		profile->write (page.begin () + profile_entry->address.offset);

		delete profile;

		++profile_entry;
	}
	profile_dir.write (&mem);

	mem.sync ();

	return EXIT_SUCCESS;
}
