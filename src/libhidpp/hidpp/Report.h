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

#ifndef LIBHIDPP_HIDPP_REPORT_H
#define LIBHIDPP_HIDPP_REPORT_H

#include <hidpp/defs.h>
#include <hid/ReportDescriptor.h>

#include <array>
#include <vector>

namespace HIDPP
{

/**
 * Contains a HID++ report.
 *
 * \ingroup hidpp
 *
 * It can be used for both HID++ 1.0 or 2.0 reports.
 *
 * Common fields are:
 *  - Report type (see \ref Type)
 *  - Device index (see \ref DeviceIndex)
 *  - Parameters
 *
 * HID++ 1.0 fields are:
 *  - SubID
 *  - Address
 *
 * HID++ 2.0 fields are:
 *  - Feature index
 *  - Function index
 *  - Software ID
 */
class Report
{
	static constexpr std::size_t HeaderLength = 4;
public:
	enum Type: uint8_t {
		Short = 0x10,
		Long = 0x11,
		VeryLong = 0x12,
	};

	static inline constexpr std::size_t reportLength (Type type) noexcept {
		switch (type) {
		case Type::Short: return 7;
		case Type::Long: return 20;
		case Type::VeryLong: return 64;
		default: return 0;
		}
	}

	inline static constexpr std::size_t parameterLength (Type type) noexcept {
		return reportLength (type) - HeaderLength;
	}

	/**
	 * Exception for reports with invalid report ID.
	 */
	class InvalidReportID: std::exception
	{
	public:
		InvalidReportID ();
		virtual const char *what () const noexcept;
	};
	/**
	 * Exception for reports where the length is not the expected one
	 * from the report type.
	 */
	class InvalidReportLength: std::exception
	{
	public:
		InvalidReportLength ();
		virtual const char *what () const noexcept;
	};

	/**
	 * Build the report by copying the raw data.
	 *
	 * \param report_id	Report ID
	 * \param data		Raw report data
	 * \param length	Length of the \p data array.
	 *
	 * \throws InvalidReportID
	 * \throws InvalidReportLength
	 */
	Report (uint8_t report_id, const uint8_t *data, std::size_t length);

	/**
	 * Build the report by moving the raw data.
	 *
	 * \param data	Report data including the report ID in its first byte.
	 *
	 * \throws InvalidReportID
	 * \throws InvalidReportLength
	 */
	Report (std::vector<uint8_t> &&data);

	/**
	 * Access report type.
	 */
	Type type () const;
	/**
	 * Access report device index.
	 */
	DeviceIndex deviceIndex () const;

	/**
	 * \name HID++ 1.0
	 *
	 * \{
	 */

	/**
	 * HID++ 1.0 constructor from type.
	 *
	 * Parameters size is set from \p type and is filled with zeroes.
	 */
	Report (Type type,
		DeviceIndex device_index,
		uint8_t sub_id,
		uint8_t address);
	/**
	 * HID++ 1.0 constructor from parameters.
	 *
	 * The type of the report is guessed from \p params size.
	 *
	 * \throws InvalidReportLength
	 */
	Report (DeviceIndex device_index,
		uint8_t sub_id,
		uint8_t address,
		std::vector<uint8_t>::const_iterator param_begin,
		std::vector<uint8_t>::const_iterator param_end);

	/**
	 * Access HID++ 1.0 report subID.
	 */
	uint8_t subID () const;
	/**
	 * Set HID++ 1.0 report subID.
	 */
	void setSubID (uint8_t sub_id);

	/**
	 * Access HID++ 1.0 report address.
	 */
	uint8_t address () const;
	/**
	 * Set HID++ 1.0 report address.
	 */
	void setAddress (uint8_t address);

	/**
	 * Check if the report is a HID++ 1.0 error report.
	 *
	 * Each pointer can be null and is then ignored.
	 *
	 * \param sub_id	The subID of the report responsible for the error.
	 * \param address	The address of the report responsible for the error.
	 * \param error_code	The error code of the error.
	 *
	 * \return \c true if the report is a HID++ 1.0 error, \c false otherwise.
	 *
	 * \sa HIDPP10::Error
	 */
	bool checkErrorMessage10 (uint8_t *sub_id, uint8_t *address, uint8_t *error_code) const;

	/**\}*/

	/**
	 * \name HID++ 2.0
	 *
	 * \{
	 */

	/**
	 * HID++ 2.0 constructor from type.
	 *
	 * Parameters size is set from \p type and is filled with zeroes.
	 */
	Report (Type type,
		DeviceIndex device_index,
		uint8_t feature_index,
		unsigned int function,
		unsigned int sw_id);
	/**
	 * HID++ 2.0 constructor from parameters.
	 *
	 * The type of the report is guessed from \p params size.
	 *
	 * \throws InvalidReportLength
	 */
	Report (DeviceIndex device_index,
		uint8_t feature_index,
		unsigned int function,
		unsigned int sw_id,
		std::vector<uint8_t>::const_iterator param_begin,
		std::vector<uint8_t>::const_iterator param_end);

	/**
	 * Access HID++ 2.0 report feature index.
	 */
	uint8_t featureIndex () const;
	/**
	 * Set HID++ 2.0 report feature index.
	 */
	void setFeatureIndex (uint8_t feature_index);

	/**
	 * Access HID++ 2.0 report function.
	 */
	unsigned int function () const;
	/**
	 * Set HID++ 2.0 report function.
	 */
	void setFunction (unsigned int function);

	/**
	 * Access HID++ 2.0 software ID.
	 */
	unsigned int softwareID () const;
	/**
	 * Set HID++ 2.0 software ID.
	 */
	void setSoftwareID (unsigned int sw_id);

	/**
	 * Check if the report is a HID++ 2.0 error report.
	 *
	 * Each pointer can be null and is then ignored.
	 *
	 * \param feature_index	The feature index of the report responsible for the error.
	 * \param function	The function of the report responsible for the error.
	 * \param sw_id		The software ID of the report responsible for the error.
	 * \param error_code	The error code of the error.
	 *
	 * \return \c true if the report is a HID++ 2.0 error, \c false otherwise.
	 *
	 * \sa HIDPP20::Error
	 */
	bool checkErrorMessage20 (uint8_t *feature_index, unsigned int *function, unsigned int *sw_id, uint8_t *error_code) const;

	/**\}*/

	/**
	 * Get the parameter length of the report.
	 */
	std::size_t parameterLength () const;

	/** Begin iterator for parameters. */
	std::vector<uint8_t>::iterator parameterBegin ();
	/** Begin iterator for parameters. */
	std::vector<uint8_t>::const_iterator parameterBegin () const;
	/** End iterator for parameters. */
	std::vector<uint8_t>::iterator parameterEnd ();
	/** End iterator for parameters. */
	std::vector<uint8_t>::const_iterator parameterEnd () const;

	/**
	 * Get the raw HID report (without the ID).
	 */
	const std::vector<uint8_t> &rawReport () const;

private:
	std::vector<uint8_t> _data;
};

inline constexpr auto MaxReportLength = Report::reportLength (Report::VeryLong);
inline constexpr auto ShortParamLength = Report::parameterLength (Report::Short);
inline constexpr auto LongParamLength = Report::parameterLength (Report::Long);
inline constexpr auto VeryLongParamLength = Report::parameterLength (Report::VeryLong);

}

#endif

