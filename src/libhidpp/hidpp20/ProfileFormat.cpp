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

#include "ProfileFormat.h"
#include <hidpp20/IOnboardProfiles.h>
#include <misc/Endian.h>
#include <misc/Log.h>

#include <codecvt>

using namespace HIDPP;
using namespace HIDPP20;

enum ButtonType: uint8_t
{
	ButtonHID = 0x80,
	ButtonSpecial = 0x90,
	ButtonMacro = 0x00,
	ButtonDisabled = 0xff,
};

enum ButtonHIDSubType: uint8_t
{
	ButtonHIDMouse = 1,
	ButtonHIDKey = 2,
	ButtonHIDConsumerControl = 3,
};

enum LEDEffect: uint8_t
{
	EffectOff = 0,
	EffectConstant = 0x01,
	EffectPulse = 0x0a,
	EffectCycle = 0x03,
};

ProfileFormat::ProfileFormat (const IOnboardProfiles::Description &desc):
	Base::ProfileFormat (ProfileSize, MaxButtonCount, MaxModeCount),
	_desc (desc)
{
	// TODO: check profile format in desc
}

const std::map<std::string, SettingDesc> &ProfileFormat::generalSettings () const
{
	return GeneralSettings;
}

const std::map<std::string, SettingDesc> &ProfileFormat::modeSettings () const
{
	return ModeSettings;
}

const EnumDesc &ProfileFormat::specialActions () const
{
	return SpecialActions;
}

ComposedSetting ProfileFormat::readLEDEffect (std::vector<uint8_t>::const_iterator begin)
{
	ComposedSetting settings;
	uint8_t type = begin[0];
	switch (type) {
	case 0: // Off
		break;
	case 0x01: // Constant
		settings.emplace ("color", Color { begin[1], begin[2], begin[3] });
		break;
	case 0x0a: // Pulse
		settings.emplace ("color", Color { begin[2], begin[3], begin[4] });
		settings.emplace ("period", static_cast<int> (readBE<uint16_t> (begin+5)));
		settings.emplace ("brightness", static_cast<int> (begin[8]));
		break;
	case 0x03: // Cycle
		settings.emplace ("period", static_cast<int> (readBE<uint16_t> (begin+7)));
		settings.emplace ("brightness", static_cast<int> (begin[9]));
		break;
	default:
		Log::error () << "Invalid LED effect type" << std::endl;
		Log::debug ().printBytes ("LED Effect", begin, begin+11);
		return settings;
	}
	settings.emplace ("type", EnumValue (ProfileFormat::LEDEffects, type));
	return settings;
}

Profile ProfileFormat::read (std::vector<uint8_t>::const_iterator begin) const
{
	for (unsigned int i = 0; i < 16; ++i)
		Log::debug ().printBytes ("profile", begin+16*i, begin+16*(i+1));
	// TODO: missing settings
	// TODO: add settings depending on desc
	Profile profile;
	profile.settings.emplace ("report_rate", static_cast<int> (begin[0]));
	profile.settings.emplace ("default_dpi", static_cast<int> (begin[1]));
	profile.settings.emplace ("switched_dpi", static_cast<int> (begin[2]));
	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		uint16_t dpi = readLE<uint16_t> (begin+3+2*i);
		if (dpi == 0x0000 || dpi == 0xFFFF)
			break;
		profile.modes.push_back ({
			{ "dpi", static_cast<int> (dpi) },
		});
	}
	profile.settings.emplace ("color", Color {
		begin[13],
		begin[14],
		begin[15]
	});
	profile.settings.emplace ("power_mode", EnumValue (PowerModes, begin[16]));
	profile.settings.emplace ("angle_snapping", begin[17] == 0x02);
	profile.settings.emplace ("unknown0", static_cast<int> (begin[18]));
	profile.settings.emplace ("unknown1", static_cast<int> (begin[19]));
	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		auto button_data = begin+32 + 4*i;
		switch (button_data[0]) {
		case ButtonHID:
			switch (button_data[1]) {
			case ButtonHIDMouse:
				profile.buttons.emplace_back (Profile::Button::MouseButtonsType (),
							      readBE<uint16_t> (button_data+2));
				break;
			case ButtonHIDKey:
				profile.buttons.emplace_back (button_data[2], button_data[3]);
				break;
			case ButtonHIDConsumerControl:
				profile.buttons.emplace_back (Profile::Button::ConsumerControlType (),
							      readBE<uint16_t> (button_data+2));
				break;
			}
			break;
		case ButtonSpecial:
			profile.buttons.emplace_back (Profile::Button::SpecialType (), button_data[1]);
			break;
		case ButtonMacro:
			profile.buttons.emplace_back (Address { button_data[2], button_data[1], button_data[3]});
			break;
		case ButtonDisabled:
			profile.buttons.emplace_back ();
			break;
		default:
			Log::error () << "Invalid button type code" << std::endl;
			profile.buttons.emplace_back ();
		}
	}
	std::u16string name;
	for (int i = 0; i < 24; ++i)
		name.push_back (readLE<char16_t> (begin+160+2*i));
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv16;
	profile.settings.emplace ("name", conv16.to_bytes (name));
	profile.settings.emplace ("logo_effect", readLEDEffect (begin+208));
	profile.settings.emplace ("side_effect", readLEDEffect (begin+219));
	return profile;
}

void ProfileFormat::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
}

const std::map<std::string, SettingDesc> ProfileFormat::GeneralSettings = {
	{ "report_rate", SettingDesc (1, 8, 4) },
	{ "default_dpi", SettingDesc (0, MaxModeCount-1, 0) },
	{ "switched_dpi", SettingDesc (0, MaxModeCount-1, 0) },
	{ "color", SettingDesc (Color { 255, 255, 255 }) }, // Unused on G502 (profile format 2)
	{ "power_mode", SettingDesc (PowerModes, -1)}, // Only for profile format 3?
	{ "angle_snapping", SettingDesc (false) },
	{ "unknown", SettingDesc (0, 65535, 65535) },
	{ "name", SettingDesc ("") },
	{ "logo_effect", SettingDesc {
		{ "type", SettingDesc (LEDEffects, EffectConstant) },
		{ "color", SettingDesc (Color { 255, 255, 255 }) },
		{ "period", SettingDesc (0, 65535, 10000) },
		{ "brightness", SettingDesc (0, 100, 100) },
	}},
	{ "side_effect", SettingDesc {
		{ "type", SettingDesc (LEDEffects, EffectConstant) },
		{ "color", SettingDesc (Color { 255, 255, 255 }) },
		{ "period", SettingDesc (0, 65535, 10000) },
		{ "brightness", SettingDesc (0, 100, 100) },
	}},
};

const std::map<std::string, SettingDesc> ProfileFormat::ModeSettings = {
	{ "dpi", SettingDesc (0, 50000, 1200) }, // TODO: Get proper values from AdjustableDPI
};

const EnumDesc ProfileFormat::SpecialActions = {
	{ "WheelLeft", 1 },
	{ "WheelRight", 2 },
	{ "ResolutionNext", 3 },
	{ "ResolutionPrev", 4 },
	{ "ResolutionDefault", 5 },
	{ "ResolutionCycle", 6 },
	{ "ResolutionSwitch", 7 },
	{ "ProfileCycle", 10 },
	{ "ModeSwitch", 11 },
	{ "BatteryLevel", 12 },
};

const EnumDesc ProfileFormat::LEDEffects = {
	{ "Off", EffectOff},
	{ "Constant", EffectConstant },
	{ "Pulse", EffectPulse },
	{ "Cycle", EffectCycle },
};

const EnumDesc ProfileFormat::PowerModes = {
	{ "NotApplicable", 0xff },
};

std::unique_ptr<Base::ProfileFormat> HIDPP20::getProfileFormat (HIDPP20::Device *device)
{
	auto desc = IOnboardProfiles (device).getDescription ();
	return std::unique_ptr<Base::ProfileFormat> (new ProfileFormat (desc));
}
