/*
 * Copyright 2016 Clément Vuchener
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

#include "MemoryMapping.h"

#include <misc/Endian.h>
#include <misc/CRC.h>
#include <misc/Log.h>

using namespace HIDPP::Base;

MemoryMapping::MemoryMapping (bool write_crc):
	_write_crc (write_crc)
{
}

const std::vector<uint8_t> &MemoryMapping::getReadOnlyPage (const Address &address)
{
	return getPage (address).data;
}

std::vector<uint8_t> &MemoryMapping::getWritablePage (const Address &address)
{
	auto &page = getPage (address);
	page.modified = true;
	return page.data;
}

void MemoryMapping::sync ()
{
	for (auto &p: _pages) {
		auto &address = p.first;
		auto &page = p.second;
		if (page.modified) {
			if (_write_crc) {
				uint16_t crc = CRC::CCITT (page.data.begin (),
							   page.data.end () - sizeof (crc));
				writeBE (page.data.end () - sizeof (crc), crc);
			}
			writePage (address, page.data);
			page.modified = false;
		}
	}
}

MemoryMapping::Page &MemoryMapping::getPage (Address address)
{
	address.offset = 0;
	auto it = _pages.find (address);
	if (it == _pages.end ()) {
		it = _pages.emplace (address, Page { false }).first;
		readPage (address, it->second.data);
	}
	return it->second;
}

