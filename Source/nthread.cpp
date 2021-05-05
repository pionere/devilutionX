/**
 * @file nthread.cpp
 *
 * Implementation of functions for managing game ticks.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE sgbNetUpdateRate;
static CCritSect sgMemCrit;
unsigned gdwDeltaBytesSec;
bool _sbNthreadShouldRun;
uint32_t gdwTurnsInTransit;
uint32_t* glpMsgTbl[MAX_PLRS];
SDL_threadID glpNThreadId;
char sgbSyncCountdown;
int turn_upper_bit;
bool _gbTicsOutOfSync;
char sgbPacketCountdown;
bool _gbThreadIsRunning;
unsigned gdwLargestMsgSize;
unsigned gdwNormalMsgSize;
int last_tick;
//float gfProgressToNextGameTick = 0.0f; ANIM_GAMELOGIC

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

		turn = turn_upper_bit | (new_cur_turn & 0x7FFFFFFF);
		turn_upper_bit = 0;

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
	if (sgbPacketCountdown) {
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

void nthread_set_turn_upper_bit()
{
	turn_upper_bit = 0x80000000;
}

void nthread_start(bool set_turn_upper_bit)
{
	unsigned largestMsgSize;
	_SNETCAPS caps;

	last_tick = SDL_GetTicks();
	sgbPacketCountdown = 1;
	sgbSyncCountdown = 1;
	_gbTicsOutOfSync = true;
	if (set_turn_upper_bit)
		nthread_set_turn_upper_bit();
	else
		turn_upper_bit = 0;
	//caps.size = 36;
	SNetGetProviderCaps(&caps);
	/*if (!SNetGetProviderCaps(&caps)) {
		app_fatal("nthread1:\n%s", SDL_GetError());
	}*/
	gdwTurnsInTransit = caps.defaultturnsintransit;
	if (!caps.defaultturnsintransit)
		gdwTurnsInTransit = 1;
	if (caps.defaultturnssec <= 20 && caps.defaultturnssec)
		sgbNetUpdateRate = 20 / caps.defaultturnssec;
	else
		sgbNetUpdateRate = 1;
	largestMsgSize = 512;
	if (caps.maxmessagesize < 512)
		largestMsgSize = caps.maxmessagesize;
	gdwDeltaBytesSec = caps.bytessec >> 2;
	gdwLargestMsgSize = largestMsgSize;
	gdwNormalMsgSize = caps.bytessec * sgbNetUpdateRate / 20;
	gdwNormalMsgSize *= 3;
	gdwNormalMsgSize >>= 2;
	if (caps.maxplayers > MAX_PLRS)
		caps.maxplayers = MAX_PLRS;
	gdwNormalMsgSize /= caps.maxplayers;
	while (gdwNormalMsgSize < 0x80) {
		gdwNormalMsgSize *= 2;
		sgbNetUpdateRate *= 2;
	}
	if (gdwNormalMsgSize > largestMsgSize)
		gdwNormalMsgSize = largestMsgSize;
	if (gbMaxPlayers != 1) {
		_gbThreadIsRunning = false;
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
	gdwNormalMsgSize = 0;
	gdwLargestMsgSize = 0;
	if (sghThread != NULL && glpNThreadId != SDL_GetThreadID(NULL)) {
		if (!_gbThreadIsRunning)
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
		_gbThreadIsRunning = bStart;
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

/**
 * @brief Calculates the progress in time to the next game tick
 * @return Progress as a fraction (0.0f to 1.0f)
 */
/* ANIM_GAMELOGIC
void nthread_UpdateProgressToNextGameTick()
{
	if (!gbRunGame || PauseMode || (gbMaxPlayers == 1 && gmenu_is_active())) // if game is not running or paused there is no next gametick in the near future
		return;
	int currentTickCount = SDL_GetTicks();
	int ticksElapsed = last_tick - currentTickCount;
	if (ticksElapsed <= 0) {
		gfProgressToNextGameTick = 1.0f; // game tick is due
		return;
	}
	int ticksAdvanced = gnTickDelay - ticksElapsed;
	float fraction = (float)ticksAdvanced / (float)gnTickDelay;
	if (fraction > 1.0f)
		gfProgressToNextGameTick = 1.0f;
	if (fraction < 0.0f)
		gfProgressToNextGameTick = 0.0f;
	gfProgressToNextGameTick = fraction;
}*/

DEVILUTION_END_NAMESPACE
