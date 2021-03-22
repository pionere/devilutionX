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

static void ScaleJoystickAxes(float *x, float *y)
{
	//radial and scaled dead_zone
	//http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
	//input values go from -32767.0...+32767.0, output values are from -1.0 to 1.0;

	const float deadzone = 0.07f;
	/*if (deadzone == 0) {
		return;
	}
	if (deadzone >= 1.0) {
		*x = 0;
		*y = 0;
		return;
	}*/

	const float maximum = 32767.0f;
	float analog_x = *x;
	float analog_y = *y;
	float dead_zone = deadzone * maximum;

	float magnitude = std::sqrt(analog_x * analog_x + analog_y * analog_y);
	if (magnitude >= dead_zone) {
		// find scaled axis values with magnitudes between zero and maximum
		float scalingFactor = (magnitude - dead_zone) / (maximum - dead_zone) / magnitude;
		analog_x = (analog_x * scalingFactor);
		analog_y = (analog_y * scalingFactor);

		// clamp to ensure results will never exceed the max_axis value
		float clamping_factor = 1.0f;
		float abs_analog_x = std::fabs(analog_x);
		float abs_analog_y = std::fabs(analog_y);
		if (abs_analog_x > 1.0f || abs_analog_y > 1.0f) {
			if (abs_analog_x < abs_analog_y)
				abs_analog_x = abs_analog_y;
			clamping_factor /= abs_analog_x;
		}
		*x = (clamping_factor * analog_x);
		*y = (clamping_factor * analog_y);
	} else {
		*x = 0;
		*y = 0;
	}
}

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

static void ScaleJoysticks()
{
	if (controller.leftStickNeedsScaling) {
		controller.leftStickX = (float)controller.leftStickXUnscaled;
		controller.leftStickY = (float)controller.leftStickYUnscaled;
		ScaleJoystickAxes(&controller.leftStickX, &controller.leftStickY);
		controller.leftStickNeedsScaling = false;
	}

	if (controller.rightStickNeedsScaling) {
		controller.rightStickX = (float)controller.rightStickXUnscaled;
		controller.rightStickY = (float)controller.rightStickYUnscaled;
		ScaleJoystickAxes(&controller.rightStickX, &controller.rightStickY);
		controller.rightStickNeedsScaling = false;
	}
}

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrl_event)
{
#if HAS_GAMECTRL == 1
	GameController *const controller = GameController::Get(event);
	if (controller != NULL && controller->ProcessAxisMotion(event)) {
		ScaleJoysticks();
		return true;
	}
#endif
#if HAS_JOYSTICK == 1
	Joystick *const joystick = Joystick::Get(event);
	if (joystick != NULL && joystick->ProcessAxisMotion(event)) {
		ScaleJoysticks();
		return true;
	}
#endif
#if HAS_KBCTRL == 1
	if (ProcessKbCtrlAxisMotion(event))
		return true;
#endif
#if HAS_DPAD == 1
	if (SimulateRightStickWithDpad(event, ctrl_event))
		return true;
#endif
	return false;
}

AxisDirection GetLeftStickOrDpadDirection(bool allow_dpad)
{
	const float stickX = controller.leftStickX;
	const float stickY = controller.leftStickY;

	AxisDirection result { AxisDirectionX_NONE, AxisDirectionY_NONE };

	if (stickY >= 0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_UP))) {
		result.y = AxisDirectionY_UP;
	} else if (stickY <= -0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_DOWN))) {
		result.y = AxisDirectionY_DOWN;
	}

	if (stickX <= -0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_LEFT))) {
		result.x = AxisDirectionX_LEFT;
	} else if (stickX >= 0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_RIGHT))) {
		result.x = AxisDirectionX_RIGHT;
	}

	return result;
}

DEVILUTION_END_NAMESPACE

#endif
