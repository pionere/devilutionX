/**
 * @file nthread.cpp
 *
 * Implementation of functions for managing game ticks.
 */
#include "all.h"
#include "storm/storm_net.h"
#include <deque>

DEVILUTION_BEGIN_NAMESPACE

const unsigned gdwDeltaBytesSec = 0x100000; // TODO: add to SNetGameData ? (was bytessec and 1000000 in vanilla)

/* The id of the next turn. */
uint32_t sgbSentThisCycle;
Uint32 guNextTick;
/* The number of game-logic cycles between turns. */
BYTE gbNetUpdateRate;
#if !NONET
/* The number of extra turns to be queued to ensure fluent gameplay. */
BYTE gbEmptyTurns;
#ifdef ADAPTIVE_NETUPDATE
/* The 'health' of the connection. Incremented on timeout, decremented if a turn is received on time. */
char gbNetUpdateWeight;
#endif // ADAPTIVE_NETUPDATE
/* The thread to handle turns while connecting or loading a level. */
static SDL_Thread* sghThread = NULL;
/* Main mutex of the thread to control its execution. */
static CCritSect sgThreadMutex;
/* Data mutex of the thread to control access to sgTurnQueue. */
static CCritSect sgDataMutex;
#endif // NONET
/* Queued turns while loading a level, or delta-info. */
static std::deque<SNetTurnPkt*> sgTurnQueue;
/* Counter to keep track of the network-update(turn) progress. */
static BYTE sgbPacketCountdown;
static bool _gbTickInSync;
static bool _gbThreadLive;
static bool _gbRunThread;

void nthread_send_turn(BYTE *data, unsigned len)
{
	uint32_t turn = sgbSentThisCycle;
// enabled for everyone to allow connection with adaptive hosts
//#ifdef ADAPTIVE_NETUPDATE
#if !NONET
restart:
#endif
	SNetSendTurn(SwapLE32(turn), data, len);
	turn++;
	// commented out to raise the possible up-time of a game
	// minor hickup might occur around overflow, but ignore it for the moment
	//if (turn >= (UINT32_MAX / gbNetUpdateRate))
	//	turn &= 0xFFFF;
//#ifdef ADAPTIVE_NETUPDATE
#if !NONET
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
#if !NONET
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
		// skip turn of the delta-info, but increment the turn-counter
		if (turn->nmpTurn != guDeltaTurn || guDeltaTurn == 0)
			multi_process_turn(turn);
		MemFreeDbg(turn);
		multi_process_msgs();
		gdwGameLogicTurn += gbNetUpdateRate;
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
		if (geBufferMsgs == MSG_GAME_DELTA_LOAD) {
			multi_process_msgs();
			if (geBufferMsgs != MSG_GAME_DELTA_LOAD) {
				// delta-download finished -> jump to 'present'
				// necessary in case the current player is the only player on a hosted server
				sgbSentThisCycle = sgbSentThisCycle >= guDeltaTurn ? sgbSentThisCycle : guDeltaTurn;
			} else
				sgbSentThisCycle = 0;
		} else if (geBufferMsgs == MSG_GAME_DELTA_WAIT) {
			sgbSentThisCycle = 0;
			// wait a few turns to stabilize the turn-id
			if (sgTurnQueue.size() * gbNetUpdateRate * gnTickDelay > 500) {
				geBufferMsgs = MSG_GAME_DELTA_LOAD;

				TCmd cmd;
				cmd.bCmd = NMSG_SEND_GAME_DELTA;
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
#endif
void nthread_start()
{
	assert(geBufferMsgs == MSG_NORMAL);
	guNextTick = SDL_GetTicks() /*+ gnTickDelay*/;
	_gbTickInSync = true;
	sgbSentThisCycle = 0;
	sgbPacketCountdown = 1;
#if !NONET
	gbEmptyTurns = 0;
#ifdef ADAPTIVE_NETUPDATE
	gbNetUpdateWeight = 0;
#else
	// allow connection with adaptive hosts
	if (gbNetUpdateRate == 1 && !IsLocalGame)
		gbEmptyTurns = 1;
#endif
	if (!IsLocalGame) {
		_gbRunThread = false;
		sgThreadMutex.Enter();
		_gbThreadLive = true;
		assert(sghThread == NULL);
		sghThread = CreateThread(nthread_handler);
		assert(sghThread != NULL);
	}
#endif
}

void nthread_cleanup()
{
	SNetTurnPkt* tmp;
#if !NONET
	_gbThreadLive = false;
	if (sghThread != NULL && SDL_GetThreadID(sghThread) != SDL_GetThreadID(NULL)) {
		if (!_gbRunThread)
			sgThreadMutex.Leave();
		SDL_WaitThread(sghThread, NULL);
		sghThread = NULL;
	}
#endif
	while (!sgTurnQueue.empty()) {
		tmp = sgTurnQueue.front();
		sgTurnQueue.pop_front();
		MemFreeDbg(tmp);
	}
}

void nthread_run()
{
	gbLvlLoad = 10;
#if !NONET
	if (sghThread != NULL && !_gbRunThread) {
		_gbRunThread = true;
		sgThreadMutex.Leave();

		plrmsg_delay(true);
	}
#endif
}

bool nthread_level_turn()
{
	bool result = false;

	while (true) {
		switch (nthread_recv_turns()) {
		case TS_ACTIVE: {
			SNetTurnPkt* turn = SNetReceiveTurn(player_state);
			if (geBufferMsgs == MSG_NORMAL) {
				multi_process_turn(turn);
				MemFreeDbg(turn);
			} else {
				assert(geBufferMsgs == MSG_LVL_DELTA_WAIT);
				multi_pre_process_turn(turn);
				sgTurnQueue.push_back(turn);
			}
			if (guSendLevelData != 0) {
				if (IsLocalGame) {
					guSendLevelData = 0;
					assert(mypnum == 0);
					guReceivedLevelDelta = 1;
				} else {
					assert(mypnum < MAX_PLRS);
					LevelDeltaExport();
				}
			}

			multi_process_msgs();
#ifdef ADAPTIVE_NETUPDATE
			if (SNetGetTurnsInTransit() <= gbEmptyTurns)
#endif
				multi_send_turn_packet();
			result = true;
		} break;
		case TS_LIVE: {
			int delta = guNextTick - SDL_GetTicks();
			if (delta > 0) {
				SDL_Delay(delta);
			}
			continue;
		}
#if !NONET
		case TS_TIMEOUT:
			if (gbEmptyTurns < 50) {
				SDL_Delay(1);
				continue;
			}
			// sync took too much time -> disconnect
			/* fall-through */
		case TS_DESYNC:
			app_warn("Unable to join level.");
			gbRunGame = false;
			break;
#endif
		default:
			ASSUME_UNREACHABLE
			break;
		}
		break;
	}

	return result;
}

static bool nthread_process_pending_delta_turns(bool pre)
{
	SNetTurnPkt* turn;
	int i;

	while (gbRunGame) {
		//sgDataMutex.Enter();
		if (sgTurnQueue.empty()) {
			//sgDataMutex.Leave();
			break;
		}

		turn = sgTurnQueue.front();
		if (geBufferMsgs != MSG_LVL_DELTA_SKIP_JOIN)
			multi_process_turn(turn);
		else
			geBufferMsgs = MSG_NORMAL;
		if (pre && turn->nmpTurn >= guDeltaTurn) {
			net_assert(turn->nmpTurn == guDeltaTurn);
			break;
		}
		sgTurnQueue.pop_front();
		//sgDataMutex.Leave();

		MemFreeDbg(turn);
		multi_process_msgs();
		if (!pre) {
			for (i = gbNetUpdateRate; i > 0; i--) {
				++gbLvlLoad;
				game_logic();
			}
		} else {
			gdwGameLogicTurn += gbNetUpdateRate;
		}
	}

	return gbRunGame;
}

/*
                         lvl    out  reply  proc. proc. proc.  proc.  valid       turns  
 phase		       idx dun  cmd   reqs  dlvl  join  other  msgs/d delta  inc. timed queued  mode
 0. send plrinfo        +  +     -      -   -     -     -      -      -       -     +     +     [normal]
 1. wait delta          +  +     0      -   -     -     -      -      -       -     +     +     wait-delta
 2. req. delta(t)       +  +     0      -   -     -     -      -      -       -     +     +     wait->load-delta
 3. store delta(t)      +  +     0      -   -     -     -      t/-    -       x     +     +     load-delta
 4. pend. drop/plrinfo  +  +     -      -   -     -     -      -      -       -     +     +     [normal]
-----
 5. load level          +  +     0      -   -     -     -      -      -       -     +     +     [normal]
 6. (skip) pend. turns EXT +     -      -   -     +*    +      m/+    -       +     -     -     normal
 7. clear out          EXT +    [+]   -/+   -     +*    +      m/+    +       +     +     -     normal
 8. send lvljoin&wait  EXT +   join     -   -     -     -      -      +       -     ?     -     wait-lvldelta
 9. wait lvldelta      EXT +    [+]   -/+   +     +*    -      m/+    -       +     +     +     wait->proc-lvldelta
10. (proc) pend. turns EXT +     -      -   -     +**   +      m/+    -      -+     -     -     proc-lvldelta
11. load lvldelta       +  +     -      -   -     -     -      -      -       -     ?     -     [proc-lvldelta]
12. act. turns          +  +     -      -   -     +     +      m/+    -       +     -     -     skip/normal
13. act. game           +  +    [+]   +/+   -     +     +      m/+    +       +     +     -     normal
                               [dlvl]
                              [delta]

*: activate players, register join to reply with dlvl without init
**: init lvl players (if active) without activate/join registration

 */
void nthread_finish(UINT uMsg)
{
	uint32_t lastGameTurn;
	unsigned tmp;

	if (uMsg == DVL_DWM_NEWGAME) {
		if (gbLoadGame/*&& gbValidSaveFile*/) {
			assert(sghThread == NULL);
			assert(geBufferMsgs == MSG_NORMAL);
			assert(sgbPacketCountdown == 1);
			return;
		}
	}
	// phase 5 done
	// phase 6 begin
	// set current level to an invalid level
	//  so the localized messages are considered external
	assert(currLvl._dLevelIdx == myplr._pDunLevel);
	currLvl._dLevelIdx = DLV_INVALID;
#if !NONET
	// process messages arrived during level-load
	if (sghThread != NULL) {
		nthread_process_pending_turns();

		// 'pause' the nthread
		sgThreadMutex.Enter();
		_gbRunThread = false;

		nthread_process_pending_turns();
	}
	IncProgress();
#endif
	// phase 6 end
	// phase 7 begin - clear queued outgoing messages (e.g. CMD_DEACTIVATEPORTAL)
	for (int i = SNetGetTurnsInTransit(); i > 0; i--) {
		if (!nthread_level_turn())
			goto done;
	}
	IncProgress();
	// phase 7 end
	// phase 8
	NetSendCmdJoinLevel();
	// phase 9 begin - wait for join level replies
	//   process only joinlevel commands and deltalevel/leave messages
	assert(guReceivedLevelDelta == 0);
	geBufferMsgs = MSG_LVL_DELTA_WAIT;
	// TODO: delta_init_level_data ?
	//memset(gsDeltaData.ddRecvLastCmd, NMSG_LVL_DELTA_END, sizeof(gsDeltaData.ddRecvLastCmd));
	gsDeltaData.ddRecvLastCmd = NMSG_LVL_DELTA_END;
	//gsDeltaData.ddDeltaSender = SNPLAYER_ALL;
	//gsDeltaData.ddSendRecvOffset = 0;
	assert((gdwLastGameTurn * gbNetUpdateRate) == gdwGameLogicTurn);
	lastGameTurn = gdwLastGameTurn;
	while (geBufferMsgs == MSG_LVL_DELTA_WAIT) {
		if (!nthread_level_turn())
			goto done;
		unsigned allPlayers = 0;
		for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (plr._pActive)
				allPlayers |= 1 << pnum;
		}
		if (allPlayers == guReceivedLevelDelta) {
			break;
		}
	}
	IncProgress();
	assert(geBufferMsgs == MSG_LVL_DELTA_WAIT ||
		(gdwLastGameTurn >= guDeltaTurn && guDeltaTurn > lastGameTurn));	// TODO: overflow hickup
	gdwLastGameTurn = lastGameTurn;
	gdwGameLogicTurn = lastGameTurn * gbNetUpdateRate;
	tmp = guSendLevelData; // preserve this mask, requests of the pending turns are supposed to be handled
	// phase 9 end
#if !NONET
	if (geBufferMsgs != MSG_LVL_DELTA_WAIT) {
		// phase 10a - level-delta received
		assert(geBufferMsgs == MSG_LVL_DELTA_PROC);
		if (!nthread_process_pending_delta_turns(true))
			goto done;
		// phase 11 - load received level-delta
		assert(geBufferMsgs == MSG_LVL_DELTA_PROC);
		LevelDeltaLoad();
		assert(geBufferMsgs == MSG_NORMAL);
		assert(currLvl._dLevelIdx == myplr._pDunLevel);
		// phase 12
		// assert(geBufferMsgs == MSG_NORMAL);
		geBufferMsgs = MSG_LVL_DELTA_SKIP_JOIN;
		nthread_process_pending_delta_turns(false);
#else
	assert(geBufferMsgs == MSG_LVL_DELTA_WAIT);
	if (FALSE) {
#endif /* !NONET */
	} else {
		// phase 10b
		geBufferMsgs = MSG_NORMAL;
		guDeltaTurn = UINT32_MAX;
		if (!nthread_process_pending_delta_turns(true))
			goto done;
		// phase 11-12b
		assert(currLvl._dLevelIdx == DLV_INVALID);
		currLvl._dLevelIdx = myplr._pDunLevel;
		if (IsMultiGame) {
			ResyncQuests();
			DeltaLoadLevel();
		}
		SyncPortals();
		InitLvlPlayer(mypnum, true);
	}
	guSendLevelData &= tmp;
done:
	// skip till next turn
	if (_gbTickInSync) {
		guNextTick += (gbNetUpdateRate - sgbPacketCountdown) * gnTickDelay;
	}
	sgbPacketCountdown = 1; //gbNetUpdateRate;
	// reset DeltaTurn to prevent turn-skips in case of turn_id-overflow
	guDeltaTurn = 0;
	// reset mask of received level-deltas
	guReceivedLevelDelta = 0;
	// reset geBufferMsgs to normal
	geBufferMsgs = MSG_NORMAL;
	plrmsg_delay(false);
}

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
