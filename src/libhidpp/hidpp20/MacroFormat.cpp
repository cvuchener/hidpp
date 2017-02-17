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

#include "MacroFormat.h"

#include <misc/Endian.h>
#include <misc/Log.h>

using namespace HIDPP;
using namespace HIDPP20;

static const std::map<Macro::Item::Instruction, uint8_t> OpCodes = {
	{ Macro::Item::NoOp, 0x00 },
	{ Macro::Item::WaitRelease, 0x01 },
	{ Macro::Item::RepeatUntilRelease, 0x02 },
	{ Macro::Item::RepeatForever, 0x03 },
	{ Macro::Item::MouseWheel, 0x20 },
	{ Macro::Item::MouseHWheel, 0x21 },
	{ Macro::Item::Delay, 0x40 },
	{ Macro::Item::MouseButtonPress, 0x41 },
	{ Macro::Item::MouseButtonRelease, 0x42 },
	{ Macro::Item::ModifiersKeyPress, 0x43 },
	{ Macro::Item::ModifiersKeyRelease, 0x44 },
	{ Macro::Item::ConsumerControlPress, 0x45 },
	{ Macro::Item::ConsumerControlRelease, 0x46 },
	{ Macro::Item::Jump, 0x60 },
	{ Macro::Item::MousePointer, 0x61 },
	{ Macro::Item::End, 0xff },
};

static std::size_t getOpLength (uint8_t op_code)
{
	switch (op_code & 0xE0) {
	case 0x00:
		return 1;
	case 0x20:
		return 2;
	case 0x40:
		return 3;
	case 0x60:
		return 5;
	default:
		return 1;
	}
}

std::size_t MacroFormat::getLength (const Macro::Item &item) const
{
	Macro::Item::Instruction instr = item.instruction ();
	if (instr == Macro::Item::ModifiersPress ||
	    instr == Macro::Item::ModifiersRelease ||
	    instr == Macro::Item::KeyPress ||
	    instr == Macro::Item::KeyRelease) {
		instr = Macro::Item::ModifiersKeyPress;
	}
	auto it = OpCodes.find (instr);
	if (it == OpCodes.end ())
		throw AbstractMacroFormat::UnsupportedInstruction (instr);
	return getOpLength (it->second);
}

void MacroFormat::writeAddress (std::vector<uint8_t>::iterator it, const Address &addr) const
{
	*(it++) = addr.mem_type;
	*(it++) = addr.page;
	writeBE<uint16_t> (it, addr.offset);
}

std::vector<uint8_t>::iterator
MacroFormat::writeItem (std::vector<uint8_t>::iterator it,
			const Macro::Item &item,
			std::vector<uint8_t>::iterator &jump_addr_it) const
{
	Macro::Item::Instruction instr = item.instruction ();

	if (instr == Macro::Item::ModifiersPress ||
	    instr == Macro::Item::ModifiersRelease ||
	    instr == Macro::Item::KeyPress ||
	    instr == Macro::Item::KeyRelease) {
		Macro::Item::Instruction new_instr;
		if (instr == Macro::Item::ModifiersPress ||
		    instr == Macro::Item::KeyPress) {
			new_instr = Macro::Item::ModifiersKeyPress;
		}
		if (instr == Macro::Item::ModifiersRelease ||
		    instr == Macro::Item::KeyRelease) {
			new_instr = Macro::Item::ModifiersKeyRelease;
		}
		Macro::Item new_item (new_instr);
		if (instr == Macro::Item::ModifiersPress ||
		    instr == Macro::Item::ModifiersRelease) {
			new_item.setModifiers (item.modifiers ());
			new_item.setKeyCode (0);
		}
		if (instr == Macro::Item::KeyPress ||
		    instr == Macro::Item::KeyRelease) {
			new_item.setModifiers (0);
			new_item.setKeyCode (item.keyCode ());
		}
		return writeItem (it, new_item, jump_addr_it);
	}

	auto op = OpCodes.find (instr);
	if (op == OpCodes.end ())
		throw AbstractMacroFormat::UnsupportedInstruction (instr);
	*(it++) = op->second;
	switch (instr) {
	case Macro::Item::MouseWheel:
	case Macro::Item::MouseHWheel:
		*(it++) = static_cast<int8_t> (item.wheel ());
		return it;
	case Macro::Item::Delay:
		it = writeBE<uint16_t> (it, item.delay ());
		return it;
	case Macro::Item::MouseButtonPress:
	case Macro::Item::MouseButtonRelease:
		it = writeBE<uint16_t> (it, item.buttons ());
		return it;
	case Macro::Item::ModifiersKeyPress:
	case Macro::Item::ModifiersKeyRelease:
		*(it++) = item.modifiers ();
		*(it++) = item.keyCode ();
		return it;
	case Macro::Item::ConsumerControlPress:
	case Macro::Item::ConsumerControlRelease:
		it = writeBE<uint16_t> (it, item.consumerControl ());
		return it;
	case Macro::Item::Jump:
		jump_addr_it = it;
		return it+4;
	case Macro::Item::MousePointer:
		it = writeBE<int16_t> (it, item.mouseX ());
		it = writeBE<int16_t> (it, item.mouseY ());
		return it;
	default:
		return it;
	}

}


Macro::Item MacroFormat::parseItem (std::vector<uint8_t>::const_iterator &it, Address &jump_addr) const
{
	static std::map<uint8_t, Macro::Item::Instruction> instr_map; // reversed OpCodes
	if (instr_map.empty ()) {
		for (const auto &p: OpCodes)
			instr_map.emplace (p.second, p.first);
	}

	uint8_t op_code = *(it++);
	auto instr = instr_map.find (op_code);
	if (instr != instr_map.end ()) {
		Macro::Item item (instr->second);
		switch (instr->second) {
		case Macro::Item::MouseWheel:
		case Macro::Item::MouseHWheel:
			item.setWheel (static_cast<int8_t> (*(it++)));
			break;
		case Macro::Item::Delay:
			item.setDelay (readBE<uint16_t> (it));
			it += 2;
			break;
		case Macro::Item::MouseButtonPress:
		case Macro::Item::MouseButtonRelease:
			item.setButtons (readBE<uint16_t> (it));
			it += 2;
			break;
		case Macro::Item::ModifiersKeyPress:
		case Macro::Item::ModifiersKeyRelease:
			item.setModifiers (*(it++));
			item.setKeyCode (*(it++));
			break;
		case Macro::Item::ConsumerControlPress:
		case Macro::Item::ConsumerControlRelease:
			item.setConsumerControl (readBE<uint16_t> (it));
			it += 2;
			break;
		case Macro::Item::Jump:
			jump_addr.mem_type = *(it++);
			jump_addr.page = *(it++);
			jump_addr.offset = readBE<uint16_t> (it);
			it += 2;
			break;
		case Macro::Item::MousePointer:
			item.setMouseX (readBE<int16_t> (it));
			it += 2;
			item.setMouseY (readBE<int16_t> (it));
			it += 2;
			break;
		default:
			break;
		}
		return item;
	}
	Log::error ().printf ("Invalid op-code: %02hhx\n", op_code);
	throw std::runtime_error ("Invalid op-code in HID++2.0 macro");
}

std::unique_ptr<AbstractMacroFormat> HIDPP20::getMacroFormat (Device *device)
{
	return std::unique_ptr<AbstractMacroFormat> (new MacroFormat ());
}
