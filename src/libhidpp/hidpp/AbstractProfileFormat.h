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

#ifndef LIBHIDPP_HIDPP_ABSTRACT_PROFILE_FORMAT_H
#define LIBHIDPP_HIDPP_ABSTRACT_PROFILE_FORMAT_H

#include <hidpp/Profile.h>

namespace HIDPP
{

/**
 * Abstract class for profile formats.
 */
class AbstractProfileFormat
{
public:
	/**
	 * Constructor
	 *
	 * \param size	Size of the profile when stored in memory.
	 * \param max_button_count	The maximum button count that this profile format can store.
	 * \param max_mode_count	The maximum mode count that this profile format can store.
	 */
	AbstractProfileFormat (size_t size, unsigned int max_button_count, unsigned int max_mode_count);
	virtual ~AbstractProfileFormat ();

	/**
	 * Size of the profile when stored in memory
	 */
	size_t size () const;
	/**
	 * The maximum button count that this profile format can store.
	 */
	unsigned int maxButtonCount () const;
	/**
	 * The maximum mode count that this profile format can store.
	 */
	unsigned int maxModeCount () const;

	/**
	 * The list of settings and their names used by the whole profile.
	 */
	virtual const std::map<std::string, SettingDesc> &generalSettings () const = 0;
	/**
	 * The list of settings and their names used by each mode.
	 */
	virtual const std::map<std::string, SettingDesc> &modeSettings () const = 0;
	/**
	 * The list of special actions that can be mapped to buttons
	 */
	virtual const EnumDesc &specialActions () const = 0;

	/**
	 * Read the profile beginning at \p begin.
	 *
	 * \returns the parsed profile.
	 */
	virtual Profile read (std::vector<uint8_t>::const_iterator begin) const = 0;
	/**
	 * Write the profile \p profile at \p begin.
	 */
	virtual void write (const Profile &profile, std::vector<uint8_t>::iterator begin) const = 0;

private:
	size_t _size;
	unsigned int _max_button_count;
	unsigned int _max_mode_count;
};

}

#endif
