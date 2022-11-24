#pragma once

#include "../types.h"

#if HAS_TOUCHPAD
#ifdef USE_SDL1
static_assert(FALSE, "Touchpad is not supported in SDL1.");
#endif
#include <SDL.h>

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void handle_touch(SDL_Event* event);
void finish_simulated_mouse_clicks();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_TOUCHPAD
