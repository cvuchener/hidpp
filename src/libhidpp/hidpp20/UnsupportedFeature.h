/*
 * Copyright 2017 Clément Vuchener
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

#ifndef LIBHIDPP_HIDPP20_UNSUPPORTED_FEATURE_H
#define LIBHIDPP_HIDPP20_UNSUPPORTED_FEATURE_H

#include <stdexcept>

namespace HIDPP20
{

class UnsupportedFeature: public std::exception
{
public:
	UnsupportedFeature (uint16_t feature_id, const char *name);

	virtual const char *what () const noexcept;
	uint16_t featureID () const;

private:
	uint16_t _feature_id;
	std::string _msg;
};

}

#endif
