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

#ifndef HIDPP10_DEVICE_H
#define HIDPP10_DEVICE_H

#include <hidpp/Device.h>
#include <hidpp/Report.h>

namespace HIDPP10
{

class Device: public HIDPP::Device
{
public:
	Device (const std::string &path, HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice);

	void setRegister (uint8_t address,
			  const std::vector<uint8_t> &params,
			  std::vector<uint8_t> *results);
	void getRegister (uint8_t address,
			  const std::vector<uint8_t> *params,
			  std::vector<uint8_t> &results);

	void sendDataPacket (uint8_t sub_id, uint8_t seq_num,
			     const std::vector<uint8_t> &params,
			     bool wait_for_ack = false);
private:
	template<uint8_t sub_id, std::size_t params_length, std::size_t results_length>
	void accessRegister (uint8_t address,
			     const std::vector<uint8_t> *params,
			     std::vector<uint8_t> *results);
};

}

#endif
