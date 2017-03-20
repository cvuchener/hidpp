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

#ifndef HIDPP20_IMOUSEBUTTONSPY_H
#define HIDPP20_IMOUSEBUTTONSPY_H

#include <hidpp20/FeatureInterface.h>

namespace HIDPP20
{

class IMouseButtonSpy: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x8110;

	enum Function {
		GetMouseButtonCount = 0,
		StartMouseButtonSpy = 1,
		StopMouseButtonSpy = 2,
	};

	enum Event {
		MouseButtonEvent = 0,
	};

	IMouseButtonSpy (Device *dev);

	unsigned int getMouseButtonCount ();
	void startMouseButtonSpy ();
	void stopMouseButtonSpy ();

	static uint16_t mouseButtonEvent (const HIDPP::Report &event);
};

}

#endif

