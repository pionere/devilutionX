#pragma once

#include "../../types.h"

// Keyboard keys acting like gamepad buttons
#ifndef HAS_KBCTRL
#define HAS_KBCTRL 0
#endif

#if HAS_KBCTRL == 1
#include <SDL.h>
#include "../controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

ControllerButton KbCtrlToControllerButton(const SDL_Event &event);

bool IsKbCtrlButtonPressed(ControllerButton button);

bool ProcessKbCtrlAxisMotion(const SDL_Event &event);

DEVILUTION_END_NAMESPACE
#endif
