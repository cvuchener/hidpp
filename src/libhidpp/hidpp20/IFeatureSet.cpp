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

#include <hidpp20/Device.h>
#include <hidpp20/IRoot.h>
#include <misc/Log.h>
#include <misc/Endian.h>

using namespace HIDPP20;

IFeatureSet::IFeatureSet (Device *dev):
	_dev (dev),
	_index (IRoot (dev).getFeature (ID))
{
	Log::printf (Log::Debug, "Feature [0x%04hx] IFeatureSet has index 0x%02hhx\n", ID, _index);
}

uint8_t IFeatureSet::index () const
{
	return _index;
}

unsigned int IFeatureSet::getCount ()
{
	std::vector<uint8_t> params, results;
	results = _dev->callFunction (_index, GetCount, params);
	return results[0];
}

uint16_t IFeatureSet::getFeatureID (uint8_t feature_index,
				    bool *obsolete,
				    bool *hidden)
{
	std::vector<uint8_t> params (1), results;
	params[0] = feature_index;
	results = _dev->callFunction (_index, GetFeatureID, params);
	if (obsolete)
		*obsolete = results[2] & (1<<7);
	if (hidden)
		*hidden = results[2] & (1<<6);
	return readBE<uint16_t> (results, 0);
}

