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

#ifndef HIDPP_IDS_H
#define HIDPP_IDS_H

#include <cstdint>

namespace HIDPP
{
	namespace ID {
		constexpr uint16_t G5 =		0xc041;
		constexpr uint16_t G5_2007 =	0xc049;
		constexpr uint16_t G7 =		0xc51a; // or is it the receiver ID?
		constexpr uint16_t G9 =		0xc048;
		constexpr uint16_t G9x =	0xc066;
		constexpr uint16_t G9x_MW3 =	0xc249;
		constexpr uint16_t G500 =	0xc068;
		constexpr uint16_t G700 =	0xc06b;
		constexpr uint16_t G500s =	0xc24e;
		constexpr uint16_t G700s =	0xc07c;
	}
}

#endif
