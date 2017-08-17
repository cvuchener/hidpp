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

#ifndef LIBHIDPP_HIDPP20_PROFILE_FORMAT_H
#define LIBHIDPP_HIDPP20_PROFILE_FORMAT_H

#include <hidpp/AbstractProfileFormat.h>
#include <hidpp20/IOnboardProfiles.h>

#include <memory>

namespace HIDPP20
{

class ProfileFormat: public HIDPP::AbstractProfileFormat
{
public:
	ProfileFormat (const IOnboardProfiles::Description &desc);

	virtual const std::map<std::string, HIDPP::SettingDesc> &generalSettings () const;
	virtual const std::map<std::string, HIDPP::SettingDesc> &modeSettings () const;
	virtual const HIDPP::EnumDesc &specialActions () const;

	virtual HIDPP::Profile read (std::vector<uint8_t>::const_iterator begin) const;
	virtual void write (const HIDPP::Profile &profile, std::vector<uint8_t>::iterator begin) const;

private:
	IOnboardProfiles::Description _desc;
	std::map<std::string, HIDPP::SettingDesc> _general_settings;
	bool _has_g_shift;
	bool _has_dpi_shift;
	bool _has_rgb_effects;
	bool _has_power_modes;

	static HIDPP::ComposedSetting readRGBEffect (std::vector<uint8_t>::const_iterator begin);
	static void writeRGBEffect (std::vector<uint8_t>::iterator begin, const HIDPP::ComposedSetting &settings);

	static const std::map<uint8_t, size_t> ProfileLength;
	static constexpr unsigned int MaxButtonCount = 16;
	static constexpr unsigned int MaxModeCount = 5;

	static const std::map<std::string, HIDPP::SettingDesc> RGBEffectSettings;
	static const std::map<std::string, HIDPP::SettingDesc> CommonGeneralSettings;
	static const std::map<std::string, HIDPP::SettingDesc> ModeSettings;
	static const HIDPP::EnumDesc SpecialActions;
	static const HIDPP::EnumDesc RGBEffects;
	static const HIDPP::EnumDesc PowerModes;
};

std::unique_ptr<HIDPP::AbstractProfileFormat> getProfileFormat (Device *device);

}

#endif
