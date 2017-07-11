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

#ifndef LIBHIDPP_HIDPP_ABSTRACT_PROFILE_DIRECTORY_FORMAT_H
#define LIBHIDPP_HIDPP_ABSTRACT_PROFILE_DIRECTORY_FORMAT_H

#include <hidpp/ProfileDirectory.h>

namespace HIDPP
{

/**
 * Abstract class for profile directory formats.
 */
class AbstractProfileDirectoryFormat
{
public:
	virtual ~AbstractProfileDirectoryFormat () = default;

	/**
	 * Access the complete list of settings and their names used
	 * by this format.
	 */
	virtual const std::map<std::string, SettingDesc> &settings () const = 0;

	/**
	 * Read the profile directory beginning at \p begin.
	 *
	 * \returns The parsed profile directory.
	 */
	virtual ProfileDirectory read (std::vector<uint8_t>::const_iterator begin) const = 0;

	/**
	 * Write the profile directory \p profile_directory at \p begin.
	 */
	virtual void write (const ProfileDirectory &profile_directory, std::vector<uint8_t>::iterator begin) const = 0;
};

}

#endif
