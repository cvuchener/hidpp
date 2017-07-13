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

#include "IMouseButtonSpy.h"

#include <misc/Endian.h>
#include <cassert>

using namespace HIDPP20;

constexpr uint16_t IMouseButtonSpy::ID;

IMouseButtonSpy::IMouseButtonSpy (Device *dev):
	FeatureInterface (dev, ID, "MouseButtonSpy")
{
}

unsigned int IMouseButtonSpy::getMouseButtonCount ()
{
	auto results = call (GetMouseButtonCount);
	return results[0];
}

void IMouseButtonSpy::startMouseButtonSpy ()
{
	call (StartMouseButtonSpy);
}

void IMouseButtonSpy::stopMouseButtonSpy ()
{
	call (StopMouseButtonSpy);
}

std::vector<uint8_t> IMouseButtonSpy::getMouseButtonMapping ()
{
	return call (GetMouseButtonMapping);
}

void IMouseButtonSpy::setMouseButtonMapping (const std::vector<uint8_t> &button_mapping)
{
	call (SetMouseButtonMapping, button_mapping);
}

uint16_t IMouseButtonSpy::mouseButtonEvent (const HIDPP::Report &event)
{
	assert (event.function () == MouseButtonEvent);
	return readBE<uint16_t> (event.parameterBegin ());
}
