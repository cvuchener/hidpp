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

#include "Device.h"

#include <hidpp/Dispatcher.h>
#include <hidpp10/Device.h>
#include <hidpp10/IReceiver.h>
#include <misc/Log.h>

#include <algorithm>

using namespace HIDPP;

Device::Device (Dispatcher *dispatcher, DeviceIndex device_index):
	_dispatcher (dispatcher), _device_index (device_index)
{
	if (device_index >= WirelessDevice1 && device_index <= WirelessDevice6) {
		HIDPP10::Device ur (_dispatcher, DefaultDevice);
		HIDPP10::IReceiver ireceiver (&ur);
		ireceiver.getDeviceInformation (device_index - 1,
						nullptr,
						nullptr,
						&_product_id,
						nullptr);
		_name = ireceiver.getDeviceName (device_index - 1);
	}
	else {
		auto &hidraw = _dispatcher->hidraw ();
		_product_id = hidraw.productID ();
		_name = hidraw.name ();
	}
}

Dispatcher *Device::dispatcher () const
{
	return _dispatcher;
}

DeviceIndex Device::deviceIndex () const
{
	return _device_index;
}

uint16_t Device::productID () const
{
	return _product_id;
}

std::string Device::name () const
{
	return _name;
}
