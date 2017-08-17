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

#include <misc/Endian.h>

#include <cassert>

using namespace HIDPP20;

constexpr uint16_t IOnboardProfiles::ID;

IOnboardProfiles::IOnboardProfiles (Device *dev):
	FeatureInterface (dev, ID, "OnboardProfiles")
{
}

IOnboardProfiles::Description IOnboardProfiles::getDescription ()
{
	std::vector<uint8_t> results;
	results = call (GetDescription);
	return Description {
		results[0], // Memory model
		results[1], // Profile format
		results[2], // Macro format
		results[3], results[4], // Profile counts
		results[5], // Button count
		results[6], readBE<uint16_t> (results, 7), // Sector (page) count and size
		results[9], results[10]
	};
}

IOnboardProfiles::Mode IOnboardProfiles::getMode ()
{
	std::vector<uint8_t> results;
	results = call (GetMode);
	return static_cast<Mode> (results[0]);
}

void IOnboardProfiles::setMode (Mode mode)
{
	std::vector<uint8_t> params (1);
	params[0] = static_cast<uint8_t> (mode);
	call (SetMode, params);
}

std::tuple<IOnboardProfiles::MemoryType, unsigned int> IOnboardProfiles::getCurrentProfile ()
{
	std::vector<uint8_t> results;
	results = call (GetCurrentProfile);
	return std::make_tuple (static_cast<MemoryType> (results[0]), results[1]);
}

void IOnboardProfiles::setCurrentProfile (MemoryType mem_type, unsigned int index)
{
	std::vector<uint8_t> params (2);
	params[0] = mem_type;
	params[1] = index;
	call (SetCurrentProfile, params);
}

std::vector<uint8_t> IOnboardProfiles::memoryRead (MemoryType mem_type, unsigned int page, unsigned int offset)
{
	std::vector<uint8_t> params (4), results;
	params[0] = mem_type;
	params[1] = page;
	writeBE<uint16_t> (params, 2, offset);
	return call (MemoryRead, params);
}

void IOnboardProfiles::memoryAddrWrite (unsigned int page, unsigned int offset, unsigned int length)
{
	std::vector<uint8_t> params (6);
	params[0] = MemoryType::Writeable;
	params[1] = page;
	writeBE<uint16_t> (params, 2, offset);
	writeBE<uint16_t> (params, 4, length);
	call (MemoryAddrWrite, params);
}

void IOnboardProfiles::memoryWrite (std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
{
	assert (std::distance (begin, end) <= LineSize);
	call (MemoryWrite, begin, end);
}

void IOnboardProfiles::memoryWriteEnd ()
{
	call (MemoryWriteEnd);
}

unsigned int IOnboardProfiles::getCurrentDPIIndex ()
{
	std::vector<uint8_t> results;
	results = call (GetCurrentDPIIndex);
	return results[0];
}

void IOnboardProfiles::setCurrentDPIIndex (unsigned int index)
{
	std::vector<uint8_t> params (1);
	params[0] = index;
	call (SetCurrentDPIIndex, params);
}

std::tuple<IOnboardProfiles::MemoryType, unsigned int> IOnboardProfiles::currentProfileChanged (const HIDPP::Report &event)
{
	assert (event.function () == CurrentProfileChanged);
	auto params = event.parameterBegin ();
	return std::make_tuple (static_cast<MemoryType> (params[0]), params[1]);
}

unsigned int IOnboardProfiles::currentDPIIndexChanged (const HIDPP::Report &event)
{
	assert (event.function () == CurrentDPIIndexChanged);
	return event.parameterBegin ()[0];
}
