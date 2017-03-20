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

#include "Log.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cstring>

Log::Category::Category (const char *tag, bool enabled_by_default):
	_enabled (enabled_by_default),
	_tag (tag)
{
}

void Log::Category::enable (bool enabled)
{
	_enabled = enabled;
}

void Log::Category::enable (const std::string &sub, bool enabled)
{
	_sub_categories[sub] = enabled;
}

bool Log::Category::isEnabled (const std::string &sub) const
{
	if (!sub.empty ()) {
		auto it = _sub_categories.find (sub);
		if (it != _sub_categories.end ())
			return it->second;
	}
	return _enabled;
}

std::string Log::Category::tag (const std::string &sub) const
{
	if (!sub.empty ())
		return _tag + ':' + sub;
	return _tag;
}

Log::Category Log::Error ("error", true);
Log::Category Log::Warning ("warning");
Log::Category Log::Info ("info");
Log::Category Log::Debug ("debug");

std::mutex Log::_mutex;

Log::Log ():
	_buf ("null")
{
}

Log::Log (const std::string &prefix):
	std::ostream (&_buf),
	_buf (prefix)
{
}

Log::Log (Log &&log):
	std::ostream (std::move (log)),
	_buf (std::move (log._buf))
{
}

Log::~Log ()
{
}

void Log::init (const char *setting_string)
{
	#define ADD_CATEGORY_BY_TAG(cat) { cat.tag (), &cat }
	static std::map<std::string, Category *> categories_by_tag {
		ADD_CATEGORY_BY_TAG (Log::Error),
		ADD_CATEGORY_BY_TAG (Log::Warning),
		ADD_CATEGORY_BY_TAG (Log::Info),
		ADD_CATEGORY_BY_TAG (Log::Debug),
	};
	const char *current;
	if (setting_string)
		current = setting_string;
	else if (!(current = getenv ("HIDPP_LOG")))
		return;

	// Enable verbose mode, if setting is present (even if empty).
	Log::Warning.enable ();

	const char *end = current + strlen (current);
	while (current != end) {
		bool enabling = true;
		const char *next = std::find (current, end, ',');
		const char *sub = std::find (current, next, ':');
		if (*current == '-') {
			enabling = false;
			++current;
		}
		std::string tag (current, sub);
		auto it = categories_by_tag.find (tag);
		if (it == categories_by_tag.end ())
			std::cerr << "Invalid log category tag: " << tag << std::endl;
		else {
			if (sub != next)
				it->second->enable (std::string (sub+1, next), enabling);
			else
				it->second->enable (enabling);
		}
		if (next != end)
			++next; // skip comma
		current = next;
	}
}

Log Log::log (const Category *category, const char *sub)
{
	if (sub && category->isEnabled (sub))
		return Log (category->tag (sub));
	if (!sub && category->isEnabled ())
		return Log (category->tag ());
	return Log ();
}

void Log::printf (const char *format, ...)
{
	char *str, *current, *end;
	int len;
	if (!*this)
		return;
	va_list args, copy;
	va_start (args, format);
	va_copy (copy, args);
	len = vsnprintf (nullptr, 0, format, copy);
	str = new char[len+1];
	vsnprintf (str, len+1, format, args);
	current = str;
	while (*current != '\0' && (end = strchr (current, '\n'))) {
		*this << std::string (current, end) << std::endl;
		current = end+1;
	}
	if (*current != '\0') {
		*this << std::string (current);
	}
	delete[] str;
	va_end (args);
}

Log::LogBuf::LogBuf (const std::string &prefix):
	_prefix (prefix)
{
}

int Log::LogBuf::sync ()
{
	std::unique_lock<std::mutex> lock (_mutex);
	std::cerr << "[" << _prefix << "] " << str ();
	str (std::string ());
	return 0;
}
