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

#include "ProfileDirectoryFormat.h"

using namespace HIDPP;
using namespace HIDPP20;

const std::map<std::string, HIDPP::SettingDesc> &ProfileDirectoryFormat::settings () const
{
	return Settings;
}

ProfileDirectory ProfileDirectoryFormat::read (std::vector<uint8_t>::const_iterator begin) const
{
	ProfileDirectory dir;
	while (true) {
		uint8_t mem_type = begin[0];
		if (mem_type == 0xFF)
			break;
		uint8_t page = begin[1];
		dir.entries.emplace_back ();
		dir.entries.back ().profile_address = {mem_type, page, 0},
		dir.entries.back ().settings.emplace ("enabled", static_cast<bool> (begin[2]));
		// TODO: byte 3 role is unknown
		begin += 4;
	}
	return dir;
}

void ProfileDirectoryFormat::write (const ProfileDirectory &profile_directory, std::vector<uint8_t>::iterator begin) const
{
}

const std::map<std::string, SettingDesc> ProfileDirectoryFormat::Settings = {
	{ "enabled", SettingDesc (true) },
};

std::unique_ptr<Base::ProfileDirectoryFormat> HIDPP20::getProfileDirectoryFormat (HIDPP20::Device *device)
{
	return std::unique_ptr<Base::ProfileDirectoryFormat> (new ProfileDirectoryFormat ());
}
