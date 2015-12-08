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

#include "MacroText.h"

#include <sstream>
#include <map>
#include <regex>

#include <misc/Log.h>

using HIDPP10::Macro;

std::string macroToText (Macro::const_iterator begin, Macro::const_iterator end)
{
	Macro::const_iterator current;

	unsigned int next_label = 0;
	std::map<const Macro::Item *, std::string> labels;

	for (current = begin; current != end; ++current) {
		if (current->isJump ()) {
			Macro::const_iterator dest = current->jumpDestination ();
			if (labels.find (&(*dest)) != labels.end ())
				continue;
			std::stringstream ss;
			ss << "label" << next_label++;
			labels.insert ({&(*dest), ss.str ()});
		}
	}

	std::stringstream ss;

	for (current = begin; current != end; ++current) {
		auto it = labels.find (&(*current));
		if (it != labels.end ()) {
			ss << it->second << ":" << std::endl;
		}

		if (current->isShortDelay ()) {
			ss << "ShortDelay " << current->delay () << ";" << std::endl;
			continue;
		}

		switch (current->opCode ()) {
		case Macro::Item::NoOp:
			ss << "NoOp;" << std::endl;
			break;

		case Macro::Item::WaitRelease:
			ss << "WaitRelease;" << std::endl;
			break;

		case Macro::Item::RepeatUntilRelease:
			ss << "RepeatUntilRelease;" << std::endl;
			break;

		case Macro::Item::RepeatForever:
			ss << "Repeat;" << std::endl;
			break;

		case Macro::Item::KeyPress:
			ss << "KeyPress " << (unsigned int) current->keyCode () << ";" << std::endl;
			break;

		case Macro::Item::KeyRelease:
			ss << "KeyRelease " << (unsigned int) current->keyCode () << ";" << std::endl;
			break;

		case Macro::Item::ModifierPress:
			ss << "ModifierPress " << (unsigned int) current->modifiers () << ";" << std::endl;
			break;

		case Macro::Item::ModifierRelease:
			ss << "ModifierRelease " << (unsigned int) current->modifiers () << ";" << std::endl;
			break;

		case Macro::Item::MouseWheel:
			ss << "MouseWheel " << (int) current->wheel () << ";" << std::endl;
			break;

		case Macro::Item::MouseButtonPress:
			ss << "MouseButtonPress " << current->buttons () << ";" << std::endl;
			break;

		case Macro::Item::MouseButtonRelease:
			ss << "MouseButtonRelease " << current->buttons () << ";" << std::endl;
			break;

		case Macro::Item::ConsumerControl:
			ss << "ConsumerControl " << current->consumerControl () << ";" << std::endl;
			break;

		case Macro::Item::Delay:
			ss << "Delay " << current->delay () << ";" << std::endl;
			break;

		case Macro::Item::Jump:
			ss << "Jump " << labels[&(*current->jumpDestination ())] << ";" << std::endl;
			break;

		case Macro::Item::JumpIfPressed:
			ss << "JumpIfPressed " << labels[&(*current->jumpDestination ())] << ";" << std::endl;
			break;

		case Macro::Item::MousePointer:
			ss << "MousePointer " << current->mouseX () << " " << current->mouseY () << ";" << std::endl;
			break;

		case Macro::Item::JumpIfReleased:
			ss << "JumpIfReleased " << current->delay () << " " << labels[&(*current->jumpDestination ())] << ";" << std::endl;
			break;

		case Macro::Item::End:
			ss << "End;" << std::endl;
			break;
		}	
	}

	return ss.str ();
}

Macro textToMacro (const std::string &text)
{
	static const std::regex InstructionRegex ("(?:\\s*(\\w+):)?\\s*(\\w+)(?:\\s+([^;]+))?\\s*(?:;|$)");
	static const std::regex ParamRegex ("\\S+");

	std::map<std::string, Macro::iterator> labels;
	std::vector<std::pair<Macro::Item *, std::string>> jumps;

	Macro macro;

	std::string::const_iterator current = text.begin ();
	std::smatch results;
	while (std::regex_search (current, text.end (),
				  results, InstructionRegex,
				  std::regex_constants::match_continuous)) {
		std::string label = results.str (1);
		std::string instruction = results.str (2);
		std::vector<std::string> params;

		std::smatch param_res;
		std::string::const_iterator current_param = results[3].first;
		while (std::regex_search (current_param, results[3].second, param_res, ParamRegex)) {
			params.push_back (param_res.str (0));
			current_param = param_res[0].second;
		}

		bool press;
		if (instruction == "NoOp") {
			macro.emplace_back (Macro::Item::NoOp);
		}
		else if (instruction == "WaitRelease") {
			macro.emplace_back (Macro::Item::WaitRelease);
		}
		else if (instruction == "RepeatUntilRelease") {
			macro.emplace_back (Macro::Item::RepeatUntilRelease);
		}
		else if (instruction == "Repeat") {
			macro.emplace_back (Macro::Item::RepeatForever);
		}
		else if ((press = (instruction == "KeyPress")) || instruction == "KeyRelease") {
			if (press)
				macro.emplace_back (Macro::Item::KeyPress);
			else
				macro.emplace_back (Macro::Item::KeyRelease);
			unsigned int code = std::stoul (params[0], nullptr, 0);
			if (code > 255)
				Log::warning () << "Key code " << code << " is too big." << std::endl;
			macro.back ().setKeyCode (static_cast<uint8_t> (code));
		}
		else if ((press = (instruction == "ModifierPress")) || instruction == "ModifierRelease") {
			if (press)
				macro.emplace_back (Macro::Item::ModifierPress);
			else
				macro.emplace_back (Macro::Item::ModifierRelease);
			unsigned int bits = std::stoul (params[0], nullptr, 0);
			if  (bits > 255)
				Log::warning () << "Invalid flags for modifiers." << std::endl;
			macro.back ().setModifiers (static_cast<uint8_t> (bits));
		}
		else if (instruction == "MouseWheel") {
			macro.emplace_back (Macro::Item::MouseWheel);
			int wheel = std::stoi (params[0]);
			macro.back ().setWheel (wheel);
		}
		else if ((press = (instruction == "MouseButtonPress")) || instruction == "MouseButtonRelease") {
			if (press)
				macro.emplace_back (Macro::Item::MouseButtonPress);
			else
				macro.emplace_back (Macro::Item::MouseButtonRelease);
			unsigned int bits = std::stoul (params[0], nullptr, 0);
			if (bits > 65535)
				Log::warning () << "Invalid mouse bits." << std::endl;
			macro.back ().setButtons (bits);
		}
		else if (instruction == "ConsumerControl") {
			macro.emplace_back (Macro::Item::ConsumerControl);
			unsigned int code = std::stoul (params[0], nullptr, 0);
			macro.back ().setConsumerControl (code);
		}
		else if (instruction == "Delay") {
			macro.emplace_back (Macro::Item::Delay);
			unsigned int delay = std::stoul (params[0]);
			macro.back ().setDelay (delay);
		}
		else if (instruction == "Jump") {
			macro.emplace_back (Macro::Item::Jump);
			jumps.emplace_back (&macro.back (), params[0]);
		}
		else if (instruction == "JumpIfPressed") {
			macro.emplace_back (Macro::Item::JumpIfPressed);
			jumps.emplace_back (&macro.back (), params[0]);
		}
		else if (instruction == "MousePointer") {
			macro.emplace_back (Macro::Item::MousePointer);
			int x = std::stoi (params[0]);
			int y = std::stoi (params[1]);
			macro.back ().setMouseX (x);
			macro.back ().setMouseY (y);
		}
		else if (instruction == "JumpIfReleased") {
			macro.emplace_back (Macro::Item::JumpIfReleased);
			unsigned int delay = std::stoul (params[0]);
			macro.back ().setDelay (delay);
			jumps.emplace_back (&macro.back (), params[1]);
		}
		else if (instruction == "End") {
			macro.emplace_back (Macro::Item::End);
		}
		else if (instruction == "ShortDelay") {
			unsigned int delay = std::stoul (params[0]);
			macro.emplace_back (Macro::Item::getShortDelayCode (delay));
		}
		else {
			Log::error () << "Unknown instruction " << instruction << std::endl;
			return Macro ();
		}

		if (!label.empty ()) {
			labels.emplace (label, std::prev (macro.end ()));
		}

		current = results[0].second;
	}

	for (auto pair: jumps) {
		Macro::Item *item = pair.first;
		const std::string &label = pair.second;
		auto it = labels.find (label);
		if (it == labels.end ()) {
			Log::error () << "Unknown label " << label << std::endl;
			return Macro ();
		}
		item->setJumpDestination (it->second);
	}
	
	return macro;
}
