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

#ifndef HIDPP20_IFEATURESET_H
#define HIDPP20_IFEATURESET_H

#include <cstdint>

namespace HIDPP20
{

class Device;

class IFeatureSet
{
public:
	static constexpr uint16_t ID = 0x0001;

	enum Function {
		GetCount = 0,
		GetFeatureID = 1,
	};

	IFeatureSet (Device *dev);

	uint8_t index () const;

	unsigned int getCount ();
	uint16_t getFeatureID (uint8_t feature_index,
			       bool *obsolete = nullptr,
			       bool *hidden = nullptr);

private:
	Device *_dev;
	uint8_t _index;
};

}

#endif

