#pragma once
// Unifies joystick, gamepad, and keyboard controller APIs.

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

// NOTE: FACE-buttons on a VITA controller:     X O [] /\
//                    on a XBox 360 controller: A, B, X, Y
//                    on a Nintendo controller: B, A, Y, X
typedef enum ControllerButton {
	ControllerButton_NONE = 0,
	ControllerButton_IGNORE,
	ControllerButton_AXIS_TRIGGERLEFT,   // ZL (aka L2)
	ControllerButton_AXIS_TRIGGERRIGHT,  // ZR (aka R2)
	ControllerButton_BUTTON_FACE_BOTTOM, // SDL_CONTROLLER_BUTTON_A
	ControllerButton_BUTTON_FACE_RIGHT,  // SDL_CONTROLLER_BUTTON_B
	ControllerButton_BUTTON_FACE_LEFT,   // SDL_CONTROLLER_BUTTON_X
	ControllerButton_BUTTON_FACE_TOP,    // SDL_CONTROLLER_BUTTON_Y
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
