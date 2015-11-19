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

#ifndef HIDPP10_IRESOLUTION_H
#define HIDPP10_IRESOLUTION_H

namespace HIDPP10
{

enum IResolutionType {
	IResolutionType0,
	IResolutionType3,
};

class Device;
class Sensor;

class IResolution0
{
public:
	IResolution0 (Device *dev, const Sensor *sensor);

	unsigned int getCurrentResolution ();
	void setCurrentResolution (unsigned int dpi);

private:
	Device *_dev;
	const Sensor *_sensor;
};

class IResolution3
{
public:
	IResolution3 (Device *dev, const Sensor *sensor);

	void getCurrentResolution (unsigned int &x_dpi, unsigned int &y_dpi);
	void setCurrentResolution (unsigned int x_dpi, unsigned int y_dpi);

	bool getAngleSnap ();
	void setAngleSnap (bool angle_snap);

private:
	Device *_dev;
	const Sensor *_sensor;
};

}

#endif
