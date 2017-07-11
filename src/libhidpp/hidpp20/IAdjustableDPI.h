/*
 * Copyright 2017 Clément Vuchener
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

#ifndef LIBHIDPP_HIDPP20_IADJUSTABLEDPI_H
#define LIBHIDPP_HIDPP20_IADJUSTABLEDPI_H

#include <hidpp20/FeatureInterface.h>

#include <vector>

namespace HIDPP20
{

class IAdjustableDPI: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x2201;

	enum Function {
		GetSensorCount = 0,
		GetSensorDPIList = 1,
		GetSensorDPI = 2,
		SetSensorDPI = 3,
	};

	IAdjustableDPI (Device *dev);

	/**
	 * \returns the number of sensor on this device.
	 */
	unsigned int getSensorCount ();

	/**
	 * Get the list of supported DPI values.
	 *
	 * The list is either the complete list of supported
	 * values or the minimum and maximum values. When only
	 * the minmum and maximum values are given, \p dpi_step
	 * is set and the function returns true.
	 *
	 * \param[in]	index		Sensor index
	 * \param[out]	dpi_list	DPI list
	 * \param[out]	dpi_step	DPI list step
	 *
	 * \return true if dpi_step is set.
	 */
	bool getSensorDPIList (unsigned int index,
			       std::vector<unsigned int> &dpi_list,
			       unsigned int &dpi_step);

	/**
	 * \param[in]	index	Sensor index
	 *
	 * \returns the current and default DPI values.
	 */
	std::tuple<unsigned int, unsigned int> getSensorDPI (unsigned int index);

	/**
	 * \param[in]	index	Sensor index
	 * \param[in]	dpi	New DPI value
	 */
	void setSensorDPI (unsigned int index, unsigned int dpi);
};

}

#endif

