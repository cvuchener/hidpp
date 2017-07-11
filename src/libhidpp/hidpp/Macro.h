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

#ifndef LIBHIDPP_HIDPP_MACRO_H
#define LIBHIDPP_HIDPP_MACRO_H

#include <string>
#include <map>
#include <cstdint>
#include <list>
#include <hidpp/Address.h>

namespace HIDPP
{

class AbstractMacroFormat;
class AbstractMemoryMapping;

/**
 * Store a macro as a list of macro items.
 *
 * ### Loop macro
 *
 * Loops recognized by isLoop() and created by buildLoop() are
 * divided in three parts: prologue, inner loop, epilogue.
 * Each part may be empty.
 *
 * It executes like the following pseudo-code:
 *
 *     play prologue
 *     while button is pressed
 *         play inner loop
 *     play epilogue
 *
 * If the loop delay is not null, the first iteration of the loop
 * will be delayed by the given time but the epilogue/end will
 * be played immediately if the button is released during this delay.
 */
class Macro
{
public:
	class Item
	{
	public:
		enum Instruction {
			/**
			 * No operation, do nothing (used for padding).
			 */
			NoOp,
			/**
			 * Wait for the button release.
			 */
			WaitRelease,
			/**
			 * Repeat the macro from the beginning.
			 */
			RepeatUntilRelease,
			/**
			 * Repeat the macro forever.
			 */
			RepeatForever,
			/**
			 * Press a key. \see setKeyCode()
			 */
			KeyPress,
			/**
			 * Release a key. \see setKeyCode()
			 */
			KeyRelease,
			/**
			 * Press modifier keys. \see setModifiers()
			 */
			ModifiersPress,
			/**
			 * Release modifier keys. \see setModifiers()
			 */
			ModifiersRelease,
			/**
			 * Press a standard key and modifer keys at
			 * the same time. \see setKeyCode() setModifiers()
			 */
			ModifiersKeyPress,
			/**
			 * Release a standard key and modifer keys at
			 * the same time. \see setKeyCode() setModifiers()
			 */
			ModifiersKeyRelease,
			/**
			 * Move the vertical wheel. \see setWheel()
			 */
			MouseWheel,
			/**
			 * Move the horizontal wheel. \see setWheel()
			 */
			MouseHWheel,
			/**
			 * Press mouse buttons. \see setButtons()
			 */
			MouseButtonPress,
			/**
			 * Release mouse buttons. \see setButtons()
			 */
			MouseButtonRelease,
			/**
			 * Set the currently pressed consumer control (0 for release). \see setConsumerControl()
			 */
			ConsumerControl,
			/**
			 * Press a consumer control key. \see setConsumerControl()
			 */
			ConsumerControlPress,
			/**
			 * Release a consumer control key. \see setConsumerControl()
			 */
			ConsumerControlRelease,
			/**
			 * Wait a specific time. \see setDelay()
			 */
			Delay,
			/**
			 * Wait a specific time. This a compressed version of Delay. \see setDelay()
			 */
			ShortDelay,
			/**
			 * Unconditional jump. \see setJumpDestination()
			 */
			Jump,
			/**
			 * Jump if the button is still pressed. \see setJumpDestination()
			 */
			JumpIfPressed,
			/**
			 * Move the mouse pointer. \see setMouseX() setMouseY()
			 */
			MousePointer,
			/**
			 * Jump if the button is released during a given time-out.
			 * \see setJumpDestination() setDelay()
			 */
			JumpIfReleased,
			/**
			 * Ends the macro.
			 */
			End,
		};

		static const std::map<Instruction, std::string> InstructionStrings;

		/**
		 * Create an item using instruction \p instr.
		 * If the instruction requires parameters, they
		 * must be set with the corresponding setter.
		 */
		Item (Instruction instr);

		Instruction instruction () const;

		/**
		 * \returns a HID usage code for a keyboard key.
		 * \see setKeyCode()
		 */
		uint8_t keyCode () const;
		/**
		 * \param key a HID usage code for a keyboard key.
		 * \see keyCode()
		 */
		void setKeyCode (uint8_t key);

		/**
		 * Modifier keys stored as an 8-bit field.
		 *
		 * Each bit corresponding to HID usage from 0xe0 to 0xe8.
		 *
		 * \return modifier keys as a bit field.
		 * \see setModifiers()
		 */
		uint8_t modifiers () const;
		/**
		 * \param modifiers modifier keys as a bit field.
		 * \see modifiers()
		 */
		void setModifiers (uint8_t modifiers);

		/**
		 * \returns wheel delta.
		 * \see setWheel()
		 */
		int wheel () const;
		/**
		 * \param wheel wheel delta.
		 * \see wheel()
		 */
		void setWheel (int wheel);

		/**
		 * Mouse buttons stored as a 16-bit field.
		 *
		 * \returns mouse buttons.
		 * \see setButtons()
		 */
		uint16_t buttons () const;
		/**
		 * \param buttons mouse buttons.
		 * \see buttons()
		 */
		void setButtons (uint16_t buttons);

		/**
		 * \returns HID usage code for a consumer control.
		 * \see setConsumerControl()
		 */
		uint16_t consumerControl () const;
		/**
		 * \param cc HID usage code for a consumer control.
		 * \see consumerControl()
		 */
		void setConsumerControl (uint16_t cc);

		/**
		 * \returns delay in milliseconds.
		 * \see setDelay()
		 */
		unsigned int delay () const;
		/**
		 * \param delay delay in milliseconds.
		 * \see delay()
		 */
		void setDelay (unsigned int delay);

		/**
		 * \returns true if the item is any kind of jump instruction.
		 * \see jumpDestination() setJumpDestination()
		 */
		bool isJump () const;
		/**
		 * \returns the destination of the jump.
		 * \see setJumpDestination()
		 */
		std::list<Item>::iterator jumpDestination ();
		/**
		 * \returns the destination of the jump.
		 * \see setJumpDestination()
		 */
		std::list<Item>::const_iterator jumpDestination () const;
		/**
		 * \param dest destination of the jump.
		 * \see jumpDestination()
		 */
		void setJumpDestination (std::list<Item>::iterator dest);

		/**
		 * \returns horizontal mouse pointer delta.
		 * \see setMouseX()
		 */
		int mouseX () const;
		/**
		 * \returns vertical mouse pointer delta.
		 * \see setMouseY()
		 */
		int mouseY () const;
		/**
		 * \param delta horizontal mouse pointer delta
		 * \see mouseX()
		 */
		void setMouseX (int delta);
		/**
		 * \param delta vertical mouse pointer delta
		 * \see mouseY()
		 */
		void setMouseY (int delta);

		/**
		 * A simple instruction is a simple action that do
		 * not affect the structure of the macro. It must
		 * not be a jump or loop or conditional instruction
		 * (conditional jump, wait release, repeat, ...).
		 */
		bool isSimple () const;
		/**
		 * An item has successor if the next item in the
		 * macro may be executed after this one.
		 *
		 * Unconditional jumps or end instruction do not
		 * have a successor.
		 */
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

	/**
	 * Create an empty macro.
	 */
	Macro ();

	/**
	 * Parse a macro from memory.
	 *
	 * \param format	Format for parsing the macro.
	 * \param mem		Memory for accessing the macro.
	 * \param address	Address of the macro start.
	 */
	Macro (const AbstractMacroFormat &format, AbstractMemoryMapping &mem, Address address);

	explicit Macro (const Macro &);
	Macro (Macro &&) = default;

	Macro &operator= (const Macro &) = delete;
	Macro &operator= (Macro &&) = default;

	/**
	 * Writes the macro to memory.
	 *
	 * \p start may be changed if there is not enough space at the
	 * given address.
	 *
	 * \param format	Format for encoding the macro.
	 * \param mem		Memory where to write the macro.
	 * \param [inout] start	Starting address of the macro
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

	/**
	 * Check if the macro only contains simple instructions
	 * except for the End instruction at the end.
	 */
	bool isSimple () const;
	/**
	 * Check if the macro can be interpreted as a loop with
	 * optional prologue (pre-loop macro), inner loop, and
	 * epilogue (post-loop macro).
	 *
	 * \param[out] pre_begin	Beginning of the prologue.
	 * \param[out] pre_end		After-end iterator of the prologue.
	 * \param[out] loop_begin	Beginning of the inner loop.
	 * \param[out] loop_end		After-end iterator of the inner loop.
	 * \param[out] post_begin	Beginning of the epilogue.
	 * \param[out] post_end		After-end iterator of the epilogue.
	 * \param[out] loop_delay	Delay of the inner loop.
	 */
	bool isLoop (const_iterator &pre_begin, const_iterator &pre_end,
		     const_iterator &loop_begin, const_iterator &loop_end,
		     const_iterator &post_begin, const_iterator &post_end,
		     unsigned int &loop_delay) const;

	/**
	 * Build a macro from simple items.
	 */
	static Macro buildSimple (const_iterator begin, const_iterator end);
	/**
	 * Build a loop macro from its three parts. Any of them may be empty.
	 *
	 * \param[in] pre_begin	Beginning of the prologue.
	 * \param[in] pre_end		After-end iterator of the prologue.
	 * \param[in] loop_begin	Beginning of the inner loop.
	 * \param[in] loop_end		After-end iterator of the inner loop.
	 * \param[in] post_begin	Beginning of the epilogue.
	 * \param[in] post_end		After-end iterator of the epilogue.
	 * \param[in] loop_delay	Delay of the inner loop.
	 */
	static Macro buildLoop (const_iterator pre_begin, const_iterator pre_end,
				const_iterator loop_begin, const_iterator loop_end,
				const_iterator post_begin, const_iterator post_end,
				unsigned int loop_delay);

private:
	std::list<Item> _items;
};

}

#endif

