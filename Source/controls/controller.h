#pragma once

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include <SDL.h>

#include "../defs.h"

#include "devices/kbcontroller.h"
#include "devices/joystick.h"
#include "devices/game_controller.h"

#include "controller_buttons.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool sgbControllerActive;

struct ControllerButtonEvent {
	ControllerButton button;
	bool up;
};

// NOTE: Not idempotent because of how it handles axis triggers.
// Must be called exactly once per SDL input event.
ControllerButtonEvent ToControllerButtonEvent(const SDL_Event& event);

bool IsControllerButtonPressed(ControllerButton button);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
