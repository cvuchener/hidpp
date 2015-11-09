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

#include <hidpp10/Error.h>

using namespace HIDPP10;

Error::Error (ErrorCode error_code):
	_error_code (error_code)
{
}

const char *Error::what () const noexcept
{
	switch (_error_code) {
	case Success:
		return "Success";
	case InvalidSubID:
		return "Invalid sub ID";
	case InvalidAddress:
		return "Invalid address";
	case InvalidValue:
		return "Invalid value";
	case ConnectFail:
		return "Connect fail";
	case TooManyDevices:
		return "Too many devices";
	case AlreadyExists:
		return "Already exists";
	case Busy:
		return "Busy";
	case UnknownDevice:
		return "Unknown device";
	case ResourceError:
		return "Resource error";
	case RequestUnavailable:
		return "Request unavailable";
	case InvalidParamValue:
		return "Invalid param value";
	case WrongPINCode:
		return "Wrong PIN code";
	default:
		return "Unknown error code";
	}
}

Error::ErrorCode Error::errorCode () const
{
	return _error_code;
}

