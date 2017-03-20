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

#ifndef HIDPP_DISPATCHER_THREAD_H
#define HIDPP_DISPATCHER_THREAD_H

#include <hidpp/Dispatcher.h>
#include <misc/HIDRaw.h>
#include <misc/EventQueue.h>
#include <thread>
#include <future>
#include <list>
#include <map>
#include <functional>
#include <chrono>

namespace HIDPP
{

class DispatcherThread: public Dispatcher
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
		std::function<void (std::exception_ptr)> except;
		bool only_once;
		Listener (const std::function<void (const Report &)> &fn,
			  const std::function<void (std::exception_ptr)> &except,
			  bool only_once);
	};
	typedef std::multimap<std::tuple<DeviceIndex, uint8_t>, Listener> listener_container;

public:
	class NotRunning: public std::exception
	{
	public:
		const char *what () const noexcept;
	};

	DispatcherThread (const char *path);
	~DispatcherThread ();

	const HIDRaw &hidraw () const;

	virtual uint16_t vendorID () const;
	virtual uint16_t productID () const;
	virtual std::string name () const;
	virtual void sendCommandWithoutResponse (const Report &report);
	virtual std::unique_ptr<Dispatcher::AsyncReport> sendCommand (Report &&report);
	virtual std::unique_ptr<Dispatcher::AsyncReport> getNotification (DeviceIndex index, uint8_t sub_id);

	typedef listener_container::iterator listener_iterator;
	/**
	 * Add a listener function for events matching \p index and \p sub_id.
	 *
	 * The queue is interrupted is the thread is stopped while
	 * it is still registered.
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

	bool running () const;

private:
	void run ();
	void processReport (std::vector<uint8_t> &&raw_report);

	HIDRaw _dev;
	command_container _commands;
	listener_container _listeners;
	std::mutex _mutex;
	std::thread _thread;
	bool _running;
	std::exception_ptr _exception;

	template<typename Container, Container DispatcherThread::*container>
	class AsyncReport;
	using CommandResponse = DispatcherThread::AsyncReport<
		DispatcherThread::command_container,
		&DispatcherThread::_commands>;
	friend CommandResponse;
	using Notification = DispatcherThread::AsyncReport<
		DispatcherThread::listener_container,
		&DispatcherThread::_listeners>;
	friend Notification;
};

}

#endif
