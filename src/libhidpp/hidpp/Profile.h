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

#ifndef LIBHIDPP_HIDPP_PROFILE_H
#define LIBHIDPP_HIDPP_PROFILE_H

#include <hidpp/Address.h>
#include <hidpp/Setting.h>

namespace HIDPP
{

struct Profile
{
	class Button
	{
	public:
		enum class Type {
			Disabled,
			MouseButtons,
			Key,
			ConsumerControl,
			Special,
			Macro,
		};

		struct MouseButtonsType {};
		struct ConsumerControlType {};
		struct SpecialType {};

		Button ();
		Button (MouseButtonsType, unsigned int buttons);
		Button (uint8_t modifiers, uint8_t key);
		Button (ConsumerControlType, unsigned int code);
		Button (SpecialType, unsigned int code);
		Button (Address address);

		Type type () const;

		void disable ();

		unsigned int mouseButtons () const;
		void setMouseButtons (unsigned int buttons);

		uint8_t modifierKeys () const;
		uint8_t key () const;
		void setKey (uint8_t modifiers, uint8_t key);

		unsigned int consumerControl () const;
		void setConsumerControl (unsigned int code);

		unsigned int special () const;
		void setSpecial (unsigned int code);

		Address macro () const;
		void setMacro (Address address);

	private:
		Type _type;
		union {
			unsigned int buttons;
			struct {
				uint8_t mod;
				uint8_t key;
			} key;
			unsigned int code;
			Address address;
		} _params;
	};

	std::map<std::string, Setting> settings;
	std::vector<Button> buttons;
	std::vector<std::map<std::string, Setting>> modes;
};

}

#endif
