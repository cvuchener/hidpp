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

#include <hidpp10/WriteError.h>

#include <sstream>

using namespace HIDPP10;

WriteError::WriteError (uint8_t error_code):
	_error_code (error_code)
{
	std::stringstream ss;
	ss << "Write error: " << (unsigned int) error_code;
	_error_message = ss.str ();
}

const char *WriteError::what () const noexcept
{
	return _error_message.c_str ();
}

