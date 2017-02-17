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

#include <hidpp/SettingLookup.h>

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
		auto &entry = dir.entries.back ();
		entry.profile_address = {mem_type, page, 0},
		entry.settings.emplace ("enabled", static_cast<bool> (begin[2]));
		entry.settings.emplace ("dir_unknown", static_cast<int> (begin[3]));
		begin += 4;
	}
	return dir;
}

void ProfileDirectoryFormat::write (const ProfileDirectory &profile_directory, std::vector<uint8_t>::iterator begin) const
{
	for (const auto &entry: profile_directory.entries) {
		SettingLookup settings (entry.settings, Settings);
		begin[0] = entry.profile_address.mem_type;
		begin[1] = entry.profile_address.page;
		begin[2] = (settings.get<bool> ("enabled") ? 0x01 : 0x00);
		begin[3] = settings.get<int> ("dir_unknown");
		begin += 4;
	}
	begin[0] = begin[1] = 0xff;
}

const std::map<std::string, SettingDesc> ProfileDirectoryFormat::Settings = {
	{ "enabled", SettingDesc (true) },
	{ "dir_unknown", SettingDesc (0, 255, 0) },
};

std::unique_ptr<AbstractProfileDirectoryFormat> HIDPP20::getProfileDirectoryFormat (HIDPP20::Device *device)
{
	return std::unique_ptr<AbstractProfileDirectoryFormat> (new ProfileDirectoryFormat ());
}
