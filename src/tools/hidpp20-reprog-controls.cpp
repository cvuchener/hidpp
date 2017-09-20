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

#include <hidpp/SimpleDispatcher.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/IReprogControlsV4.h>
#include <cstdio>
#include <memory>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

using namespace HIDPP20;

template<typename T>
static void printFlags (T flags, std::vector<std::tuple<T, const char*>> list)
{
	bool first = true;
	for (auto t: list) {
		uint8_t flag;
		const char *str;
		std::tie (flag, str) = t;
		if (flags & flag) {
			if (first)
				first = false;
			else
				printf (", ");
			printf ("%s", str);
		}
	}
	if (first)
		printf ("-");
}

enum Flag {
	NoChange,
	On,
	Off
};
bool setFlag (Flag *flag, const char *optarg)
{
	if (optarg == nullptr)
		*flag = On;
	else {
		int v = strtol (optarg, nullptr, 10);
		*flag = (v == 0 ? Off : On);
	}
	return true;
}

int main (int argc, char *argv[])
{
	static const char *args = "device_path info|get|set [control_id] [remap_id]";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;

	Flag divert = NoChange, persist = NoChange, raw_xy = NoChange;

	std::vector<Option> options = {
		Option ('t', "divert", Option::OptionalArgument,
			"value", "Temporarily divert the button event (0 for disabling)",
			std::bind (setFlag, &divert, std::placeholders::_1)),
		Option ('p', "persist", Option::OptionalArgument,
			"value", "Persistently divert the button event (0 for disabling)",
			std::bind (setFlag, &persist, std::placeholders::_1)),
		Option ('r', "rawxy", Option::OptionalArgument,
			"value", "Divert the raw XY event (0 for disabling)",
			std::bind (setFlag, &raw_xy, std::placeholders::_1)),
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

	const char *path = argv[first_arg];
	std::string op = argv[first_arg+1];
	first_arg += 2;

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (path);
	}
	catch (std::exception &e) {
		fprintf (stderr, "Failed to open device: %s.\n", e.what ());
		return EXIT_FAILURE;
	}
	Device dev (dispatcher.get (), device_index);
	try {
		IReprogControlsV4 irc (&dev);
		if (op == "info") {
			unsigned int count = irc.getControlCount ();
			printf ("Control\tTask\tInfo\tFn\tCapabilities\tGroup\tRemap to groups\n");
			for (unsigned int i = 0; i < count; ++i) {
				auto info = irc.getControlInfo (i);
				printf ("0x%04hx\t0x%04hx", info.control_id, info.task_id);
				if (info.flags & IReprogControlsV4::MouseButton)
					printf ("\tmouse");
				else if (info.flags & IReprogControlsV4::FKey)
					printf ("\tF%d", info.pos);
				else if (info.flags & IReprogControlsV4::HotKey)
					printf ("\thotkey");
				else
					printf ("\t-");

				if (info.flags & IReprogControlsV4::FnToggle)
					printf ("\ttoggle");
				else
					printf ("\t-");

				printf ("\t");
				printFlags (info.flags | ((uint16_t) info.additional_flags << 8), {
					std::make_tuple (IReprogControlsV4::ReprogHint, "reprog"),
					std::make_tuple (IReprogControlsV4::TemporaryDivertable, "divert"),
					std::make_tuple (IReprogControlsV4::PersistentDivertable, "persist"),
					std::make_tuple (IReprogControlsV4::Virtual, "virtual"),
					std::make_tuple (IReprogControlsV4::RawXY<<8, "rawxy"),
				});
				printf ("\t");
				if (info.group != 0)
					printf ("%d", info.group);
				else
					printf ("-");
				printf ("\t");
				if (info.group_mask != 0) {
					bool first = true;
					for (unsigned int i = 0; i < 8; ++i)
						if (info.group_mask & 1<<i) {
							if (first)
								first = false;
							else
								printf ("+");
							printf ("%d", i+1);
						}
				}
				else
					printf ("-");
				printf ("\n");
			}
		}
		else if (op == "get") {
			int cid;
			char *endptr;
			if (argc-first_arg < 1) {
				fprintf (stderr, "Missing control id.\n");
				return EXIT_FAILURE;
			}
			cid = strtol (argv[first_arg], &endptr, 0);
			if (*endptr != '\0' || cid < 0 || cid > 65535) {
				fprintf (stderr, "Invalid control ID value.\n");
				return EXIT_FAILURE;
			}
			uint8_t flags;
			uint16_t remap = irc.getControlReporting (cid, flags);
			printf ("0x%04hx (flags: ", remap);
			printFlags (flags, {
				std::make_tuple (IReprogControlsV4::TemporaryDiverted, "divert"),
				std::make_tuple (IReprogControlsV4::PersistentDiverted, "persist"),
				std::make_tuple (IReprogControlsV4::RawXYDiverted, "rawxy"),
			});
			printf (")\n");

		}
		else if (op == "set") {
			int cid, remap;
			char *endptr;
			if (argc-first_arg < 1) {
				fprintf (stderr, "Missing control id.\n");
				return EXIT_FAILURE;
			}
			cid = strtol (argv[first_arg], &endptr, 0);
			if (*endptr != '\0' || cid < 0 || cid > 65535) {
				fprintf (stderr, "Invalid control ID value.\n");
				return EXIT_FAILURE;
			}
			if (argc-first_arg < 2) {
				remap = 0;
			}
			else {
				remap = strtol (argv[first_arg+1], &endptr, 0);
				if (*endptr != '\0' || remap < 0 || remap > 65535) {
					fprintf (stderr, "Invalid remap control ID value.\n");
					return EXIT_FAILURE;
				}
			}
			uint8_t flags = 0;
			if (divert != NoChange)
				flags |= IReprogControlsV4::ChangeTemporaryDivert;
			if (divert == On)
				flags |= IReprogControlsV4::TemporaryDiverted;
			if (persist != NoChange)
				flags |= IReprogControlsV4::ChangePersistentDivert;
			if (persist == On)
				flags |= IReprogControlsV4::PersistentDiverted;
			if (raw_xy != NoChange)
				flags |= IReprogControlsV4::ChangeRawXYDivert;
			if (raw_xy == On)
				flags |= IReprogControlsV4::RawXYDiverted;
			irc.setControlReporting (cid, flags, remap);
		}
		else {
			fprintf (stderr, "Invalid operation: %s\n", op.c_str ());
			return EXIT_FAILURE;
		}
	}
	catch (Error e) {
		fprintf (stderr, "Error code %d: %s\n", e.errorCode (), e.what ());
		return e.errorCode ();
	}

	return EXIT_SUCCESS;
}

