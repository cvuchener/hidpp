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

#include <hidpp10/IResolution.h>

#include <hidpp10/defs.h>
#include <hidpp10/Device.h>
#include <hidpp10/Sensor.h>

using namespace HIDPP10;

IResolution0::IResolution0 (Device *dev, const Sensor *sensor):
	_dev (dev), _sensor (sensor)
{
}

unsigned int IResolution0::getCurrentResolution ()
{
	ByteArray results (HIDPP::ShortParamLength);
	_dev->getRegister (SensorResolution, nullptr, results);
	return _sensor->toDPI (results[0]);
}

void IResolution0::setCurrentResolution (unsigned int dpi)
{
	ByteArray params (HIDPP::ShortParamLength);
	params[0] = _sensor->fromDPI (dpi);
	_dev->setRegister (SensorResolution, params, nullptr);
}

IResolution3::IResolution3 (Device *dev, const Sensor *sensor):
	_dev (dev), _sensor (sensor)
{
}

void IResolution3::getCurrentResolution (unsigned int &x_dpi, unsigned int &y_dpi)
{
	ByteArray results (HIDPP::LongParamLength);
	_dev->getRegister (SensorResolution, nullptr, results);
	x_dpi = _sensor->toDPI (results.getLE<uint16_t> (0));
	y_dpi = _sensor->toDPI (results.getLE<uint16_t> (2));
}

void IResolution3::setCurrentResolution (unsigned int x_dpi, unsigned int y_dpi)
{
	ByteArray params (HIDPP::LongParamLength);
	params.setLE<uint16_t> (0, _sensor->fromDPI (x_dpi));
	params.setLE<uint16_t> (2, _sensor->fromDPI (y_dpi));
	_dev->setRegister (SensorResolution, params, nullptr);
}

bool IResolution3::getAngleSnap ()
{
	ByteArray results (HIDPP::LongParamLength);
	_dev->getRegister (SensorResolution, nullptr, results);
	switch (results[5]) {
	case 0x01:
		return false;
	case 0x02:
		return true;
	default:
		throw std::runtime_error ("Invalid angle snap value");
	}
}

void IResolution3::setAngleSnap (bool angle_snap)
{
	ByteArray params (HIDPP::LongParamLength);
	params[5] = angle_snap ? 0x02 : 0x01;
	_dev->setRegister (SensorResolution, params, nullptr);
}

