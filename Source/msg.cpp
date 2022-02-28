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
static TMegaPkt* sgpMegaPkt;
/* The tail of the sgpMegaPkt linked list. */
static TMegaPkt* sgpCurrPkt;
/* Flag to tell if the delta info is currently processed. */
bool deltaload;
/* Container to keep the delta info (multi) and save file data (single). */
DeltaData gsDeltaData;
/* Counter to keep the progress of delta-load
 * 0..NUM_LEVELS     : level data
 * NUM_LEVELS + 1    : junk data (portals/quests)
 * NUM_LEVELS + 2 .. : players data
 * MAX_CHUNKS - 1    : end of data
 * MAX_CHUNKS        : download success
 */
static BYTE gbGameDeltaChunks;
/* the current messaging mode. (MSG_*) */
_msg_mode geBufferMsgs = MSG_NORMAL;
/* Buffer holding the character message to send over to other players */
char gbNetMsg[MAX_SEND_STR_LEN];

static void DeltaAllocMegaPkt()
{
	TMegaPkt* result;

	sgpCurrPkt = (TMegaPkt*)DiabloAllocPtr(sizeof(TMegaPkt));
	sgpCurrPkt->pNext = NULL;
	sgpCurrPkt->dwSpaceLeft = sizeof(result->data);

	result = (TMegaPkt*)&sgpMegaPkt;
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
	TMegaPkt* pkt;

	for (pkt = sgpMegaPkt; pkt != NULL; pkt = pkt->pNext) {
		data = pkt->data;
		dataEnd = data + sizeof(pkt->data) - pkt->dwSpaceLeft;
		while (data != dataEnd) {
			pnum = *data;
			data++;
			pktSize = ParseMsg(pnum, (TCmd*)data);
			// only known drop/send_plrinfo msgs are in the queue
			assert(pktSize != 0);
			data += pktSize;
		}
	}
}

static void DeltaQueuePacket(int pnum, const void* packet, unsigned dwSize)
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

void msg_send_drop_plr(int pnum, BYTE reason)
{
	TFakeDropPlr cmd;

	cmd.bCmd = NMSG_PLRDROP;
	cmd.bReason = reason;
	// FIXME: timestamp?

	DeltaQueuePacket(pnum, &cmd, sizeof(cmd));
}

static void msg_mask_monhit(int pnum)
{
	int i;
	BYTE mask;

	static_assert(MAX_PLRS < 8, "msg_mask_monhit uses BYTE mask for pnum.");
	mask = ~(1 << pnum);
	for (i = 0; i < MAXMONSTERS; i++)
		gsDeltaData.ddLevel->monster[i]._mWhoHit &= mask;
	for (i = 0; i < MAXMONSTERS; i++)
		monsters[i]._mWhoHit &= mask;
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
	gsDeltaData.ddRecvLastCmd = NMSG_DLEVEL_END;
	gsDeltaData.ddDeltaSender = SNPLAYER_ALL;
	assert(gsDeltaData.ddSendRecvOffset == 0);
	// trigger delta-download in nthread
	geBufferMsgs = MSG_GAME_DELTA_WAIT;
	//guDeltaStart = SDL_GetTicks();
	success = UiProgressDialog("Waiting for game data...", msg_wait_for_delta);
	assert(geBufferMsgs == MSG_NORMAL || !success || gbGameDeltaChunks != MAX_CHUNKS);
	geBufferMsgs = MSG_NORMAL;
	if (success) {
		if (gbGameDeltaChunks == MAX_CHUNKS) {
			msg_mask_monhit(mypnum);
			gdwLastGameTurn = guDeltaTurn - 1;
			gdwGameLogicTurn = guDeltaTurn * gbNetUpdateRate;
			// process messages queued during delta-load
			//geBufferMsgs = MSG_RUN_DELTA;
			DeltaProcessMegaPkts();
			//geBufferMsgs = MSG_NORMAL;
		} else {
			success = false;
#if DEBUG_MODE || DEV_MODE
			app_warn(/*gbGameDeltaChunks == DELTA_ERROR_DISCONNECT ? "The game ended %d" :*/"Unable to get game data %d", gbGameDeltaChunks);
#else
			app_warn(/*gbGameDeltaChunks == DELTA_ERROR_DISCONNECT ? "The game ended" :*/"Unable to get game data");
#endif
		}
	}
	DeltaFreeMegaPkts();
	return success;
}

static BYTE* DeltaExportLevel(BYTE bLevel)
{
	DItemStr* item;
	DMonsterStr* mon;
	int i;
	BYTE* dst = gsDeltaData.ddSendRecvBuf.content;

	static_assert(sizeof(gsDeltaData.ddSendRecvBuf.content) >= sizeof(DLevel) + 1, "Delta-Level might not fit to the buffer.");

	// level-index
	*dst = bLevel;
	dst++;

	// export items
	item = gsDeltaData.ddLevel[bLevel].item;
	for (i = 0; i < lengthof(gsDeltaData.ddLevel[bLevel].item); i++, item++) {
		if (item->bCmd == DCMD_INVALID) {
			*dst = DCMD_INVALID;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DItemStr*>(dst), *item);
			dst += sizeof(DItemStr);
		}
	}

	// export objects
	memcpy(dst, gsDeltaData.ddLevel[bLevel].object, sizeof(gsDeltaData.ddLevel[bLevel].object));
	dst += sizeof(gsDeltaData.ddLevel[bLevel].object);

	// export monsters
	mon = gsDeltaData.ddLevel[bLevel].monster;
	for (i = 0; i < lengthof(gsDeltaData.ddLevel[bLevel].monster); i++, mon++) {
		if (mon->_mCmd == DCMD_MON_INVALID) {
			*dst = DCMD_MON_INVALID;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DMonsterStr*>(dst), *mon);
			dst += sizeof(DMonsterStr);
		}
	}

	return dst;
}

static void DeltaImportLevel()
{
	DItemStr* item;
	DMonsterStr* mon;
	int i;
	BYTE *src, bLvl;

	static_assert(sizeof(gsDeltaData.ddSendRecvBuf.content) >= (sizeof(DLevel) + 1), "DLevel does not fit to the buffer.");

	src = gsDeltaData.ddSendRecvBuf.content;
	// level-index
	bLvl = *src;
	src++;

	gsDeltaData.ddLevelChanged[bLvl] = true;

	// import items
	item = gsDeltaData.ddLevel[bLvl].item;
	for (i = 0; i < MAXITEMS; i++, item++) {
		if (*src == DCMD_INVALID) {
			static_assert((int)DCMD_INVALID == 0, "DeltaImportLevel initializes the items with zero, assuming the invalid command to be zero.");
			memset(item, 0, sizeof(DItemStr));
			src++;
		} else {
			copy_pod(*item, *reinterpret_cast<DItemStr*>(src));
			// TODO: validate data from internet
			// assert(dst->bCmd == DCMD_SPAWNED || dst->bCmd == DCMD_TAKEN || dst->bCmd == DCMD_DROPPED);
			src += sizeof(DItemStr);
		}
	}
	// import objects
	memcpy(gsDeltaData.ddLevel[bLvl].object, src, sizeof(gsDeltaData.ddLevel[bLvl].object));
	src += sizeof(gsDeltaData.ddLevel[bLvl].object);

	// import monsters
	mon = gsDeltaData.ddLevel[bLvl].monster;
	for (i = 0; i < MAXMONSTERS; i++, mon++) {
		if (*src == DCMD_MON_INVALID) {
			static_assert((int)DCMD_MON_INVALID == 0, "DeltaImportLevel initializes the monsters with zero, assuming the invalid command to be zero.");
			memset(mon, 0, sizeof(DMonsterStr));
			src++;
		} else {
			copy_pod(*mon, *reinterpret_cast<DMonsterStr*>(src));
			src += sizeof(DMonsterStr);
		}
	}
}

static BYTE* DeltaExportJunk()
{
	DQuest* mq;
	int i;
	BYTE* dst = gsDeltaData.ddSendRecvBuf.content;

	// TODO: add delta_SetMultiQuest instead?
	mq = gsDeltaData.ddJunk.jQuests;
	for (i = 0; i < NUM_QUESTS; i++) {
		mq->qstate = quests[i]._qactive;
		mq->qlog = quests[i]._qlog;
		mq->qvar1 = quests[i]._qvar1;
		mq++;
	}

	static_assert(sizeof(gsDeltaData.ddSendRecvBuf.content) >= sizeof(gsDeltaData.ddJunk), "Delta-Junk does not fit to the buffer.");
	// export portals + quests + golems
	memcpy(dst, &gsDeltaData.ddJunk, sizeof(gsDeltaData.ddJunk));
	dst += sizeof(gsDeltaData.ddJunk);

	return dst;
}

static void DeltaImportJunk()
{
	DPortal* pD;
	DQuest* mq;
	int i;
	BYTE* src = gsDeltaData.ddSendRecvBuf.content;

	gsDeltaData.ddJunkChanged = true;

	// import portals + quests + golems
	memcpy(&gsDeltaData.ddJunk, src, sizeof(gsDeltaData.ddJunk));
	//src += sizeof(gsDeltaData.ddJunk);

	// update the game state
	// portals
	pD = gsDeltaData.ddJunk.jPortals;
	for (i = 0; i < MAXPORTAL; i++, pD++) {
		if (pD->level != DLV_TOWN) {
			ActivatePortal(i, pD->x, pD->y, pD->level);
		}
		//else
		//	SetPortalStats(i, false, 0, 0, 0);
	}
	// quests
	mq = gsDeltaData.ddJunk.jQuests;
	for (i = 0; i < NUM_QUESTS; i++, mq++) {
		quests[i]._qlog = mq->qlog;
		quests[i]._qactive = mq->qstate;
		quests[i]._qvar1 = mq->qvar1;
	}
}

static BYTE* DeltaExportPlr(int pnum)
{
	BYTE* dst = gsDeltaData.ddSendRecvBuf.content;

	static_assert(sizeof(gsDeltaData.ddSendRecvBuf.content) >= (sizeof(PkPlayerStruct) + 1), "Delta-Plr does not fit to the buffer.");

	// player-index
	*dst = pnum;
	dst++;

	PackPlayer((PkPlayerStruct*)dst, pnum);

	return dst + sizeof(PkPlayerStruct);
}

static void DeltaImportPlr()
{
	int pnum;
	BYTE* src = gsDeltaData.ddSendRecvBuf.content;

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

static DWORD DeltaCompressData(BYTE* end)
{
	DWORD size, pkSize;

	size = end - gsDeltaData.ddSendRecvBuf.content;
	pkSize = PkwareCompress(gsDeltaData.ddSendRecvBuf.content, size);
	gsDeltaData.ddSendRecvBuf.compressed = size != pkSize;

	return pkSize + sizeof(gsDeltaData.ddSendRecvBuf.compressed);
}

void DeltaExportData(int pnum, uint32_t turn)
{
	BYTE* dstEnd;
	int size, i;
	BYTE src, numChunks = 0;

	// levels
	for (i = 0; i < lengthof(gsDeltaData.ddLevel); i++) {
		if (!gsDeltaData.ddLevelChanged[i])
			continue;
		dstEnd = DeltaExportLevel(i);
		size = DeltaCompressData(dstEnd);
		dthread_send_delta(pnum, NMSG_DLEVEL_DATA, &gsDeltaData.ddSendRecvBuf, size);
		src = 0;
		dthread_send_delta(pnum, NMSG_DLEVEL_SEP, &src, 1);
		numChunks++;
	}
	// junk
	if (gsDeltaData.ddJunkChanged) {
		dstEnd = DeltaExportJunk();
		size = DeltaCompressData(dstEnd);
		dthread_send_delta(pnum, NMSG_DLEVEL_JUNK, &gsDeltaData.ddSendRecvBuf, size);
		numChunks++;
	}
	// players
	for (i = 0; i < MAX_PLRS; i++) {
		if (plx(i)._pActive) {
			dstEnd = DeltaExportPlr(i);
			size = DeltaCompressData(dstEnd);
			dthread_send_delta(pnum, NMSG_DLEVEL_PLR, &gsDeltaData.ddSendRecvBuf, size);
			src = 0;
			dthread_send_delta(pnum, NMSG_DLEVEL_SEP, &src, 1);
			numChunks++;
		}
	}

	// current number of chunks sent + turn-id + end
	DeltaDataEnd deltaEnd;
	deltaEnd.compressed = FALSE;
	deltaEnd.numChunks = numChunks;
	deltaEnd.turn = SDL_SwapLE32(gdwLastGameTurn + 1);
	assert(turn == gdwLastGameTurn + 1);
	assert(turn * gbNetUpdateRate == gdwGameLogicTurn);
	dthread_send_delta(pnum, NMSG_DLEVEL_END, &deltaEnd, sizeof(deltaEnd));
}

static void DeltaImportData()
{
	if (gsDeltaData.ddSendRecvBuf.compressed)
		PkwareDecompress(gsDeltaData.ddSendRecvBuf.content, gsDeltaData.ddSendRecvOffset, sizeof(gsDeltaData.ddSendRecvBuf.content));

	gbGameDeltaChunks++;

	if (gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_DATA) {
		DeltaImportLevel();
	} else if (gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_JUNK) {
		DeltaImportJunk();
	} else {
		assert(gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_PLR);
		DeltaImportPlr();
	}
}

static void DeltaImportEnd(TCmdPlrInfoHdr* cmd)
{
	DeltaDataEnd* buf;

	// stop nthread from processing the delta messages
	geBufferMsgs = MSG_NORMAL;

	net_assert(SwapLE16(cmd->wBytes) == sizeof(DeltaDataEnd));
	buf = (DeltaDataEnd*)&cmd[1];
	net_assert(!buf->compressed);
	if (gbGameDeltaChunks != buf->numChunks) {
		// not all chunks arrived -> quit
		gbGameDeltaChunks = DELTA_ERROR_FAIL_3;
		return;
	}
	guDeltaTurn = SDL_SwapLE32(buf->turn);
	gbGameDeltaChunks = MAX_CHUNKS - 1;
}

static unsigned On_DLEVEL(TCmd* pCmd, int pnum)
{
	TCmdPlrInfoHdr* cmd = (TCmdPlrInfoHdr*)pCmd;

	if (geBufferMsgs != MSG_GAME_DELTA_LOAD)
		goto done; // the player is already active -> drop the packet

	if (gsDeltaData.ddDeltaSender != pnum) {
		if (gsDeltaData.ddDeltaSender != SNPLAYER_ALL) {
			// delta is already on its way from a different player -> drop the packet
			goto done;
		}
		if (gsDeltaData.ddSendRecvOffset != 0) {
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
			gsDeltaData.ddDeltaSender = pnum;
			// gsDeltaData.ddRecvLastCmd = NMSG_DLEVEL_END;
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
		gsDeltaData.ddDeltaSender = pnum;
		gsDeltaData.ddRecvLastCmd = cmd->bCmd;
		// gsDeltaData.ddSendRecvOffset = 0;
	} else {
		// a packet from a previous sender
		if (gsDeltaData.ddRecvLastCmd != cmd->bCmd) {
			// process previous package
			if (gsDeltaData.ddRecvLastCmd != NMSG_DLEVEL_SEP && gsDeltaData.ddRecvLastCmd != NMSG_DLEVEL_END)
				DeltaImportData();
			gsDeltaData.ddRecvLastCmd = cmd->bCmd;
			if (gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_END) {
				// final package -> done
				DeltaImportEnd(cmd);
				goto done;
			} else if (gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_SEP) {
				// separator package -> wait for next
				goto done;
			} else {
				// start receiving a new package
				assert(gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_DATA || gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_JUNK || gsDeltaData.ddRecvLastCmd == NMSG_DLEVEL_PLR);
				gsDeltaData.ddSendRecvOffset = 0;
			}
		} else {
			// continue previous package
			if (gsDeltaData.ddRecvLastCmd != NMSG_DLEVEL_DATA && gsDeltaData.ddRecvLastCmd != NMSG_DLEVEL_JUNK && gsDeltaData.ddRecvLastCmd != NMSG_DLEVEL_PLR) {
				// lost or duplicated package -> drop the connection and quit
				gbGameDeltaChunks = DELTA_ERROR_FAIL_1;
				goto done;
			}
		}
	}

	if (cmd->wOffset != gsDeltaData.ddSendRecvOffset) {
		// lost or duplicated package -> drop the connection and quit
		gbGameDeltaChunks = DELTA_ERROR_FAIL_2;
		goto done;
	}

	memcpy(((BYTE*)&gsDeltaData.ddSendRecvBuf) + cmd->wOffset, &cmd[1], cmd->wBytes);
	gsDeltaData.ddSendRecvOffset += cmd->wBytes;
done:
	return cmd->wBytes + sizeof(*cmd);
}

void delta_init()
{
	//gsDeltaData.ddJunkChanged = false;
	//memset(gsDeltaData.ddLevelChanged, 0, sizeof(gsDeltaData.ddLevelChanged));
	static_assert((int)DLV_TOWN == 0, "delta_init initializes the portal levels to zero, assuming none of the portals starts from the town.");
	//memset(&gsDeltaData.ddJunk, 0, sizeof(gsDeltaData.ddJunk));
	static_assert((int)DCMD_INVALID == 0, "delta_init initializes the items with zero, assuming the invalid command to be zero.");
	static_assert((int)DCMD_MON_INVALID == 0, "delta_init initializes the monsters with zero, assuming the invalid command to be zero.");
	static_assert((int)CMD_SYNCDATA == 0, "delta_init initializes the objects with zero, assuming none of the valid commands for an object to be zero.");
	//memset(gsDeltaData.ddLevel, 0, sizeof(gsDeltaData.ddLevel));
	//memset(gsDeltaData.ddLocal, 0, sizeof(gsDeltaData.ddLocal));
	//gsDeltaData.ddSendRecvOffset = 0;
	memset(&gsDeltaData, 0, sizeof(gsDeltaData));
	assert(!deltaload);
}

static void delta_monster_corpse(const TCmdLocBParam1* pCmd)
{
	BYTE bLevel;
	DMonsterStr* mon;
	int i;

	if (!IsMultiGame)
		return;

	bLevel = pCmd->bParam1;
	net_assert(bLevel < NUM_LEVELS);
	// commented out, because _mCmd must be already set at this point
	//gsDeltaData.ddLevelChanged[bLevel] = true;
	mon = gsDeltaData.ddLevel[bLevel].monster;
	for (i = 0; i < lengthof(gsDeltaData.ddLevel[bLevel].monster); i++, mon++) {
		if (mon->_mCmd == DCMD_MON_DEAD
		 && mon->_mx == pCmd->x && mon->_my == pCmd->y) {
			mon->_mCmd = DCMD_MON_DESTROYED;
		}
	}
}

static BYTE delta_kill_monster(const TCmdMonstKill* mon)
{
	DMonsterStr* pD;
	int mnum;
	BYTE bLevel, whoHit = 0;

	if (mon->mkPnum < MAX_PLRS)
		whoHit |= 1 << mon->mkPnum;

	mnum = SwapLE16(mon->mkMnum);
	if (!IsMultiGame) {
		return whoHit | monsters[mnum]._mWhoHit;
	}

	bLevel = mon->mkParam1.bParam1;
	net_assert(bLevel < NUM_LEVELS);

	gsDeltaData.ddLevelChanged[bLevel] = true;
	pD = &gsDeltaData.ddLevel[bLevel].monster[mnum];
	static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "delta_kill_monster expects ordered DCMD_MON_ enum I.");
	static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "delta_kill_monster expects ordered DCMD_MON_ enum II.");
	if (pD->_mCmd >= DCMD_MON_DEAD)
		return 0;
	delta_monster_corpse(&mon->mkParam1);
	pD->_mCmd = (mon->mkMode != MM_STONE && mnum >= MAX_MINIONS/*mon->_mType != MT_GOLEM*/) ? DCMD_MON_DEAD : DCMD_MON_DESTROYED;
	pD->_mx = mon->mkParam1.x;
	pD->_my = mon->mkParam1.y;
	pD->_mdir = mon->mkDir;
	pD->_mleaderflag = MLEADER_NONE;
	pD->_mhitpoints = 0;
	return whoHit | pD->_mWhoHit;
}

static void delta_monster_hp(const TCmdMonstDamage* mon, int pnum)
{
	DMonsterStr* pD;
	BYTE bLevel;

	if (!IsMultiGame)
		return;

	bLevel = mon->mdLevel;
	net_assert(bLevel < NUM_LEVELS);
	// commented out, because these changes are ineffective unless _mCmd is already set
	//gsDeltaData.ddLevelChanged[bLevel] = true;
	pD = &gsDeltaData.ddLevel[bLevel].monster[SwapLE16(mon->mdMnum)];
	static_assert(MAX_PLRS < 8, "delta_monster_hp uses BYTE mask for pnum.");
	pD->_mWhoHit |= 1 << pnum;
	// In vanilla code the value was discarded if hp was higher than the current one.
	// That disregards the healing monsters.
	// Now it is always updated except the monster is already dead.
	//if (pD->_mCmd != DCMD_MON_DEAD && pD->_mCmd != DCMD_MON_DESTROYED)
		pD->_mhitpoints = mon->mdHitpoints;
}

static void delta_sync_monster(const TSyncHeader* pHdr)
{
	DMonsterStr* pDLvlMons;
	DMonsterStr* pD;
	WORD wLen;
	const TSyncMonster* pSync;
	const BYTE* pbBuf;

	assert(IsMultiGame);

	net_assert(pHdr->bLevel < NUM_LEVELS);

	gsDeltaData.ddLevelChanged[pHdr->bLevel] = true;
	pDLvlMons = gsDeltaData.ddLevel[pHdr->bLevel].monster;

	pbBuf = (const BYTE*)&pHdr[1];
	for (wLen = SwapLE16(pHdr->wLen); wLen >= sizeof(TSyncMonster); wLen -= sizeof(TSyncMonster)) {
		pSync = (TSyncMonster*)pbBuf;
		pD = &pDLvlMons[pSync->_mndx];
		static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "delta_sync_monster expects ordered DCMD_MON_ enum I.");
		static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "delta_sync_monster expects ordered DCMD_MON_ enum II.");
		if (pD->_mCmd < DCMD_MON_DEAD) {
			pD->_mCmd = DCMD_MON_ACTIVE;
			pD->_mx = pSync->_mx;
			pD->_my = pSync->_my;
			pD->_mdir = pSync->_mdir;
			pD->_mleaderflag = pSync->_mleaderflag;
			pD->_mactive = pSync->_mactive;
			pD->_mhitpoints = pSync->_mhitpoints;
		}
		pbBuf += sizeof(TSyncMonster);
	}
	assert(wLen == 0);
}

static void delta_awake_golem(TCmdGolem* pG, int mnum)
{
	DMonsterStr* pD;
	BYTE bLevel;

	if (!IsMultiGame)
		return;

	gsDeltaData.ddJunkChanged = true;
	gsDeltaData.ddJunk.jGolems[mnum] = pG->goMonLevel;

	InitGolemStats(mnum, pG->goMonLevel);

	bLevel = pG->goDunLevel;
	net_assert(bLevel < NUM_LEVELS);

	gsDeltaData.ddLevelChanged[bLevel] = true;
	pD = &gsDeltaData.ddLevel[bLevel].monster[mnum];
	pD->_mCmd = DCMD_MON_ACTIVE;
	pD->_mx = pG->goX;
	pD->_my = pG->goY;
	pD->_mactive = SQUELCH_MAX;
	// pD->_mdir = DIR_S; -- should not matter
	static_assert(MLEADER_NONE == 0, "delta_awake_golem expects _mleaderflag to be set by zerofill.");
	// pD->_mleaderflag = MLEADER_NONE;
	pD->_mhitpoints = monsters[mnum]._mmaxhp;
}

static void delta_leave_sync(BYTE bLevel)
{
	if (bLevel == DLV_TOWN) {
		glSeedTbl[DLV_TOWN] = GetRndSeed();
		return;
	}
	memcpy(&gsDeltaData.ddLocal[bLevel].automapsv, automapview, sizeof(automapview));
}

static void delta_sync_object(int oi, BYTE bCmd, BYTE bLevel)
{
	if (!IsMultiGame)
		return;
	net_assert(bLevel < NUM_LEVELS);
	gsDeltaData.ddLevelChanged[bLevel] = true;
	gsDeltaData.ddLevel[bLevel].object[oi].bCmd = bCmd;
}

static bool delta_get_item(const TCmdGItem* pI)
{
	DItemStr* pD;
	int i;
	BYTE bLevel;

	if (!IsMultiGame)
		return FindGetItem(SwapLE32(pI->item.dwSeed), SwapLE16(pI->item.wIndx), SwapLE16(pI->item.wCI)) != -1;

	bLevel = pI->bLevel;
	net_assert(bLevel < NUM_LEVELS);

	pD = gsDeltaData.ddLevel[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == DCMD_INVALID || pD->item.dwSeed != pI->item.dwSeed || pD->item.wIndx != pI->item.wIndx || pD->item.wCI != pI->item.wCI)
			continue;

		switch (pD->bCmd) {
		case DCMD_ITM_TAKEN:
			return false;
		case DCMD_ITM_SPAWNED:
			gsDeltaData.ddLevelChanged[bLevel] = true;
			pD->bCmd = DCMD_ITM_TAKEN;
			return true;
		case DCMD_ITM_MOVED:
			pD->bCmd = DCMD_ITM_TAKEN;
			return true;
		case DCMD_ITM_DROPPED:
			pD->bCmd = DCMD_INVALID;
			return true;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}

	if (pI->fromFloor) {
		net_assert(!IsLvlVisited(bLevel));
		// level was not visited by the current player
		// assume it is floor item spawned on the ground
		gsDeltaData.ddLevelChanged[bLevel] = true;
		pD = gsDeltaData.ddLevel[bLevel].item;
		for (i = 0; i < MAXITEMS; i++, pD++) {
			if (pD->bCmd == DCMD_INVALID) {
				pD->bCmd = DCMD_ITM_TAKEN;
				pD->x = pI->x;
				pD->y = pI->y;
				copy_pod(pD->item, pI->item);
				return true;
			}
		}
	}

	return false;
}

static bool delta_put_item(const PkItemStruct* pItem, BYTE bLevel, int x, int y)
{
	int i;
	DItemStr* pD;

	if (!IsMultiGame)
		return true;

	net_assert(bLevel < NUM_LEVELS);
	// set out of loop to reduce the number of locals
	// this might not change the level if there were MAXITEMS number of floor-items
	gsDeltaData.ddLevelChanged[bLevel] = true;
	pD = gsDeltaData.ddLevel[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != DCMD_INVALID
		 && pD->item.dwSeed == pItem->dwSeed
		 && pD->item.wIndx == pItem->wIndx
		 && pD->item.wCI == pItem->wCI) {
			if (pD->bCmd == DCMD_ITM_TAKEN) {
				pD->bCmd = DCMD_ITM_MOVED;
				pD->x = x;
				pD->y = y;
			}
			//else
			//	app_fatal("Trying to drop a floor item?");
			return true;
		}
	}

	pD = gsDeltaData.ddLevel[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == DCMD_INVALID) {
			pD->bCmd = DCMD_ITM_DROPPED;
			pD->x = x;
			pD->y = y;
			copy_pod(pD->item, *pItem);
			return true;
		}
	}

	return false;
}

void PackPkItem(PkItemStruct* dest, const ItemStruct* src)
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
	ItemStruct* is;
	int i;
	DItemStr* pD;

	if (!IsMultiGame)
		return;

	is = &items[ii];
	is->_iFloorFlag = TRUE;
	pD = gsDeltaData.ddLevel[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != DCMD_INVALID
		 && pD->item.dwSeed == SwapLE32(is->_iSeed)
		 && pD->item.wIndx == SwapLE16(is->_iIdx)
		 && pD->item.wCI == SwapLE16(is->_iCreateInfo)) {
			return;
		}
	}

	pD = gsDeltaData.ddLevel[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == DCMD_INVALID) {
			pD->bCmd = DCMD_ITM_SPAWNED;
			pD->x = is->_ix;
			pD->y = is->_iy;
			PackPkItem(&pD->item, is);
			break;
		}
	}
}

void DeltaSaveLevel()
{
	assert(IsMultiGame);
	delta_leave_sync(currLvl._dLevelIdx);
}

static void UnPackEar(const PkItemStruct* src)
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
	items[MAXITEMS]._iCreateInfo = SwapLE16(*(WORD*)&items[MAXITEMS]._iName[7]);
	items[MAXITEMS]._iSeed = SwapLE32(*(DWORD*)&items[MAXITEMS]._iName[9]);
}

void UnPackPkItem(const PkItemStruct* src)
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

static void UpdateLeader(int mnum, BYTE prevFlag, BYTE newFlag)
{
	if (prevFlag == newFlag)
		return;
	if (newFlag == MLEADER_NONE) {
		MonUpdateLeader(mnum);
		return;
	}
	assert(prevFlag == MLEADER_PRESENT && newFlag == MLEADER_AWAY);
	monsters[monsters[mnum].leader].packsize--;
}

void DeltaLoadLevel()
{
	DMonsterStr* mstr;
	DObjectStr* dstr;
	MonsterStruct* mon;
	DItemStr* itm;
	int ii;
	int i;
	int x, y;

	assert(IsMultiGame);

	deltaload = true;
	if (currLvl._dLevelIdx != DLV_TOWN) {
		for (i = 0; i < MAX_MINIONS; i++)
			InitGolemStats(i, gsDeltaData.ddJunk.jGolems[i]);

		mstr = gsDeltaData.ddLevel[currLvl._dLevelIdx].monster;
		for (i = 0; i < nummonsters; i++, mstr++) {
			if (mstr->_mCmd != DCMD_MON_INVALID) {
				// skip minions and prespawn skeletons
				if (!MINION_NR_INACTIVE(i))
					RemoveMonFromMap(i);
				x = mstr->_mx;
				y = mstr->_my;
				mon = &monsters[i];
				SetMonsterLoc(mon, x, y);
				mon->_mdir = mstr->_mdir;
				UpdateLeader(i, mon->leaderflag, mstr->_mleaderflag);
				// SyncDeadLight: inline for better performance + apply to moving monsters
				if (mon->mlid != NO_LIGHT)
					ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
				static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "DeltaLoadLevel expects ordered DCMD_MON_ enum I.");
				static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "DeltaLoadLevel expects ordered DCMD_MON_ enum II.");
				if (mstr->_mCmd >= DCMD_MON_DEAD) {
					AddDead(i, mstr->_mCmd);
				} else {
					mon->_mhitpoints = SwapLE32(mstr->_mhitpoints);
					mon->_msquelch = mstr->_mactive;
					mon->_mWhoHit = mstr->_mWhoHit;
					dMonster[mon->_mx][mon->_my] = i + 1;
					// SyncMonsterAnim(mnum);
					assert(mon->_mmode == MM_STAND);
					mon->_mAnimData = mon->_mAnims[MA_STAND].aData[mon->_mdir];
				}
			}
		}
		// SyncDeadLight();

		memcpy(automapview, gsDeltaData.ddLocal[currLvl._dLevelIdx].automapsv, sizeof(automapview));

		dstr = gsDeltaData.ddLevel[currLvl._dLevelIdx].object;
		for (i = 0; i < MAXOBJECTS; i++, dstr++) {
			if (dstr->bCmd != DCMD_INVALID) {
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
				default:
					ASSUME_UNREACHABLE
					break;
				}
			}
		}
	}

	// load items last, because they depend on the object state
	//  I. remove items
	itm = gsDeltaData.ddLevel[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, itm++) {
		if (itm->bCmd == DCMD_ITM_TAKEN || itm->bCmd == DCMD_ITM_MOVED) {
			ii = FindGetItem(SwapLE32(itm->item.dwSeed), SwapLE16(itm->item.wIndx), SwapLE16(itm->item.wCI));
			if (ii != -1) {
				if (dItem[items[ii]._ix][items[ii]._iy] == ii + 1)
					dItem[items[ii]._ix][items[ii]._iy] = 0;
				DeleteItems(ii);
			}
		}
	}
	//  II. place items
	itm = gsDeltaData.ddLevel[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, itm++) {
		if (itm->bCmd == DCMD_ITM_DROPPED || itm->bCmd == DCMD_ITM_MOVED) {
			UnPackPkItem(&itm->item);
			x = itm->x;
			y = itm->y;
			if (!CanPut(x, y))
				FindItemLocation(x, y, &x, &y, DSIZEX / 2);

			ii = itemactive[numitems];
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

void NetSendCmdJoinLevel()
{
	TCmdJoinLevel cmd;

	cmd.bCmd = CMD_JOINLEVEL;
	cmd.lLevel = currLvl._dLevelIdx;
	cmd.px = ViewX;
	cmd.py = ViewY;
	cmd.php = SwapLE32(myplr._pHPBase);
	cmd.pmp = SwapLE32(myplr._pManaBase);
	cmd.lTimer1 = SwapLE16(myplr._pTimer[PLTR_INFRAVISION]);
	cmd.lTimer2 = SwapLE16(myplr._pTimer[PLTR_RAGE]);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdAckJoinLevel()
{
	TCmdAckJoinLevel cmd;

	cmd.bCmd = CMD_ACK_JOINLEVEL;
	cmd.lManashield = myplr._pManaShield;
	cmd.lTimer1 = SwapLE16(myplr._pTimer[PLTR_INFRAVISION]);
	cmd.lTimer2 = SwapLE16(myplr._pTimer[PLTR_RAGE]);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
	//dthread_send_delta(pnum, CMD_ACK_JOINLEVEL, &cmd, sizeof(cmd));
}

void NetSendCmd(BYTE bCmd)
{
	TCmd cmd;

	cmd.bCmd = bCmd;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonstKill(int mnum, int pnum)
{
	TCmdMonstKill cmd;
	MonsterStruct* mon;

	cmd.mkParam1.bCmd = CMD_MONSTDEATH;
	cmd.mkPnum = pnum;
	cmd.mkMnum = SwapLE16(mnum);
	mon = &monsters[mnum];
	cmd.mkExp = SwapLE16(mon->_mExp);
	cmd.mkMonLevel = mon->_mLevel;
	cmd.mkParam1.x = mon->_mx;
	cmd.mkParam1.y = mon->_my;
	cmd.mkDir = mon->_mdir;
	cmd.mkMode = mon->_mmode;
	cmd.mkParam1.bParam1 = currLvl._dLevelIdx;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdGolem()
{
	TCmdGolem cmd;
	MonsterStruct* mon;

	mon = &monsters[mypnum];
	cmd.bCmd = CMD_AWAKEGOLEM;
	cmd.goX = mon->_mx;
	cmd.goY = mon->_my;
	cmd.goMonLevel = mon->_mLevel;
	cmd.goDunLevel = currLvl._dLevelIdx;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLoc(BYTE bCmd, BYTE x, BYTE y)
{
	TCmdLoc cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLocBParam1(BYTE bCmd, BYTE x, BYTE y, BYTE bParam1)
{
	TCmdLocBParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.bParam1 = bParam1;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLocParam1(BYTE bCmd, BYTE x, BYTE y, WORD wParam1)
{
	TCmdLocParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.wParam1 = SwapLE16(wParam1);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdParam1(BYTE bCmd, WORD wParam1)
{
	TCmdParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = SwapLE16(wParam1);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdParam2(BYTE bCmd, WORD wParam1, WORD wParam2)
{
	TCmdParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = SwapLE16(wParam1);
	cmd.wParam2 = SwapLE16(wParam2);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdBParam1(BYTE bCmd, BYTE bParam1)
{
	TCmdBParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.bParam1 = bParam1;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdBParam2(BYTE bCmd, BYTE bParam1, BYTE bParam2)
{
	TCmdBParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.bParam1 = bParam1;
	cmd.bParam2 = bParam2;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendShrineCmd(BYTE type, int seed)
{
	TCmdShrine cmd;

	cmd.bCmd = CMD_SHRINE;
	cmd.shType = type;
	cmd.shSeed = SwapLE32(seed);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdQuest(BYTE q, bool extOnly)
{
	TCmdQuest cmd;

	cmd.bCmd = extOnly ? CMD_SYNCQUESTEXT : CMD_SYNCQUEST;
	cmd.q = q;
	cmd.qstate = quests[q]._qactive;
	cmd.qlog = quests[q]._qlog;
	cmd.qvar1 = quests[q]._qvar1;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void SendStoreCmd1(unsigned idx, BYTE bStoreId, int value)
{
	TCmdStore1 cmd;

	cmd.bCmd = CMD_STORE_1;
	cmd.stCmd = bStoreId;
	cmd.stLoc = idx;
	cmd.stValue = SwapLE32(value);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void SendStoreCmd2(BYTE bStoreId)
{
	TCmdStore2 cmd;

	cmd.bCmd = CMD_STORE_2;
	cmd.stCmd = bStoreId;
	PackPkItem(&cmd.item, &storeitem);
	cmd.stValue = SwapLE32(storeitem._iIvalue);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
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
	cmd.fromFloor = is->_iFloorFlag;

	PackPkItem(&cmd.item, is);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdPutItem(BYTE x, BYTE y)
{
	TCmdPItem cmd;

	cmd.bCmd = CMD_PUTITEM;
	cmd.bLevel = currLvl._dLevelIdx;
	cmd.x = x;
	cmd.y = y;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdRespawnItem(int ii)
{
	ItemStruct* is;
	TCmdRPItem cmd;

	is = &items[ii];
	cmd.bCmd = CMD_RESPAWNITEM;
	cmd.x = is->_ix;
	cmd.y = is->_iy;
	cmd.bLevel = currLvl._dLevelIdx;

	PackPkItem(&cmd.item, is);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdSpawnItem(bool flipFlag)
{
	ItemStruct* is;
	TCmdRPItem cmd;

	is = &items[MAXITEMS];
	cmd.bCmd = CMD_SPAWNITEM;
	cmd.x = is->_ix;
	cmd.y = is->_iy;
	cmd.bLevel = currLvl._dLevelIdx;
	cmd.bFlipFlag = flipFlag;

	PackPkItem(&cmd.item, is);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdDelItem(BYTE bLoc)
{
	TCmdBParam1 cmd;

	cmd.bCmd = CMD_DELPLRITEM;
	cmd.bParam1 = bLoc;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdItemSkill(int cii, BYTE skill, char from)
{
	TCmdItemOp cmd;

	cmd.bCmd = CMD_OPERATEITEM;
	cmd.ioIdx = cii;
	cmd.iou.skill = skill;
	cmd.iou.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLocAttack(BYTE x, BYTE y, BYTE skill, char from)
{
	TCmdLocAttack cmd;

	cmd.bCmd = (myplr._pSkillFlags & SFLAG_MELEE) ? CMD_SATTACKXY : CMD_RATTACKXY;
	cmd.x = x;
	cmd.y = y;
	cmd.lau.skill = skill;
	cmd.lau.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLocSkill(BYTE x, BYTE y, BYTE skill, char from)
{
	TCmdLocSkill cmd;

	cmd.bCmd = CMD_SPELLXY;
	cmd.x = x;
	cmd.y = y;
	cmd.lsu.skill = skill;
	cmd.lsu.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdPlrAttack(int pnum, BYTE skill, char from)
{
	TCmdPlrAttack cmd;

	cmd.bCmd = (myplr._pSkillFlags & SFLAG_MELEE) ? CMD_ATTACKPID : CMD_RATTACKPID;
	cmd.paPnum = pnum;
	cmd.pau.skill = skill;
	cmd.pau.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdPlrSkill(int pnum, BYTE skill, char from)
{
	TCmdPlrSkill cmd;

	cmd.bCmd = CMD_SPELLPID;
	cmd.psPnum = pnum;
	cmd.psu.skill = skill;
	cmd.psu.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonstAttack(BYTE bCmd, int mnum, BYTE skill, char from)
{
	TCmdMonstAttack cmd;

	cmd.bCmd = bCmd;
	cmd.maMnum = SwapLE16(mnum);
	cmd.mau.skill = skill;
	cmd.mau.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonstSkill(int mnum, BYTE skill, char from)
{
	TCmdMonstSkill cmd;

	cmd.bCmd = CMD_SPELLID;
	cmd.msMnum = SwapLE16(mnum);
	cmd.msu.skill = skill;
	cmd.msu.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonstDamage(int mnum, int hitpoints)
{
	TCmdMonstDamage cmd;

	cmd.bCmd = CMD_MONSTDAMAGE;
	cmd.mdLevel = currLvl._dLevelIdx;
	cmd.mdMnum = SwapLE16(mnum);
	cmd.mdHitpoints = SwapLE32(hitpoints);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonstCorpse(int mnum)
{
	TCmdLocBParam1 cmd;
	MonsterStruct* mon;

	mon = &monsters[mnum];
	cmd.bCmd = CMD_MONSTCORPSE;
	cmd.bParam1 = currLvl._dLevelIdx;
	cmd.x = mon->_mx;
	cmd.y = mon->_my;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdString(unsigned int pmask)
{
	int dwStrLen;
	TCmdString cmd;

	static_assert((sizeof(gbNetMsg) + 2) <= (sizeof(MsgPkt) - sizeof(MsgPktHdr)), "String message does not fit in MsgPkt.");
	dwStrLen = strlen(gbNetMsg);
	cmd.bCmd = NMSG_STRING;
	memcpy(cmd.str, gbNetMsg, dwStrLen + 1);
	multi_send_direct_msg(pmask, (BYTE*)&cmd, dwStrLen + 2);
}

static void delta_open_portal(int pnum, BYTE x, BYTE y, BYTE bLevel)
{
	net_assert(bLevel < NUM_LEVELS);
	gsDeltaData.ddJunkChanged = true;
	gsDeltaData.ddJunk.jPortals[pnum].x = x;
	gsDeltaData.ddJunk.jPortals[pnum].y = y;
	gsDeltaData.ddJunk.jPortals[pnum].level = bLevel;
}

void delta_close_portal(int pnum)
{
	//memset(&gsDeltaData.ddJunk.portal[pnum], 0, sizeof(gsDeltaData.ddJunk.portal[pnum]));
	gsDeltaData.ddJunk.jPortals[pnum].level = DLV_TOWN;
	// assert(gsDeltaData.ddJunkChanged == true);
}

static void check_update_plr(int pnum)
{
	if (IsMultiGame) {
		assert(pnum == mypnum);
		pfile_update(true);
	}
}

#if DEV_MODE
static void msg_errorf(const char* pszFmt, ...)
{
	//static DWORD msg_err_timer;
	//DWORD ticks;
	char msg[256];
	va_list va;

	va_start(va, pszFmt);
	//ticks = SDL_GetTicks();
	//if (ticks - msg_err_timer >= 5000) {
	//	msg_err_timer = ticks;
		vsnprintf(msg, sizeof(msg), pszFmt, va);
		ErrorPlrMsg(msg);
	//}
	va_end(va);
}
#endif

static unsigned On_SYNCDATA(TCmd* pCmd, int pnum)
{
	TSyncHeader* pHdr = (TSyncHeader*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == pHdr->bLevel)
	//	sync_update(pnum, pHdr);
	delta_sync_monster(pHdr);

	return SwapLE16(pHdr->wLen) + sizeof(*pHdr);
}

static unsigned On_WALKXY(TCmd* pCmd, int pnum)
{
	TCmdLoc* cmd = (TCmdLoc*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, true);
		plr.destAction = ACTION_WALK;
	}

	return sizeof(*cmd);
}

static unsigned On_ADDSTR(TCmd* pCmd, int pnum)
{
	IncreasePlrStr(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDMAG(TCmd* pCmd, int pnum)
{
	IncreasePlrMag(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDDEX(TCmd* pCmd, int pnum)
{
	IncreasePlrDex(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDVIT(TCmd* pCmd, int pnum)
{
	IncreasePlrVit(pnum);

	return sizeof(*pCmd);
}

static unsigned On_BLOCK(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_BLOCK;
		plr.destParam1 = cmd->bParam1; // direction
	}
	return sizeof(*cmd);
}

static unsigned On_GOTOGETITEM(TCmd* pCmd, int pnum)
{
	TCmdLocParam1* cmd = (TCmdLocParam1*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr.destAction = ACTION_PICKUPITEM;
		plr.destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_GETITEM(TCmd* pCmd, int pnum)
{
	TCmdGItem* cmd = (TCmdGItem*)pCmd;
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
			// assert(!plr._pHoldItem._iFloorFlag);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_GOTOAGETITEM(TCmd* pCmd, int pnum)
{
	TCmdLocParam1* cmd = (TCmdLocParam1*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		MakePlrPath(pnum, cmd->x, cmd->y, false);
		plr.destAction = ACTION_PICKUPAITEM;
		plr.destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_AUTOGETITEM(TCmd* pCmd, int pnum)
{
	TCmdGItem* cmd = (TCmdGItem*)pCmd;
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

static unsigned On_PUTITEM(TCmd* pCmd, int pnum)
{
	TCmdPItem* cmd = (TCmdPItem*)pCmd;
	ItemStruct* pi;
	int x, y;

	pi = &plr._pHoldItem;
	if (pi->_itype != ITYPE_NONE) {
		x = cmd->x;
		y = cmd->y;
#ifdef HELLFIRE
		if (cmd->bLevel == DLV_TOWN && CheckTownTrigs(pnum, x, y, pi->_iIdx)) {
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
		if (delta_put_item(&pkItem, cmd->bLevel, x, y)) {
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
	}

	return sizeof(*cmd);
}

static unsigned On_RESPAWNITEM(TCmd* pCmd, int pnum)
{
	TCmdRPItem* cmd = (TCmdRPItem*)pCmd;

	if (currLvl._dLevelIdx == cmd->bLevel && pnum != mypnum) {
		UnPackPkItem(&cmd->item);
		SyncPutItem(-1, cmd->x, cmd->y, true);
	}
	delta_put_item(&cmd->item, cmd->bLevel, cmd->x, cmd->y);

	return sizeof(*cmd);
}

static unsigned On_SPAWNITEM(TCmd* pCmd, int pnum)
{
	TCmdRPItem* cmd = (TCmdRPItem*)pCmd;

	if (delta_put_item(&cmd->item, cmd->bLevel, cmd->x, cmd->y) &&
		currLvl._dLevelIdx == cmd->bLevel) {
		UnPackPkItem(&cmd->item);
		SyncPutItem(-1, cmd->x, cmd->y, cmd->bFlipFlag);
	}

	return sizeof(*cmd);
}

static bool CheckPlrSkillUse(int pnum, CmdSkillUse &su)
{
	int ma;
	BYTE sn = su.skill;
	char sf = su.from;
	bool sameLvl = currLvl._dLevelIdx == plr._pDunLevel;

	net_assert(sn != SPL_NULL && sn < NUM_SPELLS);

	if (plr._pmode != PM_DEATH && (spelldata[sn].sFlags & plr._pSkillFlags) == spelldata[sn].sFlags) {
		su.from = plr._pSkillLvl[sn];
		if (sf == SPLFROM_MANA) {
#if DEBUG_MODE
			if (debug_mode_key_inverted_v)
				return true;
#endif
			if (su.from == 0)
				return false;
			// do not deduct mana if the skill/level matches the set (skill based) action
			static_assert((int)ACTION_ATTACK + 1 == (int)ACTION_ATTACKMON, "CheckPlrSkillUse expects ordered action-ids I.");
			static_assert((int)ACTION_ATTACKMON + 1 == (int)ACTION_ATTACKPLR, "CheckPlrSkillUse expects ordered action-ids II.");
			static_assert((int)ACTION_ATTACKPLR + 1 == (int)ACTION_RATTACK, "CheckPlrSkillUse expects ordered action-ids III.");
			static_assert((int)ACTION_RATTACK + 1 == (int)ACTION_RATTACKMON, "CheckPlrSkillUse expects ordered action-ids IV.");
			static_assert((int)ACTION_RATTACKMON + 1 == (int)ACTION_RATTACKPLR, "CheckPlrSkillUse expects ordered action-ids V.");
			static_assert((int)ACTION_RATTACKPLR + 1 == (int)ACTION_SPELL, "CheckPlrSkillUse expects ordered action-ids VI.");
			static_assert((int)ACTION_SPELL + 1 == (int)ACTION_SPELLMON, "CheckPlrSkillUse expects ordered action-ids VII.");
			static_assert((int)ACTION_SPELLMON + 1 == (int)ACTION_SPELLPLR, "CheckPlrSkillUse expects ordered action-ids VIII.");
			if (sn == plr.destParam3 && ((BYTE)su.from) == plr.destParam4 &&
			 plr.destAction >= ACTION_ATTACK && plr.destAction <= ACTION_SPELLPLR)
				return sameLvl;
			net_assert(plr._pMemSkills & SPELL_MASK(sn));
			// always grant skill-activity to prevent de-sync
			// TODO: add checks to prevent abuse?
			ma = GetManaAmount(pnum, sn);
			plr._pSkillActivity[sn] = std::min((ma >> (6 + 1)) + plr._pSkillActivity[sn], UCHAR_MAX);
			// TODO: enable this for every player
			if (pnum == mypnum) {
				if (plr._pMana < ma)
					return false;
				PlrDecMana(pnum, ma);
			}
		} else if (sf == SPLFROM_ABILITY) {
			net_assert(plr._pAblSkills & SPELL_MASK(sn));
		} else {
			net_assert((BYTE)sf < NUM_INVELEM);
			if (!SyncUseItem(pnum, sf, sn))
				return false;
		}
		return sameLvl;
	}
	return false;
}

static unsigned On_SATTACKXY(TCmd* pCmd, int pnum)
{
	TCmdLocAttack* cmd = (TCmdLocAttack*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->lau)) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_ATTACK;
		plr.destParam1 = cmd->x;
		plr.destParam2 = cmd->y;
		plr.destParam3 = cmd->lau.skill; // attack skill
		plr.destParam4 = (BYTE)cmd->lau.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKXY(TCmd* pCmd, int pnum)
{
	TCmdLocAttack* cmd = (TCmdLocAttack*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->lau)) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_RATTACK;
		plr.destParam1 = cmd->x;
		plr.destParam2 = cmd->y;
		plr.destParam3 = cmd->lau.skill; // attack skill
		plr.destParam4 = (BYTE)cmd->lau.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLXY(TCmd* pCmd, int pnum)
{
	TCmdLocSkill* cmd = (TCmdLocSkill*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->lsu)) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_SPELL;
		plr.destParam1 = cmd->x;
		plr.destParam2 = cmd->y;
		plr.destParam3 = cmd->lsu.skill; // spell
		plr.destParam4 = (BYTE)cmd->lsu.from; // spllvl (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_OPERATEITEM(TCmd* pCmd, int pnum)
{
	TCmdItemOp* cmd = (TCmdItemOp*)pCmd;

	if (plr._pmode == PM_DEATH) // FIXME: not in exact sync! (see SyncUseItem and DoBuckle)
		return sizeof(*cmd);

	// manipulate the item
	net_assert((BYTE)cmd->iou.from < NUM_INVELEM ||
		(cmd->iou.from == SPLFROM_ABILITY && cmd->iou.skill != SPL_OIL));
	net_assert(cmd->ioIdx < NUM_INVELEM);

#ifdef HELLFIRE
	if (cmd->iou.skill == SPL_OIL)
		DoOil(pnum, cmd->iou.from, cmd->ioIdx);
	else
#endif
		DoAbility(pnum, cmd->iou.from, cmd->ioIdx);

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		// add cast effect
		ClrPlrPath(pnum);
		plr.destAction = ACTION_SPELL;
		plr.destParam1 = plr._px;
		plr.destParam2 = plr._py;
		plr.destParam3 = cmd->iou.skill; // spell
		plr.destParam4 = 0;              // spllvl (should not matter)
	}

	return sizeof(*cmd);
}

static unsigned On_OPOBJXY(TCmd* pCmd, int pnum)
{
	TCmdLocParam1* cmd = (TCmdLocParam1*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		int oi = SwapLE16(cmd->wParam1);
		plr.destAction = ACTION_OPERATE;
		plr.destParam1 = oi;
		plr.destParam2 = cmd->x;
		plr.destParam3 = cmd->y;
		static_assert((int)ODT_NONE == 0, "BitOr optimization of On_OPOBJXY expects ODT_NONE to be zero.");
		MakePlrPath(pnum, cmd->x, cmd->y, !(objects[oi]._oSolidFlag | objects[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_DISARMXY(TCmd* pCmd, int pnum)
{
	TCmdLocParam1* cmd = (TCmdLocParam1*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		int oi = SwapLE16(cmd->wParam1);
		plr.destAction = ACTION_DISARM;
		plr.destParam1 = oi;
		plr.destParam2 = cmd->x;
		plr.destParam3 = cmd->y;
		static_assert((int)ODT_NONE == 0, "BitOr optimization of On_DISARMXY expects ODT_NONE to be zero.");
		MakePlrPath(pnum, cmd->x, cmd->y, !(objects[oi]._oSolidFlag | objects[oi]._oDoorFlag));
	}

	return sizeof(*cmd);
}

static unsigned On_OPOBJT(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		plr.destAction = ACTION_OPERATETK;
		plr.destParam1 = SwapLE16(cmd->wParam1);
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKID(TCmd* pCmd, int pnum)
{
	TCmdMonstAttack* cmd = (TCmdMonstAttack*)pCmd;
	int mnum;

	if (CheckPlrSkillUse(pnum, cmd->mau)) {
		mnum = SwapLE16(cmd->maMnum);
		plr.destAction = ACTION_ATTACKMON;
		plr.destParam1 = mnum;
		plr.destParam3 = cmd->mau.skill; // attack skill
		plr.destParam4 = (BYTE)cmd->mau.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_ATTACKPID(TCmd* pCmd, int pnum)
{
	TCmdPlrAttack* cmd = (TCmdPlrAttack*)pCmd;
	int tnum;

	if (CheckPlrSkillUse(pnum, cmd->pau)) {
		tnum = cmd->paPnum;
		plr.destAction = ACTION_ATTACKPLR;
		plr.destParam1 = tnum;
		plr.destParam3 = cmd->pau.skill; // attack skill
		plr.destParam4 = (BYTE)cmd->pau.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKID(TCmd* pCmd, int pnum)
{
	TCmdMonstAttack* cmd = (TCmdMonstAttack*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->mau)) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_RATTACKMON;
		plr.destParam1 = SwapLE16(cmd->maMnum);  // target id
		plr.destParam3 = cmd->mau.skill; // attack skill
		plr.destParam4 = (BYTE)cmd->mau.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_RATTACKPID(TCmd* pCmd, int pnum)
{
	TCmdPlrAttack* cmd = (TCmdPlrAttack*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->pau)) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_RATTACKPLR;
		plr.destParam1 = cmd->paPnum;    // target id
		plr.destParam3 = cmd->pau.skill; // attack skill
		plr.destParam4 = (BYTE)cmd->pau.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLID(TCmd* pCmd, int pnum)
{
	TCmdMonstSkill* cmd = (TCmdMonstSkill*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->msu)) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_SPELLMON;
		plr.destParam1 = SwapLE16(cmd->msMnum); // mnum
		plr.destParam3 = cmd->msu.skill;        // spell
		plr.destParam4 = (BYTE)cmd->msu.from;   // spllvl (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_SPELLPID(TCmd* pCmd, int pnum)
{
	TCmdPlrSkill* cmd = (TCmdPlrSkill*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->psu)) {
		ClrPlrPath(pnum);
		plr.destAction = ACTION_SPELLPLR;
		plr.destParam1 = cmd->psPnum;    // pnum
		plr.destParam3 = cmd->psu.skill; // spell
		plr.destParam4 = (BYTE)cmd->psu.from; // spllvl (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_KNOCKBACK(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;
	int mnum = SwapLE16(cmd->wParam1);

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		// assert(mnum >= MAX_MINIONS); TODO: validate data from internet
		if (!CanTalkToMonst(mnum) && (monsters[mnum]._mmaxhp >> 6) < plr._pMagic) {
			MonGetKnockback(mnum, plr._px, plr._py);
			MonStartHit(mnum, pnum, 0, 0);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_TALKXY(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;
	int mnum = SwapLE16(cmd->wParam1);

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		MakePlrPath(pnum, monsters[mnum]._mx, monsters[mnum]._my, false);
		plr.destAction = ACTION_TALK;
		plr.destParam1 = mnum;
	}

	return sizeof(*cmd);
}

static unsigned On_NEWLVL(TCmd* pCmd, int pnum)
{
	TCmdParam2* cmd = (TCmdParam2*)pCmd;

	StartNewLvl(pnum, SwapLE16(cmd->wParam1), SwapLE16(cmd->wParam2));

	return sizeof(*cmd);
}

static unsigned On_TWARP(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;
	BYTE idx = cmd->bParam1;

	net_assert(idx < MAX_PLRS);

	if (plr._pmode != PM_DEATH)
		StartTWarp(pnum, idx);

	return sizeof(*cmd);
}

static unsigned On_MONSTDEATH(TCmd* pCmd, int pnum)
{
	TCmdMonstKill* cmd = (TCmdMonstKill*)pCmd;
	int i, lvl;
	unsigned totplrs, xp;
	BYTE whoHit, mask;

	if (pnum != mypnum && currLvl._dLevelIdx == cmd->mkParam1.bParam1)
		MonSyncStartKill(SwapLE16(cmd->mkMnum), cmd->mkParam1.x, cmd->mkParam1.y, cmd->mkPnum);

	whoHit = delta_kill_monster(cmd);

	totplrs = 0;
	mask = whoHit;
	while (mask != 0) {
		totplrs += (mask & 1) != 0;
		mask >>= 1;
	}
	if (totplrs != 0) {
		xp = cmd->mkExp / totplrs;
		lvl = cmd->mkMonLevel;
		for (i = 0; i < MAX_PLRS; i++, whoHit >>= 1) {
			if ((whoHit & 1) && plx(i)._pmode != PM_DEATH)
				AddPlrExperience(i, lvl, xp);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_AWAKEGOLEM(TCmd* pCmd, int pnum)
{
	TCmdGolem* cmd = (TCmdGolem*)pCmd;

	delta_awake_golem(cmd, pnum);

	return sizeof(*cmd);
}

static unsigned On_MONSTDAMAGE(TCmd* pCmd, int pnum)
{
	TCmdMonstDamage* cmd = (TCmdMonstDamage*)pCmd;

	delta_monster_hp(cmd, pnum);

	return sizeof(*cmd);
}

static unsigned On_MONSTCORPSE(TCmd* pCmd, int pnum)
{
	TCmdLocBParam1* cmd = (TCmdLocBParam1*)pCmd;

	delta_monster_corpse(cmd);

	return sizeof(*cmd);
}

static unsigned On_PLRDEAD(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	if (pnum != mypnum)
		SyncPlrKill(pnum, cmd->bParam1);
	else
		check_update_plr(pnum);

	return sizeof(*cmd);
}

static unsigned On_PLRRESURRECT(TCmd* pCmd, int pnum)
{
	SyncPlrResurrect(pnum);
	if (pnum == mypnum)
		check_update_plr(pnum);

	return sizeof(*pCmd);
}

static unsigned On_DOOROPEN(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncDoorOpen(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_DOOROPEN, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_DOORCLOSE(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncDoorClose(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_DOORCLOSE, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPDISABLE(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapDisable(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPDISABLE, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPOPEN(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapOpen(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPOPEN, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPCLOSE(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapClose(SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_TRAPCLOSE, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_OPERATEOBJ(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
		SyncOpObject(pnum, SwapLE16(cmd->wParam1));
	delta_sync_object(SwapLE16(cmd->wParam1), CMD_OPERATEOBJ, plr._pDunLevel);

	return sizeof(*cmd);
}

/**
 * Sync Shrine effect with every player
 */
static unsigned On_SHRINE(TCmd* pCmd, int pnum)
{
	TCmdShrine* cmd = (TCmdShrine*)pCmd;

	if (plr._pmode != PM_DEATH)
		SyncShrineCmd(pnum, cmd->shType, SwapLE32(cmd->shSeed));

	return sizeof(*cmd);
}

static unsigned On_SPLITPLRGOLD(TCmd* pCmd, int pnum)
{
	TCmdParam2* cmd = (TCmdParam2*)pCmd;
	WORD r = SwapLE16(cmd->wParam1);

	net_assert(r < NUM_INV_GRID_ELEM);

	// if (plr._pmode != PM_DEATH)
		SyncSplitGold(pnum, r, SwapLE16(cmd->wParam2));

	return sizeof(*cmd);
}

static unsigned On_PASTEPLRITEM(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;
	BYTE r = cmd->bParam1;

	net_assert(r < SLOTXY_BELT_FIRST);

	if (plr._pmode != PM_DEATH)
		InvPasteItem(pnum, r);

	return sizeof(*cmd);
}

static unsigned On_PASTEPLRBELTITEM(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;
	BYTE r = cmd->bParam1;

	r -= SLOTXY_BELT_FIRST;
	net_assert(r < MAXBELTITEMS);

	if (plr._pmode != PM_DEATH)
		InvPasteBeltItem(pnum, r);

	return sizeof(*cmd);
}

static unsigned On_CUTPLRITEM(TCmd* pCmd, int pnum)
{
	TCmdBParam2* cmd = (TCmdBParam2*)pCmd;
	BYTE r = cmd->bParam1;

	net_assert(r < NUM_INVELEM);

	if (plr._pmode != PM_DEATH)
		InvCutItem(pnum, r, cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_DELPLRITEM(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;
	BYTE r = cmd->bParam1;

	net_assert(r < NUM_INVELEM);

	SyncPlrItemRemove(pnum, r);

	return sizeof(*cmd);
}

static unsigned On_USEPLRITEM(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;
	BYTE r = cmd->bParam1;

	net_assert(r < NUM_INVELEM);

	if (plr._pmode != PM_DEATH)	// FIXME: not in exact sync! (see DoAbility and FIXME in SyncUseItem)
		SyncUseItem(pnum, r, SPL_INVALID);

	return sizeof(*cmd);
}

static unsigned On_SEND_GAME_DELTA(TCmd* pCmd, int pnum)
{
	net_assert((unsigned)pnum < MAX_PLRS);

	if (pnum != mypnum)
		guSendGameDelta |= 1 << pnum;

	return sizeof(*pCmd);
}

static unsigned On_PLRINFO(TCmd* pCmd, int pnum)
{
	TCmdPlrInfoHdr* cmd = (TCmdPlrInfoHdr*)pCmd;

	net_assert((unsigned)pnum < MAX_PLRS);

	if (geBufferMsgs == MSG_GAME_DELTA_LOAD || geBufferMsgs == MSG_GAME_DELTA_WAIT)
		DeltaQueuePacket(pnum, cmd, cmd->wBytes + sizeof(*cmd));
	else if (pnum != mypnum)
		multi_recv_plrinfo_msg(pnum, cmd);

	return cmd->wBytes + sizeof(*cmd);
}

static unsigned ON_PLRDROP(TCmd* pCmd, int pnum)
{
	TFakeDropPlr* cmd = (TFakeDropPlr*)pCmd;

	net_assert((unsigned)pnum < MAX_PLRS);

	multi_deactivate_player(pnum, cmd->bReason);

	return sizeof(*cmd);
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

static unsigned On_ACK_JOINLEVEL(TCmd* pCmd, int pnum)
{
	TCmdAckJoinLevel* cmd = (TCmdAckJoinLevel*)pCmd;

	plr._pManaShield = cmd->lManashield;
	plr._pTimer[PLTR_INFRAVISION] = SwapLE16(cmd->lTimer1) > gbNetUpdateRate ? SwapLE16(cmd->lTimer1) - gbNetUpdateRate : 0;
	plr._pTimer[PLTR_RAGE] = msg_calc_rage(cmd->lTimer2);
	CalcPlrItemVals(pnum, false); // last parameter should not matter

	return sizeof(*cmd);
}

static unsigned On_JOINLEVEL(TCmd* pCmd, int pnum)
{
	TCmdJoinLevel* cmd = (TCmdJoinLevel*)pCmd;

	plr._pLvlChanging = FALSE;
	if (plr._pmode != PM_DEATH)
		plr._pInvincible = FALSE;
	net_assert(cmd->lLevel < NUM_LEVELS);
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
			net_assert(plr._pTeam == pnum);
			net_assert(plr._pManaShield == 0);
			net_assert(cmd->lLevel == DLV_TOWN);
			net_assert(cmd->lTimer1 == 0);
			net_assert(cmd->lTimer2 == 0);
			plr._pActive = TRUE;
			gbActivePlayers++;
			EventPlrMsg("Player '%s' (level %d) just joined the game", plr._pName, plr._pLevel);
			msg_mask_monhit(pnum);
		}
		plr._pHPBase = SwapLE32(cmd->php);
		plr._pManaBase = SwapLE32(cmd->pmp);
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

static unsigned On_ACTIVATEPORTAL(TCmd* pCmd, int pnum)
{
	TCmdLocBParam1* cmd = (TCmdLocBParam1*)pCmd;

	ActivatePortal(pnum, cmd->x, cmd->y, cmd->bParam1);
	if (pnum != mypnum) {
		if (currLvl._dLevelIdx == DLV_TOWN)
			AddInTownPortal(pnum);
		else if (currLvl._dLevelIdx == cmd->bParam1) {
			int i;
			for (i = 0; i < nummissiles; i++) {
				MissileStruct* mis = &missile[missileactive[i]];
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

static unsigned On_DEACTIVATEPORTAL(TCmd* pCmd, int pnum)
{
	if (PortalOnLevel(pnum))
		RemovePortalMissile(pnum);
	DeactivatePortal(pnum);
	delta_close_portal(pnum);

	return sizeof(*pCmd);
}

static unsigned On_RETOWN(TCmd* pCmd, int pnum)
{
	RestartTownLvl(pnum);

	return sizeof(*pCmd);
}

static unsigned On_STRING(TCmd* pCmd, int pnum)
{
	TCmdString* cmd = (TCmdString*)pCmd;

	//if (geBufferMsgs != MSG_GAME_DELTA_LOAD && geBufferMsgs != MSG_GAME_DELTA_WAIT) {
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

static unsigned On_INVITE(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	 // TODO: check (cmd->bParam1 == mypnum) should not be necessary in a server/client solution
	if (cmd->bParam1 == mypnum && plr._pTeam == pnum) {
		guTeamInviteRec |= (1 << pnum);
		EventPlrMsg("%s invited to their team.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_ACK_INVITE(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

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

static unsigned On_DEC_INVITE(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteSent &= ~(1 << pnum);

		EventPlrMsg("%s rejected your invitation.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_REV_INVITE(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteRec &= ~(1 << pnum);

		EventPlrMsg("%s revoked the invitation.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_KICK_PLR(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;
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
	BYTE c = cmd->stCmd, r = cmd->stLoc;

	net_assert(plr._pmode != PM_DEATH);
	net_assert(plr._pDunLevel == DLV_TOWN);
	net_assert(c == STORE_SSELL || c == STORE_SIDENTIFY || c == STORE_SREPAIR || c == STORE_WRECHARGE || c == STORE_BOY);
	net_assert(r < NUM_INVELEM);

	SyncStoreCmd(pnum, c, r, SwapLE32(cmd->stValue));

	return sizeof(*cmd);
}

/**
 * Execute store transactions II.
 */
static unsigned On_STORE_2(TCmd* pCmd, int pnum)
{
	TCmdStore2* cmd = (TCmdStore2*)pCmd;
	BYTE c = cmd->stCmd;

	net_assert(plr._pmode != PM_DEATH);
	net_assert(plr._pDunLevel == DLV_TOWN);
	net_assert(c == STORE_HBUY || c == STORE_SBUY || c == STORE_SPBUY || c == STORE_WBUY || c == STORE_BBOY);

	UnPackPkItem(&cmd->item);
	SyncStoreCmd(pnum, c, MAXITEMS, SwapLE32(cmd->stValue));

	return sizeof(*cmd);
}

/* Sync item rewards in town. */
static unsigned On_QTOWNER(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	net_assert(plr._pmode != PM_DEATH);
	net_assert(plr._pDunLevel == DLV_TOWN);

	SyncTownerQ(pnum, SwapLE16(cmd->wParam1));

	return sizeof(*cmd);
}

/* Sync item rewards in dungeon. */
static unsigned On_QMONSTER(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	if (plr._pmode != PM_DEATH)
		SyncMonsterQ(pnum, SwapLE16(cmd->wParam1));

	return sizeof(*cmd);
}

/**
 * Sync Quest with every player.
 */
static unsigned On_SYNCQUEST(TCmd* pCmd, int pnum)
{
	TCmdQuest* cmd = (TCmdQuest*)pCmd;

	if (pnum != mypnum)
		SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
	gsDeltaData.ddJunkChanged = true;

	return sizeof(*cmd);
}

/**
 * Same as On_SYNCQUEST, but sends the message only to players on other levels.
 */
static unsigned On_SYNCQUESTEXT(TCmd* pCmd, int pnum)
{
	TCmdQuest* cmd = (TCmdQuest*)pCmd;

	if (currLvl._dLevelIdx != plr._pDunLevel || geBufferMsgs == MSG_INITIAL_PENDINGTURN)
		SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
	gsDeltaData.ddJunkChanged = true;

	return sizeof(*cmd);
}

#if DEV_MODE
static unsigned On_DUMP_MONSTERS(TCmd* pCmd, int pnum)
{
	int i, mnum;
	MonsterStruct* mon;

	for (i = 0; i < nummonsters; i++) {
		mnum = monstactive[i];
		mon = &monsters[mnum];

		LogErrorF("D-Mon", "idx:%d mnum:%d "
	"mo:%d "
	"sq:%d "
	"idx:%d "
	"pc:%d "
	"wh:%d "
	"g:%d "
	"gv1:%d "
	"gv2:%d "
	"gv3:%d "
	"x:%d "
	"y:%d "
	"fx:%d "
	"fy:%d "
	"ox:%d "
	"oy:%d "
	"xf:%d "
	"yf:%d "
	"dir:%d "
	"e:%d "
	"ex:%d "
	"ey:%d "
	"l:%d "
	"fl:%d "
	"ac:%d "
	"al:%d "
	"af:%d "
	"df:%d "
	"v1:%d "
	"v2:%d "
	"v3:%d "
	"v4:%d "
	"v5:%d "
	"v6:%d "
	"v7:%d "
	"v8:%d "
	"mhp:%d "
	"hp:%d "
	"lx:%d "
	"ly:%d "
	"rs:%d "
	"as:%d "
	"ut:%d "
	"tr:%d "
	"dv:%d "
	"mlid:%d "
	"le:%d "
	"lf:%d "
	"ps:%d "
	"CB:%d "
	"lvl:%d "
	"sf:%d "
	"ai:%d "
	"i:%d "
	"f:%d "
	"h:%d "
	"di:%d "
	"da:%d "
	"h2:%d "
	"di2:%d "
	"da2:%d "
	"g1:%d "
	"g2:%d "
	"ac:%d "
	"ev:%d "
	"af:%d "
	"af2:%d "
	"mr:%d "
	"tr:%d "
	"xp:%d "
	"ms:%d "
	"ty:%d "
	"w:%d "
	"xo:%d ",
			i,
			mnum,
			mon->_mmode,
	mon->_msquelch,
	mon->_mMTidx,
	mon->_mpathcount,
	mon->_mWhoHit,
	mon->_mgoal,
	mon->_mgoalvar1,
	mon->_mgoalvar2,
	mon->_mgoalvar3,
	mon->_mx,                // Tile X-position of monster
	mon->_my,                // Tile Y-position of monster
	mon->_mfutx,             // Future tile X-position of monster. Set at start of walking animation
	mon->_mfuty,             // Future tile Y-position of monster. Set at start of walking animation
	mon->_moldx,             // Most recent X-position in dMonster.
	mon->_moldy,             // Most recent Y-position in dMonster.
	mon->_mxoff,             // Monster sprite's pixel X-offset from tile.
	mon->_myoff,             // Monster sprite's pixel Y-offset from tile.
	mon->_mdir,              // Direction faced by monster (direction enum)
	mon->_menemy,            // The current target of the monster. An index in to either the plr or monster array based on the _meflag value.
	mon->_menemyx,          // X-coordinate of enemy (usually correspond's to the enemy's futx value)
	mon->_menemyy,          // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
	mon->_mListener,        // the player to whom the monster is talking to
	mon->_mAnimFrameLen, // Tick length of each frame in the current animation
	mon->_mAnimCnt,   // Increases by one each game tick, counting how close we are to _mAnimFrameLen
	mon->_mAnimLen,   // Number of frames in current animation
	mon->_mAnimFrame, // Current frame of animation.
	mon->_mDelFlag,
	mon->_mVar1,
	mon->_mVar2,
	mon->_mVar3, // Used to store the original mode of a stoned monster. Not 'thread' safe -> do not use for anything else! 
	mon->_mVar4,
	mon->_mVar5,
	mon->_mVar6, // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar7, // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar8, // Value used to measure progress for moving from one tile to another
	mon->_mmaxhp,
	mon->_mhitpoints,
	mon->_lastx, // the last known X-coordinate of the enemy
	mon->_lasty, // the last known Y-coordinate of the enemy
	mon->_mRndSeed,
	mon->_mAISeed,
	mon->_uniqtype,
	mon->_uniqtrans,
	mon->_udeadval,
	mon->mlid,
	mon->leader, // the leader of the monster
	mon->leaderflag, // the status of the monster's leader
	mon->packsize, // the number of 'pack'-monsters close to their leader
	mon->falign_CB,
	mon->_mLevel,
	mon->_mSelFlag,
	mon->_mAi,
	mon->_mInt,
	mon->_mFlags,
	mon->_mHit, // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	mon->_mMinDamage,
	mon->_mMaxDamage,
	mon->_mHit2, // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	mon->_mMinDamage2,
	mon->_mMaxDamage2,
	mon->_mMagic,
	mon->_mMagic2,     // unused
	mon->_mArmorClass, // AC+evasion: used against physical-hit (melee+projectile)
	mon->_mEvasion,    // evasion: used against magic-projectile
	mon->_mAFNum,
	mon->_mAFNum2,
	mon->_mMagicRes,
	mon->_mTreasure,
	mon->_mExp,
	mon->mtalkmsg,
	mon->_mType,
	mon->_mAnimWidth,
	mon->_mAnimXOffset);
	DMonsterStr* mstr = &gsDeltaData.ddLevel[myplr._pDunLevel].monster[i];
	if (mstr->_mCmd != DCMD_MON_INVALID) {
		LogErrorF("D-Mon", "delta ",
	"_mCmd:%d "
	"_mx:%d "
	"_my:%d "
	"_mdir:%d "
	"_mactive:%d "
	"_mhitpoints:%d "
	"_mWhoHit:%d ",

	mstr->_mCmd,
	mstr->_mx,
	mstr->_my,
	mstr->_mdir,
	mstr->_mactive,
	mstr->_mhitpoints,
	mstr->_mWhoHit);
	} else {
		LogErrorF("D-Mon", "delta _mCmd:0");
	}
	}

	return sizeof(*pCmd);
}

static const int ITEMCHECK_LEN = 4 + 2 + 2 + 1 + 1 + 1 + 1 + 4 + 4;
static BYTE* SendItem(ItemStruct* is, BYTE* dst)
{
	BYTE* dstStart = dst;

	*(BYTE*)dst = is->_itype;
	dst += sizeof(BYTE);

	*(INT*)dst = is->_iSeed;
	dst += sizeof(INT);
	
	*(WORD*)dst = is->_iIdx;
	dst += sizeof(WORD);
	*(WORD*)dst = is->_iCreateInfo;
	dst += sizeof(WORD);

	*(BYTE*)dst = is->_ix;
	dst += sizeof(BYTE);
	*(BYTE*)dst = is->_iy;
	dst += sizeof(BYTE);
	*(BOOLEAN*)dst = is->_iIdentified;
	dst += sizeof(BOOLEAN);

	*(INT*)dst = is->_iCharges;
	dst += sizeof(INT);
	//int _iDurability;
	*(INT*)dst = is->_ivalue;
	dst += sizeof(INT);

	assert((size_t)dst - (size_t)dstStart == ITEMCHECK_LEN);
	return dst;
}

static unsigned On_REQUEST_ITEMCHECK(TCmd* pCmd, int pnum)
{
	BYTE items[256];
	BYTE* buf = items;
	*buf = CMD_DO_ITEMCHECK;

	for (int i = 0; i < MAX_PLRS; i++) {
		buf = &items[1];
		*buf = i;
		buf++;
		*buf = 0;
		buf++;

		buf = SendItem(&plx(i)._pHoldItem, buf);
		for (int j = 0; j < NUM_INVLOC; j++)
			buf = SendItem(&plx(i)._pInvBody[j], buf);

		static_assert((NUM_INVLOC + 1) * ITEMCHECK_LEN + 3 <= UCHAR_MAX, "Body items dont fit into a single message.");
		//const int mm = (NUM_INVLOC + 1) * ITEMCHECK_LEN + 3;
		NetSendChunk(items, (NUM_INVLOC + 1) * ITEMCHECK_LEN + 3);

		buf = &items[1];
		buf++;
		*buf = 1;
		buf++;

		for (int j = 0; j < MAXBELTITEMS; j++)
			buf = SendItem(&plx(i)._pSpdList[j], buf);

		static_assert(MAXBELTITEMS * ITEMCHECK_LEN + 3 <= UCHAR_MAX, "Belt items dont fit into a single message");
		NetSendChunk(items, MAXBELTITEMS * ITEMCHECK_LEN + 3);

		static_assert(NUM_INV_GRID_ELEM == 40, "On_REQUEST_ITEMCHECK works with hardcoded item-grid");
		for (int k = 0; k < NUM_INV_GRID_ELEM / 10; k++) {
			buf = &items[1];
			buf++;
			*buf = 2 + k;
			buf++;

			for (int j = k * 10; j < k * 10 + 10; j++)
				buf = SendItem(&plx(i)._pInvList[j], buf);

			static_assert(10 * ITEMCHECK_LEN + 3 <= UCHAR_MAX, "One line of inventory items dont fit into a single message");
			NetSendChunk(items, 10 * ITEMCHECK_LEN + 3);
		}
	}

	return sizeof(*pCmd);
}

static void PrintItemMismatch(ItemStruct* is, const char* field, int myval, int extval, int sp, int pnum, int locId, int subloc)
{
	const char* loc = locId == 0 ? "belt" : (locId == 1 ? "body" : "inv");
	int row = locId >= 2 ? locId - 2 : 0;
	msg_errorf("%d received %s (%d vs. %d) from %d for plr%d %s item at %d:%d", mypnum, field, myval, extval, sp, pnum, loc, row, subloc);
}

static BYTE* CheckItem(ItemStruct* is, BYTE* src, int pnum, int loc, int subloc, int sp)
{
	BYTE* srcStart = src;
	bool placeholder, none;

	if (is->_itype != *(BYTE*)src) {
		PrintItemMismatch(is, "type", is->_itype, *(BYTE*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(BYTE);

	placeholder = is->_itype == ITYPE_PLACEHOLDER;
	none = is->_itype == ITYPE_NONE;

	if (!none && !placeholder && is->_iSeed != *(INT*)src) {
		PrintItemMismatch(is, "seed", is->_iSeed, *(INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);
	
	if (!none && !placeholder && is->_iIdx != *(WORD*)src) {
		PrintItemMismatch(is, "idx", is->_iIdx, *(WORD*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(WORD);
	if (!none && !placeholder && is->_iCreateInfo != *(WORD*)src) {
		PrintItemMismatch(is, "ci", is->_iCreateInfo, *(WORD*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(WORD);

	if (placeholder && is->_ix != *(BYTE*)src) {
		PrintItemMismatch(is, "x", is->_ix, *(BYTE*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(BYTE);
	//if (!none && !placeholder && is->_iy != *(BYTE*)src) {
	//	PrintItemMismatch(is, "x", is->_iy, *(BYTE*)src, sp, pnum, loc, subloc);
	//}
	src += sizeof(BYTE);
	if (!none && !placeholder && is->_iIdentified != *(BOOLEAN*)src) {
		PrintItemMismatch(is, "iden", is->_iIdentified, *(BOOLEAN*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(BOOLEAN);

	if (!none && !placeholder && is->_iCharges != *(INT*)src) {
		PrintItemMismatch(is, "chg", is->_iCharges, *(INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);
	//int _iDurability;
	if (!none && !placeholder && is->_ivalue != *(INT*)src) {
		PrintItemMismatch(is, "value", is->_ivalue, *(INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);

	assert((size_t)src - (size_t)srcStart == ITEMCHECK_LEN);
	return src;
}

static unsigned On_DO_ITEMCHECK(TCmd* pCmd, int pnum)
{
	BYTE* src = (BYTE*)pCmd;
	src++;
	int i = *src, k;
	src++;
	k = *src;
	src++;

//	LogErrorF("Item", "ItemCheck %d. for %d running data from %d.", k, i, pnum);

	switch (k) {
	case 0: // hold+body items
		src = CheckItem(&plx(i)._pHoldItem, src, i, 0, -1, pnum);
		for (int j = 0; j < NUM_INVLOC; j++) {
			src = CheckItem(&plx(i)._pInvBody[j], src, i, 0, j, pnum);
		}
		break;
	case 1: // belt items
		for (int j = 0; j < MAXBELTITEMS; j++) {
			src = CheckItem(&plx(i)._pSpdList[j], src, i, 1, j, pnum);
		}
		break;
	default: // inventory items
		k = k - 2;
		for (int j = k * 10; j < k * 10 + 10; j++) {
			src = CheckItem(&plx(i)._pInvList[j], src, i, k + 2, j % 10, pnum);
		}
		break;
	}

//	LogErrorF("Item", "ItemCheck done.");
	return (size_t)src - (size_t)pCmd;
}

#endif

#if DEBUG_MODE
static unsigned On_CHEAT_EXPERIENCE(TCmd* pCmd, int pnum)
{
	if (plr._pLevel < MAXCHARLEVEL) {
		plr._pExperience = plr._pNextExper;
		NextPlrLevel(pnum);
	}
	return sizeof(*pCmd);
}

static unsigned On_CHEAT_SPELL_LEVEL(TCmd* pCmd, int pnum)
{
	IncreasePlrSkillLvl(pnum, plr._pAltAtkSkill);
	return sizeof(*pCmd);
}

static unsigned On_DEBUG(TCmd* pCmd, int pnum)
{
	return sizeof(*pCmd);
}
#endif

static unsigned On_SETSHIELD(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;

	plr._pManaShield = cmd->bParam1;

	return sizeof(*cmd);
}

static unsigned On_REMSHIELD(TCmd* pCmd, int pnum)
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

static unsigned On_OPENSPIL(TCmd* pCmd, int pnum)
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

unsigned ParseMsg(int pnum, TCmd* pCmd)
{
	net_assert((unsigned)pnum < MAX_PLRS || pnum == SNPLAYER_MASTER);
//#ifndef NOHOSTING
	if ((unsigned)pnum >= MAX_PLRS && pnum != SNPLAYER_MASTER) {
//#else
//	if ((unsigned)pnum >= MAX_PLRS) {
//#endif
		dev_fatal("ParseMsg: illegal player %d", pnum);
	}
	switch (pCmd->bCmd) {
	case NMSG_SEND_GAME_DELTA:
		return On_SEND_GAME_DELTA(pCmd, pnum);
	case NMSG_PLRINFO:
		return On_PLRINFO(pCmd, pnum);
	case NMSG_DLEVEL_DATA:
	case NMSG_DLEVEL_SEP:
	case NMSG_DLEVEL_JUNK:
	case NMSG_DLEVEL_PLR:
	case NMSG_DLEVEL_END:
		return On_DLEVEL(pCmd, pnum);
	case NMSG_STRING:
		return On_STRING(pCmd, pnum);
	case NMSG_PLRDROP:
		return ON_PLRDROP(pCmd, pnum);
	}

	SNetDropPlayer(pnum);
	return 0;
}

unsigned ParseCmd(int pnum, TCmd* pCmd)
{
	net_assert((unsigned)pnum < MAX_PLRS);
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("ParseCmd: illegal player %d", pnum);
	}
	assert(geBufferMsgs != MSG_GAME_DELTA_LOAD && geBufferMsgs != MSG_GAME_DELTA_WAIT);
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
	case CMD_MONSTCORPSE:
		return On_MONSTCORPSE(pCmd, pnum);
	case CMD_AWAKEGOLEM:
		return On_AWAKEGOLEM(pCmd, pnum);
	case CMD_PLRDEAD:
		return On_PLRDEAD(pCmd, pnum);
	case CMD_PLRRESURRECT:
		return On_PLRRESURRECT(pCmd, pnum);
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
	case CMD_PUTITEM:
		return On_PUTITEM(pCmd, pnum);
	case CMD_RESPAWNITEM:
		return On_RESPAWNITEM(pCmd, pnum);
	case CMD_SPAWNITEM:
		return On_SPAWNITEM(pCmd, pnum);
	case CMD_GETITEM:
		return On_GETITEM(pCmd, pnum);
	case CMD_AUTOGETITEM:
		return On_AUTOGETITEM(pCmd, pnum);
	case CMD_GOTOGETITEM:
		return On_GOTOGETITEM(pCmd, pnum);
	case CMD_GOTOAGETITEM:
		return On_GOTOAGETITEM(pCmd, pnum);
	case CMD_OPERATEITEM:
		return On_OPERATEITEM(pCmd, pnum);
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
	case CMD_JOINLEVEL:
		return On_JOINLEVEL(pCmd, pnum);
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
#if DEV_MODE
	case CMD_DUMP_MONSTERS:
		return On_DUMP_MONSTERS(pCmd, pnum);
	case CMD_REQUEST_ITEMCHECK:
		return On_REQUEST_ITEMCHECK(pCmd, pnum);
	case CMD_DO_ITEMCHECK:
		return On_DO_ITEMCHECK(pCmd, pnum);
#endif
#if DEBUG_MODE
	case CMD_CHEAT_EXPERIENCE:
		return On_CHEAT_EXPERIENCE(pCmd, pnum);
	case CMD_CHEAT_SPELL_LEVEL:
		return On_CHEAT_SPELL_LEVEL(pCmd, pnum);
	case CMD_DEBUG:
		return On_DEBUG(pCmd, pnum);
#endif	
	}

	SNetDropPlayer(pnum);
	return NET_NORMAL_MSG_SIZE;
}

DEVILUTION_END_NAMESPACE
