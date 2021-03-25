#pragma once

#include "../../types.h"
#include <SDL.h>

#include "all.h"
#include "./axis_direction.h"
#include "./controller_buttons.h"

#ifndef HAS_DPAD
#define HAS_DPAD 0
#endif

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
#include "controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

struct ControllerButtonEvent {
	ControllerButton button;
	bool up;
};

class Controller {
public:
	// Raw axis values.
	float leftStickX, leftStickY, rightStickX, rightStickY;
	// Axis values scaled to [-1, 1] range and clamped to a deadzone.
	float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;
	// Whether stick positions have been updated and need rescaling.
	bool leftStickNeedsScaling, rightStickNeedsScaling;

	// Returns direction of the left thumb stick or DPad (if allow_dpad = true).
	AxisDirection GetLeftStickOrDpadDirection(bool allow_dpad = true);

	// Normalize joystick values
	void ScaleJoysticks();

	// NOTE: Not idempotent because of how it handles axis triggers.
	// Must be called exactly once for each SDL input event.
	ControllerButton ToControllerButton(const SDL_Event &event) const;

private:
	void ScaleJoystickAxes(float *x, float *y);
};

// NOTE: Not idempotent because of how it handles axis triggers.
// Must be called exactly once per SDL input event.
ControllerButtonEvent ToControllerButtonEvent(const SDL_Event &event);

bool IsControllerButtonPressed(ControllerButton button);

bool HandleControllerAddedOrRemovedEvent(const SDL_Event &event);

DEVILUTION_END_NAMESPACE

#endif
