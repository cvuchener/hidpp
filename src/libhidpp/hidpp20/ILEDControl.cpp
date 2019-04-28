/*
 * Copyright 2019 Clément Vuchener
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

#include <hidpp20/ILEDControl.h>

#include <misc/Endian.h>

#include <cassert>

using namespace HIDPP20;

constexpr uint16_t ILEDControl::ID;

ILEDControl::ILEDControl (Device *dev):
	FeatureInterface (dev, ID, "LEDControl")
{
}

unsigned int ILEDControl::getCount()
{
	std::vector<uint8_t> results;
	results = call (GetCount);
	return results[0];
}

ILEDControl::Info ILEDControl::getInfo(unsigned int led_index)
{
	std::vector<uint8_t> params (1), results;
	params[0] = led_index;
	results = call (GetInfo, params);
	return Info {
		static_cast<Type> (results[1]), // type
		results[2], // physical count
		readBE<uint16_t> (results, 3), // modes
		results[5], // config_capabilities
	};
}

bool ILEDControl::getSWControl()
{
	std::vector<uint8_t> results;
	results = call (GetSWControl);
	return results[0];
}

void ILEDControl::setSWControl(bool software_controlled)
{
	std::vector<uint8_t> params (1);
	params[0] = software_controlled ? 0x01 : 0x00;
	call (SetSWControl, params);
}

ILEDControl::State ILEDControl::getState(unsigned int led_index)
{
	std::vector<uint8_t> params (1), results;
	params[0] = led_index;
	results = call (GetState, params);
	State state { static_cast<Mode> (readLE<uint16_t> (results, 1)) };
	switch (state.mode) {
	case On:
		state.on.index = readBE<uint16_t> (results, 3);
		break;
	case Blink:
		state.blink.index = readBE<uint16_t> (results, 3);
		state.blink.on_duration = readBE<uint16_t> (results, 5);
		state.blink.off_duration = readBE<uint16_t> (results, 7);
		break;
	case Travel:
		state.travel.delay = readBE<uint16_t> (results, 5);
		break;
	case Breathing:
		state.breathing.max_brightness = readBE<uint16_t> (results, 3);
		state.breathing.period = readBE<uint16_t> (results, 5);
		state.breathing.timeout = readBE<uint16_t> (results, 7);
		break;
	default:
		break;
	}
	return state;
}

void ILEDControl::setState(unsigned int led_index, const State &state)
{
	std::vector<uint8_t> params (9);
	params[0] = led_index;
	writeLE<uint16_t> (params, 1, state.mode);
	switch (state.mode) {
	case On:
		writeBE<uint16_t> (params, 3, state.on.index);
		break;
	case Blink:
		writeBE<uint16_t> (params, 3, state.blink.index);
		writeBE<uint16_t> (params, 5, state.blink.on_duration);
		writeBE<uint16_t> (params, 7, state.blink.off_duration);
		break;
	case Travel:
		writeBE<uint16_t> (params, 5, state.travel.delay);
		break;
	case Breathing:
		writeBE<uint16_t> (params, 3, state.breathing.max_brightness);
		writeBE<uint16_t> (params, 5, state.breathing.period);
		writeBE<uint16_t> (params, 7, state.breathing.timeout);
		break;
	default:
		break;
	}
	call (SetState, params);
}

ILEDControl::Config ILEDControl::getConfig(unsigned int led_index)
{
	std::vector<uint8_t> params (1), results;
	params[0] = led_index;
	results = call (GetConfig, params);
	return static_cast<Config> (results[1]);
}

void ILEDControl::setConfig(unsigned int led_index, Config config)
{
	std::vector<uint8_t> params (2);
	params[0] = led_index;
	params[1] = config;
	call (SetConfig, params);
}

