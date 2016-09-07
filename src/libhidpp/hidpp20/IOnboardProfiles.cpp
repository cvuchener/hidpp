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
#include <misc/Log.h>
#include <misc/Endian.h>

using namespace HIDPP20;

IOnboardProfiles::IOnboardProfiles (Device *dev):
	_dev (dev),
	_index (IRoot (dev).getFeature (ID))
{
	Log::printf (Log::Debug, "Feature [0x%04hx] IOnboardProfiles has index 0x%02hhx\n", ID, _index);
}

uint8_t IOnboardProfiles::index () const
{
	return _index;
}

IOnboardProfiles::Description IOnboardProfiles::getDescription ()
{
	std::vector<uint8_t> params, results;
	results = _dev->callFunction (_index, GetDescription, params);
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
	std::vector<uint8_t> params, results;
	results = _dev->callFunction (_index, GetMode, params);
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
	std::vector<uint8_t> params, results;
	results = _dev->callFunction (_index, GetCurrentProfile, params);
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
	params[3] = offset;
	return _dev->callFunction (_index, MemoryRead, params);
}

void IOnboardProfiles::memoryAddrWrite (unsigned int page, unsigned int offset)
{
	std::vector<uint8_t> params (5);
	params[0] = MemoryType::Writeable;
	params[1] = page;
	params[3] = offset;
	params[4] = 0x01;
	_dev->callFunction (_index, MemoryAddrWrite, params);
}

void IOnboardProfiles::memoryWrite (const std::vector<uint8_t> &data)
{
	_dev->callFunction (_index, MemoryWrite, data);
}

void IOnboardProfiles::memoryWriteEnd ()
{
	std::vector<uint8_t> params;
	_dev->callFunction (_index, MemoryWriteEnd, params);
}

int IOnboardProfiles::getCurrentDPIIndex ()
{
	std::vector<uint8_t> params, results;
	results = _dev->callFunction (_index, GetCurrentDPIIndex, params);
	return results[0];
}

void IOnboardProfiles::setCurrentDPIIndex (int index)
{
	std::vector<uint8_t> params (1);
	params[0] = index;
	_dev->callFunction (_index, SetCurrentDPIIndex, params);
}

