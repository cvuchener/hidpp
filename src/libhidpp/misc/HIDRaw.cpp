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
		throw std::system_error (errno, std::system_category (), "HIDIOCGRAWINFO");
	}
	_vendor_id = di.vendor;
	_product_id = di.product;

	char string[256];
	if (-1 == ::ioctl (_fd, HIDIOCGRAWNAME(sizeof(string)), string)) {
		throw std::system_error (errno, std::system_category (), "HIDIOCGRAWNAME");
	}
	_name = string;

	struct hidraw_report_descriptor rdesc;
	if (-1 == ::ioctl (_fd, HIDIOCGRDESCSIZE, &rdesc.size)) {
		throw std::system_error (errno, std::system_category (), "HIDIOCGRDESCSIZE");
	}
	if (-1 == ::ioctl (_fd, HIDIOCGRDESC, &rdesc)) {
		throw std::system_error (errno, std::system_category (), "HIDIOCGRDESC");
	}
	_report_desc = ReportDescriptor (rdesc.value, rdesc.size);
}

HIDRaw::HIDRaw (const HIDRaw &other):
	_fd (::dup (other._fd)),
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_report_desc (other._report_desc)
{
	if (-1 == _fd) {
		throw std::system_error (errno, std::system_category (), "dup");
	}
}

HIDRaw::~HIDRaw ()
{
	::close (_fd);
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
	Log::printBytes (Log::DebugReport, "Send HID report:",
			 report.begin (), report.end ());
	return ret;
}

int HIDRaw::readReport (std::vector<uint8_t> &report)
{
	int ret = read (_fd, report.data (), report.size ());
	if (ret == -1) {
		report.clear ();
		throw std::system_error (errno, std::system_category (), "read");
	}
	report.resize (ret);
	Log::printBytes (Log::DebugReport, "Recv HID report:",
			 report.begin (), report.end ());
	return ret;
}

int HIDRaw::readReport (std::vector<uint8_t> &report, int timeout)
{
	int ret;
	timeval to = { timeout, 0 };
	fd_set fds;
	FD_ZERO (&fds);
	FD_SET (_fd, &fds);
	ret = select (_fd+1, &fds, nullptr, nullptr, &to);
	if (ret == -1) {
		report.clear ();
		throw std::system_error (errno, std::system_category (), "select");
	}
	if (FD_ISSET (_fd, &fds))
		return readReport (report);
	throw TimeoutError ();
}

const char *HIDRaw::TimeoutError::what () noexcept
{
	return "readReport timed out";
}
