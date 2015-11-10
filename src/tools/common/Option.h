#ifndef OPTION_H
#define OPTION_H

#include <functional>
#include <string>
#include <vector>

class Option
{
public:
	enum Type {
		NoArgument,
		OptionalArgument,
		RequiredArgument,
	};

	Option (char short_opt,
		const char *long_opt,
		Type type,
		const char *arg_name,
		const char *description,
		std::function<bool(const char*)> callback);
	
	std::string usageLine () const;

	static bool processOptions (int argc, char *argv[],
				    const std::vector<Option> &options,
				    int &first_arg);

	bool operator() (const char *optarg = nullptr) const;

private:
	char _short_opt;
	std::string _long_opt;
	Type _type;
	std::string _arg_name, _description;
	std::function<bool(const char*)> _callback;
};

#endif

