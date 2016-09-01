/*
 * Copyright 2015 Clément Vuchener
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

#ifndef HIDPP10_PROFILE_H
#define HIDPP10_PROFILE_H

#include <cstdint>
#include <string>
#include <vector>
#include <hidpp10/Address.h>

namespace HIDPP10
{

class Sensor;

enum ProfileType {
	NoProfile,
	G9ProfileType,
	G500ProfileType,
	G700ProfileType,
};

class Profile
{
public:
	struct Color {
		uint8_t r, g, b;
	};

	class Button {
	public:
		enum Type: uint8_t {
			Macro,
			MouseButton = 0x81,
			Key = 0x82,
			Special = 0x83,
			ConsumerControl = 0x84,
			Disabled = 0x8f,
		};

		enum SpecialFunction: uint16_t {
			PanLeft = 0x0001,
			PanRight = 0x0002,
			BatteryLevel = 0x0003,
			NextMode = 0x0004,
			PreviousMode = 0x0008,
			CycleMode = 0x0009,
			NextProfile = 0x0010,
			CycleProfile = 0x0011,
			PreviousProfile = 0x0020,
		};

		static std::string specialFunctionToString (SpecialFunction);
		static SpecialFunction specialFunctionFromString (const std::string &);

		Button ();

		void read (std::vector<uint8_t>::const_iterator begin);
		void write (std::vector<uint8_t>::iterator begin) const;

		Type type () const;

		unsigned int mouseButton () const;
		void setMouseButton (unsigned int button);

		uint8_t modifierKeys () const;
		uint8_t key () const;
		void setKey (uint8_t modifiers, uint8_t key_code);

		SpecialFunction special () const;
		void setSpecial (SpecialFunction special);

		uint16_t consumerControl () const;
		void setConsumerControl (uint16_t code);

		Address macro () const;
		void setMacro (Address address);

		void disable ();

	private:
		Type _type;
		union {
			uint16_t button;
			struct {
				uint8_t modifiers;
				uint8_t code;
			} key;
			SpecialFunction special;
			uint16_t consumer_control;
			Address macro;
		} _params;
	};

	Profile (unsigned int button_count);
	virtual ~Profile ();

	virtual std::size_t profileLength () const = 0;
	virtual void read (std::vector<uint8_t>::const_iterator begin) = 0;
	virtual void write (std::vector<uint8_t>::iterator begin) const = 0;

	unsigned int buttonCount () const;
	const Button &button (unsigned int index) const;
	Button &button (unsigned int index);

protected:
	void readButtons (std::vector<uint8_t>::const_iterator begin);
	void writeButtons (std::vector<uint8_t>::iterator begin) const;

private:
	std::vector<Button> _buttons;
};

class G500Profile: public Profile
{
public:
	G500Profile (const Sensor *sensor);
	virtual ~G500Profile ();

	virtual std::size_t profileLength () const;
	virtual void read (std::vector<uint8_t>::const_iterator begin);
	virtual void write (std::vector<uint8_t>::iterator begin) const;

	struct ResolutionMode {
		unsigned int x_res, y_res;
		std::vector<bool> leds;
	};

	static constexpr unsigned int MaxModeCount = 5;

	unsigned int modeCount () const;
	void setModeCount (unsigned int count);
	ResolutionMode resolutionMode (unsigned int index) const;
	void setResolutionMode (unsigned int index, ResolutionMode resolutionMode);

	unsigned int defaultMode () const;
	void setDefaultMode (unsigned int index);

	Color color () const;
	void setColor (Color color);

	bool angleSnap () const;
	void setAngleSnap (bool enabled);

	int liftThreshold () const;
	void setLiftThreshold (int lift);

	unsigned int pollInterval () const;
	void setPollInterval (unsigned int interval);

private:
	const Sensor *_sensor;
	Color _color;
	uint8_t _angle;
	std::vector<ResolutionMode> _modes;
	bool _angle_snap;
	unsigned int _default_mode;
	int _lift;
	uint8_t _unk;
	unsigned int _poll_interval;
};

}

#endif

