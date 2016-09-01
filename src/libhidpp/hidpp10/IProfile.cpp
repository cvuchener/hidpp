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

#include <hidpp10/IProfile.h>

#include <hidpp10/Device.h>
#include <hidpp10/defs.h>

using namespace HIDPP10;

IProfile::IProfile (Device *dev):
	_dev (dev)
{
}

int IProfile::activeProfile ()
{
	std::vector<uint8_t> results (HIDPP::ShortParamLength);
	_dev->getRegister (CurrentProfile, nullptr, results);
	if (results[0] == FactoryDefault)
		return -1;
	if (results[0] == ProfileIndex)
		return results[1];
	throw std::runtime_error ("Invalid active profile type.");
}

void IProfile::loadFactoryDefault ()
{
	std::vector<uint8_t> params (HIDPP::ShortParamLength);
	params[0] = FactoryDefault;
	_dev->setRegister (CurrentProfile, params, nullptr);
}

void IProfile::loadProfileFromIndex (unsigned int index)
{
	std::vector<uint8_t> params (HIDPP::ShortParamLength);
	params[0] = ProfileIndex;
	params[1] = index;
	_dev->setRegister (CurrentProfile, params, nullptr);
}

void IProfile::loadProfileFromAddress (Address address)
{
	std::vector<uint8_t> params (HIDPP::ShortParamLength);
	params[0] = ProfileAddress;
	params[1] = address.page;
	params[2] = address.offset;
	_dev->setRegister (CurrentProfile, params, nullptr);
}

void IProfile::reloadActiveProfile ()
{
	std::vector<uint8_t> current_value (HIDPP::ShortParamLength);
	_dev->getRegister (CurrentProfile, nullptr, current_value);
	_dev->setRegister (CurrentProfile, current_value, nullptr);
}

