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

#include "ProfileFormatG700.h"

#include <hidpp10/ProfileFormatCommon.h>
#include <hidpp/SettingLookup.h>
#include <hidpp/Field.h>

using namespace HIDPP;
using namespace HIDPP10;

namespace Fields
{
static constexpr std::size_t ModeSize = 4;

static constexpr auto Modes =		StructArrayField<ModeSize, 5> (0);
static constexpr auto DefaultDPI =	Field<uint8_t> (20);
static constexpr auto Angle =		Field<uint8_t> (21);
static constexpr auto AngleSnapping =	Field<uint8_t> (22);
static constexpr auto Unknown0 =	Field<uint8_t> (23);
static constexpr auto ReportRate =	Field<uint8_t> (24);
static constexpr auto Unknown1 =	Field<uint8_t> (25);
static constexpr auto Unknown2 =	Field<uint8_t> (26);
static constexpr auto Unknown3 =	Field<uint8_t> (27);
static constexpr auto Unknown4 =	Field<uint8_t> (28);
static constexpr auto PowerMode =	Field<uint8_t> (29);
static constexpr auto Unknown5 =	Field<uint8_t> (30);
static constexpr auto Unknown6 =	Field<uint8_t> (31);
static constexpr auto Unknown7 =	Field<uint8_t> (32);
static constexpr auto Unknown8 =	Field<uint8_t> (33);
static constexpr auto Unknown9 =	Field<uint8_t> (34);
static constexpr auto Buttons =		StructArrayField<ButtonSize, 13> (35);

namespace Mode
{
static constexpr auto DPIX =	Field<uint8_t> (0);
static constexpr auto DPIY =	Field<uint8_t> (1);
static constexpr auto LEDs =	Field<uint16_t, LittleEndian> (2);
}
}

const std::map<std::string, SettingDesc> ProfileFormatG700::GeneralSettings = {
	{ "default_dpi", SettingDesc (0, MaxModeCount-1, 0) },
	{ "angle", SettingDesc (0x00, 0xff, 0x80) },
	{ "angle_snapping", SettingDesc (false) },
	{ "unknown0", SettingDesc (0x00, 0xff, 0x10) },
	{ "report_rate", SettingDesc (1, 8, 4) },
	{ "unknown1", SettingDesc (0x00, 0xff, 0x00) }, // 0x00 or 0x01
	{ "unknown2", SettingDesc (0x00, 0xff, 0x2c) }, // 0x2c or 0x0a
	{ "unknown3", SettingDesc (0x00, 0xff, 0x00) }, // 0x00 or 0x02 or 0x04
	{ "unknown4", SettingDesc (0x00, 0xff, 0x58) }, // 0x58 or 0xb0 or 0x3c
	{ "power_mode", SettingDesc (50, 200, 100) },
	{ "unknown5", SettingDesc (0x00, 0xff, 0xff) }, // 0xff or 0x1f
	{ "unknown6", SettingDesc (0x00, 0xff, 0xbc) },
	{ "unknown7", SettingDesc (0x00, 0xff, 0x00) },
	{ "unknown8", SettingDesc (0x00, 0xff, 0x09) },
	{ "unknown9", SettingDesc (0x00, 0xff, 0x31) },
};

const EnumDesc ProfileFormatG700::SpecialActions = {
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

ProfileFormatG700::ProfileFormatG700 (const Sensor &sensor):
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

const std::map<std::string, SettingDesc> &ProfileFormatG700::generalSettings () const
{
	return GeneralSettings;
}

const std::map<std::string, SettingDesc> &ProfileFormatG700::modeSettings () const
{
	return _mode_settings;
}

const EnumDesc &ProfileFormatG700::specialActions () const
{
	return SpecialActions;
}

Profile ProfileFormatG700::read (std::vector<uint8_t>::const_iterator begin) const
{
	using namespace Fields;
	Profile profile;

	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		auto mode = Modes.begin (begin, i);
		uint8_t dpi_x = Mode::DPIX.read (mode);
		if (i > 0 && dpi_x == 0)
			break;
		uint8_t dpi_y = Mode::DPIY.read (mode);
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

	profile.settings.emplace ("default_dpi", static_cast<int> (DefaultDPI.read (begin)));
	profile.settings.emplace ("angle", static_cast<int> (Angle.read (begin)));
	profile.settings.emplace ("angle_snapping", AngleSnapping.read (begin) == 0x02);
	profile.settings.emplace ("unknown0", static_cast<int> (Unknown0.read (begin)));
	profile.settings.emplace ("report_rate", static_cast<int> (ReportRate.read (begin)));
	profile.settings.emplace ("unknown1", static_cast<int> (Unknown1.read (begin)));
	profile.settings.emplace ("unknown2", static_cast<int> (Unknown2.read (begin)));
	profile.settings.emplace ("unknown3", static_cast<int> (Unknown3.read (begin)));
	profile.settings.emplace ("unknown4", static_cast<int> (Unknown4.read (begin)));
	profile.settings.emplace ("power_mode", static_cast<int> (PowerMode.read (begin)));
	profile.settings.emplace ("unknown5", static_cast<int> (Unknown5.read (begin)));
	profile.settings.emplace ("unknown6", static_cast<int> (Unknown6.read (begin)));
	profile.settings.emplace ("unknown7", static_cast<int> (Unknown7.read (begin)));
	profile.settings.emplace ("unknown8", static_cast<int> (Unknown8.read (begin)));
	profile.settings.emplace ("unknown9", static_cast<int> (Unknown9.read (begin)));

	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		profile.buttons.push_back (parseButton (Buttons.begin (begin, i)));
	}

	return profile;
}

void ProfileFormatG700::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
	using namespace Fields;
	SettingLookup general (profile.settings, GeneralSettings);

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

	unsigned int default_dpi = general.get<int> ("default_dpi");
	if (default_dpi >= profile.modes.size ())
		default_dpi = profile.modes.size () - 1;
	DefaultDPI.write (begin, default_dpi);

	Angle.write (begin, general.get<int> ("angle"));

	bool angle_snapping = general.get<bool> ("angle_snapping");
	AngleSnapping.write (begin, angle_snapping ? 0x01 : 0x02);

	Unknown0.write (begin, general.get<int> ("unknown0"));
	ReportRate.write (begin, general.get<int> ("report_rate"));
	Unknown1.write (begin, general.get<int> ("unknown1"));
	Unknown2.write (begin, general.get<int> ("unknown2"));
	Unknown3.write (begin, general.get<int> ("unknown3"));
	Unknown4.write (begin, general.get<int> ("unknown4"));
	PowerMode.write (begin, general.get<int> ("power_mode"));
	Unknown5.write (begin, general.get<int> ("unknown5"));
	Unknown6.write (begin, general.get<int> ("unknown6"));
	Unknown7.write (begin, general.get<int> ("unknown7"));
	Unknown8.write (begin, general.get<int> ("unknown8"));
	Unknown9.write (begin, general.get<int> ("unknown9"));

	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		Profile::Button button;
		if (i < profile.buttons.size ())
			button = profile.buttons[i];
		writeButton (Buttons.begin (begin, i), button);
	}
}

