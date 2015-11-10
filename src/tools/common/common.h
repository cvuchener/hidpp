#ifndef COMMON_H
#define COMMON_H

#include "Option.h"

std::string getUsage (const char *program, const char *args,
		      const std::vector<Option> *options);

#endif
