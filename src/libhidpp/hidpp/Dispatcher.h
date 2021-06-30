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

#ifndef LIBHIDPP_HIDPP_DISPATCHER_H
#define LIBHIDPP_HIDPP_DISPATCHER_H

#include <hidpp/Report.h>
#include <memory>
#include <map>
#include <functional>
#include <optional>

namespace HIDPP
{

class Dispatcher
{
public:
	typedef std::function<bool (const Report &)> event_handler;
	typedef std::multimap<std::tuple<DeviceIndex, uint8_t>, event_handler> listener_container;
	typedef listener_container::iterator listener_iterator;

	/**
	 * Exception when no HID++ report is found in the report descriptor.
	 */
	class NoHIDPPReportException: public std::exception
	{
	public:
		virtual const char *what () const noexcept;
	};

	class TimeoutError: public std::exception
	{
		virtual const char *what () const noexcept;
	};

	class AsyncReport
	{
	public:
		virtual ~AsyncReport ();

		/**
		 * Get the report.
		 *
		 * This method blocks until the report is received.
		 *
		 * \throws HIDPP10::Error, HIDPP20::Error,
		 *	std::system_error, std::runtime_error
		 */
		virtual Report get () = 0;

		/**
		 * Get the report with a timeout.
		 *
		 * This method blocks until the report is received
		 * or the timeout expires.
		 *
		 * A TimeoutError is thrown is no report is received.
		 *
		 * \throws HIDPP10::Error, HIDPP20::Error,
		 *	std::system_error, std::runtime_error
		 */
		virtual Report get (int timeout) = 0;
	};

	virtual ~Dispatcher ();

	virtual uint16_t vendorID () const = 0;
	virtual uint16_t productID () const = 0;
	virtual std::string name () const = 0;

	/**
	 * Sends the report without expecting any answer from the device.
	 */
	virtual void sendCommandWithoutResponse (const Report &report) = 0;

	/**
	 * Sends the report expecting a matching (device index, sub ID and address)
	 * answer.
	 *
	 * \returns object for retrieving the answer report asynchronously.
	 */
	virtual std::unique_ptr<AsyncReport> sendCommand (Report &&report) = 0;

	/**
	 * Get exactly one notification matching \p index and \p sub_id.
	 *
	 * \return object for retrieving the notification report asynchronously.
	 */
	virtual std::unique_ptr<AsyncReport> getNotification (DeviceIndex index, uint8_t sub_id) = 0;

	/**
	 * Add a listener function for events matching \p index and \p sub_id.
	 *
	 * \param index		Event device index
	 * \param sub_id	Event sub_id (or feature index)
	 * \param handler	Callback for handling the event
	 *
	 * \returns The listener iterator used for unregistering.
	 */
	virtual listener_iterator registerEventHandler (DeviceIndex index, uint8_t sub_id, const event_handler &handler);

	/**
	 * Unregister the event handler given by the iterator.
	 */
	virtual void unregisterEventHandler (listener_iterator it);

	struct ReportInfo {
		enum Flags { // flags are also the usage for collections and reports
			HasShortReport = 1<<0,
			HasLongReport = 1<<1,
			HasVeryLongReport = 1<<2,
		};
		int flags;

		bool hasReport (Report::Type type) const noexcept {
			switch (type) {
			case Report::Short: return flags & HasShortReport;
			case Report::Long: return flags & HasLongReport;
			case Report::VeryLong: return flags & HasVeryLongReport;
			default: return false;
			}
		}

		std::optional<Report::Type> findReport (std::size_t minimum_parameter_length = 0) const noexcept {
			for (auto type: { Report::Short, Report::Long, Report::VeryLong })
				if (hasReport (type) && minimum_parameter_length <= Report::parameterLength (type))
					return type;
			return std::nullopt;
		}
	};
	ReportInfo reportInfo () const noexcept { return _report_info; }

protected:
	void processEvent (const Report &);
	void checkReportDescriptor (const HID::ReportDescriptor &report_desc);

private:
	listener_container _listeners;
	ReportInfo _report_info;
};

}

#endif
