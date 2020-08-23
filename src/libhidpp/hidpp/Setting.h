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

#ifndef LIBHIDPP_HIDPP_SETTING_H
#define LIBHIDPP_HIDPP_SETTING_H

#include <vector>
#include <map>
#include <stdexcept>

#include <hidpp/Enum.h>

namespace HIDPP
{

struct Color
{
	uint8_t r, g, b;
};

typedef std::vector<bool> LEDVector;

class Setting;
typedef std::map<std::string, Setting> ComposedSetting;

//typedef std::variant<std::string, bool, int, LEDVector, Color, ComposedSetting, EnumValue> Setting;

class Setting
{
public:
	enum class Type {
		String = 0,
		Boolean,
		Integer,
		LEDVector,
		Color,
		ComposedSetting,
		Enum,
	};

	template<typename T> static Type type ();

	template<typename T>
	struct base_type
	{
		typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
	};

	template<typename T>
	Setting (T value):
		_type (type<typename base_type<T>::type> ()),
		_value (new typename base_type<T>::type (value))
	{
	}

	Setting (const Setting &other);
	Setting (Setting &&other);

	~Setting ();

	Type type () const;

	template<typename T>
	const T &get () const {
		if (_type != type<T> ())
			throw std::runtime_error ("Invalid type");
		return *reinterpret_cast<T *> (_value);
	}

	template<typename T>
	T &get () {
		if (_type != type<T> ())
			throw std::runtime_error ("Invalid type");
		return *reinterpret_cast<T *> (_value);
	}

	std::string toString () const;
private:
	Type _type;
	void *_value;
};

template<> Setting::Type Setting::type<std::string> ();
template<> Setting::Type Setting::type<bool> ();
template<> Setting::Type Setting::type<int> ();
template<> Setting::Type Setting::type<LEDVector> ();
template<> Setting::Type Setting::type<Color> ();
template<> Setting::Type Setting::type<ComposedSetting> ();
template<> Setting::Type Setting::type<EnumValue> ();

class SettingDesc
{
public:
	typedef std::map<std::string, SettingDesc> container;
	typedef container::const_iterator const_iterator;

	SettingDesc (const std::string &default_value);
	SettingDesc (bool default_value);
	SettingDesc (int min, int max, int default_value);
	SettingDesc (const LEDVector &default_value);
	SettingDesc (const Color &default_value);
	SettingDesc (std::initializer_list<container::value_type> sub_settings);
	SettingDesc (const container &sub_settings);
	SettingDesc (const EnumDesc &enum_desc, int default_value);

	bool check (const Setting &setting) const;

	Setting convertFromString (const std::string &str) const;
	Setting defaultValue () const;

	Setting::Type type () const;
	std::pair<int, int> integerRange () const;
	unsigned int LEDCount () const;
	const EnumDesc &enumDesc () const;

	bool isComposed () const;
	const_iterator begin () const;
	const_iterator end () const;
	const_iterator find (const std::string &name) const;

private:
	Setting::Type _type;
	int _min, _max;
	unsigned int _led_count;
	container _sub_settings;
	const EnumDesc *_enum_desc;
	Setting _default_value;
};

}

#endif
