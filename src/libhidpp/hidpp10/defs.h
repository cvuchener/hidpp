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

#ifndef HIDPP10_DEFS_H
#define HIDPP10_DEFS_H

#include <cstdint>

namespace HIDPP10
{
	enum SubID: uint8_t {
		SendDataAcknowledgement = 0x50,
		SetRegisterShort = 0x80,
		GetRegisterShort = 0x81,
		SetRegisterLong = 0x82,
		GetRegisterLong = 0x83,
		ErrorMessage = 0x8F,
		SendDataBegin = 0x90,
		SendDataContinue = 0x91,
		SendDataBeginAck = 0x92,
		SendDataContinueAck = 0x93,
	};

	enum RegisterAddress: uint8_t {
		EnableNotifications = 0x00,
		EnableIndividualFeatures = 0x01,
		ConnectionState = 0x02,
		BatteryStatus = 0x07,
		BatteryMileage = 0x0D,
		CurrentProfile = 0x0F,
		LEDStatus = 0x51,
		LEDIntensity = 0x54,
		LEDColor = 0x57,
		SensorSettings = 0x61,
		SensorResolution = 0x63,
		USBPollRate = 0x64,
		MemoryOperation = 0xA0,
		ResetSeqNum = 0xA1,
		MemoryRead = 0xA2,
		DevicePairing = 0xB2,
		DeviceActivity = 0xB3,
		DevicePairingInfo = 0xB5,
		FirmwareInfo = 0xF1,
	};

	constexpr std::size_t PageSize = 512;
	constexpr std::size_t RAMSize = 400;
}

#endif

