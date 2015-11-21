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

#ifndef SYS_CALL_ERROR
#define SYS_CALL_ERROR

#include <stdexcept>
#include <string>

class SysCallError: public std::exception
{
public:
	SysCallError (const char *what, int err, const char *where = nullptr);

	virtual const char *what () const noexcept;
	int error () const noexcept;

private:
	int _error;
	std::string _error_string;
};

#endif
