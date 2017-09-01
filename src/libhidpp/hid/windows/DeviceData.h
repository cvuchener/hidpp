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

#ifndef LIBHIDPP_HID_WINDOWS_DEVICE_DATA_H
#define LIBHIDPP_HID_WINDOWS_DEVICE_DATA_H

#include <string>
#include <memory>

extern "C" {
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
}

class DeviceData
{
public:
	DeviceData (HDEVINFO hdevinfo, const wchar_t *device_id);
	virtual ~DeviceData ();

	DEVINST deviceInst () const;
	std::tuple<bool, DEVINST> parentInst () const;

	static std::wstring getDeviceID (DEVINST inst);

protected:
	DeviceData ();

	SP_DEVINFO_DATA _info;
};

class DeviceInterfaceData: public DeviceData
{
public:
	DeviceInterfaceData (HDEVINFO hdevinfo, PSP_DEVICE_INTERFACE_DATA interface_data);
	~DeviceInterfaceData ();

	const wchar_t *devicePath () const;

private:
	PSP_DEVICE_INTERFACE_DETAIL_DATA _interface_detail;
};

class DeviceEnumerator
{
public:
	DeviceEnumerator (const GUID *interface_class);

	std::unique_ptr<DeviceInterfaceData> get (DWORD index);

	HDEVINFO devinfo () const;

private:
	const GUID *_class;
	HDEVINFO _hdevinfo;
};

#endif
