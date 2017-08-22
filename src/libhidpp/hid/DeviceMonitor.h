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

#ifndef LIBHIDPP_HID_DEVICE_MONITOR_H
#define LIBHIDPP_HID_DEVICE_MONITOR_H

#include <memory>

namespace HID
{

/**
 * Enumerates and monitors HID devices.
 *
 * Subclass and implement \ref addDevice and \ref removeDevice.
 *
 * "pathes" passed to \ref addDevice and \ref removeDevice are
 * implementation-dependant but should be compatible with the
 * constructor of the corresponding implementation of \ref HIDRaw.
 */
class DeviceMonitor
{
public:
	DeviceMonitor ();
	virtual ~DeviceMonitor ();

	/**
	 * Enumerate current devices.
	 *
	 * \ref addDevice will be called for each device currently connected.
	 */
	void enumerate ();

	/**
	 * Monitors HID devices.
	 *
	 * \ref addDevice and \ref removeDevice will be called a device is
	 * added or removed respectively. When starting it enumerates all
	 * devices that were already present.
	 */
	void run ();

	/**
	 * Stop monitoring devices.
	 *
	 * Calling this method will make \ref run exit its monitoring.
	 */
	void stop ();

protected:
	virtual void addDevice (const char *path) = 0;
	virtual void removeDevice (const char *path) = 0;

private:
	struct PrivateImpl;
	std::unique_ptr<PrivateImpl> _p;
};

}

#endif
