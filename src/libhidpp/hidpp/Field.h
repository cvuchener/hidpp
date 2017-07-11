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

#ifndef LIBHIDPP_HIDPP_FIELD_H
#define LIBHIDPP_HIDPP_FIELD_H

#include <misc/Endian.h>

namespace HIDPP
{

enum ByteOrder
{
	Undefined,
	BigEndian,
	LittleEndian,
};

template<typename T, ByteOrder BO = Undefined>
class Field;

template<typename T>
class Field<T, Undefined>
{
public:
	unsigned int offset;
	static constexpr std::size_t size = sizeof (T);

	constexpr Field (unsigned int offset):
		offset (offset)
	{
	}

	typename std::enable_if<sizeof (T) == sizeof (uint8_t), T>::type
	read (std::vector<uint8_t>::const_iterator it) const
	{
		return static_cast<T> (*(it+offset));
	}

	typename std::enable_if<sizeof (T) == sizeof (uint8_t)>::type
	write (std::vector<uint8_t>::iterator it, T value) const
	{
		*(it+offset) = static_cast<uint8_t> (value);
	}
};

template<typename T>
class Field<T, BigEndian>
{
public:
	unsigned int offset;
	static constexpr std::size_t size = sizeof (T);

	constexpr Field (unsigned int offset):
		offset (offset)
	{
	}

	T read (std::vector<uint8_t>::const_iterator it) const
	{
		return readBE<T> (it+offset);
	}

	void write (std::vector<uint8_t>::iterator it, T value) const
	{
		writeBE (it+offset, value);
	}
};

template<typename T>
class Field<T, LittleEndian>
{
public:
	unsigned int offset;
	static constexpr std::size_t size = sizeof (T);

	constexpr Field (unsigned int offset):
		offset (offset)
	{
	}

	T read (std::vector<uint8_t>::const_iterator it) const
	{
		return readLE<T> (it+offset);
	}

	void write (std::vector<uint8_t>::iterator it, T value) const
	{
		writeLE (it+offset, value);
	}
};

template<>
class Field<Color, Undefined>
{
public:
	unsigned int offset;
	static constexpr std::size_t size = sizeof (Color);

	constexpr Field (unsigned int offset):
		offset (offset)
	{
	}

	Color read (std::vector<uint8_t>::const_iterator it) const
	{
		return { it[offset+0], it[offset+1], it[offset+2] };
	}

	void write (std::vector<uint8_t>::iterator it, Color value) const
	{
		it[offset+0] = value.r;
		it[offset+1] = value.g;
		it[offset+2] = value.b;
	}
};

template<typename T, std::size_t Count, ByteOrder BO = Undefined>
class ArrayField
{
public:
	unsigned int offset;
	static constexpr std::size_t size = Count * sizeof (T);
	typedef Field<T, BO> ItemField;

	constexpr ArrayField (unsigned int offset):
		offset (offset)
	{
	}

	T read (std::vector<uint8_t>::const_iterator it, unsigned int index) const
	{
		return ItemField (offset + index * sizeof (T)).read (it);
	}

	void write (std::vector<uint8_t>::iterator it, unsigned int index, T value) const
	{
		ItemField (offset + index * sizeof (T)).write (it, value);
	}
};

template<std::size_t S>
class StructField
{
public:
	unsigned int offset;
	static constexpr std::size_t size = S;

	constexpr StructField (unsigned int offset):
		offset (offset)
	{
	}

	template<typename It>
	It begin (It it) const
	{
		return it + offset;
	}

	template<typename It>
	It end (It it) const
	{
		return it + offset + size;
	}
};

template<std::size_t S, std::size_t Count>
class StructArrayField
{
public:
	unsigned int offset;
	static constexpr std::size_t size = Count * S;
	typedef StructField<S> ItemField;

	constexpr StructArrayField (unsigned int offset):
		offset (offset)
	{
	}

	template<typename It>
	It begin (It it, unsigned int index) const
	{
		return it + offset + index * S;
	}

	template<typename It>
	It end (It it, unsigned int index) const
	{
		return it + offset + (index+1) * S;
	}
};

}

#endif
