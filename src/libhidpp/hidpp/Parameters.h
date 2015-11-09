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

#ifndef HIDPP_PARAMETERS_H
#define HIDPP_PARAMETERS_H

#include <vector>
#include <cstdint>

namespace HIDPP
{

constexpr std::size_t ShortParamLength = 3;
constexpr std::size_t LongParamLength = 16;

class Parameters: public std::vector<uint8_t>
{
public:
	Parameters ();
	Parameters (std::size_t length);
	Parameters (const std::vector<uint8_t> &vector);
	Parameters (const std::vector<uint8_t> &&vector);
	Parameters (std::initializer_list<uint8_t> init);

	uint16_t getWordLE (unsigned int index) const;
	void setWordLE (unsigned int index, uint16_t value);
	uint16_t getWordBE (unsigned int index) const;
	void setWordBE (unsigned int index, uint16_t value);
	uint8_t &operator[] (std::size_t index);
	uint8_t operator[] (std::size_t index) const;
};

}

#endif

