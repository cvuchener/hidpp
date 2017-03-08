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

#include <hidpp/Dispatcher.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/IMouseButtonSpy.h>
#include <hidpp20/IOnboardProfiles.h>
#include <hidpp20/UnsupportedFeature.h>
#include <cstdio>
#include <memory>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

extern "C" {
#include <unistd.h>
#include <signal.h>
#include <string.h>
}

using namespace HIDPP20;

class EventListener
{
public:
	virtual const HIDPP20::FeatureInterface *feature () const = 0;
	virtual void handleEvent (const HIDPP::Report &event) = 0;
};

class ButtonListener: public EventListener
{
	HIDPP20::IMouseButtonSpy _imbs;
	HIDPP::Dispatcher::listener_iterator _it;
	unsigned int _button_count;
	uint16_t _button_state;
public:
	ButtonListener (HIDPP20::Device *dev, EventQueue<HIDPP::Report> *queue):
		_imbs (dev),
		_it (dev->dispatcher ()->registerEventQueue (dev->deviceIndex (), _imbs.index (), queue)),
		_button_count (_imbs.getMouseButtonCount ()),
		_button_state (0)
	{
		printf ("The mouse has %d buttons.\n", _button_count);
		_imbs.startMouseButtonSpy ();
	}

	~ButtonListener ()
	{
		_imbs.stopMouseButtonSpy ();
		_imbs.device ()->dispatcher ()->unregisterEventQueue (_it);
	}

	const HIDPP20::FeatureInterface *feature () const
	{
		return &_imbs;
	}

	void handleEvent (const HIDPP::Report &event)
	{
		uint16_t new_state;
		switch (event.function ()) {
		case IMouseButtonSpy::MouseButtonEvent:
			new_state = IMouseButtonSpy::mouseButtonEvent (event);
			for (unsigned int i = 0; i < _button_count; ++i) {
				if ((_button_state ^ new_state) & (1<<i))
					printf ("Button %d: %s\n", i, (new_state & (1<<i) ? "pressed" : "released"));
			}
			_button_state = new_state;
			break;
		}
	}
};

class ProfileListener: public EventListener
{
	HIDPP20::IOnboardProfiles _iop;
	HIDPP::Dispatcher::listener_iterator _it;
	HIDPP20::IOnboardProfiles::Mode _old_mode;
public:
	ProfileListener (HIDPP20::Device *dev, EventQueue<HIDPP::Report> *queue):
		_iop (dev),
		_it (dev->dispatcher ()->registerEventQueue (dev->deviceIndex (), _iop.index (), queue)),
		_old_mode (_iop.getMode ())
	{
		_iop.setMode (IOnboardProfiles::Mode::Onboard);
	}

	~ProfileListener ()
	{
		_iop.setMode (_old_mode);
		_iop.device ()->dispatcher ()->unregisterEventQueue (_it);
	}

	const HIDPP20::FeatureInterface *feature () const
	{
		return &_iop;
	}

	void handleEvent (const HIDPP::Report &event)
	{
		switch (event.function ()) {
		case IOnboardProfiles::CurrentProfileChanged:
			printf ("Current profile changed: %u\n",
				IOnboardProfiles::currentProfileChanged (event));
			break;
		case IOnboardProfiles::CurrentDPIIndexChanged:
			printf ("Current dpi index changed: %u\n",
			IOnboardProfiles::currentDPIIndexChanged (event));
			break;
		}
	}
};

EventQueue<HIDPP::Report> *queue;

void sigint (int)
{
	queue->interrupt ();
}

int main (int argc, char *argv[])
{
	static const char *args = "/dev/hidrawX";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;

	std::vector<Option> options = {
		DeviceIndexOption (device_index),
		VerboseOption (),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	if (argc-first_arg < 1) {
		fprintf (stderr, "Too few arguments.\n");
		fprintf (stderr, "%s", getUsage (argv[0], args, &options).c_str ());
		return EXIT_FAILURE;
	}

	HIDPP::Dispatcher dispatcher (argv[first_arg]);
	Device dev (&dispatcher, device_index);

	queue = new EventQueue<HIDPP::Report>;
	struct sigaction sa;
	memset (&sa, 0, sizeof (struct sigaction));
	sa.sa_handler = sigint;
	sigaction (SIGINT, &sa, nullptr);

	{
		std::map<uint8_t, std::unique_ptr<EventListener>> listeners;
		try {
			auto ptr = std::make_unique<ButtonListener> (&dev, queue);
			listeners.emplace (ptr->feature ()->index (), std::move (ptr));
		}
		catch (HIDPP20::UnsupportedFeature e) {
			printf ("%s\n", e.what ());
		}
		try {
			auto ptr = std::make_unique<ProfileListener> (&dev, queue);
			listeners.emplace (ptr->feature ()->index (), std::move (ptr));
		}
		catch (HIDPP20::UnsupportedFeature e) {
			printf ("%s\n", e.what ());
		}
		while (auto opt = queue->pop ()) {
			const auto &report = opt.value ();
			listeners[report.featureIndex ()]->handleEvent (report);
		}
	}
	delete queue;

	return EXIT_SUCCESS;
}

