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

#ifndef LIBHIDPP_HIDPP10_IMEMORY_H
#define LIBHIDPP_HIDPP10_IMEMORY_H

#include <vector>
#include <cstdint>

#include <hidpp/Address.h>

namespace HIDPP10
{

class Device;

class IMemory
{
public:
	enum MemoryOp: uint8_t {
		Fill = 2,
		Copy = 3,
	};

	IMemory (Device *dev);

	int readSome (HIDPP::Address address, uint8_t *buffer, std::size_t maxlen);
	void readMem (HIDPP::Address address, std::vector<uint8_t> &data);


	void writeMem (HIDPP::Address address, const std::vector<uint8_t> &data);
	void writePage (uint8_t page, const std::vector<uint8_t> &data);

	void resetSequenceNumber ();
	void fillPage (uint8_t page);

private:
	Device *_dev;
};

}

#endif
