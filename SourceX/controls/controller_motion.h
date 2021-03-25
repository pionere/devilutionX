#pragma once

// Processes and stores mouse and joystick motion.

#include <SDL.h>

#include "./controller.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1

DEVILUTION_BEGIN_NAMESPACE

extern class Controller controller;

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrl_event);

DEVILUTION_END_NAMESPACE

#endif
