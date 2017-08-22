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

#ifndef LIBHIDPP_HIDPP_SIMPLE_DISPATCHER_H
#define LIBHIDPP_HIDPP_SIMPLE_DISPATCHER_H

#include <hidpp/Dispatcher.h>
#include <hid/RawDevice.h>

namespace HIDPP
{

/**
 * Simple single-threaded dispatcher.
 *
 * Except for stop(), no method should be called
 * when another one is being called or when a command
 * or a notification is in progress. Since events can
 * be received while waiting for a command response
 * or notification, event handlers should not send
 * commands or wait for notifications on the same
 * dispatcher.
 *
 * TODO: Fix timeout being reset each time a report is received.
 */
class SimpleDispatcher: public Dispatcher
{
public:
	SimpleDispatcher (const char *path);
	~SimpleDispatcher ();

	const HID::RawDevice &hidraw () const;

	virtual uint16_t vendorID () const;
	virtual uint16_t productID () const;
	virtual std::string name () const;
	virtual void sendCommandWithoutResponse (const Report &report);
	virtual std::unique_ptr<Dispatcher::AsyncReport> sendCommand (Report &&report);
	virtual std::unique_ptr<Dispatcher::AsyncReport> getNotification (DeviceIndex index, uint8_t sub_id);

	void listen ();
	void stop ();

private:
	Report getReport (int timeout = -1);

	HID::RawDevice _dev;

	class CommandResponse: public Dispatcher::AsyncReport
	{
		SimpleDispatcher *dispatcher;
		Report report;
	public:
		CommandResponse (SimpleDispatcher *, Report &&);
		virtual Report get ();
		virtual Report get (int timeout);
	};
	friend CommandResponse;
	class Notification: public Dispatcher::AsyncReport
	{
		SimpleDispatcher *dispatcher;
		DeviceIndex index;
		uint8_t sub_id;
	public:
		Notification (SimpleDispatcher *, DeviceIndex, uint8_t);
		virtual Report get ();
		virtual Report get (int timeout);
	};
	friend Notification;
};

}

#endif
