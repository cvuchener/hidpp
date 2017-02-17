#include "ProfileFormatCommon.h"

#include <hidpp/Field.h>

using namespace HIDPP;
using namespace HIDPP10;

namespace ButtonFields
{
static constexpr auto Type =		Field<uint8_t> (0);
static constexpr auto MouseButtons =	Field<uint16_t, LittleEndian> (1);
static constexpr auto Modifiers =	Field<uint8_t> (1);
static constexpr auto Key =		Field<uint8_t> (2);
static constexpr auto Special =		Field<uint16_t, LittleEndian> (1);
static constexpr auto ConsumerControl =	Field<uint16_t, LittleEndian> (1);
static constexpr auto MacroPage =	Field<uint8_t> (0);
static constexpr auto MacroOffset =	Field<uint8_t> (1);
}

enum ButtonType: uint8_t
{
	ButtonMouse = 0x81,
	ButtonKey = 0x82,
	ButtonSpecial = 0x83,
	ButtonConsumerControl = 0x84,
	ButtonDisabled = 0x8f,
};

Profile::Button HIDPP10::parseButton (std::vector<uint8_t>::const_iterator begin)
{
	using namespace ButtonFields;
	switch (Type.read (begin)) {
	case ButtonMouse:
		return Profile::Button (Profile::Button::MouseButtonsType (),
					MouseButtons.read (begin));
	case ButtonKey:
		return Profile::Button (Modifiers.read (begin),
					Key.read (begin));
	case ButtonSpecial:
		return Profile::Button (Profile::Button::SpecialType (),
					Special.read (begin));
	case ButtonConsumerControl:
		return Profile::Button (Profile::Button::ConsumerControlType (),
					ConsumerControl.read (begin));
	case ButtonDisabled:
		return Profile::Button ();
	default:
		return Profile::Button (Address {
			0,
			MacroPage.read (begin),
			MacroOffset.read (begin)
		});
	}
}

void HIDPP10::writeButton (std::vector<uint8_t>::iterator begin, const Profile::Button &button)
{
	using namespace ButtonFields;
	std::fill (begin, begin+ButtonSize, 0);
	switch (button.type ()) {
	case Profile::Button::Type::Disabled:
		Type.write (begin, ButtonDisabled);
		break;
	case Profile::Button::Type::MouseButtons:
		Type.write (begin, ButtonMouse);
		MouseButtons.write (begin, button.mouseButtons ());
		break;
	case Profile::Button::Type::Key:
		Type.write (begin, ButtonKey);
		Modifiers.write (begin, button.modifierKeys ());
		Key.write (begin, button.key ());
		break;
	case Profile::Button::Type::ConsumerControl:
		Type.write (begin, ButtonConsumerControl);
		ConsumerControl.write (begin, button.consumerControl ());
		break;
	case Profile::Button::Type::Special:
		Type.write (begin, ButtonSpecial);
		Special.write (begin, button.special ());
		break;
	case Profile::Button::Type::Macro: {
		Address addr = button.macro ();
		MacroPage.write (begin, addr.page);
		MacroOffset.write (begin, addr.offset);
		break;
	}
	}
}

