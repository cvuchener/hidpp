/*
 * Copyright 2022, Logitech, Inc.
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
 */

#ifndef LIBHIDPP_HIDPP20_IDEVICETYPENAME_H
#define LIBHIDPP_HIDPP20_IDEVICETYPENAME_H

#include <hidpp20/FeatureInterface.h>

#include <string>

namespace HIDPP20
{

/**
 * Device Type and Name
 */
class IDeviceTypeName: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x0005;

	enum Function {
		GetDeviceNameCount = 0,
		GetDeviceName = 1,
		GetDeviceType = 2,
	};

	IDeviceTypeName (Device *dev);

	enum DeviceType: uint8_t
	{
		Keyboard               =  0,
		RemoteControl          =  1,
		Numpad                 =  2,
		Mouse                  =  3,
		Trackpad               =  4,
		Trackball              =  5,
		Presenter              =  6,
		Receiver               =  7,
		Headset                =  8,
		Webcam                 =  9,
		SteeringWheel          = 10,
		Joystick               = 11,
		Gamepad                = 12,
		Dock                   = 13,
		Speaker                = 14,
		Microphone             = 15,
		IlluminationLight      = 16,
		ProgrammableController = 17,
		CarSimPedals           = 18,
		Adapter                = 19,
	};

	uint8_t getDeviceNameCount ();
	std::string getDeviceName ();
	DeviceType getDeviceType ();

	static std::string getDeviceTypeString(DeviceType type);
};

}

#endif
