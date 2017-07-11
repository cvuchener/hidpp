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

#ifndef LIBHIDPP_LOG_H
#define LIBHIDPP_LOG_H

#include <ostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <mutex>

class Log: public std::ostream
{
public:
	Log (const Log &) = delete;
	Log (Log &&);
	~Log ();

	class Category
	{
	public:
		/**
		 * \param tag	General tag for the category.
		 * \param enabled_by_default	Default state of the category.
		 */
		Category (const char *tag, bool enabled_by_default = false);

		/**
		 * Enable (or disable) the whole category.
		 */
		void enable (bool enabled = true);
		/**
		 * Enable (or disable) the specific subcategory.
		 */
		void enable (const std::string &sub, bool enabled = true);
		/**
		 * Check is the category (and optionally, the subcategory)
		 * is enabled.
		 */
		bool isEnabled (const std::string &sub = std::string ()) const;


		/**
		 * Get the tag string for this category (and subcategory).
		 */
		std::string tag (const std::string &sub = std::string ()) const;
	private:
		bool _enabled;
		std::map<std::string, bool> _sub_categories;
		std::string _tag;
	};
	static Category Error, Warning, Info, Debug;

	/**
	 * Initialize categories and subcategories states from parameter string
	 * or environment variable.
	 *
	 * If not called categories are in their default state (only Error
	 * is enabled).
	 *
	 * If \p setting_string is null, settings are taken from the environment
	 * variable HIDPP_LOG instead of the parameter.
	 *
	 * If the setting string is set but empty, default verbose mode is used
	 * (Warning is enabled).
	 *
	 * The setting string is a comma-separated list of categories settings
	 * in the format [-]category[:subcategory]. If the setting begins with '-'
	 * the category is disabled instead of enabled. Subcategories can have
	 * different settings than their category: "debug,-debug:uninteresting"
	 * would activate debug messages except for "uninteresting" messages.
	 *
	 * Only the default categories "error", "warning", "info" and "debug"
	 * are initialized.
	 */
	static void init (const char *setting_string = nullptr);
	/**
	 * Get a log object for the corresponding category (and subcategory).
	 */
	static Log log (const Category *category, const char *sub = nullptr);

	static inline Log error (const char *sub = nullptr) { return log (&Error, sub); }
	static inline Log warning (const char *sub = nullptr) { return log (&Warning, sub); }
	static inline Log info (const char *sub = nullptr) { return log (&Info, sub); }
	static inline Log debug (const char *sub = nullptr) { return log (&Debug, sub); }

	void printf (const char *format, ...)
		__attribute__ ((format (printf, 2, 3)));

	template <class InputIterator>
	void printBytes (const std::string &prefix,
			 InputIterator begin, InputIterator end) {
		if (!*this)
			return;
		*this << prefix;
		for (auto it = begin; it != end; ++it) {
			uint8_t byte = *it;
			*this << " "
			      << std::hex
			      << std::setw (2)
			      << std::setfill ('0')
			      << static_cast<unsigned int> (byte);
		}
		*this << std::endl;
	}

private:
	Log ();
	Log (const std::string &prefix);

	class LogBuf: public std::stringbuf {
	public:
		LogBuf (const std::string &prefix);
		virtual int sync ();

	private:
		std::string _prefix;
	} _buf;

	static std::mutex _mutex;
};

#endif
