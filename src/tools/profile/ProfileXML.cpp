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

#include <hid/UsageStrings.h>
#include <misc/Log.h>
#include <sstream>

using namespace HIDPP;
using namespace HID;
using namespace tinyxml2;

ProfileXML::ProfileXML (const AbstractProfileFormat *profile_format,
			const AbstractProfileDirectoryFormat *profdir_format):
	_profile_settings (profile_format->generalSettings ()),
	_mode_settings (profile_format->modeSettings ()),
	_entry_settings (profdir_format->settings ()),
	_special_actions (profile_format->specialActions ())
{
}

static
void InsertCData (XMLElement *el, const std::string &str)
{
	XMLDocument *doc = el->GetDocument ();
	XMLText *txt = doc->NewText (str.c_str ());
	txt->SetCData (true);
	el->InsertEndChild (txt);
}

// TODO: add macro vector
static
void insertButton (const Profile::Button &button, const Macro &macro, XMLNode *parent, const EnumDesc &special_actions)
{
	XMLDocument *doc = parent->GetDocument ();
	XMLElement *el;
	switch (button.type ()) {
	case Profile::Button::Type::Macro: {
		el = doc->NewElement ("macro");
		Macro::const_iterator pre_begin, pre_end, loop_begin, loop_end, post_begin, post_end;
		unsigned int loop_delay;
		if (macro.isSimple ()) {
			std::string str = std::string ("\n") + macroToText (macro.begin (), std::prev (macro.end ()));
			InsertCData (el, str);
			el->SetAttribute ("type", "simple");
		}
		else if (macro.isLoop (pre_begin, pre_end, loop_begin, loop_end, post_begin, post_end, loop_delay)) {
			std::string pre_str = std::string ("\n") + macroToText (pre_begin, pre_end);
			std::string loop_str = std::string ("\n") + macroToText (loop_begin, loop_end);
			std::string post_str = std::string ("\n") + macroToText (post_begin, post_end);

			XMLElement *pre = doc->NewElement ("pre");
			InsertCData (pre, pre_str);
			el->InsertEndChild (pre);

			XMLElement *loop = doc->NewElement ("loop");
			InsertCData (loop, loop_str);
			el->InsertEndChild (loop);

			XMLElement *post = doc->NewElement ("post");
			InsertCData (post, post_str);
			el->InsertEndChild (post);

			el->SetAttribute ("type", "loop");
			el->SetAttribute ("loop-delay", loop_delay);
		}
		else {
			std::string str = std::string ("\n") + macroToText (macro.begin (), macro.end ());
			XMLText *txt = doc->NewText (str.c_str ());
			txt->SetCData (true);
			el->InsertEndChild (txt);
			el->SetAttribute ("type", "advanced");
		}
		break;
	}

	case Profile::Button::Type::MouseButtons: {
		el = doc->NewElement ("mouse-button");
		unsigned int button_mask = button.mouseButtons ();
		el->SetText (buttonString (button_mask).c_str ());
		break;
	}

	case Profile::Button::Type::Key: {
		el = doc->NewElement ("key");
		uint8_t modifier_mask = button.modifierKeys ();
		if (modifier_mask != 0) {
			el->SetAttribute ("modifiers", modifierString (modifier_mask).c_str ());
		}
		uint8_t key = button.key ();
		if (key != 0)
			el->SetText (keyString (key).c_str ());
		break;
	}

	case Profile::Button::Type::Special: {
		el = doc->NewElement ("special");
		unsigned int special = button.special ();
		try {
			el->SetText (special_actions.toString (special).c_str ());
		}
		catch (std::exception &e) {
			Log::error () << "Failed to set special action text: " << e.what () << std::endl;
		}
		break;
	}

	case Profile::Button::Type::ConsumerControl: {
		el = doc->NewElement ("consumer-control");
		uint8_t cc = button.consumerControl ();
		if (cc != 0)
			el->SetText (consumerControlString (cc).c_str ());
		break;
	}

	case Profile::Button::Type::Disabled:
		el = doc->NewElement ("disabled");
		break;
	}
	parent->InsertEndChild (el);
}

static
void insertSetting (const std::string &name, const Setting &value, XMLNode *parent)
{
	XMLDocument *doc = parent->GetDocument ();
	XMLElement *element = doc->NewElement (name.c_str ());
	if (value.type () == Setting::Type::ComposedSetting) {
		for (const auto &p: value.get<ComposedSetting> ())
			insertSetting (p.first, p.second, element);
	}
	else {
		try {
			element->SetText (value.toString ().c_str ());
		}
		catch (std::exception &e) {
			Log::error () << "Failed to set setting \"" << name
				      << "\" text: " << e.what () << std::endl;
		}
	}
	parent->InsertEndChild (element);
}

void ProfileXML::write (const Profile &profile, const ProfileDirectory::Entry &entry, const std::vector<Macro> &macros, XMLNode *node)
{
	XMLDocument *doc = node->GetDocument ();

	for (const auto &p: entry.settings)
		insertSetting (p.first, p.second, node);

	XMLElement *modes = doc->NewElement ("modes");
	for (const auto &mode: profile.modes) {
		XMLElement *mode_el = doc->NewElement ("mode");
		for (const auto &p: mode)
			insertSetting (p.first, p.second, mode_el);
		modes->InsertEndChild (mode_el);
	}
	node->InsertEndChild (modes);

	for (const auto &p: profile.settings)
		insertSetting (p.first, p.second, node);

	XMLElement *buttons = doc->NewElement ("buttons");
	for (unsigned int i = 0; i < profile.buttons.size (); ++i) {
		const auto &button = profile.buttons[i];
		const auto &macro = macros[i];
		insertButton (button, macro, buttons, _special_actions);
	}
	node->InsertEndChild (buttons);
}

static
void readButton (const XMLElement *element, Profile::Button &button, Macro &macro, const EnumDesc &special_actions)
{
	const char *text;

	std::string name = element->Name ();
	if (name == "macro") {
		button.setMacro (Address ());
		std::string type;
		if (element->Attribute ("type"))
			type = element->Attribute ("type");
		if (type.empty () || type == "simple") {
			if ((text = element->GetText ())) {
				Macro simple = textToMacro (text);
				macro = Macro::buildSimple (simple.begin (), simple.end ());
			}
		}
		else if (type == "loop") {
			unsigned int loop_delay;
			switch (element->QueryUnsignedAttribute ("loop-delay", &loop_delay)) {
			case XML_SUCCESS:
				break;
			case XML_WRONG_ATTRIBUTE_TYPE:
				Log::error () << "Invalid loop delay value." << std::endl;
				// Fall-through default value
			case XML_NO_ATTRIBUTE:
				loop_delay = 0;
				break;
			default:
				throw std::logic_error ("Unexpected tinyxml2 error");
			}

			Macro pre, loop, post;
			const XMLElement *pre_el = element->FirstChildElement ("pre");
			if (pre_el && (text = pre_el->GetText ())) {
				pre = textToMacro (text);
			}
			const XMLElement *loop_el = element->FirstChildElement ("loop");
			if (loop_el && (text = loop_el->GetText ())) {
				loop = textToMacro (text);
			}
			const XMLElement *post_el = element->FirstChildElement ("post");
			if (post_el && (text = post_el->GetText ())) {
				post = textToMacro (text);
			}
			macro = Macro::buildLoop (pre.begin (), pre.end (),
						  loop.begin (), loop.end (),
						  post.begin (), post.end (),
						  loop_delay);

		}
		else if (type == "advanced" && (text = element->GetText ())) {
			macro = textToMacro (text);
		}
	}
	else if (name == "mouse-button") {
		std::string str = element->GetText ();
		button.setMouseButtons (buttonMask (str));
	}
	else if (name == "key") {
		unsigned int modifiers = 0;
		if (const char *attr = element->Attribute ("modifiers")) {
			modifiers = modifierMask (attr);
		}
		unsigned int key_code = ((text = element->GetText ()) ? keyUsageCode (text) : 0);
		button.setKey (modifiers, key_code);
	}
	else if (name == "special") {
		if (!(text = element->GetText ()))
			throw std::runtime_error ("empty special string");
		unsigned int special = special_actions.fromString (text);
		button.setSpecial (special);
	}
	else if (name == "consumer-control") {
		unsigned int cc = ((text = element->GetText ()) ? consumerControlCode (text) : 0);
		button.setConsumerControl (cc);
	}
	else if (name == "disabled") {
		button.disable ();
	}
	else {
		Log::warning () << "Disabling button with invalid tag name " << element->Name () << std::endl;
		button.disable ();
	}
}

static
Setting readSetting (const XMLElement *element, const SettingDesc &desc)
{
	if (desc.isComposed ()) {
		ComposedSetting settings;

		const XMLElement *child = element->FirstChildElement ();
		while (child) {
			std::string name = child->Name ();
			auto it = desc.find (name);
			if (it == desc.end ()) {
				Log::warning () << "Ignoring invalid sub-setting: "
						<< name << std::endl;
			}
			else {
				settings.emplace (name, readSetting (child, it->second));
			}
			child = child->NextSiblingElement ();
		}
		return settings;
	}
	else {
		const char * text = element->GetText ();
		return desc.convertFromString (text ? text : std::string ());
	}
}

void ProfileXML::read (const XMLNode *node, Profile &profile, ProfileDirectory::Entry &entry, std::vector<Macro> &macros)
{
	const XMLElement *element = node->FirstChildElement ();
	while (element) {
		std::string name = element->Name ();
		if (name == "modes") {
			const XMLElement *mode_el = element->FirstChildElement ("mode");
			while (mode_el) {
				profile.modes.emplace_back ();
				auto &current_mode = profile.modes.back ();

				const XMLElement *setting = mode_el->FirstChildElement ();
				while (setting) {
					std::string sname = setting->Name ();
					auto it = _mode_settings.find (sname);
					if (it == _mode_settings.end ()) {
						Log::warning () << "Ignoring invalid mode setting: "
								<< sname << std::endl;
					}
					else {
						current_mode.emplace (sname, readSetting (setting, it->second));
					}
					setting = setting->NextSiblingElement ();
				}
				mode_el = mode_el->NextSiblingElement ("mode");
			}
		}
		else if (name == "buttons") {
			const XMLElement *button = element->FirstChildElement ();
			profile.buttons.clear ();
			macros.clear ();
			while (button) {
				profile.buttons.emplace_back ();
				macros.emplace_back ();
				readButton (button, profile.buttons.back (), macros.back (),  _special_actions);
				button = button->NextSiblingElement ();
			}
		}
		else {
			auto it = _entry_settings.find (name);
			if (it != _entry_settings.end ()) {
				entry.settings.emplace (name, readSetting (element, it->second));
			}
			else if ((it = _profile_settings.find (name)) != _profile_settings.end ()) {
				profile.settings.emplace (name, readSetting (element, it->second));
			}
			else {
				Log::warning () << "Ignoring invalid setting: "
						<< name << std::endl;
			}
		}
		element = element->NextSiblingElement ();
	}
}
