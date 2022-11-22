#pragma once

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#endif

namespace dvl {

typedef struct event_emul {
	SDL_mutex* mutex;
	SDL_cond* cond;
} event_emul;

void StartEvent(event_emul& ev);
void EndEvent(event_emul& ev);
void SetEvent(event_emul& ev);
void ResetEvent(event_emul& ev);
void WaitForEvent(event_emul& ev);
SDL_Thread* CreateThread(SDL_ThreadFunction handler);

} // namespace dvl
