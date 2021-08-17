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

#ifndef LIBHIDPP_HIDPP20_ERROR_H
#define LIBHIDPP_HIDPP20_ERROR_H

#include <stdexcept>
#include <vector>

#include <hidpp20/defs.h>

namespace HIDPP20
{

class Error: public std::exception
{
public:
	enum ErrorCode: uint8_t {
		NoError = 0,
		Unknown = 1,
		InvalidArgument = 2,
		OutOfRange = 3,
		HWError = 4,
		LogitechInternal = 5,
		InvalidFeatureIndex = 6,
		InvalidFunctionID = 7,
		Busy = 8,
		Unsupported = 9,
		UnknownDevice = 10,
	};

	Error (uint8_t error_code);
	Error (uint8_t error_code, std::vector<uint8_t> error_data);

	virtual const char *what () const noexcept;
	uint8_t errorCode () const;
	const std::vector<uint8_t>& errorData () const { return _error_data; }

private:
	uint8_t _error_code;
	std::vector<uint8_t> _error_data;
};

}

#endif
