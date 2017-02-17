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

#ifndef PROFILE_XML_H
#define PROFILE_XML_H

#include <hidpp/Profile.h>
#include <hidpp/ProfileDirectory.h>
#include <hidpp/Macro.h>
#include <hidpp/AbstractProfileFormat.h>
#include <hidpp/AbstractProfileDirectoryFormat.h>
#include <tinyxml2.h>
#include <functional>

class ProfileXML
{
public:
	ProfileXML (const HIDPP::AbstractProfileFormat *profile_format,
		    const HIDPP::AbstractProfileDirectoryFormat *profdir_format);

	// TODO: add macro vector
	void write (const HIDPP::Profile &profile,
		    const HIDPP::ProfileDirectory::Entry &entry,
		    const std::vector<HIDPP::Macro> &macros,
		    tinyxml2::XMLNode *node);
	void read (const tinyxml2::XMLNode *node,
		   HIDPP::Profile &profile,
		   HIDPP::ProfileDirectory::Entry &entry,
		   std::vector<HIDPP::Macro> &macros);

private:
	const std::map<std::string, HIDPP::SettingDesc> &_profile_settings;
	const std::map<std::string, HIDPP::SettingDesc> &_mode_settings;
	const std::map<std::string, HIDPP::SettingDesc> &_entry_settings;
	const HIDPP::EnumDesc &_special_actions;
};

#endif
