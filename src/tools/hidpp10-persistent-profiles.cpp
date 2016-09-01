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
	static const char *args = "/dev/hidrawX read|write [file]";
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

	if (argc-first_arg < 2 || argc-first_arg > 3) {
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];
	std::string op = argv[first_arg+1];

	Device dev (path, device_index);
	const MouseInfo *info = getMouseInfo (dev.productID ());
	MemoryMapping mem (&dev);

	std::function<Profile * ()> new_profile;
	XMLToProfile xml_to_profile;
	ProfileToXML profile_to_xml;

	switch (info->profile_type) {
	case G500ProfileType:
		new_profile = [&info] () {
			return new G500Profile (info->sensor);
		};
		xml_to_profile = XMLToG500Profile;
		profile_to_xml = G500ProfileToXML;
		break;

	default:
		fprintf (stderr, "Profile format not supported\n");
		return EXIT_FAILURE;
	}

	if (op == "write") {
		// Read XML input
		std::string xml;
		std::ifstream file;
		std::istream *input;
		if (argc-first_arg == 3) {
			file.open (argv[first_arg+2]);
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

		// Parse XML
		tinyxml2::XMLDocument doc;
		doc.Parse (xml.c_str ());
		if (doc.Error ()) {
			fprintf (stderr, "Error parsing XML:\n%s\n", doc.GetErrorStr2 ());
			return EXIT_FAILURE;
		}

		ProfileDirectory profile_dir;
		std::vector<std::pair<Profile *, std::vector<Macro>>> profiles;
		// The first profile will be written on page 2
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

		// Macro are written from the next page after profiles
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
			std::vector<uint8_t> &page = mem.getWritablePage (profile_entry->address.page);
			profile->write (page.begin () + profile_entry->address.offset);

			delete profile;

			++profile_entry;
		}
		profile_dir.write (&mem);

		mem.sync ();
	}
	else if (op == "read") {
		XMLPrinter printer;
		XMLDocument doc;
		XMLElement *root = doc.NewElement ("profiles");

		ProfileDirectory profile_dir (&mem);
		for (unsigned int i = 0; i < profile_dir.size (); ++i) {
			const ProfileEntry &entry = profile_dir[i];
			Profile *profile = new_profile ();
			const std::vector<uint8_t> &page = mem.getReadOnlyPage (entry.address.page);
			profile->read (page.begin () + entry.address.offset*2);

			std::vector<Macro> macros (profile->buttonCount ());
			for (unsigned int j = 0; j < profile->buttonCount (); ++j) {
				const Profile::Button &button = profile->button (j);
				if (button.type () == Profile::Button::Macro) {
					Address address = button.macro ();
					macros.emplace (macros.begin () + j,
							mem, address)->simplify ();
				}
			}

			XMLElement *element = doc.NewElement ("profile");
			profile_to_xml (profile, macros, element);
			root->InsertEndChild (element);

			delete profile;
		}
		doc.InsertEndChild (root);
		doc.Print (&printer);

		// Read XML input
		std::ofstream file;
		std::ostream *output;
		if (argc-first_arg == 3) {
			file.open (argv[first_arg+2]);
			output = &file;
		}
		else {
			output = &std::cout;
		}
		*output << printer.CStr ();

	}
	else {
		fprintf (stderr, "Invalid operation.\n");
		return EXIT_FAILURE;
	}




	return EXIT_SUCCESS;
}
