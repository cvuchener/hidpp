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

#include "DeviceMonitor.h"

#include <misc/Log.h>

#include <string>
#include <locale>
#include <codecvt>
#include <condition_variable>
#include <set>

extern "C" {
#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
}

#include "windows/error_category.h"
#include "windows/DeviceData.h"

using namespace HID;

struct DeviceMonitor::PrivateImpl
{
	std::mutex mutex;
	std::condition_variable cond;
};

DeviceMonitor::DeviceMonitor ():
	_p (std::make_unique<PrivateImpl> ())
{
}

DeviceMonitor::~DeviceMonitor ()
{
}

void DeviceMonitor::enumerate ()
{
	std::wstring_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>> wconv;

	GUID hid_guid;
	HidD_GetHidGuid (&hid_guid);

	std::set<std::wstring> ids;
	DeviceEnumerator enumerator (&hid_guid);
	int i = 0;
	while (auto dev = enumerator.get (i++)) {
		bool ok;
		DEVINST parent;
		std::tie (ok, parent) = dev->parentInst ();
		if (!ok)
			continue;
		auto ret = ids.emplace (DeviceData::getDeviceID (parent));
		if (ret.second)
			addDevice (wconv.to_bytes (*ret.first).c_str ());
	}
}

void DeviceMonitor::run ()
{
	enumerate ();

	Log::warning () << "Hot plug monitoring not supported" << std::endl;

	std::unique_lock<std::mutex> lock (_p->mutex);
	_p->cond.wait (lock);
}

void DeviceMonitor::stop ()
{
	_p->cond.notify_all ();
}
