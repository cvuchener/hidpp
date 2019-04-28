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

#ifndef LIBHIDPP_HIDPP20_IBATTERYLEVELSTATUS_H
#define LIBHIDPP_HIDPP20_IBATTERYLEVELSTATUS_H

#include <hidpp20/FeatureInterface.h>

namespace HIDPP20
{

/**
 * Battery informations
 *
 * If Capability::number_of_levels is 4, the levels are
 *  - 0% - 10%: critical
 *  - 11% - 30%: low
 *  - 31% - 80%: good
 *  - 81% - 100%: full
 */
class IBatteryLevelStatus: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x1000;

	enum Function {
		GetBatteryLevelStatus = 0,
		GetBatteryCapability = 1,
	};

	enum Event {
		BatteryLevelEvent = 0,
	};

	IBatteryLevelStatus (Device *dev);

	enum Status: uint8_t
	{
		Discharging = 0, // in use
		Recharging = 1,
		ChargeInFinalState = 2,
		ChargeComplete = 3,
		RechargingBelowOptimalSpeed = 4,
		InvalidBatteryType = 5,
		ThermalError = 6,
		OtherChargingError = 7,
	};

	struct LevelStatus
	{
		uint8_t discharge_level; ///< current level in %, 0 means unknown
		uint8_t discharge_next_level; ///< next level in % when discharging, 0 otherwise
		Status status;
	};

	struct Capability
	{
		uint8_t number_of_levels; ///< min: 2, max: 100, if less than 10 or mileage is disabled, use 4
		enum Flag: uint8_t {
			DisableBatteryOSD = 0x01,
			EnableMileageCalculation = 0x02, ///< ignored if number_of_level is less than 10
			Rechargeable = 0x04,
		};
		uint8_t flags;
		uint16_t nominal_battery_life; ///< only used when mileage is enabled.
		uint8_t critical_level; ///< in %, only used when mileage is enabled
	};

	LevelStatus getLevelStatus ();
	Capability getCapability ();

	static LevelStatus batteryLevelEvent (const HIDPP::Report &event);

private:
	static LevelStatus parseLevelStatus (std::vector<uint8_t>::const_iterator params);
};

}

#endif

