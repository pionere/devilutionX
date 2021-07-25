/**
 * @file dthread.cpp
 *
 * Implementation of functions for updating game state from network commands.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static CCritSect sgMemCrit;
static DMegaPkt *sgpInfoHead;
static bool _gbThreadLive;
static event_emul *sghWorkToDoEvent;
static SDL_Thread *sghThread = NULL;

static int SDLCALL dthread_handler(void* data)
{
	DMegaPkt *pkt;
	unsigned dwMilliseconds;

	while (_gbThreadLive) {
		if (sgpInfoHead == NULL)
			WaitForEvent(sghWorkToDoEvent);

		sgMemCrit.Enter();
		pkt = sgpInfoHead;
		if (sgpInfoHead != NULL)
			sgpInfoHead = sgpInfoHead->dmpNext;
		else
			ResetEvent(sghWorkToDoEvent);
		sgMemCrit.Leave();

		if (pkt != NULL) {
			if (pkt->dmpPlr != MAX_PLRS)
				multi_send_large_direct_msg(pkt->dmpPlr, pkt->dmpCmd, &pkt->data[0], pkt->dmpLen);
			dwMilliseconds = (1000 / 4) * pkt->dmpLen / gdwDeltaBytesSec;

			mem_free_dbg(pkt);

			if (dwMilliseconds != 0)
				SDL_Delay(1);
		}
	}

	return 0;
}

void dthread_remove_player(int pnum)
{
	DMegaPkt *pkt;

	sgMemCrit.Enter();
	for (pkt = sgpInfoHead; pkt != NULL; pkt = pkt->dmpNext) {
		if (pkt->dmpPlr == pnum)
			pkt->dmpPlr = MAX_PLRS;
	}
	sgMemCrit.Leave();
}

void dthread_send_delta(int pnum, BYTE cmd, void *pbSrc, int dwLen)
{
	DMegaPkt *pkt;
	DMegaPkt *p;

	assert(gbMaxPlayers != 1);

	pkt = (DMegaPkt *)DiabloAllocPtr(dwLen + sizeof(DMegaPkt) - sizeof(pkt->data));
	pkt->dmpNext = NULL;
	pkt->dmpPlr = pnum;
	pkt->dmpCmd = cmd;
	pkt->dmpLen = dwLen;
	memcpy(&pkt->data[0], pbSrc, dwLen);
	sgMemCrit.Enter();
	p = (DMegaPkt *)&sgpInfoHead;
	while (p->dmpNext != NULL) {
		p = p->dmpNext;
	}
	p->dmpNext = pkt;

	SetEvent(sghWorkToDoEvent);
	sgMemCrit.Leave();
}

void dthread_start()
{
	if (gbMaxPlayers == 1) {
		return;
	}

	sghWorkToDoEvent = StartEvent();
	assert(sghWorkToDoEvent != NULL);

	_gbThreadLive = true;

	sghThread = CreateThread(dthread_handler);
	assert(sghThread != NULL);
}

void dthread_cleanup()
{
	DMegaPkt *tmp;

	if (sghWorkToDoEvent == NULL) {
		return;
	}

	_gbThreadLive = false;
	SetEvent(sghWorkToDoEvent);
	if (sghThread != NULL && SDL_GetThreadID(sghThread) != SDL_GetThreadID(NULL)) {
		SDL_WaitThread(sghThread, NULL);
		sghThread = NULL;
	}
	EndEvent(sghWorkToDoEvent);
	sghWorkToDoEvent = NULL;

	while (sgpInfoHead != NULL) {
		tmp = sgpInfoHead->dmpNext;
		MemFreeDbg(sgpInfoHead);
		sgpInfoHead = tmp;
	}
}

DEVILUTION_END_NAMESPACE
