#include "thread.h"

#include "../types.h"
#include "appfat.h"
#include <SDL.h>

DEVILUTION_BEGIN_NAMESPACE

SDL_Thread* CreateThread(SDL_ThreadFunction handler)
{
#ifdef USE_SDL1
	SDL_Thread *ret = SDL_CreateThread(handler, NULL);
#else
	SDL_Thread *ret = SDL_CreateThread(handler, NULL, NULL);
#endif
	if (ret == NULL) {
		sdl_error(ERR_SDL_THREAD_CREATE);
	}
	return ret;
}

void StartEvent(event_emul &ev)
{
	ev.mutex = SDL_CreateMutex();
	if (ev.mutex == NULL) {
		sdl_error(ERR_SDL_MUTEX_CREATE);
	}
	ev.cond = SDL_CreateCond();
	if (ev.cond == NULL) {
		sdl_error(ERR_SDL_COND_CREATE);
	}
}

void EndEvent(event_emul &ev)
{
	SDL_DestroyCond(ev.cond);
	ev.cond = NULL;
	SDL_DestroyMutex(ev.mutex);
	ev.mutex = NULL;
}

void SetEvent(event_emul &ev)
{
	if (SDL_LockMutex(ev.mutex) < 0 || SDL_CondSignal(ev.cond) < 0 || SDL_UnlockMutex(ev.mutex) < 0) {
		sdl_error(ERR_SDL_EVENT_SET);
	}
}

void ResetEvent(event_emul &ev)
{
	if (SDL_LockMutex(ev.mutex) < 0 || SDL_CondWaitTimeout(ev.cond, ev.mutex, 0) < 0 || SDL_UnlockMutex(ev.mutex) < 0) {
		sdl_error(ERR_SDL_EVENT_RESET);
	}
}

void WaitForEvent(event_emul &ev)
{
	if (SDL_LockMutex(ev.mutex) < 0) {
		sdl_error(ERR_SDL_EVENT_LOCK);
	}
	if (SDL_CondWait(ev.cond, ev.mutex) < 0 || SDL_CondSignal(ev.cond) < 0 || SDL_UnlockMutex(ev.mutex) < 0) {
		sdl_error(ERR_SDL_EVENT_WAIT);
	}
}

DEVILUTION_END_NAMESPACE
