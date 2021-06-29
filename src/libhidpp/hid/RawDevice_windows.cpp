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

static const int8_t preparse_data_header_magic[8] = {'H', 'i', 'd', 'P', ' ', 'K', 'D', 'R'};
struct preparsed_data_header
{
	int8_t magic[8];
	uint16_t usage, usage_page;
	uint16_t unk1[3];
	uint16_t input_item_count;
	uint16_t unk2;
	uint16_t input_report_byte_length;
	uint16_t unk3;
	uint16_t output_item_count;
	uint16_t unk4;
	uint16_t output_report_byte_length;
	uint16_t unk5;
	uint16_t feature_item_count;
	uint16_t item_count;
	uint16_t feature_report_byte_length;
	uint16_t size_bytes;
	uint16_t unk6;
};
static_assert(sizeof(preparsed_data_header) == 44);
struct preparsed_data_item
{
	uint16_t usage_page;
	uint8_t report_id;
	uint8_t bit_index;
	uint16_t bit_size;
	uint16_t report_count;
	uint16_t byte_index;
	uint16_t bit_count;
	uint32_t bit_field;
	uint32_t unk1;
	uint16_t link_usage_page, link_usage;
	uint32_t unk2[9];
	uint16_t usage_minimum, usage_maximum;
	uint16_t string_minimum, string_maximum;
	uint16_t designator_minimum, designator_maximum;
	uint16_t data_index_minimum, data_index_maximum;
	uint32_t unk3;
	int32_t logical_minimum, logical_maximum;
	int32_t physical_minimum, physical_maximum;
	uint32_t unit, unit_exponent;
};
static_assert(sizeof(preparsed_data_item) == 104);

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
			Log::debug ("hid").printf ("Opened device \"%s\" (%04x:%04x)\n",
					_name.c_str (), _vendor_id, _product_id);
			first = false;
		}

		PHIDP_PREPARSED_DATA preparsed_data;
		if (!HidD_GetPreparsedData (hdev, &preparsed_data)) {
			err = GetLastError ();
			throw std::system_error (err, windows_category (),
						 "HidD_GetPreparsedData");
		}

		std::unique_ptr<_HIDP_PREPARSED_DATA, decltype(&HidD_FreePreparsedData)> unique_preparsed_data (preparsed_data, &HidD_FreePreparsedData);

		auto header = reinterpret_cast<const preparsed_data_header *> (preparsed_data);
		auto item = reinterpret_cast<const preparsed_data_item *> (header+1);

		auto &collection = _report_desc.collections.emplace_back ();
		collection.usage = {header->usage_page, header->usage};

		for (auto [report_type, item_count]: {
				std::make_tuple (ReportID::Type::Input, header->input_item_count),
				std::make_tuple (ReportID::Type::Output, header->output_item_count),
				std::make_tuple (ReportID::Type::Feature, header->feature_item_count)}) {
			std::map<ReportID, int> last_bit_pos; // byte_index * 8 + bit_index
			for (int i = 0; i < item_count; ++i, ++item) {
				ReportID id = {report_type, item->report_id};
				auto [pos_it, pos_inserted] = last_bit_pos.emplace (id, -1);
				auto [it, report_inserted] = collection.reports.emplace (id, 0);
				int pos = item->byte_index * 8 + item->bit_index;
				if (pos_it->second != -1 && pos_it->second >= pos) {
					// Split items are in reverse order, try merging them if the position goes backward
					auto &f = it->second.back ();
					auto usages = std::get_if<std::vector<Usage>> (&f.usages);
					if (!usages || item->usage_minimum != item->usage_maximum) {
						Log::error ("reportdesc") << "Split item is a range item" << std::endl;
						continue;
					}
					auto item_usage = Usage {item->usage_page, item->usage_minimum};
					if (item->bit_size != f.size || item->bit_field != f.flags.bits)
						Log::error ("reportdesc") << "Split item mismatch" << std::endl;
					if (pos_it->second == pos) {
						if (item->report_count != f.count)
							Log::error ("reportdesc") << "Split item report count mismatch" << std::endl;
						usages->insert (usages->begin (), item_usage);
					}
					else {
						if (pos_it->second - pos != item->bit_size)
							Log::error ("reportdesc") << "Split item unexpected position" << std::endl;
						f.count += item->report_count;
						if (usages->size () > 1 || usages->front () != item_usage)
							usages->insert (usages->begin (), item_usage);
					}
				}
				else {
					// Create new item
					auto &f = it->second.emplace_back ();
					f.flags.bits = item->bit_field;
					f.count = item->report_count;
					f.size = item->bit_size;
					if (item->usage_minimum == item->usage_maximum)
						f.usages = std::vector {Usage {item->usage_page, item->usage_minimum}};
					else
						f.usages = std::make_pair (
								Usage {item->usage_page, item->usage_minimum},
								Usage {item->usage_page, item->usage_maximum});
				}
				pos_it->second = pos;
			}
		}

		for (const auto &[id, fields]: collection.reports) {
			auto [it, inserted] = _p->reports.emplace (id.id, hdev);
			if (!inserted && it->second != hdev)
				throw std::runtime_error ("Same Report ID on different handle.");
		}
	}
	logReportDescriptor ();


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
