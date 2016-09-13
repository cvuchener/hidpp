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

#ifndef LOG_H
#define LOG_H

#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <mutex>

class Log: public std::ostream
{
public:
	Log (const Log &) = delete;
	Log (Log &&);
	~Log ();

	enum Level {
		Error,
		Warning,
		Info,
		Debug,
		DebugReport,
	};

	static void setLevel (Level level);
	static Level level ();

	static Log log (Level level);

	static inline Log error () { return log (Error); }
	static inline Log warning () { return log (Warning); }
	static inline Log info () { return log (Info); }
	static inline Log debug () { return log (Debug); }
	static inline Log debugReport () { return log (DebugReport); }

	void printf (const char *format, ...)
		__attribute__ ((format (printf, 2, 3)));

	template <class InputIterator>
	void printBytes (const std::string &prefix,
			 InputIterator begin, InputIterator end) {
		if (!*this)
			return;
		*this << prefix;
		std::for_each (begin, end, [this] (uint8_t byte) {
			*this << " "
			      << std::hex
			      << std::setw (2)
			      << std::setfill ('0')
			      << static_cast<unsigned int> (byte);
		});
		*this << std::endl;
	}

private:
	Log ();
	Log (const std::string &prefix);

	class LogBuf: public std::stringbuf {
	public:
		LogBuf (const std::string &prefix);
		virtual int sync ();

	private:
		std::string _prefix;
	} _buf;

	static Level _level;
	static Log _error, _warning, _info, _debug, _null;
	static std::mutex _mutex;
};

#endif
