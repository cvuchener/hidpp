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

#include "IReprogControlsV4.h"

#include <misc/Endian.h>
#include <cassert>

using namespace HIDPP20;

constexpr uint16_t IReprogControlsV4::ID;

IReprogControlsV4::IReprogControlsV4 (Device *dev):
	FeatureInterface (dev, ID, "ReprogControlsV4")
{
}

unsigned int IReprogControlsV4::getControlCount ()
{
	std::vector<uint8_t> results;
	results = call (GetControlCount);
	return results[0];
}

IReprogControlsV4::ControlInfo IReprogControlsV4::getControlInfo (unsigned int index)
{
	std::vector<uint8_t> params (1), results;
	params[0] = index;
	results = call (GetControlInfo, params);
	ControlInfo ci;
	ci.control_id = readBE<uint16_t> (results, 0);
	ci.task_id = readBE<uint16_t> (results, 2);
	ci.flags = results[4];
	ci.pos = results[5];
	ci.group = results[6];
	ci.group_mask = results[7];
	ci.additional_flags = results[8];
	return ci;
}

uint16_t IReprogControlsV4::getControlReporting (uint16_t control_id, uint8_t &flags)
{
	std::vector<uint8_t> params (2), results;
	writeBE<uint16_t> (params, 0, control_id);
	results = call (GetControlReporting, params);
	flags = results[2];
	return readBE<uint16_t> (results, 3);
}

void IReprogControlsV4::setControlReporting (uint16_t control_id, uint8_t flags, uint16_t remap)
{
	std::vector<uint8_t> params (5), results;
	writeBE<uint16_t> (params, 0, control_id);
	params[2] = flags;
	writeBE<uint16_t> (params, 3, remap);
	call (SetControlReporting, params);
}

std::vector<uint16_t> IReprogControlsV4::divertedButtonEvent (const HIDPP::Report &event)
{
	assert (event.function () == DivertedButtonEvent);
	std::vector<uint16_t> buttons;
	auto params = event.parameterBegin ();
	for (unsigned int i = 0; i < 4; ++i) {
		uint16_t control_id = readBE<uint16_t> (params + 2*i);
		if (control_id == 0)
			break;
		buttons.push_back (control_id);
	}
	return buttons;
}

IReprogControlsV4::Move IReprogControlsV4::divertedRawXYEvent (const HIDPP::Report &event)
{
	assert (event.function () == DivertedRawXYEvent);
	auto params = event.parameterBegin ();
	return Move { readBE<int16_t> (params+0), readBE<int16_t> (params+2) };
}
