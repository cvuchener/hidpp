/*
 * Copyright 2016 Clément Vuchener
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

#include "AbstractProfileFormat.h"

using namespace HIDPP;

AbstractProfileFormat::AbstractProfileFormat (size_t size, unsigned int max_button_count, unsigned int max_mode_count):
	_size (size),
	_max_button_count (max_button_count),
	_max_mode_count (max_mode_count)
{
}

AbstractProfileFormat::~AbstractProfileFormat ()
{
}

size_t AbstractProfileFormat::size () const
{
	return _size;
}

unsigned int AbstractProfileFormat::maxButtonCount () const
{
	return _max_button_count;
}

unsigned int AbstractProfileFormat::maxModeCount () const
{
	return _max_mode_count;
}

