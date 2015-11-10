#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

class Log
{
public:
	enum Level {
		Error,
		Warning,
		Info,
		Debug,
		DebugReport,
	};

	Log () = delete;

	static void setLevel (Level level);
	static Level level ();

	static std::ostream &stream (Level level);

	static inline std::ostream &error () { return stream (Error); }
	static inline std::ostream &warning () { return stream (Warning); }
	static inline std::ostream &info () { return stream (Info); }
	static inline std::ostream &debug () { return stream (Debug); }

	static void printf (Level level, const char *format, ...)
		__attribute__ ((format (printf, 2, 3)));
	
	template<class InputIterator>
	static
	void printBytes (Level level, const std::string &prefix,
			 InputIterator begin, InputIterator end) {
		std::ostream &os = stream (level);
		if (!os)
			return;
		os << prefix;
		std::for_each (begin, end, [&os] (uint8_t byte) {
			os << " "
			   << std::hex
			   << std::setw (2)
			   << std::setfill ('0')
			   << static_cast<unsigned int> (byte);
		});
		os << std::endl;
	}

private:
	static const char *levelTag (Level level);

	static Level _level;
	static std::ofstream _null_stream;
};

#endif
