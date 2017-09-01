/*
 * Copyright 2017 Clément Vuchener
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

#ifndef LIBHIDPP_HID_WINDOWS_ERROR_CATEGORY_H
#define LIBHIDPP_HID_WINDOWS_ERROR_CATEGORY_H

#include <system_error>

namespace windows
{

class error_category: public std::error_category
{
public:
	virtual const char *name () const noexcept;
	virtual std::string message (int condition) const;
};

}

const std::error_category &windows_category () noexcept;

#endif
