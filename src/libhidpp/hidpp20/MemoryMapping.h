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

#ifndef LIBHIDPP_HIDPP20_MEMORY_MAPPING_H
#define LIBHIDPP_HIDPP20_MEMORY_MAPPING_H

#include <hidpp/AbstractMemoryMapping.h>
#include <hidpp20/IOnboardProfiles.h>

namespace HIDPP20
{

class MemoryMapping: public HIDPP::AbstractMemoryMapping
{
public:
	MemoryMapping (Device *dev, bool write_crc = true);

	virtual std::vector<uint8_t>::const_iterator getReadOnlyIterator (const HIDPP::Address &address);
	virtual std::vector<uint8_t>::iterator getWritableIterator (const HIDPP::Address &address);
	virtual bool computeOffset (std::vector<uint8_t>::const_iterator it, HIDPP::Address &address);

protected:
	virtual void readPage (const HIDPP::Address &address, std::vector<uint8_t> &data);
	virtual void writePage (const HIDPP::Address &address, const std::vector<uint8_t> &data);

private:
	IOnboardProfiles _iop;
	IOnboardProfiles::Description _desc;
};

}

#endif
