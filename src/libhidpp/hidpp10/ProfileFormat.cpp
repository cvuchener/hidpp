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

#include <hidpp10/Device.h>
#include <hidpp10/DeviceInfo.h>
#include <hidpp/SettingLookup.h>
#include <hidpp/Field.h>

using namespace HIDPP;
using namespace HIDPP10;

static constexpr auto ButtonSize = 3;
namespace ButtonFields
{
static constexpr auto Type =		Field<uint8_t> (0);
static constexpr auto MouseButtons =	Field<uint16_t, LittleEndian> (1);
static constexpr auto Modifiers =	Field<uint8_t> (1);
static constexpr auto Key =		Field<uint8_t> (2);
static constexpr auto Special =		Field<uint16_t, LittleEndian> (1);
static constexpr auto ConsumerControl =	Field<uint16_t, LittleEndian> (1);
static constexpr auto MacroPage =	Field<uint8_t> (0);
static constexpr auto MacroOffset =	Field<uint8_t> (1);
}

enum ButtonType: uint8_t
{
	ButtonMouse = 0x81,
	ButtonKey = 0x82,
	ButtonSpecial = 0x83,
	ButtonConsumerControl = 0x84,
	ButtonDisabled = 0x8f,
};

enum SpecialAction: uint16_t
{
	WheelLeft = 0x01,
	WheelRight = 0x02,
	BatteryLevel = 0x03,
	ResolutionNext = 0x04,
	ResolutionCycleNext = 0x05,
	ResolutionPrev = 0x08,
	ResolutionCyclePrev = 0x09,
	ProfileNext = 0x10,
	ProfileCycleNext = 0x11,
	ProfilePrev = 0x20,
	ProfileCyclePrev = 0x21,
	ProfileSwitch = 0x40,
};

static
Profile::Button parseButton (std::vector<uint8_t>::const_iterator begin)
{
	using namespace ButtonFields;
	switch (Type.read (begin)) {
	case ButtonMouse:
		return Profile::Button (Profile::Button::MouseButtonsType (),
					MouseButtons.read (begin));
	case ButtonKey:
		return Profile::Button (Modifiers.read (begin),
					Key.read (begin));
	case ButtonSpecial:
		return Profile::Button (Profile::Button::SpecialType (),
					Special.read (begin));
	case ButtonConsumerControl:
		return Profile::Button (Profile::Button::ConsumerControlType (),
					ConsumerControl.read (begin));
	case ButtonDisabled:
		return Profile::Button ();
	default:
		return Profile::Button (Address {
			0,
			MacroPage.read (begin),
			MacroOffset.read (begin)
		});
	}
}

static
void writeButton (std::vector<uint8_t>::iterator begin, const Profile::Button &button)
{
	using namespace ButtonFields;
	std::fill (begin, begin+ButtonSize, 0);
	switch (button.type ()) {
	case Profile::Button::Type::Disabled:
		Type.write (begin, ButtonDisabled);
		break;
	case Profile::Button::Type::MouseButtons:
		Type.write (begin, ButtonMouse);
		MouseButtons.write (begin, button.mouseButtons ());
		break;
	case Profile::Button::Type::Key:
		Type.write (begin, ButtonKey);
		Modifiers.write (begin, button.modifierKeys ());
		Key.write (begin, button.key ());
		break;
	case Profile::Button::Type::ConsumerControl:
		Type.write (begin, ButtonConsumerControl);
		ConsumerControl.write (begin, button.consumerControl ());
		break;
	case Profile::Button::Type::Special:
		Type.write (begin, ButtonSpecial);
		Special.write (begin, button.special ());
		break;
	case Profile::Button::Type::Macro: {
		Address addr = button.macro ();
		MacroPage.write (begin, addr.page);
		MacroOffset.write (begin, addr.offset);
		break;
	}
	}
}

namespace G500Fields
{
static constexpr std::size_t ModeSize = 6;

static constexpr auto ProfileColor =	Field<Color> (0);
static constexpr auto Angle =		Field<uint8_t> (3);
static constexpr auto Modes =		StructArrayField<ModeSize, 5> (4);
static constexpr auto AngleSnapping =	Field<uint8_t> (34);
static constexpr auto DefaultDPI =	Field<uint8_t> (35);
static constexpr auto LiftThreshold =	Field<uint8_t> (36);
static constexpr auto Unknown =		Field<uint8_t> (37);
static constexpr auto ReportRate =	Field<uint8_t> (38);
static constexpr auto Buttons =		StructArrayField<ButtonSize, 13> (39);

namespace Mode
{
static constexpr auto DPIX =	Field<uint16_t, BigEndian> (0);
static constexpr auto DPIY =	Field<uint16_t, BigEndian> (2);
static constexpr auto LEDs =	Field<uint16_t, LittleEndian> (4);
}
}

const std::map<std::string, SettingDesc> G500ProfileFormat::GeneralSettings = {
	{ "color", SettingDesc (Color { 255, 0, 0 }) },
	{ "angle", SettingDesc (0x00, 0xff, 0x80) },
	{ "angle_snapping", SettingDesc (false) },
	{ "default_dpi", SettingDesc (0, MaxModeCount-1, 0) },
	{ "lift_threshold", SettingDesc (-15, 15, 0) },
	{ "unknown", SettingDesc (0x00, 0xff, 0x10) },
	{ "report_rate", SettingDesc (1, 8, 4) },
};

const EnumDesc G500ProfileFormat::SpecialActions = {
	{ "WheelLeft", WheelLeft },
	{ "WheelRight", WheelRight },
	{ "BatteryLevel", BatteryLevel },
	{ "ResolutionNext", ResolutionNext },
	{ "ResolutionCycleNext", ResolutionCycleNext },
	{ "ResolutionPrev", ResolutionPrev },
	{ "ResolutionCyclePrev", ResolutionCyclePrev },
	{ "ProfileNext", ProfileNext },
	{ "ProfileCycleNext", ProfileCycleNext },
	{ "ProfilePrev", ProfilePrev },
	{ "ProfileCyclePrev", ProfileCyclePrev },
	{ "ProfileSwitch0", ProfileSwitch + (0<<8) },
	{ "ProfileSwitch1", ProfileSwitch + (1<<8) },
	{ "ProfileSwitch2", ProfileSwitch + (2<<8) },
	{ "ProfileSwitch3", ProfileSwitch + (3<<8) },
	{ "ProfileSwitch4", ProfileSwitch + (4<<8) },
};

G500ProfileFormat::G500ProfileFormat (const Sensor &sensor):
	Base::ProfileFormat (ProfileSize, MaxButtonCount, MaxModeCount),
	_sensor (sensor),
	_dpi_setting (sensor.minimumResolution (), sensor.maximumResolution (),
			std::min (800u, sensor.maximumResolution ())),
	_mode_settings {
		{ "dpi_x", _dpi_setting },
		{ "dpi_y", _dpi_setting },
		{ "leds", SettingDesc (LEDVector (LEDCount, false)) },
	}
{
}

const std::map<std::string, SettingDesc> &G500ProfileFormat::generalSettings () const
{
	return GeneralSettings;
}

const std::map<std::string, SettingDesc> &G500ProfileFormat::modeSettings () const
{
	return _mode_settings;
}

const EnumDesc &G500ProfileFormat::specialActions () const
{
	return SpecialActions;
}

Profile G500ProfileFormat::read (std::vector<uint8_t>::const_iterator begin) const
{
	using namespace G500Fields;
	Profile profile;
	profile.settings.emplace ("color", ProfileColor.read (begin));
	profile.settings.emplace ("angle", static_cast<int> (Angle.read (begin)));
	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		auto mode = Modes.begin (begin, i);
		uint16_t dpi_x = Mode::DPIX.read (mode);
		if (i > 0 && dpi_x == 0)
			break;
		uint16_t dpi_y = Mode::DPIY.read (mode);
		LEDVector leds;
		uint16_t led_flags = Mode::LEDs.read (mode);
		for (unsigned int j = 0; j < LEDCount; ++j) {
			int led = (led_flags >> 4*j) & 0x0f;
			if (led == 0)
				break;
			leds.push_back (led == 0x02);
		}
		profile.modes.push_back ({
			{ "dpi_x", static_cast<int> (_sensor.toDPI (dpi_x)) },
			{ "dpi_y", static_cast<int> (_sensor.toDPI (dpi_y)) },
			{ "leds", leds },
		});
	}
	profile.settings.emplace ("angle_snapping", AngleSnapping.read (begin) == 0x02);
	profile.settings.emplace ("default_dpi", static_cast<int> (DefaultDPI.read (begin)));
	profile.settings.emplace ("lift_threshold", static_cast<int> (LiftThreshold.read (begin))-16);
	profile.settings.emplace ("unknown", static_cast<int> (Unknown.read (begin)));
	profile.settings.emplace ("report_rate", static_cast<int> (ReportRate.read (begin)));
	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		profile.buttons.push_back (parseButton (Buttons.begin (begin, i)));
	}
	return profile;
}

void G500ProfileFormat::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
	using namespace G500Fields;
	SettingLookup general (profile.settings, GeneralSettings);
	ProfileColor.write (begin, general.get<Color> ("color"));
	Angle.write (begin, general.get<int> ("angle"));

	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		auto it = Modes.begin (begin, i);
		if (i >= profile.modes.size ()) {
			std::fill (Modes.begin (begin, i), Modes.end (begin, i), 0);
		}
		else {
			SettingLookup mode (profile.modes[i], _mode_settings);

			int dpi_x = mode.get<int> ("dpi_x");
			Mode::DPIX.write (it, _sensor.fromDPI (dpi_x));
			int dpi_y = mode.get ("dpi_y", dpi_x);
			Mode::DPIY.write (it, _sensor.fromDPI (dpi_y));

			LEDVector leds = mode.get<LEDVector> ("leds");
			uint16_t led_flags = 0;
			for (unsigned int j = 0; j < LEDCount && j < leds.size (); ++j)
				led_flags |= (leds[j] ? 0x02 : 0x01) << 4*j;
			Mode::LEDs.write (it, led_flags);
		}
	}

	bool angle_snapping = general.get<bool> ("angle_snapping");
	AngleSnapping.write (begin, angle_snapping ? 0x01 : 0x02);

	unsigned int default_dpi = general.get<int> ("default_dpi");
	if (default_dpi >= profile.modes.size ())
		default_dpi = profile.modes.size () - 1;
	DefaultDPI.write (begin, default_dpi);

	LiftThreshold.write (begin, 16 + general.get<int> ("lift_threshold"));
	Unknown.write (begin, general.get<int> ("unknown"));
	ReportRate.write (begin, general.get<int> ("report_rate"));

	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		Profile::Button button;
		if (i < profile.buttons.size ())
			button = profile.buttons[i];
		writeButton (Buttons.begin (begin, i), button);
	}
}

std::unique_ptr<Base::ProfileFormat> HIDPP10::getProfileFormat (HIDPP10::Device *device)
{
	auto info = getMouseInfo (device->productID ());
	switch (info->profile_type) {
	case G500ProfileType:
		return std::unique_ptr<Base::ProfileFormat> (new G500ProfileFormat (*info->sensor));
	default:
		throw std::runtime_error ("Unsupported device");
	}
}

