/*
 * Copyright 2019 Clément Vuchener
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

#include <hidpp/SimpleDispatcher.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/ILEDControl.h>
#include <cstdio>
#include <iostream>
#include <memory>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

using namespace HIDPP20;

static const char *LEDType (ILEDControl::Type type)
{
	switch (type) {
	case ILEDControl::Battery:
		return "Battery";
	case ILEDControl::Dpi:
		return "Dpi";
	case ILEDControl::Profile:
		return "Profile";
	case ILEDControl::Logo:
		return "Logo";
	case ILEDControl::Cosmetic:
		return "Cosmetic";
	default:
		return "unknown";
	}
}

template<typename T>
struct Enum
{
	std::map<T, std::string> names;
	std::map<std::string, T> values;

	Enum (std::initializer_list<std::pair<const T, std::string>> pairs): names (pairs)
	{
		for (const auto &p: pairs)
			values.emplace (p.second, p.first);
	}

	const std::string &name (T value) const
	{
		static const std::string unknown = "unknown";
		auto it = names.find (value);
		if (it == names.end ())
			return unknown;
		else
			return it->second;
	}

	template<typename K>
	T value (K &&name) const
	{
		auto it = values.find (std::forward<K> (name));
		if (it == values.end ())
			throw std::invalid_argument ("unknown value");
		else
			return it->second;
	}
};

static const Enum<ILEDControl::Mode> Modes = {
	{ ILEDControl::Off, "off" },
	{ ILEDControl::On, "on" },
	{ ILEDControl::Blink, "blink" },
	{ ILEDControl::Travel, "travel" },
	{ ILEDControl::RampUp, "rampup" },
	{ ILEDControl::RampDown, "rampdown" },
	{ ILEDControl::Heartbeat, "heartbeat" },
	{ ILEDControl::Breathing, "breathing" },
};

static const Enum<ILEDControl::Config> Configs = {
	{ ILEDControl::AlwaysOff, "alwaysoff" },
	{ ILEDControl::AlwaysOn, "alwayson" },
	{ ILEDControl::Auto, "auto" },
};

static void print_state(const ILEDControl::State &state)
{
	std::cout << Modes.name (state.mode);
	switch (state.mode) {
	case ILEDControl::On:
		std::cout << "\tindex: " << state.on.index;
		break;
	case ILEDControl::Blink:
		std::cout << "\tindex: " << state.blink.index;
		std::cout << ", on duration: " << state.blink.on_duration;
		std::cout << ", off duration: " << state.blink.off_duration;
		break;
	case ILEDControl::Travel:
		std::cout << "\tdelay: " << state.travel.delay;
		break;
	case ILEDControl::Breathing:
		std::cout << "\tmax brightness: " << state.breathing.max_brightness;
		std::cout << ", period: " << state.breathing.period;
		std::cout << ", timeout: " << state.breathing.timeout;
		break;
	default:
		break;
	}
	std::cout << std::endl;
}

int main (int argc, char *argv[])
{
	static const char *args = "device_path info|control|state|config [params...]";
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

	if (argc-first_arg < 2) {
		std::cerr << "Too few arguments." << std::endl;
		std::cerr << getUsage (argv[0], args, &options) << std::endl;
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];
	std::string op = argv[first_arg+1];
	first_arg += 2;

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (path);
	}
	catch (std::exception &e) {
		std::cerr << "Failed to open device: " << e.what () << "." << std::endl;
		return EXIT_FAILURE;
	}
	Device dev (dispatcher.get (), device_index);
	try {
		char *endptr;
		ILEDControl iled (&dev);
		if (op == "info") {
			unsigned int count = iled.getCount ();
			std::cout << "LED\tType\tModes\tConfigs" << std::endl;
			for (unsigned int i = 0; i < count; ++i) {
				auto info = iled.getInfo (i);
				std::cout << i << "\t" << LEDType (info.type) << "\t";
				if (info.modes != 0) {
					bool first = true;
					for (unsigned int j = 0; j < 16; ++j) {
						auto mode = static_cast<ILEDControl::Mode>(1<<j);
						if (!(info.modes & mode))
							continue;
						if (first)
							first = false;
						else
							printf (",");
						std::cout << Modes.name (mode);
					}
				}
				else {
					std::cout << "N/A";
				}
				std::cout << "\t";
				if (info.config_capabilities != 0) {
					bool first = true;
					for (unsigned int j = 0; j < 8; ++j) {
						auto config = static_cast<ILEDControl::Config>(1<<j);
						if (!(info.config_capabilities & config))
							continue;
						if (first)
							first = false;
						else
							printf (",");
						std::cout << Configs.name (config);
					}
				}
				else {
					std::cout << "N/A";
				}
				std::cout << std::endl;
			}
		}
		else if (op == "control") {
			if (argc-first_arg < 1) {
				bool swcontrol = iled.getSWControl ();
				std::cout << (swcontrol ? "sw" : "hw") << std::endl;
			}
			else {
				std::string mode = argv[first_arg];
				if (mode == "sw") {
					iled.setSWControl (true);
				} else if (mode == "hw") {
					iled.setSWControl (false);
				}
				else {
					std::cerr << "Invalid control mode." << std::endl;
					return EXIT_FAILURE;
				}
			}
		}
		else if (op == "state") {
			if (argc-first_arg < 1) {
				unsigned int count = iled.getCount ();
				for (unsigned int i = 0; i < count; ++i) {
					auto state = iled.getState (i);
					std::cout << i << "\t";
					print_state(state);
				}
			}
			else {
				int index = strtol (argv[first_arg], &endptr, 0);
				if (*endptr != '\0' || index < 0 || index > 255) {
					std::cerr << "Invalid LED index." << std::endl;
					return EXIT_FAILURE;
				}
				if (argc-first_arg < 2) {
					print_state(iled.getState (index));
				}
				else {
					auto mode = Modes.value(argv[first_arg+1]);
				}
			}

		}
		else {
			std::cerr << "Invalid operation: " << op << "." << std::endl;
			return EXIT_FAILURE;
		}
	}
	catch (Error &e) {
		std::cerr << "Error code " << e.errorCode () << ": " << e.what () << std::endl;
		return e.errorCode ();
	}

	return EXIT_SUCCESS;
}

