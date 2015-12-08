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

#include <stdexcept>
#include <hidpp10/Profile.h>
#include <hidpp10/Sensor.h>

using namespace HIDPP10;

Profile::Button::Button ():
	_type (Disabled)
{
}

void Profile::Button::read (ByteArray::const_iterator begin)
{
	switch (*begin) {
	case MouseButton:
		_type = MouseButton;
		_params.button = ByteArray::getLE<uint16_t> (begin+1);
		break;
	
	case Key:
		_type = Key;
		_params.key.modifiers = *(begin+1);
		_params.key.code = *(begin+2);
		break;
	
	case Special:
		_type = Special;
		_params.special = static_cast<SpecialFunction> (ByteArray::getLE<uint16_t> (begin+1));
		break;

	case ConsumerControl:
		_type = ConsumerControl;
		_params.consumer_control = ByteArray::getBE<uint16_t> (begin+1);
		break;
			
	case Disabled:
		_type = Disabled;
		break;

	default:
		_type = Macro;
		_params.macro.page = *begin;
		_params.macro.offset = *(begin+1);
	}
}

void Profile::Button::write (ByteArray::iterator begin) const
{
	switch (_type) {
	case MouseButton:
		*begin = MouseButton;
		ByteArray::setLE<uint16_t> (begin+1, _params.button);
		break;
	
	case Key:
		*begin = Key;
		*(begin+1) = _params.key.modifiers;
		*(begin+2) = _params.key.code;
		break;
	
	case Special:
		*begin = Special;
		ByteArray::setLE<uint16_t> (begin+1, _params.special);
		break;

	case ConsumerControl:
		*begin = ConsumerControl;
		ByteArray::setBE<uint16_t> (begin+1, _params.consumer_control);
		break;
	
	case Disabled:
		*begin = Disabled;
		break;
	
	case Macro:
		*begin = _params.macro.page;
		*(begin+1) = _params.macro.offset;
		break;
	}
}

Profile::Button::Type Profile::Button::type () const
{
	return _type;
}

unsigned int Profile::Button::mouseButton () const
{
	return _params.button;
}

void Profile::Button::setMouseButton (unsigned int button)
{
	_type = MouseButton;
	_params.button = button;
}

uint8_t Profile::Button::modifierKeys () const
{
	return _params.key.modifiers;
}

uint8_t Profile::Button::key () const
{
	return _params.key.code;
}

void Profile::Button::setKey (uint8_t modifiers, uint8_t key_code)
{
	_type = Key;
	_params.key.modifiers = modifiers;
	_params.key.code = key_code;
}

Profile::Button::SpecialFunction Profile::Button::special () const
{
	return _params.special;
}

void Profile::Button::setSpecial (Profile::Button::SpecialFunction special)
{
	_type = Special;
	_params.special = special;
}

uint16_t Profile::Button::consumerControl () const
{
	return _params.consumer_control;
}

void Profile::Button::setConsumerControl (uint16_t code)
{
	_type = ConsumerControl;
	_params.consumer_control = code;
}

uint8_t Profile::Button::macroPage () const
{
	return _params.macro.page;
}

uint8_t Profile::Button::macroOffset () const
{
	return _params.macro.offset;
}

void Profile::Button::setMacro (uint8_t page, uint8_t offset)
{
	_type = Macro;
	_params.macro.page = page;
	_params.macro.offset = offset;
}

void Profile::Button::disable ()
{
	_type = Disabled;
}


unsigned int Profile::buttonCount () const
{
	return _buttons.size ();
}

const Profile::Button &Profile::button (unsigned int index) const
{
	return _buttons[index];
}

Profile::Button &Profile::button (unsigned int index)
{
	return _buttons[index];
}

Profile::Profile (unsigned int button_count):
	_buttons (button_count)
{
}

Profile::~Profile ()
{
}

void Profile::readButtons (ByteArray::const_iterator begin)
{
	for (Button &button: _buttons) {
		button.read (begin);
		begin += 3;
	}
}
	

void Profile::writeButtons (ByteArray::iterator begin) const
{
	for (const Button &button: _buttons) {
		button.write (begin);
		begin += 3;
	}
}

G500Profile::G500Profile (const Sensor *sensor):
	Profile (13), _sensor (sensor),
	_color ({ 255, 0, 0 }),
	_angle (0x80),
	_lift (0x10), _unk (0x10),
	_poll_interval (8)
{
}

G500Profile::~G500Profile ()
{
}

std::size_t G500Profile::profileLength () const
{
	return 78;
}

void G500Profile::read (ByteArray::const_iterator begin)
{
	_color.r = *(begin+0);
	_color.g = *(begin+1);
	_color.b = *(begin+2);
	_angle = *(begin+3);
	_modes.clear ();
	for (unsigned int i = 0; i < MaxModeCount; ++i) {
		uint16_t x_res = ByteArray::getBE<uint16_t> (begin+4+i*6);
		if (i > 0 && x_res == 0) // Mode is disabled
			break;
		uint16_t y_res = ByteArray::getBE<uint16_t> (begin+4+i*6+2);
		uint16_t leds_raw = ByteArray::getLE<uint16_t> (begin+4+i*6+4);
		std::vector<bool> leds;
		for (unsigned int j = 0; j < 4; ++j) {
			unsigned int l = (leds_raw >> j*4) & 0x0F;
			if (l == 0)
				break;
			else if (l == 1)
				leds.push_back (false);
			else if (l == 2)
				leds.push_back (true);
			else
				throw std::runtime_error ("Invalid LED value in G500 profile");
		}
		_modes.emplace_back (ResolutionMode ({
					_sensor->toDPI (x_res),
					_sensor->toDPI (y_res),
					leds }));
	}
	_angle_snap = *(begin+34) == 0x02;
	_default_mode = *(begin+35);
	_lift = *(begin+36);
	_unk = *(begin+37);
	_poll_interval = *(begin+38);
	readButtons (begin+39);
}

void G500Profile::write (ByteArray::iterator begin) const
{
	*(begin+0) = _color.r;
	*(begin+1) = _color.g;
	*(begin+2) = _color.b;
	*(begin+3) = _angle;
	for (unsigned int i = 0; i < _modes.size (); ++i) {
		ByteArray::setBE<uint16_t> (begin+4+i*6,
					    _sensor->fromDPI (_modes[i].x_res));
		ByteArray::setBE<uint16_t> (begin+4+i*6+2,
					    _sensor->fromDPI (_modes[i].y_res));
		uint16_t leds = 0;
		for (unsigned int j = 0; j < _modes[i].leds.size (); ++j)
			leds |= (_modes[i].leds[j] ? 0x02 : 0x01) << (j*4);
		ByteArray::setLE<uint16_t> (begin+4+i*6+4, leds);
	}
	// Disable the mode after the last one
	if (_modes.size () < MaxModeCount) {
		ByteArray::setBE<uint16_t> (begin+4+_modes.size ()*6, 0);
	}
	*(begin+34) = (_angle_snap ? 0x02 : 0x01);
	*(begin+35) = _default_mode;
	*(begin+36) = _lift;
	*(begin+37) = _unk;
	*(begin+38) = _poll_interval;
	writeButtons (begin+39);
}

unsigned int G500Profile::modeCount () const
{
	return _modes.size ();
}

void G500Profile::setModeCount (unsigned int count)
{
	if (count > MaxModeCount)
		count = MaxModeCount;
	_modes.resize (count);
	// TODO: Add default values for added modes
}

G500Profile::ResolutionMode G500Profile::resolutionMode (unsigned int index) const
{
	return _modes[index];
}

void G500Profile::setResolutionMode (unsigned int index, ResolutionMode resolutionMode)
{
	_modes[index] = resolutionMode;
}

unsigned int G500Profile::defaultMode () const
{
	return _default_mode;
}

void G500Profile::setDefaultMode (unsigned int index)
{
	_default_mode = index;
}

bool G500Profile::angleSnap () const
{
	return _angle_snap;
}

void G500Profile::setAngleSnap (bool enabled)
{
	_angle_snap = enabled;
}

unsigned int G500Profile::pollInterval () const
{
	return _poll_interval;
}

void G500Profile::setPollInterval (unsigned int interval)
{
	_poll_interval = interval;
}

