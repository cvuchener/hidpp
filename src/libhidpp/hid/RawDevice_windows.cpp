/*
 * Copyright 2017 Clément Vuchener
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

#include "RawDevice.h"

#include <misc/Log.h>

#include <stdexcept>
#include <locale>
#include <codecvt>
#include <array>
#include <map>
#include <set>
#include <cstring>
#include <cassert>

extern "C" {
#include <windows.h>
#include <winbase.h>
#include <hidsdi.h>
}

#include "windows/error_category.h"
#include "windows/DeviceData.h"

#define ARRAY_SIZE(a) (sizeof (a)/sizeof ((a)[0]))

class RAII_HANDLE
{
	HANDLE _handle;

public:
	RAII_HANDLE ():
		_handle (INVALID_HANDLE_VALUE)
	{
	}

	RAII_HANDLE (HANDLE handle):
		_handle (handle)
	{
	}

	RAII_HANDLE (RAII_HANDLE &&other):
		_handle (other._handle)
	{
		other._handle = INVALID_HANDLE_VALUE;
	}

	~RAII_HANDLE ()
	{
		CloseHandle (_handle);
	}

	RAII_HANDLE &operator= (HANDLE handle)
	{
		if (_handle != INVALID_HANDLE_VALUE)
			CloseHandle (_handle);
		_handle = handle;
		return *this;
	}

	RAII_HANDLE &operator= (RAII_HANDLE &&handle)
	{
		std::swap (*this, handle);
		return *this;
	}

	inline operator HANDLE () const
	{
		return _handle;
	}
};

using namespace HID;

struct RawDevice::PrivateImpl
{
	struct Device
	{
		RAII_HANDLE file;
		RAII_HANDLE event;
		HIDP_CAPS caps;
	};
	std::vector<Device> devices;
	std::map<uint8_t, HANDLE> reports;
	RAII_HANDLE interrupted_event;
};

RawDevice::RawDevice ():
	_p (std::make_unique<PrivateImpl> ())
{
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

template<typename Caps, NTSTATUS (*GetCaps) (HIDP_REPORT_TYPE, Caps *, PUSHORT, PHIDP_PREPARSED_DATA)>
static std::set<uint8_t> readCaps (USHORT count, HIDP_REPORT_TYPE report_type, PHIDP_PREPARSED_DATA preparsed_data, ReportDescriptor &rdesc)
{
	if (count == 0)
		return {};
	auto windebug = Log::debug ("windows");
	std::vector<Caps> caps (count);
	USHORT len = count;
	if (HIDP_STATUS_SUCCESS != GetCaps (report_type, caps.data (), &len, preparsed_data)) {
		throw std::runtime_error ("HidP_GetButtonCaps failed for input reports");
	}

	std::set<uint8_t> report_ids;
	for (const auto &cap: caps) {
		report_ids.insert (cap.ReportID);

		// Hack around the lack of raw report descriptor
		if (cap.ReportID == 0x10
		    && cap.UsagePage == 0xff00
		    && !cap.IsRange && cap.NotRange.Usage == 0x0001) {
			rdesc.insert (rdesc.end (),
				      ShortReportDesc.begin (),
				      ShortReportDesc.end ());
		}
		if (cap.ReportID == 0x11
		    && cap.UsagePage == 0xff00
		    && !cap.IsRange && cap.NotRange.Usage == 0x0002) {
			rdesc.insert (rdesc.end (),
				      LongReportDesc.begin (),
				      LongReportDesc.end ());
		}
	}
	return report_ids;
}

RawDevice::RawDevice (const std::string &path):
	_p (std::make_unique<PrivateImpl> ())
{
	std::wstring_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>> wconv;
	std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> u16conv;

	std::wstring parent_id = wconv.from_bytes (path);

	DWORD err;

	GUID hid_guid;
	HidD_GetHidGuid (&hid_guid);

	bool first = true;
	DeviceEnumerator enumerator (&hid_guid);
	DEVINST parent_inst = DeviceData (enumerator.devinfo (), parent_id.c_str ()).deviceInst ();
	int i = 0;
	while (auto dev = enumerator.get (i++)) {
		bool ok;
		DEVINST parent;
		std::tie (ok, parent) = dev->parentInst ();
		if (!ok || parent != parent_inst)
			continue;

		HANDLE hdev = CreateFile (dev->devicePath (),
					  GENERIC_READ | GENERIC_WRITE,
					  FILE_SHARE_READ | FILE_SHARE_WRITE,
					  NULL, OPEN_EXISTING,
					  FILE_FLAG_OVERLAPPED, NULL);
		if (hdev == INVALID_HANDLE_VALUE) {
			err = GetLastError ();
			Log::debug () << "Failed to open device "
				      << wconv.to_bytes (dev->devicePath ()) << ": "
				      << windows_category ().message (err) << std::endl;
			continue;
		}

		HANDLE event = CreateEvent (NULL, TRUE, TRUE, NULL);
		if (event == INVALID_HANDLE_VALUE) {
			err = GetLastError ();
			throw std::system_error (err, windows_category (),
						 "CreateEvent");
		}

		_p->devices.push_back ({hdev, event});

		if (first) {
			HIDD_ATTRIBUTES attrs;
			if (!HidD_GetAttributes (hdev, &attrs)) {
				err = GetLastError ();
				throw std::system_error (err, windows_category (),
							 "HidD_GetAttributes");
			}
			_vendor_id = attrs.VendorID;
			_product_id = attrs.ProductID;

			char16_t buffer[256];
			if (!HidD_GetManufacturerString (hdev, buffer, ARRAY_SIZE (buffer))) {
				err = GetLastError ();
				throw std::system_error (err, windows_category (),
							 "HidD_GetManufacturerString");
			}
			_name = u16conv.to_bytes (buffer);
			if (!HidD_GetProductString (hdev, buffer, ARRAY_SIZE (buffer))) {
				err = GetLastError ();
				throw std::system_error (err, windows_category (),
							 "HidD_GetProductString");
			}
			_name += " " + u16conv.to_bytes (buffer);
			first = false;
		}

		PHIDP_PREPARSED_DATA preparsed_data;
		if (!HidD_GetPreparsedData (hdev, &preparsed_data)) {
			err = GetLastError ();
			throw std::system_error (err, windows_category (),
						 "HidD_GetPreparsedData");
		}

		HIDP_CAPS &caps = _p->devices.back ().caps;
		if (HIDP_STATUS_SUCCESS != HidP_GetCaps (preparsed_data, &caps)) {
			HidD_FreePreparsedData (preparsed_data);
			throw std::runtime_error ("HidP_GetCaps failed");
		}

		try {
			auto ib = readCaps<HIDP_BUTTON_CAPS, HidP_GetButtonCaps> (
					caps.NumberInputButtonCaps,
					HidP_Input, preparsed_data,
					_report_desc);
			auto iv = readCaps<HIDP_VALUE_CAPS, HidP_GetValueCaps> (
					caps.NumberInputValueCaps,
					HidP_Input, preparsed_data,
					_report_desc);
			auto ob = readCaps<HIDP_BUTTON_CAPS, HidP_GetButtonCaps> (
					caps.NumberOutputButtonCaps,
					HidP_Output, preparsed_data,
					_report_desc);
			auto ov = readCaps<HIDP_VALUE_CAPS, HidP_GetValueCaps> (
					caps.NumberOutputValueCaps,
					HidP_Output, preparsed_data,
					_report_desc);
			for (const auto &ids: { ib, iv, ob, ov }) {
				for (uint8_t id: ids) {
					auto ret = _p->reports.emplace (id, hdev);
					if (!ret.second && ret.first->second != hdev)
						throw std::runtime_error ("Same Report ID on different handle.");
				}
			}
		}
		catch (std::exception &) {
			HidD_FreePreparsedData (preparsed_data);
			throw;
		}

		HidD_FreePreparsedData (preparsed_data);
	}

	_p->interrupted_event = CreateEvent (NULL, FALSE, FALSE, NULL);
	if (_p->interrupted_event == INVALID_HANDLE_VALUE) {
		err = GetLastError ();
		throw std::system_error (err, windows_category (),
					 "CreateEvent");
	}
}

RawDevice::RawDevice (const RawDevice &other):
	_p (std::make_unique<PrivateImpl> ()),
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_name (other._name),
	_report_desc (other._report_desc)
{
	DWORD err;

	for (const auto &dev: other._p->devices) {
		HANDLE hdev = ReOpenFile (dev.file,
					  GENERIC_READ | GENERIC_WRITE,
					  FILE_SHARE_READ | FILE_SHARE_WRITE,
					  FILE_FLAG_OVERLAPPED);
		if (hdev == INVALID_HANDLE_VALUE) {
			err = GetLastError ();
			throw std::system_error (err, windows_category (),
						 "ReOpenFile");
		}

		HANDLE event = CreateEvent (NULL, TRUE, TRUE, NULL);
		if (event == INVALID_HANDLE_VALUE) {
			err = GetLastError ();
			throw std::system_error (err, windows_category (),
						 "CreateEvent");
		}

		_p->devices.push_back ({hdev, event});
	}

	_p->interrupted_event = CreateEvent (NULL, FALSE, FALSE, NULL);
	if (_p->interrupted_event == INVALID_HANDLE_VALUE) {
		err = GetLastError ();
		throw std::system_error (err, windows_category (),
					 "CreateEvent");
	}
}

RawDevice::RawDevice (RawDevice &&other):
	_p (std::make_unique<PrivateImpl> ()),
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_name (std::move (other._name)),
	_report_desc (std::move (other._report_desc))
{
	std::swap (_p, other._p);
}

RawDevice::~RawDevice ()
{
}

int RawDevice::writeReport (const std::vector<uint8_t> &report)
{
	DWORD err, written;
	OVERLAPPED overlapped;
	memset (&overlapped, 0, sizeof (OVERLAPPED));
	auto it = _p->reports.find (report[0]);
	if (it == _p->reports.end ())
		throw std::runtime_error ("Report ID not found.");
	if (!WriteFile (it->second, report.data (), report.size (),
			&written, &overlapped)) {
		err = GetLastError ();
		if (err == ERROR_IO_PENDING) {
			if (!GetOverlappedResult (it->second, &overlapped,
						  &written, TRUE)) {
				err = GetLastError ();
				throw std::system_error (err, windows_category (),
							 "GetOverlappedResult");
			}
		}
		else
			throw std::system_error (err, windows_category (), "WriteFile");
	}
	Log::debug ("report").printBytes ("Send HID report:", report.begin (), report.end ());
	return written;
}

class AsyncRead
{
	HANDLE file;
	OVERLAPPED overlapped;
	bool pending;

public:
	AsyncRead (HANDLE file, HANDLE event):
		file (file),
		pending (false)
	{
		memset (&overlapped, 0, sizeof (OVERLAPPED));
		overlapped.hEvent = event;
	}

	bool read (LPVOID buffer, DWORD size, LPDWORD read)
	{
		DWORD err;
		if (ReadFile (file, buffer, size, read, &overlapped))
			return pending = false;
		err = GetLastError ();
		if (err == ERROR_IO_PENDING)
			return pending = true;
		else
			throw std::system_error (err, windows_category (),
						 "ReadFile");
	}

	void finish (LPDWORD read)
	{
		DWORD err;
		pending = false;
		if (!GetOverlappedResult (file, &overlapped, read, FALSE)) {
			err = GetLastError ();
			throw std::system_error (err, windows_category (),
						 "GetOverlappedResult");
		}
	}

	~AsyncRead ()
	{
		DWORD err;
		if (pending && !CancelIoEx (file, &overlapped)) {
			err = GetLastError ();
			Log::error () << "Failed to cancel async read: "
				      << windows_category ().message (err)
				      << std::endl;
		}
	}
};

int RawDevice::readReport (std::vector<uint8_t> &report, int timeout)
{
	DWORD err, read, ret, i;
	assert (_p->interrupted_event != INVALID_HANDLE_VALUE);
	std::vector<HANDLE> handles = { _p->interrupted_event };
	std::vector<AsyncRead> reads; // vector destructor will automatically cancel pending IOs.
	reads.reserve (_p->devices.size ()); // Reserve memory so overlapped are not moved.

	for (auto &dev: _p->devices) {
		if (report.size () < dev.caps.InputReportByteLength)
			continue; // skip device with reports that would not fit in the buffer
		reads.emplace_back (dev.file, dev.event);
		if (!reads.back ().read (report.data (), report.size (), &read))
			goto report_read;
		handles.push_back (dev.event);
	}
	ret = WaitForMultipleObjects (handles.size (), handles.data (), FALSE,
				      (timeout < 0 ? INFINITE : timeout));
	switch (ret) {
	case WAIT_OBJECT_0: // interrupted
	case WAIT_TIMEOUT:
		return 0;
	case WAIT_FAILED:
		err = GetLastError ();
		throw std::system_error (err, windows_category (), "WaitForMultipleObject");
	default:
		i = ret-WAIT_OBJECT_0-1;
		if (i < 0 || i >= reads.size ())
			throw std::runtime_error ("Unexpected return value from WaitForMultipleObject");
		else
			reads[i].finish (&read);
	}
report_read:
	report.resize (read);
	Log::debug ("report").printBytes ("Recv HID report:", report.begin (), report.end ());
	return read;
}

void RawDevice::interruptRead ()
{
	DWORD err;
	if (!SetEvent (_p->interrupted_event)) {
		err = GetLastError ();
		throw std::system_error (err, windows_category (), "SetEvent");
	}
}
