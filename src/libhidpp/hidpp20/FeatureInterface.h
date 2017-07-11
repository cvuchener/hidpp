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

#ifndef LIBHIDPP_HIDPP20_FEATURE_INTERFACE_H
#define LIBHIDPP_HIDPP20_FEATURE_INTERFACE_H

#include <hidpp20/Device.h>

#include <cstdint>
#include <vector>

namespace HIDPP20
{

class FeatureInterface
{
public:
	FeatureInterface (Device *dev, uint16_t id, const char *name);

	Device *device () const;

	uint8_t index () const;

	template<typename... Params>
	std::vector<uint8_t> call (unsigned int function, Params... params)
	{
		return _dev->callFunction (_index, function, params...);
	}

private:
	Device *_dev;
	uint8_t _index;
};

}

#endif

