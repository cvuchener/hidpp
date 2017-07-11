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

#ifndef LIBHIDPP_HIDPP10_DEVICE_H
#define LIBHIDPP_HIDPP10_DEVICE_H

#include <hidpp/Device.h>
#include <hidpp/Report.h>

namespace HIDPP { class Dispatcher; }

namespace HIDPP10
{

class Device: public HIDPP::Device
{
public:
	Device (HIDPP::Dispatcher *dispatcher, HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice);
	Device (HIDPP::Device &&device);

	void setRegister (uint8_t address,
			  const std::vector<uint8_t> &params,
			  std::vector<uint8_t> *results);
	void getRegister (uint8_t address,
			  const std::vector<uint8_t> *params,
			  std::vector<uint8_t> &results);

	void sendDataPacket (uint8_t sub_id, uint8_t seq_num,
			     std::vector<uint8_t>::const_iterator param_begin,
			     std::vector<uint8_t>::const_iterator param_end,
			     bool wait_for_ack = false);
private:
	template<uint8_t sub_id, HIDPP::Report::Type request_type, HIDPP::Report::Type result_type>
	void accessRegister (uint8_t address,
			     const std::vector<uint8_t> *params,
			     std::vector<uint8_t> *results);
};

}

#endif
