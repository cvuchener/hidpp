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

class ByteArray: public std::vector<uint8_t>
{
public:
	ByteArray ();
	ByteArray (std::size_t length);
	ByteArray (const std::vector<uint8_t> &vector);
	ByteArray (const std::vector<uint8_t> &&vector);
	ByteArray (std::initializer_list<uint8_t> init);

	uint8_t &operator[] (std::size_t index);
	const uint8_t &operator[] (std::size_t index) const;

	template <typename T> static T getLE (const_iterator iterator);
	template <typename T> static void setLE (iterator iterator, T value);
	template <typename T> static T getBE (const_iterator iterator);
	template <typename T> static void setBE (iterator iterator, T value);

	template <typename T> T getLE (unsigned int index) const;
	template <typename T> void setLE (unsigned int index, T value);
	template <typename T> T getBE (unsigned int index) const;
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

#endif

