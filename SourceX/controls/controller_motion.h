#pragma once

// Processes and stores mouse and joystick motion.

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

// Raw axis values.
extern float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;

// Axis values scaled to [-1, 1] range and clamped to a deadzone.
extern float leftStickX, leftStickY, rightStickX, rightStickY;

// Whether stick positions have been updated and need rescaling.
extern bool leftStickNeedsScaling, rightStickNeedsScaling;

void ScaleJoysticks();

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event);

DEVILUTION_END_NAMESPACE
