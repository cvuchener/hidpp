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

#include <hidpp/Report.h>

#include <hidpp10/defs.h>
#include <hidpp20/defs.h>

using namespace HIDPP;

Report::InvalidReportID::InvalidReportID ()
{
}

const char *Report::InvalidReportID::what () const noexcept
{
	return "Invalid Report ID for a HID++ report";
}

Report::InvalidReportLength::InvalidReportLength ()
{
}

const char *Report::InvalidReportLength::what () const noexcept
{
	return "Invalid Report Length for a HID++ report";
}

// Offsets
#define TYPE	0
#define DEVICE_INDEX	1
#define SUB_ID	2
#define ADDRESS	3

Report::Report (uint8_t type, const uint8_t *data, std::size_t length):
	_header ({ type })
{
	switch (static_cast<Type> (type)) {
	case Short:
		_params.resize (ShortParamLength);
		break;
	case Long:
		_params.resize (LongParamLength);
		break;
	default:
		throw InvalidReportID ();
	}
	if (length != Report::HeaderLength-1+_params.size ())
		throw InvalidReportLength ();

	std::copy (data, data+HeaderLength-1, &_header[1]);
	std::copy (data+HeaderLength-1, data+length, _params.begin ());
}

Report::Report (Type type,
		HIDPP::DeviceIndex device_index,
		uint8_t sub_id,
		uint8_t address):
	_header({ type, device_index, sub_id, address })
{
	switch (type) {
	case Short:
		_params.resize (ShortParamLength, 0);
		break;
	case Long:
		_params.resize (LongParamLength, 0);
		break;
	}
}

Report::Report (HIDPP::DeviceIndex device_index,
		uint8_t sub_id,
		uint8_t address,
		const std::vector<uint8_t> &params):
	_header({ 0, device_index, sub_id, address }),
	_params (params)
{
	switch (params.size ()) {
	case ShortParamLength:
		_header[TYPE] = Short;
		break;
	case LongParamLength:
		_header[TYPE] = Long;
		break;
	default:
		throw InvalidReportLength ();
	}
}

Report::Report (Type type,
		DeviceIndex device_index,
		uint8_t feature_index,
		unsigned int function,
		unsigned int sw_id):
	_header({ type, device_index, feature_index,
		  static_cast<uint8_t> ((function & 0x0F) << 4 | (sw_id & 0x0F)) })
{
	switch (type) {
	case Short:
		_params.resize (ShortParamLength, 0);
		break;
	case Long:
		_params.resize (LongParamLength, 0);
		break;
	}
}

Report::Report (DeviceIndex device_index,
		uint8_t feature_index,
		unsigned int function,
		unsigned int sw_id,
		const std::vector<uint8_t> &params):
	_header({ 0, device_index, feature_index,
		  static_cast<uint8_t> ((function & 0x0F) << 4 | (sw_id & 0x0F)) }),
	_params (params)
{
	switch (params.size ()) {
	case ShortParamLength:
		_header[TYPE] = Short;
		break;
	case LongParamLength:
		_header[TYPE] = Long;
		break;
	default:
		throw InvalidReportLength ();
	}
} 

Report::Type Report::type () const
{
	return static_cast<Type> (_header[TYPE]);
}

DeviceIndex Report::deviceIndex () const
{
	return static_cast<HIDPP::DeviceIndex> (_header[DEVICE_INDEX]);
}

uint8_t Report::subID () const
{
	return _header[SUB_ID];
}

void Report::setSubID (uint8_t sub_id)
{
	_header[SUB_ID] = sub_id;
}

uint8_t Report::address () const
{
	return _header[ADDRESS];
}

void Report::setAddress (uint8_t address)
{
	_header[ADDRESS] = address;
}

uint8_t Report::featureIndex () const
{
	return _header[SUB_ID];
}

void Report::setfeatureIndex (uint8_t feature_index)
{
	_header[SUB_ID] = feature_index;
}

unsigned int Report::function () const
{
	return (_header[ADDRESS] & 0xF0) >> 4;
}

void Report::setFunction (unsigned int function)
{
	_header[ADDRESS] = (function & 0x0F) << 4 | (_header[ADDRESS] & 0x0F);
}

unsigned int Report::softwareID () const
{
	return _header[ADDRESS] & 0x0F;
}

void Report::setSoftwareID (unsigned int sw_id)
{
	_header[ADDRESS] = (_header[ADDRESS] & 0xF0) | (sw_id & 0x0F);
}

std::size_t Report::paramLength () const
{
	return paramLength (static_cast<Type> (_header[TYPE]));
}

std::size_t Report::paramLength (Type type)
{
	switch (type) {
	case Short:
		return ShortParamLength;
	case Long:
		return LongParamLength;
	default:
		throw std::logic_error ("Invalid type");
	}
}

std::vector<uint8_t> &Report::params ()
{
	return _params;
}

const std::vector<uint8_t> &Report::params () const
{
	return _params;
}

const std::vector<uint8_t> Report::rawReport () const
{
	std::vector<uint8_t> report (HeaderLength + _params.size ());
	std::copy (_header.begin (), _header.end (), report.begin ());
	std::copy (_params.begin (), _params.end (), report.begin () + HeaderLength);
	return report;
}

bool Report::checkErrorMessage10 (uint8_t *sub_id,
				  uint8_t *address,
				  uint8_t *error_code) const
{
	if (static_cast<Type> (_header[TYPE]) != Short ||
	    _header[SUB_ID] != HIDPP10::ErrorMessage)
		return false;
	
	if (sub_id)
		*sub_id = _header[3];
	if (address)
		*address = _params[0];
	if (error_code)
		*error_code = _params[1];
	return true;
}

bool Report::checkErrorMessage20 (uint8_t *feature_index,
				  unsigned int *function,
				  unsigned int *sw_id,
				  uint8_t *error_code) const
{
	if (static_cast<Type> (_header[TYPE]) != Long ||
	    _header[SUB_ID] != HIDPP20::ErrorMessage)
		return false;
	
	if (feature_index)
		*feature_index = _header[3];
	if (function)
		*function = (_params[0] & 0xF0) >> 4;
	if (sw_id)
		*sw_id = _params[0] & 0x0F;
	if (error_code)
		*error_code = _params[1];
	return true;
}

