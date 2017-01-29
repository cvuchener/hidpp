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

#include "Enum.h"

using namespace HIDPP;

EnumDesc::EnumDesc (std::initializer_list<container::value_type> values):
	_values (values)
{
}

EnumDesc::const_iterator EnumDesc::begin () const
{
	return _values.begin ();
}

EnumDesc::const_iterator EnumDesc::end () const
{
	return _values.end ();
}

int EnumDesc::fromString (const std::string &str) const
{
	auto it = _values.find (str);
	if (it == _values.end ())
		throw std::runtime_error ("string is not a valid enum value");
	return it->second;
}

std::string EnumDesc::toString (int value) const
{
	for (const auto &p: _values)
		if (p.second == value)
			return p.first;
	throw std::runtime_error ("value is not in enum");
}

bool EnumDesc::check (int value) const
{
	for (const auto &p: _values)
		if (p.second == value)
			return true;
	return false;
}

EnumValue::EnumValue (const EnumDesc &desc, int value):
	_desc (desc),
	_value (value)
{
}

int EnumValue::get () const
{
	return _value;
}

void EnumValue::set (int value)
{
	_value = value;
}

std::string EnumValue::toString () const
{
	return _desc.toString (_value);
}


const EnumDesc &EnumValue::desc () const
{
	return _desc;
}
