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

#ifndef LIBHIDPP_HIDPP_SETTING_LOOKUP_H
#define LIBHIDPP_HIDPP_SETTING_LOOKUP_H

#include <hidpp/Setting.h>
#include <misc/Log.h>

namespace HIDPP
{

class SettingLookup
{
public:
	SettingLookup (const std::map<std::string, Setting> &values, const std::map<std::string, SettingDesc> &descs);

	template<typename T>
	T get (const std::string &name)
	{
		const SettingDesc &desc = _descs.at (name);
		auto it = _values.find (name);
		if (it == _values.end ())
			return desc.defaultValue ().get<T> ();
		if (!desc.check (it->second)) {
			Log::error () << "Invalid value in setting \"" << name
				      << "\", using default value instead."
				      << std::endl;
			return desc.defaultValue ().get<T> ();
		}
		return it->second.get<T> ();
	}

	template<typename T>
	T get (const std::string &name, T default_value)
	{
		const SettingDesc &desc = _descs.at (name);
		auto it = _values.find (name);
		if (it == _values.end ())
			return default_value;
		if (!desc.check (it->second)) {
			Log::error () << "Invalid value in setting \"" << name
				      << "\", using default value instead."
				      << std::endl;
			return default_value;
		}
		return it->second.get<T> ();
	}

private:
	const std::map<std::string, Setting> &_values;
	const std::map<std::string, SettingDesc> &_descs;
};

}

#endif
