#pragma once

#if DEBUG_MODE
#include <SDL.h>
#define DoLog SDL_Log
#else
#define DoLog(x, ...) ((void)0)
#endif
