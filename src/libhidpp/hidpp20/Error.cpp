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

#include <hidpp20/Error.h>

using namespace HIDPP20;

Error::Error (ErrorCode error_code):
	_error_code (error_code)
{
}

const char *Error::what () const noexcept
{
	switch (_error_code) {
	case NoError:
		return "No error";
	case Unknown:
		return "Unknown";
	case InvalidArgument:
		return "Invalid argument";
	case OutOfRange:
		return "Out of range";
	case HWError:
		return "Hardware error";
	case LogitechInternal:
		return "Logitech internal";
	case InvalidFeatureIndex:
		return "Invalid feature index";
	case InvalidFunctionID:
		return "Invalid function ID";
	case Busy:
		return "Busy";
	case Unsupported:
		return "Unsupported";
	default:
		return "Unknown error code";
	}
}

Error::ErrorCode Error::errorCode () const
{
	return _error_code;
}

