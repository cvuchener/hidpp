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
	Log::init ();
	return Option (
		'v', "verbose",
		Option::OptionalArgument, "list",
		"Enable verbose mode or change verbosity settings from a comma-separated list of category setting: [-]category[:subcategory].",
		[] (const char *optarg) -> bool {
			if (!optarg)
				optarg = "";
			Log::init (optarg);
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
