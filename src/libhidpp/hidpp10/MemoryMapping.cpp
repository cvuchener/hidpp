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

#include <hidpp10/defs.h>
#include <misc/Log.h>

using namespace HIDPP;
using namespace HIDPP10;

MemoryMapping::MemoryMapping (Device *dev, bool write_crc):
	AbstractMemoryMapping (write_crc),
	_imem (dev),
	_iprofile (dev)
{
}

std::vector<uint8_t>::const_iterator MemoryMapping::getReadOnlyIterator (const Address &address)
{
	auto &page = getReadOnlyPage (address);
	return page.begin () + address.offset*2;
}

std::vector<uint8_t>::iterator MemoryMapping::getWritableIterator (const Address &address)
{
	auto &page = getWritablePage (address);
	return page.begin () + address.offset*2;
}

bool MemoryMapping::computeOffset (std::vector<uint8_t>::const_iterator it, Address &address)
{
	auto &page = getReadOnlyPage (address);
	int dist = distance (page.begin (), it);
	if (dist % 2 == 1)
		return false;
	address.offset = dist/2;
	return true;
}

void MemoryMapping::readPage (const Address &address, std::vector<uint8_t> &data)
{
	data.resize (PageSize);
	_imem.readMem (address, data);
}

void MemoryMapping::writePage (const Address &address, const std::vector<uint8_t> &data)
{
	_imem.writePage (address.page, data);
}
