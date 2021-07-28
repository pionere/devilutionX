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
		ErrSdl();
	}
	return ret;
}

void StartEvent(event_emul &ev)
{
	ev.mutex = SDL_CreateMutex();
	if (ev.mutex == NULL) {
		ErrSdl();
	}
	ev.cond = SDL_CreateCond();
	if (ev.cond == NULL) {
		ErrSdl();
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
	if (SDL_LockMutex(ev.mutex) <= -1 || SDL_CondSignal(ev.cond) <= -1 || SDL_UnlockMutex(ev.mutex) <= -1) {
		ErrSdl();
	}
}

void ResetEvent(event_emul &ev)
{
	if (SDL_LockMutex(ev.mutex) <= -1 || SDL_CondWaitTimeout(ev.cond, ev.mutex, 0) <= -1 || SDL_UnlockMutex(ev.mutex) <= -1) {
		ErrSdl();
	}
}

void WaitForEvent(event_emul &ev)
{
	if (SDL_LockMutex(ev.mutex) <= -1) {
		ErrSdl();
	}
	if (SDL_CondWait(ev.cond, ev.mutex) <= -1 || SDL_CondSignal(ev.cond) <= -1 || SDL_UnlockMutex(ev.mutex) <= -1) {
		ErrSdl();
	}
}

DEVILUTION_END_NAMESPACE
