/*
 * Copyright 2016-2017 Clément Vuchener
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

#include "ProfileFormatG500.h"

#include <hidpp10/ProfileFormatCommon.h>
#include <hidpp/SettingLookup.h>
#include <hidpp/Field.h>

using namespace HIDPP;
using namespace HIDPP10;

namespace Fields
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

const std::map<std::string, SettingDesc> ProfileFormatG500::GeneralSettings = {
	{ "color", SettingDesc (Color { 255, 0, 0 }) },
	{ "angle", SettingDesc (0x00, 0xff, 0x80) },
	{ "angle_snapping", SettingDesc (false) },
	{ "default_dpi", SettingDesc (0, MaxModeCount-1, 0) },
	{ "lift_threshold", SettingDesc (-15, 15, 0) },
	{ "unknown", SettingDesc (0x00, 0xff, 0x10) },
	{ "report_rate", SettingDesc (1, 8, 4) },
};

const EnumDesc ProfileFormatG500::SpecialActions = {
	{ "WheelLeft", WheelLeft },
	{ "WheelRight", WheelRight },
	{ "ResolutionNext", ResolutionNext },
	{ "ResolutionPrev", ResolutionPrev },
	{ "ProfileNext", ProfileNext },
	{ "ProfilePrev", ProfilePrev },
	{ "ProfileSwitch0", ProfileSwitch + (0<<8) },
	{ "ProfileSwitch1", ProfileSwitch + (1<<8) },
	{ "ProfileSwitch2", ProfileSwitch + (2<<8) },
	{ "ProfileSwitch3", ProfileSwitch + (3<<8) },
	{ "ProfileSwitch4", ProfileSwitch + (4<<8) },
};

ProfileFormatG500::ProfileFormatG500 (const Sensor &sensor):
	AbstractProfileFormat (ProfileSize, MaxButtonCount, MaxModeCount),
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

const std::map<std::string, SettingDesc> &ProfileFormatG500::generalSettings () const
{
	return GeneralSettings;
}

const std::map<std::string, SettingDesc> &ProfileFormatG500::modeSettings () const
{
	return _mode_settings;
}

const EnumDesc &ProfileFormatG500::specialActions () const
{
	return SpecialActions;
}

Profile ProfileFormatG500::read (std::vector<uint8_t>::const_iterator begin) const
{
	using namespace Fields;
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

void ProfileFormatG500::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
	using namespace Fields;
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

