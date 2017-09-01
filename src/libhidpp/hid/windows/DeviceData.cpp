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

#include "DeviceData.h"

#include "error_category.h"

DeviceData::DeviceData ()
{
}

DeviceData::DeviceData (HDEVINFO hdevinfo, const wchar_t *device_id)
{
	DWORD err;
	_info.cbSize = sizeof (SP_DEVINFO_DATA);
	if (!SetupDiOpenDeviceInfo (hdevinfo, device_id, 0, 0, &_info)) {
		err = GetLastError ();
		throw std::system_error (err, windows_category (),
					 "SetupDiOpenDeviceInfo");
	}
}

DeviceData::~DeviceData ()
{
}

DEVINST DeviceData::deviceInst () const
{
	return _info.DevInst;
}

std::tuple<bool, DEVINST> DeviceData::parentInst () const
{
	DEVINST parent;
	ULONG status, problem_number;
	CONFIGRET cr;
	cr = CM_Get_DevNode_Status (&status, &problem_number, _info.DevInst, 0);
	if (cr == CR_NO_SUCH_DEVINST)
		return std::make_tuple (false, parent);
	else if (cr != CR_SUCCESS)
		throw std::system_error (ERROR_UNIDENTIFIED_ERROR,
					 windows_category (), "CM_Get_DevNode_Status");

	cr = CM_Get_Parent (&parent, _info.DevInst, 0);
	if (cr != CR_SUCCESS)
		throw std::system_error (ERROR_UNIDENTIFIED_ERROR,
					 windows_category (), "CM_Get_Parent");
	return std::make_tuple (true, parent);
}

std::wstring DeviceData::getDeviceID (DEVINST inst)
{
	CONFIGRET cr;
	ULONG len;
	cr = CM_Get_Device_ID_Size (&len, inst, 0);
	if (cr != CR_SUCCESS)
		throw std::system_error (ERROR_UNIDENTIFIED_ERROR,
					 windows_category (), "CM_Get_Device_ID_Size");
	std::wstring id (len+1, L'\0');
	cr = CM_Get_Device_ID (inst, &id[0], id.size (), 0);
	if (cr != CR_SUCCESS)
		throw std::system_error (ERROR_UNIDENTIFIED_ERROR,
					 windows_category (), "CM_Get_Device_ID");
	id.resize (len);
	return id;
}

DeviceInterfaceData::DeviceInterfaceData (HDEVINFO hdevinfo, PSP_DEVICE_INTERFACE_DATA interface_data)
{
	DWORD len, err;
	if (!SetupDiGetDeviceInterfaceDetail (hdevinfo, interface_data, NULL, 0,
					      &len, NULL)) {
	    err = GetLastError ();
	    if (err != ERROR_INSUFFICIENT_BUFFER)
		    throw std::system_error (err, windows_category (),
					     "SetupDiGetDeviceInterfaceDetail");
	}
	_interface_detail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA> (operator new (len));
	_interface_detail->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
	_info.cbSize = sizeof (SP_DEVINFO_DATA);
	if (!SetupDiGetDeviceInterfaceDetail (hdevinfo, interface_data,
					      _interface_detail, len,
					      NULL, &_info)) {
		err = GetLastError ();
		throw std::system_error (err, windows_category (),
					 "SetupDiGetDeviceInterfaceDetail");
	}
}

DeviceInterfaceData::~DeviceInterfaceData ()
{
	operator delete (_interface_detail);
}

const wchar_t *DeviceInterfaceData::devicePath () const
{
	return _interface_detail->DevicePath;
}

DeviceEnumerator::DeviceEnumerator (const GUID *interface_class):
	_class (interface_class)
{
	_hdevinfo = SetupDiGetClassDevs (_class, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (_hdevinfo == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError ();
		throw std::system_error (err, windows_category (),
					 "SetupDiGetClassDevs");
	}
}

std::unique_ptr<DeviceInterfaceData> DeviceEnumerator::get (DWORD index)
{
	DWORD err;
	SP_DEVICE_INTERFACE_DATA interface_data;
	interface_data.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
	if (!SetupDiEnumDeviceInterfaces (_hdevinfo, NULL, _class,
					  index, &interface_data)) {
		err = GetLastError ();
		if (err == ERROR_NO_MORE_ITEMS)
			return std::unique_ptr<DeviceInterfaceData> ();
		else
			throw std::system_error (err, windows_category (),
						 "SetupDiEnumDeviceInterfaces");
	}
	return std::make_unique<DeviceInterfaceData> (_hdevinfo, &interface_data);
}


HDEVINFO DeviceEnumerator::devinfo () const
{
	return _hdevinfo;
}
