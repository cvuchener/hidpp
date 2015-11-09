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

#ifndef HIDPP_REPORT_H
#define HIDPP_REPORT_H

#include <hidpp/defs.h>
#include <hidpp/Parameters.h>

#include <array>
#include <vector>

namespace HIDPP
{

class Report
{
public:
	enum Type: uint8_t {
		Short = 0x10,
		Long = 0x11,
	};

	class InvalidReportID: std::exception
	{
	public:
		InvalidReportID ();
		virtual const char *what () const noexcept;
	};
	class InvalidReportLength: std::exception
	{
	public:
		InvalidReportLength ();
		virtual const char *what () const noexcept;
	};

	static constexpr std::size_t MaxDataLength = 19;
	
	/*
	 * Raw data constructor
	 */
	Report (uint8_t report_id, const uint8_t *data, std::size_t length);

	/*
	 * HID++ 1.0 constructors
	 */
	Report (Type type,
		DeviceIndex device_index,
		uint8_t sub_id,
		uint8_t address);
	Report (DeviceIndex device_index,
		uint8_t sub_id,
		uint8_t address,
		const Parameters &params);

	/*
	 * HID++ 2.0 constructors
	 */
	Report (Type type,
		DeviceIndex device_index,
		uint8_t feature_index,
		unsigned int function,
		unsigned int sw_id);
	Report (DeviceIndex device_index,
		uint8_t feature_index,
		unsigned int function,
		unsigned int sw_id,
		const Parameters &params);

	Type type () const;
	DeviceIndex deviceIndex () const;

	/*
	 * HID++ 1.0
	 */
	uint8_t subID () const;
	void setSubID (uint8_t sub_id);

	uint8_t address () const;
	void setAddress (uint8_t address);

	/*
	 * HID++ 2.0
	 */

	uint8_t featureIndex () const;
	void setfeatureIndex (uint8_t feature_index);

	unsigned int function () const;
	void setFunction (unsigned int function);

	unsigned int softwareID () const;
	void setSoftwareID (unsigned int sw_id);

	/*
	 * Parameters
	 */
	std::size_t paramLength () const;
	static std::size_t paramLength (Type type);
	
	Parameters &params ();
	const Parameters &params () const;

	/*
	 * Raw HID report (without the ID)
	 */
	const std::vector<uint8_t> rawReport () const;

	/*
	 * Error testing
	 */
	bool checkErrorMessage10 (uint8_t *sub_id, uint8_t *address, uint8_t *error_code) const;
	bool checkErrorMessage20 (uint8_t *feature_index, unsigned int *function, unsigned int *sw_id, uint8_t *error_code) const;

private:
	static constexpr std::size_t HeaderLength = 4;
	std::array<uint8_t, HeaderLength> _header;
	Parameters _params;
};

}

#endif

