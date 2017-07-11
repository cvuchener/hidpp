/*
 * Copyright 2016 Clément Vuchener
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

#ifndef LIBHIDPP_HIDPP_PROFILE_DIRECTORY_H
#define LIBHIDPP_HIDPP_PROFILE_DIRECTORY_H

#include <hidpp/Address.h>
#include <hidpp/Setting.h>

namespace HIDPP
{

struct ProfileDirectory
{
	struct Entry
	{
		Address profile_address;
		std::map<std::string, Setting> settings;
	};

	std::vector<Entry> entries;
};

}

#endif
