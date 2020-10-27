#pragma once

// Joystick mappings for SDL1 and additional buttons on SDL2.

#include <SDL.h>
#include "controls/controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

ControllerButton JoyButtonToControllerButton(const SDL_Event &event);

bool IsJoystickButtonPressed(ControllerButton button);

bool ProcessJoystickAxisMotion(const SDL_Event &event);

SDL_Joystick *CurrentJoystick();
int CurrentJoystickIndex();

void InitJoystick();

DEVILUTION_END_NAMESPACE
