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

#ifndef HIDPP10_MEMORY_MAPPING_H
#define HIDPP10_MEMORY_MAPPING_H

#include <hidpp10/IMemory.h>
#include <vector>

namespace HIDPP10
{

class Device;

class MemoryMapping
{
public:
	MemoryMapping (Device *dev);

	const std::vector<uint8_t> &getReadOnlyPage (unsigned int page);
	/**
	 * Get the page \p page and mark it as "modified".
	 */
	std::vector<uint8_t> &getWritablePage (unsigned int page);

	/**
	 * Write all modified pages to the device memory except for page 0
	 * (page 0 cannot be written as a whole page).
	 */
	void sync ();

private:
	void getPage (unsigned int page);

	IMemory _imem;
	enum PageState
	{
		Absent,
		Synced,
		Modified,
	};
	std::vector<std::pair<PageState, std::vector<uint8_t>>> _pages;
};

}

#endif

