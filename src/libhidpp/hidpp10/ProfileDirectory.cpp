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

#include <hidpp10/ProfileDirectory.h>

#include <hidpp10/MemoryMapping.h>

using namespace HIDPP10;

ProfileDirectory::ProfileDirectory ()
{
}

ProfileDirectory::ProfileDirectory (MemoryMapping *mem)
{
	const std::vector<uint8_t> &page = mem->getReadOnlyPage (1);
	unsigned int i = 0;
	while (page[i] != 0xff) {
		emplace_back (ProfileEntry ({
			{ page[i+0], page[i+1] },
			page[i+2]
		}));
		i += 3;
	}
}

void ProfileDirectory::write (MemoryMapping *mem) const
{
	std::vector<uint8_t> &page = mem->getWritablePage (1);
	unsigned int i = 0;
	for (const ProfileEntry &entry: *this) {
		page[i+0] = entry.address.page;
		page[i+1] = entry.address.offset;
		page[i+2] = entry.leds;
		i += 3;
	}
	page[i] = 0xff;
}

