/*
 * Copyright 2015 Clément Vuchener
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

#include "ProfileXML.h"

#include "MacroText.h"
#include <misc/UsageStrings.h>
#include <misc/Log.h>
#include <sstream>

using namespace HIDPP10;
using namespace tinyxml2;

void ButtonsToXML (const Profile *profile, const std::vector<Macro> &macros, XMLNode *node)
{
	XMLDocument *doc = node->GetDocument ();
	for (unsigned int i = 0; i < profile->buttonCount (); ++i) {
		const Profile::Button &button = profile->button (i);
		XMLElement *el;
		switch (button.type ()) {
		case Profile::Button::Macro: {
			el = doc->NewElement ("macro");
			std::string text = std::string ("\n") +
					   macroToText (macros[i].begin (),
							macros[i].end ());
			XMLText *txt = doc->NewText (text.c_str ());
			txt->SetCData (true);
			el->InsertEndChild (txt);
			break;
		}

		case Profile::Button::MouseButton: {
			el = doc->NewElement ("mouse-button");
			unsigned int button_mask = button.mouseButton ();
			el->SetText (buttonString (button_mask).c_str ());
			break;
		}

		case Profile::Button::Key: {
			el = doc->NewElement ("key");
			uint8_t modifier_mask = button.modifierKeys ();
			if (modifier_mask != 0) {
				el->SetAttribute ("modifiers", modifierString (modifier_mask).c_str ());
			}
			uint8_t key = button.key ();
			el->SetText (keyString (key).c_str ());
			break;
		}

		case Profile::Button::Special: {
			el = doc->NewElement ("special");
			Profile::Button::SpecialFunction special = button.special ();
			el->SetText (Profile::Button::specialFunctionToString (special).c_str ());
			break;
		}

		case Profile::Button::ConsumerControl: {
			el = doc->NewElement ("consumer-control");
			uint8_t cc = button.consumerControl ();
			el->SetText (consumerControlString (cc).c_str ());
			break;
		}

		case Profile::Button::Disabled:
			el = doc->NewElement ("disabled");
			break;
		}
		node->InsertEndChild (el);
	}
}

void G500ProfileToXML (const Profile *p, const std::vector<Macro> &macros, XMLNode *node)
{
	const G500Profile *profile = dynamic_cast<const G500Profile *> (p);
	if (!profile)
		return;

	XMLDocument *doc = node->GetDocument ();

	XMLElement *resolutions = doc->NewElement ("resolutions");
	for (unsigned int i = 0; i < profile->modeCount (); ++i) {
		G500Profile::ResolutionMode mode = profile->resolutionMode (i);
		XMLElement *resolution = doc->NewElement ("resolution");
		resolution->SetAttribute ("x", mode.x_res);
		resolution->SetAttribute ("y", mode.y_res);
		std::string leds;
		for (bool led: mode.leds)
			leds += (led ? "1" : "0");
		resolution->SetAttribute ("leds", leds.c_str ());
		resolutions->InsertEndChild (resolution);
	}
	resolutions->SetAttribute ("default", profile->defaultMode ());
	node->InsertEndChild (resolutions);

	XMLElement *polling = doc->NewElement ("polling-interval");
	polling->SetText (profile->pollInterval ());
	node->InsertEndChild (polling);

	XMLElement *angle_snap = doc->NewElement ("angle-snap");
	angle_snap->SetText (profile->angleSnap ());
	node->InsertEndChild (angle_snap);

	XMLElement *buttons = doc->NewElement ("buttons");
	ButtonsToXML (profile, macros, buttons);
	node->InsertEndChild (buttons);
}

void XMLToButtons (const XMLNode *node, Profile *profile, std::vector<Macro> &macros)
{
	unsigned int i = 0;
	const XMLElement *element = node->FirstChildElement ();
	while (element) {
		if (i > profile->buttonCount ()) {
			Log::warning () << "Too many buttons, last ones are ignored." << std::endl;
			break;
		}
		Profile::Button &button = profile->button (i);

		std::string name = element->Name ();
		if (name == "macro") {
			button.setMacro (Address ());
			macros[i] = textToMacro (element->GetText ());
		}
		else if (name == "mouse-button") {
			std::string str = element->GetText ();
			button.setMouseButton (buttonMask (str));
		}
		else if (name == "key") {
			unsigned int modifiers = 0;
			if (const char *attr = element->Attribute ("modifiers")) {
				modifiers = modifierMask (attr);
			}
			unsigned int key_code = keyUsageCode (element->GetText ());
			button.setKey (modifiers, key_code);
		}
		else if (name == "special") {
			Profile::Button::SpecialFunction special =
				Profile::Button::specialFunctionFromString (element->GetText ());
			button.setSpecial (special);
		}
		else if (name == "consumer-control") {
			unsigned int cc = consumerControlCode (element->GetText ());
			button.setConsumerControl (cc);
		}
		else if (name == "disabled") {
			button.disable ();
		}
		else {
			Log::warning () << "Ignoring button with invalid tag name " << element->Name () << std::endl;
		}
		element = element->NextSiblingElement ();
		++i;
	}
}

void XMLToG500Profile (const XMLNode *node, Profile *p, std::vector<Macro> &macros)
{
	G500Profile *profile = dynamic_cast<G500Profile *> (p);
	if (!profile)
		return;

	const XMLElement *element = node->FirstChildElement ();
	while (element) {
		std::string name = element->Name ();
		if (name == "resolutions") {
			std::vector<G500Profile::ResolutionMode> modes;
			const XMLElement *res_el = element->FirstChildElement ("resolution");
			while (res_el) {
				G500Profile::ResolutionMode mode;
				if (XML_NO_ERROR != res_el->QueryUnsignedAttribute ("x", &mode.x_res))
					Log::error () << "Invalid x resolution attribute." << std::endl;
				if (XML_NO_ERROR != res_el->QueryUnsignedAttribute ("y", &mode.y_res))
					Log::error () << "Invalid x resolution attribute." << std::endl;
				std::string leds = res_el->Attribute ("leds");
				for (unsigned int i = 0; i < leds.size (); ++i) {
					if (leds[i] == '0')
						mode.leds.push_back (false);
					else if (leds[i] == '1')
						mode.leds.push_back (true);
					else
						Log::error () << "Invalid LED value" << std::endl;
				}
				modes.push_back (mode);

				res_el = res_el->NextSiblingElement ("resolution");
			}

			profile->setModeCount (modes.size ());
			for (unsigned int i = 0; i < modes.size (); ++i)
				profile->setResolutionMode (i, modes[i]);

			unsigned int default_mode;
			if (XML_NO_ERROR != element->QueryUnsignedAttribute ("default", &default_mode))
					Log::error () << "Invalid default resolution mode attribute." << std::endl;
			profile->setDefaultMode (default_mode);
		}
		else if (name == "polling-interval") {
			unsigned int interval;
			if (XML_NO_ERROR != element->QueryUnsignedText (&interval))
				Log::error () << "Invalid polling interval." << std::endl;
			profile->setPollInterval (interval);
		}
		else if (name == "angle-snap") {
			bool angle_snap;
			if (XML_NO_ERROR != element->QueryBoolText (&angle_snap))
				Log::error () << "Invalid angle snap." << std::endl;
			profile->setAngleSnap (angle_snap);
		}
		else if (name == "buttons") {
			XMLToButtons (element, profile, macros);
		}
		else {
			Log::warning () << "Ignored element " << element->Name () << std::endl;
		}
		element = element->NextSiblingElement ();
	}
}
