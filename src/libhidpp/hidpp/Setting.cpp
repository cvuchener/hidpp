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

#include "Setting.h"

#include <misc/Log.h>

#include <array>
#include <sstream>
#include <limits>
#include <cassert>

using namespace HIDPP;

template<>
Setting::Type Setting::type<std::string> ()
{
	return Type::String;
}
template<>
Setting::Type Setting::type<bool> ()
{
	return Type::Boolean;
}
template<>
Setting::Type Setting::type<int> ()
{
	return Type::Integer;
}
template<>
Setting::Type Setting::type<LEDVector> ()
{
	return Type::LEDVector;
}
template<>
Setting::Type Setting::type<Color> ()
{
	return Type::Color;
}
template<>
Setting::Type Setting::type<ComposedSetting> ()
{
	return Type::ComposedSetting;
}
template<>
Setting::Type Setting::type<EnumValue> ()
{
	return Type::Enum;
}

Setting::Setting (const Setting &other):
	_type (other._type)
{
	switch (_type) {
	case Type::String:
		_value = new std::string (*reinterpret_cast<std::string *> (other._value));
		break;
	case Type::Boolean:
		_value = new bool (*reinterpret_cast<bool *> (other._value));
		break;
	case Type::Integer:
		_value = new int (*reinterpret_cast<int *> (other._value));
		break;
	case Type::LEDVector:
		_value = new LEDVector (*reinterpret_cast<LEDVector *> (other._value));
		break;
	case Type::Color:
		_value = new Color (*reinterpret_cast<Color *> (other._value));
		break;
	case Type::ComposedSetting:
		_value = new ComposedSetting (*reinterpret_cast<ComposedSetting *> (other._value));
		break;
	case Type::Enum:
		_value = new EnumValue (*reinterpret_cast<EnumValue *> (other._value));
		break;
	}
}

Setting::Setting (Setting &&other):
	_type (other._type),
	_value (other._value)
{
	other._value = nullptr;
}

Setting::~Setting ()
{
	if (!_value)
		return;
	switch (_type) {
	case Type::String:
		delete reinterpret_cast<std::string *> (_value);
		break;
	case Type::Boolean:
		delete reinterpret_cast<bool *> (_value);
		break;
	case Type::Integer:
		delete reinterpret_cast<int *> (_value);
		break;
	case Type::LEDVector:
		delete reinterpret_cast<LEDVector *> (_value);
		break;
	case Type::Color:
		delete reinterpret_cast<Color *> (_value);
		break;
	case Type::ComposedSetting:
		delete reinterpret_cast<ComposedSetting *> (_value);
		break;
	case Type::Enum:
		delete reinterpret_cast<EnumValue *> (_value);
		break;
	}
}

Setting::Type Setting::type () const
{
	return _type;
}

std::string Setting::toString () const
{
	switch (_type) {
	case Type::String:
		return get<std::string> ();

	case Type::Boolean:
		return (get<bool> () ? "true" : "false");

	case Type::Integer: {
		std::stringstream ss;
		ss << get<int> ();
		return ss.str ();
	}

	case Type::LEDVector: {
		std::string str;
		for (bool led: get<LEDVector> ())
			str += (led ? '1' : '0');
		return str;
	}

	case Type::Color: {
		std::array<char, 7> str;
		const Color &c = get<Color> ();
		snprintf (str.data (), str.size (), "%02hhx%02hhx%02hhx", c.r, c.g, c.b);
		return std::string (str.data (), str.size ());
	}

	case Type::Enum:
		return get<EnumValue> ().toString ();

	case Type::ComposedSetting:
	default:
		throw std::runtime_error ("Invalid type for conversion");
	}
}

SettingDesc::SettingDesc (const std::string &default_value):
	_type (Setting::Type::String),
	_default_value (default_value)
{
}

SettingDesc::SettingDesc (bool default_value):
	_type (Setting::Type::Boolean),
	_default_value (default_value)
{
}

SettingDesc::SettingDesc (int min, int max, int default_value):
	_type (Setting::Type::Integer),
	_min (min),
	_max (max),
	_default_value (default_value)
{
}

SettingDesc::SettingDesc (const LEDVector &default_value):
	_type (Setting::Type::LEDVector),
	_led_count (default_value.size ()),
	_default_value (default_value)
{
}

SettingDesc::SettingDesc (const Color &default_value):
	_type (Setting::Type::Color),
	_default_value (default_value)
{
}

SettingDesc::SettingDesc (std::initializer_list<container::value_type> sub_settings):
	_type (Setting::Type::ComposedSetting),
	_sub_settings (sub_settings),
	_default_value (ComposedSetting ())
{
}

SettingDesc::SettingDesc (const container &sub_settings):
	_type (Setting::Type::ComposedSetting),
	_sub_settings (sub_settings),
	_default_value (ComposedSetting ())
{
}

SettingDesc::SettingDesc (const EnumDesc &enum_desc, int default_value):
	_type (Setting::Type::Enum),
	_enum_desc (&enum_desc),
	_default_value (EnumValue (enum_desc, default_value))
{
}

bool SettingDesc::check (const Setting &setting) const
{
	if (setting.type () != _type)
		return false;
	switch (_type) {
	case Setting::Type::String:
	case Setting::Type::Boolean:
	case Setting::Type::Color:
		return true;

	case Setting::Type::Integer: {
		int value = setting.get<int> ();
		return value >= _min && value <= _max;
	}

	case Setting::Type::LEDVector:
		return setting.get<LEDVector> ().size () == _led_count;

	case Setting::Type::Enum: {
		EnumValue value = setting.get<EnumValue> ();
		return &value.desc () == _enum_desc && _enum_desc->check (value.get ());
	}
	case Setting::Type::ComposedSetting:
		for (const auto &pair: setting.get<ComposedSetting> ()) {
			auto it = _sub_settings.find (pair.first);
			if (it == _sub_settings.end ()) {
				Log::debug () << "Unknwon sub-setting: " << pair.first << std::endl;
				return false;
			}
			if (!it->second.check (pair.second)) {
				Log::debug () << "Sub-setting \"" << pair.first << "\" is not valid." << std::endl;
				return false;
			}
		}
		return true;
	default:
		return false;
	}
}

Setting SettingDesc::convertFromString (const std::string &str) const
{
	switch (_type) {
	case Setting::Type::String:
		return str;

	case Setting::Type::Boolean:
		if (str == "true" || str == "on")
			return true;
		else if (str == "false" || str == "off")
			return false;
		else
			throw std::runtime_error ("string is not a boolean value");

	case Setting::Type::Integer: {
		char *end;
		int value = strtol (str.c_str (), &end, 0);
		if (*end != '\0')
			throw std::runtime_error ("string is not a number");
		if (value < _min || value > _max)
			throw std::runtime_error ("number is out of range");
		return value;
	}

	case Setting::Type::LEDVector: {
		LEDVector vec;
		for (unsigned int i = 0; i < _led_count; ++i) {
			if (i >= str.size ())
				throw std::runtime_error ("LED vector is too short");
			char c = str[i];
			if (c == '1')
				vec.push_back (true);
			else if (c == '0')
				vec.push_back (false);
			else
				throw std::runtime_error ("invalid character in LED vector");
		}
		return vec;
	}

	case Setting::Type::Color: {
		Color c;
		if (3 != sscanf (str.c_str (), "%02hhx%02hhx%02hhx", &c.r, &c.g, &c.b))
			throw  std::runtime_error ("string is not a color value");
		return c;
	}

	case Setting::Type::Enum:
		return Setting (EnumValue (*_enum_desc, _enum_desc->fromString (str)));

	case Setting::Type::ComposedSetting:
	default:
		throw std::logic_error ("invalid type in conversion");
	}
}

Setting SettingDesc::defaultValue () const
{
	return _default_value;
}

Setting::Type SettingDesc::type () const
{
	return _type;
}

std::pair<int, int> SettingDesc::integerRange () const
{
	assert (_type == Setting::Type::Integer);
	return std::make_pair (_min, _max);
}

unsigned int SettingDesc::LEDCount () const
{
	assert (_type == Setting::Type::LEDVector);
	return _led_count;
}

const EnumDesc &SettingDesc::enumDesc () const
{
	assert (_type == Setting::Type::Enum);
	return *_enum_desc;
}

bool SettingDesc::isComposed () const
{
	return _type == Setting::Type::ComposedSetting;
}

SettingDesc::const_iterator SettingDesc::find (const std::string &name) const
{
	return _sub_settings.find (name);
}

SettingDesc::const_iterator SettingDesc::begin () const
{
	return _sub_settings.begin ();
}

SettingDesc::const_iterator SettingDesc::end () const
{
	return _sub_settings.end ();
}

