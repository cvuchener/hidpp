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
#include <misc/SysCallError.h>

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
		throw SysCallError ("open", errno, __PRETTY_FUNCTION__);
	}
	
	struct hidraw_devinfo di;
	if (-1 == ::ioctl (_fd, HIDIOCGRAWINFO, &di)) {
		throw SysCallError ("HIDIOCGRAWINFO", errno, __PRETTY_FUNCTION__);
	}
	_vendor_id = di.vendor;
	_product_id = di.product;

	char string[256];
	if (-1 == ::ioctl (_fd, HIDIOCGRAWNAME(sizeof(string)), string)) {
		throw SysCallError ("HIDIOCGRAWNAME", errno, __PRETTY_FUNCTION__);
	}
	_name = string;

	struct hidraw_report_descriptor rdesc;
	if (-1 == ::ioctl (_fd, HIDIOCGRDESCSIZE, &rdesc.size)) {
		throw SysCallError ("HIDIOCGRDESCSIZE", errno, __PRETTY_FUNCTION__);
	}
	if (-1 == ::ioctl (_fd, HIDIOCGRDESC, &rdesc)) {
		throw SysCallError ("HIDIOCGRDESC", errno, __PRETTY_FUNCTION__);
	}
	_report_desc = ReportDescriptor (rdesc.value, rdesc.size);
}

HIDRaw::HIDRaw (const HIDRaw &other):
	_fd (::dup (other._fd)),
	_vendor_id (other._vendor_id), _product_id (other._product_id),
	_report_desc (other._report_desc)
{
	if (-1 == _fd) {
		throw SysCallError ("dup", errno, __PRETTY_FUNCTION__);
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
		throw SysCallError ("write", errno, __PRETTY_FUNCTION__);
	}
	return ret;
}

int HIDRaw::readReport (std::vector<uint8_t> &report)
{
	int ret = read (_fd, report.data (), report.size ());
	if (ret == -1) {
		report.clear ();
		throw SysCallError ("read", errno, __PRETTY_FUNCTION__);
	}
	report.resize (ret);
	return ret;
}
	
