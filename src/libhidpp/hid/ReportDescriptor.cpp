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

#include "ReportDescriptor.h"

#include <type_traits>
#include <stack>
#include <stdexcept>

#include <misc/Log.h>

using namespace HID;

enum ItemType
{
	Main = 0,
	Global = 1,
	Local = 2,
	Long = 3,
};

enum MainItem
{
	Input = 8,
	Output = 9,
	Feature = 11,
	Collection = 10,
	EndCollection = 12,
};

enum GlobalItem
{
	UsagePage = 0,
	ReportSize = 7,
	ReportID = 8,
	ReportCount = 9,
	Push = 10,
	Pop = 11,
};

enum LocalItem
{
	Usage = 0,
	UsageMinimum = 1,
	UsageMaximum = 2,
	Delimiter = 10,
};

struct item_t
{
	ItemType type;
	uint8_t tag;
	unsigned int size;
	const uint8_t *data;
	HID::Usage get_usage (uint16_t default_page) const {
		switch (size) {
		case 1:
			return HID::Usage (default_page, data[0]);
		case 2:
			return HID::Usage (default_page, get<uint16_t> ());
		case 4:
			return HID::Usage (get<uint32_t> ());
		default:
			throw std::runtime_error ("invalid item size");
		}
	}
	template <typename T>
	std::enable_if_t<std::is_unsigned_v<T>, T> get () const {
		if (sizeof (T) < size)
			throw std::runtime_error ("invalid item size");
		T value = 0;
		for (unsigned int i = 0; i < size; ++i)
			value |= (data[i]&0xFF) << (8*i);
		return value;
	}
	template <typename T>
	std::enable_if_t<std::is_signed_v<T>, T> get () const {
		if (sizeof (T) < size)
			throw std::runtime_error ("invalid item size");
		T value = 0;
		for (unsigned int i = 0; i < size; ++i)
			value |= (data[i]&0xFF) << (8*i);
		if (data[size-1] & 0x80) // sign extend
			for (unsigned int i = size; i < sizeof (T); ++i)
				value |= 0xFF << (8*i);
		return value;
	}
};

std::tuple<item_t, const uint8_t *> read_item (const uint8_t *data, std::size_t length)
{
	if (data[0] == 0xFE) {
		if (length < 2)
			throw std::runtime_error ("unexpected end of data");
		unsigned int size = data[1];
		if (length < 3+size)
			throw std::runtime_error ("unexpected end of data");
		return {{
				ItemType::Long, // type
				data[2], // tag
				size, // size
				&data[3] // data
			}, &data[3+size]};
	}
	else {
		unsigned int size = data[0] & 0x03;
		if (size == 3) size = 4;
		if (length < 1+size)
			throw std::runtime_error ("unexpected end of data");
		return {{
				ItemType ((data[0] & 0x0C) >> 2), // type
				uint8_t ((data[0] & 0xF0) >> 4), // tag
				size, // size
				&data[1] // data
			}, &data[1+size]};
	}
}

ReportDescriptor ReportDescriptor::fromRawData (const uint8_t *data, std::size_t length)
{
	struct global_context_t {
		uint16_t usage_page;
		unsigned int report_size;
		uint8_t report_id;
		unsigned int report_count;
	};
	std::stack<global_context_t> global;
	global.push({});
	struct local_context_t {
		std::vector<HID::Usage> usages;
		HID::Usage usage_min, usage_max;
	} local;
	enum {
		Closed,
		OpenedFirst,
		OpenedOthers,
	} delimiter_state = Closed;
	ReportDescriptor descriptor;
	int collection_depth = 0;
	while (length > 0) {
		auto [item, next_data] = read_item (data, length);
		length -= (next_data - data);
		data = next_data;
		switch (item.type) {
		case ItemType::Main:
			switch (static_cast<MainItem> (item.tag)) {
			case MainItem::Input:
			case MainItem::Output:
			case MainItem::Feature: {
				ReportID id = {
					static_cast<ReportID::Type> (item.tag),
					global.top ().report_id
				};
				auto [it, inserted] = descriptor.collections.back ().reports.emplace (id, 0);
				ReportField::Flags flags = {item.get<unsigned int> ()};
				if (!local.usages.empty () ||
						local.usage_min != Usage () || local.usage_max != Usage () ||
						!flags.Constant ()) { // exclude padding fields
					auto &f = it->second.emplace_back ();
					f.flags = flags;
					f.count = global.top ().report_count;
					f.size = global.top ().report_size;
					if (!local.usages.empty ())
						f.usages = std::move (local.usages);
					else
						f.usages = std::make_pair (local.usage_min, local.usage_max);
				}
				break;
			}
			case MainItem::Collection:
				if (collection_depth == 0) {
					auto &c = descriptor.collections.emplace_back ();
					c.type = static_cast<ReportCollection::Type> (item.get<unsigned int> ());
					if (local.usages.size () != 1)
						throw std::runtime_error ("invalid collection usage");
					c.usage = local.usages.front ();
				}
				++collection_depth;
				break;
			case MainItem::EndCollection:
				if (collection_depth <= 0)
					throw std::runtime_error ("unexpected End Collection item");
				--collection_depth;
				break;
			default:
				break;
			}
			local = {};
			break;
		case ItemType::Global:
			switch (static_cast<GlobalItem> (item.tag)) {
			case GlobalItem::UsagePage:
				global.top ().usage_page = item.get<uint16_t> ();
				break;
			case GlobalItem::ReportSize:
				global.top ().report_size = item.get<unsigned int> ();
				break;
			case GlobalItem::ReportID:
				global.top ().report_id = item.get<uint8_t> ();
				break;
			case GlobalItem::ReportCount:
				global.top ().report_count = item.get<unsigned int> ();
				break;
			case GlobalItem::Push:
				global.push (global.top ());
				break;
			case GlobalItem::Pop:
				global.pop ();
				break;
			default:
				break;
			}
			break;
		case ItemType::Local:
			switch (static_cast<LocalItem> (item.tag)) {
			case LocalItem::Usage:
				if (delimiter_state != OpenedOthers)
					local.usages.push_back (item.get_usage (global.top ().usage_page));
				if (delimiter_state == OpenedFirst)
					delimiter_state = OpenedOthers;
				break;
			case LocalItem::UsageMinimum:
				local.usage_min = item.get_usage (global.top ().usage_page);
				break;
			case LocalItem::UsageMaximum:
				local.usage_max = item.get_usage (global.top ().usage_page);
				break;
			case LocalItem::Delimiter:
				switch (item.get<unsigned int> ()) {
				case 1:
					if (delimiter_state != Closed)
						throw std::runtime_error ("delimiter mismatch");
					delimiter_state = OpenedFirst;
					break;
				case 0:
					if (delimiter_state == Closed)
						throw std::runtime_error ("delimiter mismatch");
					delimiter_state = Closed;
					break;
				default: break;
				}
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
	if (collection_depth != 0)
		Log::warning ("reportdescriptor") << "Some collections are not closed";
	return descriptor;
}
