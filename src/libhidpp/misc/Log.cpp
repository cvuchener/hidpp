/*
 * Copyright 2016 Clément Vuchener
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

#include "Log.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cstring>

Log::Level Log::_level = Log::Error;

std::mutex Log::_mutex;

Log::Log ():
	_buf ("null")
{
}

Log::Log (const std::string &prefix):
	std::ostream (&_buf),
	_buf (prefix)
{
}

Log::Log (Log &&log):
	std::ostream (std::move (log)),
	_buf (std::move (log._buf))
{
}

Log::~Log ()
{
}

void Log::setLevel (Log::Level level)
{
	_level = level;
}

Log::Level Log::level ()
{
	return _level;
}

Log Log::log (Log::Level level)
{
	if (level <= _level) {
		switch (level) {
		case Error:
			return Log ("error");
		case Warning:
			return Log ("warning");
		case Info:
			return Log ("info");
		case Debug:
			return Log ("debug");
		case DebugReport:
			return Log ("debug report");
		}
	}
	return Log ();
}

void Log::printf (const char *format, ...)
{
	char *str, *current, *end;
	int len;
	if (!*this)
		return;
	va_list args, copy;
	va_start (args, format);
	va_copy (copy, args);
	len = vsnprintf (nullptr, 0, format, copy);
	str = new char[len+1];
	vsnprintf (str, len+1, format, args);
	current = str;
	while (*current != '\0' && (end = strchr (current, '\n'))) {
		*this << std::string (current, end) << std::endl;
		current = end+1;
	}
	if (*current != '\0') {
		*this << std::string (current);
	}
	delete[] str;
	va_end (args);
}

Log::LogBuf::LogBuf (const std::string &prefix):
	_prefix (prefix)
{
}

int Log::LogBuf::sync ()
{
	std::unique_lock<std::mutex> lock (_mutex);
	std::cerr << "[" << _prefix << "] " << str ();
	str (std::string ());
	return 0;
}
