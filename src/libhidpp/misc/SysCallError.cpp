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

#include <misc/SysCallError.h>

#include <sstream>
#include <cstring>

SysCallError::SysCallError (const char *what, int err, const char *where)
{
	std::stringstream ss;
	ss << what << ": " << strerror (err);
	if (where)
		ss << " (in " << where << ")";
	_error_string = ss.str ();
}

const char *SysCallError::what () const noexcept
{
	return _error_string.c_str ();
}
