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

#include "UnsupportedFeature.h"

#include <sstream>
#include <iomanip>

using namespace HIDPP20;

UnsupportedFeature::UnsupportedFeature (uint16_t feature_id):
	_feature_id (feature_id)
{
	std::stringstream ss;
	ss << "Feature 0x";
	ss << std::hex << std::setw (4) << std::setfill ('0') << feature_id;
	ss << " unsupported.";
	_msg.assign (ss.str ());
}

const char *UnsupportedFeature::what () const noexcept
{
	return _msg.c_str ();
}

uint16_t UnsupportedFeature::featureID () const
{
	return _feature_id;
}

