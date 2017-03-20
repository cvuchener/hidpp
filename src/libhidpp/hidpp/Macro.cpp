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

#include "Macro.h"

#include <hidpp/AbstractMacroFormat.h>
#include <hidpp/AbstractMemoryMapping.h>

#include <misc/Log.h>

#include <cassert>
#include <map>
#include <vector>
#include <stack>
#include <iterator>

using namespace HIDPP;

const std::map<Macro::Item::Instruction, std::string> Macro::Item::InstructionStrings = {
	{ NoOp, "NoOp" },
	{ WaitRelease, "WaitRelease" },
	{ RepeatUntilRelease, "RepeatUntilRelease" },
	{ RepeatForever, "Repeat" },
	{ KeyPress, "KeyPress" },
	{ KeyRelease, "KeyRelease" },
	{ ModifiersPress, "ModifiersPress" },
	{ ModifiersRelease, "ModifiersRelease" },
	{ ModifiersKeyPress, "ModifiersKeyPress" },
	{ ModifiersKeyRelease, "ModifiersKeyRelease" },
	{ MouseWheel, "MouseWheel" },
	{ MouseHWheel, "MouseHWheel" },
	{ MouseButtonPress, "MouseButtonPress" },
	{ MouseButtonRelease, "MouseButtonRelease" },
	{ ConsumerControl, "ConsumerControl" },
	{ ConsumerControlPress, "ConsumerControlPress" },
	{ ConsumerControlRelease, "ConsumerControlRelease" },
	{ Delay, "Delay" },
	{ ShortDelay, "ShortDelay" },
	{ Jump, "Jump" },
	{ JumpIfPressed, "JumpIfPressed" },
	{ MousePointer, "MousePointer" },
	{ JumpIfReleased, "JumpIfReleased" },
	{ End, "End" },
};

Macro::Item::Item (Instruction instr):
	_instr (instr)
{
}

Macro::Item::Instruction Macro::Item::instruction () const
{
	return _instr;
}

uint8_t Macro::Item::keyCode () const
{
	return _params.key.key;
}

void Macro::Item::setKeyCode (uint8_t key)
{
	assert (_instr == KeyPress || _instr == KeyRelease || _instr == ModifiersKeyPress || _instr == ModifiersKeyRelease);
	_params.key.key = key;
}

uint8_t Macro::Item::modifiers () const
{
	return _params.key.modifiers;
}

void Macro::Item::setModifiers (uint8_t modifiers)
{
	assert (_instr == ModifiersPress || _instr == ModifiersRelease || _instr == ModifiersKeyPress || _instr == ModifiersKeyRelease);
	_params.key.modifiers = modifiers;
}

int Macro::Item::wheel () const
{
	return _params.wheel;
}

void Macro::Item::setWheel (int wheel)
{
	assert (_instr == MouseWheel || _instr == MouseHWheel);
	_params.wheel = wheel;
}

uint16_t Macro::Item::buttons () const
{
	return _params.buttons;
}

void Macro::Item::setButtons (uint16_t buttons)
{
	assert (_instr == MouseButtonPress || _instr == MouseButtonRelease);
	_params.buttons = buttons;
}

uint16_t Macro::Item::consumerControl () const
{
	return _params.cc;
}

void Macro::Item::setConsumerControl (uint16_t cc)
{
	assert (_instr == ConsumerControl || _instr == ConsumerControlPress || _instr == ConsumerControlRelease);
	_params.cc = cc;
}

unsigned int Macro::Item::delay () const
{
	return _params.delay;
}

void Macro::Item::setDelay (unsigned int delay)
{
	assert (_instr == Delay || _instr == ShortDelay || _instr == JumpIfReleased);
	_params.delay = delay;
}

bool Macro::Item::isJump () const
{
	return _instr == Jump || _instr == JumpIfPressed || _instr == JumpIfReleased;
}

std::list<Macro::Item>::iterator Macro::Item::jumpDestination ()
{
	return _dest;
}

std::list<Macro::Item>::const_iterator Macro::Item::jumpDestination () const
{
	return _dest;
}

void Macro::Item::setJumpDestination (std::list<Item>::iterator dest)
{
	_dest = dest;
}

int Macro::Item::mouseX () const
{
	return _params.mouse.x;
}

int Macro::Item::mouseY () const
{
	return _params.mouse.y;
}

void Macro::Item::setMouseX (int delta)
{
	assert (_instr == MousePointer);
	_params.mouse.x = delta;
}

void Macro::Item::setMouseY (int delta)
{
	assert (_instr == MousePointer);
	_params.mouse.y = delta;
}

bool Macro::Item::isSimple () const
{
	switch (_instr) {
	case NoOp:
	case KeyPress:
	case KeyRelease:
	case ModifiersPress:
	case ModifiersRelease:
	case ModifiersKeyPress:
	case ModifiersKeyRelease:
	case MouseWheel:
	case MouseHWheel:
	case MouseButtonPress:
	case MouseButtonRelease:
	case ConsumerControl:
	case ConsumerControlPress:
	case ConsumerControlRelease:
	case Delay:
	case ShortDelay:
	case MousePointer:
		return true;
	default:
		return false;
	}
}

bool Macro::Item::hasSuccessor () const
{
	switch (_instr) {
	case Jump:
	case End:
		return false;
	default:
		return true;
	}
}

Macro::Macro ()
{
}

Macro::Macro (const AbstractMacroFormat &format, AbstractMemoryMapping &mem, Address address)
{
	std::map<Address, iterator> parsed_items;
	std::vector<std::pair <Item *, Address>> incomplete_ref;

	std::vector<uint8_t>::const_iterator current = mem.getReadOnlyIterator (address);

	std::stack<Address> jump_dests;
	while (true) {
		Address dest;
		auto last = current;
		_items.emplace_back (format.parseItem (current, dest));
		Item &item = _items.back ();

		// Memorize aligned items by address
		if (mem.computeOffset (last, address)) {
			parsed_items.emplace (address, std::prev(_items.end ()));
		}

		if (item.isJump ()) {
			jump_dests.push (dest); // Keep destination for later parsing
			incomplete_ref.emplace_back (&item, dest);
		}

		if (!item.hasSuccessor ()) {
			// Find the first non parsed address
			do {
				if (jump_dests.empty ())
					goto parse_end;
				address = jump_dests.top ();
				jump_dests.pop ();
			} while (parsed_items.find (address) != parsed_items.end ());

			current = mem.getReadOnlyIterator (address);
		}
	}
parse_end:

	for (auto pair: incomplete_ref) {
		// Find item iterator at referenced address
		Item *item = pair.first;
		const Address &address = pair.second;
		item->setJumpDestination (parsed_items.at (address));
	}
}

Macro::Macro (const Macro &other):
	_items (other._items)
{
	auto debug = Log::debug ("macro");
	debug << "Copying macro" << std::endl;
	// The destination of every jump needs to be corrected in the copied list
	std::map<const Item *, iterator> translation;

	const_iterator other_it = other._items.begin ();
	for (iterator it = _items.begin ();
	     it != _items.end ();
	     ++it, ++other_it) {
		debug.printf ("Add translation %p to %p\n", &(*other_it), &(*it));
		translation.insert ({ &(*other_it), it });
	}

	for (Item &item: _items) {
		if (item.isJump ()) {
			Item *old_item = &(*item.jumpDestination ());
			iterator new_item = translation[old_item];
			debug.printf ("Replace %p with %p\n", old_item, &(*new_item));

			item.setJumpDestination (new_item);
		}
	}
}

Address Macro::write (const AbstractMacroFormat &format, AbstractMemoryMapping &mem, Address &start) const
{
	auto debug = Log::debug ("macro");
	typedef std::vector<uint8_t>::iterator iterator;
	std::map<const Item *, Address> jump_dests; // Associate address with jump destination items
	std::vector<std::pair<const Item *, iterator>> jump_addrs; // Jumps and their address positions

	for (auto item: _items) {
		if (item.isJump ()) {
			jump_dests.emplace(&*item.jumpDestination (), Address ());
		}
	}

	Address current_page = start;
	current_page.offset = 0;
	auto current = mem.getWritableIterator (start);
	auto page_end = mem.getWritablePage (start).end ();

	constexpr std::size_t CRCLength = 2;
	const std::size_t jump_len = format.getJumpLength ();
	bool check_end_of_page_jump = true;
	bool first_instruction = true;

	for (auto it = _items.begin (); it != _items.end (); ++it) {
		const Item &item = *it;
		auto jump_dest = jump_dests.find (&item);
		bool is_jump_dest = jump_dest != jump_dests.end ();

		std::size_t item_len = format.getLength (item);

		Address item_addr = current_page;

		if (check_end_of_page_jump) {
			auto instr_location = current;
			while (is_jump_dest && !mem.computeOffset (instr_location, item_addr)) {
				// The current index is not aligned and the item is
				// the destination of a jump. We need to add padding.
				++instr_location;
			}
			if ((int) (item_len + jump_len + CRCLength) > std::distance (instr_location, page_end)) {
				// If we write this item now, there will not be enough
				// room to write a jump. We check if the whole macro can
				// fit before the end of page.
				debug << "Check end of page jump at " << std::distance (instr_location, page_end) << " bytes from the end" << std::endl;
				bool need_jump = false;
				instr_location += item_len;
				for (auto it2 = std::next(it); it2 != _items.end (); ++it2) {
					while (!mem.computeOffset (instr_location, item_addr) && jump_dests.count (&*it2) > 0) {
						// Padding will be needed
						++instr_location;
					}
					instr_location += format.getLength (*it2);
					if ((int) CRCLength > std::distance (instr_location, page_end)) {
						// index reached end of page
						need_jump = true;
						break;
					}
				}
				if (need_jump) {
					// Jump to the beginning of the next page
					++current_page.page;
					if (!first_instruction) {
						assert (std::distance (current, page_end) > (int) (jump_len + CRCLength));
						debug << "Adding jump to page " << current_page.page << std::endl;
						format.writeJump (current, current_page);
					}
					current = mem.getWritableIterator (current_page);
					page_end = mem.getWritablePage (current_page).end ();
					if (first_instruction) {
						debug << "Macro start was moved because of lacking space at the given address" << std::endl;
						start = current_page;
					}
				}
				else {
					// The macro will fit in the current page, no need
					// to check again.
					debug << "Macro end fits in the current page" << std::endl;
					check_end_of_page_jump = false;
				}
			}
		}

		// Add padding if required
		while (is_jump_dest && !mem.computeOffset (current, item_addr)) {
			current = format.writeNoOp (current);
			debug << "Macro padding" << std::endl;
		}

		// Write the item itself
		std::vector<uint8_t>::iterator jump_addr_it;
		current = format.writeItem (current, item, jump_addr_it);
		debug.printBytes (Item::InstructionStrings.at (item.instruction ()), current-item_len, current);

		// Remember jump address position for later resolution
		if (item.isJump ()) {
			jump_addrs.emplace_back (&item, jump_addr_it);
		}

		// Remember item address for later jump resolution
		if (is_jump_dest) {
			jump_dest->second = item_addr;
		}

		if (first_instruction)
			first_instruction = false;
	}

	// Write jump addresses
	for (auto jump_addr: jump_addrs) {
		const Item &dest = *jump_addr.first->jumpDestination ();
		const Address &addr = jump_dests.at (&dest);
		auto jump_addr_it = jump_addr.second;
		format.writeAddress (jump_addr_it, addr);
	}

	// Return the next valid address
	Address next_addr = current_page;
	while (!mem.computeOffset (current, next_addr))
		++current;
	return next_addr;
}

void Macro::simplify ()
{
	auto debug = Log::debug ("macro");
	std::map<Item *, std::vector<Item *>> back_refs;

	for (auto item: _items) {
		if (item.isJump ()) {
			back_refs[&*item.jumpDestination ()].push_back (&item);
		}
	}


	auto it = _items.begin ();
	while (it != _items.end ()) {
		if (it->instruction () == Item::NoOp ||
		    (it->instruction () == Item::Jump && it->jumpDestination () == std::next(it))) {
			for (Item *jump: back_refs[&*it]) {
				jump->setJumpDestination (std::next(it));
			}
			debug.printf ("Remove useless macro item %p: instruction = %d\n", &*it, it->instruction ());
			it = _items.erase (it);
		}
		else
			++it;
	}
}

std::list<Macro::Item>::iterator Macro::begin ()
{
	return _items.begin ();
}

std::list<Macro::Item>::const_iterator Macro::begin () const
{
	return _items.begin ();
}

std::list<Macro::Item>::iterator Macro::end ()
{
	return _items.end ();
}

std::list<Macro::Item>::const_iterator Macro::end () const
{
	return _items.end ();
}

const Macro::Item &Macro::back () const
{
	return _items.back ();
}

Macro::Item &Macro::back ()
{
	return _items.back ();
}

void Macro::emplace_back (Item::Instruction instr)
{
	_items.emplace_back (instr);
}

bool Macro::isSimple () const
{
	for (auto it = _items.begin (); it != _items.end (); ++it) {
		if (!it->isSimple ()) {
			if (it->instruction () == Item::End)
				return std::next(it) == _items.end ();
			return false;
		}
	}
	return false;
}

bool Macro::isLoop (const_iterator &pre_begin, const_iterator &pre_end,
		    const_iterator &loop_begin, const_iterator &loop_end,
		    const_iterator &post_begin, const_iterator &post_end,
		    unsigned int &loop_delay) const
{
	enum State {
		Init,
		OptionalLoop,
		AfterLoop,
	} state = Init;

	pre_begin = _items.begin ();

	for (auto it = _items.begin (); it != _items.end (); ++it) {
		if (it->isSimple ())
			continue;

		switch (it->instruction ()) {
		case Item::RepeatUntilRelease:
			if (state != Init)
				return false;
			pre_end = pre_begin;
			loop_begin = pre_begin;
			loop_end = it;
			post_begin = std::next (it);
			loop_delay = 0;
			state = AfterLoop;
			break;

		case Item::JumpIfPressed: {
			const_iterator dest = it->jumpDestination ();
			if (state == Init) {
				// Check that the destination is before
				// the current instruction.
				const_iterator it2;
				for (it2 = _items.begin (); it2 != it; ++it2)
					if (it2 == dest)
						break;
				if (it2 == it) // dest not found
					return false;

				pre_end = dest;
				loop_begin = dest;
				loop_end = it;
				post_begin = std::next (it);
				loop_delay = 0;
				state = AfterLoop;
			}
			else if (state == OptionalLoop) {
				loop_end = it;
				post_begin = std::next (it);
				// Check jump destinations (pre_end is JumpIfReleased)
				if (pre_end->jumpDestination () != post_begin ||
				    dest != loop_begin)
					return false;
				state = AfterLoop;
			}
			else
				return false;
			break;
		}

		case Item::JumpIfReleased:
			if (state != Init)
				return false;
			pre_end = it;
			loop_begin = std::next (it);
			loop_delay = it->delay ();
			state = OptionalLoop;
			break;

		case Item::WaitRelease:
			if (state != Init)
				return false;
			pre_end = it;
			loop_begin = loop_end = it;
			post_begin = std::next (it);
			loop_delay = 0;
			state = AfterLoop;
			break;


		case Item::End:
			post_end = it;
			return state == AfterLoop;

		default:
			return false;
		}
	}
	return false;
}

Macro Macro::buildSimple (const_iterator begin, const_iterator end)
{
	Macro macro;
	for (auto it = begin; it != end; ++it) {
		if (!it->isSimple ())
			throw std::invalid_argument ("Macro item must be simple");
		macro._items.push_back (*it);
	}
	macro._items.emplace_back (Item::End);
	return macro;
}

Macro Macro::buildLoop (const_iterator pre_begin, const_iterator pre_end,
			const_iterator loop_begin, const_iterator loop_end,
			const_iterator post_begin, const_iterator post_end,
			unsigned int loop_delay)
{
	// Check inputs
	for (auto it = pre_begin; it != pre_end; ++it) {
		if (!it->isSimple ())
			throw std::invalid_argument ("Macro item must be simple");
	}
	for (auto it = loop_begin; it != loop_end; ++it) {
		if (!it->isSimple ())
			throw std::invalid_argument ("Macro item must be simple");
	}
	for (auto it = post_begin; it != post_end; ++it) {
		if (!it->isSimple ())
			throw std::invalid_argument ("Macro item must be simple");
	}

	Macro macro;
	if (loop_begin == loop_end) {
		// Inner loop is empty, use wait instruction
		macro._items.insert (macro._items.end (), pre_begin, pre_end);
		macro._items.emplace_back (Item::WaitRelease);
		macro._items.insert (macro._items.end (), post_begin, post_end);
		macro._items.emplace_back (Item::End);
	}
	else if (loop_delay > 0) {
		// Use JumpIfReleased to delay the loop
		macro._items.insert (macro._items.end (), pre_begin, pre_end);
		iterator released_jump = macro._items.emplace (macro._items.end (), Item::JumpIfReleased);
		iterator loop = macro._items.insert (macro._items.end (), loop_begin, loop_end);
		iterator pressed_jump = macro._items.emplace (macro._items.end (), Item::JumpIfPressed);
		iterator post = macro._items.insert (macro._items.end (), post_begin, post_end);
		macro._items.emplace_back (Item::End);

		released_jump->setDelay (loop_delay);
		released_jump->setJumpDestination (post);
		pressed_jump->setJumpDestination (loop);
	}
	else if (pre_begin == pre_end) {
		// No pre-loop instruction, use repeat instruction
		macro._items.insert (macro._items.end (), loop_begin, loop_end);
		macro._items.emplace_back (Item::RepeatUntilRelease);
		macro._items.insert (macro._items.end (), post_begin, post_end);
		macro._items.emplace_back (Item::End);
	}
	else {
		// Pre-loop is non-empty, and loop is played at least once
		// Use a single JumpIfpressed at the end of loop
		macro._items.insert (macro._items.end (), pre_begin, pre_end);
		iterator loop = macro._items.insert (macro._items.end (), loop_begin, loop_end);
		macro._items.emplace (macro._items.end (), Item::JumpIfPressed)->setJumpDestination (loop);
		macro._items.insert (macro._items.end (), post_begin, post_end);
		macro._items.emplace_back (Item::End);
	}
	return macro;
}
