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

#ifndef HIDPP_ABSTRACT_MACRO_FORMAT_H
#define HIDPP_ABSTRACT_MACRO_FORMAT_H

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
 * NoOp is used for padding and is expected to be exactly one byte long
 */
class AbstractMacroFormat
{
public:
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

	virtual std::size_t getLength (const Macro::Item &item) const = 0;

	virtual std::size_t getJumpLength () const;

	virtual void writeAddress (std::vector<uint8_t>::iterator it,
				   const Address &addr) const = 0;

	virtual std::vector<uint8_t>::iterator
	writeItem (std::vector<uint8_t>::iterator it,
		   const Macro::Item &item,
		   std::vector<uint8_t>::iterator &jump_addr_it) const = 0;

	std::vector<uint8_t>::iterator writeNoOp (std::vector<uint8_t>::iterator it) const;
	std::vector<uint8_t>::iterator writeJump (std::vector<uint8_t>::iterator it, const Address &addr) const;

	virtual Macro::Item parseItem (std::vector<uint8_t>::const_iterator &it, Address &jump_addr) const = 0;
};

}

#endif
