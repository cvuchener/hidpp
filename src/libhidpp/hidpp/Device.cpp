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

#include <hidpp/Device.h>

#include <hidpp10/Device.h>
#include <hidpp10/IReceiver.h>
#include <hidpp20/IRoot.h>
#include <hidpp10/Error.h>
#include <hidpp20/Error.h>
#include <misc/Log.h>

#include <algorithm>

using namespace HIDPP;

Device::NoHIDPPReportException::NoHIDPPReportException ()
{
}

const char *Device::NoHIDPPReportException::what () const noexcept
{
	return "No HID++ report";
}

static const std::array<uint8_t, 27> ShortReportDesc = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x01,		// Usage (0001 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x10,		//   Report ID (16)
	0x75, 0x08,		//   Report Size (8)
	0x95, 0x06,		//   Report Count (6)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

static const std::array<uint8_t, 27> LongReportDesc = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x02,		// Usage (0002 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x11,		//   Report ID (17)
	0x75, 0x08,		//   Report Size (8)
	0x95, 0x13,		//   Report Count (19)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

/* Alternative versions from the G602 */
static const std::array<uint8_t, 27> ShortReportDesc2 = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x01,		// Usage (0001 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x10,		//   Report ID (16)
	0x95, 0x06,		//   Report Count (6)
	0x75, 0x08,		//   Report Size (8)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x01,		//   Usage (0001 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

static const std::array<uint8_t, 27> LongReportDesc2 = {
	0x06, 0x00, 0xFF,	// Usage Page (FF00 - Vendor)
	0x09, 0x02,		// Usage (0002 - Vendor)
	0xA1, 0x01,		// Collection (Application)
	0x85, 0x11,		//   Report ID (17)
	0x95, 0x13,		//   Report Count (19)
	0x75, 0x08,		//   Report Size (8)
	0x15, 0x00,		//   Logical Minimum (0)
	0x26, 0xFF, 0x00,	//   Logical Maximum (255)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x81, 0x00,		//   Input (Data, Array, Absolute)
	0x09, 0x02,		//   Usage (0002 - Vendor)
	0x91, 0x00,		//   Output (Data, Array, Absolute)
	0xC0			// End Collection
};

Device::Device (const std::string &path, DeviceIndex device_index):
	HIDRaw (path), _device_index (device_index)
{
	const HIDRaw::ReportDescriptor &rdesc = getReportDescriptor ();
	if (rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 ShortReportDesc.begin (),
					 ShortReportDesc.end ()) &&
	    rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 ShortReportDesc2.begin (),
					 ShortReportDesc2.end ()))
		throw NoHIDPPReportException ();
	if (rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 LongReportDesc.begin (),
					 LongReportDesc.end ()) &&
	    rdesc.end () == std::search (rdesc.begin (), rdesc.end (),
					 LongReportDesc2.begin (),
					 LongReportDesc2.end ()))
		throw NoHIDPPReportException ();

	if (device_index >= WirelessDevice1 && device_index <= WirelessDevice6) {
		HIDPP10::Device ur (path, DefaultDevice);
		HIDPP10::IReceiver ireceiver (&ur);
		ireceiver.getDeviceInformation (device_index - 1,
						nullptr,
						nullptr,
						&_product_id,
						nullptr);
		_name = ireceiver.getDeviceName (device_index - 1);
	}
	else {
		_product_id = HIDRaw::productID ();
		_name = HIDRaw::name ();
	}
}

DeviceIndex Device::deviceIndex () const
{
	return _device_index;
}

void Device::getProtocolVersion (unsigned int &major, unsigned int &minor)
{
	constexpr int software_id = 1; // Must be a 4 bit unsigned value
	Report request (Report::Short,
			_device_index,
			HIDPP20::IRoot::index,
			HIDPP20::IRoot::Ping,
			software_id);
	sendReport (request);
	while (true) {
		Report response = getReport (true); // Time out if there is no valid response received fast enough

		if (response.deviceIndex () != _device_index) {
			Log::debug () << __FUNCTION__ << ": "
				      << "Ignored report with wrong device index"
				      << std::endl;
			continue;
		}

		uint8_t sub_id, address, error_code;
		if (response.checkErrorMessage10 (&sub_id, &address, &error_code)) {
			if (sub_id != HIDPP20::IRoot::index ||
			    address != (HIDPP20::IRoot::Ping << 4 | software_id)) {
				Log::debug () << __FUNCTION__ << ": "
					      << "Ignored error message with wrong subID or address"
					      << std::endl;
				continue;
			}
			if (error_code != HIDPP10::Error::InvalidSubID)
				throw HIDPP10::Error (static_cast<HIDPP10::Error::ErrorCode> (error_code));
			major = 1;
			minor = 0;
			return;
		}
		uint8_t feature_index;
		unsigned int function, sw_id;
		if (response.checkErrorMessage20 (&feature_index, &function, &sw_id, &error_code)) {
			if (feature_index != HIDPP20::IRoot::index || function != HIDPP20::IRoot::Ping || sw_id != software_id) {
				Log::debug () << __FUNCTION__ << ": "
					      << "Ignored error message with wrong feature/function/softwareID"
					      << std::endl;
				continue;
			}
			throw HIDPP20::Error (static_cast<HIDPP20::Error::ErrorCode> (error_code));
		}
		if (response.featureIndex () == HIDPP20::IRoot::index &&
		    response.function () == HIDPP20::IRoot::Ping &&
		    response.softwareID () == software_id) {
			major = response.params ()[0];
			minor = response.params ()[1];
			return;
		}
		Log::debug () << __FUNCTION__ << ": "
			      << "Ignored report with wrong feature/function/softwareID"
			      << std::endl;
	}
}

uint16_t Device::productID () const
{
	return _product_id;
}

std::string Device::name () const
{
	return _name;
}

int Device::sendReport (const Report &report)
{
	return writeReport (report.rawReport ());
}

Report Device::getReport (bool timeout)
{
	std::vector<uint8_t> raw_report;
	while (true) {
		try {
			raw_report.resize (Report::MaxDataLength+1);
			if (timeout)
				readReport (raw_report, 1);
			else
				readReport (raw_report);
			return Report (raw_report[0], &raw_report[1], raw_report.size () - 1);
		}
		catch (Report::InvalidReportID e) {
			// Ignore non-HID++ reports
			Log::debug () << __FUNCTION__ << ": "
				      << "Ignored non HID++ report" << std::endl;
			Log::printBytes (Log::Debug, "Ignored report:",
					 raw_report.begin (), raw_report.end ());
			continue;
		}
		catch (Report::InvalidReportLength e) {
			// Ignore non-HID++ reports
			Log::warning () << __FUNCTION__ << ": "
					<< "Invalid HID++ report length" << std::endl;
			Log::printBytes (Log::Warning, "Ignored report:",
					 raw_report.begin (), raw_report.end ());
			continue;
		}
	}
}
