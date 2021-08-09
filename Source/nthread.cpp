/**
 * @file nthread.cpp
 *
 * Implementation of functions for managing game ticks.
 */
#include "all.h"
#include "storm/storm_net.h"
#include <deque>

DEVILUTION_BEGIN_NAMESPACE

BYTE gbNetUpdateRate;
BYTE gbEmptyTurns;
#ifdef ADAPTIVE_NETUPDATE
char gbNetUpdateWeight;
#endif
/* The id of the next turn. */
uint32_t sgbSentThisCycle;
/* Main mutex of the thread to control its execution. */
static CCritSect sgThreadMutex;
/* Data mutex of the thread to control access to sgTurnQueue. */
static CCritSect sgDataMutex;
/* Queued turns while loading a level, or delta-info. */
static std::deque<SNetTurnPkt*> sgTurnQueue;
/* Counter to keep track of the network-update(turn) progress. */
static BYTE sgbPacketCountdown;
const unsigned gdwDeltaBytesSec = 0x100000; // TODO: add to SNetGameData ? (was bytessec and 1000000 in vanilla)
const unsigned gdwLargestMsgSize = MAX_NETMSG_SIZE; // TODO: add to SNetGameData ? (was maxmessagesize in vanilla)
const unsigned gdwNormalMsgSize = MAX_NETMSG_SIZE;
Uint32 guNextTick;
static bool _gbTickInSync;
static SDL_Thread *sghThread = NULL;
static bool _gbThreadLive;
static bool _gbRunThread;

void nthread_send_turn(BYTE *data, unsigned len)
{
	uint32_t turn = sgbSentThisCycle;
// enabled for everyone to allow connection with adaptive hosts
//#ifdef ADAPTIVE_NETUPDATE
#ifndef NONET
restart:
#endif
	SNetSendTurn(SwapLE32(turn), data, len);
	turn++;
	if (turn >= 0x7FFFFFFF)
		turn &= 0xFFFF;
//#ifdef ADAPTIVE_NETUPDATE
#ifndef NONET
	if (gbEmptyTurns != 0 && SNetGetTurnsInTransit() <= gbEmptyTurns) {
		len = 0;
		goto restart;
	}
#endif
	sgbSentThisCycle = turn;
}

int nthread_recv_turns()
{
	turn_status status = SNetPollTurns(player_state);

	if (status == TS_DESYNC)
		return status;
	sgbPacketCountdown--;
	if (sgbPacketCountdown != 0) {
		guNextTick += gnTickDelay;
		return TS_LIVE;
	}
	sgbPacketCountdown = gbNetUpdateRate;
	if (status == TS_TIMEOUT) {
#ifdef ADAPTIVE_NETUPDATE
		if (_gbTickInSync) {
			gbNetUpdateWeight += 10;
			if (gbNetUpdateWeight > 100) {
				if (((gbEmptyTurns * gbNetUpdateRate) * gnTickDelay) < 200) {
					gbEmptyTurns++;
					gbNetUpdateWeight = 0;
				} else {
					gbNetUpdateWeight = 50;
				}
			}
		}
#endif
		_gbTickInSync = false;
		sgbPacketCountdown = 1;
		return TS_TIMEOUT;
	} else {
#ifdef ADAPTIVE_NETUPDATE
		gbNetUpdateWeight--;
		if (gbNetUpdateWeight < -100) {
			if (gbEmptyTurns != 0) {
				gbEmptyTurns--;
				gbNetUpdateWeight = 0;
			} else {
				gbNetUpdateWeight = -50;
			}
		}
#endif
		if (!_gbTickInSync) {
			_gbTickInSync = true;
			guNextTick = SDL_GetTicks();
		}
		guNextTick += gnTickDelay;
		return TS_ACTIVE;
	}
}

static void nthread_parse_turns()
{
	SNetTurnPkt* turn = SNetReceiveTurn(player_state);

	sgDataMutex.Enter();
	sgTurnQueue.push_back(turn);
	sgDataMutex.Leave();
}

static void nthread_process_pending_turns()
{
	SNetTurnPkt* turn;
	int i;

	while (gbRunGame) {
		sgDataMutex.Enter();
		if (sgTurnQueue.empty()) {
			sgDataMutex.Leave();
			break;
		}

		turn = sgTurnQueue.front();
		sgTurnQueue.pop_front();
		sgDataMutex.Leave();

		// skip turns arrived before delta-info
		if (turn->nmpTurn < guDeltaTurn) {
			MemFreeDbg(turn);
			continue;
		}

		multi_process_turn(turn);
		MemFreeDbg(turn);
		multi_process_msgs();
		for (i = gbNetUpdateRate; i > 0; i--) {
			game_logic();
			++gbLvlLoad;
		}
		// remove the generated NetCmds
		// there should be only a few obsolete ones (e.g. monster hit by a trap)
		multi_init_buffers();

		// reset geBufferMsgs to normal
		assert(geBufferMsgs == MSG_NORMAL || geBufferMsgs == MSG_INITIAL_PENDINGTURN);
		geBufferMsgs = MSG_NORMAL;
	}
}

static int SDLCALL nthread_handler(void* data)
{
	int delta;

	while (_gbThreadLive) {
		sgThreadMutex.Enter();
		if (!_gbThreadLive) {
			sgThreadMutex.Leave();
			break;
		}
		if (geBufferMsgs == MSG_DOWNLOAD_DELTA) {
			multi_process_msgs();
			if (geBufferMsgs != MSG_DOWNLOAD_DELTA) {
				// delta-download finished -> jump to 'present'
				// necessary in case the current player is the only player on a hosted server
				sgbSentThisCycle = sgbSentThisCycle >= guDeltaTurn ? sgbSentThisCycle : guDeltaTurn;
			} else
				sgbSentThisCycle = 0;
		} else if (geBufferMsgs == MSG_REQUEST_DOWNLOAD_DELTA) {
			sgbSentThisCycle = 0;
			// wait a few turns to stabilize the turn-id
			if (sgTurnQueue.size() * gbNetUpdateRate * gnTickDelay > 500) {
				geBufferMsgs = MSG_DOWNLOAD_DELTA;
				
				TCmd cmd;
				cmd.bCmd = NMSG_SEND_DELTAINFO;
				multi_send_direct_msg(SNPLAYER_ALL, (BYTE*)&cmd, sizeof(cmd));
			}
		}

		switch (nthread_recv_turns()) {
		case TS_DESYNC: {
			uint32_t turn = /*SwapLE32(*/SNetLastTurn(player_state);//);
			if (!(player_state[mypnum] & PCS_TURN_ARRIVED))
				sgbSentThisCycle = turn;
			nthread_parse_turns();
			nthread_send_turn();
			delta = 1; //gnTickDelay;
		} break;
		case TS_ACTIVE:
			nthread_parse_turns();
			nthread_send_turn();
			/* fall-through */
		case TS_LIVE:
			delta = guNextTick - SDL_GetTicks();
			break;
		case TS_TIMEOUT:
			delta = 1; //gnTickDelay;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		sgThreadMutex.Leave();
		if (delta > 0)
			SDL_Delay(delta);
	}
	return 0;
}

void nthread_start()
{
	guNextTick = SDL_GetTicks() /*+ gnTickDelay*/;
	_gbTickInSync = true;
	sgbSentThisCycle = 0;
	sgbPacketCountdown = 1;
	gbEmptyTurns = 0;
#ifdef ADAPTIVE_NETUPDATE
	gbNetUpdateWeight = 0;
#elif !defined(NONET)
	// allow connection with adaptive hosts
	if (gbNetUpdateRate == 1 && !IsLocalGame)
		gbEmptyTurns = 1;
#endif
	static_assert(sizeof(TurnPkt) <= gdwNormalMsgSize, "TurnPkt does not fit in a message.");
	static_assert(sizeof(MsgPkt) <= gdwNormalMsgSize, "MsgPkt does not fit in a message.");
	if (!IsLocalGame) {
		_gbRunThread = false;
		sgThreadMutex.Enter();
		_gbThreadLive = true;
		sghThread = CreateThread(nthread_handler);
		assert(sghThread != NULL);
	}
}

void nthread_cleanup()
{
	SNetTurnPkt* tmp;

	_gbThreadLive = false;
	if (sghThread != NULL && SDL_GetThreadID(sghThread) != SDL_GetThreadID(NULL)) {
		if (!_gbRunThread)
			sgThreadMutex.Leave();
		SDL_WaitThread(sghThread, NULL);
		sghThread = NULL;
	}
	while (!sgTurnQueue.empty()) {
		tmp = sgTurnQueue.front();
		sgTurnQueue.pop_front();
		MemFreeDbg(tmp);
	}
}

void nthread_run()
{
	plrmsg_delay(true);
	if (sghThread != NULL) {
		sgThreadMutex.Leave();
		_gbRunThread = true;
	}
}

void nthread_finish()
{
	if (sghThread != NULL) {
		// for the first pending turn the current player should be considered
		//  as 'external' in On_SYNCQUESTEXT
		geBufferMsgs = MSG_INITIAL_PENDINGTURN;

		nthread_process_pending_turns();

		// 'pause' the nthread
		sgThreadMutex.Enter();
		_gbRunThread = false;

		nthread_process_pending_turns();

		// skip till next turn
		if (_gbTickInSync) {
			guNextTick += (gbNetUpdateRate - sgbPacketCountdown) * gnTickDelay;
		}
		sgbPacketCountdown = 1; //gbNetUpdateRate;
		// reset DeltaTurn to prevent turn-skips in case of turn_id-overflow
		guDeltaTurn = 0;
		// reset geBufferMsgs to normal in case there was no pending turn
		geBufferMsgs = MSG_NORMAL;
	}
	// TODO: move these somewhere else?
	//   1. it prevents NetInit from calling in a symmetric way
	//   2. plrmsg_delay calls are non-symmetric...
	if (myplr._pmode == PM_NEWLVL) { // skip in case the game is loaded
		NetSendCmdSendJoinLevel();
	}
	plrmsg_delay(false);
}

/**
 * @brief Checks if it's time for the logic to advance
 * @return True if the engine should tick
 */
bool nthread_has_50ms_passed()
{
	Uint32 currentTickCount;
	int ticksElapsed;

	currentTickCount = SDL_GetTicks();
	ticksElapsed = currentTickCount - guNextTick;
	// catch up if the host is too slow (only in local games)
	if (IsLocalGame && ticksElapsed > (int)(10 * gnTickDelay)) {
		guNextTick = currentTickCount;
		ticksElapsed = 0;
	}
	return ticksElapsed >= 0;
}

DEVILUTION_END_NAMESPACE
