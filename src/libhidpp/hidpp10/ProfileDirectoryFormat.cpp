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
using namespace HIDPP10;

ProfileDirectoryFormat::ProfileDirectoryFormat (unsigned int led_count):
	_led_count (led_count)
{
	if (led_count > 0)
		_settings.emplace ("leds", SettingDesc (LEDVector (led_count, false)));
}

const std::map<std::string, HIDPP::SettingDesc> &ProfileDirectoryFormat::settings () const
{
	return _settings;
}

ProfileDirectory ProfileDirectoryFormat::read (std::vector<uint8_t>::const_iterator begin) const
{
	ProfileDirectory dir;
	while (true) {
		uint8_t page = begin[0];
		if (page == 0xFF)
			break;
		uint8_t offset = begin[1];
		dir.entries.push_back ({ Address {0, page, offset} });
		if (_led_count > 0) {
			uint8_t bits = begin[2];
			LEDVector leds;
			for (unsigned int i = 0; i < _led_count; ++i)
				leds.push_back (bits & 1<<i);
			dir.entries.back ().settings.emplace ("leds", leds);
		}
		begin += 3;
	}
	return dir;
}

void ProfileDirectoryFormat::write (const ProfileDirectory &dir, std::vector<uint8_t>::iterator begin) const
{
	for (const auto &entry: dir.entries) {
		SettingLookup settings (entry.settings, _settings);
		begin[0] = entry.profile_address.page;
		begin[1] = entry.profile_address.offset;
		LEDVector leds = settings.get<LEDVector> ("leds");
		begin[2] = 0;
		for (unsigned int i = 0; i < _led_count && i < leds.size (); ++i)
			if (leds[i])
				begin[2] |= 1<<i;
		begin += 3;
	}
	begin[0] = 0xFF;
}

std::unique_ptr<AbstractProfileDirectoryFormat> HIDPP10::getProfileDirectoryFormat (Device *device)
{
	return std::unique_ptr<AbstractProfileDirectoryFormat> (new ProfileDirectoryFormat (4));
}
