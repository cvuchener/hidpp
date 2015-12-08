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

#ifndef PROFILE_XML_H
#define PROFILE_XML_H

#include <hidpp10/Profile.h>
#include <hidpp10/Macro.h>
#include <tinyxml2.h>
#include <functional>

typedef std::function<void (const HIDPP10::Profile *,
			    const std::vector<HIDPP10::Macro> &macros,
			    tinyxml2::XMLNode *)> ProfileToXML;
typedef std::function<void (const tinyxml2::XMLNode *,
			    HIDPP10::Profile *,
			    std::vector<HIDPP10::Macro> &macros)> XMLToProfile;

void G500ProfileToXML (const HIDPP10::Profile *profile,
		       const std::vector<HIDPP10::Macro> &macros,
		       tinyxml2::XMLNode *node);
void XMLToG500Profile (const tinyxml2::XMLNode *node,
		       HIDPP10::Profile *profile,
		       std::vector<HIDPP10::Macro> &macros);

#endif
