#include "controller_motion.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1

#include <cmath>

#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"
#include "controls/devices/kbcontroller.h"
#include "controls/controller.h"
#include "controls/game_controls.h"

DEVILUTION_BEGIN_NAMESPACE

Controller controller;

// SELECT + D-Pad to simulate right stick movement.
static bool SimulateRightStickWithDpad(const SDL_Event &event, ControllerButtonEvent ctrl_event)
{
	if (dpad_hotkeys)
		return false;
	static bool simulating = false;
	if (ctrl_event.button == ControllerButton_BUTTON_BACK) {
		if (ctrl_event.up && simulating) {
			controller.rightStickX = controller.rightStickY = 0;
			simulating = false;
		}
		return false;
	}
	if (!IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
		return false;
	switch (ctrl_event.button) {
	case ControllerButton_BUTTON_DPAD_LEFT:
		controller.rightStickX = ctrl_event.up ? 0.0f : -1.0f;
		break;
	case ControllerButton_BUTTON_DPAD_RIGHT:
		controller.rightStickX = ctrl_event.up ? 0.0f : 1.0f;
		break;
	case ControllerButton_BUTTON_DPAD_UP:
		controller.rightStickY = ctrl_event.up ? 0.0f : 1.0f;
		break;
	case ControllerButton_BUTTON_DPAD_DOWN:
		controller.rightStickY = ctrl_event.up ? 0.0f : -1.0f;
		break;
	default:
		return false;
	}
	simulating = !(controller.rightStickX == 0 && controller.rightStickY == 0);

	return true;
}

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrl_event)
{
#if HAS_GAMECTRL == 1
	GameController *const controller = GameController::Get(event);
	if (controller != NULL && controller->ProcessAxisMotion(event)) {
		controller->ScaleJoysticks();
		return true;
	}
#endif
#if HAS_JOYSTICK == 1
	Joystick *const joystick = Joystick::Get(event);
	if (joystick != NULL && joystick->ProcessAxisMotion(event)) {
		joystick->ScaleJoysticks();
		return true;
	}
#endif
#if HAS_KBCTRL == 1
	KeyboardController *const keyboardController = KeyboardController::Get(event);
	if (keyboardController != NULL && keyboardController->ProcessAxisMotion(event)) {
		keyboardController->ScaleJoysticks();
		return true;
	}
#endif
#if HAS_DPAD == 1
	if (SimulateRightStickWithDpad(event, ctrl_event))
		return true;
#endif
	return false;
}

DEVILUTION_END_NAMESPACE

#endif
