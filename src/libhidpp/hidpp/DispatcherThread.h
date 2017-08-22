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

#ifndef LIBHIDPP_HIDPP_DISPATCHER_THREAD_H
#define LIBHIDPP_HIDPP_DISPATCHER_THREAD_H

#include <hidpp/Dispatcher.h>
#include <hid/RawDevice.h>
#include <future>
#include <list>
#include <map>
#include <chrono>

namespace HIDPP
{

class DispatcherThread: public Dispatcher
{
public:
	class NotRunning: public std::exception
	{
	public:
		const char *what () const noexcept;
	};

	DispatcherThread (const char *path);
	~DispatcherThread ();

	const HID::RawDevice &hidraw () const;

	virtual uint16_t vendorID () const;
	virtual uint16_t productID () const;
	virtual std::string name () const;
	virtual void sendCommandWithoutResponse (const Report &report);
	virtual std::unique_ptr<Dispatcher::AsyncReport> sendCommand (Report &&report);
	virtual std::unique_ptr<Dispatcher::AsyncReport> getNotification (DeviceIndex index, uint8_t sub_id);


	virtual listener_iterator registerEventHandler (DeviceIndex index, uint8_t sub_id, const event_handler &handler);
	virtual void unregisterEventHandler (listener_iterator it);

	void run ();
	void stop ();

private:
	struct Command
	{
		Report request;
		std::promise<Report> response;
	};
	typedef std::list<Command> command_container;
	typedef command_container::iterator command_iterator;

	void cancelCommand (command_iterator);

	struct Notification
	{
		listener_iterator listener;
		std::promise<Report> notification;
	};
	typedef std::list<Notification> notification_container;
	typedef notification_container::iterator notification_iterator;

	void cancelNotification (notification_iterator);

	void processReport (std::vector<uint8_t> &&raw_report);

	HID::RawDevice _dev;
	command_container _commands;
	notification_container _notifications;
	std::mutex _command_mutex, _listener_mutex;
	bool _stopped;
	std::exception_ptr _exception;

	template<typename Iterator,
		 void (DispatcherThread::*cancel) (Iterator),
		 std::mutex DispatcherThread::*mutex>
	class AsyncReport;
	using AsyncCommandResponse = DispatcherThread::AsyncReport<
		DispatcherThread::command_iterator,
		&DispatcherThread::cancelCommand,
		&DispatcherThread::_command_mutex>;
	friend AsyncCommandResponse;
	using AsyncNotification = DispatcherThread::AsyncReport<
		DispatcherThread::notification_iterator,
		&DispatcherThread::cancelNotification,
		&DispatcherThread::_listener_mutex>;
	friend AsyncNotification;
};

}

#endif
