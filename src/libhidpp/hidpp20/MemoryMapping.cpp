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

#include <algorithm>
#include <cassert>

using namespace HIDPP;
using namespace HIDPP20;

MemoryMapping::MemoryMapping (Device *dev, bool write_crc):
	AbstractMemoryMapping (write_crc),
	_iop (dev),
	_desc (_iop.getDescription ())
{
}

std::vector<uint8_t>::const_iterator MemoryMapping::getReadOnlyIterator (const Address &address)
{
	auto &page = getReadOnlyPage (address);
	return page.begin () + address.offset;
}

std::vector<uint8_t>::iterator MemoryMapping::getWritableIterator (const Address &address)
{
	auto &page = getWritablePage (address);
	return page.begin () + address.offset;
}

bool MemoryMapping::computeOffset (std::vector<uint8_t>::const_iterator it, Address &address)
{
	auto &page = getReadOnlyPage (address);
	int dist = distance (page.begin (), it);
	address.offset = dist;
	return true;
}

void MemoryMapping::readPage (const Address &address, std::vector<uint8_t> &data)
{
	data.resize (_desc.sector_size);
	for (unsigned int i = 0; i < _desc.sector_size; i += IOnboardProfiles::LineSize) {
		auto vec = _iop.memoryRead (static_cast<IOnboardProfiles::MemoryType> (address.mem_type), address.page, i);
		std::copy_n (vec.begin (), IOnboardProfiles::LineSize, &data[i]);
	}
}

void MemoryMapping::writePage (const Address &address, const std::vector<uint8_t> &data)
{
	assert (address.mem_type == IOnboardProfiles::Writeable);
	_iop.memoryAddrWrite (address.page, address.offset, _desc.sector_size);
	constexpr size_t LineSize = IOnboardProfiles::LineSize;
	for (unsigned int i = 0; i < _desc.sector_size; i += LineSize) {
		_iop.memoryWrite (data.begin () + i, data.begin () + i + LineSize);
	}
	_iop.memoryWriteEnd ();
}

