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
#include <hidpp10/Error.h>
#include <hidpp20/IRoot.h>
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
		_product_id = _dispatcher->productID ();
		_name = _dispatcher->name ();
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

std::tuple<unsigned int, unsigned int> Device::protocolVersion ()
{
	static constexpr unsigned int software_id = 1;
	Report request (Report::Short, _device_index, HIDPP20::IRoot::index, HIDPP20::IRoot::Ping, software_id);
	auto response = _dispatcher->sendCommand (std::move (request));
	try {
		auto report = response->get (500);
		auto params = report.parameterBegin ();
		return std::make_tuple (params[0], params[1]);
	}
	catch (HIDPP10::Error e) {
		if (e.errorCode () == HIDPP10::Error::InvalidSubID) // Expected error from a HID++ 1.0 device
			return std::make_tuple (1, 0);
		else
			throw e;
	}
}
