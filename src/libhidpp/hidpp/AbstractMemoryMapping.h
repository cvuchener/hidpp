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

#ifndef LIBHIDPP_HIDPP_ABSTRACT_MEMORY_MAPPING_H
#define LIBHIDPP_HIDPP_ABSTRACT_MEMORY_MAPPING_H

#include <hidpp/Address.h>
#include <vector>
#include <map>
#include <cstdint>

namespace HIDPP
{

/**
 * Abstract class for accessing paged memory.
 *
 * Subclass must implement readPage and writePage
 * for memory access, and getReadOnlyIterator,
 * getWritableIterator and computeOffset to convert
 * between address offsets and iterators.
 */
class AbstractMemoryMapping
{
public:
	AbstractMemoryMapping (bool write_crc = true);

	/**
	 * Get the page at \p address (offset is ignored) as read-only.
	 */
	const std::vector<uint8_t> &getReadOnlyPage (const Address &address);
	/**
	 * Get the page at \p address (offset is ignored) and mark it as "modified".
	 */
	std::vector<uint8_t> &getWritablePage (const Address &address);

	/**
	 * Write all modified pages to the device memory.
	 */
	void sync ();

	/**
	 * Get a read-only iterator to the position corresponding
	 * to the address \p address.
	 *
	 * Implementation must retrieve the page using getReadOnlyPage.
	 */
	virtual std::vector<uint8_t>::const_iterator getReadOnlyIterator (const Address &address) = 0;
	/**
	 * Get a read-only iterator to the position corresponding
	 *
	 * Implementation must retrieve the page using getWritablePage.
	 * to the address \p address.
	 */
	virtual std::vector<uint8_t>::iterator getWritableIterator (const Address &address) = 0;

	/**
	 * Compute the offset of the position \p it.
	 *
	 * \p address must be filled except for the offset field.
	 *
	 * \return false if the position has no offset.
	 */
	virtual bool computeOffset (std::vector<uint8_t>::const_iterator it, Address &address) = 0;

protected:
	/**
	 * Read the page at \p address and fill data.
	 */
	virtual void readPage (const Address &address, std::vector<uint8_t> &data) = 0;
	/**
	 * Write the data in \p data in page at \p address.
	 */
	virtual void writePage (const Address &address, const std::vector<uint8_t> &data) = 0;

private:
	bool _write_crc;
	struct Page {
		bool modified;
		std::vector<uint8_t> data;
	};
	std::map<Address, Page> _pages;

	Page &getPage (Address address);
};

}

#endif
