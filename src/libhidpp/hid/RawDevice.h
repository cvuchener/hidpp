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

#ifndef LIBHIDPP_HID_RAW_DEVICE_H
#define LIBHIDPP_HID_RAW_DEVICE_H

#include <string>
#include <vector>
#include <memory>

#include <hid/ReportDescriptor.h>

namespace HID
{

class RawDevice
{
public:
	RawDevice (const std::string &path);
	RawDevice (const RawDevice &other);
	RawDevice (RawDevice &&other);
	~RawDevice ();

	inline uint16_t vendorID () const
	{
		return _vendor_id;
	}
	inline uint16_t productID () const
	{
		return _product_id;
	}
	const std::string &name () const
	{
		return _name;
	}
	const ReportDescriptor &getReportDescriptor () const
	{
		return _report_desc;
	}

	int writeReport (const std::vector<uint8_t> &report);

	/**
	 * \param[out]	report	HID report
	 * \param[in]	timeout	Time-out in milliseconds, negative for no timeout.
	 *
	 * \returns report size or 0 if interrupted or timed out.
	 */
	int readReport (std::vector<uint8_t> &report, int timeout = -1);

	/**
	 * Interrupts the current (or next) readReport call so it returns immediately.
	 */
	void interruptRead ();

private:
	RawDevice ();

	struct PrivateImpl;
	std::unique_ptr<PrivateImpl> _p;

	uint16_t _vendor_id, _product_id;
	std::string _name;
	ReportDescriptor _report_desc;

	void logReportDescriptor () const;
};

}

#endif

