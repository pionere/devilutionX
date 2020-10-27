#pragma once

#include <SDL.h>
#include "../../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifndef USE_SDL1
void handle_touch(SDL_Event *event, int current_mouse_x, int current_mouse_y);
#endif

DEVILUTION_END_NAMESPACE
