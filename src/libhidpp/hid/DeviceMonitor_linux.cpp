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

extern "C" {
#include <unistd.h>
#include <sys/select.h>
#include <libudev.h>
}

using namespace HID;

struct DeviceMonitor::PrivateImpl
{
	struct udev *ctx;
	int pipe[2];
};

DeviceMonitor::DeviceMonitor ():
	_p (std::make_unique<PrivateImpl> ())
{
	if (-1 == pipe (_p->pipe))
		throw std::system_error (errno, std::system_category (), "pipe");

	_p->ctx = udev_new ();
	if (!_p->ctx)
		throw std::runtime_error ("udev_new failed");
}

DeviceMonitor::~DeviceMonitor ()
{
	udev_unref (_p->ctx);
	for (int i = 0; i < 2; ++i)
		close (_p->pipe[i]);
}

void DeviceMonitor::enumerate ()
{
	int ret;

	struct udev_enumerate *enumerator = udev_enumerate_new (_p->ctx);
	if (!enumerator)
		throw std::runtime_error ("udev_enumerate_new failed");

	ret = udev_enumerate_add_match_subsystem (enumerator, "hidraw");
	if (0 != ret)
		throw std::system_error (-ret, std::system_category (),
					 "udev_enumerate_add_match_subsystem");

	ret = udev_enumerate_scan_devices (enumerator);
	if (0 != ret)
		throw std::system_error (-ret, std::system_category (),
					 "udev_enumerate_scan_devices");

	struct udev_list_entry *current;
	udev_list_entry_foreach (current, udev_enumerate_get_list_entry (enumerator)) {
		const char *name = udev_list_entry_get_name (current);
		struct udev_device *device = udev_device_new_from_syspath (_p->ctx, name);
		addDevice (udev_device_get_devnode (device));
		udev_device_unref (device);
	}
	udev_enumerate_unref (enumerator);
}

void DeviceMonitor::run ()
{
	int ret;

	struct udev_monitor *monitor = udev_monitor_new_from_netlink (_p->ctx, "udev");
	if (!monitor)
		throw std::runtime_error ("udev_monitor_new_from_netlink failed");

	ret = udev_monitor_filter_add_match_subsystem_devtype (monitor, "hidraw", nullptr);
	if (0 != ret)
		throw std::system_error (-ret, std::system_category (),
					 "udev_monitor_filter_add_match_subsystem_devtype");

	ret = udev_monitor_enable_receiving (monitor);
	if (0 != ret)
		throw std::system_error (-ret, std::system_category (),
					 "udev_monitor_enable_receiving");

	enumerate ();

	int fd = udev_monitor_get_fd (monitor);
	while (true) {
		fd_set fds;
		FD_ZERO (&fds);
		FD_SET (_p->pipe[0], &fds);
		FD_SET (fd, &fds);
		if (-1 == select (std::max (_p->pipe[0], fd)+1, &fds, nullptr, nullptr, nullptr)) {
			if (errno == EINTR)
				continue;
			throw std::system_error (errno, std::system_category (), "select");
		}
		if (FD_ISSET (fd, &fds)) {
			struct udev_device *device = udev_monitor_receive_device (monitor);
			std::string action = udev_device_get_action (device);
			if (action == "add")
				addDevice (udev_device_get_devnode (device));
			else if (action == "remove")
				removeDevice (udev_device_get_devnode (device));
			udev_device_unref (device);
		}
		if (FD_ISSET (_p->pipe[0], &fds)) {
			char c;
			if (-1 == read (_p->pipe[0], &c, sizeof (char)))
				throw std::system_error (errno, std::system_category (),
							 "read pipe");
			break;
		}
	}

	udev_monitor_unref (monitor);
}

void DeviceMonitor::stop ()
{
	char c = 0;
	if (-1 == write (_p->pipe[1], &c, sizeof (char)))
		throw std::system_error (errno, std::system_category (), "write pipe");
}
