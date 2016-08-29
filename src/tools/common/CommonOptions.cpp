#include "CommonOptions.h"

#include <misc/Log.h>

#include "common.h"

Option DeviceIndexOption (HIDPP::DeviceIndex &device_index)
{
	return Option (
		'd', "device",
		Option::RequiredArgument, "index",
		"Use wireless device index.",
		[&device_index] (const char *optarg) -> bool {
			char *endptr;
			int index = strtol (optarg, &endptr, 10);
			if (*endptr != '\0') {
				fprintf (stderr, "Invalid device index: %s\n", optarg);
				return false;
			}
			switch (index) {
			case 0:
				device_index = HIDPP::CordedDevice;
				break;
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
				return false;
			}
			return true;
		}
	);
}

Option VerboseOption ()
{
	return Option (
		'v', "verbose",
		Option::OptionalArgument, "level",
		"Change verbosity level (level may be error|warning|info|debug|report).",
		[] (const char *optarg) -> bool {
			if (optarg) {
				std::string level = optarg;
				if (level == "error")
					Log::setLevel (Log::Error);
				else if (level == "warning")
					Log::setLevel (Log::Warning);
				else if (level == "info")
					Log::setLevel (Log::Info);
				else if (level == "debug")
					Log::setLevel (Log::Debug);
				else if (level == "report")
					Log::setLevel (Log::DebugReport);
				else {
					fprintf (stderr, "Invalid verbose level: %s\n", optarg);
					return false;
				}
			}
			else {
				Log::setLevel (Log::Warning);
			}
			return true;
		}
	);
}

Option HelpOption (const char *program, const char *args,
		   const std::vector<Option> *options)
{
	return Option (
		'h', "help",
		Option::NoArgument, "",
		"Print this message.",
		[program, args, options] (const char *optarg) -> bool {
			fprintf (stderr, "%s", getUsage (program, args, options).c_str ());
			exit (EXIT_SUCCESS);
		}
	);
}
