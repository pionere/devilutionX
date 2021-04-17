#pragma once

#include "../types.h"

#ifndef HAS_TOUCHPAD
#define HAS_TOUCHPAD 0
#endif

#if HAS_TOUCHPAD == 1
#ifdef USE_SDL1
static_assert(FALSE, "Touchpad is not supported in SDL1.");
#endif
#include <SDL.h>

DEVILUTION_BEGIN_NAMESPACE

void handle_touch(SDL_Event *event, int current_mouse_x, int current_mouse_y);
void finish_simulated_mouse_clicks(int current_mouse_x, int current_mouse_y);

DEVILUTION_END_NAMESPACE

#endif
