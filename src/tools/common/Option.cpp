#include "Option.h"

#include <sstream>
#include <map>

extern "C" {
#include <getopt.h>
}


Option::Option (char short_opt,
		const char *long_opt,
		Type type,
		const char *arg_name,
		const char *description,
		std::function<bool(const char *)> callback):
	_short_opt (short_opt), _long_opt (long_opt),
	_type (type),
	_arg_name (arg_name), _description (description),
	_callback (callback)
{
}
	
std::string Option::usageLine () const
{
	std::stringstream ss;
	ss << "	-" << _short_opt
	   << ",--" << _long_opt;
	if (_type == OptionalArgument)
		ss << " [" << _arg_name << "]";
	else if (_type == RequiredArgument)
		ss << " " << _arg_name;
	ss << "	" << _description << std::endl;
	return ss.str ();
}

bool Option::processOptions (int argc, char *argv[],
			     const std::vector<Option> &options,
			     int &first_arg)
{
	std::stringstream shortopts;
	std::vector<struct option> longopts;
	std::map<char, unsigned int> shortopt_index;
	for (unsigned int i = 0; i < options.size (); ++i) {
		const Option &opt = options[i];

		shortopts << opt._short_opt;
		if (opt._type == OptionalArgument)
			shortopts << "::";
		else if (opt._type == RequiredArgument)
			shortopts << ":";
		shortopt_index[opt._short_opt] = i;
		
		struct option longopt;
		longopt.name = opt._long_opt.c_str ();
		switch (opt._type) {
		case NoArgument:
			longopt.has_arg = no_argument;
			break;
		case OptionalArgument:
			longopt.has_arg = optional_argument;
			break;
		case RequiredArgument:
			longopt.has_arg = required_argument;
			break;
		}
		longopt.flag = nullptr;
		longopt.val = 256 + i;
		longopts.push_back (longopt);
	}
	longopts.emplace_back (option{});

	int opt;
	while (-1 != (opt = getopt_long (argc, argv,
					 shortopts.str ().c_str (),
					 longopts.data (), nullptr))) {
		unsigned int i;
		if (opt == '?')
			return false;

		if (opt < 256)
			i = shortopt_index[opt];
		else
			i = opt - 256;

		if (!options[i]._callback (optarg))
			return false;
	}
	first_arg = optind;
	return true;
}

bool Option::operator() (const char *optarg) const
{
	return _callback (optarg);
}
