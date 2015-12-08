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

#include <misc/ByteArray.h>

ByteArray::ByteArray ()
{
}

ByteArray::ByteArray (std::size_t length):
	std::vector<uint8_t> (length, 0)
{
}

ByteArray::ByteArray (const std::vector<uint8_t> &vector):
	std::vector<uint8_t> (vector)
{
}

ByteArray::ByteArray (const std::vector<uint8_t> &&vector):
	std::vector<uint8_t> (vector)
{
}

ByteArray::ByteArray (std::initializer_list<uint8_t> init):
	std::vector<uint8_t> (init)
{
}

uint8_t &ByteArray::operator[] (std::size_t index)
{
	if (index + sizeof (uint8_t) > size ())
		resize (index + sizeof (uint8_t), 0);
	return std::vector<uint8_t>::operator[] (index);
}

const uint8_t &ByteArray::operator[] (std::size_t index) const
{
	return std::vector<uint8_t>::operator[] (index);
}

template <typename T>
T ByteArray::getLE (const_iterator iterator)
{
	T value = 0;
	for (unsigned int i = 0; i < sizeof (T); ++i)
		value |= *(iterator+i) << (i*8);
	return value;
}

template <typename T>
void ByteArray::setLE (iterator iterator, T value)
{
	for (unsigned int i = 0; i < sizeof (T); ++i)
		*(iterator+i) = (value >> (i*8)) & 0xFF;
}

template <typename T>
T ByteArray::getBE (const_iterator iterator)
{
	T value = 0;
	for (unsigned int i = 0; i < sizeof (T); ++i)
		value |= *(iterator+i) << ((sizeof(T)-1-i)*8);
	return value;
}

template <typename T>
void ByteArray::setBE (iterator iterator, T value)
{
	for (unsigned int i = 0; i < sizeof (T); ++i)
		*(iterator+i) = (value >> ((sizeof(T)-1-i)*8)) & 0xFF;
}

template<typename T>
T ByteArray::getLE (unsigned int index) const
{
	return getLE<T> (begin () + index);
}

template<typename T>
void ByteArray::setLE (unsigned int index, T value)
{
	if (index + sizeof (T) > size ())
		resize (index + sizeof (T), 0);
	setLE<T> (begin () + index, value);
}

template<typename T>
T ByteArray::getBE (unsigned int index) const
{
	return getBE<T> (begin () + index);
}

template<typename T>
void ByteArray::setBE (unsigned int index, T value)
{
	if (index + sizeof (T) > size ())
		resize (index + sizeof (T), 0);
	setBE<T> (begin () + index, value);
}

#define INSTANTIATE_ENDIAN_FUNC(T) \
template T ByteArray::getLE (ByteArray::const_iterator); \
template void ByteArray::setLE (ByteArray::iterator, T value);\
template T ByteArray::getBE (ByteArray::const_iterator); \
template void ByteArray::setBE (ByteArray::iterator, T value);\
template T ByteArray::getLE (unsigned int index) const; \
template void ByteArray::setLE (unsigned int index, T value); \
template T ByteArray::getBE (unsigned int index) const; \
template void ByteArray::setBE (unsigned int index, T value);

INSTANTIATE_ENDIAN_FUNC(uint16_t)
INSTANTIATE_ENDIAN_FUNC(uint32_t)
INSTANTIATE_ENDIAN_FUNC(int16_t)
INSTANTIATE_ENDIAN_FUNC(int32_t)


