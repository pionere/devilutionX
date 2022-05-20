#pragma once
// Unifies joystick, gamepad, and keyboard controller APIs.

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

// NOTE: A, B, X, Y refer to physical positions on an XBox 360 controller.
// A<->B and X<->Y are reversed on a Nintendo controller.
typedef enum ControllerButton {
	ControllerButton_NONE = 0,
	ControllerButton_IGNORE,
	ControllerButton_AXIS_TRIGGERLEFT,  // ZL (aka L2)
	ControllerButton_AXIS_TRIGGERRIGHT, // ZR (aka R2)
	ControllerButton_BUTTON_A,          // Bottom button
	ControllerButton_BUTTON_B,          // Right button
	ControllerButton_BUTTON_X,          // Left button
	ControllerButton_BUTTON_Y,          // TOP button
	ControllerButton_BUTTON_LEFTSTICK,
	ControllerButton_BUTTON_RIGHTSTICK,
	ControllerButton_BUTTON_LEFTSHOULDER,
	ControllerButton_BUTTON_RIGHTSHOULDER,
	ControllerButton_BUTTON_START,
	ControllerButton_BUTTON_BACK,
	ControllerButton_BUTTON_DPAD_UP,
	ControllerButton_BUTTON_DPAD_DOWN,
	ControllerButton_BUTTON_DPAD_LEFT,
	ControllerButton_BUTTON_DPAD_RIGHT
} ControllerButton;

inline bool IsDPadButton(ControllerButton button)
{
	return button == ControllerButton_BUTTON_DPAD_UP
	    || button == ControllerButton_BUTTON_DPAD_DOWN
	    || button == ControllerButton_BUTTON_DPAD_LEFT
	    || button == ControllerButton_BUTTON_DPAD_RIGHT;
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
