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

using namespace HIDPP10;

IMemory::IMemory (Device *dev):
	_dev (dev)
{
}

int IMemory::readSome (uint8_t page, uint8_t offset,
		       uint8_t *buffer, std::size_t maxlen)
{
	HIDPP::Parameters params (HIDPP::ShortParamLength);
	params[0] = page;
	params[1] = offset;
	HIDPP::Parameters results (HIDPP::LongParamLength);
	_dev->getRegister (MemoryRead, &params, results);
	std::size_t len = std::min (HIDPP::LongParamLength, maxlen);
	std::copy (results.begin (), results.begin () + len, buffer);
	return len;
}

void IMemory::readMem (uint8_t page, uint8_t offset,
		       std::vector<uint8_t> &data)
{
	std::size_t read = 0;
	while (read < data.size ()) {
		std::size_t len = readSome (page, offset,
					    &data[read], data.size () - read);
		offset += len/2;
		read += len;
	}
}

void IMemory::writeMem (uint8_t page, uint8_t offset,
			const std::vector<uint8_t> &data)
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
		HIDPP::Parameters params (HIDPP::LongParamLength);
		if (first) {
			sub_id = SendDataBeginAck;
			/* First packet header */
			params[0] = 0x01; // Unknown meaning
			params[1] = page;
			params[2] = offset;
			params.setWordBE(5, data.size ());
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

void IMemory::writePage (uint8_t page,
			 const std::vector<uint8_t> &data)
{
	if (data.size () > 512)
		throw std::logic_error ("page too big");
	
	fillPage (page);
	writeMem (page, 0, data);	
}

void IMemory::resetSequenceNumber ()
{
	HIDPP::Parameters params (HIDPP::ShortParamLength);
	params[0] = 1;
	_dev->setRegister (ResetSeqNum, params, nullptr);
}

void IMemory::fillPage (uint8_t page)
{
	HIDPP::Parameters params (HIDPP::LongParamLength);
	params[0] = Fill;
	params[6] = page;
	_dev->setRegister (MemoryOperation, params, nullptr);
}
