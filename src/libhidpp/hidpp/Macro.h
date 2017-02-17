/*
 * Copyright 2015-2017 Clément Vuchener
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

#ifndef HIDPP_MACRO_H
#define HIDPP_MACRO_H

#include <string>
#include <map>
#include <cstdint>
#include <list>
#include <hidpp/Address.h>

namespace HIDPP
{

class AbstractMacroFormat;
class AbstractMemoryMapping;

class Macro
{
public:
	class Item
	{
	public:
		enum Instruction {
			NoOp,
			WaitRelease,
			RepeatUntilRelease,
			RepeatForever,
			KeyPress,
			KeyRelease,
			ModifiersPress,
			ModifiersRelease,
			ModifiersKeyPress,
			ModifiersKeyRelease,
			MouseWheel,
			MouseHWheel,
			MouseButtonPress,
			MouseButtonRelease,
			ConsumerControl,
			ConsumerControlPress,
			ConsumerControlRelease,
			Delay,
			ShortDelay,
			Jump,
			JumpIfPressed,
			MousePointer,
			JumpIfReleased,
			End,
		};

		static const std::map<Instruction, std::string> InstructionStrings;

		Item (Instruction instr);

		Instruction instruction () const;

		uint8_t keyCode () const;
		void setKeyCode (uint8_t key);

		uint8_t modifiers () const;
		void setModifiers (uint8_t modifiers);

		int wheel () const;
		void setWheel (int wheel);

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
		bool hasSuccessor () const;

	private:
		Instruction _instr;
		union {
			struct {
				uint8_t key;
				uint8_t modifiers;
			} key;
			int wheel;
			uint16_t buttons;
			uint16_t cc;
			unsigned int delay;
			struct {
				int x, y;
			} mouse;
		} _params;
		std::list<Item>::iterator _dest;
	};

	Macro ();
	Macro (const AbstractMacroFormat &format, AbstractMemoryMapping &mem, Address address);

	explicit Macro (const Macro &);
	Macro (Macro &&) = default;

	Macro &operator= (const Macro &) = delete;
	Macro &operator= (Macro &&) = default;

	/**
	 * Writes the macro in \p mem using \p format.
	 *
	 * \p start may be changed if there is not enough space at the
	 * given address.
	 *
	 * \returns the first address after the macro.
	 */
	Address write (const AbstractMacroFormat &format, AbstractMemoryMapping &mem, Address &start) const;

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

	void emplace_back (Item::Instruction instr);

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

