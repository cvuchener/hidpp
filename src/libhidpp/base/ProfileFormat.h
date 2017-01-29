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

#ifndef HIDPP_PROFILE_FORMAT_H
#define HIDPP_PROFILE_FORMAT_H

#include <hidpp/Profile.h>

namespace HIDPP
{

namespace Base
{

class ProfileFormat
{
public:
	ProfileFormat (size_t size, unsigned int max_button_count, unsigned int max_mode_count);
	virtual ~ProfileFormat ();

	size_t size () const;
	unsigned int maxButtonCount () const;
	unsigned int maxModeCount () const;

	virtual const std::map<std::string, SettingDesc> &generalSettings () const = 0;
	virtual const std::map<std::string, SettingDesc> &modeSettings () const = 0;
	virtual const EnumDesc &specialActions () const = 0;

	virtual Profile read (std::vector<uint8_t>::const_iterator begin) const = 0;
	virtual void write (const Profile &profile, std::vector<uint8_t>::iterator begin) const = 0;

private:
	size_t _size;
	unsigned int _max_button_count;
	unsigned int _max_mode_count;
};

}
}

#endif
