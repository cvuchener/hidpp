#include <misc/Log.h>

#include <cstdarg>
#include <cstdio>

Log::Level Log::_level = Log::Error;
std::ofstream Log::_null_stream;

void Log::setLevel (Log::Level level)
{
	_level = level;
}

std::ostream &Log::stream (Log::Level level)
{
	if (level <= _level)
		return std::cerr;
	else
		return _null_stream;
}

void Log::printf (Level level, const char *format, ...)
{
	if (level <= _level) {
		va_list args;
		va_start (args, format);
		vprintf (format, args);
		va_end (args);
	}
}
