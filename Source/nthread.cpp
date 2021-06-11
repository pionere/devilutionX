/**
 * @file nthread.cpp
 *
 * Implementation of functions for managing game ticks.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE sgbNetUpdateRate;
static CCritSect sgMemCrit;
bool _sbNthreadShouldRun;
uint32_t gdwTurnsInTransit;
uint32_t* glpMsgTbl[MAX_PLRS];
SDL_threadID glpNThreadId;
char sgbSyncCountdown;
/* flag to tell the receiver, whether a delta should be sent */
uint32_t turn_delta_request;
bool _gbTicsOutOfSync;
static BYTE sgbPacketCountdown;
bool _gbMutexDisabled;
const unsigned gdwDeltaBytesSec = 0x100000; // TODO: add to _SNETGAMEDATA ? (was bytessec and 1000000 in vanilla)
const unsigned gdwLargestMsgSize = MAX_NETMSG_SIZE; // TODO: add to _SNETGAMEDATA ? (was maxmessagesize in vanilla)
unsigned gdwNormalMsgSize;
int last_tick;

/* data */
static SDL_Thread *sghThread = NULL;

void nthread_terminate_game(const char *pszFcn)
{
	DWORD sErr;

	sErr = SErrGetLastError();
	if (sErr == STORM_ERROR_INVALID_PLAYER) {
		return;
	} else if (sErr == STORM_ERROR_GAME_TERMINATED) {
		gbGameDestroyed = true;
	} else if (sErr == STORM_ERROR_NOT_IN_GAME) {
		gbGameDestroyed = true;
	} else {
		app_fatal("%s:\n%s", pszFcn, SDL_GetError());
	}
}

uint32_t nthread_send_and_recv_turn(uint32_t cur_turn, int turn_delta)
{
	uint32_t turn, new_cur_turn, curTurnsInTransit;

	new_cur_turn = cur_turn;
	curTurnsInTransit = SNetGetTurnsInTransit();
	/*if (!SNetGetTurnsInTransit(&curTurnsInTransit)) {
		nthread_terminate_game("SNetGetTurnsInTransit");
		return 0;
	}*/
	while (curTurnsInTransit++ < gdwTurnsInTransit) {

		turn = turn_delta_request | (new_cur_turn & 0x7FFFFFFF);
		turn_delta_request = 0;

		SNetSendTurn(turn);
		/*if (!SNetSendTurn(turn)) {
			nthread_terminate_game("SNetSendTurn");
			return 0;
		}*/

		new_cur_turn += turn_delta;
		if (new_cur_turn >= 0x7FFFFFFF)
			new_cur_turn &= 0xFFFF;
	}
	return new_cur_turn;
}

bool nthread_recv_turns(bool *received)
{
	*received = false;
	sgbPacketCountdown--;
	if (sgbPacketCountdown != 0) {
		last_tick += gnTickDelay;
		return true;
	}
	sgbSyncCountdown--;
	sgbPacketCountdown = sgbNetUpdateRate;
	if (sgbSyncCountdown != 0) {

		*received = true;
		last_tick += gnTickDelay;
		return true;
	}
#ifdef __3DS__
	return false;
#else
	if (!SNetReceiveTurns(glpMsgTbl, player_state)) {
		if (SErrGetLastError() != STORM_ERROR_NO_MESSAGES_WAITING)
			nthread_terminate_game("SNetReceiveTurns");
		_gbTicsOutOfSync = false;
		sgbSyncCountdown = 1;
		sgbPacketCountdown = 1;
		return false;
	} else {
		if (!_gbTicsOutOfSync) {
			_gbTicsOutOfSync = true;
			last_tick = SDL_GetTicks();
		}
		sgbSyncCountdown = 4;
		multi_msg_countdown();
		*received = true;
		last_tick += gnTickDelay;
		return true;
	}
#endif
}

static unsigned int nthread_handler(void *data)
{
	int delta;
	bool received;

	while (_sbNthreadShouldRun) {
		sgMemCrit.Enter();
		if (!_sbNthreadShouldRun) {
			sgMemCrit.Leave();
			break;
		}
		nthread_send_and_recv_turn(0, 0);
		if (nthread_recv_turns(&received))
			delta = last_tick - SDL_GetTicks();
		else
			delta = gnTickDelay;
		sgMemCrit.Leave();
		if (delta > 0)
			SDL_Delay(delta);
	}
	return 0;
}

void nthread_request_delta()
{
	turn_delta_request = 0x80000000;
}

void nthread_start(bool request_delta)
{
	last_tick = SDL_GetTicks();
	sgbPacketCountdown = 1;
	sgbSyncCountdown = 1;
	_gbTicsOutOfSync = true;
	if (request_delta)
		nthread_request_delta();
	else
		turn_delta_request = 0;
	//_SNETCAPS caps;
	//SNetGetProviderCaps(&caps);
	gdwTurnsInTransit = 1; // TODO: add to _SNETGAMEDATA ? (was defaultturnsintransit in vanilla)
	sgbNetUpdateRate = 2;  // TODO: add to _SNETGAMEDATA ? (was defaultturnssec in vanilla)
	// FIXME: instead of 20, gnTicksRate should be used, but does not really matter at the moment
	//  and gnTicksRate is not set at this point
	gdwNormalMsgSize = gdwDeltaBytesSec * sgbNetUpdateRate / 20;
	gdwNormalMsgSize *= 3;
	gdwNormalMsgSize >>= 2;
	gdwNormalMsgSize /= gbMaxPlayers;
	static_assert(sizeof(TPktHdr) < 128, "TPktHdr does not fit in a message.");
	while (gdwNormalMsgSize < 128) {
		gdwNormalMsgSize *= 2;
		sgbNetUpdateRate *= 2;
	}
	if (gdwNormalMsgSize > gdwLargestMsgSize)
		gdwNormalMsgSize = gdwLargestMsgSize;
	if (gbMaxPlayers != 1) {
		_gbMutexDisabled = false;
		sgMemCrit.Enter();
		_sbNthreadShouldRun = true;
		sghThread = CreateThread(nthread_handler, &glpNThreadId);
		if (sghThread == NULL) {
			app_fatal("nthread2:\n%s", SDL_GetError());
		}
	}
}

void nthread_cleanup()
{
	_sbNthreadShouldRun = false;
	gdwTurnsInTransit = 0;
	//gdwNormalMsgSize = 0;
	//gdwLargestMsgSize = 0;
	if (sghThread != NULL && glpNThreadId != SDL_GetThreadID(NULL)) {
		if (!_gbMutexDisabled)
			sgMemCrit.Leave();
		SDL_WaitThread(sghThread, NULL);
		sghThread = NULL;
	}
}

void nthread_ignore_mutex(bool bStart)
{
	if (sghThread != NULL) {
		if (bStart)
			sgMemCrit.Leave();
		else
			sgMemCrit.Enter();
		_gbMutexDisabled = bStart;
	}
}

/**
 * @brief Checks if it's time for the logic to advance
 * @return True if the engine should tick
 */
bool nthread_has_500ms_passed()
{
	DWORD currentTickCount;
	int ticksElapsed;

	currentTickCount = SDL_GetTicks();
	ticksElapsed = currentTickCount - last_tick;
	if (gbMaxPlayers == 1 && ticksElapsed > (int)(10 * gnTickDelay)) {
		last_tick = currentTickCount;
		ticksElapsed = 0;
	}
	return ticksElapsed >= 0;
}

DEVILUTION_END_NAMESPACE
