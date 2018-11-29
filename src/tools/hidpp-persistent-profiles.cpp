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
#include <hidpp20/Device.h>
#include <hidpp10/ProfileDirectoryFormat.h>
#include <hidpp20/ProfileDirectoryFormat.h>
#include <hidpp10/ProfileFormat.h>
#include <hidpp20/ProfileFormat.h>
#include <hidpp10/MemoryMapping.h>
#include <hidpp20/MemoryMapping.h>
#include <hidpp10/MacroFormat.h>
#include <hidpp20/MacroFormat.h>
#include <hidpp10/DeviceInfo.h>
#include <misc/Log.h>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

#include "profile/ProfileXML.h"

using namespace HIDPP10;
using namespace tinyxml2;

int main (int argc, char *argv[])
{
	static const char *args = "device_path read|write [file]";
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

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (path);
	}
	catch (std::exception &e) {
		fprintf (stderr, "Failed to open device: %s.\n", e.what ());
		return EXIT_FAILURE;
	}

	unsigned int major, minor;
	HIDPP::Device generic_device (dispatcher.get (), device_index);
	std::tie (major, minor) = generic_device.protocolVersion () ;

	std::unique_ptr<HIDPP::Device> device;
	std::unique_ptr<HIDPP::AbstractProfileDirectoryFormat> profdir_format;
	std::unique_ptr<HIDPP::AbstractProfileFormat> profile_format;
	std::unique_ptr<HIDPP::AbstractMemoryMapping> memory;
	std::unique_ptr<HIDPP::AbstractMacroFormat> macro_format;
	HIDPP::Address dir_address, prof_address;

	/*
	 * HID++ 1.0
	 */
	if (major == 1 && minor == 0) {
		auto dev = new HIDPP10::Device (std::move (generic_device));
		const HIDPP10::MouseInfo *info = HIDPP10::getMouseInfo (dev->productID ());
		device.reset (dev);
		profdir_format = HIDPP10::getProfileDirectoryFormat (dev);
		profile_format = HIDPP10::getProfileFormat (dev);
		macro_format = HIDPP10::getMacroFormat (dev);
		memory.reset (new HIDPP10::MemoryMapping (dev));
		dir_address = HIDPP::Address { 0, 1, 0 };
		prof_address = HIDPP::Address { 0, info->default_profile_page, 0 };
	}
	/*
	 * HID++ 2.0 and later
	 */
	else if (major >= 2) {
		auto dev = new HIDPP20::Device (std::move (generic_device));
		device.reset (dev);
		profdir_format = HIDPP20::getProfileDirectoryFormat (dev);
		profile_format = HIDPP20::getProfileFormat (dev);
		macro_format = HIDPP20::getMacroFormat (dev);
		memory.reset (new HIDPP20::MemoryMapping (dev));
		dir_address = HIDPP::Address { HIDPP20::IOnboardProfiles::Writeable, 0, 0 };
		prof_address = HIDPP::Address { HIDPP20::IOnboardProfiles::Writeable, 1, 0 };
	}
	else {
		fprintf (stderr, "Unsupported HID++ protocol version.\n");
		return EXIT_FAILURE;
	}

	ProfileXML profxml (profile_format.get (), profdir_format.get ());

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
			fprintf (stderr, "Error parsing XML:\n%s\n", doc.ErrorStr ());
			return EXIT_FAILURE;
		}

		HIDPP::ProfileDirectory profdir;
		std::vector<HIDPP::Profile> profiles;
		std::vector<std::vector<HIDPP::Macro>> macros;

		const XMLElement *root = doc.RootElement ();
		const XMLElement *element = root->FirstChildElement ("profile");
		while (element) {
			profiles.emplace_back ();
			auto &profile = profiles.back ();

			profdir.entries.push_back ({ prof_address });
			auto &entry = profdir.entries.back ();

			macros.emplace_back ();
			auto &pmacros = macros.back ();

			profxml.read (element, profile, entry, pmacros);

			element = element->NextSiblingElement ("profile");
			++prof_address.page;
		}

		// Macro are written from the next page after profiles
		HIDPP::Address macro_address = prof_address;
		for (unsigned int i = 0; i < profiles.size (); ++i) {
			auto &entry = profdir.entries[i];
			auto &profile = profiles[i];
			for (unsigned int j = 0; j < profile.buttons.size (); ++j) {
				auto &button = profile.buttons[j];
				if (button.type () == HIDPP::Profile::Button::Type::Macro) {
					auto &macro = macros[i][j];
					auto next_address = macro.write (*macro_format, *memory, macro_address);
					button.setMacro (macro_address);
					macro_address = next_address;
				}
			}
			auto it = memory->getWritableIterator (entry.profile_address);
			profile_format->write (profile, it);
		}
		{
			auto it = memory->getWritableIterator (dir_address);
			profdir_format->write (profdir, it);
		}

		memory->sync ();
	}
	else if (op == "read") {
		XMLPrinter printer;
		XMLDocument doc;
		XMLElement *root = doc.NewElement ("profiles");

		auto profdir_it = memory->getReadOnlyIterator (dir_address);
		HIDPP::ProfileDirectory profdir = profdir_format->read (profdir_it);
		for (const auto &entry: profdir.entries) {
			auto it = memory->getReadOnlyIterator (entry.profile_address);
			HIDPP::Profile profile = profile_format->read (it);

			std::vector<HIDPP::Macro> macros;
			for (const auto &button: profile.buttons) {
				if (button.type () == HIDPP::Profile::Button::Type::Macro) {
					macros.emplace_back (*macro_format, *memory, button.macro ());
					macros.back ().simplify ();
				}
				else
					macros.emplace_back ();
			}

			XMLElement *element = doc.NewElement ("profile");
			profxml.write (profile, entry, macros, element);
			root->InsertEndChild (element);
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
