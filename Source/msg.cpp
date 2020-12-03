/**
 * @file msg.cpp
 *
 * Implementation of function for sending and reciving network messages.
 */
#include "all.h"
#include "diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

static DWORD sgdwOwnerWait;
static DWORD sgdwRecvOffset;
static int sgnCurrMegaPlayer;
static DLevel sgLevels[NUMLEVELS];
static BYTE sbLastCmd;
static TMegaPkt *sgpCurrPkt;
static BYTE sgRecvBuf[sizeof(DLevel) + 1];
static BYTE sgbRecvCmd;
static LocalLevel sgLocals[NUMLEVELS];
static DJunk sgJunk;
static TMegaPkt *sgpMegaPkt;
static BOOLEAN sgbDeltaChanged;
static BYTE sgbDeltaChunks;
BOOL deltaload;
BYTE gbBufferMsgs;
char gbNetMsg[MAX_SEND_STR_LEN];
int dwRecCount;

static void msg_get_next_packet()
{
	TMegaPkt *result;

	sgpCurrPkt = (TMegaPkt *)DiabloAllocPtr(sizeof(TMegaPkt));
	sgpCurrPkt->pNext = NULL;
	sgpCurrPkt->dwSpaceLeft = sizeof(result->data);

	result = (TMegaPkt *)&sgpMegaPkt;
	while (result->pNext != NULL) {
		result = result->pNext;
	}
	result->pNext = sgpCurrPkt;
}

static void msg_free_packets()
{
	while (sgpMegaPkt != NULL) {
		sgpCurrPkt = sgpMegaPkt->pNext;
		MemFreeDbg(sgpMegaPkt);
		sgpMegaPkt = sgpCurrPkt;
	}
}

static void msg_pre_packet()
{
	int i;
	int spaceLeft, pktSize;
	TMegaPkt *pkt;
	TFakeCmdPlr *cmd, *tmpCmd;
	TFakeDropPlr *dropCmd;

	i = -1;
	for (pkt = sgpMegaPkt; pkt != NULL; pkt = pkt->pNext) {
		spaceLeft = sizeof(pkt->data);
		cmd = (TFakeCmdPlr *)pkt->data;
		while (spaceLeft != pkt->dwSpaceLeft) {
			if (cmd->bCmd == FAKE_CMD_SETID) {
				tmpCmd = cmd;
				cmd++;
				i = tmpCmd->bPlr;
				spaceLeft -= sizeof(*cmd);
			} else if (cmd->bCmd == FAKE_CMD_DROPID) {
				dropCmd = (TFakeDropPlr *)cmd;
				cmd += 3;
				spaceLeft -= sizeof(*dropCmd);
				multi_player_left(dropCmd->bPlr, dropCmd->dwReason);
			} else {
				pktSize = ParseCmd(i, (TCmd *)cmd);
				cmd = (TFakeCmdPlr *)((char *)cmd + pktSize);
				spaceLeft -= pktSize;
			}
		}
	}
}

static void msg_send_packet(int pnum, const void *packet, DWORD dwSize)
{
	TMegaPkt *packeta;
	TFakeCmdPlr cmd;

	if (pnum != sgnCurrMegaPlayer) {
		sgnCurrMegaPlayer = pnum;
		cmd.bCmd = FAKE_CMD_SETID;
		cmd.bPlr = pnum;
		msg_send_packet(pnum, &cmd, sizeof(cmd));
	}
	packeta = sgpCurrPkt;
	if (sgpCurrPkt->dwSpaceLeft < dwSize) {
		msg_get_next_packet();
		packeta = sgpCurrPkt;
	}
	memcpy((char *)&packeta[1] - packeta->dwSpaceLeft, packet, dwSize);
	sgpCurrPkt->dwSpaceLeft -= dwSize;
}

void msg_send_drop_pkt(int pnum, int reason)
{
	TFakeDropPlr cmd;

	cmd.dwReason = reason;
	cmd.bCmd = FAKE_CMD_DROPID;
	cmd.bPlr = pnum;
	msg_send_packet(pnum, &cmd, sizeof(cmd));
}

static int msg_wait_for_turns()
{
	BOOL received;
	DWORD turns;

	if (sgbDeltaChunks == 0) {
		nthread_send_and_recv_turn(0, 0);
		if (!SNetGetOwnerTurnsWaiting(&turns) && SErrGetLastError() == STORM_ERROR_NOT_IN_GAME)
			return 100;
		if (SDL_GetTicks() - sgdwOwnerWait <= 2000 && turns < gdwTurnsInTransit)
			return 0;
		sgbDeltaChunks++;
	}
	multi_process_network_packets();
	nthread_send_and_recv_turn(0, 0);
	if (nthread_has_500ms_passed(FALSE))
		nthread_recv_turns(&received);

	if (gbGameDestroyed)
		return 100;
	if (gbDeltaSender >= MAX_PLRS) {
		sgbDeltaChunks = 0;
		sgbRecvCmd = CMD_DLEVEL_END;
		gbDeltaSender = myplr;
		nthread_set_turn_upper_bit();
	}
	if (sgbDeltaChunks == MAX_CHUNKS - 1) {
		sgbDeltaChunks = MAX_CHUNKS;
		return 99;
	}
	return 100 * sgbDeltaChunks / MAX_CHUNKS;
}

BOOL msg_wait_resync()
{
	BOOL success;

	msg_get_next_packet();
	sgbDeltaChunks = 0;
	sgnCurrMegaPlayer = -1;
	sgbRecvCmd = CMD_DLEVEL_END;
	gbBufferMsgs = 1;
	sgdwOwnerWait = SDL_GetTicks();
	success = UiProgressDialog("Waiting for game data...", 1, msg_wait_for_turns, 20);
	gbBufferMsgs = 0;
	if (!success) {
		msg_free_packets();
		return FALSE;
	}

	if (gbGameDestroyed) {
		DrawDlg("The game ended");
		msg_free_packets();
		return FALSE;
	}

	if (sgbDeltaChunks != MAX_CHUNKS) {
		DrawDlg("Unable to get level data");
		msg_free_packets();
		return FALSE;
	}

	return TRUE;
}

void run_delta_info()
{
	if (gbMaxPlayers != 1) {
		gbBufferMsgs = 2;
		msg_pre_packet();
		gbBufferMsgs = 0;
		msg_free_packets();
	}
}

static BYTE *DeltaExportItem(BYTE *dst, TCmdPItem *src)
{
	int i;

	for (i = 0; i < MAXITEMS; i++) {
		if (src->bCmd == 0xFF) {
			*dst = 0xFF;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<TCmdPItem *>(dst), *src);
			dst += sizeof(TCmdPItem);
		}
		src++;
	}

	return dst;
}

static BYTE *DeltaImportItem(BYTE *src, TCmdPItem *dst)
{
	int i;

	for (i = 0; i < MAXITEMS; i++) {
		if (*src == 0xFF) {
			memset(dst, 0xFF, sizeof(TCmdPItem));
			src++;
		} else {
			copy_pod(*dst, *reinterpret_cast<TCmdPItem *>(src));
			src += sizeof(TCmdPItem);
		}
		dst++;
	}

	return src;
}

static BYTE *DeltaExportObject(BYTE *dst, DObjectStr *src)
{
	memcpy(dst, src, sizeof(DObjectStr) * MAXOBJECTS);
	return dst + sizeof(DObjectStr) * MAXOBJECTS;
}

static BYTE *DeltaImportObject(BYTE *src, DObjectStr *dst)
{
	memcpy(dst, src, sizeof(DObjectStr) * MAXOBJECTS);
	return src + sizeof(DObjectStr) * MAXOBJECTS;
}

static BYTE *DeltaExportMonster(BYTE *dst, DMonsterStr *src)
{
	int i;

	for (i = 0; i < MAXMONSTERS; i++) {
		if (src->_mx == 0xFF) {
			*dst = 0xFF;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DMonsterStr *>(dst), *src);
			dst += sizeof(DMonsterStr);
		}
		src++;
	}

	return dst;
}

static BYTE *DeltaImportMonster(BYTE *src, DMonsterStr *dst)
{
	int i;

	for (i = 0; i < MAXMONSTERS; i++) {
		if (*src == 0xFF) {
			memset(dst, 0xFF, sizeof(DMonsterStr));
			src++;
		} else {
			copy_pod(*dst, *reinterpret_cast<DMonsterStr *>(src));
			src += sizeof(DMonsterStr);
		}
		dst++;
	}

	return src;
}

static BYTE *DeltaExportJunk(BYTE *dst)
{
	DPortal *pD;
	MultiQuests *mq;
	int i;

	pD = sgJunk.portal;
	for (i = 0; i < MAXPORTAL; i++, pD++) {
		if (pD->x == 0xFF) {
			*dst = 0xFF;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DPortal *>(dst), *pD);
			dst += sizeof(*pD);
		}
	}

	mq = sgJunk.quests;
	for (i = 0; i < MAXQUESTS; i++) {
		if (questlist[i]._qflags & QUEST_ANY) {
			mq->qlog = quests[i]._qlog;
			mq->qstate = quests[i]._qactive;
			mq->qvar1 = quests[i]._qvar1;
			copy_pod(*reinterpret_cast<MultiQuests *>(dst), *mq);
			dst += sizeof(*mq);
			mq++;
		}
	}

	return dst;
}

static void DeltaImportJunk(BYTE *src)
{
	DPortal *pD;
	MultiQuests *mq;
	int i;

	pD = sgJunk.portal;
	for (i = 0; i < MAXPORTAL; i++, pD++) {
		if (*src == 0xFF) {
			memset(pD, 0xFF, sizeof(*pD));
			src++;
			SetPortalStats(i, FALSE, 0, 0, 0, DTYPE_TOWN);
		} else {
			copy_pod(*pD, *reinterpret_cast<DPortal *>(src));
			src += sizeof(*pD);
			SetPortalStats(
				i,
				TRUE,
				pD->x,
				pD->y,
				pD->level,
				pD->ltype);
		}
	}

	mq = sgJunk.quests;
	for (i = 0; i < MAXQUESTS; i++) {
		if (questlist[i]._qflags & QUEST_ANY) {
			copy_pod(*mq, *reinterpret_cast<MultiQuests *>(src));
			src += sizeof(*mq);
			quests[i]._qlog = mq->qlog;
			quests[i]._qactive = mq->qstate;
			quests[i]._qvar1 = mq->qvar1;
			mq++;
		}
	}
}

static DWORD msg_comp_level(BYTE *buffer, BYTE *end)
{
	DWORD size, pkSize;

	size = end - buffer - 1;
	pkSize = PkwareCompress(buffer + 1, size);
	*buffer = size != pkSize;

	return pkSize + 1;
}

void DeltaExportData(int pnum)
{
	BYTE *dst, *dstEnd;
	int size, i;
	char src;

	if (sgbDeltaChanged) {
		dst = (BYTE *)DiabloAllocPtr(sizeof(DLevel) + 1);
		for (i = 0; i < NUMLEVELS; i++) {
			dstEnd = dst + 1;
			dstEnd = DeltaExportItem(dstEnd, sgLevels[i].item);
			dstEnd = DeltaExportObject(dstEnd, sgLevels[i].object);
			dstEnd = DeltaExportMonster(dstEnd, sgLevels[i].monster);
			size = msg_comp_level(dst, dstEnd);
			dthread_send_delta(pnum, i + CMD_DLEVEL_0, dst, size);
		}
		dstEnd = dst + 1;
		dstEnd = DeltaExportJunk(dstEnd);
		size = msg_comp_level(dst, dstEnd);
		dthread_send_delta(pnum, CMD_DLEVEL_JUNK, dst, size);
		mem_free_dbg(dst);
	}
	src = 0;
	dthread_send_delta(pnum, CMD_DLEVEL_END, &src, 1);
}

static void DeltaImportData(BYTE cmd, DWORD recv_offset)
{
	BYTE i;
	BYTE *src;

	if (sgRecvBuf[0] != 0)
		PkwareDecompress(&sgRecvBuf[1], recv_offset, lengthof(sgRecvBuf) - 1);

	src = &sgRecvBuf[1];
	if (cmd == CMD_DLEVEL_JUNK) {
		DeltaImportJunk(src);
#ifdef HELLFIRE
	} else if (cmd >= CMD_DLEVEL_0 && cmd <= CMD_DLEVEL_24) {
#else
	} else if (cmd >= CMD_DLEVEL_0 && cmd <= CMD_DLEVEL_16) {
#endif
		i = cmd - CMD_DLEVEL_0;
		src = DeltaImportItem(src, sgLevels[i].item);
		src = DeltaImportObject(src, sgLevels[i].object);
		DeltaImportMonster(src, sgLevels[i].monster);
	} else {
		app_fatal("msg:1");
	}

	sgbDeltaChunks++;
	sgbDeltaChanged = TRUE;
}

static DWORD On_DLEVEL(int pnum, TCmd *pCmd)
{
	TCmdPlrInfoHdr *cmd = (TCmdPlrInfoHdr *)pCmd;

	if (gbDeltaSender != pnum) {
		if (cmd->bCmd == CMD_DLEVEL_END) {
			gbDeltaSender = pnum;
			sgbRecvCmd = CMD_DLEVEL_END;
		} else if (cmd->bCmd == CMD_DLEVEL_0 && cmd->wOffset == 0) {
			gbDeltaSender = pnum;
			sgbRecvCmd = CMD_DLEVEL_END;
		} else {
			return cmd->wBytes + sizeof(*cmd);
		}
	}
	if (sgbRecvCmd == CMD_DLEVEL_END) {
		if (cmd->bCmd == CMD_DLEVEL_END) {
			sgbDeltaChunks = MAX_CHUNKS - 1;
			return cmd->wBytes + sizeof(*cmd);
		} else if (cmd->bCmd == CMD_DLEVEL_0 && cmd->wOffset == 0) {
			sgdwRecvOffset = 0;
			sgbRecvCmd = cmd->bCmd;
		} else {
			return cmd->wBytes + sizeof(*cmd);
		}
	} else if (sgbRecvCmd != cmd->bCmd) {
		DeltaImportData(sgbRecvCmd, sgdwRecvOffset);
		if (cmd->bCmd == CMD_DLEVEL_END) {
			sgbDeltaChunks = MAX_CHUNKS - 1;
			sgbRecvCmd = CMD_DLEVEL_END;
			return cmd->wBytes + sizeof(*cmd);
		} else {
			sgdwRecvOffset = 0;
			sgbRecvCmd = cmd->bCmd;
		}
	}

	/// ASSERT: assert(cmd->wOffset == sgdwRecvOffset);
	memcpy(&sgRecvBuf[cmd->wOffset], &cmd[1], cmd->wBytes);
	sgdwRecvOffset += cmd->wBytes;
	return cmd->wBytes + sizeof(*cmd);
}

void delta_init()
{
	sgbDeltaChanged = FALSE;
	memset(&sgJunk, 0xFF, sizeof(sgJunk));
	memset(sgLevels, 0xFF, sizeof(sgLevels));
	memset(sgLocals, 0, sizeof(sgLocals));
	deltaload = FALSE;
}

void delta_kill_monster(int mnum, BYTE x, BYTE y, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers != 1) {
		sgbDeltaChanged = TRUE;
		pD = &sgLevels[bLevel].monster[mnum];
		pD->_mx = x;
		pD->_my = y;
		pD->_mdir = monster[mnum]._mdir;
		pD->_mhitpoints = 0;
	}
}

void delta_monster_hp(int mnum, int hp, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers != 1) {
		sgbDeltaChanged = TRUE;
		pD = &sgLevels[bLevel].monster[mnum];
		if (pD->_mhitpoints > hp)
			pD->_mhitpoints = hp;
	}
}

void delta_sync_monster(const TSyncMonster *pSync, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1) {
		return;
	}

	/// ASSERT: assert(pSync != NULL);
	/// ASSERT: assert(bLevel < NUMLEVELS);
	sgbDeltaChanged = TRUE;

	pD = &sgLevels[bLevel].monster[pSync->_mndx];
	if (pD->_mhitpoints != 0) {
		pD->_mx = pSync->_mx;
		pD->_my = pSync->_my;
		pD->_mactive = UCHAR_MAX;
		pD->_menemy = pSync->_menemy;
	}
}

static void delta_sync_golem(TCmdGolem *pG, int pnum, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers != 1) {
		sgbDeltaChanged = TRUE;
		pD = &sgLevels[bLevel].monster[pnum];
		pD->_mx = pG->_mx;
		pD->_my = pG->_my;
		pD->_mactive = UCHAR_MAX;
		pD->_menemy = pG->_menemy;
		pD->_mdir = pG->_mdir;
		pD->_mhitpoints = pG->_mhitpoints;
	}
}

static void delta_leave_sync(BYTE bLevel)
{
	int i, mnum;
	DMonsterStr *pD;

	if (gbMaxPlayers != 1) {
		if (bLevel == 0) {
			glSeedTbl[0] = GetRndSeed();
		} else {
			for (i = 0; i < nummonsters; ++i) {
				mnum = monstactive[i];
				if (monster[mnum]._mhitpoints != 0) {
					sgbDeltaChanged = TRUE;
					pD = &sgLevels[bLevel].monster[mnum];
					pD->_mx = monster[mnum]._mx;
					pD->_my = monster[mnum]._my;
					pD->_mdir = monster[mnum]._mdir;
					pD->_menemy = encode_enemy(mnum);
					pD->_mhitpoints = monster[mnum]._mhitpoints;
					pD->_mactive = monster[mnum]._msquelch;
				}
			}
			memcpy(&sgLocals[bLevel], automapview, sizeof(automapview));
		}
	}
}

static void delta_sync_object(int oi, BYTE bCmd, BYTE bLevel)
{
	if (gbMaxPlayers != 1) {
		sgbDeltaChanged = TRUE;
		sgLevels[bLevel].object[oi].bCmd = bCmd;
	}
}

static BOOL delta_get_item(TCmdGItem *pI, BYTE bLevel)
{
	TCmdPItem *pD;
	int i;

	if (gbMaxPlayers == 1)
		return TRUE;

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF || pD->wIndx != pI->wIndx || pD->wCI != pI->wCI || pD->dwSeed != pI->dwSeed)
			continue;

		if (pD->bCmd == CMD_WALKXY) {
			return TRUE;
		}
		if (pD->bCmd == CMD_STAND) {
			sgbDeltaChanged = TRUE;
			pD->bCmd = CMD_WALKXY;
			return TRUE;
		}
		if (pD->bCmd == CMD_ACK_PLRINFO) {
			sgbDeltaChanged = TRUE;
			pD->bCmd = 0xFF;
			return TRUE;
		}

		app_fatal("delta:1");
		break;
	}

	if ((pI->wCI & CF_PREGEN) == 0)
		return FALSE;

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			sgbDeltaChanged = TRUE;
			pD->bCmd = CMD_WALKXY;
			pD->x = pI->x;
			pD->y = pI->y;
			pD->wIndx = pI->wIndx;
			pD->wCI = pI->wCI;
			pD->dwSeed = pI->dwSeed;
			pD->bId = pI->bId;
			pD->bDur = pI->bDur;
			pD->bMDur = pI->bMDur;
			pD->bCh = pI->bCh;
			pD->bMCh = pI->bMCh;
			pD->wValue = pI->wValue;
			pD->dwBuff = pI->dwBuff;
#ifdef HELLFIRE
			pD->wToHit = pI->wToHit;
			pD->wMaxDam = pI->wMaxDam;
			pD->bMinStr = pI->bMinStr;
			pD->bMinMag = pI->bMinMag;
			pD->bMinDex = pI->bMinDex;
			pD->bAC = pI->bAC;
#endif
			break;
		}
	}
	return TRUE;
}

static void delta_put_item(TCmdPItem *pI, int x, int y, BYTE bLevel)
{
	int i;
	TCmdPItem *pD;

	if (gbMaxPlayers == 1) {
		return;
	}
	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != CMD_WALKXY
			&& pD->bCmd != 0xFF
			&& pD->wIndx == pI->wIndx
			&& pD->wCI == pI->wCI
			&& pD->dwSeed == pI->dwSeed) {
			if (pD->bCmd == CMD_ACK_PLRINFO)
				return;
			app_fatal("Trying to drop a floor item?");
		}
	}

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			sgbDeltaChanged = TRUE;
			copy_pod(*pD, *pI);
			pD->bCmd = CMD_ACK_PLRINFO;
			pD->x = x;
			pD->y = y;
			return;
		}
	}
}

BOOL delta_portal_inited(int i)
{
	return sgJunk.portal[i].x == 0xFF;
}

BOOL delta_quest_inited(int i)
{
	return sgJunk.quests[i].qstate != 0xFF;
}

static void PackPItem(TCmdPItem *dest, ItemStruct *src)
{
	dest->wIndx = src->IDidx;

	if (src->IDidx == IDI_EAR) {
		dest->wCI = src->_iName[8] | (src->_iName[7] << 8);
		dest->dwSeed = src->_iName[12] | ((src->_iName[11] | ((src->_iName[10] | (src->_iName[9] << 8)) << 8)) << 8);
		dest->bId = src->_iName[13];
		dest->bDur = src->_iName[14];
		dest->bMDur = src->_iName[15];
		dest->bCh = src->_iName[16];
		dest->bMCh = src->_iName[17];
		dest->wValue = src->_ivalue | (src->_iName[18] << 8) | ((src->_iCurs - ICURS_EAR_SORCEROR) << 6);
		dest->dwBuff = src->_iName[22] | ((src->_iName[21] | ((src->_iName[20] | (src->_iName[19] << 8)) << 8)) << 8);
	} else {
		dest->wCI = src->_iCreateInfo;
		dest->dwSeed = src->_iSeed;
		dest->bId = src->_iIdentified;
		dest->bDur = src->_iDurability;
		dest->bMDur = src->_iMaxDur;
		dest->bCh = src->_iCharges;
		dest->bMCh = src->_iMaxCharges;
		dest->wValue = src->_ivalue;
#ifdef HELLFIRE
		dest->wToHit = src->_iPLToHit;
		dest->wMaxDam = src->_iMaxDam;
		dest->bMinStr = src->_iMinStr;
		dest->bMinMag = src->_iMinMag;
		dest->bMinDex = src->_iMinDex;
		dest->bAC = src->_iAC;
#endif
	}
}

static void PackGItem(TCmdGItem *dest, ItemStruct *src)
{
	dest->wIndx = src->IDidx;

	if (src->IDidx == IDI_EAR) {
		dest->wCI = src->_iName[8] | (src->_iName[7] << 8);
		dest->dwSeed = src->_iName[12] | ((src->_iName[11] | ((src->_iName[10] | (src->_iName[9] << 8)) << 8)) << 8);
		dest->bId = src->_iName[13];
		dest->bDur = src->_iName[14];
		dest->bMDur = src->_iName[15];
		dest->bCh = src->_iName[16];
		dest->bMCh = src->_iName[17];
		dest->wValue = src->_ivalue | (src->_iName[18] << 8) | ((src->_iCurs - ICURS_EAR_SORCEROR) << 6);
		dest->dwBuff = src->_iName[22] | ((src->_iName[21] | ((src->_iName[20] | (src->_iName[19] << 8)) << 8)) << 8);
	} else {
		dest->wCI = src->_iCreateInfo;
		dest->dwSeed = src->_iSeed;
		dest->bId = src->_iIdentified;
		dest->bDur = src->_iDurability;
		dest->bMDur = src->_iMaxDur;
		dest->bCh = src->_iCharges;
		dest->bMCh = src->_iMaxCharges;
		dest->wValue = src->_ivalue;
#ifdef HELLFIRE
		dest->wToHit = src->_iPLToHit;
		dest->wMaxDam = src->_iMaxDam;
		dest->bMinStr = src->_iMinStr;
		dest->bMinMag = src->_iMinMag;
		dest->bMinDex = src->_iMinDex;
		dest->bAC = src->_iAC;
#endif
	}
}

void DeltaAddItem(int ii)
{
	ItemStruct *is;
	int i;
	TCmdPItem *pD;

	if (gbMaxPlayers == 1)
		return;

	is = &item[ii];
	pD = sgLevels[currlevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != 0xFF
			&& pD->wIndx == is->IDidx
			&& pD->wCI == is->_iCreateInfo
			&& pD->dwSeed == is->_iSeed
			&& (pD->bCmd == CMD_WALKXY || pD->bCmd == CMD_STAND)) {
			return;
		}
	}

	pD = sgLevels[currlevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			sgbDeltaChanged = TRUE;
			pD->bCmd = CMD_STAND;
			pD->x = is->_ix;
			pD->y = is->_iy;

			PackPItem(pD, is);
			return;
		}
	}
}

void DeltaSaveLevel()
{
	int i;

	if (gbMaxPlayers != 1) {
		for (i = 0; i < MAX_PLRS; i++) {
			if (i != myplr)
				plr[i]._pGFXLoad = 0;
		}
		plr[myplr]._pLvlVisited[currlevel] = TRUE;
		delta_leave_sync(currlevel);
	}
}

static void UnPackPItem(TCmdPItem *src)
{
	if (src->wIndx == IDI_EAR) {
		RecreateEar(
			src->wCI,
			src->dwSeed,
			src->bId,
			src->bDur,
			src->bMDur,
			src->bCh,
			src->bMCh,
			src->wValue,
			src->dwBuff);
	} else {
		RecreateItem(
			src->wIndx,
			src->wCI,
			src->dwSeed,
			src->wValue);
		if (src->bId)
			item[MAXITEMS]._iIdentified = TRUE;
		item[MAXITEMS]._iDurability = src->bDur;
		item[MAXITEMS]._iMaxDur = src->bMDur;
		item[MAXITEMS]._iCharges = src->bCh;
		item[MAXITEMS]._iMaxCharges = src->bMCh;
#ifdef HELLFIRE
		item[MAXITEMS]._iPLToHit = src->wToHit;
		item[MAXITEMS]._iMaxDam = src->wMaxDam;
		item[MAXITEMS]._iMinStr = src->bMinStr;
		item[MAXITEMS]._iMinMag = src->bMinMag;
		item[MAXITEMS]._iMinDex = src->bMinDex;
		item[MAXITEMS]._iAC = src->bAC;
#endif
	}
}

static void UnPackGItem(TCmdGItem *src)
{
	if (src->wIndx == IDI_EAR) {
		RecreateEar(
			src->wCI,
			src->dwSeed,
			src->bId,
			src->bDur,
			src->bMDur,
			src->bCh,
			src->bMCh,
			src->wValue,
			src->dwBuff);
	} else {
		RecreateItem(
			src->wIndx,
			src->wCI,
			src->dwSeed,
			src->wValue);
		if (src->bId)
			item[MAXITEMS]._iIdentified = TRUE;
		item[MAXITEMS]._iDurability = src->bDur;
		item[MAXITEMS]._iMaxDur = src->bMDur;
		item[MAXITEMS]._iCharges = src->bCh;
		item[MAXITEMS]._iMaxCharges = src->bMCh;
#ifdef HELLFIRE
		item[MAXITEMS]._iPLToHit = src->wToHit;
		item[MAXITEMS]._iMaxDam = src->wMaxDam;
		item[MAXITEMS]._iMinStr = src->bMinStr;
		item[MAXITEMS]._iMinMag = src->bMinMag;
		item[MAXITEMS]._iMinDex = src->bMinDex;
		item[MAXITEMS]._iAC = src->bAC;
#endif
	}
}

void DeltaLoadLevel()
{
	DMonsterStr *mstr;
	DObjectStr *dstr;
	MonsterStruct *mon;
	TCmdPItem* itm;
	int ii, ot;
	int i;
	int x, y;

	if (gbMaxPlayers == 1) {
		return;
	}

	deltaload = TRUE;
	if (currlevel != 0) {
		mstr = sgLevels[currlevel].monster;
		for (i = 0; i < nummonsters; i++, mstr++) {
			if (mstr->_mx != 0xFF) {
				MonClearSquares(i);
				x = mstr->_mx;
				y = mstr->_my;
				mon = &monster[i];
				mon->_mx = x;
				mon->_my = y;
				mon->_moldx = x;
				mon->_moldy = y;
				mon->_mfutx = x;
				mon->_mfuty = y;
				if (mstr->_mhitpoints != -1)
					mon->_mhitpoints = mstr->_mhitpoints;
				if (mstr->_mhitpoints == 0) {
					AddDead(i);
				} else {
					decode_enemy(i, mstr->_menemy);
					if (mon->_mx && mon->_mx != 1 || mon->_my)
						dMonster[mon->_mx][mon->_my] = i + 1;
					if (i < MAX_PLRS) {
						MAI_Golum(i);
						mon->_mFlags |= (MFLAG_TARGETS_MONSTER | MFLAG_GOLEM);
					} else {
						MonStartStand(i, mon->_mdir);
					}
					mon->_msquelch = mstr->_mactive;
				}
			}
		}
		memcpy(automapview, &sgLocals[currlevel], sizeof(automapview));
	}

	itm = sgLevels[currlevel].item;
	for (i = 0; i < MAXITEMS; i++, itm++) {
		if (itm->bCmd == CMD_WALKXY) {
			ii = FindGetItem(
				itm->wIndx,
				itm->wCI,
				itm->dwSeed);
			if (ii != -1) {
				if (dItem[item[ii]._ix][item[ii]._iy] == ii + 1)
					dItem[item[ii]._ix][item[ii]._iy] = 0;
				DeleteItem(ii, i);
			}
		} else if (itm->bCmd == CMD_ACK_PLRINFO) {
			UnPackPItem(itm);
			x = itm->x;
			y = itm->y;
			if (!CanPut(x, y))
				FindItemLocation(x, y, &x, &y, DSIZEX / 2);

			ii = itemavail[0];
			itemavail[0] = itemavail[MAXITEMS - numitems - 1];
			itemactive[numitems] = ii;
			copy_pod(item[ii], item[MAXITEMS]);
			item[ii]._ix = x;
			item[ii]._iy = y;
			dItem[x][y] = ii + 1;
			RespawnItem(ii, FALSE);
			numitems++;
		}
	}

	if (currlevel != 0) {
		dstr = sgLevels[currlevel].object;
		for (i = 0; i < MAXOBJECTS; i++, dstr++) {
			switch (dstr->bCmd) {
			case CMD_OPENDOOR:
			case CMD_CLOSEDOOR:
			case CMD_OPERATEOBJ:
			case CMD_PLROPOBJ:
				SyncOpObject(-1, dstr->bCmd, i);
				break;
			case CMD_BREAKOBJ:
				SyncBreakObj(-1, i);
				break;
			}
		}

		for (i = 0; i < nobjects; i++) {
			ot = object[objectactive[i]]._otype;
			if (ot == OBJ_TRAPL || ot == OBJ_TRAPR)
				Obj_Trap(objectactive[i]);
		}
	}
	deltaload = FALSE;
}

void NetSendCmd(BOOL bHiPri, BYTE bCmd)
{
	TCmd cmd;

	cmd.bCmd = bCmd;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdGolem(BYTE mx, BYTE my, BYTE dir, BYTE menemy, int hp, BYTE cl)
{
	TCmdGolem cmd;

	cmd.bCmd = CMD_AWAKEGOLEM;
	cmd._mx = mx;
	cmd._my = my;
	cmd._mdir = dir;
	cmd._menemy = menemy;
	cmd._mhitpoints = hp;
	cmd._currlevel = cl;
	NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLoc(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y)
{
	ALIGN_BY_1 TCmdLoc cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLocParam1(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1)
{
	TCmdLocParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.wParam1 = wParam1;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLocParam2(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1, WORD wParam2)
{
	TCmdLocParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.wParam1 = wParam1;
	cmd.wParam2 = wParam2;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLocParam3(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1, WORD wParam2, WORD wParam3)
{
	TCmdLocParam3 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.wParam1 = wParam1;
	cmd.wParam2 = wParam2;
	cmd.wParam3 = wParam3;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdParam1(BOOL bHiPri, BYTE bCmd, WORD wParam1)
{
	ALIGN_BY_1 TCmdParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = wParam1;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdParam2(BOOL bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2)
{
	TCmdParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = wParam1;
	cmd.wParam2 = wParam2;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdParam3(BOOL bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2, WORD wParam3)
{
	TCmdParam3 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = wParam1;
	cmd.wParam2 = wParam2;
	cmd.wParam3 = wParam3;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdQuest(BOOL bHiPri, BYTE q)
{
	TCmdQuest cmd;

	cmd.q = q;
	cmd.bCmd = CMD_SYNCQUEST;
	cmd.qstate = quests[q]._qactive;
	cmd.qlog = quests[q]._qlog;
	cmd.qvar1 = quests[q]._qvar1;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdGItem(BOOL bHiPri, BYTE bCmd, BYTE mast, BYTE pnum, BYTE ii)
{
	ItemStruct* is;
	TCmdGItem cmd;

	cmd.bCmd = bCmd;
	cmd.bPnum = pnum;
	cmd.bMaster = mast;
	cmd.bLevel = currlevel;
	cmd.bCursitem = ii;
	cmd.dwTime = 0;
	is = &item[ii];
	cmd.x = is->_ix;
	cmd.y = is->_iy;

	PackGItem(&cmd, is);

	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

static void NetSendCmdGItem2(BOOL usonly, BYTE bCmd, BYTE mast, BYTE pnum, TCmdGItem *p)
{
	int ticks;
	TCmdGItem cmd;

#ifdef HELLFIRE
	cmd = *p;
#else
	memcpy(&cmd, p, sizeof(cmd));
#endif
	cmd.bPnum = pnum;
	cmd.bCmd = bCmd;
	cmd.bMaster = mast;

	if (!usonly) {
		cmd.dwTime = 0;
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
		return;
	}

	ticks = SDL_GetTicks();
	if (cmd.dwTime == 0) {
		cmd.dwTime = ticks;
	} else if (ticks - cmd.dwTime > 5000) {
		return;
	}

	multi_msg_add((BYTE *)&cmd.bCmd, sizeof(cmd));
}

static BOOL NetSendCmdReq2(BYTE bCmd, BYTE mast, BYTE pnum, TCmdGItem *p)
{
	int ticks;
	TCmdGItem cmd;

#ifdef HELLFIRE
	cmd = *p;
#else
	memcpy(&cmd, p, sizeof(cmd));
#endif
	cmd.bCmd = bCmd;
	cmd.bPnum = pnum;
	cmd.bMaster = mast;

	ticks = SDL_GetTicks();
	if (cmd.dwTime == 0) {
		cmd.dwTime = ticks;
	} else if (ticks - cmd.dwTime > 5000) {
		return FALSE;
	}

	multi_msg_add((BYTE *)&cmd.bCmd, sizeof(cmd));

	return TRUE;
}

static void NetSendCmdExtra(TCmdGItem *p)
{
	TCmdGItem cmd;

#ifdef HELLFIRE
	cmd = *p;
#else
	memcpy(&cmd, p, sizeof(cmd));
#endif
	cmd.dwTime = 0;
	cmd.bCmd = CMD_ITEMEXTRA;
	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdPItem(BOOL bHiPri, BYTE bCmd, BYTE x, BYTE y)
{
	TCmdPItem cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;

	PackPItem(&cmd, &plr[myplr].HoldItem);

	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdChItem(BOOL bHiPri, ItemStruct *is, BYTE bLoc)
{
	TCmdChItem cmd;

	cmd.bCmd = CMD_CHANGEPLRITEMS;
	cmd.bLoc = bLoc;
	cmd.wIndx = is->IDidx;
	cmd.wCI = is->_iCreateInfo;
	cmd.dwSeed = is->_iSeed;
	cmd.bId = is->_iIdentified;

	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdDelItem(BOOL bHiPri, BYTE bLoc)
{
	TCmdDelItem cmd;

	cmd.bLoc = bLoc;
	cmd.bCmd = CMD_DELPLRITEMS;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdDItem(BOOL bHiPri, int ii)
{
	ItemStruct *is;
	TCmdPItem cmd;

	is = &item[ii];
	cmd.bCmd = CMD_DROPITEM;
	cmd.x = is->_ix;
	cmd.y = is->_iy;

	PackPItem(&cmd, is);

	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

static BOOL i_own_level(int nReqLevel)
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive
			&& !plr[i]._pLvlChanging
			&& plr[i].plrlevel == nReqLevel
			&& (i != myplr || gbBufferMsgs == 0))
			break;
	}

	return i == myplr;
}

void NetSendCmdDamage(BOOL bHiPri, BYTE bPlr, DWORD dwDam)
{
	TCmdDamage cmd;

	cmd.bCmd = CMD_PLRDAMAGE;
	cmd.bPlr = bPlr;
	cmd.dwDam = dwDam;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

#ifdef HELLFIRE
void NetSendCmdMonDmg(BOOL bHiPri, WORD wMon, DWORD dwDam)
{
	TCmdMonDamage cmd;

	cmd.bCmd = CMD_MONSTDAMAGE;
	cmd.wMon = wMon;
	cmd.dwDam = dwDam;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}
#endif

void NetSendCmdString(unsigned int pmask)
{
	int dwStrLen;
	TCmdString cmd;

	dwStrLen = strlen(gbNetMsg);
	cmd.bCmd = CMD_STRING;
	memcpy(cmd.str, gbNetMsg, dwStrLen + 1);
	multi_send_msg_packet(pmask, (BYTE *)&cmd.bCmd, dwStrLen + 2);
}

static DWORD On_STRING2(int pnum, TCmd *pCmd)
{
	TCmdString *cmd = (TCmdString *)pCmd;

	int len = strlen(cmd->str);
	if (gbBufferMsgs == 0)
		SendPlrMsg(pnum, cmd->str);

	return len + 2; // length of string + nul terminator + sizeof(cmd->bCmd)
}

static void delta_open_portal(int pnum, BYTE x, BYTE y, BYTE bLevel, BYTE bLType, BYTE bSetLvl)
{
	sgbDeltaChanged = TRUE;
	sgJunk.portal[pnum].x = x;
	sgJunk.portal[pnum].y = y;
	sgJunk.portal[pnum].level = bLevel;
	sgJunk.portal[pnum].ltype = bLType;
	sgJunk.portal[pnum].setlvl = bSetLvl;
}

void delta_close_portal(int pnum)
{
	memset(&sgJunk.portal[pnum], 0xFF, sizeof(sgJunk.portal[pnum]));
	sgbDeltaChanged = TRUE;
}

static void check_update_plr(int pnum)
{
	if (gbMaxPlayers != 1 && pnum == myplr)
		pfile_update(TRUE);
}

static void msg_errorf(const char *pszFmt, ...)
{
	static DWORD msg_err_timer;
	DWORD ticks;
	char msg[256];
	va_list va;

	va_start(va, pszFmt);
	ticks = SDL_GetTicks();
	if (ticks - msg_err_timer >= 5000) {
		msg_err_timer = ticks;
		vsnprintf(msg, sizeof(msg), pszFmt, va);
		ErrorPlrMsg(msg);
	}
	va_end(va);
}

static DWORD On_SYNCDATA(TCmd *pCmd, int pnum)
{
	return sync_update(pnum, (const BYTE *)pCmd);
}

static DWORD On_WALKXY(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		MakePlrPath(pnum, cmd->x, cmd->y, TRUE);
		plr[pnum].destAction = ACTION_NONE;
	}

	return sizeof(*cmd);
}

static DWORD On_ADDSTR(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		ModifyPlrStr(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_ADDMAG(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		ModifyPlrMag(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_ADDDEX(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd= (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		ModifyPlrDex(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_ADDVIT(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		ModifyPlrVit(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_SBSPELL(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs != 1) {
		int spell = cmd->wParam1;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pSBkSplType;
			plr[pnum]._pSplFrom = 1;
			plr[pnum].destAction = ACTION_SPELL;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_GOTOGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, FALSE);
		plr[pnum].destAction = ACTION_PICKUPITEM;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static DWORD On_REQUESTGITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (gbBufferMsgs != 1 && i_own_level(plr[pnum].plrlevel)) {
		if (GetItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx)) {
			int ii = FindGetItem(cmd->wIndx, cmd->wCI, cmd->dwSeed);
			if (ii != -1) {
				NetSendCmdGItem2(FALSE, CMD_GETITEM, myplr, cmd->bPnum, cmd);
				if (cmd->bPnum != myplr)
					SyncGetItem(cmd->x, cmd->y, cmd->wIndx, cmd->wCI, cmd->dwSeed);
				else
					InvGetItem(myplr, ii);
				SetItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
			} else if (!NetSendCmdReq2(CMD_REQUESTGITEM, myplr, cmd->bPnum, cmd))
				NetSendCmdExtra(cmd);
		}
	}

	return sizeof(*cmd);
}

static DWORD On_GETITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		int ii = FindGetItem(cmd->wIndx, cmd->wCI, cmd->dwSeed);
		if (delta_get_item(cmd, cmd->bLevel)) {
			if ((currlevel == cmd->bLevel || cmd->bPnum == myplr) && cmd->bMaster != myplr) {
				if (cmd->bPnum == myplr) {
					if (currlevel != cmd->bLevel) {
						UnPackGItem(cmd);
						ii = SyncPutItem(myplr, plr[myplr]._px, plr[myplr]._py, &item[MAXITEMS]);
						if (ii != -1)
							InvGetItem(myplr, ii);
					} else
						InvGetItem(myplr, ii);
				} else
					SyncGetItem(cmd->x, cmd->y, cmd->wIndx, cmd->wCI, cmd->dwSeed);
			}
		} else
			NetSendCmdGItem2(TRUE, CMD_GETITEM, cmd->bMaster, cmd->bPnum, cmd);
	}

	return sizeof(*cmd);
}

static DWORD On_GOTOAGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, FALSE);
		plr[pnum].destAction = ACTION_PICKUPAITEM;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static DWORD On_REQUESTAGITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (gbBufferMsgs != 1 && i_own_level(plr[pnum].plrlevel)) {
		if (GetItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx)) {
			int ii = FindGetItem(cmd->wIndx, cmd->wCI, cmd->dwSeed);
			if (ii != -1) {
				NetSendCmdGItem2(FALSE, CMD_AGETITEM, myplr, cmd->bPnum, cmd);
				if (cmd->bPnum != myplr)
					SyncGetItem(cmd->x, cmd->y, cmd->wIndx, cmd->wCI, cmd->dwSeed);
				else
					AutoGetItem(myplr, cmd->bCursitem);
				SetItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
			} else if (!NetSendCmdReq2(CMD_REQUESTAGITEM, myplr, cmd->bPnum, cmd))
				NetSendCmdExtra(cmd);
		}
	}

	return sizeof(*cmd);
}

static DWORD On_AGETITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		FindGetItem(cmd->wIndx, cmd->wCI, cmd->dwSeed);
		if (delta_get_item(cmd, cmd->bLevel)) {
			if ((currlevel == cmd->bLevel || cmd->bPnum == myplr) && cmd->bMaster != myplr) {
				if (cmd->bPnum == myplr) {
					if (currlevel != cmd->bLevel) {
						UnPackGItem(cmd);
						int ii = SyncPutItem(myplr, plr[myplr]._px, plr[myplr]._py, &item[MAXITEMS]);
						if (ii != -1)
							AutoGetItem(myplr, ii);
					} else
						AutoGetItem(myplr, cmd->bCursitem);
				} else
					SyncGetItem(cmd->x, cmd->y, cmd->wIndx, cmd->wCI, cmd->dwSeed);
			}
		} else
			NetSendCmdGItem2(TRUE, CMD_AGETITEM, cmd->bMaster, cmd->bPnum, cmd);
	}

	return sizeof(*cmd);
}

static DWORD On_ITEMEXTRA(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		delta_get_item(cmd, cmd->bLevel);
		if (currlevel == plr[pnum].plrlevel)
			SyncGetItem(cmd->x, cmd->y, cmd->wIndx, cmd->wCI, cmd->dwSeed);
	}

	return sizeof(*cmd);
}

static DWORD On_PUTITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (currlevel == plr[pnum].plrlevel) {
		int ii;
		if (pnum == myplr)
			ii = InvPutItem(pnum, cmd->x, cmd->y);
		else {
			UnPackPItem(cmd);
			ii = SyncPutItem(pnum, cmd->x, cmd->y, &item[MAXITEMS]);
		}
		if (ii != -1) {
			PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
			delta_put_item(cmd, item[ii]._ix, item[ii]._iy, plr[pnum].plrlevel);
			check_update_plr(pnum);
		}
		return sizeof(*cmd);
	} else {
		PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
		delta_put_item(cmd, cmd->x, cmd->y, plr[pnum].plrlevel);
		check_update_plr(pnum);
	}

	return sizeof(*cmd);
}

static DWORD On_SYNCPUTITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (currlevel == plr[pnum].plrlevel) {
		UnPackPItem(cmd);
		int ii = SyncPutItem(pnum, cmd->x, cmd->y, &item[MAXITEMS]);
		if (ii != -1) {
			PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
			delta_put_item(cmd, item[ii]._ix, item[ii]._iy, plr[pnum].plrlevel);
			check_update_plr(pnum);
		}
		return sizeof(*cmd);
	} else {
		PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
		delta_put_item(cmd, cmd->x, cmd->y, plr[pnum].plrlevel);
		check_update_plr(pnum);
	}

	return sizeof(*cmd);
}

static DWORD On_RESPAWNITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (currlevel == plr[pnum].plrlevel && pnum != myplr) {
			UnPackPItem(cmd);
			SyncPutItem(pnum, cmd->x, cmd->y, &item[MAXITEMS]);
		}
		PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
		delta_put_item(cmd, cmd->x, cmd->y, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_ATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, FALSE);
		plr[pnum].destAction = ACTION_ATTACK;
		plr[pnum].destParam1 = cmd->x;
		plr[pnum].destParam2 = cmd->y;
	}

	return sizeof(*cmd);
}

static DWORD On_SATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		plr[pnum].destAction = ACTION_ATTACK;
		plr[pnum].destParam1 = cmd->x;
		plr[pnum].destParam2 = cmd->y;
	}

	return sizeof(*cmd);
}

static DWORD On_RATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		plr[pnum].destAction = ACTION_RATTACK;
		plr[pnum].destParam1 = cmd->x;
		plr[pnum].destParam2 = cmd->y;
	}

	return sizeof(*cmd);
}

static DWORD On_SPELLXYD(TCmd *pCmd, int pnum)
{
	TCmdLocParam3 *cmd = (TCmdLocParam3 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int spell = cmd->wParam1;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			ClrPlrPath(pnum);
			plr[pnum].destAction = ACTION_SPELLWALL;
			plr[pnum].destParam1 = cmd->x;
			plr[pnum].destParam2 = cmd->y;
			plr[pnum].destParam3 = cmd->wParam2;
			plr[pnum].destParam4 = cmd->wParam3;
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pRSplType;
			plr[pnum]._pSplFrom = 0;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_SPELLXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam2 *cmd = (TCmdLocParam2 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int spell = cmd->wParam1;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			ClrPlrPath(pnum);
			plr[pnum].destAction = ACTION_SPELL;
			plr[pnum].destParam1 = cmd->x;
			plr[pnum].destParam2 = cmd->y;
			plr[pnum].destParam3 = cmd->wParam2;
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pRSplType;
			plr[pnum]._pSplFrom = 0;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_TSPELLXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam2 *cmd = (TCmdLocParam2 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int spell = cmd->wParam1;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			ClrPlrPath(pnum);
			plr[pnum].destAction = ACTION_SPELL;
			plr[pnum].destParam1 = cmd->x;
			plr[pnum].destParam2 = cmd->y;
			plr[pnum].destParam3 = cmd->wParam2;
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pTSplType;
			plr[pnum]._pSplFrom = 2;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_OPOBJXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int oi = cmd->wParam1;
		if (object[oi]._oSolidFlag || object[oi]._oDoorFlag)
			MakePlrPath(pnum, cmd->x, cmd->y, FALSE);
		else
			MakePlrPath(pnum, cmd->x, cmd->y, TRUE);
		plr[pnum].destAction = ACTION_OPERATE;
		plr[pnum].destParam1 = oi;
	}

	return sizeof(*cmd);
}

static DWORD On_DISARMXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int oi = cmd->wParam1;
		if (object[oi]._oSolidFlag || object[oi]._oDoorFlag)
			MakePlrPath(pnum, cmd->x, cmd->y, FALSE);
		else
			MakePlrPath(pnum, cmd->x, cmd->y, TRUE);
		plr[pnum].destAction = ACTION_DISARM;
		plr[pnum].destParam1 = oi;
	}

	return sizeof(*cmd);
}

static DWORD On_OPOBJT(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		plr[pnum].destAction = ACTION_OPERATETK;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static DWORD On_ATTACKID(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;
	int mnum, x, y;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		mnum = cmd->wParam1;
		plr[pnum].destParam1 = mnum;
		plr[pnum].destAction = ACTION_ATTACKMON;
		x = monster[mnum]._mfutx;
		y = monster[mnum]._mfuty;
		if (abs(plr[pnum]._px - x) > 1 || abs(plr[pnum]._py - y) > 1)
			MakePlrPath(pnum, x, y, FALSE);
	}

	return sizeof(*cmd);
}

static DWORD On_ATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int tnum = cmd->wParam1;
		MakePlrPath(pnum, plr[tnum]._pfutx, plr[tnum]._pfuty, FALSE);
		plr[pnum].destAction = ACTION_ATTACKPLR;
		plr[pnum].destParam1 = tnum;
	}

	return sizeof(*cmd);
}

static DWORD On_RATTACKID(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		plr[pnum].destAction = ACTION_RATTACKMON;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static DWORD On_RATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		plr[pnum].destAction = ACTION_RATTACKPLR;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static DWORD On_SPELLID(TCmd *pCmd, int pnum)
{
	TCmdParam3 *cmd = (TCmdParam3 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int spell = cmd->wParam2;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			ClrPlrPath(pnum);
			plr[pnum].destAction = ACTION_SPELLMON;
			plr[pnum].destParam1 = cmd->wParam1;
			plr[pnum].destParam2 = cmd->wParam3;
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pRSplType;
			plr[pnum]._pSplFrom = 0;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_SPELLPID(TCmd *pCmd, int pnum)
{
	TCmdParam3 *cmd = (TCmdParam3 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int spell = cmd->wParam2;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			ClrPlrPath(pnum);
			plr[pnum].destAction = ACTION_SPELLPLR;
			plr[pnum].destParam1 = cmd->wParam1;
			plr[pnum].destParam2 = cmd->wParam3;
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pRSplType;
			plr[pnum]._pSplFrom = 0;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_TSPELLID(TCmd *pCmd, int pnum)
{
	TCmdParam3 *cmd = (TCmdParam3 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int spell = cmd->wParam2;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			ClrPlrPath(pnum);
			plr[pnum].destAction = ACTION_SPELLMON;
			plr[pnum].destParam1 = cmd->wParam1;
			plr[pnum].destParam2 = cmd->wParam3;
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pTSplType;
			plr[pnum]._pSplFrom = 2;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_TSPELLPID(TCmd *pCmd, int pnum)
{
	TCmdParam3 *cmd = (TCmdParam3 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		int spell = cmd->wParam2;
		if (currlevel != 0 || spelldata[spell].sTownSpell) {
			ClrPlrPath(pnum);
			plr[pnum].destAction = ACTION_SPELLPLR;
			plr[pnum].destParam1 = cmd->wParam1;
			plr[pnum].destParam2 = cmd->wParam3;
			plr[pnum]._pSpell = spell;
			plr[pnum]._pSplType = plr[pnum]._pTSplType;
			plr[pnum]._pSplFrom = 2;
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static DWORD On_KNOCKBACK(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		MonGetKnockback(cmd->wParam1);
		MonStartHit(cmd->wParam1, pnum, 0);
	}

	return sizeof(*cmd);
}

static DWORD On_RESURRECT(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		DoResurrect(pnum, cmd->wParam1);
		check_update_plr(pnum);
	}

	return sizeof(*cmd);
}

static DWORD On_HEALOTHER(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel)
		DoHealOther(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_TALKXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, FALSE);
		plr[pnum].destAction = ACTION_TALK;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static DWORD On_NEWLVL(TCmd *pCmd, int pnum)
{
	TCmdParam2 *cmd = (TCmdParam2 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		StartNewLvl(pnum, cmd->wParam1, cmd->wParam2);

	return sizeof(*cmd);
}

static DWORD On_WARP(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		StartWarpLvl(pnum, cmd->wParam1);
		if (pnum == myplr && pcurs >= CURSOR_FIRSTITEM) {
			copy_pod(item[MAXITEMS], plr[myplr].HoldItem);
			AutoGetItem(myplr, MAXITEMS);
		}
	}

	return sizeof(*cmd);
}

static DWORD On_MONSTDEATH(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr) {
		if (currlevel == plr[pnum].plrlevel)
			MonSyncStartKill(cmd->wParam1, cmd->x, cmd->y, pnum);
		delta_kill_monster(cmd->wParam1, cmd->x, cmd->y, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_KILLGOLEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr) {
		if (currlevel == cmd->wParam1)
			MonSyncStartKill(pnum, cmd->x, cmd->y, pnum);
		delta_kill_monster(pnum, cmd->x, cmd->y, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_AWAKEGOLEM(TCmd *pCmd, int pnum)
{
	TCmdGolem *cmd = (TCmdGolem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (currlevel != plr[pnum].plrlevel)
		delta_sync_golem(cmd, pnum, cmd->_currlevel);
	else if (pnum != myplr) {
		int i;
		// check if this player already has an active golem
		for (i = 0; i < nummissiles; i++) {
			MissileStruct *mis = &missile[missileactive[i]];
			if (mis->_miType == MIS_GOLEM && mis->_miSource == pnum) {
				break;
			}
		}
		if (i == nummissiles)
			AddMissile(plr[pnum]._px, plr[pnum]._py, cmd->_mx, cmd->_my, cmd->_mdir, MIS_GOLEM, 0, pnum, 0, 1);
	}

	return sizeof(*cmd);
}

static DWORD On_MONSTDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdParam2 *cmd = (TCmdParam2 *)pCmd;
	int mnum;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr) {
		if (currlevel == plr[pnum].plrlevel) {
			mnum = cmd->wParam1;
			monster[mnum].mWhoHit |= 1 << pnum;

			if (monster[mnum]._mhitpoints != 0) {
				monster[mnum]._mhitpoints -= cmd->wParam2;
				if (monster[mnum]._mhitpoints < (1 << 6))
					monster[mnum]._mhitpoints = 1 << 6;
				delta_monster_hp(mnum, monster[mnum]._mhitpoints, plr[pnum].plrlevel);
			}
		}
	}

	return sizeof(*cmd);
}

static DWORD On_PLRDEAD(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		StartPlrKill(pnum, cmd->wParam1);
	else
		check_update_plr(pnum);

	return sizeof(*cmd);
}

static DWORD On_PLRDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdDamage *cmd = (TCmdDamage *)pCmd;
	DWORD dam;

	if (cmd->bPlr == myplr && currlevel != 0) {
		if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel) {
			dam = cmd->dwDam;
			if ((plr[myplr]._pHitPoints >> 6) > 0 && dam <= 192000) {
				drawhpflag = TRUE;
				plr[myplr]._pHitPoints -= dam;
				plr[myplr]._pHPBase -= dam;
				if ((plr[myplr]._pHitPoints >> 6) <= 0)
					SyncPlrKill(myplr, 1);
			}
		}
	}

	return sizeof(*cmd);
}

static DWORD On_OPENDOOR(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (currlevel == plr[pnum].plrlevel)
			SyncOpObject(pnum, CMD_OPENDOOR, cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_OPENDOOR, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_CLOSEDOOR(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (currlevel == plr[pnum].plrlevel)
			SyncOpObject(pnum, CMD_CLOSEDOOR, cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_CLOSEDOOR, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_OPERATEOBJ(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (currlevel == plr[pnum].plrlevel)
			SyncOpObject(pnum, CMD_OPERATEOBJ, cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_OPERATEOBJ, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_PLROPOBJ(TCmd *pCmd, int pnum)
{
	TCmdParam2 *cmd = (TCmdParam2 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (currlevel == plr[pnum].plrlevel)
			SyncOpObject(cmd->wParam1, CMD_PLROPOBJ, cmd->wParam2);
		delta_sync_object(cmd->wParam2, CMD_PLROPOBJ, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_BREAKOBJ(TCmd *pCmd, int pnum)
{
	TCmdParam2 *cmd = (TCmdParam2 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (currlevel == plr[pnum].plrlevel)
			SyncBreakObj(cmd->wParam1, cmd->wParam2);
		delta_sync_object(cmd->wParam2, CMD_BREAKOBJ, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static DWORD On_CHANGEPLRITEMS(TCmd *pCmd, int pnum)
{
	TCmdChItem *cmd = (TCmdChItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		CheckInvSwap(pnum, cmd->bLoc, cmd->wIndx, cmd->wCI, cmd->dwSeed, cmd->bId);

	return sizeof(*cmd);
}

static DWORD On_DELPLRITEMS(TCmd *pCmd, int pnum)
{
	TCmdDelItem *cmd = (TCmdDelItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		inv_update_rem_item(pnum, cmd->bLoc);

	return sizeof(*cmd);
}

static DWORD On_PLRLEVEL(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (cmd->wParam1 <= MAXCHARLEVEL && pnum != myplr)
		plr[pnum]._pLevel = cmd->wParam1;

	return sizeof(*cmd);
}

static DWORD On_DROPITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		delta_put_item(cmd, cmd->x, cmd->y, plr[pnum].plrlevel);

	return sizeof(*cmd);
}

static DWORD On_SEND_PLRINFO(TCmd *pCmd, int pnum)
{
	TCmdPlrInfoHdr *cmd = (TCmdPlrInfoHdr *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, cmd->wBytes + sizeof(*cmd));
	else
		recv_plrinfo(pnum, cmd, cmd->bCmd == CMD_ACK_PLRINFO);

	return cmd->wBytes + sizeof(*cmd);
}

static DWORD On_ACK_PLRINFO(TCmd *pCmd, int pnum)
{
	return On_SEND_PLRINFO(pCmd, pnum);
}

static DWORD On_PLAYER_JOINLEVEL(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;
	PlayerStruct* p;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		p = &plr[pnum];
		p->_pLvlChanging = FALSE;
		if (p->_pName[0] && !p->plractive) {
			p->plractive = TRUE;
			gbActivePlayers++;
			EventPlrMsg("Player '%s' (level %d) just joined the game", p->_pName, p->_pLevel);
		}

		if (p->plractive && myplr != pnum) {
			p->_px = cmd->x;
			p->_py = cmd->y;
			p->plrlevel = cmd->wParam1;
			p->_pGFXLoad = 0;
			if (currlevel == p->plrlevel) {
				LoadPlrGFX(pnum, PFILE_STAND);
				SyncInitPlr(pnum);
				if ((p->_pHitPoints >> 6) > 0)
					PlrStartStand(pnum, 0);
				else {
					p->_pgfxnum = 0;
					LoadPlrGFX(pnum, PFILE_DEATH);
					p->_pmode = PM_DEATH;
					NewPlrAnim(pnum, p->_pDAnim[DIR_S], p->_pDFrames, 1, p->_pDWidth);
					p->_pAnimFrame = p->_pAnimLen - 1;
					p->_pVar8 = p->_pAnimLen << 1;
					dFlags[p->_px][p->_py] |= BFLAG_DEAD_PLAYER;
				}

				p->_pvid = AddVision(p->_px, p->_py, p->_pLightRad, pnum == myplr);
				p->_plid = -1;
			}
		}
	}

	return sizeof(*cmd);
}

static DWORD On_ACTIVATEPORTAL(TCmd *pCmd, int pnum)
{
	TCmdLocParam3 *cmd = (TCmdLocParam3 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		ActivatePortal(pnum, cmd->x, cmd->y, cmd->wParam1, cmd->wParam2, cmd->wParam3);
		if (pnum != myplr) {
			if (currlevel == 0)
				AddInTownPortal(pnum);
			else if (currlevel == plr[pnum].plrlevel) {
				int i;
				for (i = 0; i < nummissiles; i++) {
					MissileStruct *mis = &missile[missileactive[i]];
					if (mis->_miType == MIS_TOWN && mis->_miSource == pnum) {
						break;
					}
				}
				if (i == nummissiles)
					AddWarpMissile(pnum, cmd->x, cmd->y);
			} else
				RemovePortalMissile(pnum);
		}
		delta_open_portal(pnum, cmd->x, cmd->y, cmd->wParam1, cmd->wParam2, cmd->wParam3);
	}

	return sizeof(*cmd);
}

static DWORD On_DEACTIVATEPORTAL(TCmd *pCmd, int pnum)
{
	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else {
		if (PortalOnLevel(pnum))
			RemovePortalMissile(pnum);
		DeactivatePortal(pnum);
		delta_close_portal(pnum);
	}

	return sizeof(*pCmd);
}

static DWORD On_RETOWN(TCmd *pCmd, int pnum)
{
	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else {
		if (pnum == myplr) {
			deathflag = FALSE;
			gamemenu_off();
		}
		RestartTownLvl(pnum);
	}

	return sizeof(*pCmd);
}

static DWORD On_SETSTR(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		SetPlrStr(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_SETDEX(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		SetPlrDex(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_SETMAG(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		SetPlrMag(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_SETVIT(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		SetPlrVit(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

static DWORD On_STRING(TCmd *pCmd, int pnum)
{
	return On_STRING2(pnum, pCmd);
}

static DWORD On_SYNCQUEST(TCmd *pCmd, int pnum)
{
	TCmdQuest *cmd = (TCmdQuest *)pCmd;

	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (pnum != myplr)
			SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
		sgbDeltaChanged = TRUE;
	}

	return sizeof(*cmd);
}

#ifdef HELLFIRE
static DWORD On_ENDREFLECT(TCmd *pCmd, int pnum)
{
	int i, mi;

	if (gbBufferMsgs != 1 && pnum != myplr && currlevel == plr[pnum].plrlevel) {
		for (i = 0; i < nummissiles; i++) {
			mi = missileactive[i];
			if (missile[mi]._miType == MIS_REFLECT && missile[mi]._miSource == pnum) {
				ClearMissileSpot(mi);
				DeleteMissile(mi, i);
			}
		}
	}

	return sizeof(*pCmd);
}
#endif

static DWORD On_ENDSHIELD(TCmd *pCmd, int pnum)
{
	int i;

	if (gbBufferMsgs != 1 && pnum != myplr && currlevel == plr[pnum].plrlevel) {
		for (i = 0; i < nummissiles; i++) {
			int mi = missileactive[i];
			if (missile[mi]._miType == MIS_MANASHIELD && missile[mi]._miSource == pnum) {
				ClearMissileSpot(mi);
				DeleteMissile(mi, i);
			}
		}
	}

	return sizeof(*pCmd);
}

static DWORD On_CHEAT_EXPERIENCE(TCmd *pCmd, int pnum)
{
#ifdef _DEBUG
	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else if (plr[pnum]._pLevel < MAXCHARLEVEL) {
		plr[pnum]._pExperience = plr[pnum]._pNextExper;
		NextPlrLevel(pnum);
	}
#endif
	return sizeof(*pCmd);
}

static DWORD On_CHEAT_SPELL_LEVEL(TCmd *pCmd, int pnum)
{
#ifdef _DEBUG
	if (gbBufferMsgs == 1)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else
		plr[pnum]._pSplLvl[plr[pnum]._pRSpell]++;
#endif
	return sizeof(*pCmd);
}

static DWORD On_DEBUG(TCmd *pCmd, int pnum)
{
	return sizeof(*pCmd);
}

static DWORD On_NOVA(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (gbBufferMsgs != 1 && currlevel == plr[pnum].plrlevel && pnum != myplr) {
		ClrPlrPath(pnum);
		plr[pnum]._pSpell = SPL_NOVA;
		plr[pnum]._pSplType = RSPLTYPE_INVALID;
		plr[pnum]._pSplFrom = 3;
		plr[pnum].destAction = ACTION_SPELL;
		plr[pnum].destParam1 = cmd->x;
		plr[pnum].destParam2 = cmd->y;
	}

	return sizeof(*cmd);
}

static DWORD On_SETSHIELD(TCmd *pCmd, int pnum)
{
	if (gbBufferMsgs != 1)
		plr[pnum].pManaShield = TRUE;

	return sizeof(*pCmd);
}

static DWORD On_REMSHIELD(TCmd *pCmd, int pnum)
{
	if (gbBufferMsgs != 1)
		plr[pnum].pManaShield = FALSE;

	return sizeof(*pCmd);
}

#ifdef HELLFIRE
static DWORD On_NAKRUL(TCmd *pCmd, int pnum)
{
	if (gbBufferMsgs != 1) {
		operate_lv24_lever();
		IsUberRoomOpened = TRUE;
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
		monster_some_crypt();
	}
	return sizeof(*pCmd);
}

static DWORD On_OPENHIVE(TCmd *pCmd, int pnum)
{
	if (gbBufferMsgs != 1) {
		AddMissile(80, 62, 81, 63, 0, MIS_HIVEEXPC, 0, pnum, 0, 0);
		T_HiveOpen();
	}
	return sizeof(*pCmd);
}

static DWORD On_OPENCRYPT(TCmd *pCmd, int pnum)
{
	if (gbBufferMsgs != 1) {
		T_CryptOpen();
		InitTownTriggers();
		if (currlevel == 0)
			PlaySFX(IS_SARC);
	}
	return sizeof(*pCmd);
}
#endif

DWORD ParseCmd(int pnum, TCmd *pCmd)
{
	sbLastCmd = pCmd->bCmd;
	if (sgwPackPlrOffsetTbl[pnum] != 0 && sbLastCmd != CMD_ACK_PLRINFO && sbLastCmd != CMD_SEND_PLRINFO)
		return 0;

	switch (sbLastCmd) {
	case CMD_SYNCDATA:
		return On_SYNCDATA(pCmd, pnum);
	case CMD_WALKXY:
		return On_WALKXY(pCmd, pnum);
	case CMD_ADDSTR:
		return On_ADDSTR(pCmd, pnum);
	case CMD_ADDDEX:
		return On_ADDDEX(pCmd, pnum);
	case CMD_ADDMAG:
		return On_ADDMAG(pCmd, pnum);
	case CMD_ADDVIT:
		return On_ADDVIT(pCmd, pnum);
	case CMD_SBSPELL:
		return On_SBSPELL(pCmd, pnum);
	case CMD_GOTOGETITEM:
		return On_GOTOGETITEM(pCmd, pnum);
	case CMD_REQUESTGITEM:
		return On_REQUESTGITEM(pCmd, pnum);
	case CMD_GETITEM:
		return On_GETITEM(pCmd, pnum);
	case CMD_GOTOAGETITEM:
		return On_GOTOAGETITEM(pCmd, pnum);
	case CMD_REQUESTAGITEM:
		return On_REQUESTAGITEM(pCmd, pnum);
	case CMD_AGETITEM:
		return On_AGETITEM(pCmd, pnum);
	case CMD_ITEMEXTRA:
		return On_ITEMEXTRA(pCmd, pnum);
	case CMD_PUTITEM:
		return On_PUTITEM(pCmd, pnum);
	case CMD_SYNCPUTITEM:
		return On_SYNCPUTITEM(pCmd, pnum);
	case CMD_RESPAWNITEM:
		return On_RESPAWNITEM(pCmd, pnum);
	case CMD_ATTACKXY:
		return On_ATTACKXY(pCmd, pnum);
	case CMD_SATTACKXY:
		return On_SATTACKXY(pCmd, pnum);
	case CMD_RATTACKXY:
		return On_RATTACKXY(pCmd, pnum);
	case CMD_SPELLXYD:
		return On_SPELLXYD(pCmd, pnum);
	case CMD_SPELLXY:
		return On_SPELLXY(pCmd, pnum);
	case CMD_TSPELLXY:
		return On_TSPELLXY(pCmd, pnum);
	case CMD_OPOBJXY:
		return On_OPOBJXY(pCmd, pnum);
	case CMD_DISARMXY:
		return On_DISARMXY(pCmd, pnum);
	case CMD_OPOBJT:
		return On_OPOBJT(pCmd, pnum);
	case CMD_ATTACKID:
		return On_ATTACKID(pCmd, pnum);
	case CMD_ATTACKPID:
		return On_ATTACKPID(pCmd, pnum);
	case CMD_RATTACKID:
		return On_RATTACKID(pCmd, pnum);
	case CMD_RATTACKPID:
		return On_RATTACKPID(pCmd, pnum);
	case CMD_SPELLID:
		return On_SPELLID(pCmd, pnum);
	case CMD_SPELLPID:
		return On_SPELLPID(pCmd, pnum);
	case CMD_TSPELLID:
		return On_TSPELLID(pCmd, pnum);
	case CMD_TSPELLPID:
		return On_TSPELLPID(pCmd, pnum);
	case CMD_KNOCKBACK:
		return On_KNOCKBACK(pCmd, pnum);
	case CMD_RESURRECT:
		return On_RESURRECT(pCmd, pnum);
	case CMD_HEALOTHER:
		return On_HEALOTHER(pCmd, pnum);
	case CMD_TALKXY:
		return On_TALKXY(pCmd, pnum);
	case CMD_DEBUG:
		return On_DEBUG(pCmd, pnum);
	case CMD_NEWLVL:
		return On_NEWLVL(pCmd, pnum);
	case CMD_WARP:
		return On_WARP(pCmd, pnum);
	case CMD_MONSTDEATH:
		return On_MONSTDEATH(pCmd, pnum);
	case CMD_KILLGOLEM:
		return On_KILLGOLEM(pCmd, pnum);
	case CMD_AWAKEGOLEM:
		return On_AWAKEGOLEM(pCmd, pnum);
	case CMD_MONSTDAMAGE:
		return On_MONSTDAMAGE(pCmd, pnum);
	case CMD_PLRDEAD:
		return On_PLRDEAD(pCmd, pnum);
	case CMD_PLRDAMAGE:
		return On_PLRDAMAGE(pCmd, pnum);
	case CMD_OPENDOOR:
		return On_OPENDOOR(pCmd, pnum);
	case CMD_CLOSEDOOR:
		return On_CLOSEDOOR(pCmd, pnum);
	case CMD_OPERATEOBJ:
		return On_OPERATEOBJ(pCmd, pnum);
	case CMD_PLROPOBJ:
		return On_PLROPOBJ(pCmd, pnum);
	case CMD_BREAKOBJ:
		return On_BREAKOBJ(pCmd, pnum);
	case CMD_CHANGEPLRITEMS:
		return On_CHANGEPLRITEMS(pCmd, pnum);
	case CMD_DELPLRITEMS:
		return On_DELPLRITEMS(pCmd, pnum);
	case CMD_PLRLEVEL:
		return On_PLRLEVEL(pCmd, pnum);
	case CMD_DROPITEM:
		return On_DROPITEM(pCmd, pnum);
	case CMD_ACK_PLRINFO:
		return On_ACK_PLRINFO(pCmd, pnum);
	case CMD_SEND_PLRINFO:
		return On_SEND_PLRINFO(pCmd, pnum);
	case CMD_PLAYER_JOINLEVEL:
		return On_PLAYER_JOINLEVEL(pCmd, pnum);
	case CMD_ACTIVATEPORTAL:
		return On_ACTIVATEPORTAL(pCmd, pnum);
	case CMD_DEACTIVATEPORTAL:
		return On_DEACTIVATEPORTAL(pCmd, pnum);
	case CMD_RETOWN:
		return On_RETOWN(pCmd, pnum);
	case CMD_SETSTR:
		return On_SETSTR(pCmd, pnum);
	case CMD_SETMAG:
		return On_SETMAG(pCmd, pnum);
	case CMD_SETDEX:
		return On_SETDEX(pCmd, pnum);
	case CMD_SETVIT:
		return On_SETVIT(pCmd, pnum);
	case CMD_STRING:
		return On_STRING(pCmd, pnum);
	case CMD_SYNCQUEST:
		return On_SYNCQUEST(pCmd, pnum);
	case CMD_ENDSHIELD:
		return On_ENDSHIELD(pCmd, pnum);
	case CMD_CHEAT_EXPERIENCE:
		return On_CHEAT_EXPERIENCE(pCmd, pnum);
	case CMD_CHEAT_SPELL_LEVEL:
		return On_CHEAT_SPELL_LEVEL(pCmd, pnum);
#ifdef HELLFIRE
	case CMD_ENDREFLECT:
		return On_ENDREFLECT(pCmd, pnum);
	case CMD_NAKRUL:
		return On_NAKRUL(pCmd, pnum);
	case CMD_OPENHIVE:
		return On_OPENHIVE(pCmd, pnum);
	case CMD_OPENCRYPT:
		return On_OPENCRYPT(pCmd, pnum);
#else
	case CMD_NOVA:
		return On_NOVA(pCmd, pnum);
	case CMD_SETSHIELD:
		return On_SETSHIELD(pCmd, pnum);
	case CMD_REMSHIELD:
		return On_REMSHIELD(pCmd, pnum);
#endif
	}

	if (sbLastCmd < CMD_DLEVEL_0 || sbLastCmd > CMD_DLEVEL_END) {
		SNetDropPlayer(pnum, LEAVE_DROP);
		return 0;
	}

	return On_DLEVEL(pnum, pCmd);
}

DEVILUTION_END_NAMESPACE
