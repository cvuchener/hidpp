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

#ifndef LIBHIDPP_ENDIAN_H
#define LIBHIDPP_ENDIAN_H

#include <tuple>

template<typename T, typename InputIt>
typename std::enable_if<std::is_integral<T>::value, InputIt>::type
writeLE (InputIt it, T value)
{
	for (int i = 0; i < (int) sizeof (T); ++i)
		*(it++) = (value >> (i*8)) & 0xFF;
	return it;
}

template<typename T, typename Container>
typename std::enable_if<std::is_integral<T>::value>::type
writeLE (Container &cont, unsigned int index, T value)
{
	writeLE (cont.begin () + index, value);
}

template<typename T, typename InputIt>
typename std::enable_if<std::is_integral<T>::value, T>::type
readLE (InputIt it)
{
	T value = 0;
	for (int i = 0; i < (int) sizeof (T); ++i)
		value |= *(it++) << (i*8);
	return value;
}

template<typename T, typename Container>
typename std::enable_if<std::is_integral<T>::value, T>::type
readLE (const Container &cont, unsigned int index)
{
	return readLE<T> (cont.begin () + index);
}

template<typename T, typename Container>
typename std::enable_if<std::is_integral<T>::value>::type
pushLE (Container &cont, T value)
{
	for (int i = 0; i < (int) sizeof (T); ++i)
		cont.push_back ((value >> (i*8)) & 0xFF);
}

template<typename T, typename InputIt>
typename std::enable_if<std::is_integral<T>::value, InputIt>::type
writeBE (InputIt it, T value)
{
	for (int i = sizeof (T)-1; i >= 0; --i)
		*(it++) = (value >> (i*8)) & 0xFF;
	return it;
}

template<typename T, typename Container>
typename std::enable_if<std::is_integral<T>::value>::type
writeBE (Container &cont, unsigned int index, T value)
{
	writeBE (cont.begin () + index, value);
}


template<typename T, typename InputIt>
typename std::enable_if<std::is_integral<T>::value, T>::type
readBE (InputIt it)
{
	T value = 0;
	for (int i = sizeof (T)-1; i >= 0; --i)
		value |= *(it++) << (i*8);
	return value;
}

template<typename T, typename Container>
typename std::enable_if<std::is_integral<T>::value, T>::type
readBE (const Container &cont, unsigned int index)
{
	return readBE<T> (cont.begin () + index);
}

template<typename T, typename Container>
typename std::enable_if<std::is_integral<T>::value>::type
pushBE (Container &cont, T value)
{
	for (int i = sizeof (T)-1; i >= 0; --i)
		cont.push_back ((value >> (i*8)) & 0xFF);
}

#endif

