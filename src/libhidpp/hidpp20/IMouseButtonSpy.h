/*
 * Copyright 2017 Clément Vuchener
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

#ifndef LIBHIDPP_HIDPP20_IMOUSEBUTTONSPY_H
#define LIBHIDPP_HIDPP20_IMOUSEBUTTONSPY_H

#include <hidpp20/FeatureInterface.h>

namespace HIDPP20
{

/**
 * Get or divert mouse button events
 *
 * MouseButtonSpy can send events with raw button
 * events indepedant of any remapping or profile
 * settings.
 *
 * The event reports 16 buttons state as bits in
 * a uint16_t value.
 *
 * Button can be remapped or disabled for standard
 * HID reports but it does not affect how they are
 * reported in MouseButtonSpy event.
 *
 * If an on-board profile is used, the remapping does
 * not apply. For using the remapping from this feature,
 * on-board profiles must be disabled by setting HostMode
 * in the OnboardProfiles feature.
 *
 * Button mapping is reset with the mouse.
 */
class IMouseButtonSpy: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x8110;

	enum Function {
		GetMouseButtonCount = 0,
		StartMouseButtonSpy = 1,
		StopMouseButtonSpy = 2,
		GetMouseButtonMapping = 3,
		SetMouseButtonMapping = 4,
	};

	enum Event {
		MouseButtonEvent = 0,
	};

	IMouseButtonSpy (Device *dev);

	unsigned int getMouseButtonCount ();

	/**
	 * Start sending mouse button events.
	 */
	void startMouseButtonSpy ();
	/**
	 * Stop sending mouse button events.
	 */
	void stopMouseButtonSpy ();

	/**
	 * Get the current button mapping.
	 *
	 * Disabled buttons have code 0. Enabled buttons use codes 1 to 16.
	 */
	std::vector<uint8_t> getMouseButtonMapping ();
	/**
	 * Set the current button mapping.
	 *
	 * 0 disables the button. Valid buttons are 1 to 16.
	 *
	 * At most 16 buttons can be remapped.
	 */
	void setMouseButtonMapping (const std::vector<uint8_t> &button_mapping);

	/**
	 * Parse the mouse button event.
	 *
	 * \returns mouse button bits.
	 */
	static uint16_t mouseButtonEvent (const HIDPP::Report &event);
};

}

#endif

