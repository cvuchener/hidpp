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

#include "RawDevice.h"

#include <misc/Log.h>

#include <stdexcept>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/hidraw.h>
}

using namespace HID;

struct RawDevice::PrivateImpl
{
	int fd;
	int pipe[2];
};

RawDevice::RawDevice ():
	_p (std::make_unique<PrivateImpl> ())
{
	_p->fd = _p->pipe[0] = _p->pipe[1] = -1;
}

RawDevice::RawDevice (const std::string &path):
	_p (std::make_unique<PrivateImpl> ())
{
	_p->fd = ::open (path.c_str (), O_RDWR);
	if (_p->fd == -1) {
		throw std::system_error (errno, std::system_category (), "open");
	}

	struct hidraw_devinfo di;
	if (-1 == ::ioctl (_p->fd, HIDIOCGRAWINFO, &di)) {
		int err = errno;
		::close (_p->fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRAWINFO");
	}
	_vendor_id = di.vendor;
	_product_id = di.product;

	char string[256];
	int ret;
	if (-1 == (ret = ::ioctl (_p->fd, HIDIOCGRAWNAME(sizeof(string)), string))) {
		int err = errno;
		::close (_p->fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRAWNAME");
	}
	_name.assign (string, ret-1); // HIDIOCGRAWNAME result includes null terminator
	Log::debug ("hid").printf ("Opened device \"%s\" (%04x:%04x)\n",
			_name.c_str (), _vendor_id, _product_id);

	struct hidraw_report_descriptor rdesc;
	if (-1 == ::ioctl (_p->fd, HIDIOCGRDESCSIZE, &rdesc.size)) {
		int err = errno;
		::close (_p->fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRDESCSIZE");
	}
	if (-1 == ::ioctl (_p->fd, HIDIOCGRDESC, &rdesc)) {
		int err = errno;
		::close (_p->fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRDESC");
	}
	try {
		_report_desc = ReportDescriptor::fromRawData (rdesc.value, rdesc.size);
		logReportDescriptor ();
	}
	catch (std::exception &e) {
		Log::error () << "Invalid report descriptor: " << e.what () << std::endl;
	}

	if (-1 == ::pipe (_p->pipe)) {
		int err = errno;
		::close (_p->fd);
		throw std::system_error (err, std::system_category (), "pipe");
	}
}

RawDevice::RawDevice (const RawDevice &other):
	_p (std::make_unique<PrivateImpl> ()),
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_name (other._name),
	_report_desc (other._report_desc)
{
	_p->fd = ::dup (other._p->fd);
	if (-1 == _p->fd) {
		throw std::system_error (errno, std::system_category (), "dup");
	}
	if (-1 == ::pipe (_p->pipe)) {
		int err = errno;
		::close (_p->fd);
		throw std::system_error (err, std::system_category (), "pipe");
	}
}

RawDevice::RawDevice (RawDevice &&other):
	_p (std::make_unique<PrivateImpl> ()),
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_name (std::move (other._name)),
	_report_desc (std::move (other._report_desc))
{
	_p->fd = other._p->fd;
	_p->pipe[0] = other._p->pipe[0];
	_p->pipe[1] = other._p->pipe[1];
	other._p->fd = other._p->pipe[0] = other._p->pipe[1] = -1;
}

RawDevice::~RawDevice ()
{
	if (_p->fd != -1) {
		::close (_p->fd);
		::close (_p->pipe[0]);
		::close (_p->pipe[1]);
	}
}

int RawDevice::writeReport (const std::vector<uint8_t> &report)
{
	int ret = write (_p->fd, report.data (), report.size ());
	if (ret == -1) {
		throw std::system_error (errno, std::system_category (), "write");
	}
	Log::debug ("report").printBytes ("Send HID report:", report.begin (), report.end ());
	return ret;
}

int RawDevice::readReport (std::vector<uint8_t> &report, int timeout)
{
	int ret;
	timeval to = { timeout/1000, (timeout%1000) * 1000 };
	fd_set fds;
	do {
		FD_ZERO (&fds);
		FD_SET (_p->fd, &fds);
		FD_SET (_p->pipe[0], &fds);
		ret = select (std::max (_p->fd, _p->pipe[0])+1,
				&fds, nullptr, nullptr,
				(timeout < 0 ? nullptr : &to));
	} while (ret == -1 && errno == EINTR);
	if (ret == -1)
		throw std::system_error (errno, std::system_category (), "select");
	if (FD_ISSET (_p->fd, &fds)) {
		ret = read (_p->fd, report.data (), report.size ());
		if (ret == -1)
			throw std::system_error (errno, std::system_category (), "read");
		report.resize (ret);
		Log::debug ("report").printBytes ("Recv HID report:", report.begin (), report.end ());
		return ret;
	}
	if (FD_ISSET (_p->pipe[0], &fds)) {
		char c;
		ret = read (_p->pipe[0], &c, sizeof (char));
		if (ret == -1)
			throw std::system_error (errno, std::system_category (), "read pipe");
	}
	return 0;
}

void RawDevice::interruptRead ()
{
	char c = 0;
	if (-1 == write (_p->pipe[1], &c, sizeof (char)))
		throw std::system_error (errno, std::system_category (), "write pipe");
}
