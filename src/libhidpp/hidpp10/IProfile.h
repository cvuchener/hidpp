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

#ifndef HIDPP10_IPROFILE_H
#define HIDPP10_IPROFILE_H

#include <hidpp10/IMemory.h>

namespace HIDPP10
{

class Device;

class IProfile
{
public:
	IProfile (Device *dev);

	enum ProfileType: uint8_t {
		ProfileIndex = 0x00,
		ProfileAddress = 0x01,
		FactoryDefault = 0xFF,
	};

	/**
	 * Get active profile.
	 *
	 * Returns -1 for factory default, or a positive
	 * integer for current profile index.
	 */
	int activeProfile ();

	void loadFactoryDefault ();
	void loadProfileFromIndex (unsigned int index);
	void loadProfileFromAddress (Address address);

	void reloadActiveProfile ();

private:
	Device *_dev;
};

}

#endif


