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

#include <hidpp10/Sensor.h>

#include <stdexcept>

using namespace HIDPP10;

ListSensor::ListSensor (std::initializer_list<unsigned int> dpi_list):
	_resolutions (dpi_list)
{
}

ListSensor::ListSensor (unsigned int first, unsigned int last, unsigned int step)
{
	for (unsigned int dpi = first; dpi <= last; dpi += step)
		_resolutions.push_back (dpi);
}

unsigned int ListSensor::fromDPI (unsigned int dpi) const
{
	unsigned int low = 0, high = _resolutions.size () -1;
	unsigned int nearest;

	/* 0 is not a valid resolution, skip it */
	if (_resolutions[low] == 0)
		++low;

	if (dpi < _resolutions[low])
		nearest = low;
	else if (dpi > _resolutions[high])
		nearest = high;
	else {
		while (high - low > 1) {
			unsigned int mid = (high + low)/2;
			unsigned int mid_dpi = _resolutions[mid];
			if (dpi < mid_dpi)
				high = mid_dpi;
			else	
				low = mid_dpi;
		}
		if (_resolutions[high]-dpi < dpi-_resolutions[low])
			nearest = high;
		else
			nearest = low;
	}

	return 0x80 | (nearest & 0x7F);
}

unsigned int ListSensor::toDPI (unsigned int internal_value) const
{
	if (internal_value == 0)
		return 0;

	if (!(internal_value & 0x80))
		throw std::runtime_error ("Invalid resolution value");
	return _resolutions[internal_value & 0x7F];
}

ListSensor::const_iterator ListSensor::begin () const
{
	return _resolutions.begin ();
}

ListSensor::const_iterator ListSensor::end () const
{
	return _resolutions.end ();
}

const ListSensor ListSensor::S6006 = { 400, 800, 1600, 2000 };
const ListSensor ListSensor::S6090 = ListSensor (0, 3200, 200);


RangeSensor::RangeSensor (unsigned int min, unsigned int max, unsigned int step,
			  unsigned int ratio_dividend, unsigned int ratio_divisor):
	_min (min), _max (max), _step (step),
	_ratio_dividend (ratio_dividend), _ratio_divisor (ratio_divisor)
{
}

unsigned int RangeSensor::fromDPI (unsigned int dpi) const
{
	if (dpi < _min)
		dpi = _min;
	else if (dpi > _max)
		dpi = _max;
	
	return (dpi * _ratio_dividend + _ratio_divisor/2) / _ratio_divisor;
}

unsigned int RangeSensor::toDPI (unsigned int internal_value) const
{
	if (internal_value == 0)
		return 0;
	
	unsigned int dpi = (internal_value * _ratio_divisor + _ratio_dividend/2) /
			   _ratio_dividend;

	if (dpi > _max)
		return _max;
	return dpi;
}

unsigned int RangeSensor::minimumResolution () const
{
	return _min;
}

unsigned int RangeSensor::maximumResolution () const
{
	return _max;
}

unsigned int RangeSensor::resolutionStepHint () const
{
	return _step;
}

const RangeSensor RangeSensor::S9500 = RangeSensor (200, 5700, 50, 17, 400);
const RangeSensor RangeSensor::S9808 = RangeSensor (200, 8200, 50, 1, 50);

