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
using namespace HIDPP10;

static const std::map<Macro::Item::Instruction, uint8_t> OpCodes = {
	{ Macro::Item::NoOp, 0x00 },
	{ Macro::Item::WaitRelease, 0x01 },
	{ Macro::Item::RepeatUntilRelease, 0x02 },
	{ Macro::Item::RepeatForever, 0x03 },
	{ Macro::Item::KeyPress, 0x20 },
	{ Macro::Item::KeyRelease, 0x21 },
	{ Macro::Item::ModifiersPress, 0x22 },
	{ Macro::Item::ModifiersRelease, 0x23 },
	{ Macro::Item::MouseWheel, 0x24 },
	{ Macro::Item::MouseButtonPress, 0x40 },
	{ Macro::Item::MouseButtonRelease, 0x41 },
	{ Macro::Item::ConsumerControl, 0x42 },
	{ Macro::Item::Delay, 0x43 },
	{ Macro::Item::Jump, 0x44 },
	{ Macro::Item::JumpIfPressed, 0x45 },
	{ Macro::Item::MousePointer, 0x60 },
	{ Macro::Item::JumpIfReleased, 0x61 },
	{ Macro::Item::End, 0xff},
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

static uint8_t getShortDelayCode (unsigned int delay)
{
	if (delay < 8)
		return 0x80; // Minimum short delay of 8ms
	else if (delay < 132)
		return 0x80 + (delay - 8 + 2) / 4;
	else if (delay < 388)
		return 0x9F + (delay - 132 + 4) / 8;
	else if (delay < 900)
		return 0xBF + (delay - 388 + 8) / 16;
	else if (delay < 1892)
		return 0xDF + (delay - 900 + 16) / 32;
	else
		return 0xFE; // Maximum short delay of 1.892s
}

static unsigned int getShortDelayDuration (uint8_t op_code)
{
	if (op_code < 0x80)
		return 0; // Not a short delay op code
	else if (op_code <= 0x9F)
		return 8 + (op_code - 0x80) * 4;
	else if (op_code <= 0xBF)
		return 132 + (op_code - 0x9F) * 8;
	else if (op_code <= 0xDF)
		return 388 + (op_code - 0xBF) * 16;
	else if (op_code <= 0xFE)
		return 900 + (op_code - 0xDF) * 32;
	else
		return 0; // Not a short delay op code
}

std::size_t MacroFormat::getLength (const Macro::Item &item) const
{
	Macro::Item::Instruction instr = item.instruction ();
	if (instr == Macro::Item::ModifiersKeyPress || instr == Macro::Item::ModifiersKeyRelease) {
		if (item.modifiers () == 0 || item.keyCode () == 0)
			return 2; // Can be done with one Modifier/Key instruction.
		else
			return 4; // Requires two instructions.
	}

	if (instr == Macro::Item::ShortDelay)
		return 1;

	auto it = OpCodes.find (instr);
	if (it == OpCodes.end ())
		throw AbstractMacroFormat::UnsupportedInstruction (instr);
	return getOpLength (it->second);
}

void MacroFormat::writeAddress (std::vector<uint8_t>::iterator it, const Address &addr) const
{
	it[0] = addr.page;
	it[1] = addr.offset;
}

std::vector<uint8_t>::iterator
MacroFormat::splitModifiersKeyItem (Macro::Item::Instruction mod_instr,
				    Macro::Item::Instruction key_instr,
				    std::vector<uint8_t>::iterator it,
				    const Macro::Item &item) const
{
	std::vector<uint8_t>::iterator unused;
	uint16_t modifiers = item.modifiers ();
	uint8_t key_code = item.keyCode ();
	if (modifiers != 0) {
		Macro::Item mod (mod_instr);
		mod.setModifiers (modifiers);
		it = writeItem (it, mod, unused);
	}
	if (key_code != 0 || modifiers == 0) {
		Macro::Item key (key_instr);
		key.setKeyCode (key_code);
		it = writeItem (it, key, unused);
	}
	return it;
}

std::vector<uint8_t>::iterator
MacroFormat::writeItem (std::vector<uint8_t>::iterator it,
			const Macro::Item &item,
			std::vector<uint8_t>::iterator &jump_addr_it) const
{
	Macro::Item::Instruction instr = item.instruction ();

	// Emulate combined modifiers+key instruction with two instructions
	if (instr == Macro::Item::ModifiersKeyPress)
		return splitModifiersKeyItem (Macro::Item::ModifiersPress,
					      Macro::Item::KeyPress,
					      it, item);
	if (instr == Macro::Item::ModifiersKeyRelease)
		return splitModifiersKeyItem (Macro::Item::ModifiersRelease,
					      Macro::Item::KeyRelease,
					      it, item);

	if (instr == Macro::Item::ShortDelay) {
		*(it++) = getShortDelayCode (item.delay ());
		return it;
	}

	auto op = OpCodes.find (instr);
	if (op == OpCodes.end ())
		throw AbstractMacroFormat::UnsupportedInstruction (instr);
	*(it++) = op->second;
	switch (instr) {
	case Macro::Item::KeyPress:
	case Macro::Item::KeyRelease:
		*(it++) = item.keyCode ();
		return it;
	case Macro::Item::ModifiersPress:
	case Macro::Item::ModifiersRelease:
		*(it++) = item.modifiers ();
		return it;
	case Macro::Item::MouseWheel:
		*(it++) = static_cast<int8_t> (item.wheel ());
		return it;
	case Macro::Item::MouseButtonPress:
	case Macro::Item::MouseButtonRelease:
		return writeLE<uint16_t> (it, item.buttons ());
	case Macro::Item::ConsumerControl:
		return writeBE<uint16_t> (it, item.consumerControl ());
	case Macro::Item::Delay:
		return writeBE<uint16_t> (it, item.delay ());
	case Macro::Item::Jump:
	case Macro::Item::JumpIfPressed:
		jump_addr_it = it;
		return it+2;
	case Macro::Item::MousePointer:
		it = writeBE<int16_t> (it, item.mouseX ());
		it = writeBE<int16_t> (it, item.mouseY ());
		return it;
	case Macro::Item::JumpIfReleased:
		it = writeBE<uint16_t> (it, item.delay ());
		jump_addr_it = it;
		return it+2;
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
		case Macro::Item::KeyPress:
		case Macro::Item::KeyRelease:
			item.setKeyCode (*(it++));
			break;
		case Macro::Item::ModifiersPress:
		case Macro::Item::ModifiersRelease:
			item.setModifiers (*(it++));
			break;
		case Macro::Item::MouseWheel:
			item.setWheel (static_cast<int8_t> (*(it++)));
			break;
		case Macro::Item::MouseButtonPress:
		case Macro::Item::MouseButtonRelease:
			item.setButtons (readLE<uint16_t> (it));
			it += 2;
			break;
		case Macro::Item::ConsumerControl:
			item.setConsumerControl (readBE<uint16_t> (it));
			it += 2;
			break;
		case Macro::Item::Delay:
			item.setDelay (readBE<uint16_t> (it));
			it += 2;
			break;
		case Macro::Item::Jump:
		case Macro::Item::JumpIfPressed:
			jump_addr.mem_type = 0;
			jump_addr.page = *(it++);
			jump_addr.offset = *(it++);
			break;
		case Macro::Item::MousePointer:
			item.setMouseX (readBE<int16_t> (it));
			it += 2;
			item.setMouseY (readBE<int16_t> (it));
			it += 2;
			break;
		case Macro::Item::JumpIfReleased:
			item.setDelay (readBE<uint16_t> (it));
			it += 2;
			jump_addr.mem_type = 0;
			jump_addr.page = *(it++);
			jump_addr.offset = *(it++);
			break;
		default:
			break;
		}
		return item;
	}
	unsigned int delay = getShortDelayDuration (op_code);
	if (delay != 0) {
		Macro::Item item (Macro::Item::ShortDelay);
		item.setDelay (delay);
		return item;
	}
	Log::error ().printf ("Invalid op-code: %02hhx\n", op_code);
	throw std::runtime_error ("Invalid op-code in HID++1.0 macro");
}

std::unique_ptr<AbstractMacroFormat> HIDPP10::getMacroFormat (Device *device)
{
	return std::unique_ptr<AbstractMacroFormat> (new MacroFormat ());
}
