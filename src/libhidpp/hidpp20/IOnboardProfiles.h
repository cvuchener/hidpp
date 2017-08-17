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

#ifndef LIBHIDPP_HIDPP20_IONBOARDPROFILES_H
#define LIBHIDPP_HIDPP20_IONBOARDPROFILES_H

#include <hidpp20/FeatureInterface.h>

#include <vector>
#include <array>

namespace HIDPP20
{

/**
 * Manage on-board profiles
 *
 * There is two type of memory. The ROM contains safe profiles that the mouse
 * can default to if there is no valid profiles. The Writeable memory is where
 * normal user profiles are written.
 *
 * When invalid profile data is found in Writeable memory. Data from ROM is used instead.
 *
 * Each memory is divided in sectors or pages. Thus, an Address is given by the
 * memory type, the page index and an offset counting bytes from the beginning
 * of the page.
 *
 * Memory reads and writes are done by lines of \ref LineSize bytes (16 bytes:
 * the size of a long HID++ payload).
 *
 * Writing memory start with one call to \ref memoryAddrWrite specifying the
 * start address of the new data and its length. Then, \ref memoryWrite must
 * be called several times, writing LineSize bytes each time (except the last
 * that can be smaller). Finally, \ref memoryWriteEnd must be called.
 * \ref memoryWriteEnd may throw errors if the data was not valid, but the data
 * is written anyway.
 */
class IOnboardProfiles: public FeatureInterface
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

	enum Event {
		CurrentProfileChanged = 0,
		CurrentDPIIndexChanged = 1,
	};

	IOnboardProfiles (Device *dev);

	struct Description {
		/**
		 * Memory model
		 *
		 * Only known memory model is 1.
		 */
		uint8_t memory_model;
		/**
		 * Profile format
		 *
		 * Known profile formats are:
		 *  - 1: used by G402 and G502 Core
		 *  - 2: used by G303 and G502 Spectrum
		 *  - 3: used by G900
		 */
		uint8_t profile_format;
		/**
		 * Macro format
		 *
		 * Only known macro format is 1.
		 */
		uint8_t macro_format;
		uint8_t profile_count;
		uint8_t profile_count_oob;
		uint8_t button_count;
		uint8_t sector_count;	///< Number of sector/page in this device memory.
		uint16_t sector_size;	///< Size in bytes of each sector/page.
		/**
		 * Various infos.
		 *
		 * - G-shift (button action shift): bits 0–1 contains 2.
		 * - DPI shift: bits 2–3 contains 2.
		 */
		uint8_t mechanical_layout;
		/**
		 * Various infos.
		 *
		 * bits 0–2 are (exclusive?) flags about connector type:
		 *  - bit 0 (value 1): corded
		 *  - bit 1 (value 2): wireless
		 *  - bit 2 (value 4): corded + wireless
		 */
		uint8_t various_info;
	};
	/**
	 * Get information about the mouse formats, capabilities, and other informations.
	 */
	Description getDescription ();

	/**
	 * Mode telling how the mouse is controlled.
	 */
	enum class Mode: uint8_t {
		NoChange = 0, ///< Used with setMode, do nothing.
		/**
		 * On-board memory mode.
		 *
		 * The mouse uses the profiles from its on-board memory.
		 *
		 * Current profile and DPI index can be read and written and event are sent
		 * when they are changed.
		 */
		Onboard = 1,
		/**
		 * Host controlled mode.
		 *
		 * Only generic button events are sent according to the mapping from
		 * \ref IMouseButtonSpy.
		 *
		 * Current profile or DPI index cannot be set (invalid argument error)
		 * and events will not be sent.
		 */
		Host = 2,
	};

	enum MemoryType: uint8_t {
		Writeable = 0,
		ROM = 1,
	};

	/**
	 * Get the current mode.
	 *
	 * \see Mode, setMode
	 */
	Mode getMode ();
	/**
	 * Change the current mode.
	 *
	 * \see Mode, getMode
	 */
	void setMode (Mode mode);

	/**
	 * Get the current profile.
	 *
	 * \return the memory type and page of the current profile.
	 *
	 * \see setCurrentProfile, currentProfileChanged
	 */
	std::tuple<MemoryType, unsigned int> getCurrentProfile ();
	/**
	 * Set the current profile.
	 *
	 * The profile is given by its address but it must be a profile from
	 * the profile directory currently in use (from Writeable memory or ROM).
	 *
	 * \see getCurrentProfile, currentProfileChanged
	 */
	void setCurrentProfile (MemoryType mem_type, unsigned int page);

	/**
	 * Size of the data return by memoryRead() or the maximum size for memoryWrite().
	 */
	static constexpr unsigned int LineSize = 16;
	/**
	 * Read \ref LineSize bytes from the given address.
	 */
	std::vector<uint8_t> memoryRead (MemoryType mem_type, unsigned int page, unsigned int offset);
	/**
	 * Initiate writing to the memory.
	 *
	 * \param[in]	page	The page index to write.
	 * \param[in]	offset	Offset where to start writing
	 * \param[in]	length	Length of the data that will be sent with \ref memoryWrite
	 *
	 * The memory type is omitted since only the Writeable memory can be written.
	 */
	void memoryAddrWrite (unsigned int page, unsigned int offset, unsigned int length);
	/**
	 * Append \ref LineSize bytes of data.
	 *
	 * \ref memoryAddrWrite must have been called before.
	 *
	 * If the data between \p begin and \p end is smaller than \ref LineSize, padding is
	 * added.
	 *
	 * If the \p length passed to \ref memoryAddrWrite is not a multiple of \ref LineSize,
	 * the extra data at the end of the last memoryWrite call is ignored.
	 */
	void memoryWrite (std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end);
	/**
	 * End writing to the memory.
	 *
	 * Must be called after \ref memoryAddrWrite and \ref memoryWrite.
	 *
	 * It may throw errors related to the data that was written (e.g. invalid data).
	 */
	void memoryWriteEnd ();

	/**
	 * Get the current DPI index.
	 *
	 * First DPI mode is index 0.
	 *
	 * \see setCurrentDPIIndex, currentDPIIndexChanged
	 */
	unsigned int getCurrentDPIIndex ();
	/**
	 * Set the current DPI index.
	 *
	 * The index must be valid for the current profile.
	 *
	 * \see getCurrentDPIIndex, currentDPIIndexChanged
	 */
	void setCurrentDPIIndex (unsigned int index);

	/**
	 * Parse a \ref CurrentProfileChanged event.
	 *
	 * \see getCurrentProfile
	 */
	static std::tuple<MemoryType, unsigned int> currentProfileChanged (const HIDPP::Report &event);
	/**
	 * Parse a \ref CurrentDPIIndexChanged event.
	 *
	 * \see getCurrentDPIIndex
	 */
	static unsigned int currentDPIIndexChanged (const HIDPP::Report &event);
};

}

#endif

