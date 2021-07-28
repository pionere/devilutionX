#include "controller_motion.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1

#include <cmath>

#include "controls/controller.h"
#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"
#include "controls/devices/kbcontroller.h"
#include "controls/game_controls.h"

DEVILUTION_BEGIN_NAMESPACE

static void ScaleJoystickAxes(float *x, float *y)
{
	//radial and scaled dead-zone
	//http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
	//input values go from -32767.0...+32767.0, output values are from -1.0 to 1.0;

	const float DEADZONE = 0.07f;
	/*if (DEADZONE == 0) {
		return;
	}
	if (DEADZONE >= 1.0) {
		*x = 0;
		*y = 0;
		return;
	}*/

	const float maximum = 32767.0f;
	float analogX = *x;
	float analogY = *y;
	float deadZone = DEADZONE * maximum;

	float magnitude = std::sqrt(analogX * analogX + analogY * analogY);
	if (magnitude >= deadZone) {
		// find scaled axis values with magnitudes between zero and maximum
		float scalingFactor = (magnitude - deadZone) / (maximum - deadZone) / magnitude;
		analogX = (analogX * scalingFactor);
		analogY = (analogY * scalingFactor);

		// clamp to ensure results will never exceed the max_axis value
		float clampingFactor = 1.0f;
		float absAnalogX = std::fabs(analogX);
		float absAnalogY = std::fabs(analogY);
		if (absAnalogX > 1.0f || absAnalogY > 1.0f) {
			if (absAnalogX < absAnalogY)
				absAnalogX = absAnalogY;
			clampingFactor /= absAnalogX;
		}
		*x = (clampingFactor * analogX);
		*y = (clampingFactor * analogY);
	} else {
		*x = 0;
		*y = 0;
	}
}

// SELECT + D-Pad to simulate right stick movement.
static bool SimulateRightStickWithDpad(ControllerButtonEvent ctrlEvent)
{
	if (dpad_hotkeys)
		return false;
	static bool simulating = false;
	if (ctrlEvent.button == ControllerButton_BUTTON_BACK) {
		if (ctrlEvent.up && simulating) {
			rightStickX = rightStickY = 0;
			simulating = false;
		}
		return false;
	}
	if (!IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
		return false;
	switch (ctrlEvent.button) {
	case ControllerButton_BUTTON_DPAD_LEFT:
		rightStickX = ctrlEvent.up ? 0.0f : -1.0f;
		break;
	case ControllerButton_BUTTON_DPAD_RIGHT:
		rightStickX = ctrlEvent.up ? 0.0f : 1.0f;
		break;
	case ControllerButton_BUTTON_DPAD_UP:
		rightStickY = ctrlEvent.up ? 0.0f : 1.0f;
		break;
	case ControllerButton_BUTTON_DPAD_DOWN:
		rightStickY = ctrlEvent.up ? 0.0f : -1.0f;
		break;
	default:
		return false;
	}
	simulating = !(rightStickX == 0 && rightStickY == 0);

	return true;
}

float leftStickX, leftStickY, rightStickX, rightStickY;
float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;
bool leftStickNeedsScaling, rightStickNeedsScaling;

static void ScaleJoysticks()
{
	if (leftStickNeedsScaling) {
		leftStickX = (float)leftStickXUnscaled;
		leftStickY = (float)leftStickYUnscaled;
		ScaleJoystickAxes(&leftStickX, &leftStickY);
		leftStickNeedsScaling = false;
	}

	if (rightStickNeedsScaling) {
		rightStickX = (float)rightStickXUnscaled;
		rightStickY = (float)rightStickYUnscaled;
		ScaleJoystickAxes(&rightStickX, &rightStickY);
		rightStickNeedsScaling = false;
	}
}

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrlEvent)
{
#if HAS_GAMECTRL == 1
	GameController *const controller = GameController::Get(event);
	if (controller != NULL && GameController::ProcessAxisMotion(event)) {
		ScaleJoysticks();
		return true;
	}
#endif
#if HAS_JOYSTICK == 1
	Joystick *const joystick = Joystick::Get(event);
	if (joystick != NULL && Joystick::ProcessAxisMotion(event)) {
		ScaleJoysticks();
		return true;
	}
#endif
#if HAS_KBCTRL == 1
	if (ProcessKbCtrlAxisMotion(event))
		return true;
#endif
#if HAS_DPAD == 1
	if (SimulateRightStickWithDpad(ctrlEvent))
		return true;
#endif
	return false;
}

AxisDirection GetLeftStickOrDpadDirection(bool allowDpad)
{
	const float stickX = leftStickX;
	const float stickY = leftStickY;

	AxisDirection result { AxisDirectionX_NONE, AxisDirectionY_NONE };

	if (stickY >= 0.5 || (allowDpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_UP))) {
		result.y = AxisDirectionY_UP;
	} else if (stickY <= -0.5 || (allowDpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_DOWN))) {
		result.y = AxisDirectionY_DOWN;
	}

	if (stickX <= -0.5 || (allowDpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_LEFT))) {
		result.x = AxisDirectionX_LEFT;
	} else if (stickX >= 0.5 || (allowDpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_RIGHT))) {
		result.x = AxisDirectionX_RIGHT;
	}

	return result;
}

DEVILUTION_END_NAMESPACE

#endif
