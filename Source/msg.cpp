/**
 * @file msg.cpp
 *
 * Implementation of function for sending and reciving network messages.
 */
#include "all.h"
#include "diabloui.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#define MAX_CHUNKS				(NUM_LEVELS + MAX_PLRS + 5)
#define DELTA_ERROR_DISCONNECT	MAX_CHUNKS + 1
#define DELTA_ERROR_FAIL_0		MAX_CHUNKS + 2
#define DELTA_ERROR_FAIL_1		MAX_CHUNKS + 3
#define DELTA_ERROR_FAIL_2		MAX_CHUNKS + 4
#define DELTA_ERROR_FAIL_3		MAX_CHUNKS + 5
#define DELTA_ERROR_FAIL_4		MAX_CHUNKS + 6

// fields to handle delta-information
/* The timestamp of the delta download-start. */
//static Uint32 guDeltaStart;
/* Linked list of TMegaPkt entities to keep the messages received during delta-load. */
static TMegaPkt *sgpMegaPkt;
/* The tail of the sgpMegaPkt linked list. */
static TMegaPkt *sgpCurrPkt;
/* Buffer to send/receive delta info. */
static DBuffer sgDeltaSendRecvBuf;
/* Current offset in the delta info buffer. */
static unsigned guGameDeltaOffset;
/* Flag to tell if the delta info is currently processed. */
bool deltaload;
/* Container to keep the delta info of portals and quests. */
static DJunk sgJunkDelta;
/* Container to keep the delta info of items/monsters/objects for each level. */
static DLevel sgLevelDelta[NUM_LEVELS];
/* Container to keep the delta info of the automap. */
static LocalLevel sgLocalDelta[NUM_LEVELS];
/* Container to keep the flags if there is active delta info for each level. */
static bool _gbLevelDeltaChanged[NUM_LEVELS];
/* Specifies whether there is an active delta info in sgJunkDelta. */
static bool _gbJunkDeltaChanged;
/* Counter to keep the progress of delta-load
 * 0..NUM_LEVELS     : level data
 * NUM_LEVELS + 1    : junk data (portals/quests)
 * NUM_LEVELS + 2 .. : players data
 * MAX_CHUNKS - 1    : end of data
 * MAX_CHUNKS        : download success
 */
static BYTE gbGameDeltaChunks;
/* the type of the last delta-load message. (NMSG_DLEVEL_*) */
static BYTE _gbGameDeltaCmd;
/* the current messaging mode. (MSG_*) */
_msg_mode geBufferMsgs = MSG_NORMAL;
/* Buffer holding the character message to send over to other players */
char gbNetMsg[MAX_SEND_STR_LEN];

static void DeltaAllocMegaPkt()
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

static void DeltaFreeMegaPkts()
{
	while (sgpMegaPkt != NULL) {
		sgpCurrPkt = sgpMegaPkt->pNext;
		MemFreeDbg(sgpMegaPkt);
		sgpMegaPkt = sgpCurrPkt;
	}
}

static void DeltaProcessMegaPkts()
{
	int pnum, pktSize;
	BYTE *data, *dataEnd;
	TMegaPkt *pkt;

	for (pkt = sgpMegaPkt; pkt != NULL; pkt = pkt->pNext) {
		data = pkt->data;
		dataEnd = data + sizeof(pkt->data) - pkt->dwSpaceLeft;
		while (data != dataEnd) {
			pnum = *data;
			data++;
			if (*data == FAKE_NMSG_DROPID) {
				TFakeDropPlr *cmd = (TFakeDropPlr *)data;
				multi_player_left(pnum, cmd->dwReason);
				pktSize = sizeof(*cmd);
			} else {
				pktSize = ParseMsg(pnum, (TCmd *)data);
				if (pktSize == 0)
					break;
			}
			data += pktSize;
		}
	}
}

static void DeltaQueuePacket(int pnum, const void *packet, unsigned dwSize)
{
	BYTE* dst;

	if (sgpCurrPkt->dwSpaceLeft < (dwSize + 1))
		DeltaAllocMegaPkt();

	dst = sgpCurrPkt->data + sizeof(sgpCurrPkt->data) - sgpCurrPkt->dwSpaceLeft;
	*dst = pnum;
	dst++;
	memcpy(dst, packet, dwSize);
	sgpCurrPkt->dwSpaceLeft -= dwSize + 1;
}

void msg_send_drop_plr(int pnum, int reason)
{
	TFakeDropPlr cmd;

	cmd.bCmd = FAKE_NMSG_DROPID;
	cmd.dwReason = reason;

	DeltaQueuePacket(pnum, &cmd, sizeof(cmd));
}

static int msg_wait_for_delta()
{
	// TODO: add timeout using guDeltaStart?

	// wait for nthread to download the delta
	SDL_Delay(gnTickDelay);

	if (gbGameDeltaChunks > MAX_CHUNKS)
		return 100;
	if (gbGameDeltaChunks == MAX_CHUNKS - 1) {
		gbGameDeltaChunks = MAX_CHUNKS;
		return 99;
	}
	return 100 * gbGameDeltaChunks / MAX_CHUNKS;
}

bool DownloadDeltaInfo()
{
	bool success;

	// assert(!IsLocalGame);
	DeltaAllocMegaPkt();
	guDeltaTurn = 0;
	gbGameDeltaChunks = 0;
	_gbGameDeltaCmd = NMSG_DLEVEL_END;
	gbDeltaSender = SNPLAYER_ALL;
	guGameDeltaOffset = 0;
	// trigger delta-download in nthread
	geBufferMsgs = MSG_REQUEST_GAME_DELTA;
	//guDeltaStart = SDL_GetTicks();
	success = UiProgressDialog("Waiting for game data...", msg_wait_for_delta);
	assert(geBufferMsgs == MSG_NORMAL || !success || gbGameDeltaChunks != MAX_CHUNKS);
	geBufferMsgs = MSG_NORMAL;
	if (success) {
		if (gbGameDeltaChunks == MAX_CHUNKS)
			return true;
#ifdef _DEVMODE
		DrawDlg(/*gbGameDeltaChunks == DELTA_ERROR_DISCONNECT ? "The game ended %d" :*/"Unable to get game data %d", gbGameDeltaChunks);
#else
		DrawDlg(/*gbGameDeltaChunks == DELTA_ERROR_DISCONNECT ? "The game ended" :*/"Unable to get game data");
#endif
	}
	DeltaFreeMegaPkts();
	return false;
}

void RunDeltaPackets()
{
	//if (IsLocalGame)
	//	return;

	//geBufferMsgs = MSG_RUN_DELTA;
	DeltaProcessMegaPkts();
	//geBufferMsgs = MSG_NORMAL;
	DeltaFreeMegaPkts();
}

static BYTE *DeltaExportLevel(BYTE bLvl)
{
	DItemStr *item;
	DMonsterStr *mon;
	int i;
	BYTE* dst = sgDeltaSendRecvBuf.content;

	static_assert(sizeof(sgDeltaSendRecvBuf.content) >= sizeof(DLevel) + 1, "Delta-Level might not fit to the buffer.");

	// level-index
	*dst = bLvl;
	dst++;

	// export items
	item = sgLevelDelta[bLvl].item;
	for (i = 0; i < lengthof(sgLevelDelta[bLvl].item); i++, item++) {
		if (item->bCmd == 0xFF) {
			*dst = 0xFF;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DItemStr *>(dst), *item);
			dst += sizeof(DItemStr);
		}
	}

	// export objects
	memcpy(dst, sgLevelDelta[bLvl].object, sizeof(sgLevelDelta[bLvl].object));
	dst += sizeof(sgLevelDelta[bLvl].object);

	// export monsters
	mon = sgLevelDelta[bLvl].monster;
	for (i = 0; i < lengthof(sgLevelDelta[bLvl].monster); i++, mon++) {
		if (mon->_mx == 0xFF) {
			*dst = 0xFF;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DMonsterStr *>(dst), *mon);
			dst += sizeof(DMonsterStr);
		}
	}

	return dst;
}

static void DeltaImportLevel()
{
	DItemStr *item;
	DMonsterStr *mon;
	int i;
	BYTE *src, bLvl;

	src = sgDeltaSendRecvBuf.content;
	// level-index
	bLvl = *src;
	src++;

	_gbLevelDeltaChanged[bLvl] = true;

	// import items
	item = sgLevelDelta[bLvl].item;
	for (i = 0; i < MAXITEMS; i++, item++) {
		if (*src == 0xFF) {
			memset(item, 0xFF, sizeof(DItemStr));
			src++;
		} else {
			copy_pod(*item, *reinterpret_cast<DItemStr *>(src));
			// TODO: validate data from internet
			// assert(dst->bCmd == DCMD_SPAWNED || dst->bCmd == DCMD_TAKEN || dst->bCmd == DCMD_DROPPED);
			src += sizeof(DItemStr);
		}
	}
	// import objects
	memcpy(sgLevelDelta[bLvl].object, src, sizeof(sgLevelDelta[bLvl].object));
	src += sizeof(sgLevelDelta[bLvl].object);

	// import monsters
	mon = sgLevelDelta[bLvl].monster;
	for (i = 0; i < MAXMONSTERS; i++, mon++) {
		if (*src == 0xFF) {
			memset(mon, 0xFF, sizeof(DMonsterStr));
			src++;
		} else {
			copy_pod(*mon, *reinterpret_cast<DMonsterStr *>(src));
			src += sizeof(DMonsterStr);
		}
	}
}

static BYTE *DeltaExportJunk()
{
	DQuest *mq;
	int i;
	BYTE* dst = sgDeltaSendRecvBuf.content;

	// TODO: add delta_SetMultiQuest instead?
	mq = sgJunkDelta.jQuests;
	for (i = 0; i < NUM_QUESTS; i++) {
		mq->qlog = quests[i]._qlog;
		mq->qstate = quests[i]._qactive;
		mq->qvar1 = quests[i]._qvar1;
		mq++;
	}

	static_assert(sizeof(sgDeltaSendRecvBuf.content) >= sizeof(sgJunkDelta), "Delta-Junk does not fit to the buffer.");
	// export portals + quests
	memcpy(dst, &sgJunkDelta, sizeof(sgJunkDelta));
	dst += sizeof(sgJunkDelta);

	return dst;
}

static void DeltaImportJunk()
{
	DPortal *pD;
	DQuest *mq;
	int i;
	BYTE* src = sgDeltaSendRecvBuf.content;

	_gbJunkDeltaChanged = true;

	// import portals + quests
	memcpy(&sgJunkDelta, src, sizeof(sgJunkDelta));
	//src += sizeof(sgJunkDelta);

	// update the game state
	// portals
	pD = sgJunkDelta.jPortals;
	for (i = 0; i < MAXPORTAL; i++, pD++) {
		if (pD->x != 0xFF) {
			ActivatePortal(i, pD->x, pD->y, pD->level);
		}
		//else
		//	SetPortalStats(i, false, 0, 0, 0);
	}
	// quests
	mq = sgJunkDelta.jQuests;
	for (i = 0; i < NUM_QUESTS; i++, mq++) {
		if (mq->qstate != 0xFF) {
			quests[i]._qlog = mq->qlog;
			quests[i]._qactive = mq->qstate;
			quests[i]._qvar1 = mq->qvar1;
		}
	}
}

static BYTE* DeltaExportPlr(int pnum)
{
	BYTE* dst = sgDeltaSendRecvBuf.content;

	static_assert(sizeof(sgDeltaSendRecvBuf.content) >= sizeof(PkPlayerStruct) + 1, "Delta-Plr does not fit to the buffer.");

	// player-index
	*dst = pnum;
	dst++;

	PackPlayer((PkPlayerStruct*)dst, pnum);

	return dst + sizeof(PkPlayerStruct);
}

static void DeltaImportPlr()
{
	int pnum;
	BYTE* src = sgDeltaSendRecvBuf.content;

	// player-index
	pnum = *src;
	if (pnum >= MAX_PLRS) {
		// invalid data -> quit
		gbGameDeltaChunks = DELTA_ERROR_FAIL_4;
		return;
	}
	src++;

	UnPackPlayer((PkPlayerStruct*)src, pnum);

	assert(!plr._pActive);
	plr._pActive = TRUE;
	gbActivePlayers++;
	EventPlrMsg("Player '%s' (level %d) is already in the game", plr._pName, plr._pLevel);
}

static DWORD DeltaCompressData(BYTE *end)
{
	DWORD size, pkSize;

	size = end - sgDeltaSendRecvBuf.content;
	pkSize = PkwareCompress(sgDeltaSendRecvBuf.content, size);
	sgDeltaSendRecvBuf.compressed = size != pkSize;

	return pkSize + sizeof(sgDeltaSendRecvBuf.compressed);
}

void DeltaExportData(int pnum, uint32_t turn)
{
	BYTE *dstEnd;
	int size, i;
	BYTE src, numChunks = 0;

	// levels
	for (i = 0; i < lengthof(sgLevelDelta); i++) {
		if (!_gbLevelDeltaChanged[i])
			continue;
		dstEnd = DeltaExportLevel(i);
		size = DeltaCompressData(dstEnd);
		dthread_send_delta(pnum, NMSG_DLEVEL_DATA, &sgDeltaSendRecvBuf, size);
		src = 0;
		dthread_send_delta(pnum, NMSG_DLEVEL_SEP, &src, 1);
		numChunks++;
	}
	// junk
	if (_gbJunkDeltaChanged) {
		dstEnd = DeltaExportJunk();
		size = DeltaCompressData(dstEnd);
		dthread_send_delta(pnum, NMSG_DLEVEL_JUNK, &sgDeltaSendRecvBuf, size);
		numChunks++;
	}
	// players
	for (i = 0; i < MAX_PLRS; i++) {
		if (plx(i)._pActive) {
			dstEnd = DeltaExportPlr(i);
			size = DeltaCompressData(dstEnd);
			dthread_send_delta(pnum, NMSG_DLEVEL_PLR, &sgDeltaSendRecvBuf, size);
			src = 0;
			dthread_send_delta(pnum, NMSG_DLEVEL_SEP, &src, 1);
			numChunks++;
		}
	}

	// current number of chunks sent + turn-id + end
	sgDeltaSendRecvBuf.compressed = FALSE;
	dstEnd = sgDeltaSendRecvBuf.content;
	*dstEnd = numChunks;
	dstEnd++;
	*(uint32_t*)dstEnd = SDL_SwapLE32(turn);
	dthread_send_delta(pnum, NMSG_DLEVEL_END, &sgDeltaSendRecvBuf, sizeof(sgDeltaSendRecvBuf.compressed) + sizeof(BYTE) + sizeof(uint32_t));
}

static void DeltaImportData()
{
	if (sgDeltaSendRecvBuf.compressed)
		PkwareDecompress(sgDeltaSendRecvBuf.content, guGameDeltaOffset, sizeof(sgDeltaSendRecvBuf.content));

	gbGameDeltaChunks++;

	if (_gbGameDeltaCmd == NMSG_DLEVEL_DATA) {
		DeltaImportLevel();
	} else if (_gbGameDeltaCmd == NMSG_DLEVEL_JUNK) {
		DeltaImportJunk();
	} else {
		assert(_gbGameDeltaCmd == NMSG_DLEVEL_PLR);
		DeltaImportPlr();
	}
}

static void DeltaImportEnd(TCmdPlrInfoHdr *cmd)
{
	DBuffer* buf;
	BYTE* data;

	// stop nthread from processing the the normal messages
	geBufferMsgs = MSG_NORMAL;

	assert(cmd->wBytes == SwapLE16(sizeof(sgDeltaSendRecvBuf.compressed) + sizeof(BYTE) + sizeof(uint32_t)));
	buf = (DBuffer*)&cmd[1];
	assert(!buf->compressed);
	data = buf->content;
	if (gbGameDeltaChunks != *data) {
		// not all chunks arrived -> quit
		gbGameDeltaChunks = DELTA_ERROR_FAIL_3;
		return;
	}
	data++;
	guDeltaTurn = SDL_SwapLE32(*(uint32_t*)data);
	gbGameDeltaChunks = MAX_CHUNKS - 1;
}

static unsigned On_DLEVEL(TCmd *pCmd, int pnum)
{
	TCmdPlrInfoHdr *cmd = (TCmdPlrInfoHdr *)pCmd;

	if (geBufferMsgs != MSG_GAME_DELTA)
		goto done; // the player is already active -> drop the packet

	if (gbDeltaSender != pnum) {
		if (gbDeltaSender != SNPLAYER_ALL) {
			// delta is already on its way from a different player -> drop the packet
			goto done;
		}
		if (guGameDeltaOffset != 0) {
			// the source of the delta is dropped -> drop the packages and quit
			gbGameDeltaChunks = DELTA_ERROR_FAIL_0;
			goto done;
		}
		if (cmd->wOffset != 0) {
			// invalid data starting offset -> drop the packet
			goto done;
		}
		if (cmd->bCmd == NMSG_DLEVEL_END) {
			// nothing received till now -> empty delta
			gbDeltaSender = pnum;
			// _gbGameDeltaCmd = NMSG_DLEVEL_END;
			DeltaImportEnd(cmd);
			goto done;
		}
		if (cmd->bCmd != NMSG_DLEVEL_DATA && cmd->bCmd != NMSG_DLEVEL_JUNK && cmd->bCmd != NMSG_DLEVEL_PLR) {
			// invalid data starting type -> drop the packet
			goto done;
		}
		/*if ((unsigned)pnum >= MAX_PLRS && pnum != SNPLAYER_MASTER) {
			// message from an invalid player -> drop the packet
			goto done;
		}*/
		// start receiving
		gbDeltaSender = pnum;
		_gbGameDeltaCmd = cmd->bCmd;
		// guGameDeltaOffset = 0;
	} else {
		// a packet from a previous sender
		if (_gbGameDeltaCmd != cmd->bCmd) {
			// process previous package
			if (_gbGameDeltaCmd != NMSG_DLEVEL_SEP && _gbGameDeltaCmd != NMSG_DLEVEL_END)
				DeltaImportData();
			_gbGameDeltaCmd = cmd->bCmd;
			if (_gbGameDeltaCmd == NMSG_DLEVEL_END) {
				// final package -> done
				DeltaImportEnd(cmd);
				goto done;
			} else if (_gbGameDeltaCmd == NMSG_DLEVEL_SEP) {
				// separator package -> wait for next
				goto done;
			} else {
				// start receiving a new package
				assert(_gbGameDeltaCmd == NMSG_DLEVEL_DATA || _gbGameDeltaCmd == NMSG_DLEVEL_JUNK || _gbGameDeltaCmd == NMSG_DLEVEL_PLR);
				guGameDeltaOffset = 0;
			}
		} else {
			// continue previous package
			if (_gbGameDeltaCmd != NMSG_DLEVEL_DATA && _gbGameDeltaCmd != NMSG_DLEVEL_JUNK && _gbGameDeltaCmd != NMSG_DLEVEL_PLR) {
				// lost or duplicated package -> drop the connection and quit
				gbGameDeltaChunks = DELTA_ERROR_FAIL_1;
				goto done;
			}
		}
	}

	if (cmd->wOffset != guGameDeltaOffset) {
		// lost or duplicated package -> drop the connection and quit
		gbGameDeltaChunks = DELTA_ERROR_FAIL_2;
		goto done;
	}

	memcpy(((BYTE*)&sgDeltaSendRecvBuf) + cmd->wOffset, &cmd[1], cmd->wBytes);
	guGameDeltaOffset += cmd->wBytes;
done:
	return cmd->wBytes + sizeof(*cmd);
}

void delta_init()
{
	_gbJunkDeltaChanged = false;
	memset(_gbLevelDeltaChanged, 0, sizeof(_gbLevelDeltaChanged));
	memset(&sgJunkDelta, 0xFF, sizeof(sgJunkDelta));
	memset(sgLevelDelta, 0xFF, sizeof(sgLevelDelta));
	memset(sgLocalDelta, 0, sizeof(sgLocalDelta));
	deltaload = false;
}

static void delta_kill_monster(const TCmdMonstKill* mon)
{
	DMonsterStr *pD;
	BYTE bLevel;

	if (!IsMultiGame)
		return;

	bLevel = mon->mkLevel;
	// TODO: validate bLevel - assert(bLevel < NUM_LEVELS);

	_gbLevelDeltaChanged[bLevel] = true;
	pD = &sgLevelDelta[bLevel].monster[SwapLE16(mon->mkMnum)];
	pD->_mx = mon->mkX;
	pD->_my = mon->mkY;
	pD->_mdir = mon->mkDir;
	pD->_mhitpoints = 0;
}

static void delta_monster_hp(const TCmdMonstDamage* mon)
{
	DMonsterStr *pD;
	BYTE bLevel;

	if (!IsMultiGame)
		return;

	bLevel = mon->mdLevel;
	// TODO: validate bLevel - assert(bLevel < NUM_LEVELS);

	_gbLevelDeltaChanged[bLevel] = true;
	pD = &sgLevelDelta[bLevel].monster[SwapLE16(mon->mdMnum)];
	// In vanilla code the value was discarded if hp was higher than the current one.
	// That disregards the healing monsters.
	// Now it is always updated except the monster is already dead.
	if (pD->_mhitpoints != 0)
		pD->_mhitpoints = mon->mdHitpoints;
}

static void delta_sync_monster(const TSyncHeader *pHdr)
{
	DLevel* pDLvl;
	DMonsterStr *pD;
	WORD wLen;
	const TSyncMonster* pSync;
	const BYTE *pbBuf;

	if (!IsMultiGame)
		return;

	// TODO: validate bLevel - assert(pHdr->bLevel < NUM_LEVELS);

	_gbLevelDeltaChanged[pHdr->bLevel] = true;
	pDLvl = &sgLevelDelta[pHdr->bLevel];

	pbBuf = (const BYTE *)&pHdr[1];

	for (wLen = SwapLE16(pHdr->wLen); wLen >= sizeof(TSyncMonster); wLen -= sizeof(TSyncMonster)) {
		pSync = (TSyncMonster *)pbBuf;
		pD = &pDLvl->monster[pSync->_mndx];
		if (pD->_mhitpoints != 0) {
			pD->_mx = pSync->_mx;
			pD->_my = pSync->_my;
			pD->_menemy = pSync->_menemy;
			pD->_mactive = pSync->_mactive;
			pD->_mhitpoints = pSync->_mhitpoints;
			pD->_mdir = pSync->_mdir;
		}
		pbBuf += sizeof(TSyncMonster);
	}
	assert(wLen == 0);
}

static void delta_awake_golem(TCmdGolem *pG, int mnum)
{
	DMonsterStr *pD;
	BYTE bLevel;

	if (!IsMultiGame)
		return;

	bLevel = pG->_currlevel;
	// TODO: validate bLevel - assert(bLevel < NUM_LEVELS);

	_gbLevelDeltaChanged[bLevel] = true;
	pD = &sgLevelDelta[bLevel].monster[mnum];
	pD->_mx = pG->_mx;
	pD->_my = pG->_my;
	pD->_mactive = SQUELCH_MAX;
	pD->_menemy = pG->_menemy;
	pD->_mdir = pG->_mdir;
	pD->_mhitpoints = pG->_mhitpoints;
}

static void delta_leave_sync(BYTE bLevel)
{
	if (bLevel == DLV_TOWN) {
		glSeedTbl[DLV_TOWN] = GetRndSeed();
		return;
	}
	memcpy(&sgLocalDelta[bLevel].automapsv, automapview, sizeof(automapview));
}

static void delta_sync_object(int oi, BYTE bCmd, BYTE bLevel)
{
	if (!IsMultiGame)
		return;

	_gbLevelDeltaChanged[bLevel] = true;
	sgLevelDelta[bLevel].object[oi].bCmd = bCmd;
}

static bool delta_get_item(const TCmdGItem *pI)
{
	DItemStr *pD;
	int i;
	BYTE bLevel;

	if (!IsMultiGame)
		return true;

	bLevel = pI->bLevel;
	// TODO: validate bLevel - assert(bLevel < NUM_LEVELS);

	pD = sgLevelDelta[bLevel].item;
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

	pD = sgLevelDelta[bLevel].item;
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

static void delta_put_item(const PkItemStruct* pItem, BYTE bLevel, int x, int y)
{
	int i;
	DItemStr *pD;

	if (!IsMultiGame)
		return;

	// TODO: validate bLevel - assert(bLevel < NUM_LEVELS);

	pD = sgLevelDelta[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != 0xFF
		 && pD->item.dwSeed == pItem->dwSeed
		 && pD->item.wIndx == pItem->wIndx
		 && pD->item.wCI == pItem->wCI) {
			if (pD->bCmd == DCMD_DROPPED)
				return;
			if (pD->bCmd == DCMD_TAKEN)
				continue; // BUGFIX: should return instead? otherwise the item is duped...
			app_fatal("Trying to drop a floor item?");
		}
	}

	pD = sgLevelDelta[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == 0xFF) {
			_gbLevelDeltaChanged[bLevel] = true;
			pD->bCmd = DCMD_DROPPED;
			pD->x = x;
			pD->y = y;
			copy_pod(pD->item, *pItem);
			return;
		}
	}
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
	DItemStr *pD;

	if (!IsMultiGame)
		return;

	is = &items[ii];
	pD = sgLevelDelta[currLvl._dLevelIdx].item;
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

	pD = sgLevelDelta[currLvl._dLevelIdx].item;
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
	assert(IsMultiGame);
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
	DItemStr* itm;
	int ii;
	int i;
	int x, y;

	assert(IsMultiGame);

	deltaload = true;
	if (currLvl._dLevelIdx != DLV_TOWN) {
		mstr = sgLevelDelta[currLvl._dLevelIdx].monster;
		for (i = 0; i < nummonsters; i++, mstr++) {
			if (mstr->_mx != 0xFF) {
				// skip minions and prespawn skeletons
				if (!MINION_NR_INACTIVE(i))
					RemoveMonFromMap(i);
				x = mstr->_mx;
				y = mstr->_my;
				mon = &monsters[i];
				SetMonsterLoc(mon, x, y);
				// check if only the position of the monster was modified
				if (mstr->_mhitpoints != -1)
					mon->_mhitpoints = SwapLE32(mstr->_mhitpoints);
				if (mstr->_mhitpoints == 0) {
					// SetDead: inline for better performance
					if (mon->mlid != NO_LIGHT)
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

		memcpy(automapview, sgLocalDelta[currLvl._dLevelIdx].automapsv, sizeof(automapview));

		dstr = sgLevelDelta[currLvl._dLevelIdx].object;
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
	itm = sgLevelDelta[currLvl._dLevelIdx].item;
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

	deltaload = false;
}

void NetSendCmdSendJoinLevel()
{
	TCmdSendJoinLevel cmd;

	cmd.bCmd = CMD_SEND_JOINLEVEL;
	cmd.lLevel = currLvl._dLevelIdx;
	cmd.px = ViewX;
	cmd.py = ViewY;
	cmd.lTimer1 = SwapLE16(myplr._pTimer[PLTR_INFRAVISION]);
	cmd.lTimer2 = SwapLE16(myplr._pTimer[PLTR_RAGE]);

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdAckJoinLevel()
{
	TCmdAckJoinLevel cmd;

	cmd.bCmd = CMD_ACK_JOINLEVEL;
	cmd.lManashield = myplr._pManaShield;
	cmd.lTimer1 = SwapLE16(myplr._pTimer[PLTR_INFRAVISION]);
	cmd.lTimer2 = SwapLE16(myplr._pTimer[PLTR_RAGE]);

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
	//dthread_send_delta(pnum, CMD_ACK_JOINLEVEL, &cmd, sizeof(cmd));
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
	cmd.mkX = monsters[mnum]._mx;
	cmd.mkY = monsters[mnum]._my;
	cmd.mkDir = monsters[mnum]._mdir;
	cmd.mkLevel = plr._pDunLevel;
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

void NetSendShrineCmd(BYTE type, int seed)
{
	TCmdShrine cmd;

	cmd.bCmd = CMD_SHRINE;
	cmd.shType = type;
	cmd.shSeed = SwapLE32(seed);

	NetSendHiPri((BYTE*)&cmd, sizeof(cmd));
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
	cmd.bLevel = currLvl._dLevelIdx;
	is = &items[ii];
	cmd.x = is->_ix;
	cmd.y = is->_iy;

	PackPkItem(&cmd.item, is);

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdPutItem(BYTE x, BYTE y)
{
	TCmdPItem cmd;

	cmd.bCmd = CMD_PUTITEM;
	cmd.bLevel = currLvl._dLevelIdx;
	cmd.x = x;
	cmd.y = y;

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdRespawnItem(int ii)
{
	ItemStruct *is;
	TCmdRPItem cmd;

	is = &items[ii];
	cmd.bCmd = CMD_RESPAWNITEM;
	cmd.x = is->_ix;
	cmd.y = is->_iy;
	cmd.bLevel = currLvl._dLevelIdx;

	PackPkItem(&cmd.item, is);

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
	TCmdRPItem cmd;

	is = &items[ii];
	cmd.bCmd = CMD_DPUTITEM;
	cmd.x = is->_ix;
	cmd.y = is->_iy;
	cmd.bLevel = currLvl._dLevelIdx;

	PackPkItem(&cmd.item, is);

	NetSendHiPri((BYTE *)&cmd, sizeof(cmd));
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

void NetSendCmdMonstDamage(int mnum, int hitpoints)
{
	TCmdMonstDamage cmd;

	cmd.bCmd = CMD_MONSTDAMAGE;
	cmd.mdLevel = currLvl._dLevelIdx;
	cmd.mdMnum = SwapLE16(mnum);
	cmd.mdHitpoints = SwapLE32(hitpoints);

	NetSendLoPri((BYTE *)&cmd, sizeof(cmd));
}

void NetSendCmdString(unsigned int pmask)
{
	int dwStrLen;
	TCmdString cmd;

	dwStrLen = strlen(gbNetMsg);
	cmd.bCmd = NMSG_STRING;
	memcpy(cmd.str, gbNetMsg, dwStrLen + 1);
	multi_send_direct_msg(pmask, (BYTE *)&cmd, dwStrLen + 2);
}

static void delta_open_portal(int pnum, BYTE x, BYTE y, BYTE bLevel)
{
	_gbJunkDeltaChanged = true;
	sgJunkDelta.jPortals[pnum].x = x;
	sgJunkDelta.jPortals[pnum].y = y;
	sgJunkDelta.jPortals[pnum].level = bLevel;
}

void delta_close_portal(int pnum)
{
	//memset(&sgJunkDelta.portal[pnum], 0xFF, sizeof(sgJunkDelta.portal[pnum]));
	sgJunkDelta.jPortals[pnum].x = 0xFF;
	// assert(_gbJunkDeltaChanged == true);
}

static void check_update_plr(int pnum)
{
	if (IsMultiGame && pnum == mypnum)
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

	//if (pnum != mypnum && currLvl._dLevelIdx == pHdr->bLevel)
	//	sync_update(pnum, pHdr);
	delta_sync_monster(pHdr);

	return SwapLE16(pHdr->wLen) + sizeof(*pHdr);
}

static unsigned On_WALKXY(TCmd *pCmd, int pnum)
{
	TCmdLoc *cmd = (TCmdLoc *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		MakePlrPath(pnum, cmd->x, cmd->y, true);
		plr.destAction = ACTION_NONE;
	}

	return sizeof(*cmd);
}

static unsigned On_ADDSTR(TCmd *pCmd, int pnum)
{
	IncreasePlrStr(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDMAG(TCmd *pCmd, int pnum)
{
	IncreasePlrMag(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDDEX(TCmd *pCmd, int pnum)
{
	IncreasePlrDex(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDVIT(TCmd *pCmd, int pnum)
{
	IncreasePlrVit(pnum);

	return sizeof(*pCmd);
}

static unsigned On_BLOCK(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_BLOCK;
		plr.destParam1 = cmd->bParam1; // direction
	}
	return sizeof(*cmd);
}

static unsigned On_GOTOGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr.destAction = ACTION_PICKUPITEM;
		plr.destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_GETITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;
	int ii;

	if (delta_get_item(cmd)) {
		if (plr._pHoldItem._itype != ITYPE_NONE) {
			PkItemStruct pkItem;
			PackPkItem(&pkItem, &plr._pHoldItem);
			delta_put_item(&pkItem, cmd->bLevel, cmd->x, cmd->y);
			// plr._pHoldItem._itype = ITYPE_NONE;
		}
		if (currLvl._dLevelIdx == cmd->bLevel) {
			ii = FindGetItem(SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
			assert(ii != -1);
			InvGetItem(pnum, ii);
		} else {
			UnPackPkItem(&cmd->item);
			copy_pod(plr._pHoldItem, items[MAXITEMS]);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_GOTOAGETITEM(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr.destAction = ACTION_PICKUPAITEM;
		plr.destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_AUTOGETITEM(TCmd *pCmd, int pnum)
{
	TCmdGItem *cmd = (TCmdGItem *)pCmd;
	int ii;
	bool result;

	if (delta_get_item(cmd)) {
		if (currLvl._dLevelIdx == cmd->bLevel) {
			ii = FindGetItem(SwapLE32(cmd->item.dwSeed), SwapLE16(cmd->item.wIndx), SwapLE16(cmd->item.wCI));
			assert(ii != -1);
			result = AutoGetItem(pnum, ii);
		} else {
			UnPackPkItem(&cmd->item);
			ii = MAXITEMS;
			result = SyncAutoGetItem(pnum, MAXITEMS);
		}
		if (!result) {
			PkItemStruct pkItem;
			PackPkItem(&pkItem, &items[ii]);
			delta_put_item(&pkItem, cmd->bLevel, cmd->x, cmd->y);
		}
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
		// open hive
		if (currLvl._dLevelIdx == DLV_TOWN) {
			AddMissile(70 + DBORDERX, 52 + DBORDERY, 71 + DBORDERX, 53 + DBORDERY, 0, MIS_HIVEEXPC, 0, pnum, 0, 0, 0);
			gbOpenWarps |= (1 << TWARP_NEST);
			T_HiveOpen();
			InitTriggers();
		}
		return true;
	}
	if (iidx == IDI_MAPOFDOOM
	 && x >= DBORDERX + 25  && x <= DBORDERX + 28 && y >= DBORDERY + 10 && y <= DBORDERY + 14
	 && quests[Q_GRAVE]._qactive != QUEST_DONE) {
		quests[Q_GRAVE]._qactive = QUEST_DONE;
		// open crypt
		if (currLvl._dLevelIdx == DLV_TOWN) {
			PlaySFX(IS_SARC);
			gbOpenWarps |= (1 << TWARP_CRYPT);
			T_CryptOpen();
			InitTriggers();
		}
		return true;
	}
	return false;
}
#endif

static unsigned On_PUTITEM(TCmd *pCmd, int pnum)
{
	TCmdPItem* cmd = (TCmdPItem*)pCmd;
	ItemStruct* pi;
	int x, y;

	pi = &plr._pHoldItem;
	if (pi->_itype != ITYPE_NONE) {
		x = cmd->x;
		y = cmd->y;
#ifdef HELLFIRE
		if (plr._pDunLevel == DLV_TOWN && CheckTownTrigs(pnum, x, y, pi->_iIdx)) {
			pi->_itype = ITYPE_NONE;
			if (pnum == mypnum) {
				check_update_plr(pnum);
				NewCursor(CURSOR_HAND);
			}
			return sizeof(*cmd);
		}
#endif
		PkItemStruct pkItem;
		PackPkItem(&pkItem, pi);
		delta_put_item(&pkItem, cmd->bLevel, x, y);

		if (currLvl._dLevelIdx == cmd->bLevel) {
			copy_pod(items[MAXITEMS], *pi);
			pi->_itype = ITYPE_NONE;
			SyncPutItem(pnum, x, y, true);
			if (pnum == mypnum) {
				check_update_plr(pnum);
				NewCursor(CURSOR_HAND);
			}
		} else {
			pi->_itype = ITYPE_NONE;
		}
	}

	return sizeof(*cmd);
}

static unsigned On_RESPAWNITEM(TCmd *pCmd, int pnum)
{
	TCmdRPItem* cmd = (TCmdRPItem*)pCmd;

	if (currLvl._dLevelIdx == cmd->bLevel && pnum != mypnum) {
		UnPackPkItem(&cmd->item);
		SyncPutItem(pnum, cmd->x, cmd->y, false);
	}
	delta_put_item(&cmd->item, cmd->bLevel, cmd->x, cmd->y);

	return sizeof(*cmd);
}

static unsigned On_DPUTITEM(TCmd *pCmd, int pnum)
{
	TCmdRPItem* cmd = (TCmdRPItem*)pCmd;

	delta_put_item(&cmd->item, cmd->bLevel, cmd->x, cmd->y);

	return sizeof(*cmd);
}

static unsigned On_SATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLocAttack *cmd = (TCmdLocAttack *)pCmd;
	int sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		sn = cmd->laSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_ATTACK;
			plr.destParam1 = cmd->x;
			plr.destParam2 = cmd->y;
			plr.destParam3 = sn;           // attack skill
			plr.destParam4 = cmd->laLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKXY(TCmd *pCmd, int pnum)
{
	TCmdLocAttack *cmd = (TCmdLocAttack *)pCmd;
	int sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		sn = cmd->laSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_RATTACK;
			plr.destParam1 = cmd->x;
			plr.destParam2 = cmd->y;
			plr.destParam3 = sn;           // attack skill
			plr.destParam4 = cmd->laLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLXY(TCmd *pCmd, int pnum)
{
	TCmdLocSkill *cmd = (TCmdLocSkill *)pCmd;
	BYTE sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		sn = cmd->lsSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_SPELL;
			plr.destParam2 = cmd->x;
			plr.destParam3 = cmd->y;
			plr.destParam1a = sn;           // spell
			plr.destParam1b = cmd->lsFrom;  // invloc
			plr.destParam1c = cmd->lsLevel; // spllvl
		} else
			msg_errorf("%s has cast an illegal spell.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_DOABILITY(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	DoAbility(pnum, cmd->bParam1, cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_DOOIL(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	DoOil(pnum, cmd->bParam1, cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_OPOBJXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		int oi = SwapLE16(cmd->wParam1);
		plr.destAction = ACTION_OPERATE;
		plr.destParam1 = oi;
		plr.destParam2 = cmd->x;
		plr.destParam3 = cmd->y;
		MakePlrPath(pnum, cmd->x, cmd->y, !(objects[oi]._oSolidFlag | objects[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_DISARMXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		int oi = SwapLE16(cmd->wParam1);
		plr.destAction = ACTION_DISARM;
		plr.destParam1 = oi;
		plr.destParam2 = cmd->x;
		plr.destParam3 = cmd->y;
		MakePlrPath(pnum, cmd->x, cmd->y, !(objects[oi]._oSolidFlag | objects[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_OPOBJT(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		plr.destAction = ACTION_OPERATETK;
		plr.destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKID(TCmd *pCmd, int pnum)
{
	TCmdMonstAttack *cmd = (TCmdMonstAttack *)pCmd;
	int mnum, x, y, sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		mnum = SwapLE16(cmd->maMnum);
		x = monsters[mnum]._mfutx;
		y = monsters[mnum]._mfuty;
		if (abs(plr._px - x) > 1 || abs(plr._py - y) > 1)
			MakePlrPath(pnum, x, y, false);
		sn = cmd->maSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_ATTACKMON;
			plr.destParam1 = mnum;
			plr.destParam2 = sn;           // attack skill
			plr.destParam3 = cmd->maLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdPlrAttack *cmd = (TCmdPlrAttack *)pCmd;
	int tnum, sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		tnum = cmd->paPnum;
		MakePlrPath(pnum, plx(tnum)._pfutx, plx(tnum)._pfuty, false);
		sn = cmd->paSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_ATTACKPLR;
			plr.destParam1 = tnum;
			plr.destParam2 = sn;           // attack skill
			plr.destParam3 = cmd->paLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKID(TCmd *pCmd, int pnum)
{
	TCmdMonstAttack *cmd = (TCmdMonstAttack *)pCmd;
	int sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		sn = cmd->maSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_RATTACKMON;
			plr.destParam1 = SwapLE16(cmd->maMnum);  // target id
			plr.destParam2 = sn;                     // attack skill
			plr.destParam3 = cmd->maLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKPID(TCmd *pCmd, int pnum)
{
	TCmdPlrAttack *cmd = (TCmdPlrAttack *)pCmd;
	int sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		sn = cmd->paSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_RATTACKPLR;
			plr.destParam1 = cmd->paPnum;  // target id
			plr.destParam2 = sn;           // attack skill
			plr.destParam3 = cmd->paLevel; // attack skill-level
		} else
			msg_errorf("%s using an illegal skill.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLID(TCmd *pCmd, int pnum)
{
	TCmdMonstSkill *cmd = (TCmdMonstSkill *)pCmd;
	BYTE sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		sn = cmd->msSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_SPELLMON;
			plr.destParam1a = sn;                    // spell
			plr.destParam1b = cmd->msFrom;           // invloc
			plr.destParam1c = cmd->msLevel;          // spllvl
			plr.destParam2 = SwapLE16(cmd->msMnum);  // mnum
		} else
			msg_errorf("%s has cast an illegal spell.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLPID(TCmd *pCmd, int pnum)
{
	TCmdPlrSkill *cmd = (TCmdPlrSkill *)pCmd;
	BYTE sn;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		sn = cmd->psSkill;
		if ((spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
			plr.destAction = ACTION_SPELLPLR;
			plr.destParam1a = sn;           // spell
			plr.destParam1b = cmd->psFrom;  // invloc
			plr.destParam1c = cmd->psLevel; // spllvl
			plr.destParam2 = cmd->psPnum;   // pnum
		} else
			msg_errorf("%s has cast an illegal spell.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_KNOCKBACK(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		// assert(cmd->wParam1 >= MAX_MINIONS); TODO: validate data from internet
		MonGetKnockback(SwapLE16(cmd->wParam1));
		MonStartHit(SwapLE16(cmd->wParam1), pnum, 0);
	}

	return sizeof(*cmd);
}

static unsigned On_TALKXY(TCmd *pCmd, int pnum)
{
	TCmdLocParam1 *cmd = (TCmdLocParam1 *)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr.destAction = ACTION_TALK;
		plr.destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_NEWLVL(TCmd *pCmd, int pnum)
{
	TCmdParam2 *cmd = (TCmdParam2 *)pCmd;

	StartNewLvl(pnum, SwapLE16(cmd->wParam1), SwapLE16(cmd->wParam2));

	return sizeof(*cmd);
}

static unsigned On_TWARP(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	StartTWarp(pnum, SwapLE16(cmd->wParam1));

	return sizeof(*cmd);
}

static unsigned On_MONSTDEATH(TCmd *pCmd, int pnum)
{
	TCmdMonstKill *cmd = (TCmdMonstKill *)pCmd;

	if (pnum != mypnum && currLvl._dLevelIdx == cmd->mkLevel)
		MonSyncStartKill(SwapLE16(cmd->mkMnum), cmd->mkX, cmd->mkY, cmd->mkPnum);
	delta_kill_monster(cmd);

	return sizeof(*cmd);
}

static unsigned On_AWAKEGOLEM(TCmd *pCmd, int pnum)
{
	TCmdGolem *cmd = (TCmdGolem *)pCmd;

	delta_awake_golem(cmd, pnum);

	return sizeof(*cmd);
}

static unsigned On_MONSTDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdMonstDamage *cmd = (TCmdMonstDamage *)pCmd;

	delta_monster_hp(cmd);

	return sizeof(*cmd);
}

static unsigned On_PLRDEAD(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (pnum != mypnum)
		StartPlrKill(pnum, cmd->bParam1);
	else
		check_update_plr(pnum);

	return sizeof(*cmd);
}

static unsigned On_PLRRESURRECT(TCmd *pCmd, int pnum)
{
	SyncPlrResurrect(pnum);
	if (pnum == mypnum)
		check_update_plr(pnum);

	return sizeof(*pCmd);
}

static unsigned On_PLRDAMAGE(TCmd *pCmd, int pnum)
{
	TCmdPlrDamage *cmd = (TCmdPlrDamage *)pCmd;

#ifndef NOHOSTING
	if (cmd->pdPnum == mypnum && mypnum < MAX_PLRS) {
#else
	if (cmd->pdPnum == mypnum) {
#endif
		if (currLvl._dType != DTYPE_TOWN && currLvl._dLevelIdx == plr._pDunLevel) {
			if (!myplr._pInvincible /*&& SwapLE32(cmd->pdDamage) <= 192000*/) {
				PlrDecHp(mypnum, SwapLE32(cmd->pdDamage), DMGTYPE_PLAYER);
			}
		}
	}

	return sizeof(*cmd);
}

static unsigned On_DOOROPEN(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncDoorOpen(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_DOOROPEN, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_DOORCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncDoorClose(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_DOORCLOSE, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPDISABLE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapDisable(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPDISABLE, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPOPEN(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapOpen(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPOPEN, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapClose(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPCLOSE, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_OPERATEOBJ(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
		SyncOpObject(pnum, SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_OPERATEOBJ, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_CHESTCLOSE(TCmd *pCmd, int pnum)
{
	TCmdParam1 *cmd = (TCmdParam1 *)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncChestClose(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_CHESTCLOSE, plr._pDunLevel);

	return sizeof(*cmd);
}

/**
 * Sync Shrine effect with every player
 */
static unsigned On_SHRINE(TCmd *pCmd, int pnum)
{
	TCmdShrine* cmd = (TCmdShrine*)pCmd;

	SyncShrineCmd(pnum, cmd->shType, SwapLE32(cmd->shSeed));

	return sizeof(*cmd);
}

static unsigned On_SPLITPLRGOLD(TCmd* pCmd, int pnum)
{
	TCmdParam2* cmd = (TCmdParam2*)pCmd;

	SyncSplitGold(pnum, SwapLE16(cmd->wParam1), SwapLE16(cmd->wParam2));

	return sizeof(*cmd);
}

static unsigned On_PASTEPLRITEM(TCmd *pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	InvPasteItem(pnum, cmd->bParam1);

	return sizeof(*cmd);
}

static unsigned On_PASTEPLRBELTITEM(TCmd *pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	InvPasteBeltItem(pnum, cmd->bParam1);

	return sizeof(*cmd);
}

static unsigned On_CUTPLRITEM(TCmd *pCmd, int pnum)
{
	TCmdBParam2* cmd = (TCmdBParam2*)pCmd;

	InvCutItem(pnum, cmd->bParam1, cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_DELPLRITEM(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	SyncPlrItemRemove(pnum, cmd->bParam1);

	return sizeof(*cmd);
}

static unsigned On_USEPLRITEM(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	// if (pnum != mypnum)
		SyncUseItem(pnum, cmd->bParam1);

	return sizeof(*cmd);
}

static unsigned On_PLRLEVEL(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (pnum != mypnum && cmd->bParam1 <= MAXCHARLEVEL)
		plr._pLevel = cmd->bParam1;

	return sizeof(*cmd);
}

static unsigned On_PLRSKILLLVL(TCmd *pCmd, int pnum)
{
	TCmdBParam2 *cmd = (TCmdBParam2 *)pCmd;

	if (pnum != mypnum && cmd->bParam2 <= MAXSPLLEVEL)
		plr._pSkillLvl[cmd->bParam1] = cmd->bParam2;

	return sizeof(*cmd);
}

static unsigned On_SEND_GAME_DELTA(TCmd *pCmd, int pnum)
{
	if (pnum != mypnum && pnum < MAX_PLRS)
		guSendGameDelta |= 1 << pnum;

	return sizeof(*pCmd);
}

static unsigned On_SEND_PLRINFO(TCmd *pCmd, int pnum)
{
	TCmdPlrInfoHdr *cmd = (TCmdPlrInfoHdr *)pCmd;

	if (geBufferMsgs == MSG_GAME_DELTA || geBufferMsgs == MSG_REQUEST_GAME_DELTA)
		DeltaQueuePacket(pnum, cmd, cmd->wBytes + sizeof(*cmd));
	else if (pnum != mypnum)
		multi_recv_plrinfo_msg(pnum, cmd);

	return cmd->wBytes + sizeof(*cmd);
}

static unsigned On_ACK_PLRINFO(TCmd *pCmd, int pnum)
{
	return On_SEND_PLRINFO(pCmd, pnum);
}

static int16_t msg_calc_rage(WORD rage)
{
	int16_t result = SwapLE16(rage), delay;

	if (result != 0) {
		delay = gbNetUpdateRate;
		if (result < 0) {
			result += delay;
			if (result >= 0)
				result = 0;
		} else {
			result -= delay;
			if (result <= 0)
				result = -(RAGE_COOLDOWN_TICK + result);
		}
	}
	return result;
}

static unsigned On_ACK_JOINLEVEL(TCmd *pCmd, int pnum)
{
	TCmdAckJoinLevel *cmd = (TCmdAckJoinLevel *)pCmd;

	plr._pManaShield = cmd->lManashield;
	plr._pTimer[PLTR_INFRAVISION] = SwapLE16(cmd->lTimer1) > gbNetUpdateRate ? SwapLE16(cmd->lTimer1) - gbNetUpdateRate : 0;
	plr._pTimer[PLTR_RAGE] = msg_calc_rage(cmd->lTimer2);
	CalcPlrItemVals(pnum, false); // last parameter should not matter

	return sizeof(*cmd);
}

static unsigned On_SEND_JOINLEVEL(TCmd *pCmd, int pnum)
{
	TCmdSendJoinLevel *cmd = (TCmdSendJoinLevel *)pCmd;

	plr._pLvlChanging = FALSE;
	if (plr._pmode != PM_DEATH)
		plr._pInvincible = FALSE;
	plr._pDunLevel = cmd->lLevel;
	plr._px = cmd->px;
	plr._py = cmd->py;
	if (pnum == mypnum) {
		InitLvlPlayer(pnum);
	} else {
		if (!plr._pActive) {
			if (plr._pName[0] == '\0') {
				// plrinfo_msg did not arrive -> drop the player
				SNetDropPlayer(pnum);
				return sizeof(*cmd);
			}
			// TODO: validate data from internet
			//assert(plr._pTeam == pnum);
			//assert(plr._pManaShield == 0);
			//assert(cmd->lLevel == DLV_TOWN);
			//assert(cmd->lTimer1 == 0);
			//assert(cmd->lTimer2 == 0);
			plr._pActive = TRUE;
			gbActivePlayers++;
			EventPlrMsg("Player '%s' (level %d) just joined the game", plr._pName, plr._pLevel);
		}
		plr._pTimer[PLTR_INFRAVISION] = SwapLE16(cmd->lTimer1) > gbNetUpdateRate ? SwapLE16(cmd->lTimer1) - gbNetUpdateRate : 0;
		plr._pTimer[PLTR_RAGE] = msg_calc_rage(cmd->lTimer2);
		if (currLvl._dLevelIdx == plr._pDunLevel) {
			InitLvlPlayer(pnum);
			CalcPlrItemVals(pnum, false); // last parameter should not matter
			NetSendCmdAckJoinLevel();
		}
	}

	return sizeof(*cmd);
}

static unsigned On_ACTIVATEPORTAL(TCmd *pCmd, int pnum)
{
	TCmdLocBParam1 *cmd = (TCmdLocBParam1 *)pCmd;

	ActivatePortal(pnum, cmd->x, cmd->y, cmd->bParam1);
	if (pnum != mypnum) {
		if (currLvl._dLevelIdx == DLV_TOWN)
			AddInTownPortal(pnum);
		else if (currLvl._dLevelIdx == cmd->bParam1) {
			int i;
			for (i = 0; i < nummissiles; i++) {
				MissileStruct *mis = &missile[missileactive[i]];
				if (mis->_miType == MIS_TOWN && mis->_miSource == pnum) {
					break;
				}
			}
			if (i == nummissiles)
				AddWarpMissile(pnum, cmd->x, cmd->y);
		}
	}
	delta_open_portal(pnum, cmd->x, cmd->y, cmd->bParam1);

	return sizeof(*cmd);
}

static unsigned On_DEACTIVATEPORTAL(TCmd *pCmd, int pnum)
{
	if (PortalOnLevel(pnum))
		RemovePortalMissile(pnum);
	DeactivatePortal(pnum);
	delta_close_portal(pnum);

	return sizeof(*pCmd);
}

static unsigned On_RETOWN(TCmd *pCmd, int pnum)
{
	RestartTownLvl(pnum);

	return sizeof(*pCmd);
}

static unsigned On_STRING(TCmd *pCmd, int pnum)
{
	TCmdString *cmd = (TCmdString *)pCmd;

	//if (geBufferMsgs != MSG_GAME_DELTA && geBufferMsgs != MSG_REQUEST_GAME_DELTA) {
		if (pnum < MAX_PLRS) {
			if (!(guTeamMute & (1 << pnum))) {
				SendPlrMsg(pnum, cmd->str);
			}
		} else {
			EventPlrMsg(cmd->str);
		}
	//}

	return strlen(cmd->str) + 2; // length of string + nul terminator + sizeof(cmd->bCmd)
}

static unsigned On_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	 // TODO: check (cmd->bParam1 == mypnum) should not be necessary in a server/client solution
	if (cmd->bParam1 == mypnum && plr._pTeam == pnum) {
		guTeamInviteRec |= (1 << pnum);
		EventPlrMsg("%s invited to their team.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_ACK_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	guTeamInviteRec &= ~(1 << pnum);
	guTeamInviteSent &= ~(1 << pnum);

	plr._pTeam = cmd->bParam1;
#ifndef NOHOSTING
	if (mypnum < MAX_PLRS && cmd->bParam1 == myplr._pTeam) {
#else
	if (cmd->bParam1 == myplr._pTeam) {
#endif
		if (pnum == mypnum)
			EventPlrMsg("You joined team %c.", 'a' + plr._pTeam);
		else
			EventPlrMsg("%s joined your team.", plr._pName);
	} else {
		EventPlrMsg("%s joined team %c.", plr._pName, 'a' + plr._pTeam);
	}

	return sizeof(*cmd);
}

static unsigned On_DEC_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteSent &= ~(1 << pnum);

		EventPlrMsg("%s rejected your invitation.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_REV_INVITE(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;

	if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteRec &= ~(1 << pnum);

		EventPlrMsg("%s revoked the invitation.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_KICK_PLR(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1 *)pCmd;
	int teamplr, team;

	teamplr = cmd->bParam1;
	team = plx(teamplr)._pTeam;
	if (pnum != teamplr) {
		// drop
		if (team == pnum) {
			plx(teamplr)._pTeam = teamplr;

			if (teamplr == mypnum) {
				EventPlrMsg("You were kicked from your team.");
			} else {
				EventPlrMsg("%s was kicked from %s team.", plx(teamplr)._pName,
#ifndef NOHOSTING
					(mypnum < MAX_PLRS && team == myplr._pTeam) ? "your" : "their");
#else
					team == myplr._pTeam ? "your" : "their");
#endif
			}
		}
	} else {
		// leave
		if (team == teamplr) {
			multi_disband_team(teamplr);
		} else {
			plx(teamplr)._pTeam = teamplr;
		}

		if (teamplr == mypnum)
			EventPlrMsg("You left your team.");
		else
			EventPlrMsg("%s left %s team.", plx(teamplr)._pName,
#ifndef NOHOSTING
				(mypnum < MAX_PLRS && team == myplr._pTeam) ? "your" : "their");
#else
				team == myplr._pTeam ? "your" : "their");
#endif
	}

	return sizeof(*cmd);
}

/**
 * Execute store transactions I.
 */
static unsigned On_STORE_1(TCmd* pCmd, int pnum)
{
	TCmdStore1* cmd = (TCmdStore1*)pCmd;

	SyncStoreCmd(pnum, cmd->stCmd, cmd->stLoc, SwapLE32(cmd->stValue));

	return sizeof(*cmd);
}

/**
 * Execute store transactions II.
 */
static unsigned On_STORE_2(TCmd* pCmd, int pnum)
{
	TCmdStore2* cmd = (TCmdStore2*)pCmd;

	UnPackPkItem(&cmd->item);
	SyncStoreCmd(pnum, cmd->stCmd, MAXITEMS, SwapLE32(cmd->stValue));

	return sizeof(*cmd);
}

/* Sync item rewards in town. */
static unsigned On_QTOWNER(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	SyncTownerQ(pnum, SwapLE16(cmd->wParam1));

	return sizeof(*cmd);
}

/* Sync item rewards in dungeon. */
static unsigned On_QMONSTER(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	SyncMonsterQ(pnum, SwapLE16(cmd->wParam1));

	return sizeof(*cmd);
}

/**
 * Sync Quest with every player.
 */
static unsigned On_SYNCQUEST(TCmd *pCmd, int pnum)
{
	TCmdQuest *cmd = (TCmdQuest *)pCmd;

	if (pnum != mypnum)
		SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
	_gbJunkDeltaChanged = true;

	return sizeof(*cmd);
}

/**
 * Same as On_SYNCQUEST, but sends the message only to players on other levels.
 */
static unsigned On_SYNCQUESTEXT(TCmd *pCmd, int pnum)
{
	TCmdQuest *cmd = (TCmdQuest *)pCmd;

	if (currLvl._dLevelIdx != plr._pDunLevel || geBufferMsgs == MSG_INITIAL_PENDINGTURN)
		SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
	_gbJunkDeltaChanged = true;

	return sizeof(*cmd);
}

#ifdef _DEBUG
static unsigned On_CHEAT_EXPERIENCE(TCmd *pCmd, int pnum)
{
	if (plr._pLevel < MAXCHARLEVEL) {
		plr._pExperience = plr._pNextExper;
		NextPlrLevel(pnum);
	}
	return sizeof(*pCmd);
}

static unsigned On_CHEAT_SPELL_LEVEL(TCmd *pCmd, int pnum)
{
	plr._pSkillLvl[plr._pAltAtkSkill]++;
	return sizeof(*pCmd);
}

static unsigned On_DEBUG(TCmd *pCmd, int pnum)
{
	return sizeof(*pCmd);
}
#endif

static unsigned On_SETSHIELD(TCmd *pCmd, int pnum)
{
	TCmdBParam1 *cmd = (TCmdBParam1*)pCmd;

	plr._pManaShield = cmd->bParam1;

	return sizeof(*cmd);
}

static unsigned On_REMSHIELD(TCmd *pCmd, int pnum)
{
	plr._pManaShield = 0;

	return sizeof(*pCmd);
}

static unsigned On_BLOODPASS(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	if (SyncBloodPass(pnum, SwapLE16(cmd->wParam1)))
		delta_sync_object(SwapLE16(cmd->wParam1), CMD_OPERATEOBJ, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_OPENSPIL(TCmd *pCmd, int pnum)
{
	quests[Q_LTBANNER]._qactive = QUEST_DONE;
	quests[Q_LTBANNER]._qvar1 = 4;

	if (currLvl._dLevelIdx == questlist[Q_LTBANNER]._qdlvl) {
		ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
		BYTE tv = dTransVal[2 * setpc_x + 1 + DBORDERX][2 * (setpc_y + 6) + 1 + DBORDERY];
		DRLG_MRectTrans(setpc_x, setpc_y + 3, setpc_x + setpc_w - 1, setpc_y + setpc_h - 1, tv);
		RedoPlayerVision();
	}
	return sizeof(*pCmd);
}

unsigned ParseMsg(int pnum, TCmd *pCmd)
{
#ifndef NOHOSTING
	if ((unsigned)pnum >= MAX_PLRS && pnum != SNPLAYER_MASTER) {
#else
	if ((unsigned)pnum >= MAX_PLRS) {
#endif
		dev_fatal("ParseMsg: illegal player %d", pnum);
	}
	switch (pCmd->bCmd) {
	case NMSG_SEND_GAME_DELTA:
		return On_SEND_GAME_DELTA(pCmd, pnum);
	case NMSG_ACK_PLRINFO:
		return On_ACK_PLRINFO(pCmd, pnum);
	case NMSG_SEND_PLRINFO:
		return On_SEND_PLRINFO(pCmd, pnum);
	case NMSG_DLEVEL_DATA:
	case NMSG_DLEVEL_SEP:
	case NMSG_DLEVEL_JUNK:
	case NMSG_DLEVEL_PLR:
	case NMSG_DLEVEL_END:
		return On_DLEVEL(pCmd, pnum);
	case NMSG_STRING:
		return On_STRING(pCmd, pnum);
	}

	SNetDropPlayer(pnum);
	return 0;
}

unsigned ParseCmd(int pnum, TCmd *pCmd)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("ParseCmd: illegal player %d", pnum);
	}
	assert(geBufferMsgs != MSG_GAME_DELTA && geBufferMsgs != MSG_REQUEST_GAME_DELTA);
	switch (pCmd->bCmd) {
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
	case CMD_SPLITPLRGOLD:
		return On_SPLITPLRGOLD(pCmd, pnum);
	case CMD_PASTEPLRITEM:
		return On_PASTEPLRITEM(pCmd, pnum);
	case CMD_PASTEPLRBELTITEM:
		return On_PASTEPLRBELTITEM(pCmd, pnum);
	case CMD_CUTPLRITEM:
		return On_CUTPLRITEM(pCmd, pnum);
	case CMD_DELPLRITEM:
		return On_DELPLRITEM(pCmd, pnum);
	case CMD_USEPLRITEM:
		return On_USEPLRITEM(pCmd, pnum);
	case CMD_DPUTITEM:
		return On_DPUTITEM(pCmd, pnum);
	case CMD_PUTITEM:
		return On_PUTITEM(pCmd, pnum);
	case CMD_RESPAWNITEM:
		return On_RESPAWNITEM(pCmd, pnum);
	case CMD_GETITEM:
		return On_GETITEM(pCmd, pnum);
	case CMD_AUTOGETITEM:
		return On_AUTOGETITEM(pCmd, pnum);
	case CMD_GOTOGETITEM:
		return On_GOTOGETITEM(pCmd, pnum);
	case CMD_GOTOAGETITEM:
		return On_GOTOAGETITEM(pCmd, pnum);
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
	case CMD_SHRINE:
		return On_SHRINE(pCmd, pnum);
	case CMD_ACTIVATEPORTAL:
		return On_ACTIVATEPORTAL(pCmd, pnum);
	case CMD_DEACTIVATEPORTAL:
		return On_DEACTIVATEPORTAL(pCmd, pnum);
	case CMD_NEWLVL:
		return On_NEWLVL(pCmd, pnum);
	case CMD_TWARP:
		return On_TWARP(pCmd, pnum);
	case CMD_RETOWN:
		return On_RETOWN(pCmd, pnum);
	case CMD_ACK_JOINLEVEL:
		return On_ACK_JOINLEVEL(pCmd, pnum);
	case CMD_SEND_JOINLEVEL:
		return On_SEND_JOINLEVEL(pCmd, pnum);
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
	case CMD_STORE_1:
		return On_STORE_1(pCmd, pnum);
	case CMD_STORE_2:
		return On_STORE_2(pCmd, pnum);
	case CMD_QTOWNER:
		return On_QTOWNER(pCmd, pnum);
	case CMD_QMONSTER:
		return On_QMONSTER(pCmd, pnum);
	case CMD_SYNCQUEST:
		return On_SYNCQUEST(pCmd, pnum);
	case CMD_SYNCQUESTEXT:
		return On_SYNCQUESTEXT(pCmd, pnum);
	case CMD_BLOODPASS:
		return On_BLOODPASS(pCmd, pnum);
	case CMD_OPENSPIL:
		return On_OPENSPIL(pCmd, pnum);
#ifdef _DEBUG
	case CMD_CHEAT_EXPERIENCE:
		return On_CHEAT_EXPERIENCE(pCmd, pnum);
	case CMD_CHEAT_SPELL_LEVEL:
		return On_CHEAT_SPELL_LEVEL(pCmd, pnum);
	case CMD_DEBUG:
		return On_DEBUG(pCmd, pnum);
#endif	
	}

	SNetDropPlayer(pnum);
	return 0;
}

DEVILUTION_END_NAMESPACE
