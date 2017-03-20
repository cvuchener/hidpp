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

#include "FeatureInterface.h"

#include <hidpp20/Device.h>
#include <hidpp20/IRoot.h>
#include <hidpp20/UnsupportedFeature.h>
#include <misc/Log.h>

using namespace HIDPP20;

FeatureInterface::FeatureInterface (Device *dev, uint16_t id, const char *name):
	_dev (dev),
	_index (IRoot (dev).getFeature (id))
{
	if (_index == 0) {
		Log::info ("feature").printf ("Feature [0x%04hx] %s is not supported\n", id, name);
		throw UnsupportedFeature (id, name);
	}
	Log::info ("feature").printf ("Feature [0x%04hx] %s has index 0x%02hhx\n", id, name, _index);
}

Device *FeatureInterface::device () const
{
	return _dev;
}

uint8_t FeatureInterface::index () const
{
	return _index;
}

