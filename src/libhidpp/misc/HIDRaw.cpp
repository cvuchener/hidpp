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

#include <misc/HIDRaw.h>
#include <misc/Log.h>

#include <sstream>
#include <stdexcept>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>
}

HIDRaw::HIDRaw (const std::string &path)
{
	_fd = ::open (path.c_str (), O_RDWR);
	if (_fd == -1) {
		throw std::system_error (errno, std::system_category (), "open");
	}

	struct hidraw_devinfo di;
	if (-1 == ::ioctl (_fd, HIDIOCGRAWINFO, &di)) {
		int err = errno;
		::close (_fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRAWINFO");
	}
	_vendor_id = di.vendor;
	_product_id = di.product;

	char string[256];
	int ret;
	if (-1 == (ret = ::ioctl (_fd, HIDIOCGRAWNAME(sizeof(string)), string))) {
		int err = errno;
		::close (_fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRAWNAME");
	}
	_name.assign (string, ret);

	struct hidraw_report_descriptor rdesc;
	if (-1 == ::ioctl (_fd, HIDIOCGRDESCSIZE, &rdesc.size)) {
		int err = errno;
		::close (_fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRDESCSIZE");
	}
	if (-1 == ::ioctl (_fd, HIDIOCGRDESC, &rdesc)) {
		int err = errno;
		::close (_fd);
		throw std::system_error (err, std::system_category (), "HIDIOCGRDESC");
	}
	_report_desc = ReportDescriptor (rdesc.value, rdesc.size);

	if (-1 == ::pipe (_pipe)) {
		int err = errno;
		::close (_fd);
		throw std::system_error (err, std::system_category (), "pipe");
	}
}

HIDRaw::HIDRaw (const HIDRaw &other):
	_fd (::dup (other._fd)),
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_name (other._name),
	_report_desc (other._report_desc)
{
	if (-1 == _fd) {
		throw std::system_error (errno, std::system_category (), "dup");
	}
	if (-1 == ::pipe (_pipe)) {
		int err = errno;
		::close (_fd);
		throw std::system_error (err, std::system_category (), "pipe");
	}
}

HIDRaw::HIDRaw (HIDRaw &&other):
	_fd (other._fd),
	_pipe {other._pipe[0], other._pipe[1]},
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_name (std::move (other._name)),
	_report_desc (std::move (other._report_desc))
{
	other._fd = other._pipe[0] = other._pipe[1] = -1;
}

HIDRaw::~HIDRaw ()
{
	if (_fd != -1) {
		::close (_fd);
		::close (_pipe[0]);
		::close (_pipe[1]);
	}
}

uint16_t HIDRaw::vendorID () const
{
	return _vendor_id;
}

uint16_t HIDRaw::productID () const
{
	return _product_id;
}

std::string HIDRaw::name () const
{
	return _name;
}

const HIDRaw::ReportDescriptor &HIDRaw::getReportDescriptor () const
{
	return _report_desc;
}

int HIDRaw::writeReport (const std::vector<uint8_t> &report)
{
	int ret = write (_fd, report.data (), report.size ());
	if (ret == -1) {
		throw std::system_error (errno, std::system_category (), "write");
	}
	Log::debug ("report").printBytes ("Send HID report:", report.begin (), report.end ());
	return ret;
}

int HIDRaw::readReport (std::vector<uint8_t> &report, int timeout)
{
	int ret;
	timeval to = { timeout/1000, (timeout%1000) * 1000 };
	fd_set fds;
	do {
		FD_ZERO (&fds);
		FD_SET (_fd, &fds);
		FD_SET (_pipe[0], &fds);
		ret = select (std::max (_fd, _pipe[0])+1,
				&fds, nullptr, nullptr,
				(timeout < 0 ? nullptr : &to));
	} while (ret == -1 && errno == EINTR);
	if (ret == -1)
		throw std::system_error (errno, std::system_category (), "select");
	if (FD_ISSET (_fd, &fds)) {
		ret = read (_fd, report.data (), report.size ());
		if (ret == -1)
			throw std::system_error (errno, std::system_category (), "read");
		report.resize (ret);
		Log::debug ("report").printBytes ("Recv HID report:", report.begin (), report.end ());
		return ret;
	}
	if (FD_ISSET (_pipe[0], &fds)) {
		char c;
		ret = read (_pipe[0], &c, sizeof (char));
		if (ret == -1)
			throw std::system_error (errno, std::system_category (), "read pipe");
	}
	return 0;
}

void HIDRaw::interruptRead ()
{
	char c = 0;
	if (-1 == write (_pipe[1], &c, sizeof (char)))
		throw std::system_error (errno, std::system_category (), "write pipe");
}
