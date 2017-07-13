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

#include <hidpp20/IFeatureSet.h>

#include <misc/Endian.h>

using namespace HIDPP20;

constexpr uint16_t IFeatureSet::ID;

IFeatureSet::IFeatureSet (Device *dev):
	FeatureInterface (dev, ID, "FeatureSet")
{
}

unsigned int IFeatureSet::getCount ()
{
	std::vector<uint8_t> results;
	results = call (GetCount);
	return results[0];
}

uint16_t IFeatureSet::getFeatureID (uint8_t feature_index,
				    bool *obsolete,
				    bool *hidden,
				    bool *internal)
{
	std::vector<uint8_t> params (1), results;
	params[0] = feature_index;
	results = call (GetFeatureID, params);
	if (obsolete)
		*obsolete = results[2] & (1<<7);
	if (hidden)
		*hidden = results[2] & (1<<6);
	if (internal)
		*internal = results[2] & (1<<5);
	return readBE<uint16_t> (results, 0);
}

