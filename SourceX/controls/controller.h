#pragma once

#include "../../types.h"
#include <SDL.h>

#include "devices/kbcontroller.h"
#include "devices/joystick.h"
#include "devices/game_controller.h"

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

// NOTE: Not idempotent because of how it handles axis triggers.
// Must be called exactly once per SDL input event.
ControllerButtonEvent ToControllerButtonEvent(const SDL_Event &event);

bool IsControllerButtonPressed(ControllerButton button);

bool HandleControllerAddedOrRemovedEvent(const SDL_Event &event);

DEVILUTION_END_NAMESPACE

#endif
