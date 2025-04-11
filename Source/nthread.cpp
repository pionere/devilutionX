/**
 * @file nthread.cpp
 *
 * Implementation of functions for managing game ticks.
 */
#include "all.h"
#include "storm/storm_net.h"
#include "utils/thread.h"
#include <deque>

DEVILUTION_BEGIN_NAMESPACE

const unsigned gdwDeltaBytesSec = 0x100000; // TODO: add to SNetGameData ? (was bytessec and 1000000 in vanilla)

/* The id of the next turn. */
turn_t sgbSentThisCycle;
Uint32 guNextTick;
/* The number of game-logic cycles between turns. */
BYTE gbNetUpdateRate;
#ifndef NONET
/* The number of extra turns to be queued to ensure fluent gameplay. */
BYTE gbEmptyTurns;
#ifdef ADAPTIVE_NETUPDATE
/* The 'health' of the connection. Incremented on timeout, decremented if a turn is received on time. */
int gbNetUpdateWeight;
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
#ifndef NONET
static bool _gbThreadLive;
static bool _gbRunThread;
#endif // NONET

void nthread_send_turn(BYTE* data, unsigned len)
{
	turn_t turn = sgbSentThisCycle;
// enabled for everyone to allow connection with adaptive hosts
//#ifdef ADAPTIVE_NETUPDATE
#ifndef NONET
restart:
#endif
	SNetSendTurn(turn, data, len);
	turn++;
	// commented out to raise the possible up-time of a game
	// minor hickup might occur around overflow, but ignore it for the moment
	//if (turn >= (UINT32_MAX / gbNetUpdateRate))
	//	turn &= 0xFFFF;
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
			gbNetUpdateWeight += 200;
			if (gbNetUpdateWeight > 1000) {
				if (((gbEmptyTurns * gbNetUpdateRate) * gnTickDelay) < 200) {
					gbEmptyTurns++;
				}
				gbNetUpdateWeight = 0;
			}
		}
#endif
		_gbTickInSync = false;
		sgbPacketCountdown = 1;
		return TS_TIMEOUT;
	} else {
#ifdef ADAPTIVE_NETUPDATE
		gbNetUpdateWeight--;
		if (gbNetUpdateWeight < -1000) {
			if (gbEmptyTurns != 0) {
				gbEmptyTurns--;
				gbNetUpdateWeight = 0;
			} else {
				gbNetUpdateWeight = -500;
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
#ifndef NONET
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
		if (turn->ntpTurn < guDeltaTurn) {
			MemFreeDbg(turn);
			continue;
		}
		// skip turn of the delta-info, but increment the turn-counter
		if (turn->ntpTurn != guDeltaTurn)
			multi_process_turn(turn);
		MemFreeDbg(turn);
		multi_process_msgs();
		gdwGameLogicTurn += gbNetUpdateRate;
	}
}

static int SDLCALL nthread_handler(void* data)
{
	Sint32 delta;

	while (_gbThreadLive) {
		sgThreadMutex.Enter();
		if (!_gbThreadLive) {
			sgThreadMutex.Leave();
			break;
		}
		if (geBufferMsgs == MSG_GAME_DELTA_LOAD) {
			multi_process_msgs();
		} else if (geBufferMsgs == MSG_GAME_DELTA_WAIT) {
			if (!sgTurnQueue.empty()) {
				geBufferMsgs = MSG_GAME_DELTA_LOAD;
			}
		}

		switch (nthread_recv_turns()) {
		case TS_DESYNC: {
			InitDiabloMsg(EMSG_DESYNC);
			// TODO: drop the offending player?
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
	gbLvlLoad = true; // TODO: set this when _pLvlChanging of the local player is set to TRUE?
	assert(geBufferMsgs == MSG_NORMAL);
	guNextTick = SDL_GetTicks() /*+ gnTickDelay*/;
	if (gbJoinGame) {
		geBufferMsgs = MSG_GAME_DELTA_WAIT;
		guNextTick += NET_JOIN_WINDOW * gnTickDelay * gbNetUpdateRate;
		NetSendCmd(CMD_REQDELTA);
	}
	_gbTickInSync = true;
	sgbPacketCountdown = 1;
#ifndef NONET
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
#ifndef NONET
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
	gbLvlLoad = true;
#ifndef NONET
	if (sghThread != NULL && !_gbRunThread) {
		_gbRunThread = true;
		sgThreadMutex.Leave();
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
					guOweLevelDelta = 0;
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
			Sint32 delta = guNextTick - SDL_GetTicks();
			if (delta > 0) {
				SDL_Delay(delta);
			}
			continue;
		}
#ifndef NONET
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
		if (pre && turn->ntpTurn == guDeltaTurn) {
			break;
		}
		sgTurnQueue.pop_front();
		//sgDataMutex.Leave();

		MemFreeDbg(turn);
		multi_process_msgs();
		if (!pre) {
			for (i = gbNetUpdateRate; i > 0; i--) {
				game_logic();
			}
		} else {
			gdwGameLogicTurn += gbNetUpdateRate;
		}
	}

	return gbRunGame;
}

static void nthread_finish_dungeon()
{
	InitSync();
	// finalize the light/vision calculations
	DRLG_RedoTrans();
	ProcessLightList();
	ProcessVisionList();
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
	turn_t lastGameTurn;
	unsigned tmp;

	if (uMsg == DVL_DWM_LOADGAME) {
#ifndef NONET
		assert(sghThread == NULL);
#endif
		assert(geBufferMsgs == MSG_NORMAL);
		assert(sgbPacketCountdown == 1);
		// IncProgress();
		// IncProgress();
		// IncProgress();
	} else {
		// phase 5 done
		// phase 6 begin
		// set current level to an invalid level
		//  so the localized messages are considered external
		assert(currLvl._dLevelIdx == myplr._pDunLevel);
		currLvl._dLevelIdx = DLV_INVALID;
#ifndef NONET
		// "Network - Pending Turns" (13)
		// process messages arrived during level-load
		if (sghThread != NULL) {
			nthread_process_pending_turns();

			// 'pause' the nthread
			sgThreadMutex.Enter();
			_gbRunThread = false;

			nthread_process_pending_turns();
		}
#endif
		IncProgress(); // "Network - Msg Queue" (14)
		// phase 6 end
		// phase 7 begin - clear queued outgoing messages (e.g. CMD_DEACTIVATEPORTAL)
		for (int i = SNetGetTurnsInTransit(); i > 0; i--) {
			if (!nthread_level_turn()) {
				// IncProgress();
				// IncProgress();
				// assert(!gbRunGame);
				goto fail;
			}
		}
		IncProgress(); // "Network - Join Level" (15)
		// phase 7 end
		// phase 8
		NetSendCmdJoinLevel();
		// phase 9 begin - wait for join level replies
		//   process only joinlevel commands and deltalevel/leave messages
		geBufferMsgs = MSG_LVL_DELTA_WAIT;
		tmp = 0;
		for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (plr._pActive)
				tmp |= 1 << pnum;
		}
		assert(tmp & (1 << mypnum));
		guOweLevelDelta = tmp;
		// TODO: delta_init_level_data ?
		//memset(gsDeltaData.ddRecvLastCmd, NMSG_LVL_DELTA_END, sizeof(gsDeltaData.ddRecvLastCmd));
		gsDeltaData.ddRecvLastCmd = NMSG_LVL_DELTA_END;
		//gsDeltaData.ddDeltaSender = SNPLAYER_ALL;
		//gsDeltaData.ddSendRecvOffset = 0;
		assert((gdwLastGameTurn * gbNetUpdateRate) == gdwGameLogicTurn);
		lastGameTurn = gdwLastGameTurn;
		tmp = 0;
		while (geBufferMsgs == MSG_LVL_DELTA_WAIT) {
			if (!nthread_level_turn()) {
				// IncProgress();
				// assert(!gbRunGame);
				goto fail;
			}
			if (guOweLevelDelta == 0) {
				break;
			}
			if (++tmp > NET_JOIN_TIMEOUT) {
				app_warn("Unable to join level.");
				gbRunGame = false;
				// IncProgress();
				goto fail;
			}
		}
		IncProgress(); // "Network - Sync delta" (16)
		assert(geBufferMsgs == MSG_LVL_DELTA_WAIT
		 || (gdwLastGameTurn >= guDeltaTurn && guDeltaTurn > lastGameTurn)); // TODO: overflow hickup
		gdwLastGameTurn = lastGameTurn;
		gdwGameLogicTurn = lastGameTurn * gbNetUpdateRate;
		tmp = guSendLevelData; // preserve this mask, requests of the pending turns are supposed to be handled
		// phase 9 end
#ifndef NONET
		if (geBufferMsgs != MSG_LVL_DELTA_WAIT) {
			// phase 10a - level-delta received
			assert(geBufferMsgs == MSG_LVL_DELTA_PROC);
			if (!nthread_process_pending_delta_turns(true))
				goto fail;
			// phase 11 - load received level-delta
			assert(geBufferMsgs == MSG_LVL_DELTA_PROC);
			geBufferMsgs = MSG_NORMAL;
			assert(currLvl._dLevelIdx == DLV_INVALID);
			currLvl._dLevelIdx = myplr._pDunLevel;
			// assert(IsMultiGame);
			ResyncQuests();
			DeltaLoadLevel();
			//SyncPortals();
			LevelDeltaLoad();
			nthread_finish_dungeon();
			// phase 12
			// assert(currLvl._dLevelIdx == myplr._pDunLevel);
			// assert(!sgTurnQueue.empty());
			// assert(geBufferMsgs == MSG_NORMAL);
			geBufferMsgs = MSG_LVL_DELTA_SKIP_JOIN;
			gbLvlLoad = false;
			nthread_process_pending_delta_turns(false);
			// assert(geBufferMsgs == MSG_NORMAL);
#else
		assert(geBufferMsgs == MSG_LVL_DELTA_WAIT);
		if (FALSE) {
#endif /* !NONET */
		} else {
			// phase 10b
			geBufferMsgs = MSG_NORMAL;
			guDeltaTurn = UINT32_MAX;
			if (!nthread_process_pending_delta_turns(true))
				goto fail;
			// assert(sgTurnQueue.empty());
			// phase 11-12b
			assert(currLvl._dLevelIdx == DLV_INVALID);
			currLvl._dLevelIdx = myplr._pDunLevel;
			ResyncQuests();
			if (IsMultiGame) {
				DeltaLoadLevel();
			} else if (IsLvlVisited(currLvl._dLevelIdx)) {
				LoadLevel();
			}
			SyncPortals();
			InitLvlPlayer(mypnum, true);
			nthread_finish_dungeon();
			// assert(geBufferMsgs == MSG_NORMAL);
		}
		guSendLevelData &= tmp;
fail:
		// skip till next turn
		if (_gbTickInSync) {
			guNextTick += (gbNetUpdateRate - sgbPacketCountdown) * gnTickDelay;
		}
		sgbPacketCountdown = 1; //gbNetUpdateRate;
		// reset DeltaTurn to prevent turn-skips in case of turn_id-overflow
		guDeltaTurn = 0;
		// reset geBufferMsgs to normal (in case of failure)
		geBufferMsgs = MSG_NORMAL;
		// reset pfile timer
		guLastSaveTurn = gdwGameLogicTurn;
		// enter the dungeon level
		PlayDungMsgs();
		guLvlVisited |= LEVEL_MASK(currLvl._dLevelIdx);
#ifdef HELLFIRE
		if (quests[Q_DEFILER]._qactive == QUEST_INIT && currLvl._dLevelIdx == questlist[Q_DEFILER]._qdlvl) {
			quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
			quests[Q_DEFILER]._qlog = TRUE;
			NetSendCmdQuest(Q_DEFILER, false); // recipient should not matter
		}
#endif
#if DEV_MODE
		if (gbActivePlayers > 1 && plx(0)._pDunLevel == plx(1)._pDunLevel) {
			NetSendCmd(CMD_REQUEST_ITEMCHECK);
			NetSendCmd(CMD_REQUEST_PLRCHECK);
		}
#endif
	}
	gbLvlLoad = false;
}

Sint32 nthread_ticks2gameturn()
{
	Uint32 now;
	Sint32 ticksRemaining;

	now = SDL_GetTicks();
	ticksRemaining = guNextTick - now;
#if 0
	// catch up if the host is too slow (only in local games)
	if (IsLocalGame && (ticksRemaining + 8 * gnTickDelay) < 0) {
		guNextTick = now;
		ticksRemaining = 0;
	}
#endif
	return ticksRemaining;
}

DEVILUTION_END_NAMESPACE
