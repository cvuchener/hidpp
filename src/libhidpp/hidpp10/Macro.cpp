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

#include <hidpp10/Macro.h>

#include <misc/Log.h>
#include <misc/Endian.h>
#include <hidpp10/defs.h>

#include <map>
#include <vector>
#include <stack>
#include <stdexcept>
#include <iterator>

using namespace HIDPP10;

Macro::Item::Item (uint8_t op_code):
	_op_code (op_code)
{
}

std::size_t Macro::Item::getOpLength (uint8_t op_code)
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

uint8_t Macro::Item::getShortDelayCode (unsigned int delay)
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

unsigned int Macro::Item::getShortDelayDuration (uint8_t op_code)
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

uint8_t Macro::Item::opCode () const
{
	return _op_code;
}

bool Macro::Item::isShortDelay () const
{
	return _op_code >= 0x80 && _op_code <= 0xFE;
}

uint8_t Macro::Item::keyCode () const
{
	return _params.key;
}

void Macro::Item::setKeyCode (uint8_t key)
{
	if (_op_code == KeyPress || _op_code == KeyRelease)
		_params.key = key;
}

uint8_t Macro::Item::modifiers () const
{
	return _params.modifiers;
}

void Macro::Item::setModifiers (uint8_t modifiers)
{
	if (_op_code == ModifierPress || _op_code == ModifierRelease)
		_params.modifiers = modifiers;
}

int8_t Macro::Item::wheel () const
{
	return _params.wheel;
}

void Macro::Item::setWheel (int8_t wheel)
{
	if (_op_code == MouseWheel)
		_params.wheel = wheel;
}

uint16_t Macro::Item::buttons () const
{
	return _params.buttons;
}

void Macro::Item::setButtons (uint16_t buttons)
{
	if (_op_code == MouseButtonPress || _op_code == MouseButtonRelease)
		_params.buttons = buttons;
}

uint16_t Macro::Item::consumerControl () const
{
	return _params.cc;
}

void Macro::Item::setConsumerControl (uint16_t cc)
{
	if (_op_code == ConsumerControl)
		_params.cc = cc;
}

unsigned int Macro::Item::delay () const
{
	if (_op_code >= 0x80 && _op_code <= 0xFE) // Short delay
		return getShortDelayDuration (_op_code);
	return _params.delay;
}

void Macro::Item::setDelay (unsigned int delay)
{
	if (_op_code == Delay || _op_code == JumpIfReleased)
		_params.delay = delay;
	else if (_op_code >= 0x80 && _op_code <= 0xFE) // Short delay
		_op_code = getShortDelayCode (delay);
}

bool Macro::Item::isJump () const
{
	return _op_code == Jump ||
		_op_code == JumpIfPressed ||
		_op_code == JumpIfReleased;
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
	if (_op_code == Jump ||
	    _op_code == JumpIfPressed ||
	    _op_code == JumpIfReleased)
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
	if (_op_code == MousePointer)
		_params.mouse.x = delta;
}

void Macro::Item::setMouseY (int delta)
{
	if (_op_code == MousePointer)
		_params.mouse.y = delta;
}

bool Macro::Item::isSimple () const
{
	if (_op_code >= 0x80 && _op_code <= 0xfe) // Short delay
		return true;
	switch (_op_code) {
	case NoOp:
	case KeyPress:
	case KeyRelease:
	case ModifierPress:
	case ModifierRelease:
	case MouseWheel:
	case MouseButtonPress:
	case MouseButtonRelease:
	case ConsumerControl:
	case Delay:
	case MousePointer:
		return true;
	default:
		return false;
	}
}

Macro::Macro ()
{
}

static Macro::Item parseItem (std::vector<uint8_t>::const_iterator begin,
			      Address &jump_dest)
{
	Macro::Item item (*begin);

	switch (*begin) {
	case Macro::Item::KeyPress:
	case Macro::Item::KeyRelease:
		item.setKeyCode (*(begin+1));
		return item;

	case Macro::Item::ModifierPress:
	case Macro::Item::ModifierRelease:
		item.setModifiers (*(begin+1));
		return item;

	case Macro::Item::MouseWheel:
		item.setWheel (*reinterpret_cast<const int8_t *> (&*(begin+1)));
		return item;

	case Macro::Item::MouseButtonPress:
	case Macro::Item::MouseButtonRelease:
		item.setButtons (readLE<uint16_t> (begin+1));
		return item;

	case Macro::Item::ConsumerControl:
		item.setConsumerControl (readBE<uint16_t> (begin+1));
		return item;

	case Macro::Item::Delay:
		item.setDelay (readBE<uint16_t> (begin+1));
		return item;

	case Macro::Item::Jump:
	case Macro::Item::JumpIfPressed:
		jump_dest.page = *(begin+1);
		jump_dest.offset = *(begin+2);
		return item;

	case Macro::Item::MousePointer:
		item.setMouseX (readBE<int16_t> (begin+1));
		item.setMouseY (readBE<int16_t> (begin+3));
		return item;

	case Macro::Item::JumpIfReleased:
		item.setDelay (readBE<uint16_t> (begin+1));
		jump_dest.page = *(begin+3);
		jump_dest.offset = *(begin+4);
		return item;

	default:
		return item;
	}
}

Macro::Macro (MemoryMapping &mem, Address address)
{
	Log::printf (Log::Debug,
		     "Reading macro (%p) in persistent memory at address %02hhx:%02hhx\n",
		     this,
		     address.page, address.offset);
	std::map<Address, iterator> parsed_items;
	std::vector<std::pair <Item *, Address>> incomplete_ref;

	unsigned int current_page = address.page;
	unsigned int current_index = 2*address.offset;

	std::stack<Address> jump_dests;
	while (true) {
		const std::vector<uint8_t> &data = mem.getReadOnlyPage (current_page);
		Address dest;
		_items.push_back (parseItem (data.begin () + current_index, dest));
		Item *item = &_items.back ();
		Log::printf (Log::Debug,
			     "Parsed macro item %p at page %02hhx, index %03x, op_code is %02hhx\n",
			     item,
			     current_page, current_index,
			     item->opCode ());

		// Memorize aligned items by address
		if (current_index % 2 == 0) {
			Address addr = {
				static_cast<uint8_t> (current_page),
				static_cast<uint8_t> (current_index/2)
			};
			Log::printf (Log::Debug,
				     "Macro item %p is aligned at %02hhx:%02hhx\n",
				     item,
				     addr.page, addr.offset);
			parsed_items.insert ({
				addr,
				std::prev(_items.end ())
			});
		}

		if (item->isJump ()) {
			jump_dests.push (dest); // Keep destination for later parsing
			incomplete_ref.emplace_back (item, dest);
		}

		if (item->opCode () == Item::End || item->opCode () == Item::Jump) {
			// Current item has no successor

			// Find the first non parsed address
			do {
				if (jump_dests.empty ())
					goto parse_end;
				dest = jump_dests.top ();
				jump_dests.pop ();
			} while (parsed_items.find (dest) != parsed_items.end ());

			current_page = dest.page;
			current_index = 2*dest.offset;
		}
		else {
			current_index += Item::getOpLength (item->opCode ());
		}
	}
parse_end:

	for (auto pair: incomplete_ref) {
		// Find item iterator at referenced address
		Item *item = pair.first;
		Address &address = pair.second;
		auto it = parsed_items[address];
		Log::printf (Log::Debug,
			     "Macro item %p references %02hhx:%02hhx: %p\n",
			     item,
			     address.page, address.offset,
			     &(*it));
		item->setJumpDestination (it);
	}
}

Macro::Macro (std::vector<uint8_t>::const_iterator begin, Address start_address)
{
	std::map<Address, iterator> parsed_items;
	std::vector<std::pair <Item *, Address>> incomplete_ref;

	unsigned int current_page = start_address.page;
	std::vector<uint8_t>::const_iterator current = begin;

	std::stack<Address> jump_dests;
	while (true) {
		Address dest;
		_items.push_back (parseItem (current, dest));
		Item *item = &_items.back ();

		// Memorize aligned items by address
		if ((current-begin) % 2 == 0)
			parsed_items.insert ({
				Address ({
					static_cast<uint8_t> (current_page),
					static_cast<uint8_t> (start_address.offset +
							(current-begin)/2)
				}),
				std::prev (_items.end ())
			});

		if (item->isJump ()) {
			if (dest.page != start_address.page)
				throw std::runtime_error ("Cannot parse macro referencing other pages");
			jump_dests.push (dest); // Keep destination for later parsing
			incomplete_ref.emplace_back (item, dest);
		}

		if (item->opCode () == Item::End || item->opCode () == Item::Jump) {
			// Current item has no successor

			// Find the first non parsed address
			do {
				if (jump_dests.empty ())
					goto parse_end;
				dest = jump_dests.top ();
				jump_dests.pop ();
			} while (parsed_items.find (dest) != parsed_items.end ());

			current = begin + 2*(dest.offset - start_address.offset);
		}
		else {
			current += Item::getOpLength (item->opCode ());
		}
	}
parse_end:

	for (auto pair: incomplete_ref) {
		// Find item iterator at referenced address
		Item *item = pair.first;
		Address &address = pair.second;
		auto it = parsed_items[address];
		Log::printf (Log::Debug,
			     "Macro item %p references %02hhx:%02hhx: %p\n",
			     item,
			     address.page, address.offset,
			     &(*it));
		item->setJumpDestination (it);
	}
}

Macro::Macro (const Macro &other):
	_items (other._items)
{
	Log::debug () << "Copying macro" << std::endl;
	// The destination of every jump needs to be corrected in the copied list
	std::map<const Item *, iterator> translation;

	const_iterator other_it = other._items.begin ();
	for (iterator it = _items.begin ();
	     it != _items.end ();
	     ++it, ++other_it) {
		Log::printf (Log::Debug,
			     "Add translation %p to %p\n",
			     &(*other_it), &(*it));

		translation.insert ({ &(*other_it), it });
	}

	for (Item &item: _items) {
		if (item.isJump ()) {
			Item *old_item = &(*item.jumpDestination ());
			iterator new_item = translation[old_item];
			Log::printf (Log::Debug,
				     "Replace %p with %p\n",
				     old_item, &(*new_item));

			item.setJumpDestination (new_item);
		}
	}
}

static void writeItem (const Macro::Item &item,
		       std::vector<uint8_t>::iterator begin,
		       std::vector<uint8_t>::iterator *jump_addr = nullptr)
{
	uint8_t op_code = item.opCode ();

	*begin = op_code;

	switch (op_code) {
	case Macro::Item::KeyPress:
	case Macro::Item::KeyRelease:
		*(begin+1) = item.keyCode ();
		return;

	case Macro::Item::ModifierPress:
	case Macro::Item::ModifierRelease:
		*(begin+1) = item.modifiers ();
		return;

	case Macro::Item::MouseWheel:
		*reinterpret_cast<int8_t *> (&*(begin+1)) = item.wheel ();
		return;

	case Macro::Item::MouseButtonPress:
	case Macro::Item::MouseButtonRelease:
		writeLE<uint16_t> (begin+1, item.buttons ());
		return;

	case Macro::Item::ConsumerControl:
		writeBE<uint16_t> (begin+1, item.consumerControl ());
		return;

	case Macro::Item::Delay:
		writeBE<uint16_t> (begin+1, item.delay ());
		return;

	case Macro::Item::Jump:
	case Macro::Item::JumpIfPressed:
		*jump_addr = begin+1;
		return;

	case Macro::Item::MousePointer:
		writeBE<int16_t> (begin+1, item.mouseX ());
		writeBE<int16_t> (begin+3, item.mouseY ());
		return;

	case Macro::Item::JumpIfReleased:
		writeBE<uint16_t> (begin+1, item.delay ());
		*jump_addr = begin+3;
		return;

	default:
		return;
	}
}

std::vector<uint8_t>::iterator
Macro::write (std::vector<uint8_t>::iterator begin, Address start_address) const
{
	typedef std::vector<uint8_t>::iterator iterator;
	std::map<const Item *, Address> jump_dests; // Associate address with jump destination items
	std::vector<std::pair<const Item *, iterator>> jump_addrs; // Jumps and their address positions

	for (auto item: _items) {
		if (item.isJump ()) {
			jump_dests.emplace(&*item.jumpDestination (), Address ());
		}
	}

	iterator current = begin;

	for (auto it = _items.begin (); it != _items.end (); ++it) {
		const Item &item = *it;

		auto jump_dest = jump_dests.find (&item);
		bool is_jump_dest = jump_dest != jump_dests.end ();

		// Add padding if required
		if (is_jump_dest && (current-begin)%2 == 1) {
			Log::printf (Log::Debug,
				     "Write padding at index %03x\n",
				     static_cast<unsigned int> (current-begin));
			writeItem (Item (Item::NoOp), current);
			++current;
		}
		if (is_jump_dest)
			Log::printf (Log::Debug,
				     "Macro item %p is aligned at %02hhx:%02hhx\n",
				     &item,
				     start_address.page, start_address.offset + static_cast<uint8_t> ((current-begin)/2));

		// Write the item itself
		Log::printf (Log::Debug,
			     "Write macro item %p index %03x, op_code is %02hhx\n",
			     &item,
			     static_cast<unsigned int> (current-begin),
			     item.opCode ());
		iterator addr;
		writeItem (item, current, &addr);

		// Remember jump address position for later resolution
		if (item.isJump ()) {
			jump_addrs.emplace_back (&item, addr);
		}

		// Remember item address for later jump resolution
		if (is_jump_dest) {
			jump_dest->second = Address {
				static_cast<uint8_t> (start_address.page),
				static_cast<uint8_t> (start_address.offset + (current-begin)/2)
			};
		}

		current += Item::getOpLength (item.opCode ());
	}

	// Write jump addresses
	for (auto jump_addr: jump_addrs) {
		const Item *dest = &*jump_addr.first->jumpDestination ();
		Address addr = jump_dests[dest];
		iterator addr_pos = jump_addr.second;
		Log::printf (Log::Debug,
			     "Macro item %p jump to %02hhx:%02hhx\n",
			     jump_addr.first,
			     addr.page, addr.offset);
		*addr_pos = addr.page;
		*(addr_pos+1) = addr.offset;
	}

	return current + (current-begin)%2;
}

Address Macro::write (MemoryMapping &mem, Address start) const
{
	typedef std::vector<uint8_t>::iterator iterator;
	std::vector<uint8_t> *page_data;
	std::map<const Item *, Address> jump_dests; // Associate address with jump destination items
	std::vector<std::pair<const Item *, iterator>> jump_addrs; // Jumps and their address positions

	for (auto item: _items) {
		if (item.isJump ()) {
			jump_dests.emplace(&*item.jumpDestination (), Address ());
		}
	}

	unsigned int current_page = start.page;
	unsigned int current_index = start.offset*2;

	constexpr std::size_t CRCSize = 2;
	static const std::size_t JumpSize = Item::getOpLength (Item::Jump);
	bool check_end_of_page_jump = true;

	for (auto it = _items.begin (); it != _items.end (); ++it) {
		const Item &item = *it;

		auto jump_dest = jump_dests.find (&item);
		bool is_jump_dest = jump_dest != jump_dests.end ();

		unsigned int item_size = Item::getOpLength (item.opCode ());

		if (check_end_of_page_jump) {
			if (is_jump_dest && current_index%2 == 1) {
				// The current index is not aligned and the item is
				// the destination of a jump. We need one byte of padding.
				item_size++;
			}
			if (current_index + item_size > PageSize-CRCSize-JumpSize) {
				// If we write this item now, there will not be enough
				// room to write a jump. We check if the whole macro can
				// fit before the end of page.
				bool need_jump = false;
				unsigned int index = current_index + item_size;
				for (auto it2 = std::next(it); it2 != _items.end (); ++it2) {
					if (index%2 == 1 && jump_dests.find (&*it2) != jump_dests.end ()) {
						// Padding will be needed
						++index;
					}
					index += Item::getOpLength (it2->opCode ());
					if (index >= PageSize - CRCSize) {
						// index reached end of page
						need_jump = true;
						break;
					}
				}
				if (need_jump) {
					// Jump to the beginning of the next page
					Log::printf (Log::Debug,
						     "Write jump over end of page %02x at index %03x\n",
						     current_page, current_index);
					page_data = &mem.getWritablePage (current_page);
					iterator addr;
					writeItem (Item (Item::Jump), page_data->begin () + current_index, &addr);
					*addr = ++current_page;
					*(addr+1) = current_index = 0;
				}
				else {
					// The macro will fit in the current page, no need
					// to check again.
					check_end_of_page_jump = false;
				}
			}
		}

		page_data = &mem.getWritablePage (current_page);

		// Add padding if required
		if (is_jump_dest && current_index%2 == 1) {
			Log::printf (Log::Debug,
				     "Write padding at page %02x, index %03x\n",
				     current_page, current_index);
			writeItem (Item (Item::NoOp), page_data->begin () + current_index);
			++current_index;
		}
		if (is_jump_dest)
			Log::printf (Log::Debug,
				     "Macro item %p is aligned at %02hhx:%02hhx\n",
				     &item,
				     current_page, current_index/2);

		// Write the item itself
		Log::printf (Log::Debug,
			     "Write macro item %p at page %02x, index %03x, op_code is %02hhx\n",
			     &item,
			     current_page, current_index,
			     item.opCode ());
		iterator addr;
		writeItem (item, page_data->begin () + current_index, &addr);

		// Remember jump address position for later resolution
		if (item.isJump ()) {
			jump_addrs.emplace_back (&item, addr);
		}

		// Remember item address for later jump resolution
		if (is_jump_dest) {
			jump_dest->second = Address {
				static_cast<uint8_t> (current_page),
				static_cast<uint8_t> (current_index/2)
			};
		}

		current_index += Item::getOpLength (item.opCode ());
	}

	// Write jump addresses
	for (auto jump_addr: jump_addrs) {
		const Item *dest = &*jump_addr.first->jumpDestination ();
		Address addr = jump_dests[dest];
		iterator addr_pos = jump_addr.second;
		Log::printf (Log::Debug,
			     "Macro item %p jump to %02hhx:%02hhx\n",
			     jump_addr.first,
			     addr.page, addr.offset);
		*addr_pos = addr.page;
		*(addr_pos+1) = addr.offset;
	}

	return Address {
		static_cast<uint8_t> (current_page),
		static_cast<uint8_t> ((current_index+1)/2)
	};
}

void Macro::simplify ()
{
	std::map<Item *, std::vector<Item *>> back_refs;

	for (auto item: _items) {
		if (item.isJump ()) {
			back_refs[&*item.jumpDestination ()].push_back (&item);
		}
	}


	auto it = _items.begin ();
	while (it != _items.end ()) {
		if (it->opCode () == Item::NoOp ||
		    (it->opCode () == Item::Jump && it->jumpDestination () == std::next(it))) {
			for (Item *jump: back_refs[&*it]) {
				jump->setJumpDestination (std::next(it));
			}
			Log::printf (Log::Debug,
				     "Remove useless macro item %p: op_code = %02hhx\n",
				     &*it, it->opCode ());
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

void Macro::emplace_back (uint8_t op_code)
{
	_items.emplace_back (op_code);
}

bool Macro::isSimple () const
{
	for (auto it = _items.begin (); it != _items.end (); ++it) {
		if (!it->isSimple ()) {
			if (it->opCode () == Item::End)
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

		switch (it->opCode ()) {
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
