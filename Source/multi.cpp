/**
 * @file multi.cpp
 *
 * Implementation of functions for keeping multiplaye games in sync.
 */
#include "all.h"
#include "diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

bool gbSomebodyWonGameKludge;
TBuffer sgHiPriBuf;
char szPlayerDescript[128];
WORD sgwPackPlrOffsetTbl[MAX_PLRS];
PkPlayerStruct netplr[MAX_PLRS];
bool gbJoinGame;
bool sgbPlayerLeftGameTbl[MAX_PLRS];
DWORD sgbSentThisCycle;
bool gbShouldValidatePackage;
BYTE gbActivePlayers;
bool gbGameDestroyed;
BOOLEAN sgbSendDeltaTbl[MAX_PLRS];
_SNETGAMEDATA sgGameInitInfo;
bool gbSelectProvider;
bool gbSelectHero;
int sglTimeoutStart;
int sgdwPlayerLeftReasonTbl[MAX_PLRS];
TBuffer sgLoPriBuf;
DWORD sgdwGameLoops;
/**
 * Specifies the maximum number of players in a game, where 1
 * represents a single player game and 4 represents a multi player game.
 */
BYTE gbMaxPlayers;
bool _gbTimeout;
BYTE gbDeltaSender;
bool _gbNetInited;
char szPlayerName[128];
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

static BYTE *multi_recv_packet(TBuffer *pBuf, BYTE *body, DWORD *size)
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

	p = &plr[myplr];
	pktHdr.wCheck = 'ip';
	pktHdr.px = p->_px;
	pktHdr.py = p->_py;
	pktHdr.php = p->_pHitPoints;
	pktHdr.pmhp = p->_pMaxHP;
	pktHdr.pmp = p->_pMana;
	pktHdr.pmmp = p->_pMaxMana;
}

static void multi_send_packet(void *packet, BYTE dwSize)
{
	TPkt pkt;

	NetRecvPlrData(pkt.hdr);
	pkt.hdr.wLen = dwSize + sizeof(pkt.hdr);
	memcpy(pkt.body, packet, dwSize);
	if (!SNetSendMessage(myplr, &pkt.hdr, pkt.hdr.wLen))
		nthread_terminate_game("SNetSendMessage0");
}

static void validate_package()
{
	BYTE *hipri_body;
	BYTE *lowpri_body;
	DWORD size, len;
	TPkt pkt;

	NetRecvPlrData(pkt.hdr);
	size = gdwNormalMsgSize - sizeof(TPktHdr);
	hipri_body = multi_recv_packet(&sgHiPriBuf, pkt.body, &size);
	lowpri_body = multi_recv_packet(&sgLoPriBuf, hipri_body, &size);
	size = sync_all_monsters(lowpri_body, size);
	len = gdwNormalMsgSize - size;
	pkt.hdr.wLen = len;
	if (!SNetSendMessage(-2, &pkt.hdr, len))
		nthread_terminate_game("SNetSendMessage");
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
	pkt.hdr.wLen = msglen;
	memcpy(pkt.body, src, len);
	for (i = 0; i < MAX_PLRS; i++, pmask >>= 1) {
		if (pmask & 1) {
			if (!SNetSendMessage(i, &pkt.hdr, msglen) && SErrGetLastError() != STORM_ERROR_INVALID_PLAYER) {
				nthread_terminate_game("SNetSendMessage");
				return;
			}
		}
	}
}

static void multi_mon_seeds()
{
	int i;
	DWORD l;

	sgdwGameLoops++;
	l = (sgdwGameLoops >> 8) | (sgdwGameLoops << 24); // _rotr(sgdwGameLoops, 8)
	for (i = 0; i < MAXMONSTERS; i++)
		monster[i]._mAISeed = l + i;
}

static void multi_handle_turn_upper_bit(int pnum)
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (player_state[i] & PS_CONNECTED && i != pnum)
			break;
	}

	if (myplr == i) {
		sgbSendDeltaTbl[pnum] = TRUE;
	} else if (myplr == pnum) {
		gbDeltaSender = i;
	}
}

static void multi_parse_turn(int pnum, int turn)
{
	DWORD absTurns;

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
			if (gdwMsgLenTbl[i] == 4)
				multi_parse_turn(i, *glpMsgTbl[i]);
		}
	}
}

static void multi_player_left_msg(int pnum, bool left)
{
	const char *pszFmt;

	if (plr[pnum].plractive) {
		RemovePortalMissile(pnum);
		DeactivatePortal(pnum);
		delta_close_portal(pnum);
		if (plr[pnum].plrlevel == currlevel) {
			AddUnLight(plr[pnum]._plid);
			AddUnVision(plr[pnum]._pvid);
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
			EventPlrMsg(pszFmt, plr[pnum]._pName);
		}
		plr[pnum].plractive = FALSE;
		plr[pnum]._pName[0] = '\0';
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
	sgbPlayerLeftGameTbl[pnum] = TRUE;
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
			SNetDropPlayer(i, LEAVE_DROP);
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
	int cnt;
	TPkt pkt;

	while (cnt = tmsg_get(&pkt)) {
		multi_handle_all_packets(myplr, (BYTE *)&pkt, cnt);
	}
}

void multi_process_network_packets()
{
	int dx, dy;
	TPktHdr *pkt;
	DWORD dwMsgSize;
	int pnum;
	PlayerStruct *p;

	multi_clear_left_tbl();
	multi_process_tmsgs();
	while (SNetReceiveMessage(&pnum, (char **)&pkt, (int *)&dwMsgSize)) {
		multi_clear_left_tbl();
		if (dwMsgSize < sizeof(TPktHdr))
			continue;
		if ((unsigned)pnum >= MAX_PLRS)
			continue;
		if (pkt->wCheck != 'ip')
			continue;
		if (pkt->wLen != dwMsgSize)
			continue;
		p = &plr[pnum];
		if (pnum != myplr) {
			// ASSERT: assert(geBufferMsgs != MSG_RUN_DELTA);
			p->_pHitPoints = pkt->php;
			p->_pMaxHP = pkt->pmhp;
			p->_pMana = pkt->pmp;
			p->_pMaxMana = pkt->pmmp;
			if (geBufferMsgs != MSG_DOWNLOAD_DELTA && p->plractive && p->_pHitPoints >= (1 << 6)) {
				if (currlevel == p->plrlevel && !p->_pLvlChanging) {
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
	DWORD dwOffset, dwBody, dwMsg;
	TPkt pkt;
	TCmdPlrInfoHdr *p;

	/// ASSERT: assert(pnum != myplr);
	/// ASSERT: assert(pbSrc != NULL);
	/// ASSERT: assert(dwLen <= 0x0ffff);

	dwOffset = 0;

	while (dwLen != 0) {
		pkt.hdr.wCheck = 'ip';
		pkt.hdr.php = 0;
		pkt.hdr.pmhp = 0;
		pkt.hdr.pmp = 0;
		pkt.hdr.pmmp = 0;
		pkt.hdr.px = 0;
		pkt.hdr.py = 0;
		p = (TCmdPlrInfoHdr *)pkt.body;
		p->bCmd = bCmd;
		p->wOffset = dwOffset;
		dwBody = gdwLargestMsgSize - sizeof(pkt.hdr) - sizeof(*p);
		if (dwLen < dwBody) {
			dwBody = dwLen;
		}
		/// ASSERT: assert(dwBody <= 0x0ffff);
		p->wBytes = dwBody;
		memcpy(&pkt.body[sizeof(*p)], pbSrc, p->wBytes);
		dwMsg = sizeof(pkt.hdr);
		dwMsg += sizeof(*p);
		dwMsg += p->wBytes;
		pkt.hdr.wLen = dwMsg;
		if (!SNetSendMessage(pnum, &pkt, dwMsg)) {
			nthread_terminate_game("SNetSendMessage2");
			return;
		}
		pbSrc += p->wBytes;
		dwLen -= p->wBytes;
		dwOffset += p->wBytes;
	}
}

static void multi_send_pinfo(int pnum, char cmd)
{
	PkPlayerStruct pkplr;

	PackPlayer(&pkplr, myplr);
	dthread_send_delta(pnum, cmd, &pkplr, sizeof(pkplr));
}

static int InitLevelType(int l)
{
	if (l == 0)
		return DTYPE_TOWN;
	if (l <= 4)
		return DTYPE_CATHEDRAL;
	if (l <= 8)
		return DTYPE_CATACOMBS;
	if (l <= 12)
		return DTYPE_CAVES;

#ifdef HELLFIRE
	if (l <= 16)
		return DTYPE_HELL;
	if (l <= 20)
		return DTYPE_CAVES; // Hive
	if (l <= 24)
		return DTYPE_CATHEDRAL; // Crypt

	return DTYPE_CATHEDRAL;
#else
	return DTYPE_HELL;
#endif
}

static void SetupLocalCoords()
{
	PlayerStruct *p;
	int x, y;

	x = 65 + DBORDERX;
	y = 58 + DBORDERY;
#ifdef _DEBUG
	if (debug_mode_key_inverted_v || debug_mode_key_d) {
		x = 39 + DBORDERX;
		y = 13 + DBORDERY;
	}
	if (!leveldebug || gbMaxPlayers != 1) {
		currlevel = 0;
		leveltype = DTYPE_TOWN;
		gbSetlevel = false;
	}
#else
	currlevel = 0;
	leveltype = DTYPE_TOWN;
	gbSetlevel = false;
#endif
	x += plrxoff[myplr];
	y += plryoff[myplr];
	p = &plr[myplr];
	p->_px = x;
	p->_py = y;
	p->_pfutx = x;
	p->_pfuty = y;
	p->plrlevel = currlevel;
	p->_pLvlChanging = TRUE;
	p->pLvlLoad = 0;
	p->_pmode = PM_NEWLVL;
	p->destAction = ACTION_NONE;
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
		pfile_create_player_description(NULL, NULL);

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
				copy_pod(plr[playerId], plr[myplr]);
				myplr = playerId;
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
		memset(plr, 0, sizeof(plr));
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
		multi_send_pinfo(-2, CMD_SEND_PLRINFO);
		gbActivePlayers = 1;
		plr[myplr].plractive = TRUE;
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

	for (i = 0; i < NUMLEVELS; i++) {
		glSeedTbl[i] = GetRndSeed();
		gnLevelTypeTbl[i] = InitLevelType(i);
	}
	SNetGetGameInfo(GAMEINFO_NAME, szPlayerName, sizeof(szPlayerName));
	SNetGetGameInfo(GAMEINFO_PASSWORD, szPlayerDescript, sizeof(szPlayerDescript));

	return true;
}

void recv_plrinfo(int pnum, TCmdPlrInfoHdr *piHdr, bool recv)
{
	PlayerStruct* p;

	if (myplr == pnum) {
		return;
	}
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
	UnPackPlayer(&netplr[pnum], pnum);
	if (!recv) {
		return;
	}

	p = &plr[pnum];
	assert(!p->plractive);
	p->plractive = TRUE;
	gbActivePlayers++;
	EventPlrMsg("Player '%s' (level %d) is already in the game", p->_pName, p->_pLevel);

	p->_pGFXLoad = 0;
	if (p->plrlevel == currlevel) {
		SyncInitPlr(pnum);
		//PlrStartStand(pnum, DIR_S);
		/*LoadPlrGFX(pnum, PFILE_STAND);
		SyncInitPlr(pnum);
		if (plr[pnum]._pHitPoints >= (1 << 6)) {
			PlrStartStand(pnum, DIR_S);
		} else {
			plr[pnum]._pgfxnum = ANIM_ID_UNARMED;
			LoadPlrGFX(pnum, PFILE_DEATH);
			plr[pnum]._pmode = PM_DEATH;
			NewPlrAnim(pnum, plr[pnum]._pDAnim, DIR_S, plr[pnum]._pDFrames, 1, plr[pnum]._pDWidth);
			plr[pnum]._pAnimFrame = plr[pnum]._pAnimLen - 1;
			plr[pnum]._pVar8 = 2 * plr[pnum]._pAnimLen;
			dFlags[plr[pnum]._px][plr[pnum]._py] |= BFLAG_DEAD_PLAYER;
		}*/
	}
}

DEVILUTION_END_NAMESPACE
