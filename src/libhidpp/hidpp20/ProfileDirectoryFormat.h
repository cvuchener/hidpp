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

#ifndef LIBHIDPP_HIDPP20_PROFILE_DIRECTORY_FORMAT_H
#define LIBHIDPP_HIDPP20_PROFILE_DIRECTORY_FORMAT_H

#include <hidpp/AbstractProfileDirectoryFormat.h>
#include <memory>

namespace HIDPP20
{

class ProfileDirectoryFormat: public HIDPP::AbstractProfileDirectoryFormat
{
public:
	virtual const std::map<std::string, HIDPP::SettingDesc> &settings () const;

	virtual HIDPP::ProfileDirectory read (std::vector<uint8_t>::const_iterator begin) const;
	virtual void write (const HIDPP::ProfileDirectory &profile_directory, std::vector<uint8_t>::iterator begin) const;

private:
	static const std::map<std::string, HIDPP::SettingDesc> Settings;
};

class Device;

std::unique_ptr<HIDPP::AbstractProfileDirectoryFormat> getProfileDirectoryFormat (Device *device);

}

#endif
