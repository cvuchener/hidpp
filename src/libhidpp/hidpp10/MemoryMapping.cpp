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

#include <hidpp10/MemoryMapping.h>

#include <hidpp10/IMemory.h>
#include <hidpp10/defs.h>
#include <misc/CRC.h>
#include <misc/Log.h>

using namespace HIDPP10;

MemoryMapping::MemoryMapping (Device *dev):
	_imem (dev)
{
}

const ByteArray &MemoryMapping::getReadOnlyPage (unsigned int page)
{
	getPage (page);
	return _pages[page].second;
}

ByteArray &MemoryMapping::getWritablePage (unsigned int page)
{
	getPage (page);
	_pages[page].first = Modified;
	return _pages[page].second;
}

void MemoryMapping::sync ()
{
	// Persistent memory start at page 1
	for (unsigned int i = 1; i < _pages.size (); ++i) {
		if (_pages[i].first == Modified) {
			uint16_t crc = CRC::CCITT (_pages[i].second.begin (),
						   _pages[i].second.end () - sizeof (crc));
			Log::printf (Log::Debug, "Page %d CRC is %04hx.\n", i, crc);
			_pages[i].second.setBE (PageSize - sizeof (crc), crc);
			_imem.writePage (i, _pages[i].second);
			_pages[i].first = Synced;
		}
	}
}

void MemoryMapping::getPage (unsigned int page)
{
	if (page >= _pages.size ())
		_pages.resize (page+1, { Absent, ByteArray () });
	if (_pages[page].first == Absent) {
		_pages[page].second.resize (PageSize);
		_imem.readMem (page, 0, _pages[page].second);
		uint16_t crc = CRC::CCITT (_pages[page].second.begin (),
					   _pages[page].second.end () - sizeof (crc));
		if (crc != _pages[page].second.getBE<uint16_t> (PageSize - sizeof (crc)))
			Log::warning () << "Invalid CRC for page " << page << std::endl;
		_pages[page].first = Synced;
	}
}

