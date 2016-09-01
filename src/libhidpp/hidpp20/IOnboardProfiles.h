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

#ifndef HIDPP20_IONBOARDPROFILES_H
#define HIDPP20_IONBOARDPROFILES_H

#include <cstdint>
#include <vector>

namespace HIDPP20
{

class Device;

class IOnboardProfiles
{
public:
	static constexpr uint16_t ID = 0x8100;

	enum Function {
		GetDescription = 0,
		SetMode = 1,
		GetMode = 2,
		SetCurrentProfile = 3,
		GetCurrentProfile = 4,
		MemoryRead = 5,
		MemoryAddrWrite = 6,
		MemoryWrite = 7,
		MemoryWriteEnd = 8,
		GetCurrentDPIIndex = 11,
		SetCurrentDPIIndex = 12,
	};

	IOnboardProfiles (Device *dev);

	uint8_t index () const;

	enum class MemoryModel: uint8_t {
		G402,
	};
	enum class ProfileFormat: uint8_t {
		G402,
		G303,
		G900,
	};
	enum class MacroFormat: uint8_t {
		G402,
	};
	struct Description {
		MemoryModel memory_model;
		ProfileFormat profile_format;
		MacroFormat macro_format;
		uint8_t profile_count;
		uint8_t profile_count_oob;
		uint8_t button_count;
		uint8_t sector_count;
		uint16_t sector_size;
		uint8_t mechanical_layout;
		uint8_t various_info;
	};
	Description getDescription ();

	enum class Mode: uint8_t {
		NoChange = 0,
		Onboard = 1,
		Host = 2,
	};

	Mode getMode ();
	void setMode (Mode mode);
	int getCurrentProfile ();
	void setCurrentProfile (int index);

	enum MemoryType: uint8_t {
		Writeable = 0,
		ROM = 1,
	};
	std::vector<uint8_t> memoryRead (MemoryType mem_type, unsigned int page, unsigned int offset);
	void memoryAddrWrite (unsigned int page, unsigned int offset);
	void memoryWrite (const std::vector<uint8_t> &data);
	void memoryWriteEnd ();

	int getCurrentDPIIndex ();
	void setCurrentDPIIndex (int index);

private:
	Device *_dev;
	uint8_t _index;
};

}

#endif

