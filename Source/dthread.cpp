/**
 * @file dthread.cpp
 *
 * Implementation of functions for updating game state from network commands.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static CCritSect sgMemCrit;
static SDL_threadID glpDThreadId;
static DMegaPkt *sgpInfoHead;
static bool _gbDthread_running;
static event_emul *sghWorkToDoEvent;
static SDL_Thread *sghThread = NULL;

static unsigned int dthread_handler(void *data)
{
	DMegaPkt *pkt;
	unsigned dwMilliseconds;

	while (_gbDthread_running) {
		if (sgpInfoHead == NULL && WaitForEvent(sghWorkToDoEvent) == -1) {
			app_fatal("dthread4:\n%s", SDL_GetError());
		}

		sgMemCrit.Enter();
		pkt = sgpInfoHead;
		if (sgpInfoHead != NULL)
			sgpInfoHead = sgpInfoHead->dmpNext;
		else
			ResetEvent(sghWorkToDoEvent);
		sgMemCrit.Leave();

		if (pkt != NULL) {
			if (pkt->dmpPlr != MAX_PLRS)
				multi_send_zero_packet(pkt->dmpPlr, pkt->dmpCmd, &pkt->data[0], pkt->dmpLen);
			dwMilliseconds = 1000 * pkt->dmpLen / gdwDeltaBytesSec;

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

void dthread_send_delta(int pnum, char cmd, void *pbSrc, int dwLen)
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
	if (sghWorkToDoEvent == NULL) {
		app_fatal("dthread:1\n%s", SDL_GetError());
	}

	_gbDthread_running = true;

	sghThread = CreateThread(dthread_handler, &glpDThreadId);
	if (sghThread == NULL) {
		app_fatal("dthread2:\n%s", SDL_GetError());
	}
}

void dthread_cleanup()
{
	DMegaPkt *tmp;

	if (sghWorkToDoEvent == NULL) {
		return;
	}

	_gbDthread_running = false;
	SetEvent(sghWorkToDoEvent);
	if (sghThread != NULL && glpDThreadId != SDL_GetThreadID(NULL)) {
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
