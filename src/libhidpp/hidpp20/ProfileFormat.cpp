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
#include <hidpp/SettingLookup.h>
#include <misc/Endian.h>
#include <misc/Log.h>

#include <codecvt>
#include <cassert>

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

enum RGBEffect: uint8_t
{
	RGBEffectOff = 0,
	RGBEffectConstant = 0x01,
	RGBEffectPulse = 0x0a,
	RGBEffectCycle = 0x03,
};

ProfileFormat::ProfileFormat (const IOnboardProfiles::Description &desc):
	Base::ProfileFormat (ProfileLength.at (_desc.profile_format),
			     _desc.button_count, MaxModeCount),
	_desc (desc),
	_general_settings (CommonGeneralSettings),
	_has_g_shift ((_desc.mechanical_layout & 0x03) == 2),
	_has_dpi_shift ((_desc.mechanical_layout & 0x0c) >> 2 == 2),
	_has_rgb_effects (
		_desc.profile_format == IOnboardProfiles::ProfileFormat::G303 ||
		_desc.profile_format == IOnboardProfiles::ProfileFormat::G900),
	_has_power_modes ((_desc.various_info & 0x07) != 1) // Not corded only
{
	assert (_desc.button_count <= MaxButtonCount);
	// TODO: check profile format in desc
	if (_has_rgb_effects) {
		_general_settings.emplace ("logo_effect", RGBEffectSettings);
		_general_settings.emplace ("side_effect", RGBEffectSettings);
	}
	if (_has_dpi_shift) {
		_general_settings.emplace ("switched_dpi", SettingDesc (0, MaxModeCount-1, 0));
	}
	if (_has_power_modes) {
		_general_settings.emplace ("power_mode", SettingDesc (PowerModes, -1));
	}
}

const std::map<std::string, SettingDesc> &ProfileFormat::generalSettings () const
{
	return _general_settings;
}

const std::map<std::string, SettingDesc> &ProfileFormat::modeSettings () const
{
	return ModeSettings;
}

const EnumDesc &ProfileFormat::specialActions () const
{
	return SpecialActions;
}

ComposedSetting ProfileFormat::readRGBEffect (std::vector<uint8_t>::const_iterator begin)
{
	ComposedSetting settings;
	uint8_t type = begin[0];
	switch (type) {
	case RGBEffectOff:
		break;
	case RGBEffectConstant:
		settings.emplace ("color", Color { begin[1], begin[2], begin[3] });
		break;
	case RGBEffectPulse:
		settings.emplace ("color", Color { begin[2], begin[3], begin[4] });
		settings.emplace ("period", static_cast<int> (readBE<uint16_t> (begin+5)));
		settings.emplace ("brightness", static_cast<int> (begin[7]));
		break;
	case RGBEffectCycle:
		settings.emplace ("period", static_cast<int> (readBE<uint16_t> (begin+6)));
		settings.emplace ("brightness", static_cast<int> (begin[8]));
		break;
	default:
		Log::error () << "Invalid LED effect type" << std::endl;
		Log::debug ().printBytes ("LED Effect", begin, begin+11);
		return settings;
	}
	settings.emplace ("type", EnumValue (ProfileFormat::RGBEffects, type));
	return settings;
}

void ProfileFormat::writeRGBEffect (std::vector<uint8_t>::iterator begin, const ComposedSetting &settings)
{
	std::fill (begin, begin+11, 0);
	SettingLookup effect (settings, RGBEffectSettings);
	int type = effect.get<EnumValue> ("type").get ();
	begin[0] = type;
	Color color;
	switch (type) {
	case RGBEffectOff:
		break;
	case RGBEffectConstant:
		color = effect.get<Color> ("color");
		begin[1] = color.r;
		begin[2] = color.g;
		begin[3] = color.b;
		break;
	case RGBEffectPulse:
		color = effect.get<Color> ("color");
		begin[2] = color.r;
		begin[3] = color.g;
		begin[4] = color.b;
		writeBE<uint16_t> (begin+5, effect.get<int> ("period"));
		begin[7] = effect.get<int> ("brightness");
		break;
	case RGBEffectCycle:
		writeBE<uint16_t> (begin+6, effect.get<int> ("period"));
		begin[8] = effect.get<int> ("brightness");
		break;
	}
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
	if (_has_power_modes) {
		profile.settings.emplace ("power_mode",
					  EnumValue (PowerModes, begin[16]));
	}
	profile.settings.emplace ("angle_snapping", begin[17] != 0);
	int rev = readLE<uint16_t> (begin+18);
	profile.settings.emplace ("revision", rev);
	for (unsigned int i = 0; i < (_has_g_shift ? 2 : 1); ++i) { // Normal/alternate buttons
		for (unsigned int j = 0; j < _desc.button_count; ++j) {
			auto button_data = begin+32 + 4*(i*MaxButtonCount + j);
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
	}
	std::u16string name;
	for (int i = 0; i < 24; ++i)
		name.push_back (readLE<char16_t> (begin+160+2*i));
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv16;
	profile.settings.emplace ("name", conv16.to_bytes (name));
	if (_has_rgb_effects) {
		profile.settings.emplace ("logo_effect", readRGBEffect (begin+208));
		profile.settings.emplace ("side_effect", readRGBEffect (begin+219));
	}
	return profile;
}

void ProfileFormat::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
	std::fill (begin, begin + ProfileLength.at (_desc.profile_format), 0xff);
	SettingLookup general (profile.settings, _general_settings);
	begin[0] = general.get<int> ("report_rate");
	begin[1] = general.get<int> ("default_dpi");
	begin[2] = general.get<int> ("switched_dpi");
	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		if (i < profile.modes.size ()) {
			SettingLookup mode (profile.modes[i], ModeSettings);
			writeLE<uint16_t> (begin+3+2*i, mode.get<int> ("dpi"));
		}
		else {
			// Write 0 after for disabled modes. Not sure if useful but it mimics LGS
			writeLE<uint16_t> (begin+3+2*i, 0);
		}
	}
	Color color = general.get<Color> ("color");
	begin[13] = color.r;
	begin[14] = color.g,
	begin[15] = color.b;
	if (_has_power_modes)
		begin[16] = general.get<EnumValue> ("power_mode").get ();
	begin[17] = (general.get<bool> ("angle_snapping") ? 0x01 : 0x00);
	writeLE<uint16_t> (begin+18, general.get<int> ("revision"));
	for (unsigned int i = 0; i < (_has_g_shift ? 2 : 1); ++i) { // Normal/alternate buttons
		for (unsigned int j = 0; j < _desc.button_count; ++j) {
			auto button_data = begin+32 + 4*(MaxButtonCount*i + j);
			const auto &button = profile.buttons[i*_desc.button_count + j];
			switch (button.type ()) {
			case Profile::Button::Type::Disabled:
				button_data[0] = ButtonDisabled;
				break;
			case Profile::Button::Type::MouseButtons:
				button_data[0] = ButtonHID;
				button_data[1] = ButtonHIDMouse;
				writeBE<uint16_t> (button_data+2, button.mouseButtons ());
				break;
			case Profile::Button::Type::Key:
				button_data[0] = ButtonHID;
				button_data[1] = ButtonHIDKey;
				button_data[2] = button.modifierKeys ();
				button_data[3] = button.key ();
				break;
			case Profile::Button::Type::ConsumerControl:
				button_data[0] = ButtonHID;
				button_data[1] = ButtonHIDConsumerControl;
				writeBE<uint16_t> (button_data+2, button.consumerControl ());
				break;
			case Profile::Button::Type::Special:
				button_data[0] = ButtonSpecial;
				button_data[1] = button.special ();
				break;
			case Profile::Button::Type::Macro: {
				Address addr = button.macro ();
				button_data[0] = addr.mem_type;
				button_data[1] = addr.page;
				writeBE<uint16_t> (button_data+2, addr.offset);
				break;
			}
			}
		}
	}
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv16;
	std::u16string name = conv16.from_bytes (general.get<std::string> ("name"));
	for (int i = 0; i < 24; ++i)
		writeLE<char16_t> (begin+160+2*i, name[i]);
	if (_has_rgb_effects) {
		writeRGBEffect (begin+208, general.get<ComposedSetting> ("logo_effect"));
		writeRGBEffect (begin+219, general.get<ComposedSetting> ("side_effect"));
	}
}

const std::map<IOnboardProfiles::ProfileFormat, size_t> ProfileFormat::ProfileLength = {
	{ IOnboardProfiles::ProfileFormat::G402, 208 }, // actually 224, but ignoring data at the end right now
	{ IOnboardProfiles::ProfileFormat::G303, 230 },
	{ IOnboardProfiles::ProfileFormat::G900, 230 },
};

const std::map<std::string, SettingDesc> ProfileFormat::RGBEffectSettings = {
	{ "type", SettingDesc (RGBEffects, RGBEffectConstant) },
	{ "color", SettingDesc (Color { 255, 255, 255 }) },
	{ "period", SettingDesc (0, 65535, 10000) },
	{ "brightness", SettingDesc (0, 100, 100) },
};

const std::map<std::string, SettingDesc> ProfileFormat::CommonGeneralSettings = {
	{ "report_rate", SettingDesc (1, 8, 4) },
	{ "default_dpi", SettingDesc (0, MaxModeCount-1, 0) },
	{ "color", SettingDesc (Color { 255, 255, 255 }) }, // Unused on G502 (profile format 2)
	{ "angle_snapping", SettingDesc (false) },
	{ "revision", SettingDesc (0, 65535, 65535) },
	{ "name", SettingDesc (std::string ("profile name")) },
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

const EnumDesc ProfileFormat::RGBEffects = {
	{ "Off", RGBEffectOff},
	{ "Constant", RGBEffectConstant },
	{ "Pulse", RGBEffectPulse },
	{ "Cycle", RGBEffectCycle },
};

const EnumDesc ProfileFormat::PowerModes = {
	{ "NotApplicable", 0xff },
};

std::unique_ptr<Base::ProfileFormat> HIDPP20::getProfileFormat (HIDPP20::Device *device)
{
	auto desc = IOnboardProfiles (device).getDescription ();
	return std::unique_ptr<Base::ProfileFormat> (new ProfileFormat (desc));
}
