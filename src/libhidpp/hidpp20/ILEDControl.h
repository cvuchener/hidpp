/*
 * Copyright 2019 Clément Vuchener
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

#ifndef LIBHIDPP_HIDPP20_ILEDCONTROL_H
#define LIBHIDPP_HIDPP20_ILEDCONTROL_H

#include <hidpp20/FeatureInterface.h>

namespace HIDPP20
{

/**
 * Control non-RGB LED features.
 */
class ILEDControl: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x1300;

	enum Function {
		GetCount = 0,
		GetInfo = 1,
		GetSWControl = 2,
		SetSWControl = 3,
		GetState = 4,
		SetState = 5,
		GetConfig = 6,
		SetConfig = 7,
	};

	ILEDControl (Device *dev);

	enum Type: uint8_t
	{
		Battery = 1,
		Dpi = 2,
		Profile = 3,
		Logo = 4,
		Cosmetic = 5,
	};

	enum Mode: uint16_t
	{
		Off = 0x0001,
		On = 0x0002,
		Blink = 0x0004,
		Travel = 0x0008,
		RampUp = 0x0010,
		RampDown = 0x0020,
		Heartbeat = 0x0040,
		Breathing = 0x0080,
	};

	enum Config: uint8_t
	{
		AlwaysOff = 0x01,
		AlwaysOn = 0x02,
		Auto = 0x04,
	};

	struct Info
	{
		Type type;
		uint8_t physical_count;
		uint16_t modes; ///< bit mask of available \ref Mode values
		uint8_t config_capabilities; ///< bit mask of available \ref Config values
	};

	struct State
	{
		Mode mode;
		union {
			struct {
				uint16_t index; ///< can be DPI index, profile index or 0xffff for all LEDs.
			} on; ///< Used when mode is \ref On.
			struct {
				uint16_t index;
				uint16_t on_duration;
				uint16_t off_duration;
			} blink; ///< Used when mode is \ref Blink.
			struct {
				uint16_t delay;
			} travel; ///< Used when mode is \ref Travel.
			struct {
				uint16_t max_brightness;
				uint16_t period;
				uint16_t timeout;
			} breathing; ///< used when mode is \ref Breathing.
		};
	};

	/**
	 * Get the number of LEDs.
	 */
	unsigned int getCount();

	/**
	 * Retrieve information on a LED.
	 */
	Info getInfo(unsigned int led_index);

	/**
	 * Get the current control mode.
	 *
	 * \return true if the LEDs are software-controlled.
	 */
	bool getSWControl();

	/**
	 * Set the current control mode.
	 */
	void setSWControl(bool software_controlled);

	/**
	 * Get the current state for a LED.
	 */
	State getState(unsigned int led_index);

	/**
	 * Change the current state for a LED.
	 *
	 * The device must be in sofware-control mode.
	 */
	void setState(unsigned int led_index, const State &state);

	/**
	 * Get the current non-volatile configuration for a LED.
	 */
	Config getConfig(unsigned int led_index);

	/**
	 * Set the non-volatile configurationfor a LED.
	 */
	void setConfig(unsigned int led_index, Config config);
};

}

#endif

