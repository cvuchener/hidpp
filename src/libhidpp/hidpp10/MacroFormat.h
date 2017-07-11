/*
 * Copyright 2017 Clément Vuchener
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

#ifndef LIBHIDPP_HIDPP10_MACRO_FORMAT_H
#define LIBHIDPP_HIDPP10_MACRO_FORMAT_H

#include <hidpp/AbstractMacroFormat.h>

#include <memory>

namespace HIDPP10
{

class MacroFormat: public HIDPP::AbstractMacroFormat
{
public:
	virtual std::size_t getLength (const HIDPP::Macro::Item &item) const;

	virtual void writeAddress (std::vector<uint8_t>::iterator it,
				   const HIDPP::Address &addr) const;

	virtual std::vector<uint8_t>::iterator
	writeItem (std::vector<uint8_t>::iterator it,
		   const HIDPP::Macro::Item &item,
		   std::vector<uint8_t>::iterator &jump_addr_it) const;


	virtual HIDPP::Macro::Item parseItem (std::vector<uint8_t>::const_iterator &it, HIDPP::Address &jump_addr) const;

private:
	std::vector<uint8_t>::iterator
	splitModifiersKeyItem (HIDPP::Macro::Item::Instruction mod_instr,
			       HIDPP::Macro::Item::Instruction key_instr,
			       std::vector<uint8_t>::iterator it,
			       const HIDPP::Macro::Item &item) const;
};

class Device;
std::unique_ptr<HIDPP::AbstractMacroFormat> getMacroFormat (Device *device);

}

#endif
