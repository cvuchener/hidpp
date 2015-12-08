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
	ProfileToXML profile_to_xml;
	
	switch (info->profile_type) {
	case G500ProfileType:
		new_profile = [&info] () {
			return new G500Profile (info->sensor);
		};
		profile_to_xml = G500ProfileToXML;
		break;

	default:
		fprintf (stderr, "Profile format not supported\n");
		return EXIT_FAILURE;
	}

	XMLPrinter printer;
	XMLDocument doc;
	XMLElement *root = doc.NewElement ("profiles");

	ProfileDirectory profile_dir (&mem);
	for (unsigned int i = 0; i < profile_dir.size (); ++i) {
		const ProfileEntry &entry = profile_dir[i];
		Profile *profile = new_profile ();
		const ByteArray &page = mem.getReadOnlyPage (entry.page);
		profile->read (page.begin () + entry.offset*2);

		std::vector<Macro> macros (profile->buttonCount ());
		for (unsigned int j = 0; j < profile->buttonCount (); ++j) {
			const Profile::Button &button = profile->button (j);
			if (button.type () == Profile::Button::Macro) {
				Address address = {
					button.macroPage (),
					button.macroOffset ()
				};
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
	printf ("%s", printer.CStr ());

	return EXIT_SUCCESS;
}
