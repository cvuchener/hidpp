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

#include "Profile.h"

using namespace HIDPP;

Profile::Button::Button ():
	_type (Type::Disabled)
{
}

Profile::Button::Button (MouseButtonsType, unsigned int buttons):
	_type (Type::MouseButtons)
{
	_params.buttons = buttons;
}

Profile::Button::Button (uint8_t modifiers, uint8_t key):
	_type (Type::Key)
{
	_params.key.mod = modifiers;
	_params.key.key = key;
}

Profile::Button::Button (ConsumerControlType, unsigned int code):
	_type (Type::ConsumerControl)
{
	_params.code = code;
}

Profile::Button::Button (SpecialType, unsigned int code):
	_type (Type::Special)
{
	_params.code = code;
}

Profile::Button::Button (Address address):
	_type (Type::Macro)
{
	_params.address = address;
}

Profile::Button::Type Profile::Button::type () const
{
	return _type;
}

void Profile::Button::disable ()
{
	_type = Type::Disabled;
}

unsigned int Profile::Button::mouseButtons () const
{
	return _params.buttons;
}

void Profile::Button::setMouseButtons (unsigned int buttons)
{
	_type = Type::MouseButtons;
	_params.buttons = buttons;
}

uint8_t Profile::Button::modifierKeys () const
{
	return _params.key.mod;
}

uint8_t Profile::Button::key () const
{
	return _params.key.key;
}

void Profile::Button::setKey (uint8_t modifiers, uint8_t key)
{
	_type = Type::Key;
	_params.key.mod = modifiers;
	_params.key.key = key;
}

unsigned int Profile::Button::consumerControl () const
{
	return _params.code;
}

void Profile::Button::setConsumerControl (unsigned int code)
{
	_type = Type::ConsumerControl;
	_params.code = code;
}

unsigned int Profile::Button::special () const
{
	return _params.code;
}

void Profile::Button::setSpecial (unsigned int code)
{
	_type = Type::Special;
	_params.code = code;
}

Address Profile::Button::macro () const
{
	return _params.address;
}

void Profile::Button::setMacro (Address address)
{
	_type = Type::Macro;
	_params.address = address;
}

