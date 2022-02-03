#pragma once

#include "../types.h"
#include <SDL.h>

#include "devices/kbcontroller.h"
#include "devices/joystick.h"
#include "devices/game_controller.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
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
