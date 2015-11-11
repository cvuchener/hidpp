#include <misc/Log.h>

#include <cstdarg>
#include <cstdio>

Log::Level Log::_level = Log::Error;
std::ofstream Log::_null_stream;

void Log::setLevel (Log::Level level)
{
	_level = level;
}

Log::Level Log::level ()
{
	return _level;
}

std::ostream &Log::stream (Log::Level level)
{
	if (level <= _level) {
		std::cerr << "[" << levelTag (level) << "] ";
		return std::cerr;
	}
	else
		return _null_stream;
}

void Log::printf (Level level, const char *format, ...)
{
	if (level <= _level) {
		std::string new_format = std::string ("[") + levelTag (level) + "] " + format;
		va_list args;
		va_start (args, format);
		vprintf (new_format.c_str (), args);
		va_end (args);
	}
}

const char *Log::levelTag (Level level)
{
	switch (level) {
	case Error:
		return "Error";
	case Warning:
		return "Warning";
	case Info:
		return "Info";
	case Debug:
		return "Debug";
	case DebugReport:
		return "Debug Report";
	}
	return "Invalid";
}
