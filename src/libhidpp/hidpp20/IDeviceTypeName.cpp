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

#include "IDeviceTypeName.h"

#include <algorithm>
#include <stdexcept>

using namespace HIDPP20;

IDeviceTypeName::IDeviceTypeName (Device *dev):
	FeatureInterface (dev, ID, "DeviceTypeName")
{
}

uint8_t IDeviceTypeName::getDeviceNameCount ()
{
	const auto results = call (GetDeviceNameCount);
	if (results.size() < 1)
		throw std::runtime_error ("Invalid GetDeviceNameCount response");
	return results[0];
}

std::string IDeviceTypeName::getDeviceName ()
{
	const auto deviceNameLength = getDeviceNameCount();	// not including null-terminators
	auto deviceName = std::string{};

	// Request chunks from the device name until we have the entire name
	while (deviceName.size() < deviceNameLength) {
		// Request characters following the part of the name we already have
		const auto params = std::vector<uint8_t>{
			static_cast<uint8_t>(deviceName.size())		// charIndex
		};
		auto results = call (GetDeviceName, params);

		// Trim all trailing zero bytes
		const auto firstZero = std::find(std::begin(results), std::end(results), 0);
		results.erase(firstZero, std::end(results));

		// Add what's left to our name
		deviceName.append(std::begin(results), std::end(results));

		// If nothing was left after trimming we likely have a device bug, e.g. a name containing
		// null bytes or a device name count that does not match the device name.
		if (results.size() == 0)
			break;	// Bail out to avoid endless loop
	}

	return deviceName;
}

IDeviceTypeName::DeviceType IDeviceTypeName::getDeviceType ()
{
	const auto results = call (GetDeviceType);
	if (results.size() < 1)
		throw std::runtime_error ("Invalid GetDeviceType response");
	return static_cast<DeviceType>(results[0]);
}

std::string IDeviceTypeName::getDeviceTypeString(DeviceType type)
{
	switch (type) {
		case DeviceType::Keyboard:               return "Keyboard";
		case DeviceType::RemoteControl:          return "Remote control";
		case DeviceType::Numpad:                 return "Numpad";
		case DeviceType::Mouse:                  return "Mouse";
		case DeviceType::Trackpad:               return "Trackpad";
		case DeviceType::Trackball:              return "Trackball";
		case DeviceType::Presenter:              return "Presenter";
		case DeviceType::Receiver:               return "Receiver";
		case DeviceType::Headset:                return "Headset";
		case DeviceType::Webcam:                 return "Webcam";
		case DeviceType::SteeringWheel:          return "Steering wheel";
		case DeviceType::Joystick:               return "Joystick";
		case DeviceType::Gamepad:                return "Gamepad";
		case DeviceType::Dock:                   return "Dock";
		case DeviceType::Speaker:                return "Speaker";
		case DeviceType::Microphone:             return "Microphone";
		case DeviceType::IlluminationLight:      return "Illumination light";
		case DeviceType::ProgrammableController: return "Programmable controller";
		case DeviceType::CarSimPedals:           return "Car sim pedals";
		case DeviceType::Adapter:                return "Adapter";
		default:
			return "<Unknown:" + std::to_string(type) + ">";
	};
}
