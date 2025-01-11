#pragma once

// Keyboard keys acting like gamepad buttons
#if HAS_KBCTRL
#include <SDL.h>

#include "../controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

ControllerButton KbCtrlToControllerButton(const SDL_Event& event);

bool IsKbCtrlButtonPressed(ControllerButton button);

bool ProcessKbCtrlAxisMotion(const SDL_Event& event);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_KBCTRL
