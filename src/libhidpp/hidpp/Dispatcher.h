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

#ifndef HIDPP_DISPATCHER_H
#define HIDPP_DISPATCHER_H

#include <misc/HIDRaw.h>
#include <misc/EventQueue.h>
#include <hidpp/Report.h>
#include <thread>
#include <future>
#include <list>
#include <map>
#include <functional>
#include <chrono>

namespace HIDPP
{

class Dispatcher
{
	struct Command
	{
		Report report;
		std::promise<Report> promised_report;
	};
	typedef std::list<Command> command_container;
	typedef command_container::iterator command_iterator;
	struct Listener
	{
		std::function<void (const Report &)> fn;
		bool only_once;
		Listener (const std::function<void (const Report &)> fn, bool only_once);
	};
	typedef std::multimap<std::tuple<DeviceIndex, uint8_t>, Listener> listener_container;
public:
	/**
	 * Exception when no HID++ report is found in the report descriptor.
	 */
	class NoHIDPPReportException: public std::exception
	{
	public:
		NoHIDPPReportException ();
		virtual const char *what () const noexcept;
	};

	class TimeoutError: public std::exception
	{
		virtual const char *what () const noexcept;
	};

	Dispatcher (const char *path);
	~Dispatcher ();

	const HIDRaw &hidraw () const;

	/**
	 * Get the version for the device with the given index.
	 *
	 * Some devices fail to answer with a valid error message
	 * when the device index is not supported. This methods throws
	 * TimeourError if an answer is not received fast enough.
	 *
	 * \return Major and minor version number.
	 */
	std::tuple<unsigned int, unsigned int> getVersion (DeviceIndex index);

	/**
	 * Sends the report without expecting any answer from the device.
	 */
	void sendCommandWithoutResponse (const Report &report);

	/**
	 * Sends the report expecting a matching (device index, sub ID and address)
	 * answer.
	 *
	 * This method returns immediately after sending the report. Receiving the
	 * answer is done asynchronously.
	 *
	 * \returns the future answer report.
	 */
	std::future<Report> sendCommand (Report &&report);

	/**
	 * Get exactly one notification matching \p index and \p sub_id.
	 */
	std::future<Report> getNotification (DeviceIndex index, uint8_t sub_id);

	typedef listener_container::iterator listener_iterator;
	/**
	 * Add a listener function for events matching \p index and \p sub_id.
	 *
	 * \param index		Event device index
	 * \param sub_id	Event sub_id (or feature index)
	 * \param queue		Queue where events will be pushed.
	 *
	 * \returns The listener iterator used for unregistering.
	 */
	listener_iterator registerEventQueue (DeviceIndex index, uint8_t sub_id, EventQueue<Report> *queue);
	/**
	 * Unregister the event queue given by the iterator.
	 */
	void unregisterEventQueue (listener_iterator it);

private:
	void run ();
	void processReport (std::vector<uint8_t> &&raw_report);

	HIDRaw _dev;
	command_container _commands;
	listener_container _listeners;
	std::mutex _mutex;
	bool _stop;
	std::thread _thread;
};

}

#endif
