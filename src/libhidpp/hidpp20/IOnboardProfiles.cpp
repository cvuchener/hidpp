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

#include <hidpp20/IOnboardProfiles.h>

#include <hidpp20/Device.h>
#include <hidpp20/IRoot.h>
#include <hidpp20/UnsupportedFeature.h>
#include <misc/Log.h>
#include <misc/Endian.h>

#include <cassert>

using namespace HIDPP20;

IOnboardProfiles::IOnboardProfiles (Device *dev):
	_dev (dev),
	_index (IRoot (dev).getFeature (ID))
{
	if (_index == 0)
		throw UnsupportedFeature (ID);
	Log::debug ().printf ("Feature [0x%04hx] IOnboardProfiles has index 0x%02hhx\n", ID, _index);
}

uint8_t IOnboardProfiles::index () const
{
	return _index;
}

IOnboardProfiles::Description IOnboardProfiles::getDescription ()
{
	std::vector<uint8_t> results;
	results = _dev->callFunction (_index, GetDescription);
	return Description {
		static_cast<MemoryModel> (results[0]),
		static_cast<ProfileFormat> (results[1]),
		static_cast<MacroFormat> (results[2]),
		results[3], results[4], // Profile counts
		results[5], // Button count
		results[6], readBE<uint16_t> (results, 7), // Sector (page) count and size
		results[9], results[10]
	};
}

IOnboardProfiles::Mode IOnboardProfiles::getMode ()
{
	std::vector<uint8_t> results;
	results = _dev->callFunction (_index, GetMode);
	return static_cast<Mode> (results[0]);
}

void IOnboardProfiles::setMode (Mode mode)
{
	std::vector<uint8_t> params (1);
	params[0] = static_cast<uint8_t> (mode);
	_dev->callFunction (_index, SetMode, params);
}

int IOnboardProfiles::getCurrentProfile ()
{
	std::vector<uint8_t> results;
	results = _dev->callFunction (_index, GetCurrentProfile);
	return results[0];
}

void IOnboardProfiles::setCurrentProfile (int index)
{
	std::vector<uint8_t> params (1);
	params[0] = index;
	_dev->callFunction (_index, SetCurrentProfile, params);
}

std::vector<uint8_t> IOnboardProfiles::memoryRead (MemoryType mem_type, unsigned int page, unsigned int offset)
{
	std::vector<uint8_t> params (4), results;
	params[0] = mem_type;
	params[1] = page;
	writeBE<uint16_t> (params, 2, offset);
	return _dev->callFunction (_index, MemoryRead, params);
}

void IOnboardProfiles::memoryAddrWrite (unsigned int page, unsigned int offset, unsigned int length)
{
	std::vector<uint8_t> params (6);
	params[0] = MemoryType::Writeable;
	params[1] = page;
	writeBE<uint16_t> (params, 2, offset);
	writeBE<uint16_t> (params, 4, length);
	_dev->callFunction (_index, MemoryAddrWrite, params);
}

void IOnboardProfiles::memoryWrite (std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
{
	assert (std::distance (begin, end) <= LineSize);
	_dev->callFunction (_index, MemoryWrite, begin, end);
}

void IOnboardProfiles::memoryWriteEnd ()
{
	_dev->callFunction (_index, MemoryWriteEnd);
}

int IOnboardProfiles::getCurrentDPIIndex ()
{
	std::vector<uint8_t> results;
	results = _dev->callFunction (_index, GetCurrentDPIIndex);
	return results[0];
}

void IOnboardProfiles::setCurrentDPIIndex (int index)
{
	std::vector<uint8_t> params (1);
	params[0] = index;
	_dev->callFunction (_index, SetCurrentDPIIndex, params);
}

