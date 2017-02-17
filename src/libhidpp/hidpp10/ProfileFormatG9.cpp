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

#include "ProfileFormatG9.h"

#include <hidpp10/ProfileFormatCommon.h>
#include <hidpp/SettingLookup.h>
#include <hidpp/Field.h>

using namespace HIDPP;
using namespace HIDPP10;

namespace Fields
{
static constexpr std::size_t ModeSize = 3;

static constexpr auto ProfileColor =	Field<Color> (0);
static constexpr auto Unknown0 =	Field<uint8_t> (1);
static constexpr auto Modes =		StructArrayField<ModeSize, 5> (2);
static constexpr auto DefaultDPI =	Field<uint8_t> (19);
static constexpr auto Unknown1 =	Field<uint8_t> (20);
static constexpr auto Unknown2 =	Field<uint8_t> (21);
static constexpr auto ReportRate =	Field<uint8_t> (22);
static constexpr auto Buttons =		StructArrayField<ButtonSize, 10> (23);
static constexpr auto Unknown3 =	Field<uint8_t> (53);
static constexpr auto Unknown4 =	Field<uint8_t> (54);
static constexpr auto Unknown5 =	Field<uint8_t> (55);

namespace Mode
{
static constexpr auto DPI =	Field<uint8_t> (0);
static constexpr auto LEDs =	Field<uint16_t, LittleEndian> (1);
}
}

const std::map<std::string, SettingDesc> ProfileFormatG9::GeneralSettings = {
	{ "color", SettingDesc (Color { 255, 0, 0 }) },
	{ "unknown0", SettingDesc (0x00, 0xff, 0x10) },
	{ "default_dpi", SettingDesc (0, MaxModeCount-1, 0) },
	{ "default_dpi_bit7", SettingDesc (false) },
	{ "unknown1", SettingDesc (0x00, 0xff, 0x21) },
	{ "unknown2", SettingDesc (0x00, 0xff, 0xa2) },
	{ "report_rate", SettingDesc (1, 8, 4) },
	{ "unknown3", SettingDesc (0x00, 0xff, 0x8f) },
	{ "unknown4", SettingDesc (0x00, 0xff, 0x00) },
	{ "unknown5", SettingDesc (0x00, 0xff, 0x00) },
};

const EnumDesc ProfileFormatG9::SpecialActions = {
	// TODO: find special actions supported by G9.
	// Using the same as the G500 in the mean time.
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

ProfileFormatG9::ProfileFormatG9 (const Sensor &sensor):
	AbstractProfileFormat (ProfileSize, MaxButtonCount, MaxModeCount),
	_sensor (sensor),
	_dpi_setting (sensor.minimumResolution (), sensor.maximumResolution (),
			std::min (800u, sensor.maximumResolution ())),
	_mode_settings {
		{ "dpi", _dpi_setting },
		{ "leds", SettingDesc (LEDVector (LEDCount, false)) },
	}
{
}

const std::map<std::string, SettingDesc> &ProfileFormatG9::generalSettings () const
{
	return GeneralSettings;
}

const std::map<std::string, SettingDesc> &ProfileFormatG9::modeSettings () const
{
	return _mode_settings;
}

const EnumDesc &ProfileFormatG9::specialActions () const
{
	return SpecialActions;
}

Profile ProfileFormatG9::read (std::vector<uint8_t>::const_iterator begin) const
{
	using namespace Fields;
	Profile profile;

	profile.settings.emplace ("color", ProfileColor.read (begin));
	profile.settings.emplace ("unknown0", static_cast<int> (Unknown0.read (begin)));

	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		auto mode = Modes.begin (begin, i);
		uint8_t dpi = Mode::DPI.read (mode);
		if (i > 0 && dpi == 0)
			break;
		LEDVector leds;
		uint16_t led_flags = Mode::LEDs.read (mode);
		for (unsigned int j = 0; j < LEDCount; ++j) {
			int led = (led_flags >> 4*j) & 0x0f;
			if (led == 0)
				break;
			leds.push_back (led == 0x02);
		}
		profile.modes.push_back ({
			{ "dpi", static_cast<int> (_sensor.toDPI (dpi)) },
			{ "leds", leds },
		});
	}

	uint8_t default_dpi = DefaultDPI.read (begin);
	bool bit7 = default_dpi & 0x80;
	profile.settings.emplace ("default_dpi", static_cast<int> (default_dpi & ~0x80));
	profile.settings.emplace ("default_dpi_bit7", bit7);

	profile.settings.emplace ("unknown1", static_cast<int> (Unknown1.read (begin)));
	profile.settings.emplace ("unknown2", static_cast<int> (Unknown2.read (begin)));
	profile.settings.emplace ("report_rate", static_cast<int> (ReportRate.read (begin)));

	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		profile.buttons.push_back (parseButton (Buttons.begin (begin, i)));
	}

	profile.settings.emplace ("unknown3", static_cast<int> (Unknown3.read (begin)));
	profile.settings.emplace ("unknown4", static_cast<int> (Unknown4.read (begin)));
	profile.settings.emplace ("unknown5", static_cast<int> (Unknown5.read (begin)));

	return profile;
}

void ProfileFormatG9::write (const Profile &profile, std::vector<uint8_t>::iterator begin) const
{
	using namespace Fields;
	SettingLookup general (profile.settings, GeneralSettings);

	ProfileColor.write (begin, general.get<Color> ("color"));
	Unknown0.write (begin, general.get<int> ("unknown0"));

	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		auto it = Modes.begin (begin, i);
		if (i >= profile.modes.size ()) {
			std::fill (Modes.begin (begin, i), Modes.end (begin, i), 0);
		}
		else {
			SettingLookup mode (profile.modes[i], _mode_settings);

			int dpi = mode.get<int> ("dpi");
			Mode::DPI.write (it, _sensor.fromDPI (dpi));

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
	if (general.get<bool> ("default_dpi_bit7"))
		default_dpi |= 0x80;
	DefaultDPI.write (begin, default_dpi);

	Unknown1.write (begin, general.get<int> ("unknown1"));
	Unknown2.write (begin, general.get<int> ("unknown2"));
	ReportRate.write (begin, general.get<int> ("report_rate"));

	for (unsigned int i = 0; i < MaxButtonCount; ++i) {
		Profile::Button button;
		if (i < profile.buttons.size ())
			button = profile.buttons[i];
		writeButton (Buttons.begin (begin, i), button);
	}

	Unknown3.write (begin, general.get<int> ("unknown3"));
	Unknown4.write (begin, general.get<int> ("unknown4"));
	Unknown5.write (begin, general.get<int> ("unknown5"));
}

