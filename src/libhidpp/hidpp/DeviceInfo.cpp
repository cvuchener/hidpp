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

#include <hidpp/DeviceInfo.h>

#include <hidpp/ids.h>
#include <hidpp10/DeviceInfo.h>

using namespace HIDPP;

DeviceInfo ReceiverInfo (
	DeviceInfo::Receiver
);

HIDPP10::MouseInfo G5Info (
	&HIDPP10::ListSensor::S6006,
	HIDPP10::IResolutionType0,
	HIDPP10::NoProfile
);

HIDPP10::MouseInfo G9Info = {
	&HIDPP10::ListSensor::S6090,
	HIDPP10::IResolutionType0,
	HIDPP10::G9ProfileType
};

HIDPP10::MouseInfo G9xInfo = {
	&HIDPP10::RangeSensor::S9500,
	HIDPP10::IResolutionType3,
	HIDPP10::G500ProfileType
};

HIDPP10::MouseInfo G500Info = {
	&HIDPP10::RangeSensor::S9500,
	HIDPP10::IResolutionType3,
	HIDPP10::G500ProfileType
};

HIDPP10::MouseInfo G500sInfo = {
	&HIDPP10::RangeSensor::S9808,
	HIDPP10::IResolutionType3,
	HIDPP10::G500ProfileType
};

HIDPP10::MouseInfo G700Info = {
	&HIDPP10::RangeSensor::S9500,
	HIDPP10::IResolutionType3,
	HIDPP10::G700ProfileType
};

HIDPP10::MouseInfo G700sInfo = {
	&HIDPP10::RangeSensor::S9808,
	HIDPP10::IResolutionType3,
	HIDPP10::G700ProfileType
};

const DeviceInfo *HIDPP::getDeviceInfo (uint16_t product_id)
{
	switch (product_id) {
	case 0xc52b: // Unifying receiver
	case 0xc52f: // Nano receiver advanced
	case 0xc531: // G700 receiver
	case 0xc532: // Unifying receiver
	case 0xc537: // G602 receiver
		return &ReceiverInfo;

	case ID::G5:
	case ID::G5_2007:
	case ID::G7:
		return &G5Info;

	case ID::G9:
		return &G9Info;

	case ID::G9x:
	case ID::G9x_MW3:
		return &G9xInfo;

	case ID::G500:
		return &G500Info;

	case ID::G500s:
		return &G500sInfo;

	case ID::G700:
		return &G700Info;

	case ID::G700s:
		return &G700sInfo;

	default:
		return nullptr;
	}
}
