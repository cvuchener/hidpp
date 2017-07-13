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

#include <hidpp20/IAdjustableDPI.h>

#include <misc/Endian.h>

using namespace HIDPP20;

constexpr uint16_t IAdjustableDPI::ID;

IAdjustableDPI::IAdjustableDPI (Device *dev):
	FeatureInterface (dev, ID, "AdjustableDPI")
{
}

unsigned int IAdjustableDPI::getSensorCount ()
{
	std::vector<uint8_t> results;
	results = call (GetSensorCount);
	return results[0];
}

bool IAdjustableDPI::getSensorDPIList (unsigned int index,
				       std::vector<unsigned int> &dpi_list,
				       unsigned int &dpi_step)
{
	std::vector<uint8_t> params (1), results;
	params[0] = index;
	results = call (GetSensorDPIList, params);
	dpi_list.clear ();
	bool has_dpi_step = false;
	uint16_t value;
	auto current = results.begin () + 1;
	while ((value = readBE<uint16_t> (current)) != 0) {
		if (value > 0xe000) {
			has_dpi_step = true;
			dpi_step = value - 0xe000;
		}
		else {
			dpi_list.push_back (value);
		}
		current += 2;
	}
	return has_dpi_step;
}

std::tuple<unsigned int, unsigned int> IAdjustableDPI::getSensorDPI (unsigned int index)
{
	std::vector<uint8_t> params (1), results;
	params[0] = index;
	results = call (GetSensorDPI, params);
	unsigned int current_dpi = readBE<uint16_t> (results, 1);
	unsigned int default_dpi = readBE<uint16_t> (results, 3);
	return std::make_tuple (current_dpi, default_dpi);
}

void IAdjustableDPI::setSensorDPI (unsigned int index, unsigned int dpi)
{
	std::vector<uint8_t> params (3);
	params[0] = index;
	writeBE<uint16_t> (params, 1, dpi);
	call (SetSensorDPI, params);
}

