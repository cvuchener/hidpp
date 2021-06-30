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
#include <cassert>

using namespace HIDPP;

Device::InvalidProtocolVersion::InvalidProtocolVersion (const std::tuple<unsigned int, unsigned int> &version)
{
	std::stringstream ss;
	unsigned int minor, major;
	std::tie (major, minor) = version;
	ss << "Invalid protocol version: " << major << "." << minor << std::endl;
	_msg = ss.str ();
}

const char *Device::InvalidProtocolVersion::what () const noexcept
{
	return _msg.c_str ();
}

Device::Device (Dispatcher *dispatcher, DeviceIndex device_index):
	_dispatcher (dispatcher), _device_index (device_index)
{
	bool is_wireless = device_index >= WirelessDevice1 && device_index <= WirelessDevice6;
	if (is_wireless) {
		// Ask receiver for device info when wireless
		HIDPP10::Device ur (_dispatcher, DefaultDevice);
		HIDPP10::IReceiver ireceiver (&ur);
		try {
			ireceiver.getDeviceInformation (device_index - 1,
							nullptr,
							nullptr,
							&_product_id,
							nullptr);
			_name = ireceiver.getDeviceName (device_index - 1);
		}
		catch (HIDPP10::Error &e) {
			if (e.errorCode () == HIDPP10::Error::InvalidValue) {
				// the invalid value is the device index
				throw HIDPP10::Error (HIDPP10::Error::UnknownDevice);
			}
			Log::error () << "Error while asking receiver for infos: " << e.what () << std::endl;
			throw;
		}
	}
	else {
		// Use HID info for corded devices
		_product_id = _dispatcher->productID ();
		_name = _dispatcher->name ();
	}

	// Check protocol version
	static constexpr unsigned int software_id = 1;
	auto type = _dispatcher->reportInfo ().findReport ();
	assert (type);
	Report request (*type, _device_index, HIDPP20::IRoot::index, HIDPP20::IRoot::Ping, software_id);
	auto response = _dispatcher->sendCommand (std::move (request));
	try {
		auto report = response->get (is_wireless ? 2000 : 500); // use longer timeout for wireless devices that can be sleeping.
		auto params = report.parameterBegin ();
		_version = std::make_tuple (params[0], params[1]);
	}
	catch (HIDPP10::Error &e) {
		// Valid HID++1.0 devices should send a "Invalid SubID" error.
		if (e.errorCode () != HIDPP10::Error::InvalidSubID)
			throw;
		_version = std::make_tuple (1, 0);
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
	return _version;
}
