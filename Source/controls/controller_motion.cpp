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

static const direction FaceDir[3][3] = {
	// NONE      UP      DOWN
	{ DIR_NONE, DIR_N, DIR_S }, // NONE
	{ DIR_W, DIR_NW, DIR_SW },  // LEFT
	{ DIR_E, DIR_NE, DIR_SE },  // RIGHT
};

int GetLeftStickOrDpadDirection(bool allowDpad)
{
	float stickX = leftStickX;
	float stickY = leftStickY;

	int dx = 0, dy = 0;

	if (allowDpad && !IsControllerButtonPressed(ControllerButton_BUTTON_START)) {
		if (IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_UP))
			stickY = 1.0f;
		else if (IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_DOWN))
			stickY = -1.0f;
		if (IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_LEFT))
			stickX = 1.0f;
		else if (IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_RIGHT))
			stickX = -1.0f;
	}

	if (stickY >= 0.5) {
		dy = 1;
	} else if (stickY <= -0.5) {
		dy = 2;
	}

	if (stickX <= -0.5) {
		dx = 1;
	} else if (stickX >= 0.5) {
		dx = 2;
	}

	return FaceDir[dx][dy];
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
