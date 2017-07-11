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

#ifndef LIBHIDPP_HIDPP_ABSTRACT_MACRO_FORMAT_H
#define LIBHIDPP_HIDPP_ABSTRACT_MACRO_FORMAT_H

#include <hidpp/Address.h>
#include <hidpp/Macro.h>
#include <vector>
#include <map>
#include <cstdint>

namespace HIDPP
{

/**
 * Abstract class from macro formats.
 *
 * At least getLength(), writeAddress(), writeItem()
 * and parseItem() must be implemented by the subclass.
 */
class AbstractMacroFormat
{
public:
	/**
	 * This exception is thrown when the macro format cannot
	 * encode the requested instruction.
	 */
	class UnsupportedInstruction: public std::exception
	{
	public:
		UnsupportedInstruction (Macro::Item::Instruction instr);

		virtual const char *what () const noexcept;

		Macro::Item::Instruction instruction () const;

	private:
		Macro::Item::Instruction _instr;
		std::string _msg;
	};

	/**
	 * Get the length of the encoded instruction.
	 */
	virtual std::size_t getLength (const Macro::Item &item) const = 0;

	/**
	 * Get the length for a jump instruction.
	 */
	virtual std::size_t getJumpLength () const;

	/**
	 * Write the address at the given location. The location
	 * must be the iterator give as \p jump_addr_it by writeItem().
	 */
	virtual void writeAddress (std::vector<uint8_t>::iterator it,
				   const Address &addr) const = 0;

	/**
	 * Write the macro item \p item at \p it.
	 *
	 * If the item is a jump the address must be later written
	 * at the position \p jump_addr_it using writeAddress().
	 *
	 * \returns the position after the written item.
	 */
	virtual std::vector<uint8_t>::iterator
	writeItem (std::vector<uint8_t>::iterator it,
		   const Macro::Item &item,
		   std::vector<uint8_t>::iterator &jump_addr_it) const = 0;

	/**
	 * Write a NoOp instruction at \p it.
	 *
	 * NoOp is used for padding and is expected to be exactly one byte long.
	 *
	 * \returns the position after the written item.
	 */
	std::vector<uint8_t>::iterator writeNoOp (std::vector<uint8_t>::iterator it) const;
	/**
	 * Write a jump to \p addr at \p it.
	 *
	 * \returns the position after the written item.
	 */
	std::vector<uint8_t>::iterator writeJump (std::vector<uint8_t>::iterator it, const Address &addr) const;

	/**
	 * Parse the item at \p it.
	 *
	 * If the instruction is a jump, the destination of the
	 * item is not set. Instead the destination item is
	 * written in \p jump_addr.
	 *
	 * \returns the position after the parsed item.
	 */
	virtual Macro::Item parseItem (std::vector<uint8_t>::const_iterator &it, Address &jump_addr) const = 0;
};

}

#endif
