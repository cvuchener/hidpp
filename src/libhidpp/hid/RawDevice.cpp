/*
 * Copyright 2021 Clément Vuchener
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

#include "RawDevice.h"

#include <misc/Log.h>

using namespace HID;

void RawDevice::logReportDescriptor () const
{
	auto debug = Log::debug ("reportdesc");
	if (!debug)
		return;
	for (const auto &collection: _report_desc.collections) {
		debug << "Collection: " << std::hex << collection.usage << std::dec << std::endl;
		for (const auto &[id, fields]: collection.reports) {
			const char *type;
			switch (id.type) {
			case ReportID::Type::Input: type = "Input"; break;
			case ReportID::Type::Output: type = "Output"; break;
			case ReportID::Type::Feature: type = "Feature"; break;
			}
			debug.printf ("- Report %s %d\n", type, id.id);
			for (const auto &f: fields) {
				debug.printf ("  - Flags: %x (%s, %s), Size: %d*%d\n",
						f.flags.bits,
						f.flags.Data () ? "Data" : "Constant",
						f.flags.Array () ? "Array" : "Variable",
						f.count, f.size);
				struct {
					Log &log;
					void operator() (const std::vector<uint32_t> &usages) const {
						for (uint32_t usage: usages)
							log << " " << usage;
					}
					void operator() (const std::pair<uint32_t, uint32_t> &usages) const {
						log << " [" << usages.first << ", " << usages.second << "]";
					}
				} print_usage{debug};
				debug << "    Usages:" << std::hex;
				std::visit (print_usage, f.usages);
				debug << std::dec << std::endl;
			}
		}
	}
}
