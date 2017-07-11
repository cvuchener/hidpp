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

#ifndef LIBHIDPP_HIDPP10_PROFILE_FORMAT_COMMON_H
#define LIBHIDPP_HIDPP10_PROFILE_FORMAT_COMMON_H

#include <hidpp/Profile.h>

namespace HIDPP10
{

enum SpecialAction: uint16_t
{
	WheelLeft = 0x01,
	WheelRight = 0x02,
	BatteryLevel = 0x03,
	ResolutionNext = 0x04,
	ResolutionCycleNext = 0x05,
	ResolutionPrev = 0x08,
	ResolutionCyclePrev = 0x09,
	ProfileNext = 0x10,
	ProfileCycleNext = 0x11,
	ProfilePrev = 0x20,
	ProfileCyclePrev = 0x21,
	ProfileSwitch = 0x40,
};

constexpr std::size_t ButtonSize = 3;

HIDPP::Profile::Button parseButton (std::vector<uint8_t>::const_iterator begin);
void writeButton (std::vector<uint8_t>::iterator begin, const HIDPP::Profile::Button &button);

}

#endif
