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

#include "UsageStrings.h"

#include <sstream>
#include <map>
#include <vector>
#include <stdexcept>

#include <misc/Log.h>

static const std::map<std::string, unsigned int> key_string_map {
	{ "A", 0x04 },
	{ "B", 0x05 },
	{ "C", 0x06 },
	{ "D", 0x07 },
	{ "E", 0x08 },
	{ "F", 0x09 },
	{ "G", 0x0a },
	{ "H", 0x0b },
	{ "I", 0x0c },
	{ "J", 0x0d },
	{ "K", 0x0e },
	{ "L", 0x0f },
	{ "M", 0x10 },
	{ "N", 0x11 },
	{ "O", 0x12 },
	{ "P", 0x13 },
	{ "Q", 0x14 },
	{ "R", 0x15 },
	{ "S", 0x16 },
	{ "T", 0x17 },
	{ "U", 0x18 },
	{ "V", 0x19 },
	{ "W", 0x1a },
	{ "X", 0x1b },
	{ "Y", 0x1c },
	{ "Z", 0x1d },
	{ "1", 0x1e },
	{ "2", 0x1f },
	{ "3", 0x20 },
	{ "4", 0x21 },
	{ "5", 0x22 },
	{ "6", 0x23 },
	{ "7", 0x24 },
	{ "8", 0x25 },
	{ "9", 0x26 },
	{ "0", 0x27 },
	{ "Return", 0x28 },
	{ "Escape", 0x29 },
	{ "Backspace", 0x2a },
	{ "Tab", 0x2b },
	{ "Space", 0x2c },
	{ "Minus", 0x2d },
	{ "Equal", 0x2e },
	{ "LeftBrace", 0x2f },
	{ "RightBrace", 0x30 },
	{ "BackSlash", 0x31 },
	{ "BackSlashISO", 0x32 },
	{ "SemiColon", 0x33 },
	{ "Apostrophe", 0x34 },
	{ "Grave", 0x35 },
	{ "Comma", 0x36 },
	{ "Dot", 0x37 },
	{ "Slash", 0x38 },
	{ "CapsLock", 0x39 },
	{ "F1", 0x3a },
	{ "F2", 0x3b },
	{ "F3", 0x3c },
	{ "F4", 0x3d },
	{ "F5", 0x3e },
	{ "F6", 0x3f },
	{ "F7", 0x40 },
	{ "F8", 0x41 },
	{ "F9", 0x42 },
	{ "F10", 0x43 },
	{ "F11", 0x44 },
	{ "F12", 0x45 },
	{ "PrintScreen", 0x46 },
	{ "ScrollLock", 0x47 },
	{ "Pause", 0x48 },
	{ "Insert", 0x49 },
	{ "Home", 0x4a },
	{ "PageUp", 0x4b },
	{ "Delete", 0x4c },
	{ "End", 0x4d },
	{ "PageDown", 0x4e },
	{ "Right", 0x4f },
	{ "Left", 0x50 },
	{ "Down", 0x51 },
	{ "Up", 0x52 },
	{ "NumLock", 0x53 },
	{ "KeyPadSlash", 0x54 },
	{ "KeyPadAsterisk", 0x55 },
	{ "KeyPadMinus", 0x56 },
	{ "KeyPadPlus", 0x57 },
	{ "KeyPadEnter", 0x58 },
	{ "KeyPad1", 0x59 },
	{ "KeyPad2", 0x5a },
	{ "KeyPad3", 0x5b },
	{ "KeyPad4", 0x5c },
	{ "KeyPad5", 0x5d },
	{ "KeyPad6", 0x5e },
	{ "KeyPad7", 0x5f },
	{ "KeyPad8", 0x60 },
	{ "KeyPad9", 0x61 },
	{ "KeyPad0", 0x62 },
	{ "KeyPadDot", 0x63 },
	{ "102nd", 0x64 },
	{ "Compose", 0x65 },
	{ "Power", 0x66 },
	{ "KeyPadEqual", 0x67 },
	{ "F13", 0x68 },
	{ "F14", 0x69 },
	{ "F15", 0x6a },
	{ "F16", 0x6b },
	{ "F17", 0x6c },
	{ "F18", 0x6d },
	{ "F19", 0x6e },
	{ "F20", 0x6f },
	{ "F21", 0x70 },
	{ "F22", 0x71 },
	{ "F23", 0x72 },
	{ "F24", 0x73 },
	{ "LeftControl", 0xe0 },
	{ "LeftShift", 0xe1 },
	{ "LeftAlt", 0xe2 },
	{ "LeftMeta", 0xe3 },
	{ "RightControl", 0xe4 },
	{ "RightShift", 0xe5 },
	{ "RightAlt", 0xe6 },
	{ "AltGr", 0xe6 },
	{ "RightMeta", 0xe7 },
};

constexpr unsigned int KeyMax = 0xff;

static const std::vector<std::string> key_strings = []() {
	std::vector<std::string> vec (KeyMax+1);
	for (const auto &pair: key_string_map) {
		vec[pair.second] = pair.first;
	}
	return vec;
} ();

std::string HID::keyString (unsigned int usage_code)
{
	if (usage_code > KeyMax || key_strings[usage_code].empty ()) {
		std::stringstream ss;
		ss << "0x" << std::hex << std::setw (2) << std::setfill ('0') << usage_code;
		return ss.str ();
	}
	return key_strings[usage_code];
}

unsigned int HID::keyUsageCode (const std::string &string)
{
	auto it = key_string_map.find (string);
	if (it != key_string_map.end ())
		return it->second;

	std::size_t pos;
	unsigned int code = std::stoul (string, &pos, 0);
	if (string.c_str ()[pos] != '\0')
		return 0;
	return code;
}

std::string HID::modifierString (uint8_t modifier_mask)
{
	std::stringstream ss;
	bool first = true;
	for (unsigned int i = 0; i < 8; ++i) {
		if (modifier_mask & 1<<i) {
			if (first)
				first = false;
			else
				ss << "+";
			ss << key_strings[0xe0+i];
		}
	}
	return ss.str ();
}

uint8_t HID::modifierMask (const std::string &string)
{
	if (string.empty ())
		return 0;
	uint8_t mask = 0;
	std::size_t current = 0, next;
	do {
		next = string.find ('+', current);
		std::string mod;
		if (next == std::string::npos) {
			mod = string.substr (current);
			current = std::string::npos;
		}
		else {
			mod = string.substr (current, next-current);
			current = next + 1;
		}
		auto it = key_string_map.find (mod);
		if (it != key_string_map.end ()) {
			if (it->second < 0xe0 || it->second >= 0xe8) {
				throw std::invalid_argument ("Invalid modifier key");
			}
			mask |= 1<<(it->second - 0xe0);
		}
		else {
			std::size_t pos;
			unsigned int code = std::stoul (mod, &pos, 0);
			if (mod.c_str ()[pos] != '\0' || code > 255) {
				throw std::invalid_argument ("Invalid modifier code");
			}
			mask |= code;
		}
	} while (current != std::string::npos);

	return mask;
}

static const std::map<std::string, unsigned int> cc_string_map {
	{ "Unassigned", 0x00 },
	{ "Consumer Control", 0x01 },
	{ "Numeric Key Pad", 0x02 },
	{ "Programmable Buttons", 0x03 },
	{ "+10", 0x20 },
	{ "+100", 0x21 },
	{ "AM/PM", 0x22 },
	{ "Power", 0x30 },
	{ "Reset", 0x31 },
	{ "Sleep", 0x32 },
	{ "Sleep After", 0x33 },
	{ "Sleep Mode", 0x34 },
	{ "Illumination", 0x35 },
	{ "Function Buttons", 0x36 },
	{ "Menu", 0x40 },
	{ "Menu  Pick", 0x41 },
	{ "Menu Up", 0x42 },
	{ "Menu Down", 0x43 },
	{ "Menu Left", 0x44 },
	{ "Menu Right", 0x45 },
	{ "Menu Escape", 0x46 },
	{ "Menu Value Increase", 0x47 },
	{ "Menu Value Decrease", 0x48 },
	{ "Data On Screen", 0x60 },
	{ "Closed Caption", 0x61 },
	{ "Closed Caption Select", 0x62 },
	{ "VCR/TV", 0x63 },
	{ "Broadcast Mode", 0x64 },
	{ "Snapshot", 0x65 },
	{ "Still", 0x66 },
	{ "Selection", 0x80 },
	{ "Assign Selection", 0x81 },
	{ "Mode Step", 0x82 },
	{ "Recall Last", 0x83 },
	{ "Enter Channel", 0x84 },
	{ "Order Movie", 0x85 },
	{ "Channel", 0x86 },
	{ "Media Selection", 0x87 },
	{ "Media Select Computer", 0x88 },
	{ "Media Select TV", 0x89 },
	{ "Media Select WWW", 0x8a },
	{ "Media Select DVD", 0x8b },
	{ "Media Select Telephone", 0x8c },
	{ "Media Select Program Guide", 0x8d },
	{ "Media Select Video Phone", 0x8e },
	{ "Media Select Games", 0x8f },
	{ "Media Select Messages", 0x90 },
	{ "Media Select CD", 0x91 },
	{ "Media Select VCR", 0x92 },
	{ "Media Select Tuner", 0x93 },
	{ "Quit", 0x94 },
	{ "Help", 0x95 },
	{ "Media Select Tape", 0x96 },
	{ "Media Select Cable", 0x97 },
	{ "Media Select Satellite", 0x98 },
	{ "Media Select Security", 0x99 },
	{ "Media Select Home", 0x9a },
	{ "Media Select Call", 0x9b },
	{ "Channel Increment", 0x9c },
	{ "Channel Decrement", 0x9d },
	{ "Media Select SAP", 0x9e },
	{ "VCR Plus", 0xa0 },
	{ "Once", 0xa1 },
	{ "Daily", 0xa2 },
	{ "Weekly", 0xa3 },
	{ "Monthly", 0xa4 },
	{ "Play", 0xb0 },
	{ "Pause", 0xb1 },
	{ "Record", 0xb2 },
	{ "Fast Forward", 0xb3 },
	{ "Rewind", 0xb4 },
	{ "Scan Next Track", 0xb5 },
	{ "Scan Previous Track", 0xb6 },
	{ "Stop", 0xb7 },
	{ "Eject", 0xb8 },
	{ "Random Play", 0xb9 },
	{ "Select DisC", 0xba },
	{ "Enter Disc", 0xbb },
	{ "Repeat", 0xbc },
	{ "Tracking", 0xbd },
	{ "Track Normal", 0xbe },
	{ "Slow Tracking", 0xbf },
	{ "Frame Forward", 0xc0 },
	{ "Frame Back", 0xc1 },
	{ "Mark", 0xc2 },
	{ "Clear Mark", 0xc3 },
	{ "Repeat From Mark", 0xc4 },
	{ "Return To Mark", 0xc5 },
	{ "Search Mark Forward", 0xc6 },
	{ "Search Mark Backwards", 0xc7 },
	{ "Counter Reset", 0xc8 },
	{ "Show Counter", 0xc9 },
	{ "Tracking Increment", 0xca },
	{ "Tracking Decrement", 0xcb },
	{ "Stop/Eject", 0xcc },
	{ "Play/Pause", 0xcd },
	{ "Play/Skip", 0xce },
	{ "Volume", 0xe0 },
	{ "Balance", 0xe1 },
	{ "Mute", 0xe2 },
	{ "Bass", 0xe3 },
	{ "Treble", 0xe4 },
	{ "Bass Boost", 0xe5 },
	{ "Surround Mode", 0xe6 },
	{ "Loudness", 0xe7 },
	{ "MPX", 0xe8 },
	{ "Volume Up", 0xe9 },
	{ "Volume Down", 0xea },
	{ "Speed Select", 0xf0 },
	{ "Playback Speed", 0xf1 },
	{ "Standard Play", 0xf2 },
	{ "Long Play", 0xf3 },
	{ "Extended Play", 0xf4 },
	{ "Slow", 0xf5 },
	{ "Fan Enable", 0x100 },
	{ "Fan Speed", 0x101 },
	{ "Light", 0x102 },
	{ "Light Illumination Level", 0x103 },
	{ "Climate Control Enable", 0x104 },
	{ "Room Temperature", 0x105 },
	{ "Security Enable", 0x106 },
	{ "Fire Alarm", 0x107 },
	{ "Police Alarm", 0x108 },
	{ "Balance Right", 0x150 },
	{ "Balance Left", 0x151 },
	{ "Bass Increment", 0x152 },
	{ "Bass Decrement", 0x153 },
	{ "Treble Increment", 0x154 },
	{ "Treble Decrement", 0x155 },
	{ "Speaker System", 0x160 },
	{ "Channel Left", 0x161 },
	{ "Channel Right", 0x162 },
	{ "Channel Center", 0x163 },
	{ "Channel Front", 0x164 },
	{ "Channel Center Front", 0x165 },
	{ "Channel Side", 0x166 },
	{ "Channel Surround", 0x167 },
	{ "Channel Low Frequency Enhancement", 0x168 },
	{ "Channel Top", 0x169 },
	{ "Channel Unknown", 0x16a },
	{ "Sub-channel", 0x170 },
	{ "Sub-channel Increment", 0x171 },
	{ "Sub-channel Decrement", 0x172 },
	{ "Alternate Audio Increment", 0x173 },
	{ "Alternate Audio Decrement", 0x174 },
	{ "Application Launch Buttons", 0x180 },
	{ "AL Launch Button Configuration Tool", 0x181 },
	{ "AL Programmable Button Configuration", 0x182 },
	{ "AL Consumer Control Configuration", 0x183 },
	{ "AL Word Processor", 0x184 },
	{ "AL Text Editor", 0x185 },
	{ "AL Spreadsheet", 0x186 },
	{ "AL Graphics Editor", 0x187 },
	{ "AL Presentation App", 0x188 },
	{ "AL Database App", 0x189 },
	{ "AL Email Reader", 0x18a },
	{ "AL Newsreader", 0x18b },
	{ "AL Voicemail", 0x18c },
	{ "AL Contacts/Address Book", 0x18d },
	{ "AL Calendar/Schedule", 0x18e },
	{ "AL Task/Project Manager", 0x18f },
	{ "AL Log/Journal/Timecard", 0x190 },
	{ "AL Checkbook/Finance", 0x191 },
	{ "AL Calculator", 0x192 },
	{ "AL A/V Capture/Playback", 0x193 },
	{ "AL Local Machine Browser", 0x194 },
	{ "AL LAN/WAN Browser", 0x195 },
	{ "AL Internet Browser", 0x196 },
	{ "AL Remote Networking/ISP Connect", 0x197 },
	{ "AL Network Conference", 0x198 },
	{ "AL Network Chat", 0x199 },
	{ "AL Telephony/Dialer", 0x19a },
	{ "AL Logon", 0x19b },
	{ "AL Logoff", 0x19c },
	{ "AL Logon/Logoff", 0x19d },
	{ "AL Terminal Lock/Screensaver", 0x19e },
	{ "AL Control Panel", 0x19f },
	{ "AL Command Line Processor/Run", 0x1a0 },
	{ "AL Process/Task Manager", 0x1a1 },
	{ "AL Select Tast/Application", 0x1a2 },
	{ "AL Next Task/Application", 0x1a3 },
	{ "AL Previous Task/Application", 0x1a4 },
	{ "AL Preemptive Halt Task/Application", 0x1a5 },
	{ "Generic GUI Application Controls", 0x200 },
	{ "AC New", 0x201 },
	{ "AC Open", 0x202 },
	{ "AC Close", 0x203 },
	{ "AC Exit", 0x204 },
	{ "AC Maximize", 0x205 },
	{ "AC Minimize", 0x206 },
	{ "AC Save", 0x207 },
	{ "AC Print", 0x208 },
	{ "AC Properties", 0x209 },
	{ "AC Undo", 0x21a },
	{ "AC Copy", 0x21b },
	{ "AC Cut", 0x21c },
	{ "AC Paste", 0x21d },
	{ "AC Select All", 0x21e },
	{ "AC Find", 0x21f },
	{ "AC Find and Replace", 0x220 },
	{ "AC Search", 0x221 },
	{ "AC Go To", 0x222 },
	{ "AC Home", 0x223 },
	{ "AC Back", 0x224 },
	{ "AC Forward", 0x225 },
	{ "AC Stop", 0x226 },
	{ "AC Refresh", 0x227 },
	{ "AC Previous Link", 0x228 },
	{ "AC Next Link", 0x229 },
	{ "AC Bookmarks", 0x22a },
	{ "AC History", 0x22b },
	{ "AC Subscriptions", 0x22c },
	{ "AC Zoom In", 0x22d },
	{ "AC Zoom Out", 0x22e },
	{ "AC Zoom", 0x22f },
	{ "AC Full Screen View", 0x230 },
	{ "AC Normal View", 0x231 },
	{ "AC View Toggle", 0x232 },
	{ "AC Scroll Up", 0x233 },
	{ "AC Scroll Down", 0x234 },
	{ "AC Scroll", 0x235 },
	{ "AC Pan Left", 0x236 },
	{ "AC Pan Right", 0x237 },
	{ "AC Pan", 0x238 },
	{ "AC New Window", 0x239 },
	{ "AC Tile Horizontally", 0x23a },
	{ "AC Tile Vertically", 0x23b },
	{ "AC Format", 0x23c },
};

constexpr unsigned int CCMax = 0x240;

static const std::vector<std::string> cc_strings = []() {
	std::vector<std::string> vec (CCMax+1);
	for (const auto &pair: cc_string_map) {
		vec[pair.second] = pair.first;
	}
	return vec;
} ();

std::string HID::consumerControlString (unsigned int usage_code)
{
	if (usage_code > CCMax || cc_strings[usage_code].empty ()) {
		std::stringstream ss;
		ss << "0x" << std::hex << std::setw (4) << std::setfill ('0') << usage_code;
		return ss.str ();
	}
	return cc_strings[usage_code];
}

unsigned int HID::consumerControlCode (const std::string &string)
{
	auto it = cc_string_map.find (string);
	if (it != cc_string_map.end ())
		return it->second;

	std::size_t pos;
	unsigned int code = std::stoul (string, &pos, 0);
	if (string.c_str ()[pos] != '\0')
		return 0;
	return code;
}

std::string HID::buttonString (unsigned int button_mask)
{
	std::stringstream ss;
	bool first = true;
	for (unsigned int i = 0; i < 8*sizeof (unsigned int); ++i) {
		if (button_mask & 1<<i) {
			if (first)
				first = false;
			else
				ss << "+";
			ss << i;
		}
	}
	return ss.str ();
}

unsigned int HID::buttonMask (const std::string &string)
{
	unsigned int mouse_buttons = 0;
	std::size_t current = 0, next;
	do {
		next = string.find ('+', current);
		std::string button_str;
		if (next == std::string::npos)
			button_str = string.substr (current);
		else
			button_str = string.substr (current, next-current);
		std::size_t pos;
		unsigned int button = std::stoul (button_str, &pos);
		if (button_str.c_str ()[pos] != '\0') {
			throw std::invalid_argument ("Invalid button number");
		}
		mouse_buttons |= 1<<button;
		current = next;
	} while (current != std::string::npos);
	return mouse_buttons;
}

