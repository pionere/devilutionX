#pragma once

// Processes and stores mouse and joystick motion.

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include "./axis_direction.h"
#include "./controller.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

// Raw axis values.
extern float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;

// Axis values scaled to [-1, 1] range and clamped to a deadzone.
extern float leftStickX, leftStickY, rightStickX, rightStickY;

// Whether the mouse cursor is being moved with the controller.
bool IsMovingMouseCursorWithController();

// update left/rightStickX/Y values from the *Unscaled values
void ScaleJoystickAxes(bool rightAxes);

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event& event);

// Updates motion state for dpads.
bool SimulateRightStickWithDpad(const ControllerButtonEvent& ctrlEvent);

// Returns direction of the left thumb stick or DPad (if allow_dpad = true).
AxisDirection GetLeftStickOrDpadDirection(bool allow_dpad);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
