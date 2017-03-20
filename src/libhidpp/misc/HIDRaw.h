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

#ifndef HID_RAW_H
#define HID_RAW_H

#include <string>
#include <vector>

class HIDRaw
{
public:
	typedef std::basic_string<uint8_t> ReportDescriptor;

	HIDRaw (const std::string &path);
	HIDRaw (const HIDRaw &other);
	HIDRaw (HIDRaw &&other);
	~HIDRaw ();

	uint16_t vendorID () const;
	uint16_t productID () const;
	std::string name () const;
	const ReportDescriptor &getReportDescriptor () const;

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
	HIDRaw ();

	int _fd;
	int _pipe[2];
	uint16_t _vendor_id, _product_id;
	std::string _name;
	ReportDescriptor _report_desc;
};

#endif

