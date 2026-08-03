#include "controller_motion.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include "../all.h"
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
	if (magnitude > deadZone) {
		// find scaled axis values with magnitudes between zero and maximum
		// if (magnitude > maximum) {
		// 	magnitude = maximum;
		// }
		float scalingFactor = (magnitude - deadZone) / (maximum - deadZone) / magnitude;
		analogX *= scalingFactor;
		analogY *= scalingFactor;
#if 0
		// clamp to ensure results will never exceed the max_axis value
		float absAnalogX = std::fabs(analogX);
		float absAnalogY = std::fabs(analogY);
		if (absAnalogX < absAnalogY)
			absAnalogX = absAnalogY;
		if (absAnalogX > 1.0f) {
			float clampingFactor = 1.0f / absAnalogX;
			analogX *= clampingFactor;
			analogY *= clampingFactor;
		}
#endif
		*x = analogX;
		*y = analogY;
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

int GetLeftStickOrDpadDirection(bool allowDpad)
{
	float stickX = leftStickX;
	float stickY = leftStickY;

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

	int res = DIR_NONE;
	constexpr float StickDirectionThreshold = 0.4F;
	if (stickX * stickX + stickY * stickY >= StickDirectionThreshold * StickDirectionThreshold) {
		const int MAX_DIST = -256;
		const int dx = stickX * MAX_DIST;
		const int dy = stickY * MAX_DIST;
		POS32 tpos = { 0, 0 };
		SHIFT_GRID(tpos.x, tpos.y, dx, dy);
		res = GetDirection(0, 0, tpos.x, tpos.y);
	}
	return res;
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
