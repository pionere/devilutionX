#include "thread.h"

#include "../types.h"
#include "appfat.h"
#include <SDL.h>
#include <set>

DEVILUTION_BEGIN_NAMESPACE

static int SDLCALL ThreadTranslate(void *ptr)
{
	auto handler = (unsigned int (*)(void *))ptr;

	return handler(NULL);
}

SDL_Thread *CreateThread(unsigned int (*handler)(void *), SDL_threadID *threadId)
{
#ifdef USE_SDL1
	SDL_Thread *ret = SDL_CreateThread(ThreadTranslate, (void *)handler);
#else
	SDL_Thread *ret = SDL_CreateThread(ThreadTranslate, NULL, (void *)handler);
#endif
	if (ret == NULL) {
		ErrSdl();
	}
	*threadId = SDL_GetThreadID(ret);
	return ret;
}

event_emul *StartEvent()
{
	event_emul *ret;
	ret = (event_emul *)malloc(sizeof(event_emul));
	ret->mutex = SDL_CreateMutex();
	if (ret->mutex == NULL) {
		ErrSdl();
	}
	ret->cond = SDL_CreateCond();
	if (ret->cond == NULL) {
		ErrSdl();
	}
	return ret;
}

void EndEvent(event_emul *event)
{
	SDL_DestroyCond(event->cond);
	SDL_DestroyMutex(event->mutex);
	free(event);
}

void SetEvent(event_emul *e)
{
	if (SDL_LockMutex(e->mutex) <= -1 || SDL_CondSignal(e->cond) <= -1 || SDL_UnlockMutex(e->mutex) <= -1) {
		ErrSdl();
	}
}

void ResetEvent(event_emul *e)
{
	if (SDL_LockMutex(e->mutex) <= -1 || SDL_CondWaitTimeout(e->cond, e->mutex, 0) <= -1 || SDL_UnlockMutex(e->mutex) <= -1) {
		ErrSdl();
	}
}

void WaitForEvent(event_emul *e)
{
	if (SDL_LockMutex(e->mutex) <= -1) {
		ErrSdl();
	}
	if (SDL_CondWait(e->cond, e->mutex) <= -1 || SDL_CondSignal(e->cond) <= -1 || SDL_UnlockMutex(e->mutex) <= -1) {
		ErrSdl();
	}
}

DEVILUTION_END_NAMESPACE
