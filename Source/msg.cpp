/**
 * @file msg.cpp
 *
 * Implementation of function for sending and reciving network messages.
 */
#include "all.h"
#include "diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

// fields to handle item records
static ItemGetRecordStruct itemrecord[MAXITEMS];
static int gnNumGetRecords;

// fields to handle delta-information
static DWORD sgdwOwnerWait;
static DWORD sgdwRecvOffset;
static int sgnCurrMegaPlayer;
static TMegaPkt *sgpCurrPkt;
static BYTE sgRecvBuf[sizeof(DLevel) + 3];
static TMegaPkt *sgpMegaPkt;
BOOL deltaload;
static DJunk sgJunk;
static DLevel sgLevels[NUMLEVELS + NUM_SETLVL];
static LocalLevel sgLocals[NUMLEVELS + NUM_SETLVL];
static bool _gbLevelDeltaChanged[NUMLEVELS + NUM_SETLVL];
static bool _gbJunkDeltaChanged;
static BYTE sgbDeltaChunks;
static BYTE _gbRecvCmd;
// the current messaging mode
_msg_mode geBufferMsgs;
// character message to send over to other players
char gbNetMsg[MAX_SEND_STR_LEN];

static void DeleteItemRecord(int i)
{
	gnNumGetRecords--;

	if (gnNumGetRecords == 0) {
		return;
	}

	itemrecord[i].nSeed = itemrecord[gnNumGetRecords].nSeed;
	itemrecord[i].nIndex = itemrecord[gnNumGetRecords].nIndex;
	itemrecord[i].wCI = itemrecord[gnNumGetRecords].wCI;
	itemrecord[i].dwTimestamp = itemrecord[gnNumGetRecords].dwTimestamp;
}

static bool ExistsItemRecord(const PkItemStruct *pki)
{
	int i;
	Uint32 dwTicks;

	dwTicks = SDL_GetTicks();

	for (i = 0; i < gnNumGetRecords; i++) {
		if (dwTicks - itemrecord[i].dwTimestamp > 6000) {
			DeleteItemRecord(i);
			i--;
		} else if (pki->dwSeed == itemrecord[i].nSeed && pki->wIndx == itemrecord[i].nIndex && pki->wCI == itemrecord[i].wCI) {
			return true;
		}
	}

	return false;
}

static void AddItemRecord(const PkItemStruct *pki)
{
	if (gnNumGetRecords == MAXITEMS) {
		return;
	}

	itemrecord[gnNumGetRecords].dwTimestamp = SDL_GetTicks();
	itemrecord[gnNumGetRecords].nSeed = pki->dwSeed;
	itemrecord[gnNumGetRecords].nIndex = pki->wIndx;
	itemrecord[gnNumGetRecords].wCI = pki->wCI;
	gnNumGetRecords++;
}

static void RemoveItemRecord(const PkItemStruct *pki)
{
	int i;
	Uint32 dwTicks;

	dwTicks = SDL_GetTicks();

	for (i = 0; i < gnNumGetRecords; i++) {
		if (dwTicks - itemrecord[i].dwTimestamp > 6000) {
			DeleteItemRecord(i);
			i--;
		} else if (pki->dwSeed == itemrecord[i].nSeed && pki->wIndx == itemrecord[i].nIndex && pki->wCI == itemrecord[i].wCI) {
			DeleteItemRecord(i);
			break;
		}
	}
}

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
				if (pktSize == 0)
					break;
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
	uint32_t turns;

	if (sgbDeltaChunks == 0) {
		nthread_send_and_recv_turn(0, 0);
		turns = SNetGetOwnerTurnsWaiting();
		//if (!SNetGetOwnerTurnsWaiting(&turns) && SErrGetLastError() == STORM_ERROR_NOT_IN_GAME)
		//	return 100;
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
	/*if (gbDeltaSender >= MAX_PLRS) {
		sgbDeltaChunks = 0;
		_gbRecvCmd = CMD_DLEVEL_END;
		gbDeltaSender = mypnum;
		nthread_set_turn_upper_bit();
	}*/
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
	_gbRecvCmd = CMD_DLEVEL_END;
	gbDeltaSender = MAX_PLRS;
	sgdwRecvOffset = 0;
	geBufferMsgs = MSG_DOWNLOAD_DELTA;
	sgdwOwnerWait = SDL_GetTicks();
	success = UiProgressDialog("Waiting for game data...", msg_wait_for_turns);
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
		mq->qlog = quests[i]._qlog;
		mq->qstate = quests[i]._qactive;
		mq->qvar1 = quests[i]._qvar1;
		copy_pod(*reinterpret_cast<MultiQuests *>(dst), *mq);
		dst += sizeof(*mq);
		mq++;
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
			SetPortalStats(i, false, 0, 0, 0);
		} else {
			copy_pod(*pD, *reinterpret_cast<DPortal *>(src));
			src += sizeof(*pD);
			SetPortalStats(i, true,	pD->x, pD->y, pD->level);
		}
	}

	mq = sgJunk.quests;
	for (i = 0; i < NUM_QUESTS; i++) {
		copy_pod(*mq, *reinterpret_cast<MultiQuests *>(src));
		src += sizeof(*mq);
		quests[i]._qlog = mq->qlog;
		quests[i]._qactive = mq->qstate;
		quests[i]._qvar1 = mq->qvar1;
		mq++;
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

	dst = NULL;

	for (i = 0; i < lengthof(sgLevels); i++)
		if (_gbLevelDeltaChanged[i])
			break;
	if (i != lengthof(sgLevels)) {
		dst = (BYTE *)DiabloAllocPtr(sizeof(sgRecvBuf));
		for (i = 0; i < lengthof(sgLevels); i++) {
			if (!_gbLevelDeltaChanged[i])
				continue;
			dstEnd = dst + 1;
			*dstEnd = i;
			dstEnd++;
			dstEnd = DeltaExportItem(dstEnd, sgLevels[i].item);
			dstEnd = DeltaExportObject(dstEnd, sgLevels[i].object);
			dstEnd = DeltaExportMonster(dstEnd, sgLevels[i].monster);
			size = msg_comp_level(dst, dstEnd);
			dthread_send_delta(pnum, CMD_DLEVEL_DATA, dst, size);
			src = 0;
			dthread_send_delta(pnum, CMD_DLEVEL_SEP, &src, 1);
		}
	}
	if (_gbJunkDeltaChanged) {
		if (dst == NULL)
			dst = (BYTE *)DiabloAllocPtr(sizeof(sgRecvBuf));
		dstEnd = dst + 1;
		dstEnd = DeltaExportJunk(dstEnd);
		size = msg_comp_level(dst, dstEnd);
		dthread_send_delta(pnum, CMD_DLEVEL_JUNK, dst, size);
	}
	mem_free_dbg(dst);

	src = 0;
	dthread_send_delta(pnum, CMD_DLEVEL_END, &src, 1);
}

static void DeltaImportData()
{
	BYTE i;
	BYTE *src;

	//if (sgRecvBuf[0] != 0)
	assert(sgRecvBuf[0] != 0);
		PkwareDecompress(&sgRecvBuf[1], sgdwRecvOffset, lengthof(sgRecvBuf) - 1);

	src = &sgRecvBuf[1];
	if (_gbRecvCmd == CMD_DLEVEL_DATA) {
		i = *src;
		src++;
		src = DeltaImportItem(src, sgLevels[i].item);
		src = DeltaImportObject(src, sgLevels[i].object);
		DeltaImportMonster(src, sgLevels[i].monster);
		_gbLevelDeltaChanged[i] = true;
	} else {
		assert(_gbRecvCmd == CMD_DLEVEL_JUNK);
		DeltaImportJunk(src);
		_gbJunkDeltaChanged = true;
	}

	sgbDeltaChunks++;
}

static unsigned On_DLEVEL(TCmd *pCmd, int pnum)
{
	TCmdPlrInfoHdr *cmd = (TCmdPlrInfoHdr *)pCmd;

	if (gbDeltaSender != pnum) {
		if (gbDeltaSender != MAX_PLRS) {
			// delta is already on its way from a different player -> drop the packet
			goto done;
		}
		if (sgdwRecvOffset != 0) {
			// the source of the delta is dropped -> drop the packages and quit
			gbGameDestroyed = true;
			goto done;
		}
		if (cmd->bCmd == CMD_DLEVEL_END) {
			// nothing received till now -> empty delta
			gbDeltaSender = pnum;
			// _gbRecvCmd = CMD_DLEVEL_END;
			sgbDeltaChunks = MAX_CHUNKS - 1;
			goto done;
		}
		if (cmd->wOffset != 0) {
			// invalid data starting offset -> drop the packet
			goto done;
		}
		if (cmd->bCmd != CMD_DLEVEL_DATA && cmd->bCmd != CMD_DLEVEL_JUNK) {
			// invalid data starting type -> drop the packet
			goto done;
		}
		if (pnum >= MAX_PLRS) {
			// message from an invalid player -> drop the packet
			goto done;
		}
		// start receiving
		gbDeltaSender = pnum;
		_gbRecvCmd = cmd->bCmd;
		// sgdwRecvOffset = 0;
	} else {
		// a packet from a previous sender
		if (_gbRecvCmd != cmd->bCmd) {
			// process previous package
			if (_gbRecvCmd != CMD_DLEVEL_SEP && _gbRecvCmd != CMD_DLEVEL_END)
				DeltaImportData();
			_gbRecvCmd = cmd->bCmd;
			if (_gbRecvCmd == CMD_DLEVEL_END) {
				// final package -> done
				sgbDeltaChunks = MAX_CHUNKS - 1;
				goto done;
			} else if (_gbRecvCmd == CMD_DLEVEL_SEP) {
				// separator package -> wait for next
				goto done;
			} else {
				// start receiving a new package
				assert(_gbRecvCmd == CMD_DLEVEL_DATA || _gbRecvCmd == CMD_DLEVEL_JUNK);
				sgdwRecvOffset = 0;
			}
		} else {
			// continue previous package
			if (_gbRecvCmd != CMD_DLEVEL_DATA && _gbRecvCmd != CMD_DLEVEL_JUNK) {
				// lost or duplicated package -> drop the connection and quit
				gbGameDestroyed = true;
				goto done;
			}
		}
	}

	if (cmd->wOffset != sgdwRecvOffset) {
		// lost or duplicated package -> drop the connection and quit
		gbGameDestroyed = true;
		goto done;
	}

	memcpy(&sgRecvBuf[cmd->wOffset], &cmd[1], cmd->wBytes);
	sgdwRecvOffset += cmd->wBytes;
done:
	return cmd->wBytes + sizeof(*cmd);
}

void delta_init()
{
	_gbJunkDeltaChanged = false;
	memset(_gbLevelDeltaChanged, 0, sizeof(_gbLevelDeltaChanged));
	memset(&sgJunk, 0xFF, sizeof(sgJunk));
	memset(sgLevels, 0xFF, sizeof(sgLevels));
	memset(sgLocals, 0, sizeof(sgLocals));
	deltaload = FALSE;
}

static void delta_kill_monster(const TCmdMonstKill* mon)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1)
		return;

	_gbLevelDeltaChanged[mon->mkLevel] = true;
	pD = &sgLevels[mon->mkLevel].monster[SwapLE16(mon->mkMnum)];
	pD->_mx = mon->mkX;
	pD->_my = mon->mkY;
	pD->_mdir = mon->mkDir;
	pD->_mhitpoints = 0;
}

static void delta_monster_hp(int mnum, int hp, BYTE bLevel)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1)
		return;

	_gbLevelDeltaChanged[bLevel] = true;
	pD = &sgLevels[bLevel].monster[mnum];
	// In vanilla code the value was discarded if hp was higher than the current one.
	// That disregards the healing monsters.
	// Now it is always updated except the monster is already dead.
	if (pD->_mhitpoints != 0)
		pD->_mhitpoints = SwapLE32(hp);
}

static void delta_sync_monster(const TSyncHeader *pHdr)
{
	DLevel* pDLvl;
	DMonsterStr *pD;
	WORD wLen;
	const TSyncMonster* pSync;
	const BYTE *pbBuf;

	if (gbMaxPlayers == 1)
		return;

	/// ASSERT: assert(pHdr->bLevel < NUMLEVELS + NUM_SETLVL);

	_gbLevelDeltaChanged[pHdr->bLevel] = true;
	pDLvl = &sgLevels[pHdr->bLevel];

	pbBuf = (const BYTE *)&pHdr[1];

	for (wLen = SwapLE16(pHdr->wLen); wLen >= sizeof(TSyncMonster); wLen -= sizeof(TSyncMonster)) {
		pSync = (TSyncMonster *)pbBuf;
		pD = &pDLvl->monster[pSync->_mndx];
		if (pD->_mhitpoints != 0) {
			pD->_mx = pSync->_mx;
			pD->_my = pSync->_my;
			pD->_mactive = SQUELCH_MAX;
			pD->_menemy = pSync->_menemy;
		}
		pbBuf += sizeof(TSyncMonster);
	}
	assert(wLen == 0);
}

static void delta_awake_golem(TCmdGolem *pG, int mnum)
{
	DMonsterStr *pD;

	if (gbMaxPlayers == 1)
		return;

	_gbLevelDeltaChanged[pG->_currlevel] = true;
	pD = &sgLevels[pG->_currlevel].monster[mnum];
	pD->_mx = pG->_mx;
	pD->_my = pG->_my;
	pD->_mactive = SQUELCH_MAX;
	pD->_menemy = pG->_menemy;
	pD->_mdir = pG->_mdir;
	pD->_mhitpoints = pG->_mhitpoints;
}

static void delta_leave_sync(BYTE bLevel)
{
	int i, mnum;
	DMonsterStr *pD;

	if (bLevel == DLV_TOWN) {
		glSeedTbl[DLV_TOWN] = GetRndSeed();
		return;
	}
	for (i = 0; i < nummonsters; i++) {
		mnum = monstactive[i];
		if (monster[mnum]._mhitpoints != 0
		 && (mnum >= MAX_MINIONS || !MINION_NR_INACTIVE(mnum))) {
			_gbLevelDeltaChanged[bLevel] = true;
			pD = &sgLevels[bLevel].monster[mnum];
			pD->_menemy = encode_enemy(mnum);
			pD->_mx = monster[mnum]._mx;
			pD->_my = monster[mnum]._my;
			pD->_mdir = monster[mnum]._mdir;
			pD->_mhitpoints = SwapLE32(monster[mnum]._mhitpoints);
			pD->_mactive = monster[mnum]._msquelch;
		}
	}
	memcpy(&sgLocals[bLevel].automapsv, automapview, sizeof(automapview));
}

static void delta_sync_object(int oi, BYTE bCmd, BYTE bLevel)
{
	if (gbMaxPlayers == 1)
		return;

	_gbLevelDeltaChanged[bLevel] = true;
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
		if (pD->bCmd == 0xFF || pD->item.dwSeed != pI->item.dwSeed || pD->item.wIndx != pI->item.wIndx || pD->item.wCI != pI->item.wCI)
			continue;

		switch (pD->bCmd) {
		case DCMD_TAKEN:
			return true;
		case DCMD_SPAWNED:
			_gbLevelDeltaChanged[bLevel] = true;
			pD->bCmd = DCMD_TAKEN;
			return true;
		case DCMD_DROPPED:
			_gbLevelDeltaChanged[bLevel] = true;
			pD->bCmd = 0xFF;
			return true;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}

	if ((pI->item.wCI & CF_PREGEN) == 0)
		return false;

	pD = sgLevels[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			_gbLevelDeltaChanged[bLevel] = true;
			pD->bCmd = DCMD_TAKEN;
			pD->x = pI->x;
			pD->y = pI->y;
			copy_pod(pD->item, pI->item);
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
		 && pD->item.dwSeed == pI->item.dwSeed
		 && pD->item.wIndx == pI->item.wIndx
		 && pD->item.wCI == pI->item.wCI) {
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
			_gbLevelDeltaChanged[bLevel] = true;
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

void PackPkItem(PkItemStruct *dest, const ItemStruct *src)
{
	if (src->_iIdx != IDI_EAR) {
		dest->dwSeed = SwapLE32(src->_iSeed);
		dest->wIndx = SwapLE16(src->_iIdx);
		dest->wCI = SwapLE16(src->_iCreateInfo);
		dest->bId = src->_iIdentified;
		dest->bDur = src->_iDurability;
		dest->bMDur = src->_iMaxDur;
		dest->bCh = src->_iCharges;
		dest->bMCh = src->_iMaxCharges;
		dest->wValue = SwapLE16(src->_ivalue);
	} else {
		dest->wIndx = SwapLE16(IDI_EAR);
		dest->wCI = SwapLE16(*(WORD*)&src->_iName[7]);
		dest->dwSeed = SwapLE32(*(DWORD*)&src->_iName[9]);
		dest->bId = src->_iName[13];
		dest->bDur = src->_iName[14];
		dest->bMDur = src->_iName[15];
		dest->bCh = src->_iName[16];
		dest->bMCh = src->_iName[17];
		dest->wValue = SwapLE16(src->_ivalue | (src->_iName[18] << 8) | ((src->_iCurs - ICURS_EAR_SORCERER) << 6));
		dest->dwBuff = SwapLE32(*(DWORD*)&src->_iName[19]);
	}
}

void DeltaAddItem(int ii)
{
	ItemStruct *is;
	int i;
	TCmdPItem *pD;

	if (gbMaxPlayers == 1)
		return;

	is = &items[ii];
	pD = sgLevels[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != 0xFF
		 && pD->item.dwSeed == SwapLE32(is->_iSeed)
		 && pD->item.wIndx == SwapLE16(is->_iIdx)
		 && pD->item.wCI == SwapLE16(is->_iCreateInfo)) {
			if (pD->bCmd == DCMD_TAKEN || pD->bCmd == DCMD_SPAWNED)
				return;
			if (pD->bCmd == DCMD_DROPPED)
				continue; // BUGFIX: should return instead? otherwise the item is duped...
		}
	}

	pD = sgLevels[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			_gbLevelDeltaChanged[currLvl._dLevelIdx] = true;
			pD->bCmd = DCMD_SPAWNED;
			pD->x = is->_ix;
			pD->y = is->_iy;
			PackPkItem(&pD->item, is);
			return;
		}
	}
}

void DeltaSaveLevel()
{
	int i;

	assert(gbMaxPlayers != 1);
	for (i = 0; i < MAX_PLRS; i++) {
		if (i != mypnum)
			players[i]._pGFXLoad = 0;
	}
	myplr._pLvlVisited[currLvl._dLevelIdx] = TRUE;
	delta_leave_sync(currLvl._dLevelIdx);
}

static void UnPackEar(const PkItemStruct *src)
{
	*(WORD*)&tempstr[0] = SwapLE16(src->wCI);
	*(DWORD*)&tempstr[2] = SwapLE32(src->dwSeed);
	tempstr[6] = src->bId;
	tempstr[7] = src->bDur;
	tempstr[8] = src->bMDur;
	tempstr[9] = src->bCh;
	tempstr[10] = src->bMCh;
	tempstr[11] = SwapLE16(src->wValue) >> 8;
	*(DWORD*)&tempstr[12] = SwapLE32(src->dwBuff);
	tempstr[16] = '\0';
	items[MAXITEMS]._iCurs = ((SwapLE16(src->wValue) >> 6) & 3) + ICURS_EAR_SORCERER;
	items[MAXITEMS]._ivalue = SwapLE16(src->wValue) & 0x3F;

	snprintf(items[MAXITEMS]._iName, sizeof(items[MAXITEMS]._iName), "Ear of %s", tempstr);
	items[MAXITEMS]._iCreateInfo = SwapLE16(*(WORD *)&items[MAXITEMS]._iName[7]);
	items[MAXITEMS]._iSeed = SwapLE32(*(DWORD *)&items[MAXITEMS]._iName[9]);
}

void UnPackPkItem(const PkItemStruct *src)
{
	uint16_t idx = SwapLE16(src->wIndx);

	if (idx != IDI_EAR) {
		RecreateItem(
			SwapLE32(src->dwSeed),
			SwapLE16(src->wIndx),
			SwapLE16(src->wCI),
			SwapLE16(src->wValue));
		items[MAXITEMS]._iIdentified = src->bId;
		items[MAXITEMS]._iDurability = src->bDur;
		items[MAXITEMS]._iMaxDur = src->bMDur;
		items[MAXITEMS]._iCharges = src->bCh;
		items[MAXITEMS]._iMaxCharges = src->bMCh;
	} else {
		SetItemData(MAXITEMS, IDI_EAR);
		UnPackEar(src);
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
	if (currLvl._dLevelIdx != DLV_TOWN) {
		mstr = sgLevels[currLvl._dLevelIdx].monster;
		for (i = 0; i < nummonsters; i++, mstr++) {
			if (mstr->_mx != 0xFF) {
				if (i >= MAX_MINIONS)
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
				// check if only the position of the monster was modified
				if (mstr->_mhitpoints != -1)
					mon->_mhitpoints = SwapLE32(mstr->_mhitpoints);
				if (mstr->_mhitpoints == 0) {
					// SetDead: inline for better performance
					if (mon->mlid != 0)
						ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
					AddDead(i);
				} else {
					decode_enemy(i, mstr->_menemy);
					dMonster[mon->_mx][mon->_my] = i + 1;
					MonStartStand(i, mon->_mdir);
					mon->_msquelch = mstr->_mactive;
				}
			}
		}
		// SetDead();

		memcpy(automapview, sgLocals[currLvl._dLevelIdx].automapsv, sizeof(automapview));

		dstr = sgLevels[currLvl._dLevelIdx].object;
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
	itm = sgLevels[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, itm++) {
		if (itm->bCmd == DCMD_TAKEN) {
			ii = FindGetItem(SwapLE32(itm->item.dwSeed), SwapLE16(itm->item.wIndx), SwapLE16(itm->item.wCI));
			if (ii != -1) {
				if (dItem[items[ii]._ix][items[ii]._iy] == ii + 1)
					dItem[items[ii]._ix][items[ii]._iy] = 0;
				DeleteItem(ii, i);
			}
		} else if (itm->bCmd == DCMD_DROPPED) {
			UnPackPkItem(&itm->item);
			x = itm->x;
			y = itm->y;
			if (!CanPut(x, y))
				FindItemLocation(x, y, &x, &y, DSIZEX / 2);

			ii = itemavail[0];
			itemavail[0] = itemavail[MAXITEMS - numitems - 1];
			itemactive[numitems] = ii;
			copy_pod(items[ii], items[MAXITEMS]);
			items[ii]._ix = x;
			items[ii]._iy = y;
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

void NetSendCmdMonstKill(int mnum, int pnum)
{
	TCmdMonstKill cmd;

	cmd.bCmd = CMD_MONSTDEATH;
	cmd.mkMnum = SwapLE16(mnum);
	cmd.mkPnum = pnum;
	cmd.mkX = monster[mnum]._mx;
	cmd.mkY = monster[mnum]._my;
	cmd.mkDir = monster[mnum]._mdir;
	cmd.mkLevel = players[pnum].plrlevel;
	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdGolem(BYTE mx, BYTE my, BYTE dir, BYTE menemy, int hp, BYTE cl)
{
	TCmdGolem cmd;

	cmd.bCmd = CMD_AWAKEGOLEM;
	cmd._mx = mx;
	cmd._my = my;
	cmd._mdir = dir;
	cmd._menemy = menemy;
	cmd._mhitpoints = SwapLE32(hp);
	cmd._currlevel = cl;
	NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLoc(bool bHiPri, BYTE bCmd, BYTE x, BYTE y)
{
	TCmdLoc cmd;

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
	cmd.wParam1 = SwapLE16(wParam1);
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdParam1(bool bHiPri, BYTE bCmd, WORD wParam1)
{
	TCmdParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = SwapLE16(wParam1);
	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdParam2(bool bHiPri, BYTE bCmd, WORD wParam1, WORD wParam2)
{
	TCmdParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = SwapLE16(wParam1);
	cmd.wParam2 = SwapLE16(wParam2);
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

void NetSendCmdQuest(BYTE q, bool extOnly)
{
	TCmdQuest cmd;

	cmd.bCmd = extOnly ? CMD_SYNCQUESTEXT : CMD_SYNCQUEST;
	cmd.q = q;
	cmd.qstate = quests[q]._qactive;
	cmd.qlog = quests[q]._qlog;
	cmd.qvar1 = quests[q]._qvar1;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdGItem(BYTE bCmd, BYTE ii)
{
	ItemStruct* is;
	TCmdGItem cmd;

	cmd.bCmd = bCmd;
	cmd.bPnum = mypnum;
	cmd.bMaster = mypnum;
	cmd.bLevel = currLvl._dLevelIdx;
	cmd.bCursitem = ii;
	cmd.dwTime = 0;
	is = &items[ii];
	cmd.x = is->_ix;
	cmd.y = is->_iy;

	PackPkItem(&cmd.item, is);

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

static void NetReSendCmdGItem(BYTE bCmd, BYTE mast, TCmdGItem *p)
{
	TCmdGItem cmd;

	copy_pod(cmd, *p);
	cmd.bCmd = bCmd;
	cmd.bMaster = mast;
	cmd.dwTime = 0;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

static bool NetSendCmdTMsg(BYTE bCmd, BYTE mast, TCmdGItem *p)
{
	int ticks;
	TCmdGItem cmd;

	copy_pod(cmd, *p);
	cmd.bCmd = bCmd;
	cmd.bMaster = mast;

	ticks = SDL_GetTicks();
	if (cmd.dwTime == 0)
		cmd.dwTime = ticks;
	else if (ticks - cmd.dwTime > 5000)
		return false;

	tmsg_add(&cmd);
	return true;
}

void NetSendCmdPItem(bool bHiPri, BYTE bCmd, ItemStruct *is, BYTE x, BYTE y)
{
	TCmdPItem cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;

	PackPkItem(&cmd.item, is);

	if (bHiPri)
		NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	else
		NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdChItem(ItemStruct *is, BYTE bLoc)
{
	TCmdChItem cmd;

	cmd.bCmd = CMD_CHANGEPLRITEM;
	cmd.bLoc = bLoc;
	cmd.dwSeed = SwapLE32(is->_iSeed);
	cmd.wIndx = SwapLE16(is->_iIdx);
	cmd.wCI = SwapLE16(is->_iCreateInfo);
	cmd.bCh = is->_iCharges;
	cmd.bMCh = is->_iMaxCharges;
	cmd.bId = is->_iIdentified;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdDelItem(BYTE bLoc)
{
	TCmdBParam1 cmd;

	cmd.bCmd = CMD_DELPLRITEM;
	cmd.bParam1 = bLoc;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdDItem(int ii)
{
	ItemStruct *is;
	TCmdPItem cmd;

	is = &items[ii];
	cmd.bCmd = CMD_DROPITEM;
	cmd.x = is->_ix;
	cmd.y = is->_iy;

	PackPkItem(&cmd.item, is);

	NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

static bool i_own_level(int nReqLevel)
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (players[i].plractive
		 && !players[i]._pLvlChanging
		 && players[i].plrlevel == nReqLevel
		 && (i != mypnum || geBufferMsgs == MSG_NORMAL))
			break;
	}
	return i == mypnum;
}

void NetSendCmdLocAttack(BYTE x, BYTE y, int skill, int lvl)
{
	TCmdLocAttack cmd;

	cmd.bCmd = (myplr._pSkillFlags & SFLAG_MELEE) ? CMD_SATTACKXY : CMD_RATTACKXY;
	cmd.x = x;
	cmd.y = y;
	cmd.laSkill = skill;
	cmd.laLevel = lvl;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdLocSkill(BYTE x, BYTE y, int skill, int from, int lvl)
{
	TCmdLocSkill cmd;

	cmd.bCmd = CMD_SPELLXY;
	cmd.x = x;
	cmd.y = y;
	cmd.lsSkill = skill;
	cmd.lsFrom = from;
	cmd.lsLevel = lvl;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdPlrAttack(int pnum, int skill, int level)
{
	TCmdPlrAttack cmd;

	cmd.bCmd = (myplr._pSkillFlags & SFLAG_MELEE) ? CMD_ATTACKPID : CMD_RATTACKPID;
	cmd.paPnum = pnum;
	cmd.paSkill = skill;
	cmd.paLevel = level;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdPlrSkill(int pnum, int skill, int from, int level)
{
	TCmdPlrSkill cmd;

	cmd.bCmd = CMD_SPELLPID;
	cmd.psPnum = pnum;
	cmd.psSkill = skill;
	cmd.psFrom = from;
	cmd.psLevel = level;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdPlrDamage(int pnum, unsigned damage)
{
	TCmdPlrDamage cmd;

	cmd.bCmd = CMD_PLRDAMAGE;
	cmd.pdPnum = pnum;
	cmd.pdDamage = SwapLE32(damage);

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdMonstAttack(BYTE bCmd, int mnum, int skill, int lvl)
{
	TCmdMonstAttack cmd;

	cmd.bCmd = bCmd;
	cmd.maMnum = SwapLE16(mnum);
	cmd.maSkill = skill;
	cmd.maLevel = lvl;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdMonstSkill(int mnum, int skill, int from, int level)
{
	TCmdMonstSkill cmd;

	cmd.bCmd = CMD_SPELLID;
	cmd.msMnum = SwapLE16(mnum);
	cmd.msSkill = skill;
	cmd.msFrom = from;
	cmd.msLevel = level;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdMonstDamage(int mnum, int hitpoints, unsigned damage)
{
	TCmdMonstDamage cmd;

	cmd.bCmd = CMD_MONSTDAMAGE;
	cmd.mdMnum = SwapLE16(mnum);
	cmd.mdHitpoints = SwapLE32(hitpoints);
	cmd.mdDamage = SwapLE32(damage);

	NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdString(unsigned int pmask)
{
	int dwStrLen;
	TCmdString cmd;

	dwStrLen = strlen(gbNetMsg);
	cmd.bCmd = CMD_STRING;
	memcpy(cmd.str, gbNetMsg, dwStrLen + 1);
	multi_send_msg_packet(pmask, (BYTE *)&cmd, dwStrLen + 2);
}

static void delta_open_portal(int pnum, BYTE x, BYTE y, BYTE bLevel)
{
	_gbJunkDeltaChanged = true;
	sgJunk.portal[pnum].x = x;
	sgJunk.portal[pnum].y = y;
	sgJunk.portal[pnum].level = bLevel;
}

void delta_close_portal(int pnum)
{
	memset(&sgJunk.portal[pnum], 0xFF, sizeof(sgJunk.portal[pnum]));
	// assert(_gbJunkDeltaChanged == true);
}

static void check_update_plr(int pnum)
{
	if (gbMaxPlayers != 1 && pnum == mypnum)
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
	TSyncHeader *pHdr = (TSyncHeader *)pCmd;

	/// ASSERT: assert(geBufferMsgs != MSG_RUN_DELTA);
	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && pnum != mypnum) {
		if (currLvl._dLevelIdx == pHdr->bLevel)
			sync_update(pnum, pHdr);
		delta_sync_monster(pHdr);
	}
	return SwapLE16(pHdr->wLen) + sizeof(*pHdr);
}

static unsigned On_WALKXY(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		MakePlrPath(pnum, cmd->x, cmd->y, true);
		players[pnum].destAction = ACTION_NONE;
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
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		players[pnum].destAction = ACTION_BLOCK;
		players[pnum].destParam1 = cmd->bParam1; // direction
	}
	return sizeof(*cmd);
}

static unsigned On_GOTOGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		players[pnum].destAction = ACTION_PICKUPITEM;
		players[pnum].destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_REQUESTGITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && i_own_level(players[pnum].plrlevel)) {
		if (!ExistsItemRecord(&cmd->item)) {
			int ii = FindGetItem(SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
			if (ii != -1) {
				NetReSendCmdGItem(CMD_GETITEM, mypnum, cmd);
				if (cmd->bPnum != mypnum)
					SyncGetItemIdx(ii);
				else
					InvGetItem(mypnum, ii);
				AddItemRecord(&cmd->item);
			} else if (!NetSendCmdTMsg(CMD_REQUESTGITEM, mypnum, cmd))
				NetReSendCmdGItem(CMD_ITEMEXTRA, cmd->bMaster, cmd);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_GETITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;
	int ii;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (delta_get_item(cmd, cmd->bLevel)) {
			if ((currLvl._dLevelIdx == cmd->bLevel || cmd->bPnum == mypnum) && cmd->bMaster != mypnum) {
				if (cmd->bPnum == mypnum) {
					if (currLvl._dLevelIdx != cmd->bLevel) {
						UnPackPkItem(&cmd->item);
						ii = SyncPutItem(mypnum, myplr._px, myplr._py, MAXITEMS, false);
						if (ii != -1)
							InvGetItem(mypnum, ii);
					} else {
						ii = FindGetItem(SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
						InvGetItem(mypnum, ii);
					}
				} else
					SyncGetItemAt(cmd->x, cmd->y, SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
			}
		} else
			NetSendCmdTMsg(CMD_GETITEM, cmd->bMaster, cmd);
	}

	return sizeof(*cmd);
}

static unsigned On_GOTOAGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		players[pnum].destAction = ACTION_PICKUPAITEM;
		players[pnum].destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_REQUESTAGITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && i_own_level(players[pnum].plrlevel)) {
		if (!ExistsItemRecord(&cmd->item)) {
			int ii = FindGetItem(SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
			if (ii != -1) {
				NetReSendCmdGItem(CMD_AGETITEM, mypnum, cmd);
				if (cmd->bPnum != mypnum)
					SyncGetItemIdx(ii);
				else
					AutoGetItem(mypnum, cmd->bCursitem);
				AddItemRecord(&cmd->item);
			} else if (!NetSendCmdTMsg(CMD_REQUESTAGITEM, mypnum, cmd))
				NetReSendCmdGItem(CMD_ITEMEXTRA, cmd->bMaster, cmd);
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
		if (delta_get_item(cmd, cmd->bLevel)) {
			if ((currLvl._dLevelIdx == cmd->bLevel || cmd->bPnum == mypnum) && cmd->bMaster != mypnum) {
				if (cmd->bPnum == mypnum) {
					if (currLvl._dLevelIdx != cmd->bLevel) {
						UnPackPkItem(&cmd->item);
						int ii = SyncPutItem(mypnum, myplr._px, myplr._py, MAXITEMS, false);
						if (ii != -1)
							AutoGetItem(mypnum, ii);
					} else
						AutoGetItem(mypnum, cmd->bCursitem);
				} else
					SyncGetItemAt(cmd->x, cmd->y, SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
			}
		} else
			NetSendCmdTMsg(CMD_AGETITEM, cmd->bMaster, cmd);
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
		if (currLvl._dLevelIdx == players[pnum].plrlevel)
			SyncGetItemAt(cmd->x, cmd->y, SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
	}

	return sizeof(*cmd);
}

#ifdef HELLFIRE
static bool CheckTownTrigs(int pnum, int x, int y, int iidx)
{
	if (iidx == IDI_RUNEBOMB
	 && x >= DBORDERX + 69 && x <= DBORDERX + 72 && y >= DBORDERY + 51 && y <= DBORDERY + 54
	 && quests[Q_FARMER]._qactive != QUEST_DONE) {
		quests[Q_FARMER]._qactive = QUEST_DONE;
		quests[Q_FARMER]._qvar1 = 2 + pnum;
		quests[Q_FARMER]._qlog = TRUE;
		if (pnum == mypnum) {
			// NetSendCmdQuest(Q_FARMER, true);
			NetSendCmd(false, CMD_OPENHIVE);
		}
		return true;
	}
	if (iidx == IDI_MAPOFDOOM
	 && x >= DBORDERX + 25  && x <= DBORDERX + 28 && y >= DBORDERY + 10 && y <= DBORDERY + 14
	 && quests[Q_GRAVE]._qactive != QUEST_DONE) {
		quests[Q_GRAVE]._qactive = QUEST_DONE;
		if (pnum == mypnum) {
			// NetSendCmdQuest(Q_GRAVE, true);
			NetSendCmd(false, CMD_OPENCRYPT);
		}
		return true;
	}
	return false;
}
#endif

static unsigned On_PUTITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;
	int x, y;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		check_update_plr(pnum);
		x = cmd->x;
		y = cmd->y;
#ifdef HELLFIRE
		if (players[pnum].plrlevel == DLV_TOWN && CheckTownTrigs(pnum, x, y, SwapLE16(cmd->item.wIndx))) {
			return sizeof(*cmd);
		}
#endif
		if (currLvl._dLevelIdx == players[pnum].plrlevel) {
			UnPackPkItem(&cmd->item);
			int ii = InvPutItem(pnum, x, y, MAXITEMS);
			if (ii == -1)
				return sizeof(*cmd);
			x = items[ii]._ix;
			y = items[ii]._iy;
		}
		delta_put_item(cmd, x, y, players[pnum].plrlevel);
		RemoveItemRecord(&cmd->item);
	}

	return sizeof(*cmd);
}

static unsigned On_SYNCPUTITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (currLvl._dLevelIdx == players[pnum].plrlevel) {
		UnPackPkItem(&cmd->item);
		int ii = SyncPutItem(pnum, cmd->x, cmd->y, MAXITEMS, true);
		if (ii != -1) {
			RemoveItemRecord(&cmd->item);
			delta_put_item(cmd, items[ii]._ix, items[ii]._iy, players[pnum].plrlevel);
			check_update_plr(pnum);
		}
	} else {
		RemoveItemRecord(&cmd->item);
		delta_put_item(cmd, cmd->x, cmd->y, players[pnum].plrlevel);
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
		if (currLvl._dLevelIdx == players[pnum].plrlevel && pnum != mypnum) {
			UnPackPkItem(&cmd->item);
			SyncPutItem(pnum, cmd->x, cmd->y, MAXITEMS, false);
		}
		RemoveItemRecord(&cmd->item);
		delta_put_item(cmd, cmd->x, cmd->y, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_DROPITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem *cmd = (TCmdPItem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		delta_put_item(cmd, cmd->x, cmd->y, players[pnum].plrlevel);

	return sizeof(*cmd);
}

static unsigned On_SATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLocAttack *cmd = (TCmdLocAttack *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->laSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_ATTACK;
			players[pnum].destParam1 = cmd->x;
			players[pnum].destParam2 = cmd->y;
			players[pnum].destParam3 = sn;           // attack skill
			players[pnum].destParam4 = cmd->laLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLocAttack *cmd = (TCmdLocAttack *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->laSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_RATTACK;
			players[pnum].destParam1 = cmd->x;
			players[pnum].destParam2 = cmd->y;
			players[pnum].destParam3 = sn;           // attack skill
			players[pnum].destParam4 = cmd->laLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLXY(TCmd *pCmd, int pnum)
{
	TCmdLocSkill *cmd = (TCmdLocSkill *)pCmd;
	BYTE sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->lsSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_SPELL;
			players[pnum].destParam2 = cmd->x;
			players[pnum].destParam3 = cmd->y;
			players[pnum].destParam1a = sn;           // spell
			players[pnum].destParam1b = cmd->lsFrom;  // invloc
			players[pnum].destParam1c = cmd->lsLevel; // spllvl
		} else
			msg_errorf("%s has cast an illegal spell.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_DOABILITY(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA) 
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		DoAbility(pnum, cmd->bParam1, cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_DOOIL(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA) 
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		DoOil(pnum, cmd->bParam1, cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_OPOBJXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		int oi = SwapLE16(cmd->wParam1);
		players[pnum].destAction = ACTION_OPERATE;
		players[pnum].destParam1 = oi;
		players[pnum].destParam2 = cmd->x;
		players[pnum].destParam3 = cmd->y;
		MakePlrPath(pnum, cmd->x, cmd->y, !(object[oi]._oSolidFlag | object[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_DISARMXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		int oi = SwapLE16(cmd->wParam1);
		players[pnum].destAction = ACTION_DISARM;
		players[pnum].destParam1 = oi;
		players[pnum].destParam2 = cmd->x;
		players[pnum].destParam3 = cmd->y;
		MakePlrPath(pnum, cmd->x, cmd->y, !(object[oi]._oSolidFlag | object[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_OPOBJT(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		players[pnum].destAction = ACTION_OPERATETK;
		players[pnum].destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKID(TCmd *pCmd, int pnum)
{
	TCmdMonstAttack *cmd = (TCmdMonstAttack *)pCmd;
	int mnum, x, y, sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		mnum = SwapLE16(cmd->maMnum);
		x = monster[mnum]._mfutx;
		y = monster[mnum]._mfuty;
		if (abs(players[pnum]._px - x) > 1 || abs(players[pnum]._py - y) > 1)
			MakePlrPath(pnum, x, y, false);
		sn = cmd->maSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_ATTACKMON;
			players[pnum].destParam1 = mnum;
			players[pnum].destParam2 = sn;           // attack skill
			players[pnum].destParam3 = cmd->maLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdPlrAttack *cmd = (TCmdPlrAttack *)pCmd;
	int tnum, sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		tnum = cmd->paPnum;
		MakePlrPath(pnum, players[tnum]._pfutx, players[tnum]._pfuty, false);
		sn = cmd->paSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_ATTACKPLR;
			players[pnum].destParam1 = tnum;
			players[pnum].destParam2 = sn;           // attack skill
			players[pnum].destParam3 = cmd->paLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKID(TCmd *pCmd, int pnum)
{
	TCmdMonstAttack *cmd = (TCmdMonstAttack *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->maSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_RATTACKMON;
			players[pnum].destParam1 = SwapLE16(cmd->maMnum);  // target id
			players[pnum].destParam2 = sn;                     // attack skill
			players[pnum].destParam3 = cmd->maLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdPlrAttack *cmd = (TCmdPlrAttack *)pCmd;
	int sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->paSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_RATTACKPLR;
			players[pnum].destParam1 = cmd->paPnum;  // target id
			players[pnum].destParam2 = sn;           // attack skill
			players[pnum].destParam3 = cmd->paLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLID(TCmd *pCmd, int pnum)
{
	TCmdMonstSkill *cmd = (TCmdMonstSkill *)pCmd;
	BYTE sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->msSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_SPELLMON;
			players[pnum].destParam1a = sn;                    // spell
			players[pnum].destParam1b = cmd->msFrom;           // invloc
			players[pnum].destParam1c = cmd->msLevel;          // spllvl
			players[pnum].destParam2 = SwapLE16(cmd->msMnum);  // mnum
		} else
			msg_errorf("%s has cast an illegal spell.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLPID(TCmd *pCmd, int pnum)
{
	TCmdPlrSkill *cmd = (TCmdPlrSkill *)pCmd;
	BYTE sn;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		ClrPlrPath(pnum);
		sn = cmd->psSkill;
		if ((spelldata[sn].sFlags & players[pnum]._pSkillFlags) == spelldata[sn].sFlags) {
			players[pnum].destAction = ACTION_SPELLPLR;
			players[pnum].destParam1a = sn;           // spell
			players[pnum].destParam1b = cmd->psFrom;  // invloc
			players[pnum].destParam1c = cmd->psLevel; // spllvl
			players[pnum].destParam2 = cmd->psPnum;   // pnum
		} else
			msg_errorf("%s has cast an illegal spell.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_KNOCKBACK(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		// assert(cmd->wParam1 >= MAX_MINIONS); TODO: validate data from internet
		MonGetKnockback(SwapLE16(cmd->wParam1));
		MonStartHit(SwapLE16(cmd->wParam1), pnum, 0);
	}

	return sizeof(*cmd);
}

static unsigned On_TALKXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (geBufferMsgs != MSG_DOWNLOAD_DELTA && currLvl._dLevelIdx == players[pnum].plrlevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		players[pnum].destAction = ACTION_TALK;
		players[pnum].destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_NEWLVL(TCmd *pCmd, int pnum)
{
	TCmdParam2 *cmd = (TCmdParam2 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != mypnum)
		StartNewLvl(pnum, SwapLE16(cmd->wParam1), SwapLE16(cmd->wParam2));

	return sizeof(*cmd);
}

static unsigned On_WARP(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		StartWarpLvl(pnum, SwapLE16(cmd->wParam1));
	}

	return sizeof(*cmd);
}

static unsigned On_MONSTDEATH(TCmd *pCmd, int pnum)
{
	TCmdMonstKill *cmd = (TCmdMonstKill *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (pnum != mypnum && currLvl._dLevelIdx == cmd->mkLevel)
			MonSyncStartKill(SwapLE16(cmd->mkMnum), cmd->mkX, cmd->mkY, cmd->mkPnum);
		delta_kill_monster(cmd);
	}

	return sizeof(*cmd);
}

static unsigned On_AWAKEGOLEM(TCmd *pCmd, int pnum)
{
	TCmdGolem *cmd = (TCmdGolem *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		delta_awake_golem(cmd, pnum);

	return sizeof(*cmd);
}

static unsigned On_MONSTDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdMonstDamage *cmd = (TCmdMonstDamage *)pCmd;
	int mnum, hp, nhp;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		mnum = SwapLE16(cmd->mdMnum);
		hp = SwapLE32(cmd->mdHitpoints);
		if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel) {
			nhp = monster[mnum]._mhitpoints - SwapLE32(cmd->mdDamage);
			if (nhp < hp)
				hp = nhp;
			if (hp < (1 << 6))
				hp = 1 << 6;
			monster[mnum]._mhitpoints = hp;
		}
		delta_monster_hp(mnum, hp, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_PLRDEAD(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != mypnum)
		StartPlrKill(pnum, cmd->bParam1);
	else
		check_update_plr(pnum);

	return sizeof(*cmd);
}

static unsigned On_PLRRESURRECT(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else {
		SyncPlrResurrect(pnum);
		if (pnum == mypnum)
			check_update_plr(pnum);
	}		

	return sizeof(*pCmd);
}

static unsigned On_PLRDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdPlrDamage *cmd = (TCmdPlrDamage *)pCmd;

	if (cmd->pdPnum == mypnum && geBufferMsgs != MSG_DOWNLOAD_DELTA) {
		if (currLvl._dType != DTYPE_TOWN && currLvl._dLevelIdx == players[pnum].plrlevel) {
			if (!myplr._pInvincible && SwapLE32(cmd->pdDamage) <= 192000) {
				PlrDecHp(mypnum, SwapLE32(cmd->pdDamage), DMGTYPE_PLAYER);
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
		//if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel)
		//	SyncDoorOpen(SwapLE16(cmd->wParam1));
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_DOOROPEN, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_DOORCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel)
		//	SyncDoorClose(SwapLE16(cmd->wParam1));
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_DOORCLOSE, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_TRAPDISABLE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel)
		//	SyncTrapDisable(SwapLE16(cmd->wParam1));
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPDISABLE, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_TRAPOPEN(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel)
		//	SyncTrapOpen(SwapLE16(cmd->wParam1));
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPOPEN, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_TRAPCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel)
		//	SyncTrapClose(SwapLE16(cmd->wParam1));
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPCLOSE, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_OPERATEOBJ(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel)
			SyncOpObject(pnum, SwapLE16(cmd->wParam1));
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_OPERATEOBJ, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_CHESTCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		//if (pnum != mypnum && currLvl._dLevelIdx == players[pnum].plrlevel)
		//	SyncChestClose(SwapLE16(cmd->wParam1));
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_CHESTCLOSE, players[pnum].plrlevel);
	}

	return sizeof(*cmd);
}

static unsigned On_CHANGEPLRITEM(TCmd *pCmd, int pnum)
{
	TCmdChItem *cmd = (TCmdChItem *)pCmd;
	ItemStruct *is;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != mypnum) {
		RecreateItem(SwapLE32(cmd->dwSeed), SwapLE16(cmd->wIndx), SwapLE16(cmd->wCI), 0);
		is = &items[MAXITEMS];
		is->_iCharges = cmd->bCh;
		is->_iMaxCharges = cmd->bMCh;
		is->_iIdentified = cmd->bId;
		SyncPlrItemChange(pnum, cmd->bLoc, MAXITEMS);
	}

	return sizeof(*cmd);
}

static unsigned On_DELPLRITEM(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != mypnum)
		SyncPlrItemRemove(pnum, cmd->bParam1);

	return sizeof(*cmd);
}

static unsigned On_USEPLRITEM(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else // if (pnum != mypnum)
		SyncUseItem(pnum, cmd->bParam1);

	return sizeof(*cmd);
}

static unsigned On_PLRLEVEL(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != mypnum && cmd->bParam1 <= MAXCHARLEVEL)
		players[pnum]._pLevel = cmd->bParam1;

	return sizeof(*cmd);
}

static unsigned On_PLRSKILLLVL(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else if (pnum != mypnum && cmd->bParam2 <= MAXSPLLEVEL)
		players[pnum]._pSkillLvl[cmd->bParam1] = cmd->bParam2;

	return sizeof(*cmd);
}

static unsigned On_SEND_PLRINFO(TCmd *pCmd, int pnum)
{
	TCmdPlrInfoHdr *cmd = (TCmdPlrInfoHdr *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, cmd->wBytes + sizeof(*cmd));
	else if (pnum != mypnum)
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
		p = &players[pnum];
		p->_pLvlChanging = FALSE;
		if (pnum != mypnum) {
			if (!p->plractive) {
				p->plractive = TRUE;
				assert(p->_pTeam == pnum);
				gbActivePlayers++;
				EventPlrMsg("Player '%s' (level %d) just joined the game", p->_pName, p->_pLevel);
			}
			p->_px = cmd->x;
			p->_py = cmd->y;
			p->plrlevel = cmd->bParam1;
			p->_pGFXLoad = 0;
			if (currLvl._dLevelIdx == p->plrlevel) {
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
					NewPlrAnim(pnum, p->_pDAnim, DIR_S, p->_pDFrames, PlrAnimFrameLens[PA_DEATH], p->_pDWidth);
					p->_pAnimFrame = p->_pAnimLen - 1;
					p->_pVar8 = 2 * p->_pAnimLen; // DEATH_TICK
					p->_pVar7 = 0; // DEATH_DELAY
					dFlags[p->_px][p->_py] |= BFLAG_DEAD_PLAYER;
				}*/

				//p->_pvid = AddVision(p->_px, p->_py, p->_pLightRad, pnum == mypnum);
				//p->_plid = -1;
			}
		}
	}

	return sizeof(*cmd);
}

static unsigned On_ACTIVATEPORTAL(TCmd *pCmd, int pnum)
{
	TCmdLocBParam1 *cmd = (TCmdLocBParam1 *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		ActivatePortal(pnum, cmd->x, cmd->y, cmd->bParam1);
		if (pnum != mypnum) {
			if (currLvl._dLevelIdx == 0)
				AddInTownPortal(pnum);
			else if (currLvl._dLevelIdx == players[pnum].plrlevel) {
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
		delta_open_portal(pnum, cmd->x, cmd->y, cmd->bParam1);
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
	TCmdString *cmd = (TCmdString *)pCmd;

	if (geBufferMsgs == MSG_NORMAL && !(guTeamMute & (1 << pnum))) {
		SendPlrMsg(pnum, cmd->str);
	}

	return strlen(cmd->str) + 2; // length of string + nul terminator + sizeof(cmd->bCmd)
}

static unsigned On_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs != MSG_NORMAL)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	 // TODO: check (cmd->bParam1 == mypnum) should not be necessary in a server/client solution
	else if (cmd->bParam1 == mypnum && players[pnum]._pTeam == pnum) {
		guTeamInviteRec |= (1 << pnum);
		EventPlrMsg("%s invited to their team.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_ACK_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs != MSG_NORMAL) {
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	} else {
		guTeamInviteRec &= ~(1 << pnum);
		guTeamInviteSent &= ~(1 << pnum);

		players[pnum]._pTeam = cmd->bParam1;
		if (cmd->bParam1 == myplr._pTeam) {
			if (pnum == mypnum)
				EventPlrMsg("You joined team %c.", 'a' + players[pnum]._pTeam);
			else
				EventPlrMsg("%s joined your team.", players[pnum]._pName);
		} else {
			EventPlrMsg("%s joined team %c.", players[pnum]._pName, 'a' + players[pnum]._pTeam);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_DEC_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteSent &= ~(1 << pnum);

		EventPlrMsg("%s rejected your invitation.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_REV_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (geBufferMsgs != MSG_NORMAL) {
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	} else if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteRec &= ~(1 << pnum);

		EventPlrMsg("%s revoked the invitation.", players[pnum]._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_KICK_PLR(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;
	int teamplr, team;

	if (geBufferMsgs != MSG_NORMAL) {
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	} else {
		teamplr = cmd->bParam1;
		team = players[teamplr]._pTeam;
		if (pnum != teamplr) {
			// drop
			if (team == pnum) {
				players[teamplr]._pTeam = teamplr;

				if (teamplr == mypnum) {
					EventPlrMsg("You were kicked from your team.");
				} else {
					EventPlrMsg("%s was kicked from %s team.", players[teamplr]._pName, team == myplr._pTeam ? "your" : "their");
				}
			}
		} else {
			// leave
			if (team == teamplr) {
				multi_disband_team(teamplr);
			} else {
				players[teamplr]._pTeam = teamplr;
			}

			if (teamplr == mypnum)
				EventPlrMsg("You left your team.");
			else
				EventPlrMsg("%s left %s team.", players[teamplr]._pName, team == myplr._pTeam ? "your" : "their");
		}
	}

	return sizeof(*cmd);
}

/**
 * Sync Quest with every player.
 */
static unsigned On_SYNCQUEST(TCmd *pCmd, int pnum)
{
	TCmdQuest *cmd = (TCmdQuest *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (pnum != mypnum)
			SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
		_gbJunkDeltaChanged = true;
	}

	return sizeof(*cmd);
}

/**
 * Same as On_SYNCQUEST, but sends the message only to players on other levels.
 */
static unsigned On_SYNCQUESTEXT(TCmd *pCmd, int pnum)
{
	TCmdQuest *cmd = (TCmdQuest *)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else {
		if (currLvl._dLevelIdx != players[pnum].plrlevel || myplr._pLvlChanging)
			SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
		_gbJunkDeltaChanged = true;
	}

	return sizeof(*cmd);
}

static unsigned On_CHEAT_EXPERIENCE(TCmd *pCmd, int pnum)
{
#ifdef _DEBUG
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else if (players[pnum]._pLevel < MAXCHARLEVEL) {
		players[pnum]._pExperience = players[pnum]._pNextExper;
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
		players[pnum]._pSkillLvl[players[pnum]._pAltAtkSkill]++;
#endif	
	return sizeof(*pCmd);
}

static unsigned On_DEBUG(TCmd *pCmd, int pnum)
{
	return sizeof(*pCmd);
}

static unsigned On_SETSHIELD(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1*)pCmd;

	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, cmd, sizeof(*cmd));
	else
		players[pnum]._pManaShield = cmd->bParam1;

	return sizeof(*cmd);
}

static unsigned On_REMSHIELD(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else
		players[pnum]._pManaShield = 0;

	return sizeof(*pCmd);
}

static unsigned On_OPENSPIL(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else if (currLvl._dLevelIdx == questlist[Q_LTBANNER]._qdlvl) {
		ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
		BYTE tv = dTransVal[2 * setpc_x + 1 + DBORDERX][2 * (setpc_y + 6) + 1 + DBORDERY];
		DRLG_MRectTrans(setpc_x, setpc_y + 3, setpc_x + setpc_w - 1, setpc_y + setpc_h - 1, tv);
		RedoPlayerVision();
	}
	return sizeof(*pCmd);
}

#ifdef HELLFIRE
static unsigned On_OPENHIVE(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else if (currLvl._dLevelIdx == DLV_TOWN) {
		AddMissile(70 + DBORDERX, 52 + DBORDERY, 71 + DBORDERX, 53 + DBORDERY, 0, MIS_HIVEEXPC, 0, pnum, 0, 0, 0);
		T_HiveOpen();
		InitTriggers();
	}
	return sizeof(*pCmd);
}

static unsigned On_OPENCRYPT(TCmd *pCmd, int pnum)
{
	if (geBufferMsgs == MSG_DOWNLOAD_DELTA)
		msg_send_packet(pnum, pCmd, sizeof(*pCmd));
	else if (currLvl._dLevelIdx == DLV_TOWN) {
		PlaySFX(IS_SARC);
		T_CryptOpen();
		InitTriggers();
	}
	return sizeof(*pCmd);
}
#endif

unsigned ParseCmd(int pnum, TCmd *pCmd)
{
	BYTE sbLastCmd = pCmd->bCmd;

	switch (sbLastCmd) {
	case CMD_SYNCDATA:
		return On_SYNCDATA(pCmd, pnum);
	case CMD_WALKXY:
		return On_WALKXY(pCmd, pnum);
	case CMD_SATTACKXY:
		return On_SATTACKXY(pCmd, pnum);
	case CMD_RATTACKXY:
		return On_RATTACKXY(pCmd, pnum);
	case CMD_SPELLXY:
		return On_SPELLXY(pCmd, pnum);
	case CMD_OPOBJXY:
		return On_OPOBJXY(pCmd, pnum);
	case CMD_DISARMXY:
		return On_DISARMXY(pCmd, pnum);
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
	case CMD_BLOCK:
		return On_BLOCK(pCmd, pnum);
	case CMD_KNOCKBACK:
		return On_KNOCKBACK(pCmd, pnum);
	case CMD_TALKXY:
		return On_TALKXY(pCmd, pnum);
	case CMD_MONSTDEATH:
		return On_MONSTDEATH(pCmd, pnum);
	case CMD_MONSTDAMAGE:
		return On_MONSTDAMAGE(pCmd, pnum);
	case CMD_AWAKEGOLEM:
		return On_AWAKEGOLEM(pCmd, pnum);
	case CMD_PLRDEAD:
		return On_PLRDEAD(pCmd, pnum);
	case CMD_PLRRESURRECT:
		return On_PLRRESURRECT(pCmd, pnum);
	case CMD_PLRDAMAGE:
		return On_PLRDAMAGE(pCmd, pnum);
	case CMD_PLRLEVEL:
		return On_PLRLEVEL(pCmd, pnum);
	case CMD_PLRSKILLLVL:
		return On_PLRSKILLLVL(pCmd, pnum);
	case CMD_SETSHIELD:
		return On_SETSHIELD(pCmd, pnum);
	case CMD_REMSHIELD:
		return On_REMSHIELD(pCmd, pnum);
	case CMD_ADDSTR:
		return On_ADDSTR(pCmd, pnum);
	case CMD_ADDMAG:
		return On_ADDMAG(pCmd, pnum);
	case CMD_ADDDEX:
		return On_ADDDEX(pCmd, pnum);
	case CMD_ADDVIT:
		return On_ADDVIT(pCmd, pnum);
	case CMD_DOABILITY:
		return On_DOABILITY(pCmd, pnum);
	case CMD_DOOIL:
		return On_DOOIL(pCmd, pnum);
	case CMD_CHANGEPLRITEM:
		return On_CHANGEPLRITEM(pCmd, pnum);
	case CMD_DELPLRITEM:
		return On_DELPLRITEM(pCmd, pnum);
	case CMD_USEPLRITEM:
		return On_USEPLRITEM(pCmd, pnum);
	case CMD_DROPITEM:
		return On_DROPITEM(pCmd, pnum);
	case CMD_GETITEM:
		return On_GETITEM(pCmd, pnum);
	case CMD_AGETITEM:
		return On_AGETITEM(pCmd, pnum);
	case CMD_PUTITEM:
		return On_PUTITEM(pCmd, pnum);
	case CMD_SYNCPUTITEM:
		return On_SYNCPUTITEM(pCmd, pnum);
	case CMD_RESPAWNITEM:
		return On_RESPAWNITEM(pCmd, pnum);
	case CMD_REQUESTGITEM:
		return On_REQUESTGITEM(pCmd, pnum);
	case CMD_REQUESTAGITEM:
		return On_REQUESTAGITEM(pCmd, pnum);
	case CMD_GOTOGETITEM:
		return On_GOTOGETITEM(pCmd, pnum);
	case CMD_GOTOAGETITEM:
		return On_GOTOAGETITEM(pCmd, pnum);
	case CMD_ITEMEXTRA:
		return On_ITEMEXTRA(pCmd, pnum);
	case CMD_OPERATEOBJ:
		return On_OPERATEOBJ(pCmd, pnum);
	case CMD_OPOBJT:
		return On_OPOBJT(pCmd, pnum);
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
	case CMD_ACTIVATEPORTAL:
		return On_ACTIVATEPORTAL(pCmd, pnum);
	case CMD_DEACTIVATEPORTAL:
		return On_DEACTIVATEPORTAL(pCmd, pnum);
	case CMD_NEWLVL:
		return On_NEWLVL(pCmd, pnum);
	case CMD_WARP:
		return On_WARP(pCmd, pnum);
	case CMD_RETOWN:
		return On_RETOWN(pCmd, pnum);
	case CMD_PLAYER_JOINLEVEL:
		return On_PLAYER_JOINLEVEL(pCmd, pnum);
	case CMD_ACK_PLRINFO:
		return On_ACK_PLRINFO(pCmd, pnum);
	case CMD_SEND_PLRINFO:
		return On_SEND_PLRINFO(pCmd, pnum);
	case CMD_DLEVEL_DATA:
	case CMD_DLEVEL_SEP:
	case CMD_DLEVEL_JUNK:
	case CMD_DLEVEL_END:
		return On_DLEVEL(pCmd, pnum);
	case CMD_STRING:
		return On_STRING(pCmd, pnum);
	case CMD_INVITE:
		return On_INVITE(pCmd, pnum);
	case CMD_ACK_INVITE:
		return On_ACK_INVITE(pCmd, pnum);
	case CMD_DEC_INVITE:
		return On_DEC_INVITE(pCmd, pnum);
	case CMD_REV_INVITE:
		return On_REV_INVITE(pCmd, pnum);
	case CMD_KICK_PLR:
		return On_KICK_PLR(pCmd, pnum);
	case CMD_SYNCQUEST:
		return On_SYNCQUEST(pCmd, pnum);
	case CMD_SYNCQUESTEXT:
		return On_SYNCQUESTEXT(pCmd, pnum);
	case CMD_OPENSPIL:
		return On_OPENSPIL(pCmd, pnum);
#ifdef HELLFIRE
	case CMD_OPENHIVE:
		return On_OPENHIVE(pCmd, pnum);
	case CMD_OPENCRYPT:
		return On_OPENCRYPT(pCmd, pnum);
#endif
	case CMD_CHEAT_EXPERIENCE:
		return On_CHEAT_EXPERIENCE(pCmd, pnum);
	case CMD_CHEAT_SPELL_LEVEL:
		return On_CHEAT_SPELL_LEVEL(pCmd, pnum);
	case CMD_DEBUG:
		return On_DEBUG(pCmd, pnum);
	}

	SNetDropPlayer(pnum);
	return 0;
}

DEVILUTION_END_NAMESPACE
