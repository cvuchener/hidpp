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

#ifndef LIBHIDPP_HIDPP20_IROOT_H
#define LIBHIDPP_HIDPP20_IROOT_H

#include <cstdint>

namespace HIDPP20
{

class Device;

class IRoot
{
public:
	static constexpr uint16_t ID = 0x0000;
	static constexpr uint8_t index = 0x00; // IRoot has a fixed index

	enum Function {
		GetFeature = 0,
		Ping = 1,
	};

	IRoot (Device *dev);

	uint8_t getFeature (uint16_t feature_id,
			    bool *obsolete = nullptr,
			    bool *hidden = nullptr);

private:
	Device *_dev;
};

}

#endif
