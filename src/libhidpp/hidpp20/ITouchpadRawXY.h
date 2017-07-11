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

#ifndef LIBHIDPP_HIDPP20_ITOUCHPADRAWXY_H
#define LIBHIDPP_HIDPP20_ITOUCHPADRAWXY_H

#include <hidpp20/FeatureInterface.h>

namespace HIDPP20
{

class ITouchpadRawXY: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x6100;

	enum Function {
		GetTouchpadInfo = 0,
		SetTouchpadRawMode = 2,
	};

	enum Event {
		TouchpadRawEvent = 0,
	};

	ITouchpadRawXY (Device *dev);

	struct TouchpadInfo {
		unsigned int x_max, y_max;
	};
	TouchpadInfo getTouchpadInfo ();

	void setTouchpadRawMode (bool enable);

	struct TouchpadRawData {
		uint16_t seqnum;
		struct Point {
			int16_t x, y;
			uint8_t unknown0;
			uint8_t unknown1;
			int id;
			int unknown2;
		} points[2];
	};
	static TouchpadRawData touchpadRawEvent (const HIDPP::Report &event);
};

}

#endif

