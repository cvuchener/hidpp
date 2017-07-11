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

#ifndef LIBHIDPP_HIDPP10_ERROR_H
#define LIBHIDPP_HIDPP10_ERROR_H

#include <stdexcept>

#include <hidpp10/defs.h>

namespace HIDPP10
{

class Error: public std::exception
{
public:
	enum ErrorCode: uint8_t {
		Success = 0x00,
		InvalidSubID = 0x01,
		InvalidAddress = 0x02,
		InvalidValue = 0x03,
		ConnectFail = 0x04,
		TooManyDevices = 0x05,
		AlreadyExists = 0x06,
		Busy = 0x07,
		UnknownDevice = 0x08,
		ResourceError = 0x09,
		RequestUnavailable = 0x0A,
		InvalidParamValue = 0x0B,
		WrongPINCode = 0x0C,
	};

	Error (uint8_t error_code);

	virtual const char *what () const noexcept;
	uint8_t errorCode () const;

private:
	uint8_t _error_code;
};

}

#endif
