#pragma once

#if HAS_TOUCHPAD
#ifdef USE_SDL1
static_assert(false, "Touchpad is not supported in SDL1.");
#endif
#include <SDL.h>

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void InitTouch();
void handle_touch(SDL_Event* event);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_TOUCHPAD
