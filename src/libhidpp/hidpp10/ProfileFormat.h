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

#ifndef HIDPP10_PROFILE_FORMAT_H
#define HIDPP10_PROFILE_FORMAT_H

#include <base/ProfileFormat.h>
#include <hidpp10/Sensor.h>

#include <memory>

namespace HIDPP10
{

class Device;

enum ProfileType {
	NoProfile,
	G9ProfileType,
	G500ProfileType,
	G700ProfileType,
};

class G500ProfileFormat: public HIDPP::Base::ProfileFormat
{
public:
	G500ProfileFormat (const Sensor &sensor);

	virtual const std::map<std::string, HIDPP::SettingDesc> &generalSettings () const;
	virtual const std::map<std::string, HIDPP::SettingDesc> &modeSettings () const;
	virtual const HIDPP::EnumDesc &specialActions () const;

	virtual HIDPP::Profile read (std::vector<uint8_t>::const_iterator begin) const;
	virtual void write (const HIDPP::Profile &profile, std::vector<uint8_t>::iterator begin) const;

private:
	const Sensor &_sensor;
	HIDPP::SettingDesc _dpi_setting;
	std::map<std::string, HIDPP::SettingDesc> _mode_settings;

	static constexpr size_t ProfileSize = 78;
	static constexpr unsigned int MaxButtonCount = 13;
	static constexpr unsigned int MaxModeCount = 5;
	static constexpr unsigned int LEDCount = 4;

	static const std::map<std::string, HIDPP::SettingDesc> GeneralSettings;
	static const HIDPP::EnumDesc SpecialActions;
};

std::unique_ptr<HIDPP::Base::ProfileFormat> getProfileFormat (Device *device);

}

#endif
