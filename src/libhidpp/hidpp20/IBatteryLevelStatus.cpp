/*
 * Copyright 2019 Clément Vuchener
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

#include "IBatteryLevelStatus.h"

#include <misc/Endian.h>
#include <cassert>

using namespace HIDPP20;

IBatteryLevelStatus::IBatteryLevelStatus (Device *dev):
	FeatureInterface (dev, ID, "BatteryLevelStatus")
{
}

IBatteryLevelStatus::LevelStatus IBatteryLevelStatus::getLevelStatus ()
{
	auto results = call (GetBatteryLevelStatus);
	return parseLevelStatus (results.begin ());
}

IBatteryLevelStatus::Capability IBatteryLevelStatus::getCapability ()
{
	auto results = call (GetBatteryCapability);
	return Capability {
		results[0], // number of levels
		results[1], // flags
		readBE<uint16_t>(results, 2), // battery life
		results[4],
	};
}

IBatteryLevelStatus::LevelStatus IBatteryLevelStatus::batteryLevelEvent (const HIDPP::Report &event)
{
	assert (event.function () == BatteryLevelEvent);
	return parseLevelStatus (event.parameterBegin ());
}

IBatteryLevelStatus::LevelStatus IBatteryLevelStatus::parseLevelStatus (std::vector<uint8_t>::const_iterator params)
{
	return LevelStatus {
		*(params + 0), // level
		*(params + 1), // next level
		static_cast<Status>(*(params + 2)), // status
	};
}
