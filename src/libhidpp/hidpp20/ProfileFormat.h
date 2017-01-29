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

#ifndef HIDPP20_PROFILE_FORMAT_H
#define HIDPP20_PROFILE_FORMAT_H

#include <base/ProfileFormat.h>
#include <hidpp20/IOnboardProfiles.h>

#include <memory>

namespace HIDPP20
{

class ProfileFormat: public HIDPP::Base::ProfileFormat
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

	static HIDPP::ComposedSetting readLEDEffect (std::vector<uint8_t>::const_iterator begin);

	static constexpr size_t ProfileSize = 230;
	static constexpr unsigned int MaxButtonCount = 32;
	static constexpr unsigned int MaxModeCount = 5;

	static const std::map<std::string, HIDPP::SettingDesc> GeneralSettings;
	static const std::map<std::string, HIDPP::SettingDesc> ModeSettings;
	static const HIDPP::EnumDesc SpecialActions;
	static const HIDPP::EnumDesc LEDEffects;
};

std::unique_ptr<HIDPP::Base::ProfileFormat> getProfileFormat (Device *device);

}

#endif
