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
#include <misc/Endian.h>

using namespace HIDPP;
using namespace HIDPP10;

enum ButtonType: uint8_t
{
	MouseButtons = 0x81,
	Key = 0x82,
	Special = 0x83,
	ConsumerControl = 0x84,
	Disabled = 0x8f,
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
	switch (*begin) {
	case ButtonType::MouseButtons:
		return Profile::Button (Profile::Button::MouseButtonsType (), readLE<uint16_t> (begin+1));
	case ButtonType::Key:
		return Profile::Button (begin[1], begin[2]);
	case ButtonType::Special:
		return Profile::Button (Profile::Button::SpecialType (), readLE<uint16_t> (begin+1));
	case ButtonType::ConsumerControl:
		return Profile::Button (Profile::Button::ConsumerControlType (), readLE<uint16_t> (begin+1));
	case ButtonType::Disabled:
		return Profile::Button ();
	default:
		return Profile::Button (Address { 0, begin[0], begin[1] });
	}
}

static
void writeButton (std::vector<uint8_t>::iterator begin, const Profile::Button &button)
{
	switch (button.type ()) {
	case Profile::Button::Type::Disabled:
		begin[0] = ButtonType::Disabled;
		begin[1] = begin[2] = 0;
		break;
	case Profile::Button::Type::MouseButtons:
		begin[0] = ButtonType::MouseButtons;
		writeLE<uint16_t> (begin+1, button.mouseButtons ());
		break;
	case Profile::Button::Type::Key:
		begin[0] = ButtonType::Key;
		begin[1] = button.modifierKeys ();
		begin[2] = button.key ();
		break;
	case Profile::Button::Type::ConsumerControl:
		begin[0] = ButtonType::ConsumerControl;
		writeLE<uint16_t> (begin+1, button.consumerControl ());
		break;
	case Profile::Button::Type::Special:
		begin[0] = ButtonType::Special;
		writeLE<uint16_t> (begin+1, button.special ());
		break;
	case Profile::Button::Type::Macro: {
		Address addr = button.macro ();
		begin[0] = addr.page;
		begin[1] = addr.offset;
		begin[2] = 0;
		break;
	}
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
	Profile profile;
	profile.settings.emplace ("color", Color {
		begin[0],
		begin[1],
		begin[2]
	});
	profile.settings.emplace ("angle", static_cast<int> (begin[3]));
	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		uint16_t dpi_x = readBE<uint16_t> (begin+4+6*i);
		if (i > 0 && dpi_x == 0)
			break;
		uint16_t dpi_y = readBE<uint16_t> (begin+4+6*i+2);
		LEDVector leds;
		for (unsigned int j = 0; j < LEDCount; ++j) {
			int led = (begin[4+6*i+4+j/2] >> 4*(j%2)) & 0x0f;
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
	profile.settings.emplace ("angle_snapping", begin[34] == 0x02);
	profile.settings.emplace ("default_dpi", static_cast<int> (begin[35]));
	profile.settings.emplace ("lift_threshold", static_cast<int> (begin[36])-16);
	profile.settings.emplace ("unknown", static_cast<int> (begin[37]));
	profile.settings.emplace ("report_rate", static_cast<int> (begin[38]));
	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		profile.buttons.push_back (parseButton (begin+39+3*i));
	}
	return profile;
}

void G500ProfileFormat::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
	SettingLookup general (profile.settings, GeneralSettings);
	Color color = general.get<Color> ("color");
	begin[0] = color.r;
	begin[1] = color.g;
	begin[2] = color.b;

	begin[3] = general.get<int> ("angle");

	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		auto it = begin+4+6*i;
		if (i >= profile.modes.size ()) {
			std::fill (it, it+6, 0);
		}
		else {
			SettingLookup mode (profile.modes[i], _mode_settings);

			int dpi_x = mode.get<int> ("dpi_x");
			writeBE<uint16_t> (it, _sensor.fromDPI (dpi_x));
			int dpi_y = mode.get ("dpi_y", dpi_x);
			writeBE<uint16_t> (it+2, _sensor.fromDPI (dpi_y));

			LEDVector leds = mode.get<LEDVector> ("leds");
			writeLE<uint16_t> (it+4, 0);
			for (unsigned int j = 0; j < LEDCount; ++j)
				it[4+j/2] |= (leds[j] ? 0x02 : 0x01) << 4*(j%2);
		}
	}

	bool angle_snapping = general.get<bool> ("angle_snapping");
	begin[34] = (angle_snapping ? 0x01 : 0x02);

	unsigned int default_dpi = general.get<int> ("default_dpi");
	if (default_dpi >= profile.modes.size ())
		default_dpi = profile.modes.size () - 1;
	begin[35] = default_dpi;

	begin[36] = 16 + general.get<int> ("lift_threshold");
	begin[37] = general.get<int> ("unknown");
	begin[38] = general.get<int> ("report_rate");

	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		Profile::Button button;
		if (i < profile.buttons.size ())
			button = profile.buttons[i];
		writeButton (begin+39+3*i, button);
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

