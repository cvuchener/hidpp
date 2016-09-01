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

#include <misc/CRC.h>

uint16_t CRC::CCITT (std::vector<uint8_t>::const_iterator begin,
		     std::vector<uint8_t>::const_iterator end,
		     uint16_t start_value)
{
	uint16_t crc = start_value;

	for (auto it = begin; it != end; ++it) {
		uint16_t temp = (crc >> 8) ^ *it;
		crc <<= 8;
		uint16_t quick = temp ^ (temp >> 4);
		crc ^= quick;
		quick <<= 5;
		crc ^= quick;
		quick <<= 7;
		crc ^= quick;
	}

	return crc;
}

