#pragma once

#include <SDL.h>
#include "controls/controller_buttons.h"

#ifndef USE_SDL1
DEVILUTION_BEGIN_NAMESPACE

ControllerButton GameControllerToControllerButton(const SDL_Event &event);

bool IsGameControllerButtonPressed(ControllerButton button);

bool ProcessGameControllerAxisMotion(const SDL_Event &event);

SDL_GameController *CurrentGameController();

// Must be called after InitJoystick().
void InitGameController();

DEVILUTION_END_NAMESPACE
#endif
