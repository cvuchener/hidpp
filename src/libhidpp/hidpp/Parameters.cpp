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

#include <hidpp/Parameters.h>

using namespace HIDPP;

Parameters::Parameters ()
{
}

Parameters::Parameters (std::size_t length):
	std::vector<uint8_t> (length, 0)
{
}

Parameters::Parameters (const std::vector<uint8_t> &vector):
	std::vector<uint8_t> (vector)
{
}

Parameters::Parameters (const std::vector<uint8_t> &&vector):
	std::vector<uint8_t> (vector)
{
}

Parameters::Parameters (std::initializer_list<uint8_t> init):
	std::vector<uint8_t> (init)
{
}

uint16_t Parameters::getWordLE (unsigned int index) const
{
	return at (index) + (at (index+1) << 8);
}

void Parameters::setWordLE (unsigned int index, uint16_t value)
{
	if (index+1 >= size ())
		resize (index+2, 0);
	at (index) = value & 0xFF;
	at (index+1) = value >> 8;
}

uint16_t Parameters::getWordBE (unsigned int index) const
{
	return (at (index) << 8) + at (index+1);
}

void Parameters::setWordBE (unsigned int index, uint16_t value)
{
	if (index+1 >= size ())
		resize (index+2, 0);
	at (index) = value >> 8;
	at (index+1) = value & 0xFF;
}

uint8_t &Parameters::operator[] (std::size_t index)
{
	if (index >= size ())
		resize (index+1, 0);
	return std::vector<uint8_t>::operator[] (index);
}

uint8_t Parameters::operator[] (std::size_t index) const
{
	return std::vector<uint8_t>::operator[] (index);
}
