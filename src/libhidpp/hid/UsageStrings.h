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

#ifndef LIBHIDPP_HID_USAGE_STRINGS_H
#define LIBHIDPP_HID_USAGE_STRINGS_H

#include <string>

namespace HID
{

std::string keyString (unsigned int usage_code);
unsigned int keyUsageCode (const std::string &string);

std::string modifierString (uint8_t modifier_mask);
uint8_t modifierMask (const std::string &string);

std::string consumerControlString (unsigned int usage_code);
unsigned int consumerControlCode (const std::string &string);

std::string buttonString (unsigned int button_mask);
unsigned int buttonMask (const std::string &string);

}

#endif
