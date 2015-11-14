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
