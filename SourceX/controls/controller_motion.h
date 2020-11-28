#pragma once

// Processes and stores mouse and joystick motion.

#include "all.h"
#include "controls/controller.h"

DEVILUTION_BEGIN_NAMESPACE

// Raw axis values. (range: -32768 to 32767)
extern int leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;

// Axis values scaled to [-1, 1] range and clamped to a deadzone.
extern float leftStickX, leftStickY, rightStickX, rightStickY;

// Whether stick positions have been updated and need rescaling.
extern bool leftStickNeedsScaling, rightStickNeedsScaling;

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrl_event);

DEVILUTION_END_NAMESPACE
