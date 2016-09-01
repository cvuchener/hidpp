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

#ifndef HIDPP10_MACRO_H
#define HIDPP10_MACRO_H

#include <cstdint>
#include <list>
#include <hidpp10/Address.h>
#include <hidpp10/MemoryMapping.h>

namespace HIDPP10
{

class Macro
{
public:
	class Item
	{
	public:
		Item (uint8_t op_code);

		enum OpCode: uint8_t {
			NoOp = 0x00,
			WaitRelease = 0x01,
			RepeatUntilRelease = 0x02,
			RepeatForever = 0x03,
			KeyPress = 0x20,
			KeyRelease = 0x21,
			ModifierPress = 0x22,
			ModifierRelease = 0x23,
			MouseWheel = 0x24,
			MouseButtonPress = 0x40,
			MouseButtonRelease = 0x41,
			ConsumerControl = 0x42,
			Delay = 0x43,
			Jump = 0x44,
			JumpIfPressed = 0x45,
			MousePointer = 0x60,
			JumpIfReleased = 0x61,
			End = 0xff,
		};

		static std::size_t getOpLength (uint8_t op_code);

		static uint8_t getShortDelayCode (unsigned int delay);
		static unsigned int getShortDelayDuration (uint8_t op_code);

		uint8_t opCode () const;
		bool isShortDelay () const;

		uint8_t keyCode () const;
		void setKeyCode (uint8_t key);

		uint8_t modifiers () const;
		void setModifiers (uint8_t modifiers);

		int8_t wheel () const;
		void setWheel (int8_t wheel);

		uint16_t buttons () const;
		void setButtons (uint16_t buttons);

		uint16_t consumerControl () const;
		void setConsumerControl (uint16_t cc);

		unsigned int delay () const;
		void setDelay (unsigned int delay);

		bool isJump () const;
		std::list<Item>::iterator jumpDestination ();
		std::list<Item>::const_iterator jumpDestination () const;
		void setJumpDestination (std::list<Item>::iterator);

		int mouseX () const;
		int mouseY () const;
		void setMouseX (int delta);
		void setMouseY (int delta);

		bool isSimple () const;

	private:
		uint8_t _op_code;
		union {
			uint8_t key;
			uint8_t modifiers;
			int8_t wheel;
			uint16_t buttons;
			uint16_t cc;
			uint16_t delay;
			struct {
				int16_t x, y;
			} mouse;
		} _params;
		std::list<Item>::iterator _dest;
	};

	Macro ();
	Macro (MemoryMapping &mem, Address address);
	Macro (std::vector<uint8_t>::const_iterator begin, Address start_address);

	explicit Macro (const Macro &);
	Macro (Macro &&) = default;

	Macro &operator= (const Macro &) = delete;
	Macro &operator= (Macro &&) = default;

	std::vector<uint8_t>::iterator write (std::vector<uint8_t>::iterator begin, Address start_address) const;
	Address write (MemoryMapping &mem, Address start) const;

	/**
	 * Remove no-op and useless unconditional jumps.
	 */
	void simplify ();

	typedef std::list<Item>::iterator iterator;
	typedef std::list<Item>::const_iterator const_iterator;

	iterator begin ();
	const_iterator begin () const;
	iterator end ();
	const_iterator end () const;

	const Item &back () const;
	Item &back ();

	void emplace_back (uint8_t op_code);

	bool isSimple () const;
	bool isLoop (const_iterator &pre_begin, const_iterator &pre_end,
		     const_iterator &loop_begin, const_iterator &loop_end,
		     const_iterator &post_begin, const_iterator &post_end,
		     unsigned int &loop_delay) const;

	static Macro buildSimple (const_iterator begin, const_iterator end);
	static Macro buildLoop (const_iterator pre_begin, const_iterator pre_end,
				const_iterator loop_begin, const_iterator loop_end,
				const_iterator post_begin, const_iterator post_end,
				unsigned int loop_delay);

private:
	std::list<Item> _items;
};

}

#endif

