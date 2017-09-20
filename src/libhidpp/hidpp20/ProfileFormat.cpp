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
#include <hidpp/Field.h>
#include <misc/Endian.h>
#include <misc/Log.h>

#include <codecvt>
#include <cassert>

using namespace HIDPP;
using namespace HIDPP20;

namespace Fields
{

static constexpr std::size_t ButtonSize = 4;
static constexpr std::size_t RGBEffectSize = 11;

static constexpr auto ReportRate =	Field<uint8_t> (0);
static constexpr auto DefaultDPI =	Field<uint8_t> (1);
static constexpr auto SwitchedDPI =	Field<uint8_t> (2);
static constexpr auto Modes =		ArrayField<uint16_t, 5, LittleEndian> (3);
static constexpr auto ProfileColor =	Field<Color> (13);
static constexpr auto PowerMode =	Field<uint8_t> (16);
static constexpr auto AngleSnapping =	Field<uint8_t> (17);
static constexpr auto Revision =	Field<uint16_t, LittleEndian> (18);
static constexpr auto Buttons =		StructArrayField<ButtonSize, 32> (32);
static constexpr auto Name =		ArrayField<char16_t, 24, LittleEndian> (160);
static constexpr auto LogoEffect =	StructField<RGBEffectSize> (208);
static constexpr auto SideEffect =	StructField<RGBEffectSize> (219);

namespace Button
{
static constexpr auto Type =		Field<uint8_t> (0);

static constexpr auto HIDType =		Field<uint8_t> (1);
static constexpr auto MouseButtons =	Field<uint16_t, BigEndian> (2);
static constexpr auto Modifiers =	Field<uint8_t> (2);
static constexpr auto Key =		Field<uint8_t> (3);
static constexpr auto ConsumerControl =	Field<uint16_t, BigEndian> (2);

static constexpr auto Special =		Field<uint8_t> (1);

static constexpr auto MacroMemType =	Field<uint8_t> (0);
static constexpr auto MacroPage =	Field<uint8_t> (1);
static constexpr auto MacroOffset =	Field<uint16_t, BigEndian> (2);
}

namespace RGBEffect
{
static constexpr auto Type =		Field<uint8_t> (0);

static constexpr auto ConstantColor =	Field<Color> (1);

static constexpr auto PulseColor =	Field<Color> (1);
static constexpr auto PulsePeriod =	Field<uint16_t, BigEndian> (4);
static constexpr auto PulseBrightness =	Field<uint8_t> (7);

static constexpr auto CyclePeriod =	Field<uint16_t, BigEndian> (6);
static constexpr auto CycleBrightness =	Field<uint8_t> (8);
}
}

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
	AbstractProfileFormat (ProfileLength.at (desc.profile_format),
			       desc.button_count, MaxModeCount),
	_desc (desc),
	_general_settings (CommonGeneralSettings),
	_has_g_shift ((_desc.mechanical_layout & 0x03) == 2),
	_has_dpi_shift ((_desc.mechanical_layout & 0x0c) >> 2 == 2),
	_has_rgb_effects (_desc.profile_format >= 2),
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

static
Profile::Button readButton (std::vector<uint8_t>::const_iterator begin)
{
	using namespace Fields::Button;
	uint8_t type;
	switch (type = Type.read (begin)) {
	case ButtonHID:
		switch (type = HIDType.read (begin)) {
		case ButtonHIDMouse:
			return Profile::Button (Profile::Button::MouseButtonsType (),
						MouseButtons.read (begin));
		case ButtonHIDKey:
			return Profile::Button (Modifiers.read (begin), Key.read (begin));
		case ButtonHIDConsumerControl:
			return Profile::Button (Profile::Button::ConsumerControlType (),
						ConsumerControl.read (begin));
		default:
			Log::error () << "Invalid HID type code: " << type << std::endl;
			return Profile::Button ();
		}
	case ButtonSpecial:
		return Profile::Button (Profile::Button::SpecialType (),
					Special.read (begin));
	case ButtonMacro:
		return Profile::Button (Address {
			MacroMemType.read (begin),
			MacroPage.read (begin),
			MacroOffset.read (begin)
		});
	case ButtonDisabled:
		return Profile::Button ();
	default:
		Log::error () << "Invalid button type code: " << type << std::endl;
		return Profile::Button ();
	}
}

static
void writeButton (std::vector<uint8_t>::iterator begin, const Profile::Button &button)
{
	using namespace Fields::Button;
	switch (button.type ()) {
	case Profile::Button::Type::Disabled:
		Type.write (begin, ButtonDisabled);
		return;
	case Profile::Button::Type::MouseButtons:
		Type.write (begin, ButtonHID);
		HIDType.write (begin, ButtonHIDMouse);
		MouseButtons.write (begin, button.mouseButtons ());
		return;
	case Profile::Button::Type::Key:
		Type.write (begin, ButtonHID);
		HIDType.write (begin, ButtonHIDKey);
		Modifiers.write (begin, button.modifierKeys ());
		Key.write (begin, button.key ());
		return;
	case Profile::Button::Type::ConsumerControl:
		Type.write (begin, ButtonHID);
		HIDType.write (begin, ButtonHIDConsumerControl);
		ConsumerControl.write (begin, button.consumerControl ());
		return;
	case Profile::Button::Type::Special:
		Type.write (begin, ButtonSpecial);
		Special.write (begin, button.special ());
		return;
	case Profile::Button::Type::Macro: {
		Address addr = button.macro ();
		MacroMemType.write (begin, addr.mem_type);
		MacroPage.write (begin, addr.page);
		MacroOffset.write (begin, addr.offset);
		return;
	}
	}
}

ComposedSetting ProfileFormat::readRGBEffect (std::vector<uint8_t>::const_iterator begin)
{
	using namespace Fields::RGBEffect;
	ComposedSetting settings;
	uint8_t type = Type.read (begin);
	switch (type) {
	case RGBEffectOff:
		break;
	case RGBEffectConstant:
		settings.emplace ("color", ConstantColor.read (begin));
		break;
	case RGBEffectPulse:
		settings.emplace ("color", PulseColor.read (begin));
		settings.emplace ("period", static_cast<int> (PulsePeriod.read (begin)));
		settings.emplace ("brightness", static_cast<int> (PulseBrightness.read (begin)));
		break;
	case RGBEffectCycle:
		settings.emplace ("period", static_cast<int> (CyclePeriod.read (begin)));
		settings.emplace ("brightness", static_cast<int> (CycleBrightness.read (begin)));
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
	using namespace Fields::RGBEffect;
	std::fill (begin, begin+11, 0);
	SettingLookup effect (settings, RGBEffectSettings);
	int type = effect.get<EnumValue> ("type").get ();
	Type.write (begin, type);
	switch (type) {
	case RGBEffectOff:
		break;
	case RGBEffectConstant:
		ConstantColor.write (begin, effect.get<Color> ("color"));
		break;
	case RGBEffectPulse:
		PulseColor.write (begin, effect.get<Color> ("color"));
		PulsePeriod.write (begin, effect.get<int> ("period"));
		PulseBrightness.write (begin, effect.get<int> ("brightness"));
		break;
	case RGBEffectCycle:
		CyclePeriod.write (begin, effect.get<int> ("period"));
		CycleBrightness.write (begin, effect.get<int> ("brightness"));
		break;
	}
}


Profile ProfileFormat::read (std::vector<uint8_t>::const_iterator begin) const
{
	using namespace Fields;
	for (unsigned int i = 0; i < 16; ++i)
		Log::debug ().printBytes ("profile", begin+16*i, begin+16*(i+1));
	// TODO: missing settings
	// TODO: add settings depending on desc
	Profile profile;
	profile.settings.emplace ("report_rate", static_cast<int> (ReportRate.read (begin)));
	profile.settings.emplace ("default_dpi", static_cast<int> (DefaultDPI.read (begin)));
	profile.settings.emplace ("switched_dpi", static_cast<int> (SwitchedDPI.read (begin)));
	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		uint16_t dpi = Modes.read (begin, i);
		if (dpi == 0x0000 || dpi == 0xFFFF)
			break;
		profile.modes.push_back ({
			{ "dpi", static_cast<int> (dpi) },
		});
	}
	profile.settings.emplace ("color", ProfileColor.read (begin));
	if (_has_power_modes) {
		profile.settings.emplace ("power_mode",
					  EnumValue (PowerModes, PowerMode.read (begin)));
	}
	profile.settings.emplace ("angle_snapping", AngleSnapping.read (begin) != 0);
	profile.settings.emplace ("revision", static_cast<int> (Revision.read (begin)));
	for (unsigned int i = 0; i < (_has_g_shift ? 2 : 1); ++i) { // Normal/alternate buttons
		for (unsigned int j = 0; j < _desc.button_count; ++j) {
			auto button_data = Buttons.begin (begin, i*MaxButtonCount + j);
			profile.buttons.emplace_back (readButton (button_data));
		}
	}
	std::u16string name;
	for (int i = 0; i < 24; ++i)
		name.push_back (Name.read (begin, i));
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv16;
	profile.settings.emplace ("name", conv16.to_bytes (name));
	if (_has_rgb_effects) {
		profile.settings.emplace ("logo_effect",
					  readRGBEffect (LogoEffect.begin (begin)));
		profile.settings.emplace ("side_effect",
					  readRGBEffect (SideEffect.begin (begin)));
	}
	return profile;
}

void ProfileFormat::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
	using namespace Fields;
	std::fill (begin, begin + ProfileLength.at (_desc.profile_format), 0xff);
	SettingLookup general (profile.settings, _general_settings);
	ReportRate.write (begin, general.get<int> ("report_rate"));
	DefaultDPI.write (begin, general.get<int> ("default_dpi"));
	SwitchedDPI.write (begin, general.get<int> ("switched_dpi"));
	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		if (i < profile.modes.size ()) {
			SettingLookup mode (profile.modes[i], ModeSettings);
			Modes.write (begin, i, mode.get<int> ("dpi"));
		}
		else {
			// Write 0 after for disabled modes.
			// Not sure if useful (0xffff works too) but it mimics LGS
			Modes.write (begin, i, 0);
		}
	}
	ProfileColor.write (begin, general.get<Color> ("color"));
	if (_has_power_modes)
		PowerMode.write (begin, general.get<EnumValue> ("power_mode").get ());
	AngleSnapping.write (begin, general.get<bool> ("angle_snapping") ? 0x01 : 0x00);
	Revision.write (begin, general.get<int> ("revision"));
	for (unsigned int i = 0; i < (_has_g_shift ? 2 : 1); ++i) { // Normal/alternate buttons
		for (unsigned int j = 0; j < _desc.button_count; ++j) {
			auto button_data = Buttons.begin (begin, MaxButtonCount*i + j);
			const auto &button = profile.buttons[i*_desc.button_count + j];
			writeButton (button_data, button);
		}
	}
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv16;
	std::u16string name = conv16.from_bytes (general.get<std::string> ("name"));
	for (int i = 0; i < 24; ++i)
		Name.write (begin, i, name[i]);
	if (_has_rgb_effects) {
		writeRGBEffect (LogoEffect.begin (begin),
				general.get<ComposedSetting> ("logo_effect"));
		writeRGBEffect (SideEffect.begin (begin),
				general.get<ComposedSetting> ("side_effect"));
	}
}

const std::map<uint8_t, size_t> ProfileFormat::ProfileLength = {
	{ 1, 208 }, // actually 224, but ignoring data at the end right now
	{ 2, 230 },
	{ 3, 230 },
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
	{ "ResolutionCycle", 5 },
	{ "ResolutionDefault", 6 },
	{ "ResolutionSwitch", 7 },
	{ "ProfileNext", 8 },
	{ "ProfilePrev", 9 },
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

std::unique_ptr<AbstractProfileFormat> HIDPP20::getProfileFormat (HIDPP20::Device *device)
{
	auto desc = IOnboardProfiles (device).getDescription ();
	return std::unique_ptr<AbstractProfileFormat> (new ProfileFormat (desc));
}
