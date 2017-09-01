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

#include <hidpp10/IReceiver.h>

#include <hidpp10/defs.h>
#include <hidpp10/Device.h>

#include <misc/Endian.h>
#include <stdexcept>

using namespace HIDPP10;

IReceiver::IReceiver (Device *dev):
	_dev (dev)
{
}

void IReceiver::getDeviceInformation (unsigned int device,
				      uint8_t *destination_id,
				      uint8_t *report_interval,
				      uint16_t *wpid,
				      DeviceType *type)
{
	if (device >= 16)
		throw std::out_of_range ("Device index too big");

	std::vector<uint8_t> params (HIDPP::ShortParamLength),
			     results (HIDPP::LongParamLength);

	params[0] = DeviceInformation | (device & 0x0F);

	_dev->getRegister (DevicePairingInfo, &params, results);

	if (params[0] != results[0])
		throw std::runtime_error ("Invalid DevicePairingInfo type");

	if (destination_id)
		*destination_id = results[1];
	if (report_interval)
		*report_interval = results[2];
	if (wpid)
		*wpid = readBE<uint16_t> (results, 3);
	if (type)
		*type = static_cast<DeviceType> (results[7]);
}

void IReceiver::getDeviceExtendedInformation (unsigned int device,
					      uint32_t *serial,
					      uint32_t *report_types,
					      PowerSwitchLocation *ps_loc)
{
	if (device >= 16)
		throw std::out_of_range ("Device index too big");

	std::vector<uint8_t> params (HIDPP::ShortParamLength),
			     results (HIDPP::LongParamLength);

	params[0] = ExtendedDeviceInformation | (device & 0x0F);

	_dev->getRegister (DevicePairingInfo, &params, results);

	if (params[0] != results[0])
		throw std::runtime_error ("Invalid DevicePairingInfo type");

	if (serial)
		*serial = readBE<uint32_t> (results, 1);
	if (report_types)
		*report_types = readBE<uint32_t> (results, 5);
	if (ps_loc)
		*ps_loc = static_cast<PowerSwitchLocation> (results[9] & 0x0F);
}

std::string IReceiver::getDeviceName (unsigned int device)
{
	if (device >= 16)
		throw std::out_of_range ("Device index too big");

	std::vector<uint8_t> params (HIDPP::ShortParamLength),
			     results (HIDPP::LongParamLength);

	params[0] = DeviceName | (device & 0x0F);

	_dev->getRegister (DevicePairingInfo, &params, results);

	if (params[0] != results[0])
		throw std::runtime_error ("Invalid DevicePairingInfo type");

	std::size_t length = results[1];
	return std::string (reinterpret_cast<char *> (&results[2]), std::min (length, std::size_t {14}));
}
