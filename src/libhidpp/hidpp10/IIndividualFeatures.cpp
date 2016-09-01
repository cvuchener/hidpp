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

#include <hidpp10/IIndividualFeatures.h>

#include <hidpp10/defs.h>
#include <hidpp10/Device.h>

using namespace HIDPP10;

IIndividualFeatures::IIndividualFeatures (Device *dev):
	_dev (dev)
{
}

unsigned int IIndividualFeatures::flags ()
{
	std::vector<uint8_t> results (HIDPP::ShortParamLength);
	_dev->getRegister (EnableIndividualFeatures, nullptr, results);
	unsigned int flags = 0;
	for (unsigned int i = 0; i < 3; ++i)
		flags |= results[i] << (i*8);
	return flags;
}

void IIndividualFeatures::setFlags (unsigned int f)
{
	std::vector<uint8_t> params (HIDPP::ShortParamLength);
	for (unsigned int i = 0; i < 3; ++i)
		params[i] = (f >> (i*8)) & 0xFF;
	_dev->setRegister (EnableIndividualFeatures, params, nullptr);
}

bool IIndividualFeatures::hasFlag (IndividualFeature feature)
{
	return flags () & feature;
}

void IIndividualFeatures::setFlag (IndividualFeature feature)
{
	unsigned int f = flags ();
	f |= feature;
	setFlags (f);
}

void IIndividualFeatures::unsetFlag (IndividualFeature feature)
{
	unsigned int f = flags ();
	f &= ~feature;
	setFlags (f);
}

