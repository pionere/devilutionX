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
static bool _gbDeltaChanged;
static BYTE sgbDeltaChunks;
BOOL deltaload;
_msg_mode geBufferMsgs;
char gbNetMsg[MAX_SEND_STR_LEN];

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
	int dataLeft, pktSize;
	TMegaPkt *pkt;

	i = -1;
	for (pkt = sgpMegaPkt; pkt != NULL; pkt = pkt->pNext) {
		dataLeft = sizeof(pkt->data) - pkt->dwSpaceLeft;
		BYTE *data = pkt->data;
		while (dataLeft != 0) {
			if (*data == FAKE_CMD_SETID) {
				TFakeCmdPlr *cmd = (TFakeCmdPlr *)data;
				i = cmd->bPlr;
				pktSize = sizeof(*cmd);
			} else if (*data == FAKE_CMD_DROPID) {
				TFakeDropPlr *cmd = (TFakeDropPlr *)data;
				multi_player_left(cmd->bPlr, cmd->dwReason);
				pktSize = sizeof(*cmd);
			} else {
				pktSize = ParseCmd(i, (TCmd *)data);
			}
			data += pktSize;
			dataLeft -= pktSize;
		}
	}
}

static void msg_send_packet(int pnum, const void *packet, unsigned dwSize)
{
	TFakeCmdPlr cmd;

	if (pnum != sgnCurrMegaPlayer) {
		sgnCurrMegaPlayer = pnum;
		cmd.bCmd = FAKE_CMD_SETID;
		cmd.bPlr = pnum;
		msg_send_packet(pnum, &cmd, sizeof(cmd));
	}
	if (sgpCurrPkt->dwSpaceLeft < dwSize)
		msg_get_next_packet();

	memcpy(sgpCurrPkt->data + sizeof(sgpCurrPkt->data) - sgpCurrPkt->dwSpaceLeft, packet, dwSize);
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
	bool received;
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
	if (nthread_has_500ms_passed())
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

bool msg_wait_resync()
{
	bool success;

	msg_get_next_packet();
	sgbDeltaChunks = 0;
	sgnCurrMegaPlayer = -1;
	sgbRecvCmd = CMD_DLEVEL_END;
	geBufferMsgs = MSG_DOWNLOAD_DELTA;
	sgdwOwnerWait = SDL_GetTicks();
	success = UiProgressDialog("Waiting for game data...", 1, msg_wait_for_turns, 20);
	geBufferMsgs = MSG_NORMAL;
	if (!success) {
		msg_free_packets();
		return false;
	}

	if (gbGameDestroyed) {
		DrawDlg("The game ended");
		msg_free_packets();
		return false;
	}

	if (sgbDeltaChunks != MAX_CHUNKS) {
		DrawDlg("Unable to get level data");
		msg_free_packets();
		return false;
	}

	return true;
}

void run_delta_info()
{
	if (gbMaxPlayers == 1)
		return;

	geBufferMsgs = MSG_RUN_DELTA;
	msg_pre_packet();
	geBufferMsgs = MSG_NORMAL;
	msg_free_packets();
}

static BYTE *DeltaExportItem(BYTE *dst, TCmdPItem *src)
{
	int i;

	for (i = 0; i < MAXITEMS; i++, src++) {
		if (src->bCmd == 0xFF) {
			*dst = 0xFF;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<TCmdPItem *>(dst), *src);
			dst += sizeof(TCmdPItem);
		}
	}

	return dst;
}

static BYTE *DeltaImportItem(BYTE *src, TCmdPItem *dst)
{
	int i;

	for (i = 0; i < MAXITEMS; i++, dst++) {
		if (*src == 0xFF) {
			memset(dst, 0xFF, sizeof(TCmdPItem));
			src++;
		} else {
			copy_pod(*dst, *reinterpret_cast<TCmdPItem *>(src));
			// TODO: validate data from internet
			// assert(dst->bCmd == DCMD_SPAWNED || dst->bCmd == DCMD_TAKEN || dst->bCmd == DCMD_DROPPED);
			src += sizeof(TCmdPItem);
		}
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

	for (i = 0; i < MAXMONSTERS; i++, src++) {
		if (src->_mx == 0xFF) {
			*dst = 0xFF;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DMonsterStr *>(dst), *src);
			dst += sizeof(DMonsterStr);
		}
	}

	return dst;
}

static BYTE *DeltaImportMonster(BYTE *src, DMonsterStr *dst)
{
	int i;

	for (i = 0; i < MAXMONSTERS; i++, dst++) {
		if (*src == 0xFF) {
			memset(dst, 0xFF, sizeof(DMonsterStr));
			src++;
		} else {
			copy_pod(*dst, *reinterpret_cast<DMonsterStr *>(src));
			src += sizeof(DMonsterStr);
		}
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
	for (i = 0; i < NUM_QUESTS; i++) {
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
			SetPortalStats(i, false, 0, 0, 0, DTYPE_TOWN);
		} else {
			copy_pod(*pD, *reinterpret_cast<DPortal *>(src));
			src += sizeof(*pD);
			SetPortalStats(i, true,	pD->x, pD->y, pD->level, pD->ltype);
		}
	}

	mq = sgJunk.quests;
	for (i = 0; i < NUM_QUESTS; i++) {
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

	if (_gbDeltaChanged) {
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
		app_fatal("Unkown network message type: %d", cmd);
	}

	sgbDeltaChunks++;
	_gbDeltaChanged = true;
}

static unsigned On_DLEVEL(int pnum, TCmd *pCmd)
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
	_gbDeltaChanged = false;
	memset(&sgJunk, 0xFF, sizeof(sgJunk));
	memset(sgLevels, 0xFF, sizeof(sgLevels));
	memset(sgLocals, 0, sizeof(sgLocals));
	deltaload = FALSE;
}

static void delta_kill_monster(int mnum, BYTE x, BYTE y, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1)
		return;

	_gbDeltaChanged = true;
	pD = &sgLevels[bLevel].monster[mnum];
	pD->_mx = x;
	pD->_my = y;
	pD->_mdir = monster[mnum]._mdir;
	pD->_mhitpoints = 0;
}

static void delta_monster_hp(int mnum, int hp, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1)
		return;

	_gbDeltaChanged = true;
	pD = &sgLevels[bLevel].monster[mnum];
	// In vanilla code the value was discarded if hp was higher than the current one.
	// That disregards the healing monsters.
	// Now it is always updated except the monster is already dead.
	if (pD->_mhitpoints != 0)
		pD->_mhitpoints = hp;
}

void delta_sync_monster(const TSyncMonster *pSync, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1)
		return;

	/// ASSERT: assert(pSync != NULL);
	/// ASSERT: assert(bLevel < NUMLEVELS);

	pD = &sgLevels[bLevel].monster[pSync->_mndx];
	if (pD->_mhitpoints == 0)
		return;

	_gbDeltaChanged = true;
	pD->_mx = pSync->_mx;
	pD->_my = pSync->_my;
	pD->_mactive = UCHAR_MAX;
	pD->_menemy = pSync->_menemy;
}

static void delta_sync_golem(TCmdGolem *pG, int mnum, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1)
		return;

	_gbDeltaChanged = true;
	pD = &sgLevels[bLevel].monster[mnum];
	pD->_mx = pG->_mx;
	pD->_my = pG->_my;
	pD->_mactive = UCHAR_MAX;
	pD->_menemy = pG->_menemy;
	pD->_mdir = pG->_mdir;
	pD->_mhitpoints = pG->_mhitpoints;
}

static void delta_leave_sync(BYTE bLevel)
{
	int i, mnum;
	DMonsterStr *pD;

	if (bLevel == 0) {
		glSeedTbl[0] = GetRndSeed();
		return;
	}
	for (i = 0; i < nummonsters; i++) {
		mnum = monstactive[i];
		if (monster[mnum]._mhitpoints != 0) {
			_gbDeltaChanged = true;
			pD = &sgLevels[bLevel].monster[mnum];
			pD->_menemy = encode_enemy(mnum);
			pD->_mx = monster[mnum]._mx;
			pD->_my = monster[mnum]._my;
			pD->_mdir = monster[mnum]._mdir;
			pD->_mhitpoints = monster[mnum]._mhitpoints;
			pD->_mactive = monster[mnum]._msquelch;
		}
	}
	memcpy(&sgLocals[bLevel].automapsv, automapview, sizeof(automapview));
}

static void delta_sync_object(int oi, BYTE bCmd, BYTE bLevel)
{
	if (gbMaxPlayers == 1)
		return;

	_gbDeltaChanged = true;
	sgLevels[bLevel].object[oi].bCmd = bCmd;
}

static bool delta_get_item(TCmdGItem *pI, BYTE bLevel)
{
	TCmdPItem *pD;
	int i;

	if (gbMaxPlayers == 1)
		return true;

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF || pD->wIndx != pI->wIndx || pD->wCI != pI->wCI || pD->dwSeed != pI->dwSeed)
			continue;

		switch (pD->bCmd) {
		case DCMD_TAKEN:
			return true;
		case DCMD_SPAWNED:
			_gbDeltaChanged = true;
			pD->bCmd = DCMD_TAKEN;
			return true;
		case DCMD_DROPPED:
			_gbDeltaChanged = true;
			pD->bCmd = 0xFF;
			return true;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}

	if ((pI->wCI & CF_PREGEN) == 0)
		return false;

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			_gbDeltaChanged = true;
			pD->bCmd = DCMD_TAKEN;
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
			pD->bMinDam = pI->bMinDam;
			pD->bMaxDam = pI->bMaxDam;
			pD->bMinStr = pI->bMinStr;
			pD->bMinMag = pI->bMinMag;
			pD->bMinDex = pI->bMinDex;
			pD->bAC = pI->bAC;
#endif
			break;
		}
	}
	return true;
}

static void delta_put_item(TCmdPItem *pI, int x, int y, BYTE bLevel)
{
	int i;
	TCmdPItem *pD;

	if (gbMaxPlayers == 1)
		return;

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != 0xFF
		 && pD->wIndx == pI->wIndx
		 && pD->wCI == pI->wCI
		 && pD->dwSeed == pI->dwSeed) {
			if (pD->bCmd == DCMD_DROPPED)
				return;
			if (pD->bCmd == DCMD_TAKEN)
				continue; // BUGFIX: should return instead? otherwise the item is duped...
			app_fatal("Trying to drop a floor item?");
		}
	}

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			_gbDeltaChanged = true;
			copy_pod(*pD, *pI);
			pD->bCmd = DCMD_DROPPED;
			pD->x = x;
			pD->y = y;
			return;
		}
	}
}

bool delta_portal_inited(int i)
{
	return sgJunk.portal[i].x == 0xFF;
}

bool delta_quest_inited(int i)
{
	return sgJunk.quests[i].qstate != 0xFF;
}

static void PackPItem(TCmdPItem *dest, ItemStruct *src)
{
	dest->wIndx = src->_iIdx;

	if (src->_iIdx == IDI_EAR) {
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
		dest->bMinDam = src->_iMinDam;
		dest->bMaxDam = src->_iMaxDam;
		dest->bMinStr = src->_iMinStr;
		dest->bMinMag = src->_iMinMag;
		dest->bMinDex = src->_iMinDex;
		dest->bAC = src->_iAC;
#endif
	}
}

static void PackGItem(TCmdGItem *dest, ItemStruct *src)
{
	dest->wIndx = src->_iIdx;

	if (src->_iIdx == IDI_EAR) {
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
		dest->bMinDam = src->_iMinDam;
		dest->bMaxDam = src->_iMaxDam;
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
		 && pD->wIndx == is->_iIdx
		 && pD->wCI == is->_iCreateInfo
		 && pD->dwSeed == is->_iSeed) {
			if (pD->bCmd == DCMD_TAKEN || pD->bCmd == DCMD_SPAWNED)
				return;
			if (pD->bCmd == DCMD_DROPPED)
				continue; // BUGFIX: should return instead? otherwise the item is duped...
		}
	}

	pD = sgLevels[currlevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			_gbDeltaChanged = true;
			pD->bCmd = DCMD_SPAWNED;
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

	assert(gbMaxPlayers != 1);
	for (i = 0; i < MAX_PLRS; i++) {
		if (i != myplr)
			plr[i]._pGFXLoad = 0;
	}
	plr[myplr]._pLvlVisited[currlevel] = TRUE;
	delta_leave_sync(currlevel);
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
		item[MAXITEMS]._iMinDam = src->bMinDam;
		item[MAXITEMS]._iMaxDam = src->bMaxDam;
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
		item[MAXITEMS]._iMinDam = src->bMinDam;
		item[MAXITEMS]._iMaxDam = src->bMaxDam;
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
	int ii;
	int i;
	int x, y;

	assert(gbMaxPlayers != 1);

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
					// SetDead: inline for better performance
					if (mon->mlid != 0)
						ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
					AddDead(i);
				} else {
					decode_enemy(i, mstr->_menemy);
					if (i >= MAX_MINIONS || !(MINION_INACTIVE(mon)))
						dMonster[mon->_mx][mon->_my] = i + 1;
					MonStartStand(i, mon->_mdir);
					mon->_msquelch = mstr->_mactive;
				}
			}
		}
		// SetDead();

		memcpy(automapview, &sgLocals[currlevel], sizeof(automapview));

		dstr = sgLevels[currlevel].object;
		for (i = 0; i < MAXOBJECTS; i++, dstr++) {
			if (dstr->bCmd != 0xFF) {
				switch (dstr->bCmd) {
				case CMD_OPERATEOBJ:
					SyncOpObject(-1, i);
					break;
				case CMD_DOOROPEN:
					SyncDoorOpen(i);
					break;
				case CMD_DOORCLOSE:
					SyncDoorClose(i);
					break;
				case CMD_TRAPDISABLE:
					SyncTrapDisable(i);
					break;
				case CMD_TRAPOPEN:
					SyncTrapOpen(i);
					break;
				case CMD_TRAPCLOSE:
					SyncTrapClose(i);
					break;
				case CMD_CHESTCLOSE:
					SyncChestClose(i);
					break;
				default:
					ASSUME_UNREACHABLE
					break;
				}
			}
		}
	}

	// load items last, because they depend on the object state
	itm = sgLevels[currlevel].item;
	for (i = 0; i < MAXITEMS; i++, itm++) {
		if (itm->bCmd == DCMD_TAKEN) {
			ii = FindGetItem(
				itm->wIndx,
				itm->wCI,
				itm->dwSeed);
			if (ii != -1) {
				if (dItem[item[ii]._ix][item[ii]._iy] == ii + 1)
					dItem[item[ii]._ix][item[ii]._iy] = 0;
				DeleteItem(ii, i);
			}
		} else if (itm->bCmd == DCMD_DROPPED) {
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
			RespawnItem(ii, false);
			numitems++;
		}
	}

	deltaload = FALSE;
}

void NetSendCmd(bool bHiPri, BYTE bCmd)
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

void NetSendCmdLoc(bool bHiPri, BYTE bCmd, BYTE x, BYTE y)
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

void NetSendCmdLocBParam1(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE bParam1)
{
	TCmdLocBParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.bParam1 = bParam1;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLocParam1(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1)
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

void NetSendCmdLocParam2(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, WORD wParam1, WORD wParam2)
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

void NetSendCmdLocBParam2(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE bParam1, BYTE bParam2)
{
	TCmdLocBParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.bParam1 = bParam1;
	cmd.bParam2 = bParam2;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLocBParam3(bool bHiPri, BYTE bCmd, BYTE x, BYTE y, BYTE bParam1, BYTE bParam2, BYTE bParam3)
{
	TCmdLocBParam3 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.bParam1 = bParam1;
	cmd.bParam2 = bParam2;
	cmd.bParam3 = bParam3;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdParam1(bool bHiPri, BYTE bCmd, WORD wParam1)
{
	ALIGN_BY_1 TCmdParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = wParam1;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdParam2(bool bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2)
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

void NetSendCmdParam3(bool bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2, WORD wParam3)
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

void NetSendCmdWBParam4(bool bHiPri, BYTE bCmd, WORD wParam1, BYTE bParam2, BYTE bParam3, BYTE bParam4)
{
	TCmdWBParam4 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = wParam1;
	cmd.bParam2 = bParam2;
	cmd.bParam3 = bParam3;
	cmd.bParam4 = bParam4;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdBParam1(bool bHiPri, BYTE bCmd, BYTE bParam1)
{
	TCmdBParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.bParam1 = bParam1;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdBParam2(bool bHiPri, BYTE bCmd, BYTE bParam1, BYTE bParam2)
{
	TCmdBParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.bParam1 = bParam1;
	cmd.bParam2 = bParam2;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdBParam3(bool bHiPri, BYTE bCmd, BYTE bParam1, BYTE bParam2, BYTE bParam3)
{
	TCmdBParam3 cmd;

	cmd.bCmd = bCmd;
	cmd.bParam1 = bParam1;
	cmd.bParam2 = bParam2;
	cmd.bParam3 = bParam3;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdQuest(bool bHiPri, BYTE q)
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

void NetSendCmdGItem(bool bHiPri, BYTE bCmd, BYTE mast, BYTE pnum, BYTE ii)
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

static void NetSendCmdGItem2(bool usonly, BYTE bCmd, BYTE mast, BYTE pnum, TCmdGItem *p)
{
	int ticks;
	TCmdGItem cmd;

	copy_pod(cmd, *p);
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

	tmsg_add(&cmd);
}

static bool NetSendCmdReq2(BYTE bCmd, BYTE mast, BYTE pnum, TCmdGItem *p)
{
	int ticks;
	TCmdGItem cmd;

	copy_pod(cmd, *p);
	cmd.bCmd = bCmd;
	cmd.bPnum = pnum;
	cmd.bMaster = mast;

	ticks = SDL_GetTicks();
	if (cmd.dwTime == 0)
		cmd.dwTime = ticks;
	else if (ticks - cmd.dwTime > 5000)
		return false;

	tmsg_add(&cmd);

	return true;
}

static void NetSendCmdExtra(TCmdGItem *p)
{
	TCmdGItem cmd;

	copy_pod(cmd, *p);
	cmd.dwTime = 0;
	cmd.bCmd = CMD_ITEMEXTRA;
	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdPItem(bool bHiPri, BYTE bCmd, ItemStruct *is, BYTE x, BYTE y)
{
	TCmdPItem cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;

	PackPItem(&cmd, is);

	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdChItem(bool bHiPri, ItemStruct *is, BYTE bLoc)
{
	TCmdChItem cmd;

	cmd.bCmd = CMD_CHANGEPLRITEMS;
	cmd.bLoc = bLoc;
	cmd.wIndx = is->_iIdx;
	cmd.wCI = is->_iCreateInfo;
	cmd.dwSeed = is->_iSeed;
	cmd.bId = is->_iIdentified;

	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdDelItem(bool bHiPri, BYTE bLoc)
{
	TCmdDelItem cmd;

	cmd.bLoc = bLoc;
	cmd.bCmd = CMD_DELPLRITEMS;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdDItem(bool bHiPri, int ii)
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

static bool i_own_level(int nReqLevel)
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive
		 && !plr[i]._pLvlChanging
		 && plr[i].plrlevel == nReqLevel
		 && (i != myplr || geBufferMsgs == MSG_NORMAL))
			break;
	}
	return i == myplr;
}

void NetSendCmdDwParam2(bool bHiPri, BYTE bCmd, DWORD dwParam1, DWORD dwParam2)
{
	TCmdDwParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.dwParam1 = dwParam1;
	cmd.dwParam2 = dwParam2;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdDwParam3(bool bHiPri, BYTE bCmd, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	TCmdDwParam3 cmd;

	cmd.bCmd = bCmd;
	cmd.dwParam1 = dwParam1;
	cmd.dwParam2 = dwParam2;
	cmd.dwParam3 = dwParam3;
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdString(unsigned int pmask)
{
	int dwStrLen;
	TCmdString cmd;

	dwStrLen = strlen(gbNetMsg);
	cmd.bCmd = CMD_STRING;
	memcpy(cmd.str, gbNetMsg, dwStrLen + 1);
	multi_send_msg_packet(pmask, (BYTE *)&cmd.bCmd, dwStrLen + 2);
}

static unsigned On_STRING2(int pnum, TCmd *pCmd)
{
	TCmdString *cmd = (TCmdString *)pCmd;

	int len = strlen(cmd->str);
	if (geBufferMsgs == MSG_NORMAL)
		SendPlrMsg(pnum, cmd->str);

	return len + 2; // length of string + nul terminator + sizeof(cmd->bCmd)
}

static void delta_open_portal(int pnum, BYTE x, BYTE y, BYTE bLevel, BYTE bLType, BYTE bSetLvl)
{
	_gbDeltaChanged = true;
	sgJunk.portal[pnum].x = x;
	sgJunk.portal[pnum].y = y;
	sgJunk.portal[pnum].level = bLevel;
	sgJunk.portal[pnum].ltype = bLType;
	sgJunk.portal[pnum].setlvl = bSetLvl;
}

void delta_close_portal(int pnum)
{
	memset(&sgJunk.portal[pnum], 0xFF, sizeof(sgJunk.portal[pnum]));
	_gbDeltaChanged = true;
}

static void check_update_plr(int pnum)
{
	if (gbMaxPlayers != 1 && pnum == myplr)
		pfile_update(true);
}

static void msg_errorf(const char *pszFmt, ...)
{
#ifdef _DEBUG
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
#endif
}

static unsigned On_SYNCDATA(TCmd *pCmd, int pnum)
{
	return sync_update(pnum, (const BYTE *)pCmd);
}

static unsigned On_WALKXY(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		MakePlrPath(pnum, cmd->x, cmd->y, true);
		plr[pnum].destAction = ACTION_NONE;
	}

	return sizeof(*cmd);
}

static unsigned On_ADDSTR(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else
		IncreasePlrStr(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDMAG(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else
		IncreasePlrMag(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDDEX(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else
		IncreasePlrDex(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDVIT(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else
		IncreasePlrVit(pnum);

	return sizeof(*pCmd);
}

static unsigned On_BLOCK(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		plr[pnum].destAction = ACTION_BLOCK;
		plr[pnum].destParam1 = cmd->wParam1; // direction
	}
	return sizeof(*cmd);
}

static unsigned On_GOTOGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr[pnum].destAction = ACTION_PICKUPITEM;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static unsigned On_REQUESTGITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && i_own_level(plr[pnum].plrlevel)) {
		if (GetItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx)) {
			int ii = FindGetItem(cmd->wIndx, cmd->wCI, cmd->dwSeed);
			if (ii != -1) {
				NetSendCmdGItem2(false, CMD_GETITEM, myplr, cmd->bPnum, cmd);
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

static unsigned On_GETITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
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
			NetSendCmdGItem2(true, CMD_GETITEM, cmd->bMaster, cmd->bPnum, cmd);
	}

	return sizeof(*cmd);
}

static unsigned On_GOTOAGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr[pnum].destAction = ACTION_PICKUPAITEM;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static unsigned On_REQUESTAGITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && i_own_level(plr[pnum].plrlevel)) {
		if (GetItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx)) {
			int ii = FindGetItem(cmd->wIndx, cmd->wCI, cmd->dwSeed);
			if (ii != -1) {
				NetSendCmdGItem2(false, CMD_AGETITEM, myplr, cmd->bPnum, cmd);
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

static unsigned On_AGETITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
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
			NetSendCmdGItem2(true, CMD_AGETITEM, cmd->bMaster, cmd->bPnum, cmd);
	}

	return sizeof(*cmd);
}

static unsigned On_ITEMEXTRA(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		delta_get_item(cmd, cmd->bLevel);
		if (currlevel == plr[pnum].plrlevel)
			SyncGetItem(cmd->x, cmd->y, cmd->wIndx, cmd->wCI, cmd->dwSeed);
	}

	return sizeof(*cmd);
}

static unsigned On_PUTITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
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
	} else {
		PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
		delta_put_item(cmd, cmd->x, cmd->y, plr[pnum].plrlevel);
		check_update_plr(pnum);
	}

	return sizeof(*cmd);
}

static unsigned On_SYNCPUTITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (currlevel == plr[pnum].plrlevel) {
		UnPackPItem(cmd);
		int ii = SyncPutItem(pnum, cmd->x, cmd->y, &item[MAXITEMS]);
		if (ii != -1) {
			PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
			delta_put_item(cmd, item[ii]._ix, item[ii]._iy, plr[pnum].plrlevel);
			check_update_plr(pnum);
		}
	} else {
		PutItemRecord(cmd->dwSeed, cmd->wCI, cmd->wIndx);
		delta_put_item(cmd, cmd->x, cmd->y, plr[pnum].plrlevel);
		check_update_plr(pnum);
	}

	return sizeof(*cmd);
}

static unsigned On_RESPAWNITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
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

static unsigned On_ATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLocBParam2 *cmd = (TCmdLocBParam2 *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		sn = cmd->bParam1;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_ATTACK;
			plr[pnum].destParam1 = cmd->x;
			plr[pnum].destParam2 = cmd->y;
			plr[pnum].destParam3 = sn;           // attack skill
			plr[pnum].destParam4 = cmd->bParam2; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLocBParam2 *cmd = (TCmdLocBParam2 *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->bParam1;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_ATTACK;
			plr[pnum].destParam1 = cmd->x;
			plr[pnum].destParam2 = cmd->y;
			plr[pnum].destParam3 = sn;           // attack skill
			plr[pnum].destParam4 = cmd->bParam2; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLocBParam2 *cmd = (TCmdLocBParam2 *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->bParam1;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_RATTACK;
			plr[pnum].destParam1 = cmd->x;
			plr[pnum].destParam2 = cmd->y;
			plr[pnum].destParam3 = sn;           // attack skill
			plr[pnum].destParam4 = cmd->bParam2; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLXY(TCmd *pCmd, int pnum)
{
	TCmdLocBParam3 *cmd = (TCmdLocBParam3 *)pCmd;
	BYTE sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->bParam1;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_SPELL;
			plr[pnum].destParam2 = cmd->x;
			plr[pnum].destParam3 = cmd->y;
			plr[pnum].destParam1a = sn;           // spell
			plr[pnum].destParam1b = cmd->bParam2; // invloc
			plr[pnum].destParam1c = cmd->bParam3; // spllvl
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_DOOIL(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA) {
		DoOil(pnum, cmd->bParam1, cmd->bParam2);
	}

	return sizeof(*cmd);
}

static unsigned On_OPOBJXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		int oi = cmd->wParam1;
		plr[pnum].destAction = ACTION_OPERATE;
		plr[pnum].destParam1 = oi;
		plr[pnum].destParam2 = cmd->x;
		plr[pnum].destParam3 = cmd->y;
		MakePlrPath(pnum, cmd->x, cmd->y, !(object[oi]._oSolidFlag | object[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_DISARMXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		int oi = cmd->wParam1;
		plr[pnum].destAction = ACTION_DISARM;
		plr[pnum].destParam1 = oi;
		plr[pnum].destParam2 = cmd->x;
		plr[pnum].destParam3 = cmd->y;
		MakePlrPath(pnum, cmd->x, cmd->y, !(object[oi]._oSolidFlag | object[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_OPOBJT(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		plr[pnum].destAction = ACTION_OPERATETK;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKID(TCmd *pCmd, int pnum)
{
	TCmdParam3 *cmd = (TCmdParam3 *)pCmd;
	int mnum, x, y, sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		mnum = cmd->wParam1;
		x = monster[mnum]._mfutx;
		y = monster[mnum]._mfuty;
		if (abs(plr[pnum]._px - x) > 1 || abs(plr[pnum]._py - y) > 1)
			MakePlrPath(pnum, x, y, false);
		sn = cmd->wParam2;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_ATTACKMON;
			plr[pnum].destParam1 = mnum;
			plr[pnum].destParam2 = sn;           // attack skill
			plr[pnum].destParam3 = cmd->wParam3; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdBParam3 *cmd = (TCmdBParam3 *)pCmd;
	int tnum, sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		tnum = cmd->bParam1;
		MakePlrPath(pnum, plr[tnum]._pfutx, plr[tnum]._pfuty, false);
		sn = cmd->bParam2;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_ATTACKPLR;
			plr[pnum].destParam1 = tnum;
			plr[pnum].destParam2 = sn;           // attack skill
			plr[pnum].destParam3 = cmd->bParam3; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKID(TCmd *pCmd, int pnum)
{
	TCmdParam3 *cmd = (TCmdParam3 *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->wParam2;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_RATTACKMON;
			plr[pnum].destParam1 = cmd->wParam1; // target id
			plr[pnum].destParam2 = sn;           // attack skill
			plr[pnum].destParam3 = cmd->wParam3; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdBParam3 *cmd = (TCmdBParam3 *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->bParam2;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_RATTACKPLR;
			plr[pnum].destParam1 = cmd->bParam1; // target id
			plr[pnum].destParam2 = sn;           // attack skill
			plr[pnum].destParam3 = cmd->bParam3; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLID(TCmd *pCmd, int pnum)
{
	TCmdWBParam4 *cmd = (TCmdWBParam4 *)pCmd;
	BYTE sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->bParam2;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_SPELLMON;
			plr[pnum].destParam1a = sn; // spell
			plr[pnum].destParam1b = cmd->bParam3; // invloc
			plr[pnum].destParam1c = cmd->bParam4; // spllvl
			plr[pnum].destParam2 = cmd->wParam1; // mnum
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLPID(TCmd *pCmd, int pnum)
{
	TCmdWBParam4 *cmd = (TCmdWBParam4 *)pCmd;
	BYTE sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->bParam2;
		if ((spelldata[sn].sFlags & plr[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			plr[pnum].destAction = ACTION_SPELLPLR;
			plr[pnum].destParam1a = sn; // spell
			plr[pnum].destParam1b = cmd->bParam3; // invloc
			plr[pnum].destParam1c = cmd->bParam4; // spllvl
			plr[pnum].destParam2 = cmd->wParam1; // pnum
		} else
			msg_errorf("%s has cast an illegal spell.", plr[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_KNOCKBACK(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		MonGetKnockback(cmd->wParam1);
		MonStartHit(cmd->wParam1, pnum, 0);
	}

	return sizeof(*cmd);
}

static unsigned On_TALKXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currlevel == plr[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr[pnum].destAction = ACTION_TALK;
		plr[pnum].destParam1 = cmd->wParam1;
	}

	return sizeof(*cmd);
}

static unsigned On_NEWLVL(TCmd *pCmd, int pnum)
{
	TCmdParam2 *cmd = (TCmdParam2 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		StartNewLvl(pnum, cmd->wParam1, cmd->wParam2);

	return sizeof(*cmd);
}

static unsigned On_WARP(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		StartWarpLvl(pnum, cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_MONSTDEATH(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (pnum != myplr && currlevel == plr[pnum].plrlevel)
			MonSyncStartKill(cmd->wParam1, cmd->x, cmd->y, pnum);
		delta_kill_monster(cmd->wParam1, cmd->x, cmd->y, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_AWAKEGOLEM(TCmd *pCmd, int pnum)
{
	TCmdGolem *cmd = (TCmdGolem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr) {
		if (currlevel == plr[pnum].plrlevel) {
			// BUGFIX: is this even necessary? CMD_SPELLXY should have notified us already...
			AddMissile(plr[pnum]._px, plr[pnum]._py, cmd->_mx, cmd->_my, cmd->_mdir, MIS_GOLEM, 0, pnum, 0, 0, 1);
		} else
			delta_sync_golem(cmd, pnum, cmd->_currlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_MONSTDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdDwParam3 *cmd = (TCmdDwParam3 *)pCmd;
	int mnum, hp, nhp;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		mnum = cmd->dwParam1;
		hp = cmd->dwParam2;
		if (pnum != myplr && currlevel == plr[pnum].plrlevel) {
			nhp = monster[mnum]._mhitpoints - cmd->dwParam3;
			if (nhp < hp)
				hp = nhp;
			if (hp < (1 << 6))
				hp = 1 << 6;
			monster[mnum]._mhitpoints = hp;
		}
		delta_monster_hp(mnum, hp, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_PLRDEAD(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		StartPlrKill(pnum, cmd->wParam1);
	else
		check_update_plr(pnum);

	return sizeof(*cmd);
}

static unsigned On_PLRFRIENDY(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr) {
		snprintf(tempstr, sizeof(tempstr), "%s is now %s.", plr[pnum]._pName, cmd->bParam1 ? "friendly" : "hostile");
		ErrorPlrMsg(tempstr);
	}
	return sizeof(*cmd);
}

static unsigned On_PLRDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdDwParam2 *cmd = (TCmdDwParam2 *)pCmd;

	if (cmd->dwParam1 == myplr && geBufferMsgs != MSG_DOWNLOAD_DELTA) {
		if (leveltype != DTYPE_TOWN && currlevel == plr[pnum].plrlevel) {
			if (!plr[myplr]._pInvincible && cmd->dwParam2 <= 192000) {
				PlrDecHp(myplr, cmd->dwParam2, 1);
			}
		}
	}

	return sizeof(*cmd);
}

static unsigned On_DOOROPEN(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != myplr && currlevel == plr[pnum].plrlevel)
		//	SyncDoorOpen(cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_DOOROPEN, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_DOORCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != myplr && currlevel == plr[pnum].plrlevel)
		//	SyncDoorClose(cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_DOORCLOSE, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_TRAPDISABLE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != myplr && currlevel == plr[pnum].plrlevel)
		//	SyncTrapDisable(cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_TRAPDISABLE, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_TRAPOPEN(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != myplr && currlevel == plr[pnum].plrlevel)
		//	SyncTrapOpen(cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_TRAPOPEN, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_TRAPCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != myplr && currlevel == plr[pnum].plrlevel)
		//	SyncTrapClose(cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_TRAPCLOSE, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_OPERATEOBJ(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (pnum != myplr && currlevel == plr[pnum].plrlevel)
			SyncOpObject(pnum, cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_OPERATEOBJ, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_CHESTCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != myplr && currlevel == plr[pnum].plrlevel)
		//	SyncChestClose(cmd->wParam1);
		delta_sync_object(cmd->wParam1, CMD_CHESTCLOSE, plr[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_CHANGEPLRITEMS(TCmd *pCmd, int pnum)
{
	TCmdChItem *cmd = (TCmdChItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		CheckInvSwap(pnum, cmd->bLoc, cmd->wIndx, cmd->wCI, cmd->dwSeed, cmd->bId);

	return sizeof(*cmd);
}

static unsigned On_DELPLRITEMS(TCmd *pCmd, int pnum)
{
	TCmdDelItem *cmd = (TCmdDelItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr)
		inv_update_rem_item(pnum, cmd->bLoc);

	return sizeof(*cmd);
}

static unsigned On_PLRLEVEL(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr && cmd->bParam1 <= MAXCHARLEVEL)
		plr[pnum]._pLevel = cmd->bParam1;

	return sizeof(*cmd);
}

static unsigned On_PLRSKILLLVL(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != myplr && cmd->bParam2 <= MAXSPLLEVEL)
		plr[pnum]._pSkillLvl[cmd->bParam1] = cmd->bParam2;

	return sizeof(*cmd);
}

static unsigned On_DROPITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		delta_put_item(cmd, cmd->x, cmd->y, plr[pnum].plrlevel);

	return sizeof(*cmd);
}

static unsigned On_SEND_PLRINFO(TCmd *pCmd, int pnum)
{
	TCmdPlrInfoHdr *cmd = (TCmdPlrInfoHdr *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, cmd->wBytes + sizeof(*cmd));
	else
		recv_plrinfo(pnum, cmd, cmd->bCmd == CMD_ACK_PLRINFO);

	return cmd->wBytes + sizeof(*cmd);
}

static unsigned On_ACK_PLRINFO(TCmd *pCmd, int pnum)
{
	return On_SEND_PLRINFO(pCmd, pnum);
}

static unsigned On_PLAYER_JOINLEVEL(TCmd *pCmd, int pnum)
{
	TCmdLocBParam1 *cmd = (TCmdLocBParam1 *)pCmd;
	PlayerStruct* p;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		p = &plr[pnum];
		p->_pLvlChanging = FALSE;
		if (!p->plractive) {
			p->plractive = TRUE;
			gbActivePlayers++;
			EventPlrMsg("Player '%s' (level %d) just joined the game", p->_pName, p->_pLevel);
		}
		if (myplr != pnum) {
			p->_px = cmd->x;
			p->_py = cmd->y;
			p->plrlevel = cmd->bParam1;
			p->_pGFXLoad = 0;
			if (currlevel == p->plrlevel) {
				SyncInitPlr(pnum);
				//PlrStartStand(pnum, DIR_S);
				/*LoadPlrGFX(pnum, PFILE_STAND);
				SyncInitPlr(pnum);
				if (p->_pHitPoints >= (1 << 6))
					PlrStartStand(pnum, DIR_S);
				else {
					p->_pgfxnum = ANIM_ID_UNARMED;
					LoadPlrGFX(pnum, PFILE_DEATH);
					p->_pmode = PM_DEATH;
					NewPlrAnim(pnum, p->_pDAnim, DIR_S, p->_pDFrames, 1, p->_pDWidth);
					p->_pAnimFrame = p->_pAnimLen - 1;
					p->_pVar8 = p->_pAnimLen << 1;
					dFlags[p->_px][p->_py] |= BFLAG_DEAD_PLAYER;
				}*/

				//p->_pvid = AddVision(p->_px, p->_py, p->_pLightRad, pnum == myplr);
				//p->_plid = -1;
			}
		}
	}

	return sizeof(*cmd);
}

static unsigned On_ACTIVATEPORTAL(TCmd *pCmd, int pnum)
{
	TCmdLocBParam3 *cmd = (TCmdLocBParam3 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		ActivatePortal(pnum, cmd->x, cmd->y, cmd->bParam1, cmd->bParam2, cmd->bParam3);
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
		delta_open_portal(pnum, cmd->x, cmd->y, cmd->bParam1, cmd->bParam2, cmd->bParam3);
	}

	return sizeof(*cmd);
}

static unsigned On_DEACTIVATEPORTAL(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else {
		if (PortalOnLevel(pnum))
			RemovePortalMissile(pnum);
		DeactivatePortal(pnum);
		delta_close_portal(pnum);
	}

	return sizeof(*pCmd);
}

static unsigned On_RETOWN(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else {
		RestartTownLvl(pnum);
	}

	return sizeof(*pCmd);
}

static unsigned On_STRING(TCmd *pCmd, int pnum)
{
	return On_STRING2(pnum, pCmd);
}

static unsigned On_SYNCQUEST(TCmd *pCmd, int pnum)
{
	TCmdQuest *cmd = (TCmdQuest *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (pnum != myplr)
			SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
		_gbDeltaChanged = true;
	}

	return sizeof(*cmd);
}

static unsigned On_CHEAT_EXPERIENCE(TCmd *pCmd, int pnum)
{
#ifdef _DEBUG
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else if (plr[pnum]._pLevel < MAXCHARLEVEL) {
		plr[pnum]._pExperience = plr[pnum]._pNextExper;
		NextPlrLevel(pnum);
	}
#endif
	return sizeof(*pCmd);
}

static unsigned On_CHEAT_SPELL_LEVEL(TCmd *pCmd, int pnum)
{
#ifdef _DEBUG
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else
		plr[pnum]._pSkillLvl[plr[pnum]._pAltAtkSkill]++;
#endif	
	return sizeof(*pCmd);
}

static unsigned On_DEBUG(TCmd *pCmd, int pnum)
{
	return sizeof(*pCmd);
}

static unsigned On_SETSHIELD(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *p = (TCmdBParam1*)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA)
		plr[pnum]._pManaShield = p->bParam1;

	return sizeof(*p);
}

static unsigned On_REMSHIELD(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs != MSG_DOWNLOAD_DELTA)
		plr[pnum]._pManaShield = 0;

	return sizeof(*pCmd);
}

static unsigned On_RESTOREHPVIT(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs != MSG_DOWNLOAD_DELTA)
		RestorePlrHpVit(pnum);
	return sizeof(*pCmd);
}

#ifdef HELLFIRE
static unsigned On_NAKRUL(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs != MSG_DOWNLOAD_DELTA) {
		DoOpenUberRoom();
		gbUberRoomOpened = true;
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
		//quests[Q_NAKRUL]._qlog = FALSE;
		WakeUberDiablo();
	}
	return sizeof(*pCmd);
}

static unsigned On_OPENHIVE(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs != MSG_DOWNLOAD_DELTA) {
		AddMissile(70 + DBORDERX, 52 + DBORDERY, 71 + DBORDERX, 53 + DBORDERY, 0, MIS_HIVEEXPC, 0, pnum, 0, 0, 0);
		T_HiveOpen();
		InitTownTriggers();
	}
	return sizeof(*pCmd);
}

static unsigned On_OPENCRYPT(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs != MSG_DOWNLOAD_DELTA) {
		T_CryptOpen();
		InitTownTriggers();
		if (currlevel == 0)
			PlaySFX(IS_SARC);
	}
	return sizeof(*pCmd);
}
#endif

unsigned ParseCmd(int pnum, TCmd *pCmd)
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
	case CMD_BLOCK:
		return On_BLOCK(pCmd, pnum);
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
	case CMD_SPELLXY:
		return On_SPELLXY(pCmd, pnum);
	case CMD_DOOIL:
		return On_DOOIL(pCmd, pnum);
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
	case CMD_KNOCKBACK:
		return On_KNOCKBACK(pCmd, pnum);
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
	case CMD_AWAKEGOLEM:
		return On_AWAKEGOLEM(pCmd, pnum);
	case CMD_MONSTDAMAGE:
		return On_MONSTDAMAGE(pCmd, pnum);
	case CMD_PLRDEAD:
		return On_PLRDEAD(pCmd, pnum);
	case CMD_PLRFRIENDY:
		return On_PLRFRIENDY(pCmd, pnum);
	case CMD_PLRDAMAGE:
		return On_PLRDAMAGE(pCmd, pnum);
	case CMD_OPERATEOBJ:
		return On_OPERATEOBJ(pCmd, pnum);
	case CMD_DOOROPEN:
		return On_DOOROPEN(pCmd, pnum);
	case CMD_DOORCLOSE:
		return On_DOORCLOSE(pCmd, pnum);
	case CMD_TRAPDISABLE:
		return On_TRAPDISABLE(pCmd, pnum);
	case CMD_TRAPOPEN:
		return On_TRAPOPEN(pCmd, pnum);
	case CMD_TRAPCLOSE:
		return On_TRAPCLOSE(pCmd, pnum);
	case CMD_CHESTCLOSE:
		return On_CHESTCLOSE(pCmd, pnum);
	case CMD_CHANGEPLRITEMS:
		return On_CHANGEPLRITEMS(pCmd, pnum);
	case CMD_DELPLRITEMS:
		return On_DELPLRITEMS(pCmd, pnum);
	case CMD_PLRLEVEL:
		return On_PLRLEVEL(pCmd, pnum);
	case CMD_PLRSKILLLVL:
		return On_PLRSKILLLVL(pCmd, pnum);
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
	case CMD_STRING:
		return On_STRING(pCmd, pnum);
	case CMD_SYNCQUEST:
		return On_SYNCQUEST(pCmd, pnum);
	case CMD_CHEAT_EXPERIENCE:
		return On_CHEAT_EXPERIENCE(pCmd, pnum);
	case CMD_CHEAT_SPELL_LEVEL:
		return On_CHEAT_SPELL_LEVEL(pCmd, pnum);
	case CMD_RESTOREHPVIT:
		return On_RESTOREHPVIT(pCmd, pnum);
#ifdef HELLFIRE
	case CMD_NAKRUL:
		return On_NAKRUL(pCmd, pnum);
	case CMD_OPENHIVE:
		return On_OPENHIVE(pCmd, pnum);
	case CMD_OPENCRYPT:
		return On_OPENCRYPT(pCmd, pnum);
#endif
	case CMD_SETSHIELD:
		return On_SETSHIELD(pCmd, pnum);
	case CMD_REMSHIELD:
		return On_REMSHIELD(pCmd, pnum);
	}

	if (sbLastCmd < CMD_DLEVEL_0 || sbLastCmd > CMD_DLEVEL_END) {
		SNetDropPlayer(pnum, LEAVE_DROP);
		return 0;
	}

	return On_DLEVEL(pnum, pCmd);
}

DEVILUTION_END_NAMESPACE
