/*
 * Copyright 2017 Clément Vuchener
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

#ifndef LIBHIDPP_HIDPP10_RAM_MAPPING_H
#define LIBHIDPP_HIDPP10_RAM_MAPPING_H

#include <hidpp/AbstractMemoryMapping.h>
#include <hidpp10/IMemory.h>
#include <hidpp10/IProfile.h>

namespace HIDPP10
{

class RAMMapping: public HIDPP::AbstractMemoryMapping
{
public:
	RAMMapping (Device *dev);

	virtual std::vector<uint8_t>::const_iterator getReadOnlyIterator (const HIDPP::Address &address);
	virtual std::vector<uint8_t>::iterator getWritableIterator (const HIDPP::Address &address);
	virtual bool computeOffset (std::vector<uint8_t>::const_iterator it, HIDPP::Address &address);

protected:
	virtual void readPage (const HIDPP::Address &address, std::vector<uint8_t> &data);
	virtual void writePage (const HIDPP::Address &address, const std::vector<uint8_t> &data);

private:
	IMemory _imem;
};

}

#endif
