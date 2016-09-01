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

#include <hidpp10/IMemory.h>

#include <hidpp10/Device.h>
#include <hidpp10/defs.h>

#include <misc/Endian.h>

using namespace HIDPP10;

IMemory::IMemory (Device *dev):
	_dev (dev)
{
}

int IMemory::readSome (Address address, uint8_t *buffer, std::size_t maxlen)
{
	std::vector<uint8_t> params (HIDPP::ShortParamLength);
	params[0] = address.page;
	params[1] = address.offset;
	std::vector<uint8_t> results (HIDPP::LongParamLength);
	_dev->getRegister (MemoryRead, &params, results);
	std::size_t len = std::min (HIDPP::LongParamLength, maxlen);
	std::copy (results.begin (), results.begin () + len, buffer);
	return len;
}

void IMemory::readMem (Address address, std::vector<uint8_t> &data)
{
	std::size_t read = 0;
	while (read < data.size ()) {
		std::size_t len = readSome (address, &data[read], data.size () - read);
		address.offset += len/2;
		read += len;
	}
}

void IMemory::writeMem (Address address, const std::vector<uint8_t> &data)
{
	static constexpr std::size_t HeaderLength = 9;
	static constexpr std::size_t FirstPacketDataLength =
			HIDPP::LongParamLength - HeaderLength;

	/*
	 * Init sequence number
	 */
	resetSequenceNumber ();

	/*
	 * Start sending packets
	 */
	bool first = true;
	std::size_t sent = 0;
	uint8_t seq_num = 0;
	while (sent < data.size ()) {
		uint8_t sub_id;
		std::vector<uint8_t> params (HIDPP::LongParamLength);
		if (first) {
			sub_id = SendDataBeginAck;
			/* First packet header */
			params[0] = 0x01; // Unknown meaning
			params[1] = address.page;
			params[2] = address.offset;
			writeBE<uint16_t> (params, 5, data.size ());
			/* Start of data */
			if (data.size () < FirstPacketDataLength) {
				std::copy (data.begin (), data.end (),
					   params.begin () + HeaderLength);
				sent = data.size ();
			}
			else {
				std::copy (data.begin (),
					   data.begin () + FirstPacketDataLength,
					   params.begin () + HeaderLength);
				sent += FirstPacketDataLength;
			}
			first = false;
		}
		else {
			sub_id = SendDataContinueAck;
			if (data.size () < HIDPP::LongParamLength) {
				std::copy (data.begin () + sent,
					   data.end (),
					   params.begin ());
				sent = data.size ();
			}
			else {
				std::copy (data.begin () + sent,
					   data.begin () + sent + HIDPP::LongParamLength,
					   params.begin ());
				sent += HIDPP::LongParamLength;
			}
		}
		_dev->sendDataPacket (sub_id, seq_num, params, true);
		seq_num++;
	}
}

void IMemory::writePage (uint8_t page, const std::vector<uint8_t> &data)
{
	if (data.size () > 512)
		throw std::logic_error ("page too big");
	
	fillPage (page);
	writeMem ({page, 0}, data);
}

void IMemory::resetSequenceNumber ()
{
	std::vector<uint8_t> params (HIDPP::ShortParamLength);
	params[0] = 1;
	_dev->setRegister (ResetSeqNum, params, nullptr);
}

void IMemory::fillPage (uint8_t page)
{
	std::vector<uint8_t> params (HIDPP::LongParamLength);
	params[0] = Fill;
	params[6] = page;
	_dev->setRegister (MemoryOperation, params, nullptr);
}

