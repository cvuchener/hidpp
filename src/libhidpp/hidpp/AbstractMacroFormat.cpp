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

#include "AbstractMacroFormat.h"

using namespace HIDPP;

AbstractMacroFormat::UnsupportedInstruction::UnsupportedInstruction (Macro::Item::Instruction instr):
	_instr (instr),
	_msg ("Unsupported Instruction: ")
{
	auto it = Macro::Item::InstructionStrings.find (instr);
	if (it == Macro::Item::InstructionStrings.end ())
		_msg += "unknown";
	else
		_msg += it->second;
}

const char *AbstractMacroFormat::UnsupportedInstruction::what () const noexcept
{
	return _msg.c_str ();
}

Macro::Item::Instruction AbstractMacroFormat::UnsupportedInstruction::instruction () const
{
	return _instr;
}

std::size_t AbstractMacroFormat::getJumpLength () const
{
	return getLength (Macro::Item::Jump);
}

std::vector<uint8_t>::iterator AbstractMacroFormat::writeNoOp (std::vector<uint8_t>::iterator it) const
{
	std::vector<uint8_t>::iterator addr_it;
	return writeItem (it, Macro::Item::NoOp, addr_it);
}

std::vector<uint8_t>::iterator AbstractMacroFormat::writeJump (std::vector<uint8_t>::iterator it, const Address &addr) const
{
	std::vector<uint8_t>::iterator addr_it;
	auto ret = writeItem (it, Macro::Item::Jump, addr_it);
	writeAddress (addr_it, addr);
	return ret;
}

