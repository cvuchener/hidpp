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

#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

#include <vector>
#include <cstdint>

/**
 * An auto-growing vector of bytes.
 *
 * Writing methods resize the vector if it is necessary.
 * Uninitialized bytes are filled with zeroes.
 *
 * This class also add endian methods for various types (\c int16_t,
 * \c uint16_t, \c int32_t, \c uint32_t).
 */
class ByteArray: public std::vector<uint8_t>
{
public:
	/**
	 * Constructs an empty vector.
	 */
	ByteArray ();
	/**
	 * Constructs a \p length size vector filled with zeroes.
	 */
	ByteArray (std::size_t length);
	/**
	 * Copy constructor.
	 */
	ByteArray (const std::vector<uint8_t> &vector);
	/**
	 * Move constructor.
	 */
	ByteArray (const std::vector<uint8_t> &&vector);
	/**
	 * Constructor from initializer list.
	 */
	ByteArray (std::initializer_list<uint8_t> init);

	/**
	 * Access a byte at \p index.
	 *
	 * This method may resize the vector.
	 */
	uint8_t &operator[] (std::size_t index);
	/**
	 * Read-only access to a byte at \p index.
	 *
	 * It does not resize the vector.
	 */
	const uint8_t &operator[] (std::size_t index) const;

	/**
	 * Read the little-endian integer starting at \p iterator.
	 */
	template <typename T> static T getLE (const_iterator iterator);
	/**
	 * Write a little-endian integer starting at \p iterator.
	 *
	 * This method does not resize the vector.
	 */
	template <typename T> static void setLE (iterator iterator, T value);
	/**
	 * Read the bug-endian integer starting at \p iterator.
	 */
	template <typename T> static T getBE (const_iterator iterator);
	/**
	 * Write a big-endian integer starting at \p iterator.
	 *
	 * This method does not resize the vector.
	 */
	template <typename T> static void setBE (iterator iterator, T value);

	/**
	 * Read the little-endian integer starting at \p index.
	 */
	template <typename T> T getLE (unsigned int index) const;
	/**
	 * Write a little-endian integer starting at \p index.
	 *
	 * This method may resize the vector.
	 */
	template <typename T> void setLE (unsigned int index, T value);
	/**
	 * Read the big-endian integer starting at \p index.
	 */
	template <typename T> T getBE (unsigned int index) const;
	/**
	 * Write a big-endian integer starting at \p index.
	 *
	 * This method may resize the vector.
	 */
	template <typename T> void setBE (unsigned int index, T value);
};

#define EXTERN_ENDIAN_TEMPLATES(T) \
extern template T ByteArray::getLE (ByteArray::const_iterator); \
extern template void ByteArray::setLE (ByteArray::iterator, T value); \
extern template T ByteArray::getBE (ByteArray::const_iterator); \
extern template void ByteArray::setBE (ByteArray::iterator, T value); \
extern template T ByteArray::getLE (unsigned int index) const; \
extern template void ByteArray::setLE (unsigned int index, T value); \
extern template T ByteArray::getBE (unsigned int index) const; \
extern template void ByteArray::setBE (unsigned int index, T value);

EXTERN_ENDIAN_TEMPLATES(uint16_t)
EXTERN_ENDIAN_TEMPLATES(uint32_t)
EXTERN_ENDIAN_TEMPLATES(int16_t)
EXTERN_ENDIAN_TEMPLATES(int32_t)

#undef EXTERN_ENDIAN_TEMPLATES

#endif

