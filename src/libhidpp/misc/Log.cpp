/*
 * Copyright 2015 Clément Vuchener
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
