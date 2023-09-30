#include "controller_motion.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include <cmath>

#include "controller.h"
#include "devices/game_controller.h"
#include "devices/joystick.h"
#include "devices/kbcontroller.h"

DEVILUTION_BEGIN_NAMESPACE

float leftStickX, leftStickY, rightStickX, rightStickY;
float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;

bool IsMovingMouseCursorWithController()
{
	return rightStickX != 0 || rightStickY != 0;
}

void ScaleJoystickAxes(bool rightAxes)
{
	float *x, *y;

	if (rightAxes) {
		rightStickX = rightStickXUnscaled;
		rightStickY = rightStickYUnscaled;
		x = &rightStickX;
		y = &rightStickY;
	} else {
		leftStickX = leftStickXUnscaled;
		leftStickY = leftStickYUnscaled;
		x = &leftStickX;
		y = &leftStickY;
	}

	// radial and scaled dead-zone
	// https://web.archive.org/web/20200130014626/www.third-helix.com:80/2013/04/12/doing-thumbstick-dead-zones-right.html
	// input values go from -32767.0...+32767.0, output values are from -1.0 to 1.0;

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
bool SimulateRightStickWithDpad(const ControllerButtonEvent& ctrlEvent)
{
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

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event& event)
{
#if HAS_GAMECTRL
	if (GameController::ProcessAxisMotion(event))
		return true;
#endif
#if HAS_JOYSTICK
	if (Joystick::ProcessAxisMotion(event))
		return true;
#endif
#if HAS_KBCTRL
	if (ProcessKbCtrlAxisMotion(event))
		return true;
#endif
	return false;
}

AxisDirection GetLeftStickOrDpadDirection(bool allowDpad)
{
	const float stickX = leftStickX;
	const float stickY = leftStickY;

	AxisDirection result { AxisDirectionX_NONE, AxisDirectionY_NONE };

	allowDpad = allowDpad && !IsControllerButtonPressed(ControllerButton_BUTTON_START);

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
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
