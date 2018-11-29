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

#include <hidpp/SimpleDispatcher.h>
#include <hidpp10/Device.h>
#include <hidpp10/DeviceInfo.h>
#include <hidpp10/defs.h>
#include <hidpp10/IProfile.h>
#include <hidpp10/RAMMapping.h>
#include <hidpp10/ProfileFormat.h>
#include <hidpp10/ProfileDirectoryFormat.h>
#include <hidpp10/MacroFormat.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

#include "profile/ProfileXML.h"

using namespace HIDPP10;
using namespace tinyxml2;
using HIDPP::Address;
using HIDPP::Profile;
using HIDPP::ProfileDirectory;
using HIDPP::Macro;

int main (int argc, char *argv[])
{
	static const char *args = "device_path [file]";
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

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (argv[first_arg]);
	}
	catch (std::exception &e) {
		fprintf (stderr, "Failed to open device: %s.\n", e.what ());
		return EXIT_FAILURE;
	}

	HIDPP10::Device dev (dispatcher.get (), device_index);
	IProfile iprofile (&dev);
	auto profile_format = getProfileFormat (&dev);
	auto profdir_format = getProfileDirectoryFormat (&dev);
	auto macro_format = getMacroFormat (&dev);
	RAMMapping memory (&dev);

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
		fprintf (stderr, "Error parsing XML:\n%s\n", doc.ErrorStr ());
		return EXIT_FAILURE;
	}

	Profile profile;
	ProfileDirectory::Entry entry; // Not actually written for temporary profiles
	std::vector<Macro> macros;

	ProfileXML profxml (profile_format.get (), profdir_format.get ());
	profxml.read (doc.RootElement (), profile, entry, macros);

	try {
		HIDPP::Address macro_address { 0, 0, (profile_format->size ()+1)/2 };
		for (unsigned int i = 0; i < profile.buttons.size (); ++i) {
			auto &button = profile.buttons[i];
			if (button.type () == HIDPP::Profile::Button::Type::Macro) {
				auto &macro = macros[i];
				auto next_address = macro.write (*macro_format, memory, macro_address);
				button.setMacro (macro_address);
				macro_address = next_address;
			}
		}
	}
	catch (std::out_of_range &e) {
		fprintf (stderr, "Cannot write macros: too long for RAM.\n");
		return EXIT_FAILURE;
	}
	Address profile_addr { 0, 0, 0 };
	profile_format->write (profile, memory.getWritableIterator (profile_addr));

	memory.sync ();
	iprofile.loadProfileFromAddress (profile_addr);

	return EXIT_SUCCESS;
}
