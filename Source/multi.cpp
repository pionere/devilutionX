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

bool gbSomebodyWonGameKludge;
/* Buffer to hold chunks with high priority. */
static TBuffer sgHiPriBuf;
/* Buffer to hold chunks with low priority. */
static TBuffer sgLoPriBuf;
/* Buffer to hold the received player-info. */
static PkPlayerStruct netplr[MAX_PLRS];
/* Current offset in netplr. */
static WORD sgwPackPlrOffsetTbl[MAX_PLRS];
static bool gbJoinGame;
static int sgbPlayerLeftGameTbl[MAX_PLRS];
static uint32_t sgbSentThisCycle;
static bool gbPacketSentRecently;
BYTE gbActivePlayers;
bool gbGameDestroyed;
static unsigned guSendDelta;
bool gbSelectProvider;
bool gbSelectHero;
static int sglTimeoutStart;
static uint32_t sgdwGameLoops;
/**
 * Specifies the maximum number of players in a game, where 1
 * represents a single player game and 4 represents a multi player game.
 */
BYTE gbMaxPlayers;
static bool _gbTimeout;
BYTE gbDeltaSender;
static bool _gbNetInited;
const char *szGameName;
const char *szGamePassword;
unsigned player_state[MAX_PLRS];

static void buffer_init(TBuffer *pBuf)
{
	pBuf->dwDataSize = 0;
	pBuf->bData[0] = 0;
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
	SNetSendMessage(SNPLAYER_ALL, (BYTE*)&pkt, len);
}

void NetSendLoPri(BYTE *pbMsg, BYTE bLen)
{
	multi_queue_chunk(&sgLoPriBuf, pbMsg, bLen);
}

void NetSendHiPri(BYTE *pbMsg, BYTE bLen)
{
	multi_queue_chunk(&sgHiPriBuf, pbMsg, bLen);

	if (!gbPacketSentRecently) {
		gbPacketSentRecently = true;
		multi_send_turn_packet();
	}
}

void multi_send_direct_msg(unsigned pmask, BYTE *src, BYTE bLen)
{
	unsigned i, len = bLen;
	TurnPkt pkt;

	memcpy(&pkt.body[0], src, len);
	len += sizeof(pkt.hdr);
	multi_init_pkt_header(pkt.hdr, len);
	static_assert(sizeof(pmask) * CHAR_BIT > MAX_PLRS, "Sending packets with unsigned int mask does not work.");
	if (pmask == SNPLAYER_ALL) {
		SNetSendMessage(SNPLAYER_ALL, (BYTE*)&pkt, len);
	} else {
		for (i = 0; i < MAX_PLRS; i++, pmask >>= 1) {
			if (pmask & 1) {
				SNetSendMessage(i, (BYTE*)&pkt, len);
			}
		}
	}
}

static void multi_mon_seeds()
{
	int i;
	uint32_t l;

	sgdwGameLoops++;
	l = (sgdwGameLoops >> 8) | (sgdwGameLoops << 24); // _rotr(sgdwGameLoops, 8)
	for (i = 0; i < MAXMONSTERS; i++)
		monster[i]._mAISeed = l + i;
}

static void multi_handle_delta_request(int pnum)
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if ((player_state[i] & PS_CONNECTED) && i != pnum)
			break;
	}

	if (mypnum == i) {
		guSendDelta |= 1 << pnum;
	} else if (mypnum == pnum) {
		gbDeltaSender = i;
	}
}

void multi_parse_turns()
{
	int pnum;
	uint32_t turn, absTurns;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (player_state[pnum] & PS_TURN_ARRIVED) {
			turn = *glpMsgTbl[pnum];
			if (turn & 0x80000000)
				multi_handle_delta_request(pnum);
			absTurns = turn & 0x7FFFFFFF;
			if (sgbSentThisCycle < gdwTurnsInTransit + absTurns) {
				if (absTurns >= 0x7FFFFFFF)
					absTurns &= 0xFFFF;
				sgbSentThisCycle = absTurns + gdwTurnsInTransit;
				sgdwGameLoops = 4 * absTurns * gbNetUpdateRate;
			}
		}
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
			pszFmt = "Player '%s' just left the game";
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

void multi_net_ping()
{
	_gbTimeout = true;
	sglTimeoutStart = SDL_GetTicks();
}

static void multi_drop_players()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if ((player_state[i] & (PS_ACTIVE | PS_CONNECTED)) == PS_CONNECTED) {
			SNetDropPlayer(i);
		}
	}
}

static void multi_check_timeout()
{
	int i, nTicks, nState, nLowestActive, nLowestPlayer;
	BYTE activePlrs, inActivePlrs;

	if (!_gbTimeout) {
		return;
	}
#ifdef _DEBUG
	if (debug_mode_key_i) {
		return;
	}
#endif

	nTicks = SDL_GetTicks() - sglTimeoutStart;
	if (nTicks > 20000) {
		gbRunGame = false;
		return;
	}
	if (nTicks < 10000) {
		return;
	}

	nLowestActive = -1;
	nLowestPlayer = -1;
	activePlrs = 0;
	inActivePlrs = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		nState = player_state[i];
		if (nState & PS_CONNECTED) {
			if (nLowestPlayer == -1) {
				nLowestPlayer = i;
			}
			if (nState & PS_ACTIVE) {
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
		gbGameDestroyed = true;
	} else if (nLowestActive == mypnum) {
		multi_drop_players();
	}
}

/**
 * @return Always true for singleplayer
 */
bool multi_handle_turn()
{
	int i;
	bool received;

	if (gbGameDestroyed) {
		gbRunGame = false;
		return false;
	}
	static_assert(sizeof(guSendDelta) * CHAR_BIT > MAX_PLRS, "Sending delta info with unsigned int mask does not work.");
	if (guSendDelta != 0) {
		for (i = 0; i < MAX_PLRS; i++, guSendDelta >>= 1) {
			if (guSendDelta & 1)
				DeltaExportData(i);
		}
	}

	sgbSentThisCycle = nthread_send_turn(sgbSentThisCycle, 1);
	if (!nthread_recv_turns(&received)) {
		multi_check_timeout();
		return false;
	}

	_gbTimeout = false;
	if (received) {
		if (!gbPacketSentRecently) {
			multi_send_turn_packet();
		} else {
			gbPacketSentRecently = false;
			if (sgHiPriBuf.dwDataSize != 0) {
				gbPacketSentRecently = true;
				multi_send_turn_packet();
			}
		}
	}
	multi_mon_seeds();

	return true;
}

static void multi_handle_all_packets(int pnum, BYTE *pData, int nSize)
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

static void multi_process_tmsgs()
{
	TCmdGItem pkt;

	while (tmsg_get(&pkt)) {
		ParseCmd(mypnum, (TCmd *)&pkt);
	}
}

void multi_process_network_packets()
{
	int dx, dy;
	TurnPktHdr *pkt;
	unsigned dwMsgSize;
	int pnum;

	multi_check_left_plrs();
	multi_process_tmsgs();
	while (SNetReceiveMessage(&pnum, (BYTE**)&pkt, &dwMsgSize)) {
		multi_check_left_plrs();
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
			//plr._pMaxMana = SwapLE32(pkt->pmmp);
			if (geBufferMsgs != MSG_DOWNLOAD_DELTA && plr.plractive && plr._pHitPoints >= (1 << 6)) {
				if (currLvl._dLevelIdx == plr.plrlevel && !plr._pLvlChanging) {
					dx = abs(plr._px - pkt->px);
					dy = abs(plr._py - pkt->py);
					if ((dx > 3 || dy > 3) && dPlayer[pkt->px][pkt->py] == 0) {
						RemovePlrFromMap(pnum);
						SetPlayerOld(pnum);
						RemovePlrFromMap(pnum);
						plr._px = pkt->px;
						plr._py = pkt->py;
						plr._pfutx = pkt->px;
						plr._pfuty = pkt->py;
						dPlayer[plr._px][plr._py] = pnum + 1;
					}
					dx = abs(plr._pfutx - plr._px);
					dy = abs(plr._pfuty - plr._py);
					if (dx > 1 || dy > 1) {
						plr._pfutx = plr._px;
						plr._pfuty = plr._py;
					}
				} else {
					plr._px = pkt->px;
					plr._py = pkt->py;
					plr._pfutx = pkt->px;
					plr._pfuty = pkt->py;
				}
			}
		}
		multi_handle_all_packets(pnum, (BYTE *)(pkt + 1), dwMsgSize - sizeof(TurnPktHdr));
	}
}

void multi_send_zero_packet(int pnum, BYTE bCmd, BYTE *pbSrc, unsigned dwLen)
{
	unsigned dwOffset, dwBody, dwMsg;
	TurnPkt pkt;
	TCmdPlrInfoHdr *p;

	/// ASSERT: assert(pnum != mypnum);
	/// ASSERT: assert(pbSrc != NULL);
	/// ASSERT: assert(dwLen <= 0x0ffff);

	dwOffset = 0;

	while (dwLen != 0) {
		pkt.hdr.wCheck = PKT_HDR_CHECK;
		pkt.hdr.php = 0;
		//pkt.hdr.pmhp = 0;
		pkt.hdr.pmp = 0;
		//pkt.hdr.pmmp = 0;
		pkt.hdr.px = 0;
		pkt.hdr.py = 0;
		p = (TCmdPlrInfoHdr *)pkt.body;
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

static void multi_send_plrinfo_msg(int pnum, char cmd)
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
	sgbPlayerLeftGameTbl[pnum] = LeftReason;
	if (LeftReason == LEAVE_ENDING)
		gbSomebodyWonGameKludge = true;

	dthread_remove_player(pnum);

	if (gbDeltaSender == pnum)
		gbDeltaSender = MAX_PLRS;
}

void NetClose()
{
	if (!_gbNetInited) {
		return;
	}

	_gbNetInited = false;
	nthread_cleanup();
	dthread_cleanup();
	tmsg_cleanup();
	UIDisconnectGame();
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
			break;
		}

		// select game
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
		sgGameInitInfo.bDifficulty = DIFF_NORMAL;
		sgGameInitInfo.bTickRate = SPEED_NORMAL;
		sgGameInitInfo.bMaxPlayers = MAX_PLRS;
		gbJoinGame = false;
		gbGameDestroyed = false;
		static_assert(LEAVE_NONE == 0, "NetInit uses memset to reset the LEAVE_ enum values.");
		memset(sgbPlayerLeftGameTbl, 0, sizeof(sgbPlayerLeftGameTbl));
		guSendDelta = 0;
		memset(players, 0, sizeof(players));
		memset(sgwPackPlrOffsetTbl, 0, sizeof(sgwPackPlrOffsetTbl));
		if (!multi_init_game(bSinglePlayer, sgGameInitInfo))
			return false;
		_gbNetInited = true;
		_gbTimeout = false;
		delta_init();
		InitPlrMsg();
		buffer_init(&sgHiPriBuf);
		buffer_init(&sgLoPriBuf);
		gbPacketSentRecently = false;
		nthread_start(gbJoinGame);
		dthread_start();
		tmsg_start();
		sgdwGameLoops = 0;
		sgbSentThisCycle = 0;
		gbDeltaSender = mypnum;
		gbSomebodyWonGameKludge = false;
		nthread_send_turn(0, 0);
		SetupLocalPlr();
		if (!gbJoinGame)
			break;
		multi_send_plrinfo_msg(SNPLAYER_ALL, CMD_SEND_PLRINFO);
		if (DownloadDeltaInfo())
			break;
		NetClose();
	}
	assert(mypnum == sgGameInitInfo.bPlayerId);
	assert(gbMaxPlayers == sgGameInitInfo.bMaxPlayers);
	assert(gnTicksRate == sgGameInitInfo.bTickRate);
	assert(gnTickDelay == 1000 / gnTicksRate);
	assert(gnDifficulty == sgGameInitInfo.bDifficulty);
	return true;
}

void multi_recv_plrinfo_msg(int pnum, TCmdPlrInfoHdr *piHdr, bool recv)
{
	// assert(pnum != mypnum);
	/// ASSERT: assert((unsigned)pnum < MAX_PLRS);

	if (sgwPackPlrOffsetTbl[pnum] != piHdr->wOffset) {
		sgwPackPlrOffsetTbl[pnum] = 0;
		if (piHdr->wOffset != 0) {
			return;
		}
	}
	if (!recv && sgwPackPlrOffsetTbl[pnum] == 0) {
		multi_send_plrinfo_msg(pnum, CMD_ACK_PLRINFO);
	}

	memcpy((char *)&netplr[pnum] + piHdr->wOffset, &piHdr[1], piHdr->wBytes); /* todo: cast? */
	sgwPackPlrOffsetTbl[pnum] += piHdr->wBytes;
	if (sgwPackPlrOffsetTbl[pnum] != sizeof(*netplr)) {
		return;
	}

	sgwPackPlrOffsetTbl[pnum] = 0;
	multi_deactivate_player(pnum, LEAVE_NONE);
	// TODO: validate PkPlayerStruct coming from internet?
	UnPackPlayer(&netplr[pnum], pnum);
	if (!recv) {
		return;
	}

	assert(!plr.plractive);
	plr.plractive = TRUE;
	gbActivePlayers++;
	EventPlrMsg("Player '%s' (level %d) is already in the game", plr._pName, plr._pLevel);

	plr._pGFXLoad = 0;
	if (plr.plrlevel == currLvl._dLevelIdx) {
		SyncInitPlr(pnum);
		//PlrStartStand(pnum, DIR_S);
		/*LoadPlrGFX(pnum, PFILE_STAND);
		SyncInitPlr(pnum);
		if (plr._pHitPoints >= (1 << 6)) {
			PlrStartStand(pnum, DIR_S);
		} else {
			plr._pgfxnum = ANIM_ID_UNARMED;
			LoadPlrGFX(pnum, PFILE_DEATH);
			plr._pmode = PM_DEATH;
			NewPlrAnim(pnum, plr._pDAnim, DIR_S, plr._pDFrames, PlrAnimFrameLens[PA_DEATH], plr._pDWidth);
			plr._pAnimFrame = plr._pAnimLen - 1;
			plr._pVar8 = 2 * plr._pAnimLen; // DEATH_TICK
			plr._pVar7 = 0; // DEATH_DELAY
			dFlags[plr._px][plr._py] |= BFLAG_DEAD_PLAYER;
		}*/
	}
}

DEVILUTION_END_NAMESPACE
