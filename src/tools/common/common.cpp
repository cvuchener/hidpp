#include "common.h"

#include <sstream>

std::string getUsage (const char *program, const char *args,
		      const std::vector<Option> *options)
{
	std::stringstream ss;
	ss << "Usage: " << program << " [options] " << args << std::endl;
	ss << "Options are:" << std::endl;
	for (const Option &opt: *options) {
		ss <<opt.usageLine ();
	}
	return ss.str ();
}
