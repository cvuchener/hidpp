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

#include "ITouchpadRawXY.h"

#include <misc/Endian.h>
#include <cassert>

using namespace HIDPP20;

constexpr uint16_t ITouchpadRawXY::ID;

ITouchpadRawXY::ITouchpadRawXY (Device *dev):
	FeatureInterface (dev, ID, "TouchpadRawXY")
{
}

ITouchpadRawXY::TouchpadInfo ITouchpadRawXY::getTouchpadInfo ()
{
	auto results = call (GetTouchpadInfo);
	TouchpadInfo info;
	info.x_max = readBE<uint16_t> (results, 0);
	info.y_max = readBE<uint16_t> (results, 2);
	return info;
}

void ITouchpadRawXY::setTouchpadRawMode (bool enable)
{
	std::vector<uint8_t> params (1);
	params[0] = (enable ? 0x01 : 0x00);
	call (SetTouchpadRawMode, params);
}

ITouchpadRawXY::TouchpadRawData ITouchpadRawXY::touchpadRawEvent (const HIDPP::Report &event)
{
	assert (event.function () == TouchpadRawEvent);
	TouchpadRawData data;
	auto params = event.parameterBegin ();
	data.seqnum = readBE<uint16_t> (params+0);
	for (unsigned int i = 0; i < 2; ++i) {
		auto pdata = params+2+7*i;
		data.points[i].x = readBE<int16_t> (pdata+0);
		data.points[i].y = readBE<int16_t> (pdata+2);
		data.points[i].unknown0 = pdata[4];
		data.points[i].unknown1 = pdata[5];
		data.points[i].id = pdata[6] >> 4;
		data.points[i].unknown2 = pdata[6] & 0x0f;
	}
	return data;
}
