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

#ifndef HIDPP10_IINDIVIDUALFEATURES_H
#define HIDPP10_IINDIVIDUALFEATURES_H

namespace HIDPP10
{

class Device;

class IIndividualFeatures
{
public:
	enum IndividualFeature: unsigned int {
		SpecialButtonFunction = 1<<1,
		EnhancedKeyUsage = 1<<2,
		FastForwardRewind = 1<<3,
		ScrollingAcceleration = 1<<6,
		ButtonsControlResolution = 1<<7,
		InhibitLockKeySound = 1<<16,
		MXAir3DEngine = 1<<18,
		LEDControl = 1<<19,
	};

	IIndividualFeatures (Device *dev);

	unsigned int flags ();
	void setFlags (unsigned int flags);

	bool hasFlag (IndividualFeature feature);
	void setFlag (IndividualFeature feature);
	void unsetFlag (IndividualFeature feature);

private:
	Device *_dev;
};

}

#endif


