#ifndef COMMON_OPTIONS_H
#define COMMON_OPTIONS_H

#include "Option.h"
#include <hidpp/defs.h>

Option DeviceIndexOption (HIDPP::DeviceIndex &device_index);
Option VerboseOption ();
Option HelpOption (const char *program, const char *args, const std::vector<Option> *options);

#endif
