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
	int i, j;
	BYTE mask;

	static_assert(MAX_PLRS < 8, "msg_mask_monhit uses BYTE mask for pnum.");
	mask = ~(1 << pnum);
	for (i = 0; i < NUM_LEVELS; i++) {
		for (j = 0; j < MAXMONSTERS; j++) {
			gsDeltaData.ddLevel[i].monster[j]._mWhoHit &= mask;
		}
	}
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

void DeltaExportData(int pnum)
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
	deltaEnd.turn = SDL_SwapLE32(gdwLastGameTurn);
	assert(gdwLastGameTurn * gbNetUpdateRate == gdwGameLogicTurn);
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
	net_assert((gsDeltaData.ddSendRecvOffset + cmd->wBytes) <= sizeof(gsDeltaData.ddSendRecvBuf));
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

static void delta_monster_corpse(const TCmdBParam2* pCmd)
{
	BYTE bLevel;
	DMonsterStr* mon;

	if (!IsMultiGame)
		return;

	bLevel = pCmd->bParam1;
	net_assert(bLevel < NUM_LEVELS);
	// commented out, because _mCmd must be already set at this point
	//gsDeltaData.ddLevelChanged[bLevel] = true;
	net_assert(pCmd->bParam2 < MAXMONSTERS);
	mon = &gsDeltaData.ddLevel[bLevel].monster[pCmd->bParam2];
	if (mon->_mCmd == DCMD_MON_DEAD)
		mon->_mCmd = DCMD_MON_DESTROYED;
}

static void delta_monster_summon(const TCmdMonstSummon* pCmd)
{
	BYTE bLevel;
	DMonsterStr* mon;

	if (!IsMultiGame)
		return;

	bLevel = pCmd->mnParam1.bParam1;
	net_assert(bLevel < NUM_LEVELS);
#if HELLFIRE
	net_assert(bLevel == SL_SKELKING || bLevel == DLV_NEST3);
#else
	net_assert(bLevel == SL_SKELKING);
#endif
	gsDeltaData.ddLevelChanged[bLevel] = true;
	net_assert(pCmd->mnMnum >= MAX_MINIONS && pCmd->mnMnum < MAXMONSTERS);
	mon = &gsDeltaData.ddLevel[bLevel].monster[pCmd->mnMnum];
	if (mon->_mCmd == DCMD_MON_ACTIVE)
		return;
	assert(mon->_mCmd == DCMD_MON_DEAD || mon->_mCmd == DCMD_MON_DESTROYED || mon->_mCmd == DCMD_MON_INVALID);
	mon->_mx = pCmd->mnParam1.x;
	mon->_my = pCmd->mnParam1.y;
	mon->_mdir = pCmd->mnDir;
	mon->_mSIdx = pCmd->mnSIdx + 1;
	mon->_mCmd = DCMD_MON_ACTIVE;
	mon->_mWhoHit = 0;
	mon->_mactive = 0;
	mon->_mleaderflag = MLEADER_NONE;
	mon->_mhitpoints = pCmd->mnMaxHp;
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
	net_assert(mnum < MAXMONSTERS);

	gsDeltaData.ddLevelChanged[bLevel] = true;
	pD = &gsDeltaData.ddLevel[bLevel].monster[mnum];
	static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "delta_kill_monster expects ordered DCMD_MON_ enum I.");
	static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "delta_kill_monster expects ordered DCMD_MON_ enum II.");
	if (pD->_mCmd >= DCMD_MON_DEAD)
		return 0;
	pD->_mCmd = mon->mkDir < NUM_DIRS ? DCMD_MON_DEAD : DCMD_MON_DESTROYED;
	pD->_mx = mon->mkParam1.x;
	pD->_my = mon->mkParam1.y;
	pD->_mdir = mon->mkDir;
	pD->_mleaderflag = MLEADER_NONE; // TODO: reset leaderflag of the minions if the info is available
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
	net_assert(SwapLE16(mon->mdMnum) < MAXMONSTERS);

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
		net_assert(pSync->_mndx < MAXMONSTERS);
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
	net_assert(wLen == 0);
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
	net_assert(oi < MAXOBJECTS);

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
		static_assert(MAXCHARLEVEL < (1 << 6), "PackPkItem stores the player level of ears in 6 bits.");
		dest->wValue = SwapLE16(src->_ivalue | (src->_iName[18] << 8) | ((src->_iCurs - ICURS_EAR_SORCERER) << 6));
		dest->dwBuff = SwapLE32(*(DWORD*)&src->_iName[19]);
	}
}

void DeltaAddItem(int ii)
{
	ItemStruct* is;
	// commented out to have a complete sync with other players
	//int i;
	//DItemStr* pD;

	//if (!IsMultiGame)
	//	return;

	is = &items[ii];
	is->_iFloorFlag = TRUE;
	/*pD = gsDeltaData.ddLevel[currLvl._dLevelIdx].item;
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
	}*/
}

void DeltaSaveLevel()
{
	assert(IsMultiGame);
	delta_leave_sync(currLvl._dLevelIdx);
}

static void UnPackEar(const PkItemStruct* src)
{
	static_assert(sizeof(items[MAXITEMS]._iName) >= sizeof("Ear of ") + 16, "UnPackEar might write too much data to _iName.");
	char* cursor = &items[MAXITEMS]._iName[sizeof("Ear of ") - 1];

	*(WORD*)&cursor[0] = SwapLE16(src->wCI);
	*(DWORD*)&cursor[2] = SwapLE32(src->dwSeed);
	cursor[6] = src->bId;
	cursor[7] = src->bDur;
	cursor[8] = src->bMDur;
	cursor[9] = src->bCh;
	cursor[10] = src->bMCh;
	cursor[11] = SwapLE16(src->wValue) >> 8;
	*(DWORD*)&cursor[12] = SwapLE32(src->dwBuff);
	cursor[16] = '\0';
	items[MAXITEMS]._iCurs = ((SwapLE16(src->wValue) >> 6) & 3) + ICURS_EAR_SORCERER;
	items[MAXITEMS]._ivalue = SwapLE16(src->wValue) & 0x3F;
	items[MAXITEMS]._iCreateInfo = SwapLE16(*(WORD*)&items[MAXITEMS]._iName[7]);
	items[MAXITEMS]._iSeed = SwapLE32(*(DWORD*)&items[MAXITEMS]._iName[9]);
}

void UnPackPkItem(const PkItemStruct* src)
{
	uint16_t idx = SwapLE16(src->wIndx);
	uint16_t value;

	net_assert(idx < NUM_IDI);
	if (idx != IDI_EAR) {
		net_assert(((SwapLE16(src->wCI) & CF_TOWN) >> 8) <= CFL_CRAFTED);
		RecreateItem(
			SwapLE32(src->dwSeed),
			SwapLE16(src->wIndx),
			SwapLE16(src->wCI));
		net_assert(items[MAXITEMS]._iMaxCharges >= src->bMCh);
		net_assert(src->bCh <= src->bMCh);
		net_assert(items[MAXITEMS]._iMaxDur >= src->bMDur);
		net_assert(src->bDur <= src->bMDur);
		if (items[MAXITEMS]._iClass == ICLASS_ARMOR || items[MAXITEMS]._iClass == ICLASS_WEAPON) {
			net_assert(src->bDur != 0);
		}
		if (idx == IDI_GOLD) {
			value = SwapLE16(src->wValue);
			net_assert(value <= GOLD_MAX_LIMIT);
			SetGoldItemValue(&items[MAXITEMS], value);
		}
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
	if (prevFlag == MLEADER_NONE) {
		// leaderflag out-of-sync because minions might be away when the leader dies
		assert(newFlag != MLEADER_SELF);
		return;
	}
	assert(prevFlag == MLEADER_PRESENT && newFlag == MLEADER_AWAY);
	monsters[mnum].leaderflag = newFlag;
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
	bool monInGame;

	assert(IsMultiGame);

	deltaload = true;
	if (currLvl._dLevelIdx != DLV_TOWN) {
		for (i = 0; i < MAX_MINIONS; i++)
			InitGolemStats(i, gsDeltaData.ddJunk.jGolems[i]);

		mstr = gsDeltaData.ddLevel[currLvl._dLevelIdx].monster;
		for (i = 0; i < MAXMONSTERS; i++, mstr++) {
			if (mstr->_mCmd != DCMD_MON_INVALID) {
				mon = &monsters[i];
				monInGame = mon->_mmode <= MM_INGAME_LAST;
				// skip minions and prespawn skeletons
				if (monInGame)
					RemoveMonFromMap(i);
				x = mstr->_mx;
				y = mstr->_my;
				SetMonsterLoc(mon, x, y);
				mon->_mdir = mstr->_mdir;
				UpdateLeader(i, mon->leaderflag, mstr->_mleaderflag);
				if (mstr->_mSIdx != 0) {
					net_assert(mstr->_mSIdx <= nummtypes);
					assert(mon->mlid == NO_LIGHT);
					// TODO: InitSummonedMonster ?
					SetRndSeed(glSeedTbl[i % NUM_LEVELS]);
					InitMonster(i, mon->_mdir, mstr->_mSIdx - 1, mon->_mx, mon->_my);
					mon->_mTreasure = NO_DROP;
					mon->_mFlags |= MFLAG_NOCORPSE;
					if (!monInGame)
						nummonsters++;
				}
				// set hitpoints for dead monsters as well to ensure sync in multiplayer
				// games even on the first game_logic run
				mon->_mhitpoints = SwapLE32(mstr->_mhitpoints);
				// SyncMonsterLight: inline for better performance + apply to moving monsters
				if (mon->mlid != NO_LIGHT)
					ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
				static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "DeltaLoadLevel expects ordered DCMD_MON_ enum I.");
				static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "DeltaLoadLevel expects ordered DCMD_MON_ enum II.");
				if (mstr->_mCmd >= DCMD_MON_DEAD) {
					if (mstr->_mCmd != DCMD_MON_DESTROYED)
						MonAddDead(i);
					// assert(mon->_mhitpoints == 0);
					// TODO: RemoveMonFromGame ?
					// reset squelch value to simplify MonFallenFear, sync_all_monsters and LevelDeltaExport
					mon->_msquelch = 0;
					mon->_mmode = i >= MAX_MINIONS ? ((mstr->_mCmd == DCMD_MON_DESTROYED || (mon->_mFlags & MFLAG_NOCORPSE)) ? MM_UNUSED : MM_DEAD) : MM_RESERVED;
					if (i >= MAX_MINIONS)
						nummonsters--;
					// SyncMonsterAnim(mnum);
					mon->_mAnimFrame = mon->_mAnims[MA_DEATH].aFrames;
					// mon->_mAnimCnt = -1;
					mon->_mAnimData = mon->_mAnims[MA_DEATH].aData[mon->_mdir];
				} else {
					mon->_msquelch = mstr->_mactive;
					mon->_mWhoHit = mstr->_mWhoHit;
					dMonster[mon->_mx][mon->_my] = i + 1;
					// SyncMonsterAnim(mnum);
					assert(mon->_mmode == MM_STAND);
					mon->_mAnimData = mon->_mAnims[MA_STAND].aData[mon->_mdir];
				}
			}
		}
		// ensure the level of Diablo is empty after she is defeated
		if (currLvl._dLevelIdx == DLV_HELL4 && quests[Q_DIABLO]._qactive == QUEST_DONE) {
			for (i = 0; i < MAXMONSTERS; i++) {
				mon = &monsters[i];
				monInGame = mon->_mmode <= MM_INGAME_LAST;
				if (monInGame) {
					RemoveMonFromMap(i);
					mon->_mmode = i >= MAX_MINIONS ? MM_UNUSED : MM_RESERVED;
				}
			}
			nummonsters = MAX_MINIONS;
		}
		// SyncMonsterLight();

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
	int i;
	ItemStruct* is;

	cmd.bCmd = CMD_JOINLEVEL;
	cmd.lLevel = myplr._pDunLevel;
	cmd.px = ViewX;
	cmd.py = ViewY;
	cmd.php = SwapLE32(myplr._pHPBase);
	cmd.pmp = SwapLE32(myplr._pManaBase);
	cmd.lTimer1 = SwapLE16(myplr._pTimer[PLTR_INFRAVISION]);
	cmd.lTimer2 = SwapLE16(myplr._pTimer[PLTR_RAGE]);
	cmd.pManaShield = myplr._pManaShield;

	for (i = 0; i < NUM_INVELEM; i++) {
		is = PlrItem(mypnum, i);
		cmd.itemsDur[i] = is->_iDurability;
	}
	cmd.itemsDur[NUM_INVELEM] = myplr._pHoldItem._iDurability;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void LevelDeltaExport()
{
	LDLevel* lvlData;

	int pnum, mnum, i, mi;
	ItemStruct* is;
	MonsterStruct* mon;
	MissileStruct* mis;
	bool validDelta, completeDelta;
	BYTE* dst;
	unsigned recipients = 0;

	validDelta = currLvl._dLevelIdx != DLV_INVALID;
	completeDelta = false;
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!(guSendLevelData & (1 << pnum)) || // pnum did not request a level-delta
//		  (guReceivedLevelDelta & (1 << pnum)) ||  // got an (empty) level delta from pnum
		  (!validDelta && !myplr._pLvlChanging && // both players are 'actively' loading
		   plr._pDunLevel == myplr._pDunLevel &&  // the same level ->
			(guRequestLevelData[pnum] > guRequestLevelData[mypnum] || (guRequestLevelData[pnum] == guRequestLevelData[mypnum] && pnum > mypnum)))) { // ignore lower priority requests 	TODO: overflow hickup
			; // skip
		} else {
			guSendLevelData &= ~(1 << pnum);
			completeDelta |= pnum != mypnum && plr._pDunLevel == myplr._pDunLevel;
			recipients |= (1 << pnum);
		}
	}
	completeDelta &= validDelta;
	// send detailed level delta only if available and appropriate
	if (completeDelta) {
		// can not be done during lvl-delta load, otherwise a separate buffer should be used
		assert(geBufferMsgs != MSG_LVL_DELTA_WAIT && geBufferMsgs != MSG_LVL_DELTA_PROC);
		static_assert(sizeof(LDLevel) <= sizeof(gsDeltaData.ddSendRecvBuf.content), "Level-Delta does not fit to the buffer.");
		lvlData = (LDLevel*)&gsDeltaData.ddSendRecvBuf.content[0];

		static_assert(MAXMONSTERS <= UCHAR_MAX, "Monster indices are transferred as BYTEs I.");
		lvlData->ldNumMonsters = nummonsters;

		static_assert(MAXMISSILES <= UCHAR_MAX, "Missile indices are transferred as BYTEs I.");
		for (i = 0; i < MAXMISSILES; i++) {
			lvlData->ldMissActive[i] = missileactive[i];
		}

		dst = &lvlData->ldContent[0];
		// export the players
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			TSyncLvlPlayer* __restrict tplr = (TSyncLvlPlayer*)dst;
			if (!plr._pActive || plr._pDunLevel != currLvl._dLevelIdx || plr._pLvlChanging) {
				tplr->spMode = PM_INVALID;
				dst++;
				continue;
			}

			tplr->spMode = plr._pmode;
			static_assert(sizeof(plr.walkpath[0]) == 1, "LevelDeltaExport exports walkpath as a bytestream.");
			static_assert(((MAX_PATH_LENGTH + 1) % 4 == 0) && (offsetof(TSyncLvlPlayer, spWalkpath) + MAX_PATH_LENGTH + 1) <= sizeof(TSyncLvlPlayer), "LevelDeltaExport uses DWORD-memcpy to optimize performance.");
			memcpy(tplr->spWalkpath, plr.walkpath, MAX_PATH_LENGTH + 1);
			tplr->spManaShield = plr._pManaShield;
			tplr->spInvincible = plr._pInvincible;
			tplr->spDestAction = plr.destAction;
			tplr->spDestParam1 = SwapLE32(plr.destParam1);
			tplr->spDestParam2 = SwapLE32(plr.destParam2);
			tplr->spDestParam3 = SwapLE32(plr.destParam3);
			tplr->spDestParam4 = SwapLE32(plr.destParam4);
			tplr->spTimer[PLTR_INFRAVISION] = SwapLE16(plr._pTimer[PLTR_INFRAVISION]);
			tplr->spTimer[PLTR_RAGE] = SwapLE16(plr._pTimer[PLTR_RAGE]);
			tplr->spx = plr._px;
			tplr->spy = plr._py;
			tplr->spfutx = plr._pfutx;
			tplr->spfuty = plr._pfuty;
			tplr->spoldx = plr._poldx;
			tplr->spoldy = plr._poldy;
//	INT spxoff;   // Player sprite's pixel X-offset from tile.
//	INT spyoff;   // Player sprite's pixel Y-offset from tile.
			tplr->spdir = plr._pdir;
			tplr->spAnimFrame = plr._pAnimFrame;
			tplr->spAnimCnt = plr._pAnimCnt;
			tplr->spHPBase = SwapLE32(plr._pHPBase);
			tplr->spManaBase = SwapLE32(plr._pManaBase);
			tplr->spVar1 = SwapLE32(plr._pVar1);
			tplr->spVar2 = SwapLE32(plr._pVar2);
			tplr->spVar3 = SwapLE32(plr._pVar3);
			tplr->spVar4 = SwapLE32(plr._pVar4);
			tplr->spVar5 = SwapLE32(plr._pVar5);
			tplr->spVar6 = SwapLE32(plr._pVar6);
			tplr->spVar7 = SwapLE32(plr._pVar7);
			tplr->spVar8 = SwapLE32(plr._pVar8);
			tplr->bItemsDur = 0;
			dst += sizeof(TSyncLvlPlayer);

			// sync durabilities
			for (i = 0; i <= NUM_INVELEM; i++) {
				if (i == NUM_INVELEM)
					is = &plr._pHoldItem;
				else
					is = PlrItem(pnum, i);
				if (ITYPE_DURABLE(is->_itype)) {
					*dst = i;
					dst++;
					*dst = is->_iDurability;
					dst++;
					tplr->bItemsDur++;
				}
			}
		}
		// export the monsters
		for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
			mon = &monsters[mnum];
			if (mon->_msquelch == 0) {
				continue;	// assume it is the same as in delta
			}
			net_assert(mon->_mmode <= MM_INGAME_LAST);
			TSyncLvlMonster* __restrict tmon = (TSyncLvlMonster*)dst;
			tmon->smMnum = SwapLE16(mnum);
			tmon->smMode = mon->_mmode;
			tmon->smSquelch = SwapLE32(mon->_msquelch);
			tmon->smPathcount = mon->_mpathcount; // unused
			tmon->smWhoHit = mon->_mWhoHit;
			tmon->smGoal = mon->_mgoal;
			tmon->smGoalvar1 = SwapLE32(mon->_mgoalvar1);
			tmon->smGoalvar2 = SwapLE32(mon->_mgoalvar2);
			tmon->smGoalvar3 = SwapLE32(mon->_mgoalvar3);
			tmon->smx = mon->_mx;                // Tile X-position of monster
			tmon->smy = mon->_my;                // Tile Y-position of monster
			tmon->smfutx = mon->_mfutx;             // Future tile X-position of monster. Set at start of walking animation
			tmon->smfuty = mon->_mfuty;             // Future tile Y-position of monster. Set at start of walking animation
			tmon->smoldx = mon->_moldx;             // Most recent X-position in dMonster.
			tmon->smoldy = mon->_moldy;             // Most recent Y-position in dMonster.
//	tmon->_mxoff;             // Monster sprite's pixel X-offset from tile.
//	tmon->_myoff;             // Monster sprite's pixel Y-offset from tile.
			tmon->smdir = mon->_mdir;              // Direction faced by monster (direction enum)
			tmon->smEnemy = SwapLE32(mon->_menemy);            // The current target of the monster. An index in to either the plr or monster array based on the _meflag value.
			tmon->smEnemyx = mon->_menemyx;          // X-coordinate of enemy (usually correspond's to the enemy's futx value)
			tmon->smEnemyy = mon->_menemyy;          // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
			tmon->smListener = mon->_mListener;        // the player to whom the monster is talking to
			tmon->smAnimCnt = mon->_mAnimCnt;   // Increases by one each game tick, counting how close we are to _mAnimFrameLen
			tmon->smAnimFrame = mon->_mAnimFrame; // Current frame of animation.
			// assert(!mon->_mDelFlag || mon->_mmode == MM_STONE);
			tmon->smDelFlag = mon->_mDelFlag; // unused
			tmon->smVar1 = SwapLE32(mon->_mVar1);
			tmon->smVar2 = SwapLE32(mon->_mVar2);
			tmon->smVar3 = SwapLE32(mon->_mVar3);
			tmon->smVar4 = SwapLE32(mon->_mVar4);
			tmon->smVar5 = SwapLE32(mon->_mVar5);
			tmon->smVar6 = SwapLE32(mon->_mVar6); // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
			tmon->smVar7 = SwapLE32(mon->_mVar7); // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
			tmon->smVar8 = SwapLE32(mon->_mVar8); // Value used to measure progress for moving from one tile to another
			tmon->smHitpoints = SwapLE32(mon->_mhitpoints);
			tmon->smLastx = mon->_lastx; // the last known X-coordinate of the enemy
			tmon->smLasty = mon->_lasty; // the last known Y-coordinate of the enemy
			//tmon->smLeader = mon->leader; // the leader of the monster
			tmon->smLeaderflag = mon->leaderflag; // the status of the monster's leader
			//tmon->smPacksize = mon->packsize; // the number of 'pack'-monsters close to their leader
	//BYTE falign_CB;
			tmon->smFlags = SwapLE32(mon->_mFlags);

			dst += sizeof(TSyncLvlMonster);
		}
		// export the missiles
		for (i = 0; i < nummissiles; i++) {
			mi = missileactive[i];
			mis = &missile[mi];

			//assert(!mis->_miDelFlag);
			TSyncLvlMissile* __restrict tmis = (TSyncLvlMissile*)dst;
			tmis->smiMi = SwapLE16(mi + MAXMONSTERS);
			tmis->smiType = mis->_miType;   // Type of projectile (MIS_*)
			//BYTE _miSubType; // unused
			tmis->smiAnimType = mis->_miAnimType;
			//BOOL _miAnimFlag;
			tmis->smiAnimCnt = mis->_miAnimCnt; // Increases by one each game tick, counting how close we are to _miAnimFrameLen
			tmis->smiAnimAdd = mis->_miAnimAdd;
			tmis->smiAnimFrame = mis->_miAnimFrame; // Current frame of animation.
			tmis->smiDrawFlag = mis->_miDrawFlag;
			tmis->smiLightFlag = mis->_miLightFlag;
			tmis->smiPreFlag = mis->_miPreFlag;
			tmis->smiUniqTrans = mis->_miUniqTrans;
			tmis->smisx = mis->_misx;    // Initial tile X-position for missile
			tmis->smisy = mis->_misy;    // Initial tile Y-position for missile
			tmis->smix = mis->_mix;     // Tile X-position of the missile
			tmis->smiy = mis->_miy;     // Tile Y-position of the missile
			tmis->smixoff = SwapLE32(mis->_mixoff);  // Sprite pixel X-offset for the missile
			tmis->smiyoff = SwapLE32(mis->_miyoff);  // Sprite pixel Y-offset for the missile
			tmis->smixvel = SwapLE32(mis->_mixvel);  // Missile tile X-velocity while walking. This gets added onto _mitxoff each game tick
			tmis->smiyvel = SwapLE32(mis->_miyvel);  // Missile tile Y-velocity while walking. This gets added onto _mitxoff each game tick
			tmis->smitxoff = SwapLE32(mis->_mitxoff); // How far the missile has travelled in its lifespan along the X-axis. mix/miy/mxoff/myoff get updated every game tick based on this
			tmis->smityoff = SwapLE32(mis->_mityoff); // How far the missile has travelled in its lifespan along the Y-axis. mix/miy/mxoff/myoff get updated every game tick based on this
			tmis->smiDir = mis->_miDir;   // The direction of the missile
			tmis->smiSpllvl = SwapLE32(mis->_miSpllvl); // int?
			tmis->smiSource = SwapLE32(mis->_miSource); // int?
			tmis->smiCaster = SwapLE32(mis->_miCaster); // int?
			tmis->smiMinDam = SwapLE32(mis->_miMinDam);
			tmis->smiMaxDam = SwapLE32(mis->_miMaxDam);
			// tmis->smiRndSeed = mis->_miRndSeed;
			tmis->smiRange = SwapLE32(mis->_miRange);
			tmis->smiLidRadius = mis->_miLid == NO_LIGHT ? 0 : LightList[mis->_miLid]._lradius;
			tmis->smiVar1 = SwapLE32(mis->_miVar1);
			tmis->smiVar2 = SwapLE32(mis->_miVar2);
			tmis->smiVar3 = SwapLE32(mis->_miVar3);
			tmis->smiVar4 = SwapLE32(mis->_miVar4);
			tmis->smiVar5 = SwapLE32(mis->_miVar5);
			tmis->smiVar6 = SwapLE32(mis->_miVar6);
			tmis->smiVar7 = SwapLE32(mis->_miVar7);
			tmis->smiVar8 = SwapLE32(mis->_miVar8);

			dst += sizeof(TSyncLvlMissile);
		}

		lvlData->wLen = SwapLE16((size_t)dst - (size_t)&lvlData->ldContent[0]);
		// send the data to the recipients
		DWORD size = /*Level*/DeltaCompressData(dst);
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (recipients & (1 << pnum)) {
				dthread_send_delta(pnum, NMSG_LVL_DELTA, &gsDeltaData.ddSendRecvBuf, size);
			}
		}
	}
	// current number of chunks sent + level + turn-id + end
	LevelDeltaEnd deltaEnd;
	deltaEnd.compressed = FALSE;
	deltaEnd.numChunks = completeDelta ? 1 : 0;
	deltaEnd.level = currLvl._dLevelIdx;
	deltaEnd.turn = SDL_SwapLE32(gdwLastGameTurn);
	for (pnum = 0; pnum < MAX_PLRS; pnum++, recipients >>= 1) {
		if (recipients & 1) {
			dthread_send_delta(pnum, NMSG_LVL_DELTA_END, &deltaEnd, sizeof(deltaEnd));
		}
	}
}

void LevelDeltaLoad()
{
	int pnum, i, mnum, mi;
	ItemStruct* is;
	MonsterStruct* mon;
	MissileStruct* mis;
	const MissileData* mds;
	BYTE* src;
	WORD wLen;
	LDLevel* lvlData;

	geBufferMsgs = MSG_NORMAL;
	assert(currLvl._dLevelIdx == DLV_INVALID);
	currLvl._dLevelIdx = myplr._pDunLevel;

	// assert(IsMultiGame);
	ResyncQuests();
	DeltaLoadLevel();
	//SyncPortals();
	// reset squelch set from delta, the message should contain more up-to-date info
	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		monsters[mnum]._msquelch = 0;
	}

	lvlData = (LDLevel*)&gsDeltaData.ddSendRecvBuf.content[0];
	// load static metadata
	static_assert(MAXMONSTERS <= UCHAR_MAX, "Monster indices are transferred as BYTEs II.");
	assert(nummonsters <= lvlData->ldNumMonsters);
	nummonsters = lvlData->ldNumMonsters;
	net_assert(nummonsters <= MAXMONSTERS);

	static_assert(MAXMISSILES <= UCHAR_MAX, "Missile indices are transferred as BYTEs II.");
	for (mi = 0; mi < MAXMISSILES; mi++) {
		net_assert(lvlData->ldMissActive[mi] < MAXMISSILES);
		missileactive[mi] = lvlData->ldMissActive[mi];
	}

	src = &lvlData->ldContent[0];
	// load players
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		TSyncLvlPlayer* __restrict tplr = (TSyncLvlPlayer*)src;
		if (tplr->spMode == PM_INVALID) {
			src++;
			continue;
		}
		if (pnum == mypnum) {
			net_assert(tplr->spMode == PM_STAND ||
				((tplr->spMode == PM_DEATH || tplr->spMode == PM_DYING) && plr._pHitPoints < (1 << 6)));
			net_assert(tplr->spWalkpath[0] == DIR_NONE);
			net_assert(tplr->spDestAction == ACTION_NONE);
			net_assert(tplr->spInvincible == 40);
			net_assert(plr._pTimer[PLTR_INFRAVISION] == SwapLE16(tplr->spTimer[PLTR_INFRAVISION]));
			net_assert(plr._pTimer[PLTR_RAGE] == SwapLE16(tplr->spTimer[PLTR_RAGE]));
			net_assert(plr._pManaShield == tplr->spManaShield);
			net_assert(plr._pHPBase == SwapLE32(tplr->spHPBase) || (plr._pHitPoints < (1 << 6) && currLvl._dLevelIdx == DLV_TOWN));
			net_assert(plr._pManaBase == SwapLE32(tplr->spManaBase));
		}
		// RemovePlrFromMap(pnum);
		net_assert((unsigned)plr._px < MAXDUNX);
		net_assert((unsigned)plr._py < MAXDUNY);
		if (dPlayer[plr._px][plr._py] == pnum + 1)
			dPlayer[plr._px][plr._py] = 0;
		net_assert(tplr->spMode < NUM_PLR_MODES);
		plr._pmode = tplr->spMode;
		static_assert(sizeof(tplr->spWalkpath[0]) == 1, "LevelDeltaLoad imports walkpath as a bytestream.");
		memcpy(plr.walkpath, tplr->spWalkpath, MAX_PATH_LENGTH);
		plr._pManaShield = tplr->spManaShield;
		plr._pInvincible = tplr->spInvincible;
		plr.destAction = tplr->spDestAction;
		plr.destParam1 = SwapLE32(tplr->spDestParam1);
		plr.destParam2 = SwapLE32(tplr->spDestParam2);
		plr.destParam3 = SwapLE32(tplr->spDestParam3);
		plr.destParam4 = SwapLE32(tplr->spDestParam4);
		plr._pTimer[PLTR_INFRAVISION] = SwapLE16(tplr->spTimer[PLTR_INFRAVISION]);
		plr._pTimer[PLTR_RAGE] = SwapLE16(tplr->spTimer[PLTR_RAGE]);
		plr._px = tplr->spx;
		plr._py = tplr->spy;
		plr._pfutx = tplr->spfutx;
		plr._pfuty = tplr->spfuty;
		plr._poldx = tplr->spoldx;
		plr._poldy = tplr->spoldy;
//	INT spxoff;   // Player sprite's pixel X-offset from tile.
//	INT spyoff;   // Player sprite's pixel Y-offset from tile.
		plr._pxoff = plr._pyoff = 0; // no need to sync these values as they are recalculated when used
		plr._pdir = tplr->spdir;
		plr._pAnimFrame = tplr->spAnimFrame;
		plr._pAnimCnt = tplr->spAnimCnt;
		plr._pHPBase = SwapLE32(tplr->spHPBase);
		plr._pManaBase = SwapLE32(tplr->spManaBase);
		plr._pVar1 = SwapLE32(tplr->spVar1);
		plr._pVar2 = SwapLE32(tplr->spVar2);
		plr._pVar3 = SwapLE32(tplr->spVar3);
		plr._pVar4 = SwapLE32(tplr->spVar4);
		plr._pVar5 = SwapLE32(tplr->spVar5);
		plr._pVar6 = SwapLE32(tplr->spVar6);
		plr._pVar7 = SwapLE32(tplr->spVar7);
		plr._pVar8 = SwapLE32(tplr->spVar8);

		src += sizeof(TSyncLvlPlayer);

		// sync durabilities
		for (i = tplr->bItemsDur; i > 0; i--) {
			if (*src == NUM_INVELEM)
				is = &plr._pHoldItem;
			else
				is = PlrItem(pnum, *src);
			net_assert(ITYPE_DURABLE(is->_itype));
			src++;
			if (pnum == mypnum) {
				net_assert(is->_iDurability == *src);
			} else {
				net_assert(is->_iMaxDur >= *src);
				net_assert(*src != 0);
			}
			is->_iDurability = *src;
			src++;
		}
		// validate data
		for (i = 0; i < MAX_PATH_LENGTH; i++) {
			if (plr.walkpath[i] == DIR_NONE)
				break;
			net_assert(plr.walkpath[i] < NUM_DIRS);
		}
		net_assert(plr._pdir < NUM_DIRS);
		// net_assert((unsigned)plr._px < MAXDUNX);
		// net_assert((unsigned)plr._py < MAXDUNY);
		net_assert((unsigned)plr._pfutx < MAXDUNX);
		net_assert((unsigned)plr._pfuty < MAXDUNY);
		net_assert((unsigned)plr._poldx < MAXDUNX);
		net_assert((unsigned)plr._poldy < MAXDUNY);

		InitLvlPlayer(pnum, false);
	}

	wLen = SwapLE16(lvlData->wLen);
	wLen -= ((size_t)src - size_t(&lvlData->ldContent[0]));
	// load monsters
	for ( ; wLen >= sizeof(TSyncLvlMonster); wLen -= sizeof(TSyncLvlMonster)) {
		TSyncLvlMonster* __restrict tmon = (TSyncLvlMonster*)src;
		mnum = SwapLE16(tmon->smMnum);
		if (mnum >= MAXMONSTERS)
			break;

		mon = &monsters[mnum];
		// RemoveMonFromMap(mnum);
		if (dMonster[mon->_mx][mon->_my] == mnum + 1)
			dMonster[mon->_mx][mon->_my] = 0;

		UpdateLeader(mnum, mon->leaderflag, tmon->smLeaderflag);

		net_assert(tmon->smMode <= MM_INGAME_LAST);
		mon->_mmode = tmon->smMode;
		mon->_msquelch = SwapLE32(tmon->smSquelch);
		mon->_mpathcount = tmon->smPathcount;
		mon->_mWhoHit = tmon->smWhoHit;
		mon->_mgoal = tmon->smGoal;
		mon->_mgoalvar1 = SwapLE32(tmon->smGoalvar1);
		mon->_mgoalvar2 = SwapLE32(tmon->smGoalvar2);
		mon->_mgoalvar3 = SwapLE32(tmon->smGoalvar3);
		mon->_mx = tmon->smx;                // Tile X-position of monster
		mon->_my = tmon->smy;                // Tile Y-position of monster
		mon->_mfutx = tmon->smfutx;             // Future tile X-position of monster. Set at start of walking animation
		mon->_mfuty = tmon->smfuty;             // Future tile Y-position of monster. Set at start of walking animation
		mon->_moldx = tmon->smoldx;             // Most recent X-position in dMonster.
		mon->_moldy = tmon->smoldy;             // Most recent Y-position in dMonster.
//	tmon->_mxoff;             // Monster sprite's pixel X-offset from tile.
//	tmon->_myoff;             // Monster sprite's pixel Y-offset from tile.
		mon->_mxoff = mon->_myoff = 0; // no need to sync these values as they are recalculated when used
		mon->_mdir = tmon->smdir;              // Direction faced by monster (direction enum)
		mon->_menemy = SwapLE32(tmon->smEnemy);            // The current target of the monster. An index in to either the plr or monster array based on the _meflag value.
		mon->_menemyx = tmon->smEnemyx;          // X-coordinate of enemy (usually correspond's to the enemy's futx value)
		mon->_menemyy = tmon->smEnemyy;          // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
		mon->_mListener = tmon->smListener;        // the player to whom the monster is talking to
		mon->_mDelFlag = tmon->smDelFlag;
		mon->_mAnimCnt = tmon->smAnimCnt;   // Increases by one each game tick, counting how close we are to _mAnimFrameLen
		mon->_mAnimFrame = tmon->smAnimFrame; // Current frame of animation.
		mon->_mVar1 = SwapLE32(tmon->smVar1);
		mon->_mVar2 = SwapLE32(tmon->smVar2);
		mon->_mVar3 = SwapLE32(tmon->smVar3);
		mon->_mVar4 = SwapLE32(tmon->smVar4);
		mon->_mVar5 = SwapLE32(tmon->smVar5);
		mon->_mVar6 = SwapLE32(tmon->smVar6); // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
		mon->_mVar7 = SwapLE32(tmon->smVar7); // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
		mon->_mVar8 = SwapLE32(tmon->smVar8); // Value used to measure progress for moving from one tile to another
		mon->_mhitpoints = SwapLE32(tmon->smHitpoints);
		mon->_lastx = tmon->smLastx; // the last known X-coordinate of the enemy
		mon->_lasty = tmon->smLasty; // the last known Y-coordinate of the enemy
//BYTE leader; // the leader of the monster
		//mon->leaderflag = tmon->smLeaderflag; // the status of the monster's leader
//BYTE packsize; // the number of 'pack'-monsters close to their leader
//BYTE falign_CB;
		mon->_mFlags = SwapLE32(tmon->smFlags);
		// move the light of the monster
		if (mon->mlid != NO_LIGHT)
			ChangeLightXY(mon->mlid, mon->_moldx, mon->_moldy);
		// place the monster
		mi = mon->_mmode;
		if (mi != MM_STONE || mon->_mhitpoints != 0) {
			dMonster[mon->_mx][mon->_my] = mnum + 1;
			if (mi == MM_STONE)
				mi = mon->_mVar3;
			if (mi == MM_WALK2) {
				dMonster[mon->_moldx][mon->_moldy] = -(mnum + 1);
			} else if (mi == MM_WALK) {
				dMonster[mon->_mfutx][mon->_mfuty] = -(mnum + 1);
			} else if (mi == MM_CHARGE) {
				dMonster[mon->_mx][mon->_my] = -(mnum + 1);
			}
			// ensure dead bodies are not placed prematurely
			if (mi == MM_DEATH) {
				if (dDead[mon->_mx][mon->_my] == mnum + 1)
					dDead[mon->_mx][mon->_my] = 0;
			} else if (mnum < MAX_MINIONS) {
				mon->_mvid = AddVision(mon->_moldx, mon->_moldy, PLR_MIN_VISRAD, false);
			}
		}
		// no monsters in town at the moment. might want to allow it for minions later...
		net_assert(currLvl._dLevelIdx != DLV_TOWN);
		SyncMonsterAnim(mnum);
		src += sizeof(TSyncLvlMonster);
	}
	// load missiles
	assert(nummissiles == 0);
	for ( ; wLen >= sizeof(TSyncLvlMissile); wLen -= sizeof(TSyncLvlMissile)) {
		TSyncLvlMissile* __restrict tmis = (TSyncLvlMissile*)src;
		net_assert(SwapLE16(tmis->smiMi) >= MAXMONSTERS);
		mi = SwapLE16(tmis->smiMi) - MAXMONSTERS;
		net_assert((unsigned)mi < MAXMISSILES);
		net_assert(nummissiles < MAXMISSILES);
		net_assert(missileactive[nummissiles] == mi);
		nummissiles++;
		mis = &missile[mi];
		memset(mis, 0, sizeof(*mis));

		mis->_miType = tmis->smiType;   // Type of projectile (MIS_*)
		//BYTE _miSubType; // unused
		mis->_miAnimType = tmis->smiAnimType;
		//BOOL _miAnimFlag;
		mis->_miAnimCnt = tmis->smiAnimCnt; // Increases by one each game tick, counting how close we are to _miAnimFrameLen
		mis->_miAnimAdd = tmis->smiAnimAdd;
		mis->_miAnimFrame = tmis->smiAnimFrame; // Current frame of animation.
		mis->_miDrawFlag = tmis->smiDrawFlag;	// could be calculated
		mis->_miLightFlag = tmis->smiLightFlag;	// could be calculated
		mis->_miPreFlag = tmis->smiPreFlag;	// could be calculated
		mis->_miUniqTrans = tmis->smiUniqTrans;
		mis->_misx = tmis->smisx;    // Initial tile X-position for missile
		mis->_misy = tmis->smisy;    // Initial tile Y-position for missile
		mis->_mix = tmis->smix;     // Tile X-position of the missile
		mis->_miy = tmis->smiy;     // Tile Y-position of the missile
		mis->_mixoff = SwapLE32(tmis->smixoff);  // Sprite pixel X-offset for the missile
		mis->_miyoff = SwapLE32(tmis->smiyoff);  // Sprite pixel Y-offset for the missile
		mis->_mixvel = SwapLE32(tmis->smixvel);  // Missile tile X-velocity while walking. This gets added onto _mitxoff each game tick
		mis->_miyvel = SwapLE32(tmis->smiyvel);  // Missile tile Y-velocity while walking. This gets added onto _mitxoff each game tick
		mis->_mitxoff = SwapLE32(tmis->smitxoff); // How far the missile has travelled in its lifespan along the X-axis. mix/miy/mxoff/myoff get updated every game tick based on this
		mis->_mityoff = SwapLE32(tmis->smityoff); // How far the missile has travelled in its lifespan along the Y-axis. mix/miy/mxoff/myoff get updated every game tick based on this
		mis->_miDir = tmis->smiDir;   // The direction of the missile
		mis->_miSpllvl = SwapLE32(tmis->smiSpllvl); // int?
		mis->_miSource = SwapLE32(tmis->smiSource); // int?
		mis->_miCaster = SwapLE32(tmis->smiCaster); // int?
		mis->_miMinDam = SwapLE32(tmis->smiMinDam);
		mis->_miMaxDam = SwapLE32(tmis->smiMaxDam);
		// mis->_miRndSeed = tmis->smiRndSeed;
		mis->_miRange = SwapLE32(tmis->smiRange);
		mis->_miVar1 = SwapLE32(tmis->smiVar1);
		mis->_miVar2 = SwapLE32(tmis->smiVar2);
		mis->_miVar3 = SwapLE32(tmis->smiVar3);
		mis->_miVar4 = SwapLE32(tmis->smiVar4);
		mis->_miVar5 = SwapLE32(tmis->smiVar5);
		mis->_miVar6 = SwapLE32(tmis->smiVar6);
		mis->_miVar7 = SwapLE32(tmis->smiVar7);
		mis->_miVar8 = SwapLE32(tmis->smiVar8);

		if (tmis->smiLidRadius != 0) {
			mis->_miLid = AddLight(mis->_mix, mis->_miy, tmis->smiLidRadius);
		} else {
			mis->_miLid = NO_LIGHT;
		}
		mds = &missiledata[mis->_miType];
		mis->_miSubType = mds->mType;
		mis->_miFlags = mds->mdFlags;
		mis->_miResist = mds->mResist;

		// PutMissile(mi); - unnecessary, since it is just a gfx
		// PutMissileF(mi, BFLAG_HAZARD)
		if (mis->_miType == MIS_FIREWALL) {
			dMissile[mis->_mix][mis->_miy] = /*dMissile[mis->_mix][mis->_miy] == 0 ? mi + 1 :*/ MIS_MULTI;
			dFlags[mis->_mix][mis->_miy] |= BFLAG_HAZARD;
		}
		// PutMissileF(mi, BFLAG_MISSILE_PRE) - unnecessary, since it is just a gfx
		//if (mis->_miType == MIS_FLASH2 || mis->_miType == MIS_ACIDPUD) {
		//	dMissile[mis->_mix][mis->_miy] = /*dMissile[mis->_mix][mis->_miy] == 0 ? mi + 1 :*/ MIS_MULTI;
		//	dFlags[mis->_mix][mis->_miy] |= BFLAG_MISSILE_PRE;
		//}
		src += sizeof(TSyncLvlMissile);
	}
	SyncMissilesAnim();

	net_assert(wLen == 0);

	// validate data
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!plr._pActive || plr._pDunLevel != currLvl._dLevelIdx || plr._pLvlChanging)
			continue;
		net_assert(PosOkPlayer(pnum, plr._px, plr._py));
		if (plr._pmode == PM_WALK) {
			net_assert(plr._px == plr._poldx);
			net_assert(plr._py == plr._poldy);
			net_assert(abs(plr._pfutx - plr._px) <= 1);
			net_assert(abs(plr._pfuty - plr._py) <= 1);
			net_assert(PosOkPlayer(pnum, plr._pfutx, plr._pfuty));
			// FIXME: validate velocity/offset
		} else if (plr._pmode == PM_WALK2) {
			net_assert(plr._px == plr._pfutx);
			net_assert(plr._py == plr._pfuty);
			net_assert(abs(plr._poldx - plr._px) <= 1);
			net_assert(abs(plr._poldy - plr._py) <= 1);
			net_assert(PosOkPlayer(pnum, plr._poldx, plr._poldy));
			// FIXME: validate velocity/offset
		} else {
			net_assert(plr._px == plr._pfutx);
			net_assert(plr._py == plr._pfuty);
			net_assert(plr._px == plr._poldx);
			net_assert(plr._py == plr._poldy);
			switch (plr._pmode) {
			case PM_ATTACK:
				net_assert(plr._pVar5 < NUM_SPELLS); // ATTACK_SKILL
				net_assert(plr._pVar6 >= 0); // ATTACK_SKILL_LEVEL
				break;
			case PM_RATTACK:
				net_assert(plr._pVar5 < NUM_SPELLS); // RATTACK_SKILL
				net_assert(plr._pVar6 >= 0); // RATTACK_SKILL_LEVEL
				break;
			case PM_SPELL:
				net_assert(plr._pVar1 >= DBORDERX && plr._pVar1 < DBORDERX + DSIZEX); // SPELL_TARGET_X
				net_assert(plr._pVar2 >= DBORDERY && plr._pVar2 < DBORDERY + DSIZEY); // SPELL_TARGET_Y
				net_assert(plr._pVar5 < NUM_SPELLS); // SPELL_NUM
				net_assert(plr._pVar6 >= 0); // SPELL_LEVEL
				break;
			}
		}
		switch (plr.destAction) {
		case ACTION_NONE:
		case ACTION_WALK:
			break;
		case ACTION_OPERATE:
			net_assert(plr.destParam1 < MAXOBJECTS);
			net_assert(plr.destParam2 < MAXDUNX);
			net_assert(plr.destParam3 < MAXDUNY);
			net_assert(abs(dObject[plr.destParam2][plr.destParam3]) == plr.destParam1 + 1);
			break;
		case ACTION_BLOCK:
			net_assert(plr.destParam1 < NUM_DIRS);
			break;
		case ACTION_ATTACKMON:
		case ACTION_RATTACKMON:
		case ACTION_SPELLMON:
			net_assert(plr.destParam1 < MAXMONSTERS);
			net_assert(plr.destParam3 < NUM_SPELLS); // ATTACK_SKILL, SPELL_NUM
			net_assert(plr.destParam4 >= 0); // ATTACK_SKILL_LEVEL, SPELL_LEVEL
			break;
		case ACTION_ATTACK:
		case ACTION_RATTACK:
			net_assert(plr.destParam3 < NUM_SPELLS); // ATTACK_SKILL
			net_assert(plr.destParam4 >= 0); // ATTACK_SKILL_LEVEL
			break;
		case ACTION_ATTACKPLR:
		case ACTION_RATTACKPLR:
		case ACTION_SPELLPLR:
			net_assert(plr.destParam1 < MAX_PLRS);
			net_assert(plr.destParam3 < NUM_SPELLS); // ATTACK_SKILL, SPELL_NUM
			net_assert(plr.destParam4 >= 0); // ATTACK_SKILL_LEVEL, SPELL_LEVEL
			break;
		case ACTION_SPELL:
			net_assert(plr.destParam1 >= DBORDERX && plr.destParam1 < DBORDERX + DSIZEX); // SPELL_TARGET_X
			net_assert(plr.destParam2 >= DBORDERY && plr.destParam2 < DBORDERY + DSIZEY); // SPELL_TARGET_Y
			net_assert(plr.destParam3 < NUM_SPELLS); // SPELL_NUM
			net_assert(plr.destParam4 >= 0); // SPELL_LEVEL
			if (plr.destParam3 == SPL_DISARM)
				net_assert(plr.destParam4 < MAXOBJECTS); // fake SPELL_LEVEL
			if (plr.destParam3 == SPL_RESURRECT)
				net_assert(plr.destParam4 < MAX_PLRS); // fake SPELL_LEVEL
			if (plr.destParam3 == SPL_TELEKINESIS) {
				switch (plr.destParam4 >> 16) {
				case MTT_ITEM:
					net_assert((plr.destParam4 & 0xFFFF) < MAXITEMS);
					break;
				case MTT_MONSTER:
					net_assert((plr.destParam4 & 0xFFFF) < MAXMONSTERS);
					break;
				case MTT_OBJECT:
					net_assert((plr.destParam4 & 0xFFFF) < MAXOBJECTS);
					break;
				default:
					net_assert(0);
					break;
				}
			}
			break;
		case ACTION_PICKUPITEM:  // put item in hand (inventory screen open)
		case ACTION_PICKUPAITEM: // put item in inventory
			net_assert(plr.destParam1 < MAXITEMS);
			break;
		case ACTION_TALK:
			net_assert(plr.destParam1 < MAXMONSTERS);
			break;
		default:
			net_assert(0);
		}
		net_assert(plr._pAnimFrame <= plr._pAnimLen);
	}

	// ProcessLightList();
	ProcessVisionList();
}

static void LevelDeltaImportEnd(TCmdPlrInfoHdr* cmd, int pnum)
{
	LevelDeltaEnd* buf;

	guReceivedLevelDelta |= 1 << pnum;

	net_assert(SwapLE16(cmd->wBytes) == sizeof(LevelDeltaEnd));
	buf = (LevelDeltaEnd*)&cmd[1];
	net_assert(!buf->compressed);
	if (buf->numChunks == 0)
		return; // empty delta -> not done yet
	net_assert(buf->level == myplr._pDunLevel);

	if (gsDeltaData.ddRecvLastCmd == NMSG_LVL_DELTA_END) {
		//gbGameDeltaChunks = DELTA_ERROR_FAIL_1;
		guReceivedLevelDelta &= ~(1 << pnum);
		return; // lost or duplicated package -> ignore and expect a timeout
	}

	// decompress level data
	//assert(gsDeltaData.ddRecvLastCmd == NMSG_LVL_DELTA);
	if (gsDeltaData.ddSendRecvBuf.compressed)
		PkwareDecompress((BYTE*)&gsDeltaData.ddSendRecvBuf.content, gsDeltaData.ddSendRecvOffset, sizeof(gsDeltaData.ddSendRecvBuf.content));

	guDeltaTurn = SDL_SwapLE32(buf->turn);
	//gbGameDeltaChunks = MAX_CHUNKS - 1;
	// switch to delta-processing mode
	geBufferMsgs = MSG_LVL_DELTA_PROC;
}

static unsigned On_LVL_DELTA(TCmd* pCmd, int pnum)
{
	TCmdPlrInfoHdr* cmd = (TCmdPlrInfoHdr*)pCmd;

	if (geBufferMsgs != MSG_LVL_DELTA_WAIT)
		goto done; // the player is already active -> drop the packet

	if (cmd->bCmd == NMSG_LVL_DELTA_END) {
		// final package -> done
		LevelDeltaImportEnd(cmd, pnum);
		goto done;
	} else {
		if (gsDeltaData.ddRecvLastCmd != cmd->bCmd) {
			if (cmd->bCmd != NMSG_LVL_DELTA) {
				// invalid data type -> drop the packet
				goto done;
			}
			gsDeltaData.ddRecvLastCmd = cmd->bCmd;
			// start receiving a new package
			gsDeltaData.ddSendRecvOffset = 0;
			gsDeltaData.ddDeltaSender = pnum;
		} else if (gsDeltaData.ddDeltaSender != pnum) {
			// delta is already on its way from a different player -> drop the packet
			goto done;
		}
	}

	if (cmd->wOffset != gsDeltaData.ddSendRecvOffset) {
		// lost or duplicated package -> drop the connection and quit
		//gbGameDeltaChunks = DELTA_ERROR_FAIL_2;
		goto done;
	}
	net_assert((gsDeltaData.ddSendRecvOffset + cmd->wBytes) <= sizeof(gsDeltaData.ddSendRecvBuf));
	memcpy(((BYTE*)&gsDeltaData.ddSendRecvBuf) + cmd->wOffset, &cmd[1], cmd->wBytes);
	gsDeltaData.ddSendRecvOffset += cmd->wBytes;
done:
	return cmd->wBytes + sizeof(*cmd);
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
	cmd.mkExp = SwapLE32(mon->_mExp);
	cmd.mkMonLevel = mon->_mLevel;
	cmd.mkParam1.x = mon->_mx;
	cmd.mkParam1.y = mon->_my;
	cmd.mkDir = (!(mon->_mFlags & MFLAG_NOCORPSE) && mon->_mmode != MM_STONE) ? mon->_mdir : NUM_DIRS;
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

void NetSendCmdLocBParam2(BYTE bCmd, BYTE x, BYTE y, BYTE bParam1, BYTE bParam2)
{
	TCmdLocBParam2 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.bParam1 = bParam1;
	cmd.bParam2 = bParam2;

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

void NetSendCmdParamBW(BYTE bCmd, BYTE bParam1, WORD wParam2)
{
	TCmdParamBW cmd;

	cmd.bCmd = bCmd;
	cmd.byteParam = bParam1;
	cmd.wordParam = SwapLE16(wParam2);

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

void NetSendCmdLocSkill(BYTE x, BYTE y, BYTE skill, char from)
{
	TCmdLocSkill cmd;

	cmd.bCmd = CMD_SKILLXY;
	cmd.x = x;
	cmd.y = y;
	cmd.lsu.skill = skill;
	cmd.lsu.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLocDisarm(BYTE x, BYTE y, BYTE oi, char from)
{
	TCmdLocDisarm cmd;

	cmd.bCmd = CMD_DISARMXY;
	cmd.x = x;
	cmd.y = y;
	cmd.oi = oi;
	cmd.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdPlrSkill(int pnum, BYTE skill, char from)
{
	TCmdPlrSkill cmd;

	cmd.bCmd = CMD_SKILLPLR;
	cmd.psPnum = pnum;
	cmd.psu.skill = skill;
	cmd.psu.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonSkill(int mnum, BYTE skill, char from)
{
	TCmdMonSkill cmd;

	cmd.bCmd = CMD_SKILLMON;
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

void NetSendCmdMonstSummon(int mnum)
{
	TCmdMonstSummon cmd;
	MonsterStruct* mon;

	mon = &monsters[mnum];
	cmd.mnParam1.bCmd = CMD_MONSTSUMMON;
	cmd.mnParam1.bParam1 = currLvl._dLevelIdx;
	cmd.mnParam1.x = mon->_mx;
	cmd.mnParam1.y = mon->_my;
	cmd.mnMnum = mnum;
	cmd.mnSIdx = mon->_mMTidx;
	cmd.mnDir = mon->_mdir;
	cmd.mnMaxHp = mon->_mmaxhp;

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
	multi_send_direct_msg(pmask, (BYTE*)&cmd, sizeof(cmd.bCmd) + dwStrLen + 1);
}

void delta_open_portal(int i, BYTE x, BYTE y, BYTE bLevel)
{
	net_assert(bLevel < NUM_LEVELS);
	gsDeltaData.ddJunkChanged = true;
	gsDeltaData.ddJunk.jPortals[i].x = x;
	gsDeltaData.ddJunk.jPortals[i].y = y;
	gsDeltaData.ddJunk.jPortals[i].level = bLevel;
}

void delta_close_portal(int i)
{
	//memset(&gsDeltaData.ddJunk.portal[i], 0, sizeof(gsDeltaData.ddJunk.portal[i]));
	gsDeltaData.ddJunk.jPortals[i].level = DLV_TOWN;
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

	if (currLvl._dLevelIdx == plr._pDunLevel && MakePlrPath(pnum, cmd->x, cmd->y, true)) {
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
	int dir;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ClrPlrPath(pnum);
		dir = cmd->bParam1;

		net_assert(dir < NUM_DIRS);

		plr.destAction = ACTION_BLOCK;
		plr.destParam1 = dir;
	}
	return sizeof(*cmd);
}

static unsigned On_GOTOGETITEM(TCmd* pCmd, int pnum)
{
	TCmdLocParam1* cmd = (TCmdLocParam1*)pCmd;
	int ii;

	if (currLvl._dLevelIdx == plr._pDunLevel && MakePlrPath(pnum, cmd->x, cmd->y, false)) {
		ii = SwapLE16(cmd->wParam1);

		net_assert(ii < MAXITEMS);

		plr.destAction = ACTION_PICKUPITEM;
		plr.destParam1 = ii;
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
	int ii;

	if (currLvl._dLevelIdx == plr._pDunLevel && MakePlrPath(pnum, cmd->x, cmd->y, false)) {
		ii = SwapLE16(cmd->wParam1);

		net_assert(ii < MAXITEMS);

		plr.destAction = ACTION_PICKUPAITEM;
		plr.destParam1 = ii;
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
	int i, j, sx, sy, dx, dy;

	if (iidx == IDI_RUNEBOMB
	 && x >= DBORDERX + 69 && x <= DBORDERX + 72 && y >= DBORDERY + 51 && y <= DBORDERY + 54
	 && quests[Q_FARMER]._qactive != QUEST_DONE) {
		quests[Q_FARMER]._qactive = QUEST_DONE;
		quests[Q_FARMER]._qvar1 = 2 + pnum;
		quests[Q_FARMER]._qlog = TRUE;
		// open hive
		if (currLvl._dLevelIdx == DLV_TOWN) {
			sx = 70 + DBORDERX; sy = 52 + DBORDERY;
			dx = 71 + DBORDERX; dy = 53 + DBORDERY;
			PlaySfxLoc(LS_FLAMWAVE, sx, sy);
			for (i = sx; i <= dx; i++)
				for (j = sy; j <= dy; j++)
					AddMissile(i, j, -1, 0, 0, MIS_EXFBALL, MST_NA, 0, 0);
			gbOpenWarps |= (1 << TWARP_NEST);
			T_HiveOpen();
			InitTriggers();
		}
		return true;
	}
	if (iidx == IDI_FANG
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

	if (plr._pmode != PM_DEATH && (spelldata[sn].sUseFlags & plr._pSkillFlags) == spelldata[sn].sUseFlags) {
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
			if (sameLvl) {
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

static unsigned On_SKILLXY(TCmd* pCmd, int pnum)
{
	TCmdLocSkill* cmd = (TCmdLocSkill*)pCmd;

	if (CheckPlrSkillUse(pnum, cmd->lsu)) {
		ClrPlrPath(pnum);
		plr.destAction = spelldata[cmd->lsu.skill].sType != STYPE_NONE ? ACTION_SPELL : ((spelldata[cmd->lsu.skill].sUseFlags & SFLAG_RANGED) ? ACTION_RATTACK : ACTION_ATTACK);
		plr.destParam1 = cmd->x;
		plr.destParam2 = cmd->y;
		plr.destParam3 = cmd->lsu.skill; // spell/skill
		plr.destParam4 = (BYTE)cmd->lsu.from; // spllvl (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_OPERATEITEM(TCmd* pCmd, int pnum)
{
	TCmdItemOp* cmd = (TCmdItemOp*)pCmd;

	if (plr._pmode == PM_DEATH)
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
	int oi;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		oi = SwapLE16(cmd->wParam1);

		net_assert(oi < MAXOBJECTS);
		net_assert(cmd->x < MAXDUNX);
		net_assert(cmd->y < MAXDUNY);
		net_assert(abs(dObject[cmd->x][cmd->y]) == oi + 1);

		static_assert((int)ODT_NONE == 0, "BitOr optimization of On_OPOBJXY expects ODT_NONE to be zero.");
		if (MakePlrPath(pnum, cmd->x, cmd->y, !(objects[oi]._oSolidFlag | objects[oi]._oDoorFlag))) {
			plr.destAction = ACTION_OPERATE;
			plr.destParam1 = oi;
			plr.destParam2 = cmd->x;
			plr.destParam3 = cmd->y;
		}
	}

	return sizeof(*cmd);
}

static unsigned On_DISARMXY(TCmd* pCmd, int pnum)
{
	TCmdLocDisarm* cmd = (TCmdLocDisarm*)pCmd;
	int oi;
	CmdSkillUse su;

	su.from = cmd->from;
	su.skill = SPL_DISARM;

	if (CheckPlrSkillUse(pnum, su)) {
		oi = cmd->oi;

		net_assert(oi < MAXOBJECTS);
		net_assert(objects[oi]._oBreak == OBM_UNBREAKABLE);
		net_assert(cmd->x < MAXDUNX);
		net_assert(cmd->y < MAXDUNY);
		net_assert(abs(dObject[cmd->x][cmd->y]) == oi + 1);

		static_assert((int)ODT_NONE == 0, "BitOr optimization of On_DISARMXY expects ODT_NONE to be zero.");
		if (MakePlrPath(pnum, cmd->x, cmd->y, !(objects[oi]._oSolidFlag | objects[oi]._oDoorFlag))) {
			plr.destAction = ACTION_SPELL;
			plr.destParam1 = cmd->x;
			plr.destParam2 = cmd->y;
			plr.destParam3 = SPL_DISARM; // spell
			plr.destParam4 = oi;         // fake spllvl
		}
	}

	return sizeof(*cmd);
}

static unsigned On_SKILLMON(TCmd* pCmd, int pnum)
{
	TCmdMonSkill* cmd = (TCmdMonSkill*)pCmd;
	int mnum;

	if (CheckPlrSkillUse(pnum, cmd->msu)) {
		ClrPlrPath(pnum);
		mnum = SwapLE16(cmd->msMnum);

		net_assert(mnum < MAXMONSTERS);

		plr.destAction = spelldata[cmd->msu.skill].sType != STYPE_NONE ? ACTION_SPELLMON : ((spelldata[cmd->msu.skill].sUseFlags & SFLAG_RANGED) ? ACTION_RATTACKMON : ACTION_ATTACKMON);
		plr.destParam1 = mnum;                // target id
		plr.destParam3 = cmd->msu.skill;      // attack spell/skill
		plr.destParam4 = (BYTE)cmd->msu.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_SKILLPLR(TCmd* pCmd, int pnum)
{
	TCmdPlrSkill* cmd = (TCmdPlrSkill*)pCmd;
	int tnum;

	if (CheckPlrSkillUse(pnum, cmd->psu)) {
		ClrPlrPath(pnum);
		tnum = cmd->psPnum;

		net_assert(tnum < MAX_PLRS);

		plr.destAction = spelldata[cmd->psu.skill].sType != STYPE_NONE ? ACTION_SPELLPLR : ((spelldata[cmd->psu.skill].sUseFlags & SFLAG_RANGED) ? ACTION_RATTACKPLR : ACTION_ATTACKPLR);
		plr.destParam1 = tnum;                // target id
		plr.destParam3 = cmd->psu.skill;      // attack spell/skill
		plr.destParam4 = (BYTE)cmd->psu.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_TALKXY(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;
	int mnum = SwapLE16(cmd->wParam1);

	net_assert(mnum < MAXMONSTERS);

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		if (MakePlrPath(pnum, monsters[mnum]._mx, monsters[mnum]._my, false)) {
			plr.destAction = ACTION_TALK;
			plr.destParam1 = mnum;
		}
	}

	return sizeof(*cmd);
}

static unsigned On_NEWLVL(TCmd* pCmd, int pnum)
{
	TCmdBParam2* cmd = (TCmdBParam2*)pCmd;

	StartNewLvl(pnum, cmd->bParam1, cmd->bParam2);

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
		xp = SwapLE32(cmd->mkExp) / totplrs;
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
	TCmdBParam2* cmd = (TCmdBParam2*)pCmd;

	delta_monster_corpse(cmd);

	return sizeof(*cmd);
}

static unsigned On_MONSTSUMMON(TCmd* pCmd, int pnum)
{
	TCmdMonstSummon* cmd = (TCmdMonstSummon*)pCmd;

	delta_monster_summon(cmd);

	return sizeof(*cmd);
}

static bool PlrDeadItem(int pnum, ItemStruct* pi, int dir)
{
	int x, y;

	if (pi->_itype == ITYPE_NONE)
		return true;

	PkItemStruct pkItem;

	PackPkItem(&pkItem, pi);
	x = plr._px + offset_x[dir];
	y = plr._py + offset_y[dir];
	if (!delta_put_item(&pkItem, plr._pDunLevel, x, y))
		return false;
	if (currLvl._dLevelIdx == plr._pDunLevel) {
		UnPackPkItem(&pkItem);
		SyncPutItem(pnum, x, y, true);
	}
	pi->_itype = ITYPE_NONE;
	return true;
}

static unsigned On_PLRDEAD(TCmd* pCmd, int pnum)
{
	TCmdBParam1* cmd = (TCmdBParam1*)pCmd;
	int i, dmgtype = cmd->bParam1;

	// TODO: reset cursor if not an item?

	if (dmgtype == DMGTYPE_NPC) {
		plr._pExperience -= (plr._pExperience - PlrExpLvlsTbl[plr._pLevel - 1]) >> 2;

		if (PlrDeadItem(pnum, &plr._pHoldItem, NUM_DIRS - 1)) {
			if (pnum == mypnum && pcursicon >= CURSOR_FIRSTITEM)
				NewCursor(CURSOR_HAND);
			static_assert((int)NUM_INVLOC < (int)NUM_DIRS, "PlrDeadItem uses inv loc as direction offset.");
			for (i = 0; i < NUM_INVLOC; i++) {
				if (!PlrDeadItem(pnum, &plr._pInvBody[i], i))
					break;
			}
			CalcPlrInv(pnum, false);
			//PlrSetHp(pnum, 0);
			//PlrSetMana(pnum, 0);
		}
	} else if (dmgtype == DMGTYPE_PLAYER) {
		ItemStruct ear;
		CreateBaseItem(&ear, IDI_EAR);
		snprintf(ear._iName, sizeof(ear._iName), "Ear of %s", plr._pName);
		const int earSets[NUM_CLASSES] = {
				ICURS_EAR_WARRIOR, ICURS_EAR_ROGUE, ICURS_EAR_SORCERER
#ifdef HELLFIRE
				, ICURS_EAR_SORCERER, ICURS_EAR_ROGUE, ICURS_EAR_WARRIOR
#endif
		};
		ear._iCurs = earSets[plr._pClass];
		//ear._iCreateInfo = SwapLE16(*(WORD *)&ear._iName[7]);
		//ear._iSeed = SwapLE32(*(DWORD *)&ear._iName[9]);
		ear._ivalue = plr._pLevel;

		PlrDeadItem(pnum, &ear, DIR_S);
	}

	SyncPlrKill(pnum, dmgtype);

	if (pnum == mypnum)
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

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncOpObject(pnum, SwapLE16(cmd->wParam1));
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
	TCmdParamBW* cmd = (TCmdParamBW*)pCmd;
	BYTE r = cmd->byteParam;

	net_assert(r < NUM_INV_GRID_ELEM);

	// if (plr._pmode != PM_DEATH)
		SyncSplitGold(pnum, r, SwapLE16(cmd->wordParam));

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

	if (plr._pmode != PM_DEATH)
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

static unsigned On_JOINLEVEL(TCmd* pCmd, int pnum)
{
	TCmdJoinLevel* cmd = (TCmdJoinLevel*)pCmd;
	int i;
	ItemStruct* is;

	// reqister request only if not processing level-delta
	//if (geBufferMsgs != MSG_LVL_DELTA_PROC) { -- does not cover all cases...
		guSendLevelData |= (1 << pnum);
		guRequestLevelData[pnum] = gdwLastGameTurn;
	//}
	// should not be the case if priority is respected
	net_assert(geBufferMsgs != MSG_LVL_DELTA_SKIP_JOIN || currLvl._dLevelIdx != cmd->lLevel);
	if (geBufferMsgs != MSG_LVL_DELTA_WAIT /*&&
	 (geBufferMsgs != MSG_LVL_DELTA_SKIP_JOIN || currLvl._dLevelIdx != cmd->lLevel)*/) {
		plr._pLvlChanging = FALSE;
		//if (plr._pmode != PM_DEATH)
			plr._pInvincible = 40;
		net_assert(cmd->lLevel < NUM_LEVELS);
		net_assert(cmd->px >= DBORDERX && cmd->px < DBORDERX + DSIZEX);
		net_assert(cmd->py >= DBORDERY && cmd->py < DBORDERY + DSIZEY);
		plr._pDunLevel = cmd->lLevel;
		plr._px = cmd->px;
		plr._py = cmd->py;
	}

	if (pnum != mypnum) {
		if (!plr._pActive) {
			if (geBufferMsgs == MSG_LVL_DELTA_PROC) {
				// joined and left while waiting for level-delta
				return sizeof(*cmd);
			}
			if (plr._pName[0] == '\0') {
				// plrinfo_msg did not arrive -> drop the player
				SNetDropPlayer(pnum);
				return sizeof(*cmd);
			}
			// TODO: validate data from internet
			net_assert(plr._pTeam == pnum);
			net_assert(cmd->pManaShield == 0);
			net_assert(cmd->lLevel == DLV_TOWN);
			net_assert(cmd->lTimer1 == 0);
			net_assert(cmd->lTimer2 == 0);
			plr._pActive = TRUE;
			gbActivePlayers++;
			EventPlrMsg("Player '%s' (level %d) just joined the game", plr._pName, plr._pLevel);
			msg_mask_monhit(pnum);
		}
		if (currLvl._dLevelIdx == plr._pDunLevel /*&& geBufferMsgs != MSG_LVL_DELTA_SKIP_JOIN*/) {
			// should not be the case if priority is respected
			net_assert(geBufferMsgs != MSG_LVL_DELTA_SKIP_JOIN);
			assert(geBufferMsgs == MSG_NORMAL);
			plr._pHPBase = SwapLE32(cmd->php);
			plr._pManaBase = SwapLE32(cmd->pmp);
			plr._pTimer[PLTR_INFRAVISION] = SwapLE16(cmd->lTimer1);
			plr._pTimer[PLTR_RAGE] = SwapLE16(cmd->lTimer2);
			plr._pManaShield = cmd->pManaShield;

			for (i = 0; i < NUM_INVELEM; i++) {
				is = PlrItem(pnum, i);
				is->_iDurability = cmd->itemsDur[i];
			}
			plr._pHoldItem._iDurability = cmd->itemsDur[NUM_INVELEM];

			InitLvlPlayer(pnum, true);
			ProcessVisionList();
		}
	}

	return sizeof(*cmd);
}

static void DoTelekinesis(int pnum, int x, int y, char from, int id)
{
	CmdSkillUse su;

	su.from = from;
	su.skill = SPL_TELEKINESIS;

	if (CheckPlrSkillUse(pnum, su)) {
		ClrPlrPath(pnum);

		plr.destAction = ACTION_SPELL;
		plr.destParam1 = x;
		plr.destParam2 = y;
		plr.destParam3 = SPL_TELEKINESIS; // spell
		plr.destParam4 = id;              // fake spllvl
	}
}

static unsigned On_TELEKINXY(TCmd* pCmd, int pnum)
{
	TCmdLocBParam2* cmd = (TCmdLocBParam2*)pCmd;

	DoTelekinesis(pnum, cmd->x, cmd->y, cmd->bParam1, (MTT_ITEM << 16) | cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_TELEKINID(TCmd* pCmd, int pnum)
{
	TCmdParamBW* cmd = (TCmdParamBW*)pCmd;
	int mnum = cmd->wordParam;

	net_assert(mnum < MAXMONSTERS && mnum >= MAX_MINIONS);

	DoTelekinesis(pnum, monsters[mnum]._mx, monsters[mnum]._my, cmd->byteParam, (MTT_MONSTER << 16) | mnum);

	return sizeof(*cmd);
}

static unsigned On_TELEKINOID(TCmd* pCmd, int pnum)
{
	TCmdParamBW* cmd = (TCmdParamBW*)pCmd;
	int oi = cmd->wordParam;

	net_assert(oi < MAXOBJECTS);

	DoTelekinesis(pnum, objects[oi]._ox, objects[oi]._oy, cmd->byteParam, (MTT_OBJECT << 16) | oi);

	return sizeof(*cmd);
}

static unsigned On_ACTIVATEPORTAL(TCmd* pCmd, int pnum)
{
	TCmdLocBParam1* cmd = (TCmdLocBParam1*)pCmd;

	net_assert(cmd->bParam1 != DLV_TOWN);

	static_assert(MAXPORTAL == MAX_PLRS, "On_ACTIVATEPORTAL uses pnum as portal-id.");
	if (currLvl._dLevelIdx == DLV_TOWN)
		AddInTownPortal(pnum);
	else if (currLvl._dLevelIdx != cmd->bParam1)
		RemovePortalMissile(pnum);

	ActivatePortal(pnum, cmd->x, cmd->y, cmd->bParam1);

	return sizeof(*cmd);
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
				ReceivePlrMsg(pnum, cmd->str);
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

	net_assert(plr._pmode != PM_DEATH && plr._pmode != PM_DYING);
	net_assert(plr._pDunLevel == DLV_TOWN);
	net_assert(c == STORE_SSELL || c == STORE_SIDENTIFY || c == STORE_SREPAIR || c == STORE_WRECHARGE || c == STORE_PEGBOY);
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

	net_assert(plr._pmode != PM_DEATH && plr._pmode != PM_DYING);
	net_assert(plr._pDunLevel == DLV_TOWN);
	net_assert(c == STORE_HBUY || c == STORE_SBUY || c == STORE_SPBUY || c == STORE_WBUY || c == STORE_PBUY);

	UnPackPkItem(&cmd->item);
	SyncStoreCmd(pnum, c, MAXITEMS, SwapLE32(cmd->stValue));

	return sizeof(*cmd);
}

/* Sync item rewards in town. */
static unsigned On_QTOWNER(TCmd* pCmd, int pnum)
{
	TCmdParam1* cmd = (TCmdParam1*)pCmd;

	net_assert(plr._pmode != PM_DEATH && plr._pmode != PM_DYING);
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

	if (currLvl._dLevelIdx != plr._pDunLevel)
		SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);
	gsDeltaData.ddJunkChanged = true;

	return sizeof(*cmd);
}

#if DEV_MODE
static unsigned On_DUMP_MONSTERS(TCmd* pCmd, int pnum)
{
	int mnum;
	MonsterStruct* mon;

	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		mon = &monsters[mnum];

		LogErrorF("D-Mon", "mnum:%d "
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
	"ty:%d "
	"w:%d "
	"xo:%d ",
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
	mon->_mvid,
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
	mon->_mType,
	mon->_mAnimWidth,
	mon->_mAnimXOffset);
	DMonsterStr* mstr = &gsDeltaData.ddLevel[myplr._pDunLevel].monster[mnum];
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

static unsigned On_REQUEST_PLRCHECK(TCmd* pCmd, int pnum)
{
	BYTE plrdata[256];
	BYTE* buf = plrdata;
	*buf = CMD_DO_PLRCHECK;

	for (int i = 0; i < MAX_PLRS; i++) {
		if (!plx(i)._pActive)
			continue;
		// basic attributes
		buf = &plrdata[1];
		*buf = i;
		buf++;

		*buf = 0; // chunk-index
		buf++;

		*buf = plx(i)._pmode;
		buf++;

		//memcpy(buf, plx(i).walkpath, MAX_PATH_LENGTH + 1);
		//buf += MAX_PATH_LENGTH + 1;

		*buf = plx(i).destAction;
		buf++;

		*(INT*)buf = plx(i).destParam1;
		buf += sizeof(INT);
		*(INT*)buf = plx(i).destParam2;
		buf += sizeof(INT);
		*(INT*)buf = plx(i).destParam3;
		buf += sizeof(INT);
		*(INT*)buf = plx(i).destParam4;
		buf += sizeof(INT);

		*buf = plx(i)._pInvincible;
		buf++;
		*buf = plx(i)._pLvlChanging;
		buf++;
		*buf = plx(i)._pDunLevel;
		buf++;
	//BYTE _pClass;
		*buf = plx(i)._pLevel;
		buf++;
	//BYTE _pRank;
		*buf = plx(i)._pTeam;
		buf++;
		*(WORD*)buf = plx(i)._pStatPts;
		buf += sizeof(WORD);
	//BYTE _pLightRad;
		*buf = plx(i)._pManaShield;
		buf++;

		*(WORD*)buf = plx(i)._pStatPts;
		buf += sizeof(WORD);

		//memcpy(buf, plx(i)._pTimer, sizeof(plx(i)._pTimer));
		//buf += sizeof(plx(i)._pTimer);

		*(INT*)buf = plx(i)._pExperience;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pNextExper;
		buf += sizeof(INT);

		*buf = plx(i)._px;
		buf++;
		*buf = plx(i)._py;
		buf++;
		*buf = plx(i)._pfutx;
		buf++;
		*buf = plx(i)._pfuty;
		buf++;
		*buf = plx(i)._poldx;
		buf++;
		*buf = plx(i)._poldy;
		buf++;
	//int _pxoff;   // Player sprite's pixel X-offset from tile.
	//int _pyoff;   // Player sprite's pixel Y-offset from tile.
		*buf = plx(i)._pdir;
		buf++;
	//int _pAnimFrameLen; // Tick length of each frame in the current animation
	//int _pAnimCnt;
	//unsigned _pAnimLen;   // Number of frames in current animation
	//int _pAnimFrame;
	//int _pAnimWidth;
	//int _pAnimXOffset;
	//unsigned _plid;
	//unsigned _pvid;
	// char _pName[PLR_NAME_LEN];
		*(WORD*)buf = plx(i)._pBaseStr;
		buf += sizeof(WORD);
		*(WORD*)buf = plx(i)._pBaseMag;
		buf += sizeof(WORD);
		*(WORD*)buf = plx(i)._pBaseDex;
		buf += sizeof(WORD);
		*(WORD*)buf = plx(i)._pBaseVit;
		buf += sizeof(WORD);

		*(INT*)buf = plx(i)._pHPBase;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pMaxHPBase;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pManaBase;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pMaxManaBase;
		buf += sizeof(INT);

		*(INT*)buf = plx(i)._pVar1;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pVar2;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pVar3;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pVar4;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pVar5;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pVar6;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pVar7;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pVar8;
		buf += sizeof(INT);

		*(INT*)buf = plx(i)._pGold;
		buf += sizeof(INT);
	//int _pStrength;
	//int _pMagic;
	//int _pDexterity;
	//int _pVitality;
		*(INT*)buf = plx(i)._pHitPoints;
		buf += sizeof(INT);
	//int _pMaxHP;     // the maximum hp of the player
		*(INT*)buf = plx(i)._pMana;
		buf += sizeof(INT);
	//int _pMaxMana;   // the maximum mana of the player

		// LogErrorF("PLRD", "Player base-data %d", (size_t)buf - (size_t)plrdata);
		
		assert((size_t)buf - (size_t)plrdata == 114);
		NetSendChunk(plrdata, (size_t)buf - (size_t)plrdata);

		// skill attributes I.
		buf = &plrdata[1];
		*buf = i;
		buf++;

		*buf = 1; // chunk-index
		buf++;

		memcpy(buf, plx(i)._pSkillLvl, sizeof(plx(i)._pSkillLvl));
		buf += sizeof(plx(i)._pSkillLvl);
		memcpy(buf, plx(i)._pSkillLvlBase, sizeof(plx(i)._pSkillLvlBase));
		buf += sizeof(plx(i)._pSkillLvlBase);
		memcpy(buf, plx(i)._pSkillActivity, sizeof(plx(i)._pSkillActivity));
		buf += sizeof(plx(i)._pSkillActivity);

		*(uint64_t*)buf = plx(i)._pMemSkills;
		buf += sizeof(uint64_t);
		*(uint64_t*)buf = plx(i)._pAblSkills;
		buf += sizeof(uint64_t);
		*(uint64_t*)buf = plx(i)._pScrlSkills;
		buf += sizeof(uint64_t);

		//LogErrorF("PLRD", "Player skill-data I. %d", (size_t)buf - (size_t)plrdata);
		assert((size_t)buf - (size_t)plrdata == 219);
		NetSendChunk(plrdata, (size_t)buf - (size_t)plrdata);

		// skill attributes II.
		buf = &plrdata[1];
		*buf = i;
		buf++;

		*buf = 2; // chunk-index
		buf++;

		memcpy(buf, plx(i)._pSkillExp, sizeof(plx(i)._pSkillExp) / 2);
		buf += sizeof(plx(i)._pSkillExp) / 2;

		// LogErrorF("PLRD", "Player skill-data II. %d", (size_t)buf - (size_t)plrdata);
		assert((size_t)buf - (size_t)plrdata == 131);
		NetSendChunk(plrdata, (size_t)buf - (size_t)plrdata);

		// skill attributes III.
		buf = &plrdata[1];
		*buf = i;
		buf++;

		*buf = 3; // chunk-index
		buf++;

		memcpy(buf, &plx(i)._pSkillExp[32], sizeof(plx(i)._pSkillExp) / 2);
		buf += sizeof(plx(i)._pSkillExp) / 2;

		//LogErrorF("PLRD", "Player skill-data III. %d", (size_t)buf - (size_t)plrdata);
		assert((size_t)buf - (size_t)plrdata == 131);
		NetSendChunk(plrdata, (size_t)buf - (size_t)plrdata);
	/*unsigned _pNFrames;
	int _pNWidth;
	unsigned _pWFrames;
	int _pWWidth;
	unsigned _pAFrames;
	int _pAWidth;
	unsigned _pAFNum;
	unsigned _pSFrames;
	int _pSWidth;
	unsigned _pSFNum;
	unsigned _pHFrames;
	int _pHWidth;
	unsigned _pDFrames;
	int _pDWidth;
	unsigned _pBFrames;
	int _pBWidth;*/
	/*BOOLEAN _pInfraFlag;
	BYTE _pgfxnum; // Bitmask indicating what variant of the sprite the player is using. Lower byte define weapon (anim_weapon_id) and higher values define armour (starting with anim_armor_id)
	BOOLEAN _pHasUnidItem; // whether the player has an unidentified (magic) item equipped
	BYTE _pAlign_B0;
	int _pISlMinDam; // min slash-damage (swords, axes)
	int _pISlMaxDam; // max slash-damage (swords, axes)
	int _pIBlMinDam; // min blunt-damage (maces, axes)
	int _pIBlMaxDam; // max blunt-damage (maces, axes)
	int _pIPcMinDam; // min puncture-damage (bows, daggers)
	int _pIPcMaxDam; // max puncture-damage (bows, daggers)
	int _pIChMinDam; // min charge-damage (shield charge)
	int _pIChMaxDam; // max charge-damage (shield charge)
	int _pIEvasion;
	int _pIAC;
	char _pMagResist;
	char _pFireResist;
	char _pLghtResist;
	char _pAcidResist;
	int _pIHitChance;
	BYTE _pSkillFlags;    // Bitmask of allowed skill-types (SFLAG_*)
	BYTE _pIBaseHitBonus; // indicator whether the base BonusToHit of the items is positive/negative/neutral
	BYTE _pICritChance; // 200 == 100%
	BYTE _pIBlockChance;
	uint64_t _pISpells; // Bitmask of staff spell
	unsigned _pIFlags;
	BYTE _pIWalkSpeed;
	BYTE _pIRecoverySpeed;
	BYTE _pIBaseCastSpeed;
	BYTE _pAlign_B1;
	int _pIGetHit;
	BYTE _pIBaseAttackSpeed;
	char _pIArrowVelBonus; // _pISplCost in vanilla code
	BYTE _pILifeSteal;
	BYTE _pIManaSteal;
	int _pIFMinDam; // min fire damage (item's added fire damage)
	int _pIFMaxDam; // max fire damage (item's added fire damage)
	int _pILMinDam; // min lightning damage (item's added lightning damage)
	int _pILMaxDam; // max lightning damage (item's added lightning damage)
	int _pIMMinDam; // min magic damage (item's added magic damage)
	int _pIMMaxDam; // max magic damage (item's added magic damage)
	int _pIAMinDam; // min acid damage (item's added acid damage)
	int _pIAMaxDam; // max acid damage (item's added acid damage)*/
	}

	return sizeof(*pCmd);
}

static void PrintPlrMismatch(const char* field, int myval, int extval, int sp, int pnum)
{
	msg_errorf("%d received %s (%d vs. %d) from %d for plr%d", mypnum, field, myval, extval, sp, pnum);
}

static void CmpPlrArray(const char* field, void* src, void* data, int size, int len, int ip, int pnum)
{
	BYTE* sbuf = (BYTE*)src;
	BYTE* dbuf = (BYTE*)data;
	for (int i = 0; i < len; i++) {

		if (size == 1) {
			if (*sbuf != *dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch(tmp, *dbuf, *sbuf, pnum, ip);
			}
		} else if (size == 2) {
			if (*(uint16_t*)sbuf != *(uint16_t*)dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch(tmp, *(uint16_t*)dbuf, *(uint16_t*)sbuf, pnum, ip);
			}
		} else if (size == 4) {
			if (*(uint32_t*)sbuf != *(uint32_t*)dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch(tmp, *(uint32_t*)dbuf, *(uint32_t*)sbuf, pnum, ip);
			}
		} else if (size == 8) {
			if (*(uint64_t*)sbuf != *(uint64_t*)dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch(tmp, *(uint64_t*)dbuf, *(uint64_t*)sbuf, pnum, ip);
			}
		}
		sbuf += size;
		dbuf += size;
	}
}

static unsigned On_DO_PLRCHECK(TCmd* pCmd, int pnum)
{
	BYTE* src = (BYTE*)pCmd;
	src++;
	int i = *src, k;
	src++;
	k = *src;
	src++;

//	LogErrorF("Item", "ItemCheck %d. for %d running data from %d.", k, i, pnum);
	if (!plx(i)._pActive)
		msg_errorf("%d received inactive plr%d from %d", mypnum, i, pnum);

	switch (k) {
	case 0: // base params

		if (plx(i)._pmode != *src)
			PrintPlrMismatch("mode", plx(i)._pmode, *src, pnum, i);
		src++;

		// walkpath

		if (plx(i).destAction != *src)
			PrintPlrMismatch("destaction", plx(i).destAction, *src, pnum, i);
		src++;

		if (plx(i).destAction != ACTION_NONE && plx(i).destParam1 != *(INT*)src)
			PrintPlrMismatch("destparam1", plx(i).destParam1, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i).destAction != ACTION_NONE && plx(i).destParam2 != *(INT*)src)
			PrintPlrMismatch("destparam2", plx(i).destParam2, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i).destAction != ACTION_NONE && plx(i).destParam3 != *(INT*)src)
			PrintPlrMismatch("destparam3", plx(i).destParam3, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i).destAction != ACTION_NONE && plx(i).destParam4 != *(INT*)src)
			PrintPlrMismatch("destparam4", plx(i).destParam4, *(INT*)src, pnum, i);
		src += sizeof(INT);

		if (plx(i)._pInvincible != *src)
			PrintPlrMismatch("invin", plx(i)._pInvincible, *src, pnum, i);
		src++;
		if (plx(i)._pLvlChanging != *src)
			PrintPlrMismatch("lvlch", plx(i)._pLvlChanging, *src, pnum, i);
		src++;
		if (plx(i)._pDunLevel != *src)
			PrintPlrMismatch("dunlevel", plx(i)._pDunLevel, *src, pnum, i);
		src++;
	//BYTE _pClass;
		if (plx(i)._pLevel != *src)
			PrintPlrMismatch("level", plx(i)._pLevel, *src, pnum, i);
		src++;
	//BYTE _pRank;
		if (plx(i)._pTeam != *src)
			PrintPlrMismatch("team", plx(i)._pTeam, *src, pnum, i);
		src++;

		if (plx(i)._pStatPts != *(WORD*)src)
			PrintPlrMismatch("statpts", plx(i)._pStatPts, *(WORD*)src, pnum, i);
		src += sizeof(WORD);

	//BYTE _pLightRad;
		if (plx(i)._pManaShield != *src)
			PrintPlrMismatch("manashield", plx(i)._pManaShield, *src, pnum, i);
		src++;

		//memcpy(buf, plx(i)._pTimer, sizeof(plx(i)._pTimer));
		//buf += sizeof(plx(i)._pTimer);

		if (plx(i)._pExperience != *(INT*)src)
			PrintPlrMismatch("expr", plx(i)._pExperience, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pNextExper != *(INT*)src)
			PrintPlrMismatch("nexpr", plx(i)._pNextExper, *(INT*)src, pnum, i);
		src += sizeof(INT);

		if (plx(i)._px != *src)
			PrintPlrMismatch("px", plx(i)._px, *src, pnum, i);
		src++;
		if (plx(i)._py != *src)
			PrintPlrMismatch("py", plx(i)._py, *src, pnum, i);
		src++;
		if (plx(i)._pfutx != *src)
			PrintPlrMismatch("pfutx", plx(i)._pfutx, *src, pnum, i);
		src++;
		if (plx(i)._pfuty != *src)
			PrintPlrMismatch("pfuty", plx(i)._pfuty, *src, pnum, i);
		src++;
		if (plx(i)._poldx != *src)
			PrintPlrMismatch("poldx", plx(i)._poldx, *src, pnum, i);
		src++;
		if (plx(i)._poldy != *src)
			PrintPlrMismatch("poldy", plx(i)._poldy, *src, pnum, i);
		src++;

	//int _pxoff;   // Player sprite's pixel X-offset from tile.
	//int _pyoff;   // Player sprite's pixel Y-offset from tile.
		if (plx(i)._pdir != *src)
			PrintPlrMismatch("dir", plx(i)._pdir, *src, pnum, i);
		src++;
	//int _pAnimFrameLen; // Tick length of each frame in the current animation
	//int _pAnimCnt;
	//unsigned _pAnimLen;   // Number of frames in current animation
	//int _pAnimFrame;
	//int _pAnimWidth;
	//int _pAnimXOffset;
	//unsigned _plid;
	//unsigned _pvid;
	// char _pName[PLR_NAME_LEN];
		if (plx(i)._pBaseStr != *(WORD*)src)
			PrintPlrMismatch("basestr", plx(i)._pBaseStr, *(WORD*)src, pnum, i);
		src += sizeof(WORD);
		if (plx(i)._pBaseMag != *(WORD*)src)
			PrintPlrMismatch("basemag", plx(i)._pBaseMag, *(WORD*)src, pnum, i);
		src += sizeof(WORD);
		if (plx(i)._pBaseDex != *(WORD*)src)
			PrintPlrMismatch("basedex", plx(i)._pBaseDex, *(WORD*)src, pnum, i);
		src += sizeof(WORD);
		if (plx(i)._pBaseVit != *(WORD*)src)
			PrintPlrMismatch("basevit", plx(i)._pBaseVit, *(WORD*)src, pnum, i);
		src += sizeof(WORD);

		if (plx(i)._pHPBase != *(INT*)src)
			PrintPlrMismatch("HPBase", plx(i)._pHPBase, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pMaxHPBase != *(INT*)src)
			PrintPlrMismatch("MaxHPBase", plx(i)._pMaxHPBase, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pManaBase != *(INT*)src)
			PrintPlrMismatch("ManaBase", plx(i)._pManaBase, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pMaxManaBase != *(INT*)src)
			PrintPlrMismatch("MaxManaBase", plx(i)._pMaxManaBase, *(INT*)src, pnum, i);
		src += sizeof(INT);

		if (plx(i)._pVar1 != *(INT*)src)
			PrintPlrMismatch("Var1", plx(i)._pVar1, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar2 != *(INT*)src)
			PrintPlrMismatch("Var2", plx(i)._pVar2, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar3 != *(INT*)src)
			PrintPlrMismatch("Var3", plx(i)._pVar3, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar4 != *(INT*)src)
			PrintPlrMismatch("Var4", plx(i)._pVar4, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar5 != *(INT*)src)
			PrintPlrMismatch("Var5", plx(i)._pVar5, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar6 != *(INT*)src)
			PrintPlrMismatch("Var6", plx(i)._pVar6, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar7 != *(INT*)src)
			PrintPlrMismatch("Var7", plx(i)._pVar7, *(INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar8 != *(INT*)src)
			PrintPlrMismatch("Var8", plx(i)._pVar8, *(INT*)src, pnum, i);
		src += sizeof(INT);

		if (plx(i)._pGold != *(INT*)src)
			PrintPlrMismatch("Gold", plx(i)._pGold, *(INT*)src, pnum, i);
		src += sizeof(INT);
	//int _pStrength;
	//int _pMagic;
	//int _pDexterity;
	//int _pVitality;
		if (plx(i)._pHitPoints != *(INT*)src)
			PrintPlrMismatch("HitPoints", plx(i)._pHitPoints, *(INT*)src, pnum, i);
		src += sizeof(INT);
	//int _pMaxHP;     // the maximum hp of the player
		if (plx(i)._pMana != *(INT*)src)
			PrintPlrMismatch("Mana", plx(i)._pMana, *(INT*)src, pnum, i);
		src += sizeof(INT);
	//int _pMaxMana;   // the maximum mana of the player
		break;
	case 1: // skill data
		CmpPlrArray("SkillLvl", src, plx(i)._pSkillLvl, sizeof(plx(i)._pSkillLvl[0]), lengthof(plx(i)._pSkillLvl), i, pnum);
		src += sizeof(plx(i)._pSkillLvl);
		CmpPlrArray("SkillLvlBase", src, plx(i)._pSkillLvlBase, sizeof(plx(i)._pSkillLvlBase[0]), lengthof(plx(i)._pSkillLvlBase), i, pnum);
		src += sizeof(plx(i)._pSkillLvlBase);
		CmpPlrArray("SkillActivity", src, plx(i)._pSkillActivity, sizeof(plx(i)._pSkillActivity[0]), lengthof(plx(i)._pSkillActivity), i, pnum);
		src += sizeof(plx(i)._pSkillActivity);

		if (plx(i)._pMemSkills != *(uint64_t*)src)
			PrintPlrMismatch("MemSkills", plx(i)._pMemSkills, *(uint64_t*)src, pnum, i);
		src += sizeof(uint64_t);
		if (plx(i)._pAblSkills != *(uint64_t*)src)
			PrintPlrMismatch("AblSkills", plx(i)._pAblSkills, *(uint64_t*)src, pnum, i);
		src += sizeof(uint64_t);
		if (plx(i)._pScrlSkills != *(uint64_t*)src)
			PrintPlrMismatch("ScrlSkills", plx(i)._pScrlSkills, *(uint64_t*)src, pnum, i);
		src += sizeof(uint64_t);
		break;
	case 2: // skill data
		CmpPlrArray("SkillExp", src, plx(i)._pSkillExp, sizeof(plx(i)._pSkillExp[0]), lengthof(plx(i)._pSkillExp) / 2, i, pnum);
		src += sizeof(plx(i)._pSkillExp) / 2;
		break;
	case 3: // skill data
		CmpPlrArray("SkillExp2", src, &plx(i)._pSkillExp[32], sizeof(plx(i)._pSkillExp[0]), lengthof(plx(i)._pSkillExp) / 2, i, pnum);
		src += sizeof(plx(i)._pSkillExp) / 2;
		break;
	}

	return (size_t)src - (size_t)pCmd;
}

static const int ITEMCHECK_LEN = 4 + 2 + 2 + 1 + 1 + 1 + 1 + 4 + 4 + 4;
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
	*(INT*)dst = is->_iDurability;
	dst += sizeof(INT);
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
	if (!none && !placeholder && is->_iDurability != *(INT*)src) {
		PrintItemMismatch(is, "dur", is->_iDurability, *(INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);
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

//	LogErrorF("Item", "ItemCheck done. %d", (size_t)src - (size_t)pCmd);
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
	net_assert(quests[Q_LTBANNER]._qactive != QUEST_NOTAVAIL);

	quests[Q_LTBANNER]._qactive = QUEST_DONE;
	quests[Q_LTBANNER]._qvar1 = 4;

	//if (QuestStatus(Q_LTBANNER))
	if (currLvl._dLevelIdx == questlist[Q_LTBANNER]._qdlvl) {
		ResyncBanner();
		//RedoLightAndVision();
	}
	return sizeof(*pCmd);
}

unsigned ParseMsg(int pnum, TCmd* pCmd)
{
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
	case NMSG_LVL_DELTA:
	case NMSG_LVL_DELTA_END:
		return On_LVL_DELTA(pCmd, pnum);
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
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("ParseCmd: illegal player %d", pnum);
	}
	assert(geBufferMsgs != MSG_GAME_DELTA_LOAD && geBufferMsgs != MSG_GAME_DELTA_WAIT);
	switch (pCmd->bCmd) {
	case CMD_SYNCDATA:
		return On_SYNCDATA(pCmd, pnum);
	case CMD_WALKXY:
		return On_WALKXY(pCmd, pnum);
	case CMD_SKILLXY:
		return On_SKILLXY(pCmd, pnum);
	case CMD_OPOBJXY:
		return On_OPOBJXY(pCmd, pnum);
	case CMD_DISARMXY:
		return On_DISARMXY(pCmd, pnum);
	case CMD_SKILLPLR:
		return On_SKILLPLR(pCmd, pnum);
	case CMD_SKILLMON:
		return On_SKILLMON(pCmd, pnum);
	case CMD_BLOCK:
		return On_BLOCK(pCmd, pnum);
	case CMD_TALKXY:
		return On_TALKXY(pCmd, pnum);
	case CMD_MONSTDEATH:
		return On_MONSTDEATH(pCmd, pnum);
	case CMD_MONSTDAMAGE:
		return On_MONSTDAMAGE(pCmd, pnum);
	case CMD_MONSTCORPSE:
		return On_MONSTCORPSE(pCmd, pnum);
	case CMD_MONSTSUMMON:
		return On_MONSTSUMMON(pCmd, pnum);
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
	case CMD_TELEKINXY:
		return On_TELEKINXY(pCmd, pnum);
	case CMD_TELEKINID:
		return On_TELEKINID(pCmd, pnum);
	case CMD_TELEKINOID:
		return On_TELEKINOID(pCmd, pnum);
	case CMD_ACTIVATEPORTAL:
		return On_ACTIVATEPORTAL(pCmd, pnum);
	case CMD_NEWLVL:
		return On_NEWLVL(pCmd, pnum);
	case CMD_TWARP:
		return On_TWARP(pCmd, pnum);
	case CMD_RETOWN:
		return On_RETOWN(pCmd, pnum);
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
	case CMD_REQUEST_PLRCHECK:
		return On_REQUEST_PLRCHECK(pCmd, pnum);
	case CMD_DO_PLRCHECK:
		return On_DO_PLRCHECK(pCmd, pnum);
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
