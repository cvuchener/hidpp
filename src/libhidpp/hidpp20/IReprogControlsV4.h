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

#ifndef LIBHIDPP_HIDPP20_IREPROGCONTROLSV4_H
#define LIBHIDPP_HIDPP20_IREPROGCONTROLSV4_H

#include <hidpp20/FeatureInterface.h>

#include <vector>

namespace HIDPP20
{

/**
 * Interface for HW remapping controls or diverting the button or XY events for software handling.
 */
class IReprogControlsV4: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x1b04;

	enum Function {
		GetControlCount = 0,
		GetControlInfo = 1,
		GetControlReporting = 2,
		SetControlReporting = 3,
	};

	enum Event {
		DivertedButtonEvent = 0,
		DivertedRawXYEvent = 1,
	};

	IReprogControlsV4 (Device *dev);

	/**
	 * \return the count of control on this device.
	 */
	unsigned int getControlCount ();

	struct ControlInfo
	{
		uint16_t control_id;
		uint16_t task_id;
		uint8_t flags;		///< Control info flags. \see ControlInfoFlags
		uint8_t pos;		///< Position for F keys, 0 for other controls.
		uint8_t group;		///< Group of this control ID.
		uint8_t group_mask;	///< The control may be remapped to control IDs from groups in this mask.
		uint8_t additional_flags;	///< More control info flags. \see ControlInfoAdditionalFlags
	};
	enum ControlInfoFlags: uint8_t {
		MouseButton = 1<<0,	///< The control is a mouse button.
		FKey = 1<<1,		///< The control is a F key.
		HotKey = 1<<2,		///< The control is a non-standard, non-F key.
		FnToggle = 1<<3,	///< The control is toggled by the fn key.
		ReprogHint = 1<<4,	///< The software can reprogram the key. If not set it should only use it as described by the task ID.
		TemporaryDivertable = 1<<5,	///< The control can be diverted temporarily.
		PersistentDivertable = 1<<6,	///< The control can be diverted persistently.
		Virtual = 1<<7,		///< The control is not a physical control, it is a function for remapping others.
	};
	enum ControlInfoAdditionalFlags: uint8_t {
		RawXY = 1<<0,	///< Raw XY data can be diverted.
	};

	/**
	 * Retrieve control \p index informations.
	 */
	ControlInfo getControlInfo (unsigned int index);

	enum ControlReportingFlags: uint8_t {
		TemporaryDiverted = 1<<0,
		ChangeTemporaryDivert = 1<<1,
		PersistentDiverted = 1<<2,
		ChangePersistentDivert = 1<<3,
		RawXYDiverted = 1<<4,
		ChangeRawXYDivert = 1<<5,
	};
	/**
	 * Get the current reporting settings for the control given by its control ID.
	 *
	 * \param[in]	control_id	Control ID.
	 * \param[out]	flags		Diverted status of the control.
	 *				Only TemporaryDiverted, PersistentDiverted and
	 *				RawXYDiverted may be set.
	 *
	 * \returns the remapped control ID. 0 if the control is remapped.
	 *
	 * \see ControlReportingFlags.
	 */
	uint16_t getControlReporting (uint16_t control_id, uint8_t &flags);

	/**
	 * Set the current reporting settings for the control given by its control ID.
	 *
	 * \param[in]	control_id	Control ID.
	 * \param[in]	flags		Diverted status of the control. The "Change" flags
	 *				must be set for actually changing the corresponding
	 *				settings.
	 * \param[in]	remap		Remapped control ID. 0 for no change.
	 *
	 * \see ControlReportingFlags.
	 */
	void setControlReporting (uint16_t control_id, uint8_t flags, uint16_t remap);

	static std::vector<uint16_t> divertedButtonEvent (const HIDPP::Report &event);

	struct Move
	{
		int16_t x, y;
	};
	static Move divertedRawXYEvent (const HIDPP::Report &event);
};

}

#endif

