/**
 * @file multi.cpp
 *
 * Implementation of functions for keeping multiplaye games in sync.
 */
#include "all.h"
#include "diabloui.h"
#include "storm/storm_net.h"
#include <ctime>
#include "DiabloUI/diablo.h"

DEVILUTION_BEGIN_NAMESPACE

// #define PKT_HDR_CHECK	SDL_SwapBE16(*((WORD*)"ip"))

/* Buffer to hold turn-chunks. */
static TBuffer sgTurnChunkBuf;
/* Specifies whether the player-info is received. */
static bool sgbPackPlrTbl[MAX_PLRS];
/** Specifies whether a game should be loaded. */
bool gbLoadGame;
/* Specifies whether the player joins an existing game. */
bool gbJoinGame;
/* The number of active players in the game. */
BYTE gbActivePlayers;
/* Mask of pnum values who requested game delta. */
unsigned guSendGameDelta;
/* Mask of pnum values who requested level delta. */
unsigned guSendLevelData;
/* Mask of pnum values from whom an (empty) level delta was received. */
unsigned guOweLevelDelta;
/* Timestamp of the level-delta requests to decide priority. */
turn_t guRequestLevelData[MAX_PLRS];
/* Specifies whether the provider needs to be selected in the menu. */
bool gbSelectProvider;
/* Specifies whether the hero needs to be selected in the menu. */
bool gbSelectHero;
/* The last tick before the timeout happened. */
static Uint32 sglTimeoutStart;
/* The last processed game turn. gdwGameLogicTurn / gbNetUpdateRate if there is no overflow. */
turn_t gdwLastGameTurn;
/* The current iteration of the game logic. */
turn_t gdwGameLogicTurn;
/**
 * Specifies the type of the current game
 * 0: single player game
 * 1: 'fake' multi player game (loopback)
 * 2: 'real' multi player game (tcp/ip, zerotier, etc...)
 * 3: game server
 */
BYTE gbGameMode;
/* Specifies whether there is a timeout at the moment. */
static bool _gbTimeout;
/* Turn-id when the delta was loaded. */
turn_t guDeltaTurn;
static bool _gbNetInited;
/* The network-state of the players. (PCS_) */
unsigned player_state[MAX_PLRS];

static void multi_init_buffers()
{
	sgTurnChunkBuf.dwDataSize = 0;
	sgTurnChunkBuf.bData[0] = 0;
}

/**
 * Queues a turn-chunk to broadcast it later.
 *
 * @param pbMsg: the content of the turn-chunk
 * @param bLen: the length of the turn-chunk
 */
void NetSendChunk(const BYTE* pbMsg, BYTE bLen)
{
	BYTE* p;

	if (sgTurnChunkBuf.dwDataSize + bLen + 1 >= sizeof(sgTurnChunkBuf.bData)) {
		// TODO: should this be fatal?
		return;
	}

	p = &sgTurnChunkBuf.bData[sgTurnChunkBuf.dwDataSize];
	sgTurnChunkBuf.dwDataSize += bLen + 1;
	*p = bLen;
	p++;
	memcpy(p, pbMsg, bLen);
	p[bLen] = 0;
}

static BYTE* multi_add_chunks(BYTE* dest, unsigned* size)
{
	BYTE* src_ptr;
	unsigned chunk_size;

	if (sgTurnChunkBuf.dwDataSize != 0) {
		src_ptr = &sgTurnChunkBuf.bData[0];
		while (true) {
			chunk_size = *src_ptr;
			if (chunk_size == 0 || chunk_size > *size)
				break;
			src_ptr++;
			memcpy(dest, src_ptr, chunk_size);
			dest += chunk_size;
			src_ptr += chunk_size;
			*size -= chunk_size;
		}
		sgTurnChunkBuf.dwDataSize -= (unsigned)(src_ptr - &sgTurnChunkBuf.bData[0]);
		memcpy(&sgTurnChunkBuf.bData[0], src_ptr, sgTurnChunkBuf.dwDataSize + 1);
	}
	return dest;
}

static void multi_init_pkt_header(TurnPktHdr& pktHdr, unsigned len)
{
	// PlayerStruct* p;

	pktHdr.wLen = static_cast<uint16_t>(len);
	// pktHdr.wCheck = PKT_HDR_CHECK;
	// p = &myplr;
	// pktHdr.px = p->_px;
	// pktHdr.py = p->_py;
	// pktHdr.php = p->_pHitPoints;
	// pktHdr.pmhp = p->_pMaxHP;
	// pktHdr.pmp = p->_pMana;
	// pktHdr.pmmp = p->_pMaxMana;
}

void multi_send_turn_packet()
{
	BYTE* dstEnd;
	unsigned remsize, len;
	TurnPkt pkt;

	remsize = NET_TURN_MSG_SIZE - sizeof(TurnPktHdr);
	dstEnd = multi_add_chunks(&pkt.body[0], &remsize);
	dstEnd = sync_all_monsters(dstEnd, remsize);
	len = (unsigned)((size_t)dstEnd - (size_t)&pkt);
	multi_init_pkt_header(pkt.hdr, len);
	nthread_send_turn((BYTE*)&pkt, len);
}

/**
 * Send a packet to the target player(s) selected by the pmask without using the queue.
 *
 * @param pmask: The mask of the player indices to receive the data. Or SNPLAYER_ALL to send to everyone.
 * @param pbSrc: the content of the message
 * @param wLen: the length of the message
 */
void multi_send_direct_msg(unsigned pmask, const BYTE* pbSrc, unsigned wLen)
{
	unsigned i, len = wLen;
	NormalMsgPkt pkt;

	memcpy(&pkt.body[0], pbSrc, len);
	len += sizeof(pkt.hdr);
	pkt.hdr.wLen = static_cast<uint16_t>(len);
	// pkt.hdr.wCheck = PKT_HDR_CHECK;
	static_assert(sizeof(pmask) * CHAR_BIT > MAX_PLRS, "Sending packets with unsigned int mask does not work.");
	if (pmask == SNPLAYER_ALL) {
		static_assert(SNPLAYER_ALL >= (1 << MAX_PLRS), "SNPLAYER_ALL does not work with pnum masks.");
		SNetSendMessage(SNPLAYER_ALL, (BYTE*)&pkt, len);
	} else {
		for (i = 0; i < MAX_PLRS; i++, pmask >>= 1) {
			if (pmask & 1) {
				SNetSendMessage(i, (BYTE*)&pkt, len);
			}
		}
	}
}

void multi_rnd_seeds()
{
	int i;
	int32_t seed;

	gdwGameLogicTurn++;
	if (!IsMultiGame)
		return;
	seed = (gdwGameLogicTurn >> 8) | (gdwGameLogicTurn << 24); // _rotr(gdwGameLogicTurn, 8)
	SetRndSeed(seed);
	for (i = 0; i < MAXMONSTERS; i++, seed++)
		monsters[i]._mAISeed = seed;
}

static void multi_parse_turns()
{
	// TODO: use pre-allocated space?
	SNetTurnPkt* turn = SNetReceiveTurn(player_state);
	multi_process_turn(turn);
	mem_free_dbg(turn);
#ifndef NONET
	if (guSendGameDelta != 0) {
#ifdef ZEROTIER
		if (provider == SELCONN_ZT || !gbJoinGame) {
#else
		if (!gbJoinGame) {
#endif
			DeltaExportData(guSendGameDelta);
		}
		guSendGameDelta = 0;
	}
#endif // !NONET
	if (guSendLevelData != 0) {
#ifndef NOHOSTING
		if (mypnum < MAX_PLRS)
			LevelDeltaExport();
		else
			guSendLevelData = 0;
#else
			LevelDeltaExport();
#endif // !NOHOSTING
	}
}

/**
 * Re-assign players from a team to the first available one.
 * @param team the team to disband
 */
void multi_disband_team(int team)
{
	int pnum, new_team;

	new_team = -1;
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (pnum == team || plr._pTeam != team)
			continue;
		if (new_team == -1) {
			new_team = pnum;
		}
		plr._pTeam = new_team;
	}
}

void multi_deactivate_player(int pnum)
{
	if (plr._pActive) {
		DeactivatePortal(pnum);
		multi_disband_team(pnum);
		// ClearPlrMsg(pnum);
		RemoveLvlPlayer(pnum);
		// if (reason != LEAVE_NONE) {
		if (pnum != mypnum) {
			const char* pszFmt = "Player '%s' left the game";
			//switch (reason) {
			//case LEAVE_NORMAL:
			//	break;
			//case LEAVE_DROP:
			//	pszFmt = "Player '%s' is disconnected";
			//	break;
			//}
			EventPlrMsg(pszFmt, plr._pName);
		} else {
			gbRunGame = false;
		}
		sgbPackPlrTbl[pnum] = false;
		plr._pActive = FALSE;
		guTeamInviteRec &= ~(1 << pnum);
		guTeamInviteSent &= ~(1 << pnum);
		guOweLevelDelta &= ~(1 << pnum);
		guSendLevelData &= ~(1 << pnum);
		guTeamMute &= ~(1 << pnum);
		gbActivePlayers--;
	}
}

/*static void multi_drop_players()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		//if (!(player_state[i] & PCS_ACTIVE) && player_state[i] & PCS_CONNECTED) {
		if ((player_state[i] & (PCS_ACTIVE | PCS_CONNECTED)) == PCS_CONNECTED) {
			SNetDropPlayer(i);
		}
	}
}*/

bool multi_check_timeout()
{
	// int i, nState, nLowestActive, nLowestPlayer;
	//BYTE activePlrs, inActivePlrs;
	Uint32 now = SDL_GetTicks();
	Sint32 dTicks;

	if (!_gbTimeout) {
		_gbTimeout = true;
		sglTimeoutStart = now;
		return false;
	}

	dTicks = now - sglTimeoutStart;
	if (dTicks > 10000) {
		SNetDisconnect();
		// gbRunGame = false;
		return true;
	}
	// commented out because a client should not be authorized to drop players
	//if (dTicks < 5000) {
		return dTicks >= 1000;
	/*}

	nLowestActive = -1;
	nLowestPlayer = -1;
	activePlrs = 0;
	inActivePlrs = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		nState = player_state[i];
		if (nState & PCS_CONNECTED) {
			if (nLowestPlayer == -1) {
				nLowestPlayer = i;
			}
			if (nState & PCS_ACTIVE) {
				activePlrs++;
				if (nLowestActive == -1) {
					nLowestActive = i;
				}
			} else {
				inActivePlrs++;
			}
		}
	}

	/// ASSERT: assert(activePlrs != 0);
	/// ASSERT: assert(nLowestActive != -1);
	/// ASSERT: assert(nLowestPlayer != -1);

	if (activePlrs < inActivePlrs
	 || (activePlrs == inActivePlrs && nLowestPlayer != nLowestActive)) {
		gbRunGame = false;
	} else if (nLowestActive == mypnum) {
		multi_drop_players();
	}
	return true;*/
}

/**
 * @return Always true for singleplayer
 */
bool multi_handle_turn()
{
	switch (nthread_recv_turns()) {
	case TS_TIMEOUT:
		return false;
	case TS_ACTIVE:
		multi_parse_turns();
		multi_process_msgs();
#ifdef ADAPTIVE_NETUPDATE
		if (SNetGetTurnsInTransit() <= gbEmptyTurns)
#endif
			multi_send_turn_packet();
		break;
	case TS_LIVE:
		break;
	case TS_DESYNC:
		InitDiabloMsg(EMSG_DESYNC);
		// TODO: drop the offending players?
		multi_parse_turns();
		multi_process_msgs();
		multi_send_turn_packet();
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	_gbTimeout = false;
	return true;
}

static void multi_process_turn_packet(int pnum, BYTE* pData, int nSize)
{
	int nLen;
	// assert(nSize != 0);
	do {
		nLen = ParseCmd(pnum, (TCmd*)pData);
		// assert(nLen != 0);
		pData += nLen;
		nSize -= nLen;
	} while (nSize > 0);
	net_assert(nSize == 0);
}

void multi_process_turn(SNetTurnPkt* turn)
{
	TurnPktHdr* pkt;
	unsigned dwMsgSize;
	int pnum;
	BYTE *data, *dataEnd;

	data = turn->data;
	dataEnd = data + turn->ntpLen;
	while (data != dataEnd) {
		pnum = *data;
		data++;
		dwMsgSize = *(unsigned*)data;
		data += sizeof(unsigned);
		pkt = (TurnPktHdr*)data;
		data += dwMsgSize;
		if (dwMsgSize <= sizeof(TurnPktHdr))
			continue;
		// assert((unsigned)pnum < MAX_PLRS);
		//if (pkt->wCheck != PKT_HDR_CHECK)
		//	continue;
		if (pkt->wLen != dwMsgSize)
			continue;
		dwMsgSize -= sizeof(TurnPktHdr);
		//if (pnum != mypnum && // prevent empty turns during level load to overwrite JOINLEVEL
		// currLvl._dLevelIdx != plr._pDunLevel) { // ignore players on the same level (should be calculated by ourself)
			// ASSERT: assert(geBufferMsgs != MSG_RUN_DELTA);
		//	plr._pHitPoints = pkt->php;
			// plr._pMaxHP = pkt->pmhp;
		//	plr._pMana = pkt->pmp;
			// plr._pMaxMana = pkt->pmmp;
		//	plr._px = pkt->px;
		//	plr._py = pkt->py;
		//}
		TCmd* cmd = (TCmd*)(pkt + 1);
		if (!plr._pActive) {
			// player is disconnected -> ignore the turn, but process CMD_JOINLEVEL/CMD_REQDELTA messages
			if (cmd->bCmd == CMD_JOINLEVEL || cmd->bCmd == CMD_REQDELTA) {
				ParseCmd(pnum, cmd);
			}
			continue;
		}
		multi_process_turn_packet(pnum, (BYTE*)cmd, dwMsgSize);
	}
	gdwLastGameTurn = turn->ntpTurn;
	gdwGameLogicTurn = turn->ntpTurn * gbNetUpdateRate;
}

/* Same as multi_process_turn, but process only CMD_JOINLEVEL/CMD_DISCONNECT messages. */
void multi_pre_process_turn(SNetTurnPkt* turn)
{
	TurnPktHdr* pkt;
	unsigned dwMsgSize;
	int pnum;
	BYTE *data, *dataEnd;

	data = turn->data;
	dataEnd = data + turn->ntpLen;
	while (data != dataEnd) {
		pnum = *data;
		data++;
		dwMsgSize = *(unsigned*)data;
		data += sizeof(unsigned);
		pkt = (TurnPktHdr*)data;
		data += dwMsgSize;
		if (dwMsgSize <= sizeof(TurnPktHdr))
			continue;
		// assert((unsigned)pnum < MAX_PLRS);
		//if (pkt->wCheck != PKT_HDR_CHECK)
		//	continue;
		if (pkt->wLen != dwMsgSize)
			continue;
		TCmd* cmd = (TCmd*)(pkt + 1);
		if (cmd->bCmd == CMD_JOINLEVEL || cmd->bCmd == CMD_DISCONNECT) {
			ParseCmd(pnum, cmd);
		}
	}
	assert(geBufferMsgs == MSG_LVL_DELTA_WAIT);
	gdwLastGameTurn = turn->ntpTurn;
	gdwGameLogicTurn = turn->ntpTurn * gbNetUpdateRate;
}

static void multi_process_msg(SNetMsgPkt* msg)
{
	MsgPktHdr* pkt;
	unsigned dwMsgSize, dwReadSize;
	int pnum;

	dwMsgSize = msg->nmpLen;
	if (dwMsgSize <= sizeof(MsgPktHdr))
		return;
	//if (pkt->wCheck != PKT_HDR_CHECK)
	//	continue;
	pkt = (MsgPktHdr*)msg->data;
	if (pkt->wLen != dwMsgSize)
		return;
	pnum = msg->nmpPlr;
	// assert((unsigned)pnum < MAX_PLRS || pnum == SNPLAYER_MASTER);
	dwMsgSize -= sizeof(MsgPktHdr);
	dwReadSize = ParseMsg(pnum, (TCmd*)&pkt[1]);
	assert(dwReadSize == dwMsgSize);
}

void multi_process_msgs()
{
	SNetMsgPkt* msg;

	//multi_process_tmsgs();
	while ((msg = SNetReceiveMessage()) != NULL) {
		multi_process_msg(msg);
		mem_free_dbg(msg);
	}
}

#ifndef NOHOSTING
static unsigned gameProgress;
extern Uint32 guNextTick;
int multi_ui_handle_turn()
{
	int i;
	Sint32 delta;

	switch (nthread_recv_turns()) {
	case TS_DESYNC:
		EventPlrMsg("Multiplayer sync problem"); // TODO: MsgStrings[EMSG_DESYNC]
		// drop the offending player(s)
		for (i = 0; i < MAX_PLRS; i++) {
			if (player_state[i] & PCS_DESYNC) {
				EventPlrMsg("Player %d. dropped due to desync.", i);
				SNetDropPlayer(i);
			}
		}
		multi_parse_turns();
		multi_process_msgs();
		nthread_send_turn();
		gameProgress++;
		delta = 1;
		_gbTimeout = false;
		break;
	case TS_ACTIVE:
		multi_parse_turns();
		multi_process_msgs();
		gdwGameLogicTurn += gbNetUpdateRate;
		gameProgress++;
		for (i = 0; i < MAX_PLRS; i++) {
			if (player_state[i] & PCS_TURN_ARRIVED) {
				gameProgress++;
			}
		}
		nthread_send_turn();
		/* fall-through */
	case TS_LIVE:
		delta = guNextTick - SDL_GetTicks();
		_gbTimeout = false;
		break;
	case TS_TIMEOUT:
		gbEmptyTurns = 0;
		delta = 1; //gnTickDelay;
		if (multi_check_timeout()) {
			for (i = 0; i < MAX_PLRS; i++) {
				if ((player_state[i] & (PCS_CONNECTED | PCS_TURN_ARRIVED)) == PCS_CONNECTED) {
					EventPlrMsg("Player %d. dropped due to timeout.", i);
					SNetDropPlayer(i);
				}
			}
			_gbTimeout = false;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (delta > 0)
		SDL_Delay(delta);

	while (gameProgress >= 100)
		gameProgress -= 100;
	return gameProgress;
}

static void RunGameServer()
{
	assert(!gbJoinGame);
	assert(mypnum == SNPLAYER_MASTER);
	currLvl._dLevelIdx = DLV_INVALID;
	gbActivePlayers = 0;
	gameProgress = 0;
	InitPlrMsg();
	UiHostGameDialog();
}
#else
static void RunGameServer()
{
}
#endif // !NOHOSTING

/**
 * Send a (possible) large packet to the target player using TMsgLargeHdr without using the queue.
 *
 * @param pnum: The player index of the player to receive the data.
 * @param bCmd: the message type (CMD_*)
 * @param pbSrc: the content of the message
 * @param dwLen: the length of the message
 */
/*void multi_send_large_direct_msg(int pnum, BYTE bCmd, const BYTE* pbSrc, unsigned dwLen)
{
	unsigned dwOffset, dwBody, dwMsg;
	NormalMsgPkt pkt;
	TMsgLargeHdr* p;

	/// ASSERT: assert(pnum != mypnum);
	/// ASSERT: assert(pbSrc != NULL);
	/// ASSERT: assert(dwLen <= 0x0ffff);

	// pkt.hdr.wCheck = PKT_HDR_CHECK;
	dwOffset = 0;

	while (dwLen != 0) {
		p = (TMsgLargeHdr*)pkt.body;
		p->bCmd = bCmd;
		p->wOffset = static_cast<uint16_t>(dwOffset);
		dwBody = NET_NORMAL_MSG_SIZE - sizeof(pkt.hdr) - sizeof(*p);
		if (dwLen < dwBody) {
			dwBody = dwLen;
		}
		/// ASSERT: assert(dwBody <= 0x0ffff);
		p->wBytes = static_cast<uint16_t>(dwBody);
		memcpy(&pkt.body[sizeof(*p)], pbSrc, dwBody);
		dwMsg = dwBody + sizeof(pkt.hdr) + sizeof(*p);
		pkt.hdr.wLen = static_cast<uint16_t>(dwMsg);
		SNetSendMessage(pnum, (BYTE*)&pkt, dwMsg);
		pbSrc += dwBody;
		dwLen -= dwBody;
		dwOffset += dwBody;
	}
}*/

void multi_send_large_msg(unsigned pmask, BYTE bCmd, unsigned bodySize)
{
	LargeMsgPkt* pkt = &gsDeltaData.ddSendRecvPkt;
	unsigned i, dwTotalLen;
	// DeltaCompressData
	DWORD dwBodySize;

	if (bodySize >= NET_COMP_MSG_SIZE) {
		dwBodySize = PkwareCompress(pkt->apMsg.tpData.content, bodySize);
	} else {
		dwBodySize = bodySize;
	}
	pkt->apMsg.tpData.compressed = bodySize != dwBodySize;
	dwBodySize += sizeof(pkt->apMsg.tpData.compressed);

	dwTotalLen = dwBodySize + sizeof(pkt->apHdr) + sizeof(pkt->apMsg.tpHdr);

	pkt->apMsg.tpHdr.bCmd = bCmd;
	pkt->apMsg.tpHdr.wBytes = static_cast<uint16_t>(dwBodySize);
	pkt->apHdr.wLen = static_cast<uint16_t>(dwTotalLen);

	if (pmask == SNPLAYER_ALL) {
		static_assert(SNPLAYER_ALL >= (1 << MAX_PLRS), "SNPLAYER_ALL does not work with pnum masks.");
		SNetSendMessage(SNPLAYER_ALL, (BYTE*)pkt, dwTotalLen);
	} else {
		for (i = 0; i < MAX_PLRS; i++, pmask >>= 1) {
			if (pmask & 1) {
				SNetSendMessage(i, (BYTE*)pkt, dwTotalLen);
			}
		}
	}
}

static void multi_broadcast_plrinfo_msg()
{
	DBuffer* buff = &gsDeltaData.ddSendRecvPkt.apMsg.tpData;

	static_assert(sizeof(PkPlayerStruct) <= sizeof(buff->content), "Plrinfo does not fit to the buffer in multi_broadcast_plrinfo_msg.");
	PackPlayer((PkPlayerStruct*)buff->content, mypnum);

	multi_send_large_msg(SNPLAYER_ALL, NMSG_PLRINFO, sizeof(PkPlayerStruct));
}

static void SetupLocalPlr()
{
	PlayerStruct* p;

	p = &myplr;
	p->_pmode = PM_NEWLVL;
	p->_pDestAction = ACTION_NONE;
	//p->_pInvincible = TRUE; - does not matter in town
	p->_pLvlChanging = TRUE;
	p->_pDunLevel = DLV_TOWN;
	p->_pTeam = mypnum;
	p->_pManaShield = 0;
	p->_pTimer[PLTR_INFRAVISION] = 0;
	p->_pTimer[PLTR_RAGE] = 0;
	// reset skills
	const PlrSkillStruct psm = { { SPL_ATTACK, RSPLTYPE_ABILITY }, { SPL_WALK, RSPLTYPE_ABILITY } };
	const PlrSkillStruct psr = { { SPL_RATTACK, RSPLTYPE_ABILITY }, { SPL_WALK, RSPLTYPE_ABILITY } };
	p->_pMainSkill = (p->_pSkillFlags & SFLAG_MELEE) ? psm : psr;
	p->_pAltSkill = { { SPL_NULL, 0 }, { SPL_NULL, 0 } };
	// recalculate _pAtkSkill and resistances (depending on the difficulty level)
	// CalcPlrInv(mypnum, false); - unnecessary, InitLvlPlayer should take care of this
	if (p->_pHitPoints == 0)
		PlrSetHp(mypnum, (1 << 6));

	assert(p->_pGFXLoad == 0);

	gbActivePlayers = 1;
	p->_pActive = TRUE;
}

void multi_ui_handle_events(SNetEventHdr* pEvt)
{
	unsigned pnum;

	if (pEvt->eventid == EVENT_TYPE_PLAYER_LEAVE_GAME) {
		pnum = pEvt->playerid;
		if (pnum == SNPLAYER_MASTER) {
			EventPlrMsg("Server is down");
		}

		// dthread_remove_player(pnum);

		if (gsDeltaData.ddDeltaSender == pnum)
			gsDeltaData.ddDeltaSender = SNPLAYER_ALL;
#ifdef ZEROTIER
	} else if (pEvt->eventid == EVENT_TYPE_PLAYER_INFO) {
		SNetZtPlr *dest = ((SNetPlrInfoEvent*)pEvt)->nePlayers;
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (!plr._pActive) {
				dest[pnum].npName[0] = '\0';
				continue;
			}
			static_assert(sizeof(dest[pnum].npName) == sizeof(dest[pnum].npName), "multi_ui_handle_events can not copy the player's name.");
			memcpy(dest[pnum].npName, plr._pName, sizeof(dest[pnum].npName));
			dest[pnum].npClass = plr._pClass;
			dest[pnum].npLevel = plr._pLevel;
			dest[pnum].npRank = plr._pRank;
			dest[pnum].npTeam = plr._pTeam;
		}
#endif
	}
}

void NetClose()
{
	if (!_gbNetInited) {
		return;
	}

	_gbNetInited = false;
	nthread_cleanup();
	// dthread_cleanup();
	UIDisconnectGame();
}

static bool multi_init_game(bool bSinglePlayer, _uigamedata& gameData)
{
	int i, dlgresult;
	int32_t seed;

	while (true) {
		// mypnum = 0;

		// select provider
		if (gbSelectProvider) {
			if (!UiSelectProvider(!bSinglePlayer)) {
				return false;
			}
			gbSelectProvider = false;
		}
		gbGameMode = bSinglePlayer ? 0
#ifndef NOHOSTING
			: (provider == SELCONN_TCPS || provider == SELCONN_TCPDS) ? 3
#endif
			: (provider == SELCONN_LOOPBACK ? 1 : 2);
		// select hero
		if (!IsGameSrv && gbSelectHero) {
			dlgresult = UiSelHeroDialog(&mySaveIdx);

			if (dlgresult == SELHERO_PREVIOUS) {
				// SErrSetLastError(1223);
#ifndef NONET
				if (bSinglePlayer)
#endif
					return false;
				gbSelectProvider = true;
				continue;
			}
		}
		gbSelectHero = bSinglePlayer;
		gameData.aePlayerId = IsGameSrv ? SNPLAYER_MASTER : 0;

		// select game
		//  sets gameData except for aePlayerId, aeSeed (if not joining a game) and aeVersionId
		dlgresult = UiSelectGame(&gameData);
		if (dlgresult == SELGAME_PREVIOUS) {
			if (IsGameSrv) {
				gbSelectProvider = true;
				// mypnum = 0;
				// gameData.aePlayerId = 0;
			}
			gbSelectHero = true;
			continue;
		}
		gbLoadGame = dlgresult == SELGAME_LOAD;
		gbJoinGame = dlgresult == SELGAME_JOIN;
		mypnum = gameData.aePlayerId;
		if (!IsGameSrv) {
			pfile_read_hero();
		}
		break;
	}

	gnTicksRate = gameData.aeTickRate;
	gnTickDelay = 1000 / gnTicksRate;
	gbNetUpdateRate = gameData.aeNetUpdateRate;
	assert(mypnum == gameData.aePlayerId);
	gnDifficulty = gameData.aeDifficulty;
	SetRndSeed(gameData.aeSeed);
	sgbSentThisCycle = gameData.aeTurn;

	for (i = 0; i < NUM_FIXLVLS; i++) {
		seed = NextRndSeed();
		seed = ((uint32_t)seed >> 8) | ((uint32_t)seed << 24); // _rotr(seed, 8)
		glSeedTbl[i] = seed;
		SetRndSeed(seed);
	}

	InitQuests();
	InitPortals();
	return true;
}

bool NetInit(bool bSinglePlayer)
{
	_uigamedata gameData;

	while (true) {
		SetRndSeed(0);
		gameData.aeSeed = time(NULL);
		gameData.aeVersionId = GAME_VERSION;
		gameData.aeTurn = 0;
		//gameData.aePlayerId = 0;
		//gameData.aeDifficulty = DIFF_NORMAL;
		//gameData.aeTickRate = SPEED_NORMAL;
		//gameData.aeNetUpdateRate = 1;
		//gameData.aeMaxPlayers = MAX_PLRS;
		gbJoinGame = false;
		memset(players, 0, sizeof(players));
		if (!multi_init_game(bSinglePlayer, gameData))
			return false;
		memset(sgbPackPlrTbl, false, sizeof(sgbPackPlrTbl));
		memset(player_state, 0, sizeof(player_state));
		guSendGameDelta = 0;
		guSendLevelData = 0;
		// guOweLevelDelta = 0;
		assert(!_gbNetInited);
		_gbNetInited = true;
		_gbTimeout = false;
		delta_init();
		InitPlrMsg();
		multi_init_buffers();
		nthread_start(/*gbJoinGame*/);
		// dthread_start();
		gdwLastGameTurn = 0;
		gdwGameLogicTurn = 0;
		multi_send_turn_packet();
		if (IsGameSrv) {
			RunGameServer();
			NetClose();
			continue;
		}
		nthread_run();
		SetupLocalPlr();
#ifndef NONET
		if (!gbJoinGame)
#endif
			break;
		multi_broadcast_plrinfo_msg();
		if (DownloadDeltaInfo()) {
			//nthread_finish(); - do not, because it would send a join-level message
			break;
		}
		NetClose();
	}
	assert(mypnum == gameData.aePlayerId);
	assert(gnTicksRate == gameData.aeTickRate);
	assert(gnTickDelay == 1000u / gnTicksRate);
	assert(gbNetUpdateRate == gameData.aeNetUpdateRate);
	assert(gnDifficulty == gameData.aeDifficulty);
	return true;
}

void multi_recv_plrinfo_msg(int pnum, const TMsgLarge* piMsg)
{
	// assert((unsigned)pnum < MAX_PLRS);
	// assert(pnum != mypnum);
	if (sgbPackPlrTbl[pnum]) {
		// player info received already -> drop
		return;
	}
	if (plr._pActive) {
		return; // player was imported during delta-load -> skip
	}

	DWORD size = piMsg->tpHdr.wBytes - sizeof(piMsg->tpData.compressed);

	if (size > sizeof(PkPlayerStruct)) {
		// invalid data -> drop
		return;
	}
	PkPlayerStruct netplr;
	memcpy(&netplr, piMsg->tpData.content, size);
	// DeltaDecompressData
	if (piMsg->tpData.compressed) {
		PkwareDecompress((BYTE*)&netplr, size, sizeof(PkPlayerStruct));
	}
	// TODO: check (decompressed) size ?
	//if (size != sizeof(PkPlayerStruct)) {
	//	// invalid data -> drop
	//	return;
	//}
#if INET_MODE
	// TODO: extend validation of PkPlayerStruct?
	if (netplr.pName[0] == '\0'
	 || netplr.pTeam != pnum
	 || netplr.pDunLevel != DLV_TOWN
	 //|| !netplr.pLvlChanging
	 || netplr.pLevel > MAXCHARLEVEL
	 || netplr.pClass >= NUM_CLASSES) {
		// invalid data -> drop
		return;
	}
#endif
	sgbPackPlrTbl[pnum] = true; // register data to prevent reactivation of a player
	UnPackPlayer(&netplr, pnum);
}

bool multi_plrinfo_received(int pnum)
{
	return sgbPackPlrTbl[pnum];
}

DEVILUTION_END_NAMESPACE
