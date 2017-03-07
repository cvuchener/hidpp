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

class ButtonListener
{
	unsigned int _button_count;
	uint16_t _button_state;
public:
	ButtonListener (unsigned int button_count):
		_button_count (button_count),
		_button_state (0)
	{
		printf ("The mouse has %d buttons.\n", _button_count);
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

void sigint (int)
{
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
	IMouseButtonSpy imbs (&dev);
	ButtonListener listener (imbs.getMouseButtonCount ());

	auto it = dispatcher.registerEventListener (device_index, imbs.index (), std::bind (&ButtonListener::handleEvent, &listener, std::placeholders::_1));
	imbs.startMouseButtonSpy ();
	struct sigaction sa;
	memset (&sa, 0, sizeof (struct sigaction));
	sa.sa_handler = sigint;
	sigaction (SIGINT, &sa, nullptr);
	pause ();
	imbs.stopMouseButtonSpy ();
	dispatcher.unregisterEventListener (it);

	return EXIT_SUCCESS;
}

