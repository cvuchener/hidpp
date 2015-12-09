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

#ifndef HIDPP10_PROFILE_DIRECTORY_H
#define HIDPP10_PROFILE_DIRECTORY_H

#include <cstdint>
#include <vector>
#include <hidpp10/Address.h>

namespace HIDPP10
{

struct ProfileEntry
{
	Address address;
	uint8_t leds;
};

class MemoryMapping;

class ProfileDirectory: public std::vector<ProfileEntry>
{
public:
	ProfileDirectory ();
	ProfileDirectory (MemoryMapping *mem);

	void write (MemoryMapping *mem) const;
};

}

#endif

