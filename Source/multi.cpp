/**
 * @file multi.cpp
 *
 * Implementation of functions for keeping multiplaye games in sync.
 */
#include "all.h"
#include "diabloui.h"
#include "storm/storm_net.h"
#include <time.h>

DEVILUTION_BEGIN_NAMESPACE

#define PKT_HDR_CHECK	SDL_SwapBE16(*((WORD*)"ip"))

/* Flag to test if Diablo was beaten while the player loaded its level. */
bool gbSomebodyWonGameKludge;
/* Buffer to hold turn-chunks with high priority. */
static TBuffer sgHiPriBuf;
/* Buffer to hold turn-chunks with low priority. */
static TBuffer sgLoPriBuf;
/* Buffer to hold the received player-info. */
static PkPlayerStruct netplr[MAX_PLRS];
/* Current offset in netplr. */
static WORD sgwPackPlrOffsetTbl[MAX_PLRS];
/* Specifies whether the player joins an existing game. */
static bool gbJoinGame;
/* A table in which the leaving players are registered with the reason for the leaving. (LEAVE_) */
static int sgbPlayerLeftGameTbl[MAX_PLRS];
/* The number of active players in the game. */
BYTE gbActivePlayers;
/* Mask of pnum values who requested delta-info. */
unsigned guSendDelta;
/* Specifies whether the provider needs to be selected in the menu. */
bool gbSelectProvider;
/* Specifies whether the hero needs to be selected in the menu. */
bool gbSelectHero;
/* The last tick before the timeout happened. */
static int sglTimeoutStart;
/* The base value to initialize the AI_Seeds of monsters in multi-player games. */
static uint32_t sgdwGameLoops;
/**
 * Specifies the maximum number of players in a game, where 1
 * represents a single player game and 4 represents a multi player game.
 */
BYTE gbMaxPlayers;
/* Specifies whether there is a timeout at the moment. */
static bool _gbTimeout;
/* Specifies the pnum of the delta-sender. */
BYTE gbDeltaSender;
/* Turn-id when the delta was loaded. */
uint32_t guDeltaTurn;
static bool _gbNetInited;
/* The name/address of the current game. (multiplayer games) */
const char *szGameName;
/* The password of the current game. (multiplayer games) */
const char *szGamePassword;
/* The network-state of the players. (PCS_) */
unsigned player_state[MAX_PLRS];

void multi_init_buffers()
{
	sgLoPriBuf.dwDataSize = 0;
	sgLoPriBuf.bData[0] = 0;
	sgHiPriBuf.dwDataSize = 0;
	sgHiPriBuf.bData[0] = 0;
}

static void multi_queue_chunk(TBuffer *buf, void *chunk, BYTE size)
{
	BYTE *p;

	if (buf->dwDataSize + size + 2 > sizeof(buf->bData)) {
		return;
	}

	p = &buf->bData[buf->dwDataSize];
	buf->dwDataSize += size + 1;
	*p = size;
	p++;
	memcpy(p, chunk, size);
	p[size] = 0;
}

static BYTE *multi_add_chunks(TBuffer *pBuf, BYTE *dest, unsigned *size)
{
	BYTE *src_ptr;
	size_t chunk_size;

	if (pBuf->dwDataSize != 0) {
		src_ptr = &pBuf->bData[0];
		while (TRUE) {
			chunk_size = *src_ptr;
			if (chunk_size == 0 || chunk_size > *size)
				break;
			src_ptr++;
			memcpy(dest, src_ptr, chunk_size);
			dest += chunk_size;
			src_ptr += chunk_size;
			*size -= chunk_size;
		}
		pBuf->dwDataSize -= (src_ptr - &pBuf->bData[0]);
		memcpy(&pBuf->bData[0], src_ptr, pBuf->dwDataSize + 1);
	}
	return dest;
}

static void multi_init_pkt_header(TurnPktHdr &pktHdr, unsigned len)
{
	PlayerStruct *p;

	pktHdr.wLen = SwapLE16(len);
	pktHdr.wCheck = PKT_HDR_CHECK;
	p = &myplr;
	pktHdr.px = p->_px;
	pktHdr.py = p->_py;
	pktHdr.php = SwapLE32(p->_pHitPoints);
	//pktHdr.pmhp = SwapLE32(p->_pMaxHP);
	pktHdr.pmp = SwapLE32(p->_pMana);
	//pktHdr.pmmp = SwapLE32(p->_pMaxMana);
}

static void multi_send_turn_packet()
{
	BYTE *dstEnd;
	unsigned size, len;
	TurnPkt pkt;

	size = gdwNormalMsgSize - sizeof(TurnPktHdr);
	dstEnd = multi_add_chunks(&sgHiPriBuf, &pkt.body[0], &size);
	dstEnd = multi_add_chunks(&sgLoPriBuf, dstEnd, &size);
	size = sync_all_monsters(dstEnd, size);
	len = gdwNormalMsgSize - size;
	multi_init_pkt_header(pkt.hdr, len);
	nthread_send_turn((BYTE*)&pkt, len);
}

/**
 * Broadcast a message with low priority using the queue.
 *
 * @param pbMsg: the content of the message
 * @param bLen: the length of the message
 */
void NetSendLoPri(BYTE* pbMsg, BYTE bLen)
{
	multi_queue_chunk(&sgLoPriBuf, pbMsg, bLen);
}

/**
 * Broadcast a message with high priority using the queue.
 *
 * @param pbMsg: the content of the message
 * @param bLen: the length of the message
 */
void NetSendHiPri(BYTE* pbMsg, BYTE bLen)
{
	multi_queue_chunk(&sgHiPriBuf, pbMsg, bLen);
}

/**
 * Send a packet to the target player(s) selected by the pmask without using the queue.
 *
 * @param pmask: The mask of the player indices to receive the data. Or SNPLAYER_ALL to send to everyone.
 * @param src: the content of the message
 * @param bLen: the length of the message
 */
void multi_send_direct_msg(unsigned pmask, BYTE* src, BYTE bLen)
{
	unsigned i, len = bLen;
	MsgPkt pkt;

	memcpy(&pkt.body[0], src, len);
	len += sizeof(pkt.hdr);
	pkt.hdr.wLen = SwapLE16(len);
	pkt.hdr.wCheck = PKT_HDR_CHECK;
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

void multi_mon_seeds()
{
	int i;
	uint32_t l;

	sgdwGameLoops++;
	l = (sgdwGameLoops >> 8) | (sgdwGameLoops << 24); // _rotr(sgdwGameLoops, 8)
	for (i = 0; i < MAXMONSTERS; i++)
		monster[i]._mAISeed = l + i;
}

static void multi_parse_turns()
{
	int pnum;
	// TODO: use pre-allocated space?
	SNetTurnPkt* turn = SNetReceiveTurn(player_state);

	multi_process_turn(turn);

	if (!gbJoinGame && guSendDelta != 0) {
		for (pnum = 0; pnum < MAX_PLRS; pnum++, guSendDelta >>= 1) {
			if (guSendDelta & 1) {
				DeltaExportData(pnum, turn->nmpTurn);
			}
		}
	}
	MemFreeDbg(turn);
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

static void multi_deactivate_player(int pnum, int reason)
{
	const char *pszFmt;

	if (plr.plractive) {
		RemovePortalMissile(pnum);
		DeactivatePortal(pnum);
		delta_close_portal(pnum);
		multi_disband_team(pnum);
		if (plr.plrlevel == currLvl._dLevelIdx) {
			AddUnLight(plr._plid);
			AddUnVision(plr._pvid);
			RemovePlrFromMap(pnum);
			RemovePlrMissiles(pnum);
		}
		if (reason != LEAVE_NONE) {
			pszFmt = "Player '%s' left the game";
			switch (reason) {
			//case LEAVE_UNKNOWN:
			//	break;
			case LEAVE_ENDING:
				pszFmt = "Player '%s' killed Diablo and left the game!";
				gbSomebodyWonGameKludge = true;
				break;
			case LEAVE_DROP:
				pszFmt = "Player '%s' dropped due to timeout";
				break;
			}
			EventPlrMsg(pszFmt, plr._pName);
		}
		sgwPackPlrOffsetTbl[pnum] = 0;
		plr.plractive = FALSE;
		plr._pName[0] = '\0';
		guTeamInviteRec &= ~(1 << pnum);
		guTeamInviteSent &= ~(1 << pnum);
		guTeamMute &= ~(1 << pnum);
		gbActivePlayers--;
	}
}

static void multi_check_left_plrs()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (sgbPlayerLeftGameTbl[i] != LEAVE_NONE) {
			if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
				msg_send_drop_plr(i, sgbPlayerLeftGameTbl[i]);
			else
				multi_deactivate_player(i, sgbPlayerLeftGameTbl[i]);

			sgbPlayerLeftGameTbl[i] = LEAVE_NONE;
		}
	}
}

void multi_player_left(int pnum, int reason)
{
	//assert(reason != LEAVE_NONE);
	sgbPlayerLeftGameTbl[pnum] = reason;
	multi_check_left_plrs();
}

static void multi_drop_players()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		//if (!(player_state[i] & PCS_ACTIVE) && player_state[i] & PCS_CONNECTED) {
		if ((player_state[i] & (PCS_ACTIVE | PCS_CONNECTED)) == PCS_CONNECTED) {
			SNetDropPlayer(i);
		}
	}
}

bool multi_check_timeout()
{
	int i, nTicks, nState, nLowestActive, nLowestPlayer;
	BYTE activePlrs, inActivePlrs;

	if (!_gbTimeout) {
		_gbTimeout = true;
		sglTimeoutStart = SDL_GetTicks();
		return false;
	}
#ifdef _DEBUG
	if (debug_mode_key_i) {
		return false;
	}
#endif

	nTicks = SDL_GetTicks() - sglTimeoutStart;
	if (nTicks > 10000) {
		gbRunGame = false;
		return true;
	}
	if (nTicks < 5000) {
		return false;
	}

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
	return true;
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
		multi_send_turn_packet();
		break;
	case TS_LIVE:
		break;
	case TS_DESYNC: {
		InitDiabloMsg(EMSG_DESYNC);
		uint32_t turn = /*SwapLE32(*/SNetLastTurn(player_state);//);
		if (!(player_state[mypnum] & PCS_TURN_ARRIVED))
			sgbSentThisCycle = turn;
		multi_parse_turns();
		multi_process_msgs();
		multi_send_turn_packet();
	} break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	_gbTimeout = false;
	multi_mon_seeds();
	return true;
}

static void multi_process_turn_packet(int pnum, BYTE *pData, int nSize)
{
	int nLen;

	while (nSize != 0) {
		nLen = ParseCmd(pnum, (TCmd *)pData);
		if (nLen == 0) {
			break;
		}
		pData += nLen;
		nSize -= nLen;
	}
}

void multi_process_turn(SNetTurnPkt* turn)
{
	TurnPktHdr *pkt;
	unsigned dwMsgSize;
	int pnum;
	BYTE *data, *dataEnd;

	data = turn->data;
	dataEnd = data + turn->nmpLen;
	while (data != dataEnd) {
		pnum = *data;
		data++;
		dwMsgSize = *(unsigned*)data;
		data += sizeof(unsigned);
		pkt = (TurnPktHdr*)data;
		data += dwMsgSize;
		if (dwMsgSize < sizeof(TurnPktHdr))
			continue;
		if ((unsigned)pnum >= MAX_PLRS)
			continue;
		if (pkt->wCheck != PKT_HDR_CHECK)
			continue;
		if (pkt->wLen != SwapLE16(dwMsgSize))
			continue;
		if (pnum != mypnum) {
			// ASSERT: assert(geBufferMsgs != MSG_RUN_DELTA);
			plr._pHitPoints = SwapLE32(pkt->php);
			//plr._pMaxHP = SwapLE32(pkt->pmhp);
			plr._pMana = SwapLE32(pkt->pmp);
			if (currLvl._dLevelIdx != plr.plrlevel) {
				SetPlayerLoc(&plr, pkt->px, pkt->py);
			}
		}
		multi_process_turn_packet(pnum, (BYTE*)(pkt + 1), dwMsgSize - sizeof(TurnPktHdr));
		//multi_check_left_plrs();
	}
	sgdwGameLoops = 4 * turn->nmpTurn * gbNetUpdateRate;
}

void multi_process_msgs()
{
	MsgPktHdr* pkt;
	unsigned dwMsgSize, dwReadSize;
	int pnum;

	multi_check_left_plrs();
	//multi_process_tmsgs();
	while (SNetReceiveMessage(&pnum, (BYTE **)&pkt, &dwMsgSize)) {
		multi_check_left_plrs();
		if (dwMsgSize < sizeof(MsgPktHdr))
			continue;
		//if ((unsigned)pnum >= MAX_PLRS)
		//	continue;
		if (pkt->wCheck != PKT_HDR_CHECK)
			continue;
		if (pkt->wLen != SwapLE16(dwMsgSize))
			continue;
		dwMsgSize -= sizeof(MsgPktHdr);
		dwReadSize = ParseMsg(pnum, (TCmd*)(pkt + 1));
		assert(dwReadSize == dwMsgSize);
	}
}

/**
 * Send a (possible) large packet to the target player using TCmdPlrInfoHdr without using the queue.
 *
 * @param pnum: The player index of the player to receive the data.
 * @param bCmd: the message type (CMD_*)
 * @param pbSrc: the content of the message
 * @param dwLen: the length of the message
 */
void multi_send_large_direct_msg(int pnum, BYTE bCmd, BYTE *pbSrc, unsigned dwLen)
{
	unsigned dwOffset, dwBody, dwMsg;
	MsgPkt pkt;
	TCmdPlrInfoHdr *p;

	/// ASSERT: assert(pnum != mypnum);
	/// ASSERT: assert(pbSrc != NULL);
	/// ASSERT: assert(dwLen <= 0x0ffff);

	pkt.hdr.wCheck = PKT_HDR_CHECK;
	dwOffset = 0;

	while (dwLen != 0) {
		p = (TCmdPlrInfoHdr*)pkt.body;
		p->bCmd = bCmd;
		p->wOffset = SwapLE16(dwOffset);
		dwBody = gdwLargestMsgSize - sizeof(pkt.hdr) - sizeof(*p);
		if (dwLen < dwBody) {
			dwBody = dwLen;
		}
		/// ASSERT: assert(dwBody <= 0x0ffff);
		p->wBytes = SwapLE16(dwBody);
		memcpy(&pkt.body[sizeof(*p)], pbSrc, dwBody);
		dwMsg = dwBody + sizeof(pkt.hdr) + sizeof(*p);
		pkt.hdr.wLen = SwapLE16(dwMsg);
		SNetSendMessage(pnum, (BYTE*)&pkt, dwMsg);
		pbSrc += dwBody;
		dwLen -= dwBody;
		dwOffset += dwBody;
	}
}

static void multi_send_plrinfo_msg(int pnum, BYTE cmd)
{
	PkPlayerStruct pkplr;

	PackPlayer(&pkplr, mypnum);
	dthread_send_delta(pnum, cmd, &pkplr, sizeof(pkplr));
}

static void SetupLocalPlr()
{
	PlayerStruct *p;
	int x, y;

	x = 65 + DBORDERX;
	y = 58 + DBORDERY;
#ifdef _DEBUG
	if (debug_mode_key_inverted_v) {
		x = 39 + DBORDERX;
		y = 13 + DBORDERY;
	}
	if (!leveldebug || gbMaxPlayers != 1) {
		EnterLevel(DLV_TOWN);
	}
#else
	EnterLevel(DLV_TOWN);
#endif
	x += plrxoff[mypnum];
	y += plryoff[mypnum];
	p = &myplr;
	SetPlayerLoc(p, x, y);
	p->plrlevel = currLvl._dLevelIdx;
	p->_pLvlChanging = TRUE;
	p->_pmode = PM_NEWLVL;
	p->destAction = ACTION_NONE;
	p->pDungMsgs = 0;
#ifdef HELLFIRE
	p->pDungMsgs2 = 0;
#endif
	lvlLoad = 10;
	gbActivePlayers = 1;
	p->plractive = TRUE;
	assert(p->_pTeam == mypnum);
	// initialize values which are stored in save files TODO: move to LoadGameLevel?
	InitAutomapOnce();
}

static void multi_handle_events(SNetEvent *pEvt)
{
	unsigned pnum, LeftReason;

	assert(pEvt->eventid == EVENT_TYPE_PLAYER_LEAVE_GAME);
	assert(pEvt->databytes == 1);

	LeftReason = pEvt->_eData[0];
	pnum = pEvt->playerid;
	if (pnum < MAX_PLRS) {
		sgbPlayerLeftGameTbl[pnum] = LeftReason;
		if (LeftReason == LEAVE_ENDING)
			gbSomebodyWonGameKludge = true;
	} else {
		assert(pnum == SNPLAYER_MASTER);
		EventPlrMsg("Server is down");
	}

	dthread_remove_player(pnum);

	if (gbDeltaSender == pnum)
		gbDeltaSender = SNPLAYER_ALL;
}

void NetClose()
{
	if (!_gbNetInited) {
		return;
	}

	_gbNetInited = false;
	nthread_cleanup();
	dthread_cleanup();
	UIDisconnectGame(gbCineflag ? LEAVE_ENDING : LEAVE_UNKNOWN);
}

/*void mainmenu_change_name(int arg1, int arg2, int arg3, int arg4, char *name_1, char *name_2)
{
	if (UiValidPlayerName(name_2))
		pfile_rename_hero(name_1, name_2);
}*/

static bool multi_init_game(bool bSinglePlayer, SNetGameData &sgGameInitInfo)
{
	int i, dlgresult, pnum;

	while (TRUE) {
		// mypnum = 0;

		// select provider
		if (gbSelectProvider) {
			if (!UiSelectProvider(!bSinglePlayer)) {
				return false;
			}
			gbSelectProvider = false;
		}
		// select hero
		if (gbSelectHero) {
			dlgresult = UiSelHeroDialog(
				pfile_ui_set_hero_infos,
				pfile_ui_save_create,
				pfile_delete_save,
				pfile_ui_set_class_stats,
				gszHero);

			if (dlgresult == SELHERO_PREVIOUS) {
				// SErrSetLastError(1223);
				if (bSinglePlayer)
					return false;
				gbSelectProvider = true;
				continue;
			}
		} else {
			dlgresult = SELHERO_NEW_DUNGEON;
		}
		gbSelectHero = bSinglePlayer;
		gbLoadGame = dlgresult == SELHERO_CONTINUE;
		pfile_create_player_description();

		if (gbLoadGame) {
			// mypnum = 0;
			sgGameInitInfo.bMaxPlayers = 1;
			sgGameInitInfo.bTickRate = gnTicksRate;
			sgGameInitInfo.bNetUpdateRate = 1;
			break;
		}

		// select game
		//  sets sgGameInitInfo except for bPlayerId, dwSeed (if not joining a game) and dwVersionId
		dlgresult = UiSelectGame(&sgGameInitInfo, multi_handle_events);
		if (dlgresult == SELGAME_PREVIOUS) {
			gbSelectHero = true;
			continue;
		}

		if (dlgresult == SELGAME_JOIN) {
			pnum = sgGameInitInfo.bPlayerId;
			if (mypnum != pnum) {
				copy_pod(plr, myplr);
				mypnum = pnum;
				myplr._pTeam = mypnum;
				//pfile_read_player_from_save();
			}
			gbJoinGame = true;
		}
		break;
	}

	gbMaxPlayers = sgGameInitInfo.bMaxPlayers;
	gnTicksRate = sgGameInitInfo.bTickRate;
	gnTickDelay = 1000 / gnTicksRate;
	gbNetUpdateRate = sgGameInitInfo.bNetUpdateRate;
	assert(mypnum == sgGameInitInfo.bPlayerId);
	gnDifficulty = sgGameInitInfo.bDifficulty;
	SetRndSeed(sgGameInitInfo.dwSeed);

	for (i = 0; i < NUM_LEVELS; i++) {
		glSeedTbl[i] = GetRndSeed();
	}
	SNetGetGameInfo(&szGameName, &szGamePassword);

	InitQuests();
	InitPortals();
	return true;
}

bool NetInit(bool bSinglePlayer)
{
	SNetGameData sgGameInitInfo;

	while (TRUE) {
		SetRndSeed(0);
		sgGameInitInfo.dwSeed = time(NULL);
		sgGameInitInfo.dwVersionId = GAME_VERSION;
		sgGameInitInfo.bPlayerId = 0;
		//sgGameInitInfo.bDifficulty = DIFF_NORMAL;
		//sgGameInitInfo.bTickRate = SPEED_NORMAL;
		//sgGameInitInfo.bNetUpdateRate = 1;
		//sgGameInitInfo.bMaxPlayers = MAX_PLRS;
		gbJoinGame = false;
		memset(players, 0, sizeof(players));
		if (!multi_init_game(bSinglePlayer, sgGameInitInfo))
			return false;
		static_assert(LEAVE_NONE == 0, "NetInit uses memset to reset the LEAVE_ enum values.");
		memset(sgbPlayerLeftGameTbl, 0, sizeof(sgbPlayerLeftGameTbl));
		memset(sgwPackPlrOffsetTbl, 0, sizeof(sgwPackPlrOffsetTbl));
		guSendDelta = 0;
		_gbNetInited = true;
		_gbTimeout = false;
		delta_init();
		InitPlrMsg();
		multi_init_buffers();
		nthread_start(); 
		dthread_start();
		sgdwGameLoops = 0;
		nthread_send_turn();
		nthread_run();
		SetupLocalPlr();
		if (!gbJoinGame)
			break;
		multi_send_plrinfo_msg(SNPLAYER_ALL, NMSG_SEND_PLRINFO);
		if (DownloadDeltaInfo()) {
			//nthread_finish(); - do not, because if would send a join-level message
			break;
		}
		NetClose();
	}
	assert(mypnum == sgGameInitInfo.bPlayerId);
	assert(gbMaxPlayers == sgGameInitInfo.bMaxPlayers);
	assert(gnTicksRate == sgGameInitInfo.bTickRate);
	assert(gnTickDelay == 1000 / gnTicksRate);
	assert(gbNetUpdateRate == sgGameInitInfo.bNetUpdateRate);
	assert(gnDifficulty == sgGameInitInfo.bDifficulty);
	return true;
}

void multi_recv_plrinfo_msg(int pnum, TCmdPlrInfoHdr* piHdr)
{
	if ((unsigned)pnum >= MAX_PLRS)
		return;
	// assert(pnum != mypnum);
	if (sgwPackPlrOffsetTbl[pnum] != piHdr->wOffset) {
		// invalid data -> drop
		return;
	}
	if (sgwPackPlrOffsetTbl[pnum] == 0 && piHdr->bCmd != NMSG_ACK_PLRINFO) {
		multi_send_plrinfo_msg(pnum, NMSG_ACK_PLRINFO);
	}

	if (piHdr->wBytes == 0)
		return; // 'invalid' data -> skip to prevent reactivation of a player
	memcpy((char *)&netplr[pnum] + piHdr->wOffset, &piHdr[1], piHdr->wBytes); /* todo: cast? */
	sgwPackPlrOffsetTbl[pnum] += piHdr->wBytes;
	if (sgwPackPlrOffsetTbl[pnum] != sizeof(*netplr)) {
		return;
	}

	//sgwPackPlrOffsetTbl[pnum] = 0; - do NOT reset the offset to prevent reactivation of a player
	if (plr.plractive)
		return; // player was imported during delta-load -> skip
	// TODO: validate PkPlayerStruct coming from internet?
	UnPackPlayer(&netplr[pnum], pnum);
}

DEVILUTION_END_NAMESPACE
