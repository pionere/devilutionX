/**
 * @file multi.cpp
 *
 * Implementation of functions for keeping multiplaye games in sync.
 */
#include "all.h"
#include "diabloui.h"
#include <time.h>

DEVILUTION_BEGIN_NAMESPACE

#define PKT_HDR_CHECK	SDL_SwapBE16(*((WORD*)"ip"))

bool gbSomebodyWonGameKludge;
static TBuffer sgHiPriBuf;
static TBuffer sgLoPriBuf;
static WORD sgwPackPlrOffsetTbl[MAX_PLRS];
static PkPlayerStruct netplr[MAX_PLRS];
static bool gbJoinGame;
static bool sgbPlayerLeftGameTbl[MAX_PLRS];
static uint32_t sgbSentThisCycle;
static bool gbShouldValidatePackage;
BYTE gbActivePlayers;
bool gbGameDestroyed;
static BOOLEAN sgbSendDeltaTbl[MAX_PLRS];
static _SNETGAMEDATA sgGameInitInfo;
bool gbSelectProvider;
bool gbSelectHero;
static int sglTimeoutStart;
static int sgdwPlayerLeftReasonTbl[MAX_PLRS];
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
	pBuf->dwNextWriteOffset = 0;
	pBuf->bData[0] = 0;
}

// Microsoft VisualC 2-11/net runtime
static int multi_check_pkt_valid(TBuffer *pBuf)
{
	return pBuf->dwNextWriteOffset == 0;
}

static void multi_copy_packet(TBuffer *buf, void *packet, BYTE size)
{
	BYTE *p;

	if (buf->dwNextWriteOffset + size + 2 > 0x1000) {
		return;
	}

	p = &buf->bData[buf->dwNextWriteOffset];
	buf->dwNextWriteOffset += size + 1;
	*p = size;
	p++;
	memcpy(p, packet, size);
	p[size] = 0;
}

static BYTE *multi_recv_packet(TBuffer *pBuf, BYTE *body, unsigned *size)
{
	BYTE *src_ptr;
	size_t chunk_size;

	if (pBuf->dwNextWriteOffset != 0) {
		src_ptr = pBuf->bData;
		while (*src_ptr != 0) {
			chunk_size = *src_ptr;
			if (chunk_size > *size)
				break;
			src_ptr++;
			memcpy(body, src_ptr, chunk_size);
			body += chunk_size;
			src_ptr += chunk_size;
			*size -= chunk_size;
		}
		memcpy(pBuf->bData, src_ptr, (pBuf->bData - src_ptr) + pBuf->dwNextWriteOffset + 1);
		pBuf->dwNextWriteOffset += (pBuf->bData - src_ptr);
	}
	return body;
}

static void NetRecvPlrData(TPktHdr &pktHdr)
{
	PlayerStruct *p;

	p = &players[myplr];
	pktHdr.wCheck = PKT_HDR_CHECK;
	pktHdr.px = p->_px;
	pktHdr.py = p->_py;
	pktHdr.php = SwapLE32(p->_pHitPoints);
	pktHdr.pmhp = SwapLE32(p->_pMaxHP);
	pktHdr.pmp = SwapLE32(p->_pMana);
	pktHdr.pmmp = SwapLE32(p->_pMaxMana);
}

static void multi_send_packet(void *packet, BYTE dwSize)
{
	TPkt pkt;

	NetRecvPlrData(pkt.hdr);
	pkt.hdr.wLen = dwSize + sizeof(pkt.hdr);
	memcpy(pkt.body, packet, dwSize);
	SNetSendMessage(myplr, &pkt.hdr, pkt.hdr.wLen);
	//if (!SNetSendMessage(myplr, &pkt.hdr, pkt.hdr.wLen))
	//	nthread_terminate_game("SNetSendMessage0");
}

static void validate_package()
{
	BYTE *hipri_body;
	BYTE *lowpri_body;
	unsigned size, len;
	TPkt pkt;

	NetRecvPlrData(pkt.hdr);
	size = gdwNormalMsgSize - sizeof(TPktHdr);
	hipri_body = multi_recv_packet(&sgHiPriBuf, pkt.body, &size);
	lowpri_body = multi_recv_packet(&sgLoPriBuf, hipri_body, &size);
	size = sync_all_monsters(lowpri_body, size);
	len = gdwNormalMsgSize - size;
	pkt.hdr.wLen = SwapLE16(len);
	SNetSendMessage(SNPLAYER_OTHERS, &pkt.hdr, len);
	//if (!SNetSendMessage(SNPLAYER_OTHERS, &pkt.hdr, len))
	//	nthread_terminate_game("SNetSendMessage");
}

void NetSendLoPri(BYTE *pbMsg, BYTE bLen)
{
	multi_copy_packet(&sgLoPriBuf, pbMsg, bLen);
	multi_send_packet(pbMsg, bLen);
}

void NetSendHiPri(BYTE *pbMsg, BYTE bLen)
{
	multi_copy_packet(&sgHiPriBuf, pbMsg, bLen);
	multi_send_packet(pbMsg, bLen);

	if (!gbShouldValidatePackage) {
		gbShouldValidatePackage = true;
		validate_package();
	}
}

void multi_send_msg_packet(unsigned int pmask, BYTE *src, BYTE len)
{
	DWORD i, msglen;
	TPkt pkt;

	NetRecvPlrData(pkt.hdr);
	msglen = len + sizeof(pkt.hdr);
	pkt.hdr.wLen = SwapLE16(msglen);
	memcpy(pkt.body, src, len);
	for (i = 0; i < MAX_PLRS; i++, pmask >>= 1) {
		if (pmask & 1) {
			SNetSendMessage(i, &pkt.hdr, msglen);
			/*if (!SNetSendMessage(i, &pkt.hdr, msglen) && SErrGetLastError() != STORM_ERROR_INVALID_PLAYER) {
				nthread_terminate_game("SNetSendMessage");
				return;
			}*/
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

static void multi_handle_turn_upper_bit(int pnum)
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if ((player_state[i] & PS_CONNECTED) && i != pnum)
			break;
	}

	if (myplr == i) {
		sgbSendDeltaTbl[pnum] = TRUE;
	} else if (myplr == pnum) {
		gbDeltaSender = i;
	}
}

static void multi_parse_turn(int pnum, uint32_t turn)
{
	uint32_t absTurns;

	if (turn >> 31)
		multi_handle_turn_upper_bit(pnum);
	absTurns = turn & 0x7FFFFFFF;
	if (sgbSentThisCycle < gdwTurnsInTransit + absTurns) {
		if (absTurns >= 0x7FFFFFFF)
			absTurns &= 0xFFFF;
		sgbSentThisCycle = absTurns + gdwTurnsInTransit;
		sgdwGameLoops = 4 * absTurns * sgbNetUpdateRate;
	}
}

void multi_msg_countdown()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (player_state[i] & PS_TURN_ARRIVED) {
			multi_parse_turn(i, *glpMsgTbl[i]);
		}
	}
}

/**
 * Re-assign players from a team to the first available one.
 * @param pnum the team to disband
 */
void multi_disband_team(int pnum)
{
	int i, team;

	team = -1;
	for (i = 0; i < MAX_PLRS; i++) {
		if (i == pnum || players[i]._pTeam != pnum)
			continue;
		if (team == -1) {
			team = i;
		}
		players[i]._pTeam = team;
	}
}

static void multi_player_left_msg(int pnum, bool left)
{
	const char *pszFmt;

	if (players[pnum].plractive) {
		RemovePortalMissile(pnum);
		DeactivatePortal(pnum);
		delta_close_portal(pnum);
		multi_disband_team(pnum);
		if (players[pnum].plrlevel == currLvl._dLevelIdx) {
			AddUnLight(players[pnum]._plid);
			AddUnVision(players[pnum]._pvid);
			RemovePlrFromMap(pnum);
			RemovePlrMissiles(pnum);
		}
		if (left) {
			pszFmt = "Player '%s' just left the game";
			switch (sgdwPlayerLeftReasonTbl[pnum]) {
			case LEAVE_ENDING:
				pszFmt = "Player '%s' killed Diablo and left the game!";
				gbSomebodyWonGameKludge = true;
				break;
			case LEAVE_DROP:
				pszFmt = "Player '%s' dropped due to timeout";
				break;
			}
			EventPlrMsg(pszFmt, players[pnum]._pName);
		}
		players[pnum].plractive = FALSE;
		players[pnum]._pName[0] = '\0';
		guTeamInviteRec &= ~(1 << pnum);
		guTeamInviteSent &= ~(1 << pnum);
		guTeamMute &= ~(1 << pnum);
		gbActivePlayers--;
	}
}

static void multi_clear_left_tbl()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (sgbPlayerLeftGameTbl[i]) {
			if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
				msg_send_drop_pkt(i, sgdwPlayerLeftReasonTbl[i]);
			else
				multi_player_left_msg(i, true);

			sgbPlayerLeftGameTbl[i] = FALSE;
			sgdwPlayerLeftReasonTbl[i] = 0;
		}
	}
}

void multi_player_left(int pnum, int reason)
{
	sgbPlayerLeftGameTbl[pnum] = true;
	sgdwPlayerLeftReasonTbl[pnum] = reason;
	multi_clear_left_tbl();
}

void multi_net_ping()
{
	_gbTimeout = true;
	sglTimeoutStart = SDL_GetTicks();
}

static void multi_check_drop_player()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (!(player_state[i] & PS_ACTIVE) && player_state[i] & PS_CONNECTED) {
			SNetDropPlayer(i);
		}
	}
}

static void multi_begin_timeout()
{
	int i, nTicks, nState, nLowestActive, nLowestPlayer;
	BYTE bGroupPlayers, bGroupCount;

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
	bGroupPlayers = 0;
	bGroupCount = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		nState = player_state[i];
		if (nState & PS_CONNECTED) {
			if (nLowestPlayer == -1) {
				nLowestPlayer = i;
			}
			if (nState & PS_ACTIVE) {
				bGroupPlayers++;
				if (nLowestActive == -1) {
					nLowestActive = i;
				}
			} else {
				bGroupCount++;
			}
		}
	}

	/// ASSERT: assert(bGroupPlayers != 0);
	/// ASSERT: assert(nLowestActive != -1);
	/// ASSERT: assert(nLowestPlayer != -1);

	if (bGroupPlayers < bGroupCount) {
		gbGameDestroyed = true;
	} else if (bGroupPlayers == bGroupCount) {
		if (nLowestPlayer != nLowestActive) {
			gbGameDestroyed = true;
		} else if (nLowestActive == myplr) {
			multi_check_drop_player();
		}
	} else if (nLowestActive == myplr) {
		multi_check_drop_player();
	}
}

/**
 * @return Always true for singleplayer
 */
bool multi_handle_delta()
{
	int i;
	bool received;

	if (gbGameDestroyed) {
		gbRunGame = false;
		return false;
	}

	for (i = 0; i < MAX_PLRS; i++) {
		if (sgbSendDeltaTbl[i]) {
			sgbSendDeltaTbl[i] = FALSE;
			DeltaExportData(i);
		}
	}

	sgbSentThisCycle = nthread_send_and_recv_turn(sgbSentThisCycle, 1);
	if (!nthread_recv_turns(&received)) {
		multi_begin_timeout();
		return false;
	}

	_gbTimeout = false;
	if (received) {
		if (!gbShouldValidatePackage) {
			gbShouldValidatePackage = true;
			validate_package();
			gbShouldValidatePackage = false;
		} else {
			gbShouldValidatePackage = false;
			if (!multi_check_pkt_valid(&sgHiPriBuf)) {
				gbShouldValidatePackage = true;
				validate_package();
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
		multi_handle_all_packets(myplr, (BYTE *)&pkt, sizeof(pkt));
	}
}

void multi_process_network_packets()
{
	int dx, dy;
	TPktHdr *pkt;
	unsigned dwMsgSize;
	int pnum;
	PlayerStruct *p;

	multi_clear_left_tbl();
	multi_process_tmsgs();
	while (SNetReceiveMessage(&pnum, (char **)&pkt, &dwMsgSize)) {
		multi_clear_left_tbl();
		if (dwMsgSize < sizeof(TPktHdr))
			continue;
		if ((unsigned)pnum >= MAX_PLRS)
			continue;
		if (pkt->wCheck != PKT_HDR_CHECK)
			continue;
		if (pkt->wLen != SwapLE16(dwMsgSize))
			continue;
		p = &players[pnum];
		if (pnum != myplr) {
			// ASSERT: assert(geBufferMsgs != MSG_RUN_DELTA);
			p->_pHitPoints = SwapLE32(pkt->php);
			p->_pMaxHP = SwapLE32(pkt->pmhp);
			p->_pMana = SwapLE32(pkt->pmp);
			p->_pMaxMana = SwapLE32(pkt->pmmp);
			if (geBufferMsgs != MSG_DOWNLOAD_DELTA && p->plractive && p->_pHitPoints >= (1 << 6)) {
				if (currLvl._dLevelIdx == p->plrlevel && !p->_pLvlChanging) {
					dx = abs(p->_px - pkt->px);
					dy = abs(p->_py - pkt->py);
					if ((dx > 3 || dy > 3) && dPlayer[pkt->px][pkt->py] == 0) {
						RemovePlrFromMap(pnum);
						SetPlayerOld(p);
						RemovePlrFromMap(pnum);
						p->_px = pkt->px;
						p->_py = pkt->py;
						p->_pfutx = pkt->px;
						p->_pfuty = pkt->py;
						dPlayer[p->_px][p->_py] = pnum + 1;
					}
					dx = abs(p->_pfutx - p->_px);
					dy = abs(p->_pfuty - p->_py);
					if (dx > 1 || dy > 1) {
						p->_pfutx = p->_px;
						p->_pfuty = p->_py;
					}
				} else {
					p->_px = pkt->px;
					p->_py = pkt->py;
					p->_pfutx = pkt->px;
					p->_pfuty = pkt->py;
				}
			}
		}
		multi_handle_all_packets(pnum, (BYTE *)(pkt + 1), dwMsgSize - sizeof(TPktHdr));
	}
	if (SErrGetLastError() != STORM_ERROR_NO_MESSAGES_WAITING)
		nthread_terminate_game("SNetReceiveMsg");
}

void multi_send_zero_packet(int pnum, BYTE bCmd, BYTE *pbSrc, DWORD dwLen)
{
	unsigned dwOffset, dwBody, dwMsg;
	TPkt pkt;
	TCmdPlrInfoHdr *p;

	/// ASSERT: assert(pnum != myplr);
	/// ASSERT: assert(pbSrc != NULL);
	/// ASSERT: assert(dwLen <= 0x0ffff);

	dwOffset = 0;

	while (dwLen != 0) {
		pkt.hdr.wCheck = PKT_HDR_CHECK;
		pkt.hdr.php = 0;
		pkt.hdr.pmhp = 0;
		pkt.hdr.pmp = 0;
		pkt.hdr.pmmp = 0;
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
		SNetSendMessage(pnum, &pkt, dwMsg);
		/*if (!SNetSendMessage(pnum, &pkt, dwMsg)) {
			nthread_terminate_game("SNetSendMessage2");
			return;
		}*/
		pbSrc += dwBody;
		dwLen -= dwBody;
		dwOffset += dwBody;
	}
}

static void multi_send_pinfo(int pnum, char cmd)
{
	PkPlayerStruct pkplr;

	PackPlayer(&pkplr, myplr);
	dthread_send_delta(pnum, cmd, &pkplr, sizeof(pkplr));
}

static void SetupLocalCoords()
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
	x += plrxoff[myplr];
	y += plryoff[myplr];
	p = &players[myplr];
	p->_px = x;
	p->_py = y;
	p->_pfutx = x;
	p->_pfuty = y;
	p->plrlevel = currLvl._dLevelIdx;
	p->_pLvlChanging = TRUE;
	p->_pmode = PM_NEWLVL;
	p->destAction = ACTION_NONE;
	lvlLoad = 10;
}

static void multi_handle_events(_SNETEVENT *pEvt)
{
	assert(pEvt->eventid == EVENT_TYPE_PLAYER_LEAVE_GAME);

	DWORD LeftReason;
	sgbPlayerLeftGameTbl[pEvt->playerid] = true;

	LeftReason = 0;
	if (pEvt->_eData != NULL && pEvt->databytes >= sizeof(DWORD))
		LeftReason = *(DWORD *)pEvt->_eData;
	sgdwPlayerLeftReasonTbl[pEvt->playerid] = LeftReason;
	if (LeftReason == LEAVE_ENDING)
		gbSomebodyWonGameKludge = true;

	sgbSendDeltaTbl[pEvt->playerid] = FALSE;
	dthread_remove_player(pEvt->playerid);

	if (gbDeltaSender == pEvt->playerid)
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

static bool multi_init_game(bool bSinglePlayer)
{
	int dlgresult, playerId;

	while (TRUE) {
		// myplr = 0;

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
			// myplr = 0;
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
			playerId = sgGameInitInfo.bPlayerId;
			if (myplr != playerId) {
				copy_pod(players[playerId], players[myplr]);
				myplr = playerId;
				players[myplr]._pTeam = myplr;
				//pfile_read_player_from_save();
			}
			gbJoinGame = true;
		}
		break;
	}

	gbMaxPlayers = sgGameInitInfo.bMaxPlayers;
	return true;
}

bool NetInit(bool bSinglePlayer)
{
	int i;

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
		memset(sgbPlayerLeftGameTbl, 0, sizeof(sgbPlayerLeftGameTbl));
		memset(sgdwPlayerLeftReasonTbl, 0, sizeof(sgdwPlayerLeftReasonTbl));
		memset(sgbSendDeltaTbl, 0, sizeof(sgbSendDeltaTbl));
		memset(players, 0, sizeof(players));
		memset(sgwPackPlrOffsetTbl, 0, sizeof(sgwPackPlrOffsetTbl));
		if (!multi_init_game(bSinglePlayer))
			return false;
		_gbNetInited = true;
		_gbTimeout = false;
		delta_init();
		InitPlrMsg();
		buffer_init(&sgHiPriBuf);
		buffer_init(&sgLoPriBuf);
		gbShouldValidatePackage = false;
		sync_init();
		nthread_start(gbJoinGame);
		dthread_start();
		tmsg_start();
		sgdwGameLoops = 0;
		sgbSentThisCycle = 0;
		gbDeltaSender = myplr;
		gbSomebodyWonGameKludge = false;
		nthread_send_and_recv_turn(0, 0);
		SetupLocalCoords();
		if (!bSinglePlayer)
			multi_send_pinfo(-2, CMD_SEND_PLRINFO);
		gbActivePlayers = 1;
		players[myplr].plractive = TRUE;
		assert(players[myplr]._pTeam == myplr);
		if (!gbJoinGame || msg_wait_resync())
			break;
		NetClose();
	}
	assert(myplr == sgGameInitInfo.bPlayerId);
	assert(gbMaxPlayers == sgGameInitInfo.bMaxPlayers);
	gnDifficulty = sgGameInitInfo.bDifficulty;
	gnTicksRate = sgGameInitInfo.bTickRate;
	gnTickDelay = 1000 / gnTicksRate;
	SetRndSeed(sgGameInitInfo.dwSeed);

	for (i = 0; i < NUMLEVELS + NUM_SETLVL; i++) {
		glSeedTbl[i] = GetRndSeed();
	}
	SNetGetGameInfo(&szGameName, &szGamePassword);
	return true;
}

void recv_plrinfo(int pnum, TCmdPlrInfoHdr *piHdr, bool recv)
{
	PlayerStruct* p;

	// assert(myplr != pnum);
	/// ASSERT: assert((unsigned)pnum < MAX_PLRS);

	if (sgwPackPlrOffsetTbl[pnum] != piHdr->wOffset) {
		sgwPackPlrOffsetTbl[pnum] = 0;
		if (piHdr->wOffset != 0) {
			return;
		}
	}
	if (!recv && sgwPackPlrOffsetTbl[pnum] == 0) {
		multi_send_pinfo(pnum, CMD_ACK_PLRINFO);
	}

	memcpy((char *)&netplr[pnum] + piHdr->wOffset, &piHdr[1], piHdr->wBytes); /* todo: cast? */
	sgwPackPlrOffsetTbl[pnum] += piHdr->wBytes;
	if (sgwPackPlrOffsetTbl[pnum] != sizeof(*netplr)) {
		return;
	}

	sgwPackPlrOffsetTbl[pnum] = 0;
	multi_player_left_msg(pnum, false);
	// TODO: validate PkPlayerStruct coming from internet?
	UnPackPlayer(&netplr[pnum], pnum);
	if (!recv) {
		return;
	}

	p = &players[pnum];
	assert(!p->plractive);
	p->plractive = TRUE;
	gbActivePlayers++;
	EventPlrMsg("Player '%s' (level %d) is already in the game", p->_pName, p->_pLevel);

	p->_pGFXLoad = 0;
	if (p->plrlevel == currLvl._dLevelIdx) {
		SyncInitPlr(pnum);
		//PlrStartStand(pnum, DIR_S);
		/*LoadPlrGFX(pnum, PFILE_STAND);
		SyncInitPlr(pnum);
		if (players[pnum]._pHitPoints >= (1 << 6)) {
			PlrStartStand(pnum, DIR_S);
		} else {
			players[pnum]._pgfxnum = ANIM_ID_UNARMED;
			LoadPlrGFX(pnum, PFILE_DEATH);
			players[pnum]._pmode = PM_DEATH;
			NewPlrAnim(pnum, players[pnum]._pDAnim, DIR_S, players[pnum]._pDFrames, PlrAnimFrameLens[PA_DEATH], players[pnum]._pDWidth);
			players[pnum]._pAnimFrame = players[pnum]._pAnimLen - 1;
			players[pnum]._pVar8 = 2 * players[pnum]._pAnimLen; // DEATH_TICK
			players[pnum]._pVar7 = 0; // DEATH_DELAY
			dFlags[players[pnum]._px][players[pnum]._py] |= BFLAG_DEAD_PLAYER;
		}*/
	}
}

DEVILUTION_END_NAMESPACE
