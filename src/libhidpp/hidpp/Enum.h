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

#ifndef LIBHIDPP_HIDPP_ENUM_H
#define LIBHIDPP_HIDPP_ENUM_H

#include <map>
#include <string>

namespace HIDPP
{

class InvalidEnumValueError: public std::exception
{
public:
	InvalidEnumValueError (int value);
	InvalidEnumValueError (const std::string &str);

	const char *what () const noexcept override;

private:
	std::string _msg;
};

class EnumDesc
{
public:
	typedef std::map<std::string, int> container;
	typedef container::const_iterator const_iterator;

	EnumDesc (std::initializer_list<container::value_type> values);

	const_iterator begin () const;
	const_iterator end () const;

	int fromString (const std::string &str) const;
	std::string toString (int value) const;

	bool check (int value) const;

private:
	std::map<std::string, int> _values;
};

class EnumValue
{
public:
	EnumValue (const EnumDesc &desc, int value);

	int get () const;
	void set (int value);

	std::string toString () const;

	const EnumDesc &desc () const;

private:
	const EnumDesc &_desc;
	int _value;
};

}

#endif
