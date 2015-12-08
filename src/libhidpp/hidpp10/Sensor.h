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

#ifndef HIDPP10_SENSOR_H
#define HIDPP10_SENSOR_H

#include <vector>

namespace HIDPP10
{

class Sensor
{
public:
	virtual unsigned int fromDPI (unsigned int dpi) const = 0;
	virtual unsigned int toDPI (unsigned int internal_value) const = 0;
};

class ListSensor: public Sensor
{
public:
	virtual unsigned int fromDPI (unsigned int dpi) const;
	virtual unsigned int toDPI (unsigned int internal_value) const;

	typedef std::vector<unsigned int> ResolutionList;
	typedef ResolutionList::const_iterator const_iterator;

	const_iterator begin () const;
	const_iterator end () const;

	static const ListSensor
		S6006,
		S6090;

private:
	ListSensor (std::initializer_list<unsigned int> dpi_list);
	ListSensor (unsigned int first, unsigned int last, unsigned int step);

	ResolutionList _resolutions;
};

class RangeSensor: public Sensor
{
public:
	virtual unsigned int fromDPI (unsigned int dpi) const;
	virtual unsigned int toDPI (unsigned int internal_value) const;

	unsigned int minimumResolution () const;
	unsigned int maximumResolution () const;
	unsigned int resolutionStepHint () const;

	static const RangeSensor
		S9500,
		S9808;

private:
	RangeSensor (unsigned int min, unsigned int max, unsigned int step,
		     unsigned int ratio_dividend, unsigned int ratio_divisor);

	unsigned int _min, _max, _step;
	unsigned int _ratio_dividend, _ratio_divisor;
};

}

#endif

