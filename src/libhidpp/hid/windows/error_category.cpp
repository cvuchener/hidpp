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

#include "error_category.h"

#include <locale>
#include <codecvt>

extern "C" {
#include <windows.h>
}

#define ARRAY_SIZE(a) (sizeof (a)/sizeof ((a)[0]))

using namespace windows;

const char *error_category::name () const noexcept
{
	return "windows";
}

std::string error_category::message (int condition) const
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;
	wchar_t str[256];
	DWORD len = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, NULL, condition, 0, str, ARRAY_SIZE (str), NULL);
	for (DWORD i = 0; i < len; ++i)
		if (str[i] == L'\r' || str[i] == L'\n')
			str[i] = L'\0';
	return wconv.to_bytes (std::wstring (str, len));
}

const std::error_category &windows_category () noexcept
{
	static windows::error_category category;
	return category;
}
