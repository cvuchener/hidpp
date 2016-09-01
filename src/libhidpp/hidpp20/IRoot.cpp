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

#include <hidpp20/IRoot.h>

#include <hidpp20/Device.h>

#include <misc/Endian.h>

using namespace HIDPP20;

IRoot::IRoot (Device *dev):
	_dev (dev)
{
}

uint8_t IRoot::getFeature (uint16_t feature_id,
			   bool *obsolete,
			   bool *hidden)
{
	std::vector<uint8_t> params (2), results;
	writeBE<uint16_t> (params, 0, feature_id);
	results = _dev->callFunction (index, GetFeature, params);
	if (obsolete)
		*obsolete = results[1] & (1<<7);
	if (hidden)
		*hidden = results[1] & (1<<6);
	return results[0];
}

