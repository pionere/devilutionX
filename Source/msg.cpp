/**
 * @file msg.cpp
 *
 * Implementation of function for sending and reciving network messages.
 */
#include "all.h"
#include "diabloui.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#define MAX_CHUNKS             (NUM_LEVELS + MAX_PLRS + 5)
#define DELTA_ERROR_DISCONNECT (MAX_CHUNKS + 1)
#define DELTA_ERROR_FAIL_0     (MAX_CHUNKS + 2)
#define DELTA_ERROR_FAIL_1     (MAX_CHUNKS + 3)
#define DELTA_ERROR_FAIL_2     (MAX_CHUNKS + 4)
#define DELTA_ERROR_FAIL_3     (MAX_CHUNKS + 5)
#define DELTA_ERROR_FAIL_4     (MAX_CHUNKS + 6)

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
			pktSize = ParseMsg(pnum, (const TCmd*)data);
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

static void msg_mask_monhit(int pnum)
{
	int i, j;
	BYTE mask;

	static_assert(MAX_PLRS < 8, "msg_mask_monhit uses BYTE mask for pnum.");
	mask = ~(1 << pnum);
	for (i = 0; i < NUM_LEVELS; i++) {
		for (j = 0; j < MAXMONSTERS; j++) {
			gsDeltaData.ddLevel[i].monster[j].dmWhoHit &= mask;
		}
	}
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
	// assert(geBufferMsgs == MSG_GAME_DELTA_WAIT);
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
			app_warn(/*gbGameDeltaChunks == DELTA_ERROR_DISCONNECT ? "The game ended %d" :*/ "Unable to get game data %d", gbGameDeltaChunks);
#else
			app_warn(/*gbGameDeltaChunks == DELTA_ERROR_DISCONNECT ? "The game ended" :*/ "Unable to get game data");
#endif
		}
	}
	DeltaFreeMegaPkts();
	return success;
}

static BYTE* DeltaExportLevel(BYTE bLevel, BYTE* dst)
{
	DDItem* item;
	DDMonster* mon;
	int i;

	static_assert(sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content) >= sizeof(DDLevel) + 1, "DLevel might not fit to the buffer in DeltaExportLevel.");

	// level-index
	*dst = bLevel;
	dst++;
	*dst = gsDeltaData.ddLevelPlrs[bLevel];
	dst++;

	// export items
	item = gsDeltaData.ddLevel[bLevel].item;
	for (i = 0; i < lengthof(gsDeltaData.ddLevel[bLevel].item); i++, item++) {
		static_assert(sizeof(item->bCmd) == sizeof(*dst), "bCmd member of DDItem does not fit to DeltaExportLevel");
		*dst = item->bCmd;
		dst++;
		if (item->bCmd >= DCMD_ITM_MOVED) {
			static_assert(DCMD_INVALID < DCMD_ITM_MOVED, "DeltaExportLevel requires ordered DCMD_ITM_* enum I.");
			static_assert(DCMD_ITM_SPAWNED < DCMD_ITM_MOVED, "DeltaExportLevel requires ordered DCMD_ITM_* enum II.");
			static_assert(DCMD_ITM_TAKEN < DCMD_ITM_MOVED, "DeltaExportLevel requires ordered DCMD_ITM_* enum III.");
			static_assert(DCMD_ITM_DROPPED >= DCMD_ITM_MOVED, "DeltaExportLevel requires ordered DCMD_ITM_* enum IV.");
			static_assert(offsetof(DDItem, bCmd) == 0, "DeltaExportLevel does not work with DDItem.");
			memcpy(dst, &(&item->bCmd)[1], sizeof(DDItem) - sizeof(item->bCmd));
			dst += sizeof(DDItem) - sizeof(item->bCmd);
		}
	}

	// export objects
	memcpy(dst, gsDeltaData.ddLevel[bLevel].object, sizeof(gsDeltaData.ddLevel[bLevel].object));
	dst += sizeof(gsDeltaData.ddLevel[bLevel].object);

	// export monsters
	mon = gsDeltaData.ddLevel[bLevel].monster;
	for (i = 0; i < lengthof(gsDeltaData.ddLevel[bLevel].monster); i++, mon++) {
		if (mon->dmCmd == DCMD_MON_INVALID) {
			*dst = DCMD_MON_INVALID;
			dst++;
		} else {
			copy_pod(*reinterpret_cast<DDMonster*>(dst), *mon);
			dst += sizeof(DDMonster);
		}
	}

	return dst;
}

static void DeltaImportLevel()
{
	DDItem* item;
	DDMonster* mon;
	int i;
	BYTE *src, bLvl;

	static_assert(sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content) >= sizeof(DDLevel) + 1, "DLevel might not fit to the buffer in DeltaImportLevel.");

	src = gsDeltaData.ddSendRecvPkt.apMsg.tpData.content;
	// level-index
	bLvl = *src;
	src++;
	// the number of players
	gsDeltaData.ddLevelPlrs[bLvl] = *src;
	src++;
	net_assert(gsDeltaData.ddLevelPlrs[bLvl] != 0);

	// import items
	item = gsDeltaData.ddLevel[bLvl].item;
	for (i = 0; i < MAXITEMS; i++, item++) {
		net_assert(*src <= DCMD_ITM_DROPPED);
		static_assert(sizeof(item->bCmd) == sizeof(*src), "bCmd member of DDItem does not fit to DeltaImportLevel");
		item->bCmd = *src;
		src++;
		static_assert(DCMD_INVALID < DCMD_ITM_MOVED, "DeltaImportLevel requires ordered DCMD_ITM_* enum I.");
		static_assert(DCMD_ITM_SPAWNED < DCMD_ITM_MOVED, "DeltaImportLevel requires ordered DCMD_ITM_* enum II.");
		static_assert(DCMD_ITM_TAKEN < DCMD_ITM_MOVED, "DeltaImportLevel requires ordered DCMD_ITM_* enum III.");
		static_assert(DCMD_ITM_DROPPED >= DCMD_ITM_MOVED, "DeltaImportLevel requires ordered DCMD_ITM_* enum IV.");
		if (item->bCmd >= DCMD_ITM_MOVED) {
			// TODO: validate data from internet
			static_assert(offsetof(DDItem, bCmd) == 0, "DeltaImportLevel does not work with DDItem.");
			memcpy(&(&item->bCmd)[1], src, sizeof(DDItem) - sizeof(item->bCmd));
			src += sizeof(DDItem) - sizeof(item->bCmd);
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
			copy_pod(*mon, *reinterpret_cast<DDMonster*>(src));
			src += sizeof(DDMonster);
		}
	}
}

static BYTE* DeltaExportJunk(BYTE* dst)
{
	DDPortal* pDPortal;
	DDQuest* pDQuest;
	DDDynLevel* pDLevel;
	int i;
	constexpr int junkDataSize = MAXPORTAL * sizeof(DDPortal) + NUM_QUESTS * sizeof(DDQuest) + NUM_DYNLVLS * sizeof(DDDynLevel) + sizeof(gsDeltaData.ddJunk);
	static_assert(sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content) >= junkDataSize, "DJunk does not fit to the buffer in DeltaExportJunk.");

	// export portals
	pDPortal = (DDPortal*)dst;
	for (i = 0; i < MAXPORTAL; i++, pDPortal++) {
		pDPortal->level = portals[i]._rlevel;
		pDPortal->x = portals[i]._rx;
		pDPortal->y = portals[i]._ry;
	}
	dst = (BYTE*)pDPortal;
	// export quests
	pDQuest = (DDQuest*)dst;
	for (i = 0; i < NUM_QUESTS; i++, pDQuest++) {
		pDQuest->qstate = quests[i]._qactive;
		pDQuest->qlog = quests[i]._qlog;
		pDQuest->qvar1 = quests[i]._qvar1;
	}
	dst = (BYTE*)pDQuest;
	// export dynamic levels
	pDLevel = (DDDynLevel*)dst;
	for (i = 0; i < NUM_DYNLVLS; i++) {
		pDLevel->dlSeed = glSeedTbl[NUM_FIXLVLS + i];
		pDLevel->dlLevel = gDynLevels[i]._dnLevel;
		pDLevel->dlType = gDynLevels[i]._dnType;
		pDLevel++;
	}
	dst = (BYTE*)pDLevel;
	// export golems
	memcpy(dst, &gsDeltaData.ddJunk, sizeof(gsDeltaData.ddJunk));
	dst += sizeof(gsDeltaData.ddJunk);

	return dst;
}

static void DeltaImportJunk()
{
	DDPortal* pDPortal;
	DDQuest* pDQuest;
	DDDynLevel* pDLevel;
	int i;
	BYTE* src = gsDeltaData.ddSendRecvPkt.apMsg.tpData.content;
	constexpr int junkDataSize = MAXPORTAL * sizeof(DDPortal) + NUM_QUESTS * sizeof(DDQuest) + NUM_DYNLVLS * sizeof(DDDynLevel) + sizeof(gsDeltaData.ddJunk);
	// static_assert(sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content) >= sizeof(gsDeltaData.ddJunk), "DJunk does not fit to the buffer in DeltaImportJunk.");
	static_assert(sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content) >= junkDataSize, "DJunk does not fit to the buffer in DeltaImportJunk.");

	// update portals
	pDPortal = (DDPortal*)src;
	for (i = 0; i < MAXPORTAL; i++, pDPortal++) {
		if (pDPortal->level != DLV_TOWN) {
			ActivatePortal(i, pDPortal->x, pDPortal->y, pDPortal->level);
		}
		//else
		//	SetPortalStats(i, false, 0, 0, 0);
	}
	src = (BYTE*)pDPortal;
	// update quests
	pDQuest = (DDQuest*)src;
	for (i = 0; i < NUM_QUESTS; i++, pDQuest++) {
		quests[i]._qlog = pDQuest->qlog;
		quests[i]._qactive = pDQuest->qstate;
		quests[i]._qvar1 = pDQuest->qvar1;
	}
	src = (BYTE*)pDQuest;
	// update dynamic levels
	pDLevel = (DDDynLevel*)src;
	for (i = 0; i < NUM_DYNLVLS; i++) {
		glSeedTbl[NUM_FIXLVLS + i] = pDLevel->dlSeed;
		gDynLevels[i]._dnLevel = pDLevel->dlLevel;
		gDynLevels[i]._dnType = pDLevel->dlType;
		pDLevel++;
	}
	src = (BYTE*)pDLevel;
	// update golems
	memcpy(&gsDeltaData.ddJunk, src, sizeof(gsDeltaData.ddJunk));
	// src += sizeof(gsDeltaData.ddJunk);
}

static BYTE* DeltaExportPlr(int pnum, BYTE* dst)
{
	static_assert(sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content) >= sizeof(PkPlayerStruct) + 1, "DPlayer does not fit to the buffer in DeltaExportPlr.");

	// player-index
	*dst = pnum;
	dst++;

	PackPlayer((PkPlayerStruct*)dst, pnum);

	return dst + sizeof(PkPlayerStruct);
}

static void DeltaImportPlr()
{
	int pnum;
	BYTE* src = gsDeltaData.ddSendRecvPkt.apMsg.tpData.content;

	static_assert(sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content) >= sizeof(PkPlayerStruct) + 1, "DPlayer does not fit to the buffer in DeltaImportPlr.");

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

static void DeltaDecompressData()
{
	if (gsDeltaData.ddSendRecvPkt.apMsg.tpData.compressed)
		PkwareDecompress(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content, gsDeltaData.ddSendRecvOffset - sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.compressed), sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData.content));
}

void DeltaExportData(int pmask)
{
	DBuffer* buff = &gsDeltaData.ddSendRecvPkt.apMsg.tpData;
	BYTE* lvlData = buff->content;
	BYTE* dstEnd;
	int i;
	BYTE numChunks = 0;

	// levels
	for (i = 0; i < lengthof(gsDeltaData.ddLevel); i++) {
		if (gsDeltaData.ddLevelPlrs[i] == 0)
			continue;
		dstEnd = DeltaExportLevel(i, lvlData);
		static_assert(NET_LARGE_MSG_SIZE <= UINT_MAX, "Size of large message can not be sent in an unsigned parameter II.");
		multi_send_large_msg(pmask, NMSG_DLEVEL_DATA, static_cast<unsigned>((size_t)dstEnd - (size_t)lvlData));
		numChunks++;
	}
	{ // junk
		dstEnd = DeltaExportJunk(lvlData);
		static_assert(NET_LARGE_MSG_SIZE <= UINT_MAX, "Size of large message can not be sent in an unsigned parameter II.");
		multi_send_large_msg(pmask, NMSG_DLEVEL_JUNK, static_cast<unsigned>((size_t)dstEnd - (size_t)lvlData));
		numChunks++;
	}
	// players
	for (i = 0; i < MAX_PLRS; i++) {
		if (plx(i)._pActive) {
			dstEnd = DeltaExportPlr(i, lvlData);
			// assert((size_t)dstEnd - (size_t)lvlData == 1 + sizeof(PkPlayerStruct));
			multi_send_large_msg(pmask, NMSG_DLEVEL_PLR, 1 + sizeof(PkPlayerStruct)); // (size_t)dstEnd - (size_t)lvlData);
			numChunks++;
		}
	}
	static_assert(lengthof(gsDeltaData.ddLevel) + 1 + MAX_PLRS < UCHAR_MAX, "DeltaExportData sends the number of chunks in a BYTE field.");
	// current number of chunks sent + turn-id + end
	DeltaDataEnd* deltaEnd = (DeltaDataEnd*)lvlData;
	deltaEnd->numChunks = numChunks;
	deltaEnd->turn = gdwLastGameTurn;
	assert(gdwLastGameTurn * gbNetUpdateRate == gdwGameLogicTurn);
	multi_send_large_msg(pmask, NMSG_DLEVEL_END, sizeof(*deltaEnd));
}

static void DeltaImportData()
{
	DeltaDecompressData();

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

static void DeltaImportEnd(const TMsgLarge* cmd)
{
	const DeltaDataEnd* buf;

	// stop nthread from processing the delta messages
	geBufferMsgs = MSG_NORMAL;

	net_assert(cmd->tpHdr.wBytes == sizeof(cmd->tpData.compressed) + sizeof(DeltaDataEnd));
	static_assert(NET_COMP_MSG_SIZE > sizeof(DeltaDataEnd), "DeltaImportEnd does not decompress the final message.");
	net_assert(!cmd->tpData.compressed);
	buf = (const DeltaDataEnd*)cmd->tpData.content;
	if (gbGameDeltaChunks != buf->numChunks) {
		// not all chunks arrived -> quit
		gbGameDeltaChunks = DELTA_ERROR_FAIL_3;
		return;
	}
	guDeltaTurn = buf->turn; // TODO: validate that it is in the near future
	gbGameDeltaChunks = MAX_CHUNKS - 1;
}

static unsigned On_DLEVEL(const TCmd* pCmd, int pnum)
{
	const TMsgLarge* cmd = (const TMsgLarge*)pCmd;

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
		/*if ((unsigned)pnum >= MAX_PLRS && pnum != SNPLAYER_MASTER) {
			// message from an invalid player -> drop the packet
			goto done;
		}*/
		// start receiving
		gsDeltaData.ddDeltaSender = pnum;
		// gsDeltaData.ddSendRecvOffset = 0;
	}

	if (cmd->tpHdr.bCmd == NMSG_DLEVEL_END) {
		// gsDeltaData.ddRecvLastCmd = NMSG_DLEVEL_END;
		DeltaImportEnd(cmd);
		goto done;
	}
	if (cmd->tpHdr.bCmd != NMSG_DLEVEL_DATA && cmd->tpHdr.bCmd != NMSG_DLEVEL_JUNK && cmd->tpHdr.bCmd != NMSG_DLEVEL_PLR) {
		// invalid data starting type -> drop the packet
		goto done;
	}

	net_assert((gsDeltaData.ddSendRecvOffset + cmd->tpHdr.wBytes) <= sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData));
	memcpy(((BYTE*)&gsDeltaData.ddSendRecvPkt.apMsg.tpData), &cmd->tpData, cmd->tpHdr.wBytes);
	gsDeltaData.ddRecvLastCmd = cmd->tpHdr.bCmd;
	gsDeltaData.ddSendRecvOffset = cmd->tpHdr.wBytes;
	DeltaImportData();
done:
	return cmd->tpHdr.wBytes + sizeof(cmd->tpHdr);
}

void delta_init()
{
	//memset(gsDeltaData.ddLevelPlrs, 0, sizeof(gsDeltaData.ddLevelPlrs));
	static_assert((int)DLV_TOWN == 0, "delta_init initializes the portal levels to zero, assuming none of the portals starts from the town.");
	//memset(&gsDeltaData.ddJunk, 0, sizeof(gsDeltaData.ddJunk));
	static_assert((int)DCMD_INVALID == 0, "delta_init initializes the items with zero, assuming the invalid command to be zero.");
	static_assert((int)DCMD_MON_INVALID == 0, "delta_init initializes the monsters with zero, assuming the invalid command to be zero.");
	static_assert((int)CMD_SYNCDATA == 0, "delta_init initializes the objects with zero, assuming none of the valid commands for an object to be zero.");
	//memset(gsDeltaData.ddLevel, 0, sizeof(gsDeltaData.ddLevel));
	//memset(gsDeltaData.ddLocal, 0, sizeof(gsDeltaData.ddLocal));
	//gsDeltaData.ddSendRecvOffset = 0;
	memset(&gsDeltaData, 0, sizeof(gsDeltaData));
	// ensure the entry-level is 'initialized'
	gsDeltaData.ddLevelPlrs[DLV_TOWN] = 1;
	assert(!deltaload);
}

static void delta_monster_corpse(const TCmdBParam2* pCmd)
{
	BYTE bLevel;
	DDMonster* mon;

	if (!IsMultiGame)
		return;

	bLevel = pCmd->bParam1;
	net_assert(bLevel < NUM_LEVELS);
	// commented out, because dmCmd must be already set at this point
	// net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	net_assert(pCmd->bParam2 < MAXMONSTERS);
	mon = &gsDeltaData.ddLevel[bLevel].monster[pCmd->bParam2];
	if (mon->dmCmd == DCMD_MON_DEAD)
		mon->dmCmd = DCMD_MON_DESTROYED;
}

static void delta_monster_summon(const TCmdMonstSummon* pCmd)
{
	BYTE bLevel;
	DDMonster* mon;

	if (!IsMultiGame)
		return;

	bLevel = pCmd->mnParam1.bParam1;
	net_assert(bLevel < NUM_LEVELS);
#ifdef HELLFIRE
	net_assert(bLevel == SL_SKELKING || bLevel == DLV_NEST3);
#else
	net_assert(bLevel == SL_SKELKING);
#endif
	net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	net_assert(pCmd->mnMnum >= MAX_MINIONS && pCmd->mnMnum < MAXMONSTERS);
	mon = &gsDeltaData.ddLevel[bLevel].monster[pCmd->mnMnum];
	if (mon->dmCmd == DCMD_MON_ACTIVE)
		return;
	assert(mon->dmCmd == DCMD_MON_DEAD || mon->dmCmd == DCMD_MON_DESTROYED || mon->dmCmd == DCMD_MON_INVALID);
	mon->dmx = pCmd->mnParam1.x;
	mon->dmy = pCmd->mnParam1.y;
	mon->dmdir = pCmd->mnDir;
	mon->dmSIdx = pCmd->mnSIdx + 1;
	mon->dmCmd = DCMD_MON_ACTIVE;
	mon->dmWhoHit = 0;
	mon->dmactive = static_cast<uint32_t>(0);
	mon->dmleaderflag = MLEADER_NONE;
	mon->dmhitpoints = pCmd->mnMaxHp;
}

static BYTE delta_kill_monster(const TCmdMonstKill* mon)
{
	DDMonster* pD;
	int mnum;
	BYTE bLevel, whoHit;

	whoHit = mon->mkPnum < MAX_PLRS ? 1 << mon->mkPnum : 0;
	if (!IsMultiGame) {
		return whoHit; // TODO: what about trap-kills?
		// return 1 << mypnum; -- exclude UMT_LACHDAN?
	}

	mnum = mon->mkMnum;
	bLevel = mon->mkParam1.bParam1;
	net_assert(bLevel < NUM_LEVELS);
	net_assert(mnum < MAXMONSTERS);

	net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	pD = &gsDeltaData.ddLevel[bLevel].monster[mnum];
	static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "delta_kill_monster expects ordered DCMD_MON_ enum I.");
	static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "delta_kill_monster expects ordered DCMD_MON_ enum II.");
	if (pD->dmCmd >= DCMD_MON_DEAD)
		return 0;
	pD->dmCmd = mon->mkDir < NUM_DIRS ? DCMD_MON_DEAD : DCMD_MON_DESTROYED;
	pD->dmx = mon->mkParam1.x;
	pD->dmy = mon->mkParam1.y;
	pD->dmdir = mon->mkDir;
	pD->dmleaderflag = MLEADER_NONE; // TODO: reset _mleaderflag of the minions if the info is available
	pD->dmhitpoints = 0;
	return whoHit | pD->dmWhoHit;
}

static void delta_monster_hp(const TCmdMonstDamage* mon, int pnum)
{
	DDMonster* pD;
	BYTE bLevel;

	if (!IsMultiGame)
		return;

	bLevel = mon->mdLevel;
	net_assert(bLevel < NUM_LEVELS);
	net_assert(mon->mdMnum < MAXMONSTERS);

	// commented out, because these changes are ineffective unless dmCmd is already set
	// net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	pD = &gsDeltaData.ddLevel[bLevel].monster[mon->mdMnum];
	static_assert(MAX_PLRS < 8, "delta_monster_hp uses BYTE mask for pnum.");
	pD->dmWhoHit |= 1 << pnum;
	// In vanilla code the value was discarded if hp was higher than the current one.
	// That disregards the healing monsters.
	// Now it is always updated except the monster is already dead.
	//if (pD->dmCmd != DCMD_MON_DEAD && pD->dmCmd != DCMD_MON_DESTROYED)
		pD->dmhitpoints = mon->mdHitpoints;
}

static void delta_sync_monster(const TSyncHeader* pHdr)
{
	DDMonster* pDLvlMons;
	DDMonster* pD;
	uint16_t wLen;
	const TSyncMonster* pSync;
	const BYTE* pbBuf;
	BYTE bLevel;

	assert(IsMultiGame);

	bLevel = pHdr->bLevel;
	net_assert(bLevel < NUM_LEVELS);

	net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	pDLvlMons = gsDeltaData.ddLevel[bLevel].monster;

	pbBuf = (const BYTE*)&pHdr[1];
	for (wLen = pHdr->wLen; wLen >= sizeof(TSyncMonster); wLen -= sizeof(TSyncMonster)) {
		pSync = (TSyncMonster*)pbBuf;
		net_assert(pSync->nmndx < MAXMONSTERS);
		pD = &pDLvlMons[pSync->nmndx];
		static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "delta_sync_monster expects ordered DCMD_MON_ enum I.");
		static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "delta_sync_monster expects ordered DCMD_MON_ enum II.");
		if (pD->dmCmd < DCMD_MON_DEAD) {
			pD->dmCmd = DCMD_MON_ACTIVE;
			pD->dmx = pSync->nmx;
			pD->dmy = pSync->nmy;
			pD->dmdir = pSync->nmdir;
			pD->dmleaderflag = pSync->nmleaderflag;
			pD->dmactive = pSync->nmactive;
			pD->dmhitpoints = pSync->nmhitpoints;
		}
		pbBuf += sizeof(TSyncMonster);
	}
	net_assert(wLen == 0);
}

static void delta_awake_golem(const TCmdGolem* pG, int mnum)
{
	DDMonster* pD;
	BYTE bLevel;

	if (!IsMultiGame)
		return;

	gsDeltaData.ddJunk.jGolems[mnum] = pG->goMonLevel;

	InitGolemStats(mnum, pG->goMonLevel);

	bLevel = pG->goDunLevel;
	net_assert(bLevel < NUM_LEVELS);

	net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	pD = &gsDeltaData.ddLevel[bLevel].monster[mnum];
	pD->dmCmd = DCMD_MON_ACTIVE;
	pD->dmx = pG->goX;
	pD->dmy = pG->goY;
	pD->dmactive = static_cast<uint32_t>(SQUELCH_MAX);
	// pD->dmdir = DIR_S; -- should not matter
	static_assert(MLEADER_NONE == 0, "delta_awake_golem expects _mleaderflag to be set by zerofill.");
	// pD->dmleaderflag = MLEADER_NONE;
	pD->dmhitpoints = monsters[mnum]._mmaxhp;
}

static void delta_leave_sync(BYTE bLevel)
{
	if (bLevel == DLV_TOWN) {
		glSeedTbl[DLV_TOWN] = NextRndSeed();
		return;
	}
	static_assert(sizeof(gsDeltaData.ddLocal[bLevel].automapsv) == sizeof(dFlags), "Automap info can not be stored in the allocated space.");

	memcpy(&gsDeltaData.ddLocal[bLevel].automapsv, dFlags, sizeof(dFlags));
}

static void delta_sync_object(int oi, BYTE bCmd, BYTE bLevel)
{
	if (!IsMultiGame)
		return;

	net_assert(bLevel < NUM_LEVELS);
	net_assert(oi < MAXOBJECTS);

	net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	gsDeltaData.ddLevel[bLevel].object[oi].bCmd = bCmd;
}

static void delta_reserve_items(const TCmdJoinLevel* cmd)
{
	DDItem* pD;
	int i, n;
	BYTE bLevel;

	// assert(IsMultiGame);
	bLevel = cmd->lLevel;
	// net_assert(bLevel < NUM_LEVELS);

	pD = gsDeltaData.ddLevel[bLevel].item;
	// net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	n = cmd->iFloorItems;
	net_assert(n < MAXITEMS);
	for (i = 0; i < n; i++, pD++) {
		if (pD->bCmd == DCMD_INVALID) {
			pD->bCmd = DCMD_ITM_SPAWNED;
		}
	}
}

static bool delta_get_item(const TCmdGItem* pI)
{
	DDItem* pD;
	int i;
	BYTE bLevel;

	if (!IsMultiGame)
		return FindGetItem(&pI->item) != -1;

	bLevel = pI->bLevel;
	net_assert(bLevel < NUM_LEVELS);

	pD = gsDeltaData.ddLevel[bLevel].item;
	// net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == DCMD_INVALID || !pI->item.PkItemEq(pD->item))
			continue;

		switch (pD->bCmd) {
		case DCMD_ITM_TAKEN:
			return false;
		case DCMD_ITM_SPAWNED:
		//	pD->bCmd = DCMD_ITM_TAKEN;
		//	return true;
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
		int n = pI->fromFloor - 1;
		net_assert(n < MAXITEMS);
		pD = gsDeltaData.ddLevel[bLevel].item;
		pD = &pD[n];
		if (pD->bCmd == DCMD_ITM_SPAWNED) {
			pD->bCmd = DCMD_ITM_TAKEN;
			return true;
		}
	}

	return false;
}

/*
 * Add an item to the delta of the given level.
 * @return -1: if there was no space, 0: if the item is a new item on the floor, 1: the item is supposed to be on the floor already
 */
static int delta_put_item(const PkItemStruct* pItem, BYTE bLevel, int x, int y)
{
	int i;
	DDItem* pD;

	if (!IsMultiGame)
		return 0;

	net_assert(bLevel < NUM_LEVELS);
	net_assert(gsDeltaData.ddLevelPlrs[bLevel] != 0);
	pD = gsDeltaData.ddLevel[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != DCMD_INVALID
		 && pD->item.PkItemEq(*pItem)) {
			bool onFloor = pD->bCmd != DCMD_ITM_TAKEN;
			if (!onFloor) {
				pD->bCmd = DCMD_ITM_MOVED;
				pD->x = x;
				pD->y = y;
			}
			return onFloor ? 1 : 0;
		}
	}

	pD = gsDeltaData.ddLevel[bLevel].item;
	for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd == DCMD_INVALID || pD->bCmd == DCMD_ITM_TAKEN) {
			pD->bCmd = pD->bCmd == DCMD_INVALID ? DCMD_ITM_DROPPED : DCMD_ITM_MOVED;
			pD->x = x;
			pD->y = y;
			copy_pod(pD->item, *pItem);
			return 0;
		}
	}

	return -1;
}

static void PackEar(PkItemStruct* dest, const ItemStruct* src)
{
	dest->wIndx = static_cast<uint16_t>(IDI_EAR);
	dest->wCI = *(uint16_t*)&src->_iPlrName[0];
	dest->dwSeed = *(int32_t*)&src->_iPlrName[2];
	dest->bId = src->_iPlrName[6];
	dest->bDur = src->_iPlrName[7];
	dest->bMDur = src->_iPlrName[8];
	dest->bCh = src->_iPlrName[9];
	dest->bMCh = src->_iPlrName[10];
	static_assert(MAXCHARLEVEL < (1 << 6), "PackPkItem stores the player level of ears in 6 bits.");
	dest->wValue = static_cast<uint16_t>(src->_ivalue | (src->_iPlrName[11] << 8) | ((src->_iCurs - ICURS_EAR_SORCERER) << 6));
	dest->dwBuff = *(uint32_t*)&src->_iPlrName[12];
	static_assert(sizeof(src->_iPlrName) == 12 + sizeof(dest->dwBuff), "packed ear-item is not stored correctly");
}

void PackPkItem(PkItemStruct* dest, const ItemStruct* src)
{
	if (src->_iIdx != IDI_EAR) {
		dest->dwSeed = src->_iSeed;
		dest->wIndx = src->_iIdx;
		dest->wCI = src->_iCreateInfo;
		dest->bId = src->_iIdentified;
		dest->bDur = src->_iDurability;
		dest->bMDur = src->_iMaxDur;
		dest->bCh = src->_iCharges;
		dest->bMCh = src->_iMaxCharges;
		static_assert(GOLD_MAX_LIMIT <= UINT16_MAX, "PackPkItem stores the gold value in 2 bytes.");
		static_assert(MAXCAMPAIGNSIZE <= 16, "PackPkItem stores the campaign status in 2 bytes.");
		dest->wValue = static_cast<uint16_t>(src->_ivalue);
	} else {
		PackEar(dest, src);
	}
}

void DeltaAddItem(int ii)
{
	ItemStruct* is;
	//int i;
	DDItem* pD;

	if (!IsMultiGame)
		return;

	is = &items[ii];
	is->_iSpawnIdx = ii + 1;
	// reserve the delta entry
	pD = gsDeltaData.ddLevel[currLvl._dLevelIdx].item;
	pD = &pD[ii];
	if (pD->bCmd == DCMD_INVALID) {
		pD->bCmd = DCMD_ITM_SPAWNED;
	}
	/*for (i = 0; i < MAXITEMS; i++, pD++) {
		if (pD->bCmd != DCMD_INVALID
		 && pD->item.dwSeed == is->_iSeed
		 && pD->item.wIndx == is->_iIdx
		 && pD->item.wCI == is->_iCreateInfo) {
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
	char* cursor = &items[MAXITEMS]._iPlrName[0];

	*(uint16_t*)&cursor[0] = src->wCI;
	*(int32_t*)&cursor[2] = src->dwSeed;
	cursor[6] = src->bId;
	cursor[7] = src->bDur;
	cursor[8] = src->bMDur;
	cursor[9] = src->bCh;
	cursor[10] = src->bMCh;
	cursor[11] = src->wValue >> 8;
	*(uint32_t*)&cursor[12] = src->dwBuff;
	cursor[16] = '\0';
	items[MAXITEMS]._iCurs = ((src->wValue >> 6) & 3) + ICURS_EAR_SORCERER;
	items[MAXITEMS]._ivalue = src->wValue & 0x3F;
	items[MAXITEMS]._iCreateInfo = SwapLE16(*(WORD*)&items[MAXITEMS]._iPlrName[0]);
	items[MAXITEMS]._iSeed = SwapLE32(*(DWORD*)&items[MAXITEMS]._iPlrName[2]);
}

void UnPackPkItem(const PkItemStruct* src)
{
	uint16_t idx = src->wIndx;
	uint16_t value;

	net_assert(idx < NUM_IDI);
	if (idx != IDI_EAR) {
		net_assert(((src->wCI & CF_TOWN) >> 8) <= CFL_CRAFTED);
		RecreateItem(
			src->dwSeed,
			src->wIndx,
			src->wCI);
		net_assert(items[MAXITEMS]._iMaxCharges >= src->bMCh);
		net_assert(src->bCh <= src->bMCh);
		// net_assert(items[MAXITEMS]._iMaxDur >= src->bMDur); -- HIDDEN_SHRINE
		net_assert(src->bDur <= src->bMDur);
		if (items[MAXITEMS]._iClass == ICLASS_ARMOR || items[MAXITEMS]._iClass == ICLASS_WEAPON) {
			net_assert(src->bDur != 0);
		}
		if (idx == IDI_GOLD) {
			value = src->wValue;
			net_assert(value <= GOLD_MAX_LIMIT);
			SetGoldItemValue(&items[MAXITEMS], value);
		} else if (idx == IDI_CAMPAIGNMAP) {
			items[MAXITEMS]._ivalue = src->wValue;
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
		// _mleaderflag out-of-sync because minions might be away when the leader dies
		assert(newFlag != MLEADER_SELF);
		return;
	}
	assert(prevFlag == MLEADER_PRESENT && newFlag == MLEADER_AWAY);
	monsters[mnum]._mleaderflag = newFlag;
	monsters[monsters[mnum]._mleader]._mpacksize--;
}

static void DeltaLoadAutomap(LocalLevel &level)
{
	int i;
	BYTE* pMapFlags;
	BYTE* pDunFlags;

	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in DeltaLoadAutomap.");
	static_assert(sizeof(level.automapsv) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in DeltaLoadAutomap.");
	pDunFlags = &dFlags[0][0];
	pMapFlags = &level.automapsv[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pDunFlags++, pMapFlags++) {
		assert((*pDunFlags & BFLAG_EXPLORED) == 0);
		*pDunFlags |= *pMapFlags & BFLAG_EXPLORED;
	}
}

void DeltaLoadLevel()
{
	DDMonster* mstr;
	DDObject* dstr;
	MonsterStruct* mon;
	DDItem* itm;
	int i;
	bool monInGame;

	assert(IsMultiGame);

	deltaload = true;
	if (currLvl._dLevelIdx != DLV_TOWN) {
		// load monsters
		for (i = 0; i < MAX_MINIONS; i++)
			InitGolemStats(i, gsDeltaData.ddJunk.jGolems[i]);

		mstr = gsDeltaData.ddLevel[currLvl._dLevelIdx].monster;
		for (i = 0; i < MAXMONSTERS; i++, mstr++) {
			if (mstr->dmCmd != DCMD_MON_INVALID) {
				mon = &monsters[i];
				monInGame = mon->_mmode <= MM_INGAME_LAST;
				// skip minions and prespawn skeletons
				if (monInGame)
					RemoveMonFromMap(i);
				SetMonsterLoc(mon, mstr->dmx, mstr->dmy);
				mon->_mdir = mstr->dmdir;
				UpdateLeader(i, mon->_mleaderflag, mstr->dmleaderflag);
				if (mstr->dmSIdx != 0) {
					net_assert(mstr->dmSIdx <= nummtypes);
					assert(mon->_mlid == NO_LIGHT);
					InitSummonedMonster(i, mon->_mdir, mstr->dmSIdx - 1, mon->_mx, mon->_my);
					if (!monInGame)
						nummonsters++;
				}
				// set hitpoints for dead monsters as well to ensure sync in multiplayer
				// games even on the first game_logic run
				mon->_mhitpoints = mstr->dmhitpoints;
#if 0 // commented out because the implementation is incomplete (e.g. what about hidden monsters)
				// SyncMonstersLight: inline for better performance + apply to moving monsters
				if (mon->_mlid != NO_LIGHT)
					ChangeLightXY(mon->_mlid, mon->_mx, mon->_my);
#endif
				static_assert(DCMD_MON_DESTROYED == DCMD_MON_DEAD + 1, "DeltaLoadLevel expects ordered DCMD_MON_ enum I.");
				static_assert(NUM_DCMD_MON == DCMD_MON_DESTROYED + 1, "DeltaLoadLevel expects ordered DCMD_MON_ enum II.");
				if (mstr->dmCmd >= DCMD_MON_DEAD) {
					if (mstr->dmCmd != DCMD_MON_DESTROYED)
						MonAddDead(i);
					// assert(mon->_mhitpoints == 0);
					// TODO: RemoveMonFromGame ?
					// reset squelch value to simplify MonFallenFear, sync_all_monsters and LevelDeltaExport
					mon->_msquelch = 0;
					mon->_mmode = i >= MAX_MINIONS ? ((mstr->dmCmd == DCMD_MON_DESTROYED || (mon->_mFlags & MFLAG_NOCORPSE)) ? MM_UNUSED : MM_DEAD) : MM_RESERVED;
					if (i >= MAX_MINIONS)
						nummonsters--;
					// SyncMonsterAnim(mnum);
					mon->_mAnimFrame = mon->_mAnims[MA_DEATH].maFrames;
					// mon->_mAnimCnt = -1;
					mon->_mAnimData = mon->_mAnims[MA_DEATH].maAnimData[mon->_mdir];
				} else {
					mon->_msquelch = mstr->dmactive;
					if (mon->_mmode == MM_RESERVED) {
						mon->_mmode = MM_STAND;
					}
					dMonster[mon->_mx][mon->_my] = i + 1;
					// SyncMonsterAnim(mnum);
					assert(mon->_mmode == MM_STAND);
					mon->_mAnimData = mon->_mAnims[MA_STAND].maAnimData[mon->_mdir];
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
		// SyncMonstersLight();

		DeltaLoadAutomap(gsDeltaData.ddLocal[currLvl._dLevelIdx]);

		// load objects
		dstr = gsDeltaData.ddLevel[currLvl._dLevelIdx].object;
		for (i = 0; i < MAXOBJECTS; i++, dstr++) {
			if (dstr->bCmd != DCMD_INVALID) {
				switch (dstr->bCmd) {
				case CMD_OPERATEOBJ:
					// SyncOpObject(-1, i);
					SyncOpObject(i);
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
				//case CMD_TRAPOPEN:
				//	SyncTrapOpen(i);
				//	break;
				//case CMD_TRAPCLOSE:
				//	SyncTrapClose(i);
				//	break;
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
			assert(items[i]._iSpawnIdx == i + 1);
			assert(dItem[items[i]._ix][items[i]._iy] == i + 1);
			DeleteItem(i);
		}
	}
	//  II. place items
	itm = gsDeltaData.ddLevel[currLvl._dLevelIdx].item;
	for (i = 0; i < MAXITEMS; i++, itm++) {
		if (itm->bCmd == DCMD_ITM_DROPPED || itm->bCmd == DCMD_ITM_MOVED) {
			UnPackPkItem(&itm->item);
			SyncPutItem(-1, itm->x, itm->y, &items[MAXITEMS], false);
		}
	}

	deltaload = false;
}

static void ExportItemDurabilites(int pnum, BYTE (&itemsDur)[NUM_INVELEM + 1])
{
	ItemStruct* is = &plr._pHoldItem;
	int i;
	static_assert(offsetof(PlayerStruct, _pHoldItem) + sizeof(ItemStruct) == offsetof(PlayerStruct, _pInvBody), "ExportItemDurabilites assumes packed items in PlayerStruct I.");
	static_assert(offsetof(PlayerStruct, _pInvBody) + NUM_INVLOC * sizeof(ItemStruct) == offsetof(PlayerStruct, _pSpdList), "ExportItemDurabilites assumes packed items in PlayerStruct II.");
	static_assert(offsetof(PlayerStruct, _pSpdList) + MAXBELTITEMS * sizeof(ItemStruct) == offsetof(PlayerStruct, _pInvList), "ExportItemDurabilites assumes packed items in PlayerStruct III.");
	static_assert(NUM_INVELEM == NUM_INVLOC + MAXBELTITEMS + NUM_INV_GRID_ELEM, "ExportItemDurabilites uses NUM_INVELEM to tool through the items in PlayerStruct.");
	for (i = 0; i < (1 + NUM_INVELEM); i++, is++) {
		itemsDur[i] = is->_iDurability;
	}
}
#if INET_MODE
static void ValidateDurability(ItemStruct* is, int pnum, int dur)
{
	if (pnum == mypnum || !ITYPE_DURABLE(is->_itype)) {
		net_assert(is->_iDurability == dur || is->_itype == ITYPE_NONE || is->_itype == ITYPE_PLACEHOLDER || is->_iIdx == IDI_EAR);
	} else {
		net_assert(is->_iMaxDur >= dur);
		net_assert(dur != 0);
	}
}
#endif
static void ImportItemDurabilities(int pnum, const BYTE (&itemsDur)[NUM_INVELEM + 1])
{
	ItemStruct* is = &plr._pHoldItem;
	int i, dur;
	static_assert(offsetof(PlayerStruct, _pHoldItem) + sizeof(ItemStruct) == offsetof(PlayerStruct, _pInvBody), "ImportItemDurabilities assumes packed items in PlayerStruct I.");
	static_assert(offsetof(PlayerStruct, _pInvBody) + NUM_INVLOC * sizeof(ItemStruct) == offsetof(PlayerStruct, _pSpdList), "ImportItemDurabilities assumes packed items in PlayerStruct II.");
	static_assert(offsetof(PlayerStruct, _pSpdList) + MAXBELTITEMS * sizeof(ItemStruct) == offsetof(PlayerStruct, _pInvList), "ImportItemDurabilities assumes packed items in PlayerStruct III.");
	static_assert(NUM_INVELEM == NUM_INVLOC + MAXBELTITEMS + NUM_INV_GRID_ELEM, "ImportItemDurabilities uses NUM_INVELEM to tool through the items in PlayerStruct.");
	for (i = 0; i < (1 + NUM_INVELEM); i++, is++) {
		dur = itemsDur[i];
#if INET_MODE
		ValidateDurability(is, pnum, dur);
#endif
		is->_iDurability = dur;
	}
}

void NetSendCmdJoinLevel()
{
	TCmdJoinLevel cmd;

	cmd.bCmd = CMD_JOINLEVEL;
	cmd.lLevel = myplr._pDunLevel;
	cmd.px = myview.x;
	cmd.py = myview.y;
	cmd.php = myplr._pHPBase;
	cmd.pmp = myplr._pManaBase;
	cmd.lTimer1 = myplr._pTimer[PLTR_INFRAVISION];
	cmd.lTimer2 = myplr._pTimer[PLTR_RAGE];
	cmd.pManaShield = myplr._pManaShield;
	cmd.iFloorItems = numitems;

	ExportItemDurabilites(mypnum, cmd.itemsDur);

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void LevelDeltaExport()
{
	LargeMsgPkt* pkt = &gsDeltaData.ddSendRecvPkt;
	DBuffer* buff = &pkt->apMsg.tpData;
	LDLevel* lvlData;

	int pnum, mnum, i, mi;
	MonsterStruct* mon;
	MissileStruct* mis;
	bool validDelta, completeDelta;
	BYTE* dst;
	unsigned recipients = 0, ulen;

	validDelta = currLvl._dLevelIdx != DLV_INVALID;
	completeDelta = false;
#ifndef NONET
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!(guSendLevelData & (1 << pnum)) || // pnum did not request a level-delta
//		  (guOweLevelDelta & (1 << pnum) == 0) || // got an (empty) level delta from pnum
		  (!validDelta && plr._pDunLevel == myplr._pDunLevel // both players are loading the same level ->
		   && (guRequestLevelData[pnum] > guRequestLevelData[mypnum] || (guRequestLevelData[pnum] == guRequestLevelData[mypnum] && pnum > mypnum)))) { // ignore lower priority requests 	TODO: overflow hickup
			; // skip
		} else {
			guSendLevelData &= ~(1 << pnum);
			completeDelta |= pnum != mypnum && plr._pDunLevel == myplr._pDunLevel;
			recipients |= (1 << pnum);
		}
	}
#else
	guSendLevelData = 0;
#endif
	completeDelta &= validDelta;
	// send detailed level delta only if available and appropriate
	if (completeDelta) {
		// can not be done during lvl-delta load, otherwise a separate buffer should be used
		assert(geBufferMsgs != MSG_LVL_DELTA_WAIT && geBufferMsgs != MSG_LVL_DELTA_PROC);
		static_assert(sizeof(LDLevel) <= sizeof(buff->content), "Level-Delta does not fit to the buffer.");
		lvlData = (LDLevel*)buff->content;

		static_assert(MAXMONSTERS <= UCHAR_MAX, "Monster indices are transferred as BYTEs I.");
		lvlData->ldNumMonsters = nummonsters;

		static_assert(MAXMISSILES <= UCHAR_MAX, "Missile indices are transferred as BYTEs I.");
		for (i = 0; i < MAXMISSILES; i++) {
			lvlData->ldMissActive[i] = missileactive[i];
		}

		dst = &lvlData->ldContent[0];
		// export the players
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			TSyncLvlPlayer* DVL_RESTRICT tplr = (TSyncLvlPlayer*)dst;
			if (!plr._pActive || plr._pDunLevel != currLvl._dLevelIdx || plr._pLvlChanging) {
				tplr->spMode = PM_INVALID;
				dst++;
				continue;
			}

			tplr->spMode = plr._pmode;
			tplr->spManaShield = plr._pManaShield;
			tplr->spInvincible = plr._pInvincible;
			tplr->spDestAction = plr._pDestAction;
			tplr->spDestParam1 = plr._pDestParam1;
			tplr->spDestParam2 = plr._pDestParam2;
			tplr->spDestParam3 = plr._pDestParam3;
			tplr->spDestParam4 = plr._pDestParam4;
			tplr->spTimer[PLTR_INFRAVISION] = plr._pTimer[PLTR_INFRAVISION];
			tplr->spTimer[PLTR_RAGE] = plr._pTimer[PLTR_RAGE];
			tplr->spx = plr._px;
			tplr->spy = plr._py;
			//tplr->spfutx = plr._pfutx;
			//tplr->spfuty = plr._pfuty;
			//tplr->spoldx = plr._poldx;
			//tplr->spoldy = plr._poldy;
			//tplr->spxoff = plr._pxoff;
			//tplr->spyoff = plr._pyoff;
			tplr->spdir = plr._pdir;
			tplr->spAnimFrame = plr._pAnimFrame;
			tplr->spAnimCnt = plr._pAnimCnt;
			tplr->spHPBase = plr._pHPBase;
			tplr->spManaBase = plr._pManaBase;
			tplr->spVar1 = plr._pVar1;
			tplr->spVar2 = plr._pVar2;
			tplr->spVar3 = plr._pVar3;
			tplr->spVar4 = plr._pVar4;
			tplr->spVar5 = plr._pVar5;
			tplr->spVar6 = plr._pVar6;
			tplr->spVar7 = plr._pVar7;
			tplr->spVar8 = plr._pVar8;

			ExportItemDurabilites(pnum, tplr->spItemsDur);

			dst += sizeof(TSyncLvlPlayer);
		}
		// export the monsters
		for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
			mon = &monsters[mnum];
			if (mon->_msquelch == 0) {
				continue; // assume it is the same as in delta
			}
			net_assert(mon->_mmode <= MM_INGAME_LAST);
			TSyncLvlMonster* DVL_RESTRICT tmon = (TSyncLvlMonster*)dst;
			tmon->smMnum = static_cast<uint16_t>(mnum);
			tmon->smMode = mon->_mmode;
			tmon->smSquelch = mon->_msquelch;
			//tmon->smPathcount = mon->_mpathcount; // unused
			//tmon->smAlign_1 = mon->_mAlign_1;     // unused
			tmon->smGoal = mon->_mgoal;
			tmon->smGoalvar1 = mon->_mgoalvar1;
			tmon->smGoalvar2 = mon->_mgoalvar2;
			tmon->smGoalvar3 = mon->_mgoalvar3;
			tmon->smx = mon->_mx;
			tmon->smy = mon->_my;
			//tmon->smfutx = mon->_mfutx;
			//tmon->smfuty = mon->_mfuty;
			//tmon->smoldx = mon->_moldx;
			//tmon->smoldy = mon->_moldy;
			//tmon->smxoff = mon->_mxoff;
			//tmon->smyoff = mon->_myoff;
			tmon->smdir = mon->_mdir;
			tmon->smenemy = mon->_menemy;
			tmon->smenemyx = mon->_menemyx;
			tmon->smenemyy = mon->_menemyy;
			tmon->smListener = mon->_mListener;
			tmon->smDelFlag = mon->_mDelFlag; // unused
			tmon->smAnimCnt = mon->_mAnimCnt;
			tmon->smAnimFrame = mon->_mAnimFrame;
			// assert(!mon->_mDelFlag || mon->_mmode == MM_STONE);
			tmon->smVar1 = mon->_mVar1;
			tmon->smVar2 = mon->_mVar2;
			tmon->smVar3 = mon->_mVar3;
			tmon->smVar4 = mon->_mVar4;
			tmon->smVar5 = mon->_mVar5;
			tmon->smVar6 = mon->_mVar6;
			tmon->smVar7 = mon->_mVar7;
			tmon->smVar8 = mon->_mVar8;
			tmon->smHitpoints = mon->_mhitpoints;
			tmon->smlastx = mon->_mlastx;
			tmon->smlasty = mon->_mlasty;
			//tmon->smLeader = mon->_mleader;
			tmon->smLeaderflag = mon->_mleaderflag;
			//tmon->smPacksize = mon->_mpacksize;
	//BYTE falign_CB;
			tmon->smFlags = mon->_mFlags;

			dst += sizeof(TSyncLvlMonster);
		}
		// export the missiles
		for (i = 0; i < nummissiles; i++) {
			mi = missileactive[i];
			mis = &missile[mi];

			//assert(!mis->_miDelFlag);
			TSyncLvlMissile* DVL_RESTRICT tmis = (TSyncLvlMissile*)dst;
			tmis->smiMi = static_cast<uint16_t>(mi + MAXMONSTERS);
			tmis->smiType = mis->_miType;
			//BYTE _miFlags;
			//BYTE _miResist;
			tmis->smiFileNum = mis->_miFileNum;
			//BOOLEAN _miDelFlag;
			//int _miUniqTrans;
			//BOOLEAN _miDrawFlag;
			//BOOLEAN _miAnimFlag;
			//BOOLEAN _miLightFlag;
			//BOOLEAN _miPreFlag;
			//BYTE* _miAnimData;
			//int _miAnimFrameLen;
			//int _miAnimLen;
			//int _miAnimWidth;
			//int _miAnimXOffset;
			tmis->smiAnimCnt = mis->_miAnimCnt;
			tmis->smiAnimAdd = mis->_miAnimAdd;
			tmis->smiAnimFrame = mis->_miAnimFrame;
			tmis->smiDir = mis->_miDir;
			tmis->smisx = mis->_misx;
			tmis->smisy = mis->_misy;
			tmis->smix = mis->_mix;
			tmis->smiy = mis->_miy;
			tmis->smixoff = mis->_mixoff / ASSET_MPL;
			tmis->smiyoff = mis->_miyoff / ASSET_MPL;
			tmis->smixvel = mis->_mixvel;
			tmis->smiyvel = mis->_miyvel;
			tmis->smitxoff = mis->_mitxoff;
			tmis->smityoff = mis->_mityoff;
			// smiDir/_miDir reordered for better alignment
			tmis->smiSpllvl = mis->_miSpllvl; // int?
			tmis->smiSource = mis->_miSource; // int?
			tmis->smiCaster = mis->_miCaster; // int?
			tmis->smiMinDam = mis->_miMinDam;
			tmis->smiMaxDam = mis->_miMaxDam;
			// tmis->smiRndSeed = mis->_miRndSeed;
			tmis->smiRange = mis->_miRange;
			// smiLidRadius/_miLid reordered for better alignment
			tmis->smiVar1 = mis->_miVar1;
			tmis->smiVar2 = mis->_miVar2;
			tmis->smiVar3 = mis->_miVar3;
			tmis->smiVar4 = mis->_miVar4;
			tmis->smiVar5 = mis->_miVar5;
			tmis->smiVar6 = mis->_miVar6;
			tmis->smiVar7 = mis->_miVar7;
			tmis->smiVar8 = mis->_miVar8;
			tmis->smiLidRadius = mis->_miLid == NO_LIGHT ? 0 : LightList[mis->_miLid]._lradius;

			dst += sizeof(TSyncLvlMissile);
		}
		ulen = (unsigned)((size_t)dst - (size_t)&lvlData->ldContent[0]);
		static_assert(sizeof(lvlData->ldContent) <= UINT16_MAX, "Length of ldContent can not be sent with a WORD field.");
		lvlData->wLen = static_cast<uint16_t>(ulen);
		// send the data to the recipients
		multi_send_large_msg(recipients, NMSG_LVL_DELTA, offsetof(LDLevel, ldContent) + ulen);
	}
	// current number of chunks sent + level + turn-id + end
	LevelDeltaEnd* deltaEnd = (LevelDeltaEnd*)buff->content;
	deltaEnd->numChunks = completeDelta ? 1 : 0;
	deltaEnd->level = currLvl._dLevelIdx;
	deltaEnd->turn = gdwLastGameTurn;
	multi_send_large_msg(recipients, NMSG_LVL_DELTA_END, sizeof(*deltaEnd));
}

void LevelDeltaLoad()
{
	int pnum, mnum, mi;
	MonsterStruct* mon;
	MissileStruct* mis;
	const MissileData* mds;
	BYTE* src;
	WORD wLen;
	LDLevel* lvlData;

	// assert(IsMultiGame);
	// reset squelch set from delta, the message should contain more up-to-date info
	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		monsters[mnum]._msquelch = 0;
	}

	lvlData = (LDLevel*)gsDeltaData.ddSendRecvPkt.apMsg.tpData.content;
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
		TSyncLvlPlayer* DVL_RESTRICT tplr = (TSyncLvlPlayer*)src;
		if (tplr->spMode == PM_INVALID) {
			src++;
			continue;
		}
		if (pnum == mypnum) {
			mi = tplr->spMode;
			net_assert(mi == PM_STAND
			 || ((mi == PM_DEATH || mi == PM_DYING) && plr._pHitPoints == 0));
			net_assert(tplr->spDestAction == ACTION_NONE);
			net_assert(tplr->spInvincible == 40);
			net_assert(plr._pTimer[PLTR_INFRAVISION] == tplr->spTimer[PLTR_INFRAVISION]);
			net_assert(plr._pTimer[PLTR_RAGE] == tplr->spTimer[PLTR_RAGE]);
			net_assert(plr._pManaShield == tplr->spManaShield);
			net_assert(plr._pHPBase == tplr->spHPBase || (plr._pHitPoints == 0 && currLvl._dLevelIdx == DLV_TOWN));
			net_assert(plr._pManaBase == tplr->spManaBase);
		}
		// RemovePlrFromMap(pnum);
		if (dPlayer[plr._px][plr._py] == pnum + 1)
			dPlayer[plr._px][plr._py] = 0;
		plr._pmode = tplr->spMode;
		plr._pManaShield = tplr->spManaShield;
		plr._pInvincible = tplr->spInvincible;
		plr._pDestAction = tplr->spDestAction;
		plr._pDestParam1 = tplr->spDestParam1;
		plr._pDestParam2 = tplr->spDestParam2;
		plr._pDestParam3 = tplr->spDestParam3;
		plr._pDestParam4 = tplr->spDestParam4;
		plr._pTimer[PLTR_INFRAVISION] = tplr->spTimer[PLTR_INFRAVISION];
		plr._pTimer[PLTR_RAGE] = tplr->spTimer[PLTR_RAGE];
		plr._px = tplr->spx;
		plr._py = tplr->spy;
		// plr._pfutx = tplr->spfutx;
		// plr._pfuty = tplr->spfuty;
		// plr._poldx = tplr->spoldx;
		// plr._poldy = tplr->spoldy;
		// plr._pxoff = tplr->spxoff;
		// plr._pyoff = tplr->spyoff;
		plr._pxoff = plr._pyoff = 0; // no need to sync these values as they are recalculated when used
		plr._pdir = tplr->spdir;
		plr._pAnimFrame = tplr->spAnimFrame;
		plr._pAnimCnt = tplr->spAnimCnt;
		plr._pHPBase = tplr->spHPBase;
		plr._pManaBase = tplr->spManaBase;
		plr._pVar1 = tplr->spVar1;
		plr._pVar2 = tplr->spVar2;
		plr._pVar3 = tplr->spVar3;
		plr._pVar4 = tplr->spVar4;
		plr._pVar5 = tplr->spVar5;
		plr._pVar6 = tplr->spVar6;
		plr._pVar7 = tplr->spVar7;
		plr._pVar8 = tplr->spVar8;

		ImportItemDurabilities(pnum, tplr->spItemsDur);

		src += sizeof(TSyncLvlPlayer);
		{ // calculate the player's (future/old) position based on its mode
			int pdir = plr._pdir;
			int px , py;
			net_assert(pdir < NUM_DIRS);
			px = plr._px;
			py = plr._py;
			net_assert(IN_DUNGEON_AREA(px, py));
			mi = plr._pmode;
			net_assert(mi < NUM_PLR_MODES);
			if (mi == PM_WALK || mi == PM_WALK2) {
				if (plr._pmode == PM_WALK) {
					plr._poldx = px;
					plr._poldy = py;

					px += offset_x[pdir];
					py += offset_y[pdir];
					plr._pfutx = px;
					plr._pfuty = py;
				} else {
					plr._pfutx = px;
					plr._pfuty = py;

					px -= offset_x[pdir];
					py -= offset_y[pdir];
					plr._poldx = px;
					plr._poldy = py;
				}
				net_assert(IN_DUNGEON_AREA(px, py));
			} else {
				// SetPlayerLoc(&plr, px, py);
				plr._pfutx = plr._poldx = px;
				plr._pfuty = plr._poldy = py;
			}
		}

		InitLvlPlayer(pnum, false);
	}

	wLen = lvlData->wLen;
	wLen -= static_cast<uint16_t>((size_t)src - (size_t)(&lvlData->ldContent[0]));
	// load monsters
	for ( ; wLen >= sizeof(TSyncLvlMonster); wLen -= sizeof(TSyncLvlMonster)) {
		TSyncLvlMonster* DVL_RESTRICT tmon = (TSyncLvlMonster*)src;
		mnum = tmon->smMnum;
		if (mnum >= MAXMONSTERS)
			break;

		mon = &monsters[mnum];
		// RemoveMonFromMap(mnum);
		if (dMonster[mon->_mx][mon->_my] == mnum + 1)
			dMonster[mon->_mx][mon->_my] = 0;

		UpdateLeader(mnum, mon->_mleaderflag, tmon->smLeaderflag);

		mon->_mmode = tmon->smMode;
		mon->_msquelch = tmon->smSquelch;
		//mon->_mpathcount = tmon->smPathcount;
		//mon->_mAlign_1 = tmon->smAlign_1;
		mon->_mgoal = tmon->smGoal;
		mon->_mgoalvar1 = tmon->smGoalvar1;
		mon->_mgoalvar2 = tmon->smGoalvar2;
		mon->_mgoalvar3 = tmon->smGoalvar3;
		mon->_mx = tmon->smx;
		mon->_my = tmon->smy;
		//mon->_mfutx = tmon->smfutx;
		//mon->_mfuty = tmon->smfuty;
		//mon->_moldx = tmon->smoldx;
		//mon->_moldy = tmon->smoldy;
		//mon->_mxoff = tmon->smxoff;
		//mon->_myoff = tmon->smyoff;
		mon->_mxoff = mon->_myoff = 0;        // no need to sync these values as they are recalculated when used
		mon->_mdir = tmon->smdir;
		mon->_menemy = tmon->smenemy;
		mon->_menemyx = tmon->smenemyx;
		mon->_menemyy = tmon->smenemyy;
		mon->_mListener = tmon->smListener;
		mon->_mDelFlag = tmon->smDelFlag;
		mon->_mAnimCnt = tmon->smAnimCnt;
		mon->_mAnimFrame = tmon->smAnimFrame;
		mon->_mVar1 = tmon->smVar1;
		mon->_mVar2 = tmon->smVar2;
		mon->_mVar3 = tmon->smVar3;
		mon->_mVar4 = tmon->smVar4;
		mon->_mVar5 = tmon->smVar5;
		mon->_mVar6 = tmon->smVar6;
		mon->_mVar7 = tmon->smVar7;
		mon->_mVar8 = tmon->smVar8;
		mon->_mhitpoints = tmon->smHitpoints;
		mon->_mlastx = tmon->smlastx;
		mon->_mlasty = tmon->smlasty;
		//BYTE _mleader; // the leader of the monster
		//mon->_mleaderflag = tmon->smLeaderflag; // the status of the monster's leader
		//BYTE _mpacksize; // the number of 'pack'-monsters close to their leader
		//BYTE _mvid; // vision id of the monster (for minions only)
		mon->_mFlags = tmon->smFlags;
		// move the light of the monster
		// assert(mon->_mlid == NO_LIGHT || (LightList[mon->_mlid]._lx == mx && LightList[mon->_mlid]._ly == my));
		//if (mon->_mlid != NO_LIGHT)
		//	ChangeLightXY(mon->_mlid, mon->_moldx, mon->_moldy);
		// place the monster
		mi = mon->_mmode;
		net_assert(mi <= MM_INGAME_LAST);
		if (mi != MM_STONE || mon->_mhitpoints != 0) {
			// calculate the monster's (future/old) position based on its mode
			int mdir, mx, my;
			if (mi == MM_STONE)
				mi = mon->_mVar3;
			net_assert(mi <= MM_INGAME_LAST);
			mdir = mon->_mdir;
			net_assert(mdir < NUM_DIRS);
			mx = mon->_mx;
			my = mon->_my;
			net_assert(IN_ACTIVE_AREA(mx, my));
			if (mi == MM_WALK || mi == MM_WALK2) {
				if (mon->_mmode == MM_WALK) {
					mon->_moldx = mx;
					mon->_moldy = my;

					mx += offset_x[mdir];
					my += offset_y[mdir];
					mon->_mfutx = mx;
					mon->_mfuty = my;
				} else {
					mon->_mfutx = mx;
					mon->_mfuty = my;

					mx -= offset_x[mdir];
					my -= offset_y[mdir];
					mon->_moldx = mx;
					mon->_moldy = my;
				}
				net_assert(IN_ACTIVE_AREA(mx, my));
				// net_assert(PosOkMonster(mnum, mx, my));
			} else {
				// net_assert(PosOkMonster(mnum, mx, my));
				// SetMonsterLoc(mon, mx, my);
				mon->_mfutx = mon->_moldx = mx;
				mon->_mfuty = mon->_moldy = my;
			}
			// InitLvlMonster
			dMonster[mon->_mx][mon->_my] = mnum + 1;
			if (mi == MM_WALK2) {
				dMonster[mon->_moldx][mon->_moldy] = -(mnum + 1);
			} else if (mi == MM_WALK) {
				dMonster[mon->_mfutx][mon->_mfuty] = -(mnum + 1);
			} else if (mi == MM_CHARGE) {
				dMonster[mon->_mx][mon->_my] = -(mnum + 1);
			}
			// ensure dead bodies are not placed prematurely
			if (mi == MM_DEATH) {
				net_assert(mon->_mhitpoints == 0);
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
		TSyncLvlMissile* DVL_RESTRICT tmis = (TSyncLvlMissile*)src;
		net_assert(tmis->smiMi >= MAXMONSTERS);
		mi = tmis->smiMi - MAXMONSTERS;
		net_assert((unsigned)mi < MAXMISSILES);
		net_assert(nummissiles < MAXMISSILES);
		net_assert(missileactive[nummissiles] == mi);
		nummissiles++;
		mis = &missile[mi];
		memset(mis, 0, sizeof(*mis));

		mis->_miType = tmis->smiType;
		//BYTE _miFlags;
		//BYTE _miResist;
		mis->_miFileNum = tmis->smiFileNum;
		//BOOLEAN _miDelFlag;
		//int _miUniqTrans;
		//BOOLEAN _miDrawFlag;
		//BOOLEAN _miAnimFlag;
		//BOOLEAN _miLightFlag;
		//BOOLEAN _miPreFlag;
		mis->_miAnimCnt = tmis->smiAnimCnt;
		mis->_miAnimAdd = tmis->smiAnimAdd;
		mis->_miAnimFrame = tmis->smiAnimFrame;
		mis->_misx = tmis->smisx;
		mis->_misy = tmis->smisy;
		mis->_mix = tmis->smix;
		mis->_miy = tmis->smiy;
		mis->_mixoff = tmis->smixoff * ASSET_MPL;
		mis->_miyoff = tmis->smiyoff * ASSET_MPL;
		mis->_mixvel = tmis->smixvel;
		mis->_miyvel = tmis->smiyvel;
		mis->_mitxoff = tmis->smitxoff;
		mis->_mityoff = tmis->smityoff;
		mis->_miDir = tmis->smiDir;
		mis->_miSpllvl = tmis->smiSpllvl;       // int?
		mis->_miSource = tmis->smiSource;       // int?
		mis->_miCaster = tmis->smiCaster;       // int?
		mis->_miMinDam = tmis->smiMinDam;
		mis->_miMaxDam = tmis->smiMaxDam;
		// mis->_miRndSeed = tmis->smiRndSeed;
		mis->_miRange = tmis->smiRange;
		mis->_miVar1 = tmis->smiVar1;
		mis->_miVar2 = tmis->smiVar2;
		mis->_miVar3 = tmis->smiVar3;
		mis->_miVar4 = tmis->smiVar4;
		mis->_miVar5 = tmis->smiVar5;
		mis->_miVar6 = tmis->smiVar6;
		mis->_miVar7 = tmis->smiVar7;
		mis->_miVar8 = tmis->smiVar8;

		if (tmis->smiLidRadius != 0) {
			net_assert(tmis->smiLidRadius <= MAX_LIGHT_RAD);
			mis->_miLid = AddLight(mis->_mix, mis->_miy, tmis->smiLidRadius);
		} else {
			mis->_miLid = NO_LIGHT;
		}
		mds = &missiledata[mis->_miType];
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
		if (plr._pmode != PM_DYING && plr._pmode != PM_DEATH) {
			int px = plr._pfutx, py = plr._pfuty;
			net_assert(PosOkPlayer(pnum, px, py));
			px = plr._poldx, py = plr._poldy;
			net_assert(PosOkPlayer(pnum, px, py));
			switch (plr._pmode) {
			case PM_ATTACK:
				net_assert((unsigned)plr._pVar5 < NUM_SPELLS); // ATTACK_SKILL
				net_assert(plr._pVar6 >= 0);                   // ATTACK_SKILL_LEVEL
				break;
			case PM_RATTACK:
				net_assert((unsigned)plr._pVar5 < NUM_SPELLS); // RATTACK_SKILL
				net_assert(plr._pVar6 >= 0);                   // RATTACK_SKILL_LEVEL
				break;
			case PM_SPELL:
				net_assert(IN_ACTIVE_AREA(plr._pVar1, plr._pVar2)); // SPELL_TARGET_X, SPELL_TARGET_Y
				net_assert((unsigned)plr._pVar5 < NUM_SPELLS);                        // SPELL_NUM
				net_assert(plr._pVar6 >= 0);                                          // SPELL_LEVEL
				break;
			}
		} else {
			net_assert(plr._pHitPoints == 0);
		}
		switch (plr._pDestAction) {
		case ACTION_NONE:
			break;
		case ACTION_WALK:
			net_assert(IN_ACTIVE_AREA(plr._pDestParam1, plr._pDestParam2));
			break;
		case ACTION_OPERATE:
			net_assert(IN_ACTIVE_AREA(plr._pDestParam1, plr._pDestParam2));
			net_assert(plr._pDestParam3 == SPL_ATTACK);
			net_assert((unsigned)plr._pDestParam4 < MAXOBJECTS);
			net_assert(abs(dObject[plr._pDestParam1][plr._pDestParam2]) == plr._pDestParam4 + 1);
			break;
		case ACTION_WALKDIR:
		case ACTION_TURN:
		case ACTION_BLOCK:
			net_assert((unsigned)plr._pDestParam1 < NUM_DIRS);
			break;
		case ACTION_ATTACKMON:
		case ACTION_RATTACKMON:
		case ACTION_SPELLMON:
			net_assert((unsigned)plr._pDestParam1 < MAXMONSTERS);
			net_assert((unsigned)plr._pDestParam3 < NUM_SPELLS); // ATTACK_SKILL, SPELL_NUM
			net_assert(plr._pDestParam4 >= 0);         // ATTACK_SKILL_LEVEL, SPELL_LEVEL
			break;
		case ACTION_ATTACK:
		case ACTION_RATTACK:
			net_assert((unsigned)plr._pDestParam3 < NUM_SPELLS); // ATTACK_SKILL
			net_assert(plr._pDestParam4 >= 0);         // ATTACK_SKILL_LEVEL
			break;
		case ACTION_ATTACKPLR:
		case ACTION_RATTACKPLR:
		case ACTION_SPELLPLR:
			net_assert((unsigned)plr._pDestParam1 < MAX_PLRS);
			net_assert((unsigned)plr._pDestParam3 < NUM_SPELLS); // ATTACK_SKILL, SPELL_NUM
			net_assert(plr._pDestParam4 >= 0);         // ATTACK_SKILL_LEVEL, SPELL_LEVEL
			break;
		case ACTION_SPELL:
			net_assert(IN_ACTIVE_AREA(plr._pDestParam1, plr._pDestParam2)); // SPELL_TARGET_X, SPELL_TARGET_Y
			net_assert((unsigned)plr._pDestParam3 < NUM_SPELLS);                              // SPELL_NUM
			net_assert(plr._pDestParam4 >= 0);                                                // SPELL_LEVEL
			if (plr._pDestParam3 == SPL_DISARM)
				net_assert((unsigned)plr._pDestParam4 < MAXOBJECTS); // fake SPELL_LEVEL
			if (plr._pDestParam3 == SPL_TELEKINESIS) {
				switch (plr._pDestParam4 >> 16) {
				case MTT_ITEM:
					net_assert((plr._pDestParam4 & 0xFFFF) < MAXITEMS);
					break;
				case MTT_MONSTER:
					net_assert((plr._pDestParam4 & 0xFFFF) < MAXMONSTERS);
					break;
				case MTT_OBJECT:
					net_assert((plr._pDestParam4 & 0xFFFF) < MAXOBJECTS);
					break;
				case MTT_PLAYER:
					net_assert((plr._pDestParam4 & 0xFFFF) < MAX_PLRS);
					break;
				default:
					net_assert(0);
					break;
				}
			}
			break;
		case ACTION_PICKUPITEM:
			net_assert(IN_ACTIVE_AREA(plr._pDestParam1, plr._pDestParam2));
			net_assert((unsigned)plr._pDestParam4 < MAXITEMS);
			break;
		case ACTION_TALK:
			net_assert((unsigned)plr._pDestParam1 < MAXMONSTERS);
			break;
		default:
			net_assert(0);
		}
		net_assert(plr._pAnimFrame <= plr._pAnimLen);
	}

	// ProcessLightList();
	// ProcessVisionList();
}

static void LevelDeltaImportEnd(const TMsgLarge* cmd, int pnum)
{
	const LevelDeltaEnd* buf;

	guOweLevelDelta &= ~(1 << pnum);

	net_assert(cmd->tpHdr.wBytes == sizeof(cmd->tpData.compressed) + sizeof(LevelDeltaEnd));
	static_assert(NET_COMP_MSG_SIZE > sizeof(LevelDeltaEnd), "LevelDeltaImportEnd does not decompress the final message.");
	net_assert(!cmd->tpData.compressed);
	buf = (const LevelDeltaEnd*)cmd->tpData.content;
	if (buf->numChunks == 0)
		return; // empty delta -> not done yet
	net_assert(buf->level == myplr._pDunLevel);

	if (gsDeltaData.ddRecvLastCmd == NMSG_LVL_DELTA_END) {
		//gbGameDeltaChunks = DELTA_ERROR_FAIL_1;
		guOweLevelDelta |= 1 << pnum;
		return; // lost or duplicated package -> ignore and expect a timeout
	}

	// decompress level data
	//assert(gsDeltaData.ddRecvLastCmd == NMSG_LVL_DELTA);
	DeltaDecompressData();

	guDeltaTurn = buf->turn; // TODO: validate that it is in the near future
	//gbGameDeltaChunks = MAX_CHUNKS - 1;
	// switch to delta-processing mode
	geBufferMsgs = MSG_LVL_DELTA_PROC;
}

static unsigned On_LVL_DELTA(const TCmd* pCmd, int pnum)
{
	const TMsgLarge* cmd = (const TMsgLarge*)pCmd;

	if (geBufferMsgs != MSG_LVL_DELTA_WAIT)
		goto done; // the player is already active -> drop the packet

	if (cmd->tpHdr.bCmd == NMSG_LVL_DELTA_END) {
		// final package -> done
		LevelDeltaImportEnd(cmd, pnum);
		goto done;
	}

	if (cmd->tpHdr.bCmd != NMSG_LVL_DELTA) {
		// invalid data type -> drop the packet
		//gbGameDeltaChunks = DELTA_ERROR_FAIL_2;
		goto done;
	}
	net_assert((gsDeltaData.ddSendRecvOffset + cmd->tpHdr.wBytes) <= sizeof(gsDeltaData.ddSendRecvPkt.apMsg.tpData));
	memcpy(((BYTE*)&gsDeltaData.ddSendRecvPkt.apMsg.tpData), &cmd->tpData, cmd->tpHdr.wBytes);
	gsDeltaData.ddRecvLastCmd = cmd->tpHdr.bCmd;
	gsDeltaData.ddSendRecvOffset = cmd->tpHdr.wBytes;
done:
	return cmd->tpHdr.wBytes + sizeof(cmd->tpHdr);
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
	cmd.mkMnum = static_cast<uint16_t>(mnum);
	mon = &monsters[mnum];
	cmd.mkExp = mon->_mExp;
	cmd.mkMonLevel = mon->_mLevel;
	cmd.mkParam1.x = mon->_mx;
	cmd.mkParam1.y = mon->_my;
	cmd.mkDir = (!(mon->_mFlags & MFLAG_NOCORPSE) && mon->_mmode != MM_STONE) ? mon->_mdir : NUM_DIRS;
	cmd.mkParam1.bParam1 = currLvl._dLevelIdx;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdGolem(BYTE x, BYTE y, BYTE lvl)
{
	TCmdGolem cmd;

	cmd.bCmd = CMD_AWAKEGOLEM;
	cmd.goX = x;
	cmd.goY = y;
	cmd.goMonLevel = lvl;
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

void NetSendCmdLocParam1(BYTE bCmd, BYTE x, BYTE y, uint16_t wParam1)
{
	TCmdLocParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.x = x;
	cmd.y = y;
	cmd.wParam1 = wParam1;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdParam1(BYTE bCmd, uint16_t wParam1)
{
	TCmdParam1 cmd;

	cmd.bCmd = bCmd;
	cmd.wParam1 = wParam1;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdParamBW(BYTE bCmd, BYTE bParam1, uint16_t wParam2)
{
	TCmdParamBW cmd;

	cmd.bCmd = bCmd;
	cmd.byteParam = bParam1;
	cmd.wordParam = wParam2;

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
	cmd.shSeed = seed;

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

void SendStoreCmd1(BYTE idx, BYTE bStoreId, int value)
{
	TCmdStore1 cmd;

	cmd.bCmd = CMD_STORE_1;
	cmd.stCmd = bStoreId;
	cmd.stLoc = idx;
	cmd.stValue = value;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void SendStoreCmd2(BYTE bStoreId)
{
	TCmdStore2 cmd;

	cmd.bCmd = CMD_STORE_2;
	cmd.stCmd = bStoreId;
	PackPkItem(&cmd.item, &storeitem);
	cmd.stValue = storeitem._iIvalue;

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
	cmd.fromFloor = is->_iSpawnIdx;

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

void NetSendCmdItemSkill(BYTE cii, const CmdSkillUse skillUse)
{
	TCmdItemOp cmd;

	cmd.bCmd = CMD_OPERATEITEM;
	cmd.ioIdx = cii;
	cmd.iou = skillUse;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLocSkill(BYTE x, BYTE y, const CmdSkillUse skillUse)
{
	TCmdLocSkill cmd;

	cmd.bCmd = CMD_SKILLXY;
	cmd.x = x;
	cmd.y = y;
	cmd.lsu = skillUse;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdLocDisarm(BYTE x, BYTE y, BYTE oi, int8_t from)
{
	TCmdLocDisarm cmd;

	cmd.bCmd = CMD_DISARMXY;
	cmd.x = x;
	cmd.y = y;
	cmd.oi = oi;
	cmd.from = from;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdPlrSkill(int pnum, const CmdSkillUse skillUse)
{
	TCmdPlrSkill cmd;

	cmd.bCmd = CMD_SKILLPLR;
	cmd.psPnum = pnum;
	cmd.psu = skillUse;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonSkill(int mnum, const CmdSkillUse skillUse)
{
	TCmdMonSkill cmd;

	cmd.bCmd = CMD_SKILLMON;
	cmd.msMnum = static_cast<uint16_t>(mnum);
	cmd.msu = skillUse;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdMonstDamage(int mnum, int hitpoints)
{
	TCmdMonstDamage cmd;

	cmd.bCmd = CMD_MONSTDAMAGE;
	cmd.mdLevel = currLvl._dLevelIdx;
	cmd.mdMnum = static_cast<uint16_t>(mnum);
	cmd.mdHitpoints = hitpoints;

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
	cmd.mnMnum = static_cast<uint16_t>(mnum);
	cmd.mnSIdx = mon->_mMTidx;
	cmd.mnDir = mon->_mdir;
	cmd.mnMaxHp = mon->_mmaxhp;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdNewLvl(BYTE fom, BYTE bLevel)
{
	TCmdNewLvl cmd;

	cmd.bCmd = CMD_NEWLVL;
	cmd.bPlayers = gbActivePlayers; // TODO: could be done in On_NEWLVL 
	cmd.bFom = fom;
	cmd.bLevel = bLevel;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdCreateLvl(int32_t seed, BYTE lvl, BYTE type)
{
	TCmdCreateLvl cmd;

	cmd.bCmd = CMD_CREATELVL;
	cmd.clPlayers = gbActivePlayers; // TODO: could be done in On_CREATELVL 
	cmd.clSeed = seed;
	cmd.clLevel = lvl;
	cmd.clType = type;

	NetSendChunk((BYTE*)&cmd, sizeof(cmd));
}

void NetSendCmdString(TMsgString* cmd, unsigned int pmask)
{
	static_assert(sizeof(TMsgString) <= (sizeof(NormalMsgPkt) - sizeof(MsgPktHdr)), "String message does not fit in NormalMsgPkt.");
	cmd->bCmd = NMSG_STRING;
	multi_send_direct_msg(pmask, (BYTE*)cmd, cmd->bsLen + 3); // length of string + nul terminator + sizeof(cmd->bCmd) + sizeof(cmd->bsLen)
}

static bool check_update_plr(int pnum)
{
	bool result = pnum == mypnum;
	if (result && IsMultiGame) {
		pfile_update(true);
	}
	return result;
}

static unsigned On_SYNCDATA(const TCmd* pCmd, int pnum)
{
	const TSyncHeader* pHdr = (const TSyncHeader*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == pHdr->bLevel)
	//	sync_update(pnum, pHdr);
	delta_sync_monster(pHdr);

	return pHdr->wLen + sizeof(*pHdr);
}

static unsigned On_WALKXY(const TCmd* pCmd, int pnum)
{
	const TCmdLoc* cmd = (const TCmdLoc*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		net_assert(IN_ACTIVE_AREA(cmd->x, cmd->y));
		plr._pDestAction = ACTION_WALK;
		plr._pDestParam1 = cmd->x;
		plr._pDestParam2 = cmd->y;
	}

	return sizeof(*cmd);
}

static unsigned On_WALKDIR(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		if (cmd->bParam1 < NUM_DIRS) {
			plr._pDestAction = ACTION_WALKDIR;
			plr._pDestParam1 = cmd->bParam1;
		} else {
			plr._pDestAction = ACTION_NONE;
		}
	}

	return sizeof(*cmd);
}

static unsigned On_ADDSTR(const TCmd* pCmd, int pnum)
{
	IncreasePlrStr(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDMAG(const TCmd* pCmd, int pnum)
{
	IncreasePlrMag(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDDEX(const TCmd* pCmd, int pnum)
{
	IncreasePlrDex(pnum);

	return sizeof(*pCmd);
}

static unsigned On_ADDVIT(const TCmd* pCmd, int pnum)
{
	IncreasePlrVit(pnum);

	return sizeof(*pCmd);
}

static unsigned On_DECHP(const TCmd* pCmd, int pnum)
{
	DecreasePlrMaxHp(pnum);

	return sizeof(*pCmd);
}

static unsigned On_TURN(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;
	int dir;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		dir = cmd->bParam1;

		net_assert(dir < NUM_DIRS);
		plr._pDestAction = ACTION_TURN;
		plr._pDestParam1 = dir;
	}
	return sizeof(*cmd);
}

static bool CheckPlrSkillUse(int pnum, const CmdSkillUse& su)
{
	int ma;
	BYTE sn = su.skill, slvl;
	int8_t sf = su.from;
	bool sameLvl = currLvl._dLevelIdx == plr._pDunLevel;

	net_assert(sn != SPL_NULL && sn < NUM_SPELLS);

	if (plr._pmode != PM_DEATH && (spelldata[sn].sUseFlags & plr._pSkillFlags) == spelldata[sn].sUseFlags) {
		slvl = plr._pSkillLvl[sn];
		if (sf == SPLFROM_MANA) {
			if (slvl == 0)
				return false;
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
			uint64_t mask = SPELL_MASK(plrAbility) | (SPL_ABI_MASK & ~SPELL_MASK(SPL_WALK));
			net_assert(mask & SPELL_MASK(sn));
		} else {
			net_assert((BYTE)sf < NUM_INVELEM);
			if (!SyncUseItem(pnum, sf, sn))
				return false;
		}
		plr._pDestParam3 = sn;
		plr._pDestParam4 = slvl;
		return sameLvl;
	}
	return false;
}

static unsigned On_BLOCK(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;
	int dir;
	CmdSkillUse su;

	su.from = SPLFROM_ABILITY;
	su.skill = SPL_BLOCK;

	if (CheckPlrSkillUse(pnum, su)) {
		dir = cmd->bParam1;

		net_assert(dir < NUM_DIRS);
		plr._pDestAction = ACTION_BLOCK;
		plr._pDestParam1 = dir;
	}
	return sizeof(*cmd);
}

static unsigned On_GOTOGETITEM(const TCmd* pCmd, int pnum)
{
	const TCmdLocParam1* cmd = (const TCmdLocParam1*)pCmd;
	int ii;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		ii = cmd->wParam1;

		net_assert(ii < MAXITEMS);
		plr._pDestAction = ACTION_PICKUPITEM;
		plr._pDestParam1 = cmd->x;
		plr._pDestParam2 = cmd->y;
		plr._pDestParam4 = ii;
	}

	return sizeof(*cmd);
}

static unsigned On_GETITEM(const TCmd* pCmd, int pnum)
{
	const TCmdGItem* cmd = (const TCmdGItem*)pCmd;
	int ii;

	if (delta_get_item(cmd)) {
		if (plr._pHoldItem._itype != ITYPE_NONE) {
			PkItemStruct pkItem;
			PackPkItem(&pkItem, &plr._pHoldItem);
			delta_put_item(&pkItem, cmd->bLevel, cmd->x, cmd->y);
			// plr._pHoldItem._itype = ITYPE_NONE;
		}
		if (currLvl._dLevelIdx == cmd->bLevel) {
			ii = FindGetItem(&cmd->item);
			assert(ii != -1);
			InvGetItem(pnum, ii);
		} else {
			UnPackPkItem(&cmd->item);
			SyncAutoGetItem(pnum, MAXITEMS);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_AUTOGETITEM(const TCmd* pCmd, int pnum)
{
	const TCmdGItem* cmd = (const TCmdGItem*)pCmd;
	int ii;
	bool result;

	if (delta_get_item(cmd)) {
		if (currLvl._dLevelIdx == cmd->bLevel) {
			ii = FindGetItem(&cmd->item);
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
		quests[Q_FARMER]._qvar1 = QV_FARMER_BOMBUSED + pnum;
		quests[Q_FARMER]._qlog = TRUE;
		// open nest
		if (currLvl._dLevelIdx == DLV_TOWN) {
			sx = 70 + DBORDERX; sy = 52 + DBORDERY;
			dx = 71 + DBORDERX; dy = 53 + DBORDERY;
			PlaySfxLoc(LS_FLAMWAVE, sx, sy);
			for (i = sx; i <= dx; i++)
				for (j = sy; j <= dy; j++)
					AddMissile(i, j, -1, 0, 0, MIS_EXFBALL, MST_NA, 0, 0);
			// TODO: ResyncQuests?
			gbOpenWarps |= (1 << TWARP_NEST);
			OpenNest();
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
			PlaySfx(IS_SARC);
			// TODO: ResyncQuests?
			gbOpenWarps |= (1 << TWARP_CRYPT);
			OpenCrypt();
			InitTriggers();
		}
		return true;
	}
	return false;
}
#endif

static unsigned On_PUTITEM(const TCmd* pCmd, int pnum)
{
	const TCmdPItem* cmd = (const TCmdPItem*)pCmd;
	ItemStruct* pi;
	int x, y, pr;

	pi = &plr._pHoldItem;
	if (pi->_itype != ITYPE_NONE) {
		x = cmd->x;
		y = cmd->y;
#ifdef HELLFIRE
		if (cmd->bLevel != DLV_TOWN || !CheckTownTrigs(pnum, x, y, pi->_iIdx))
#endif
		{
			PkItemStruct pkItem;
			PackPkItem(&pkItem, pi);
			pr = delta_put_item(&pkItem, cmd->bLevel, x, y);
			if (pr < 0) {
				return sizeof(*cmd);
			}
			if (pr == 0 && currLvl._dLevelIdx == cmd->bLevel) {
				SyncPutItem(pnum, x, y, pi, true);
			}
		}
		pi->_itype = ITYPE_NONE;
		if (check_update_plr(pnum)) {
			// SetCursorPos(MousePos.x + (cursW >> 1), MousePos.y + (cursH >> 1));
			NewCursor(CURSOR_HAND);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_SPAWNITEM(const TCmd* pCmd, int pnum)
{
	const TCmdRPItem* cmd = (const TCmdRPItem*)pCmd;

	if (delta_put_item(&cmd->item, cmd->bLevel, cmd->x, cmd->y) == 0 && currLvl._dLevelIdx == cmd->bLevel) {
		UnPackPkItem(&cmd->item);
		SyncPutItem(-1, cmd->x, cmd->y, &items[MAXITEMS], cmd->bFlipFlag);
	}

	return sizeof(*cmd);
}

static unsigned On_SKILLXY(const TCmd* pCmd, int pnum)
{
	const TCmdLocSkill* cmd = (const TCmdLocSkill*)pCmd;
	int sn, x, y;

	if (CheckPlrSkillUse(pnum, cmd->lsu)) {
		sn = cmd->lsu.skill;
		x = cmd->x;
		y = cmd->y;

		net_assert(IN_ACTIVE_AREA(x, y));
		net_assert(/*sn != SPL_WALK &&*/ sn != SPL_BLOCK);

		plr._pDestAction = spelldata[sn].sType != STYPE_NONE ? ACTION_SPELL : ((spelldata[sn].sUseFlags & SFLAG_RANGED) ? ACTION_RATTACK : ACTION_ATTACK);
		plr._pDestParam1 = x;
		plr._pDestParam2 = y;
		// plr._pDestParam3 = sn;                  // spell/skill
		// plr._pDestParam4 = (BYTE)cmd->lsu.from; // spllvl (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_OPERATEITEM(const TCmd* pCmd, int pnum)
{
	const TCmdItemOp* cmd = (const TCmdItemOp*)pCmd;

	if (plr._pmode != PM_DEATH) {
		BYTE skill = cmd->iou.skill;
		int8_t from = cmd->iou.from;
		BYTE cii = cmd->ioIdx;

	// manipulate the item
	net_assert(skill < NUM_SPELLS && (spelldata[skill].sMissile == MIS_OPITEM || spelldata[skill].sMissile == MIS_REPAIR));
	net_assert((BYTE)from < NUM_INVELEM ||
		(from == SPLFROM_ABILITY && skill != SPL_OIL));
	net_assert(cii < NUM_INVELEM);

	if (skill == SPL_OIL)
		DoOil(pnum, from, cii);
	else
		DoAbility(pnum, from, cii);

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		// add cast effect
		plr._pDestAction = ACTION_SPELL;
		plr._pDestParam1 = plr._px;
		plr._pDestParam2 = plr._py;
		plr._pDestParam3 = skill;          // spell
		plr._pDestParam4 = 0;              // spllvl (should not matter)
	}
	}
	return sizeof(*cmd);
}

static unsigned On_OPOBJXY(const TCmd* pCmd, int pnum)
{
	const TCmdLocParam1* cmd = (const TCmdLocParam1*)pCmd;
	int oi, x, y;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		oi = cmd->wParam1;
		x = cmd->x;
		y = cmd->y;

		net_assert(oi < MAXOBJECTS);
		net_assert(IN_ACTIVE_AREA(x, y));
		net_assert(abs(dObject[x][y]) == oi + 1);

		plr._pDestAction = ACTION_OPERATE;
		plr._pDestParam1 = x;
		plr._pDestParam2 = y;
		plr._pDestParam3 = SPL_ATTACK; // spell
		plr._pDestParam4 = oi;         // fake spllvl
	}

	return sizeof(*cmd);
}

static unsigned On_DISARMXY(const TCmd* pCmd, int pnum)
{
	const TCmdLocDisarm* cmd = (const TCmdLocDisarm*)pCmd;
	int oi, x, y;
	CmdSkillUse su;

	su.from = cmd->from;
	su.skill = SPL_DISARM;

	if (CheckPlrSkillUse(pnum, su)) {
		oi = cmd->oi;
		x = cmd->x;
		y = cmd->y;

		net_assert(oi < MAXOBJECTS);
		net_assert(IN_ACTIVE_AREA(x, y));
		net_assert(abs(dObject[x][y]) == oi + 1);

		plr._pDestAction = ACTION_SPELL;
		plr._pDestParam1 = x;
		plr._pDestParam2 = y;
		// plr._pDestParam3 = SPL_DISARM; // spell
		plr._pDestParam4 = oi;         // fake spllvl
	}

	return sizeof(*cmd);
}

static unsigned On_SKILLMON(const TCmd* pCmd, int pnum)
{
	const TCmdMonSkill* cmd = (const TCmdMonSkill*)pCmd;
	int sn, mnum;

	if (CheckPlrSkillUse(pnum, cmd->msu)) {
		sn = cmd->msu.skill;
		mnum = cmd->msMnum;

		net_assert(mnum < MAXMONSTERS);
		net_assert(/*sn != SPL_WALK &&*/ sn != SPL_BLOCK);

		plr._pDestAction = spelldata[sn].sType != STYPE_NONE ? ACTION_SPELLMON : ((spelldata[sn].sUseFlags & SFLAG_RANGED) ? ACTION_RATTACKMON : ACTION_ATTACKMON);
		plr._pDestParam1 = mnum;                // target id
		// plr._pDestParam3 = sn;                  // attack spell/skill
		// plr._pDestParam4 = (BYTE)cmd->msu.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_SKILLPLR(const TCmd* pCmd, int pnum)
{
	const TCmdPlrSkill* cmd = (const TCmdPlrSkill*)pCmd;
	int sn, tnum;

	if (CheckPlrSkillUse(pnum, cmd->psu)) {
		sn = cmd->psu.skill;
		tnum = cmd->psPnum;

		net_assert(tnum < MAX_PLRS);
		net_assert(/*sn != SPL_WALK &&*/ sn != SPL_BLOCK);

		plr._pDestAction = spelldata[sn].sType != STYPE_NONE ? ACTION_SPELLPLR : ((spelldata[sn].sUseFlags & SFLAG_RANGED) ? ACTION_RATTACKPLR : ACTION_ATTACKPLR);
		plr._pDestParam1 = tnum;                // target id
		// plr._pDestParam3 = sn;                  // attack spell/skill
		// plr._pDestParam4 = (BYTE)cmd->psu.from; // attack skill-level (set in CheckPlrSkillUse)
	}

	return sizeof(*cmd);
}

static unsigned On_TALKMON(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;
	int mnum;

	if (currLvl._dLevelIdx == plr._pDunLevel) {
		mnum = cmd->wParam1;

		net_assert(mnum < MAXMONSTERS);
		plr._pDestAction = ACTION_TALK;
		plr._pDestParam1 = mnum;
	}

	return sizeof(*cmd);
}

static unsigned On_NEWLVL(const TCmd* pCmd, int pnum)
{
	const TCmdNewLvl* cmd = (const TCmdNewLvl*)pCmd;
	BYTE bLevel, bPlayers;
	
	bLevel = cmd->bLevel;
	bPlayers = cmd->bPlayers;

	net_assert(bLevel < NUM_LEVELS);
	net_assert(bPlayers > 0 && bPlayers <= MAX_PLRS);
	if (gsDeltaData.ddLevelPlrs[bLevel] == 0) {
		gsDeltaData.ddLevelPlrs[bLevel] = bPlayers;
	}

	StartNewLvl(pnum, cmd->bFom, bLevel);

	return sizeof(*cmd);
}

static unsigned On_CREATELVL(const TCmd* pCmd, int pnum)
{
	const TCmdCreateLvl* cmd = (const TCmdCreateLvl*)pCmd;
	BYTE bLevel, bPlayers;
	if (plr._pDunLevel == DLV_TOWN && !plr._pLvlChanging && plr._pmode != PM_DEATH) {
	for (bLevel = NUM_FIXLVLS; bLevel < NUM_LEVELS; bLevel++) {
		int i = 0;
		for ( ; i < MAX_PLRS; i++) {
			if (plx(i)._pActive && plx(i)._pDunLevel == bLevel)
				break;
			if (portals[i]._rlevel == bLevel)
				break;
		}
		if (i == MAX_PLRS) {
			break;
		}
	}
	bPlayers = cmd->clPlayers;
	net_assert(bLevel < NUM_LEVELS);
	net_assert(bPlayers != 0 && bPlayers < MAX_PLRS);
	net_assert(cmd->clType < NUM_DTYPES);
	// reset level delta (entities + automap)
	// - multi
	static_assert((int)DCMD_INVALID == 0, "On_CREATELVL initializes the items with zero, assuming the invalid command to be zero.");
	static_assert((int)DCMD_MON_INVALID == 0, "On_CREATELVL initializes the monsters with zero, assuming the invalid command to be zero.");
	static_assert((int)CMD_SYNCDATA == 0, "On_CREATELVL initializes the objects with zero, assuming none of the valid commands for an object to be zero.");
	memset(&gsDeltaData.ddLevel[bLevel], 0, sizeof(DDLevel));
	memset(&gsDeltaData.ddLocal[bLevel], 0, sizeof(LocalLevel));
	// - single
	guLvlVisited &= ~LEVEL_MASK(bLevel);
	// setup the new level
	glSeedTbl[bLevel] = cmd->clSeed;
	gsDeltaData.ddLevelPlrs[bLevel] = bPlayers;
	static_assert(MAXCHARLEVEL + HELL_LEVEL_BONUS < CF_LEVEL, "On_CREATELVL might initialize a level which is too high for item-drops.");
	gDynLevels[bLevel - NUM_FIXLVLS]._dnLevel = cmd->clLevel;
	gDynLevels[bLevel - NUM_FIXLVLS]._dnType = cmd->clType;

	StartNewLvl(pnum, DVL_DWM_DYNLVL, bLevel);

	}
	return sizeof(*cmd);
}

static unsigned On_USEPORTAL(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;
	BYTE idx = cmd->bParam1;

	net_assert(idx < MAX_PLRS);

	UseTownPortal(pnum, idx);

	return sizeof(*cmd);
}

static unsigned On_MONSTDEATH(const TCmd* pCmd, int pnum)
{
	const TCmdMonstKill* cmd = (const TCmdMonstKill*)pCmd;
	int i, lvl;
	unsigned totplrs, xp;
	BYTE whoHit, mask;

	if (pnum != mypnum && currLvl._dLevelIdx == cmd->mkParam1.bParam1)
		MonSyncKill(cmd->mkMnum, cmd->mkParam1.x, cmd->mkParam1.y, cmd->mkPnum);

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

static unsigned On_AWAKEGOLEM(const TCmd* pCmd, int pnum)
{
	const TCmdGolem* cmd = (const TCmdGolem*)pCmd;

	delta_awake_golem(cmd, pnum);

	return sizeof(*cmd);
}

static unsigned On_MONSTDAMAGE(const TCmd* pCmd, int pnum)
{
	const TCmdMonstDamage* cmd = (const TCmdMonstDamage*)pCmd;

	delta_monster_hp(cmd, pnum);

	return sizeof(*cmd);
}

static unsigned On_MONSTCORPSE(const TCmd* pCmd, int pnum)
{
	const TCmdBParam2* cmd = (const TCmdBParam2*)pCmd;

	delta_monster_corpse(cmd);

	return sizeof(*cmd);
}

static unsigned On_MONSTSUMMON(const TCmd* pCmd, int pnum)
{
	const TCmdMonstSummon* cmd = (const TCmdMonstSummon*)pCmd;

	delta_monster_summon(cmd);

	return sizeof(*cmd);
}

static bool PlrDeadItem(int pnum, ItemStruct* pi, int dir)
{
	int x, y, pr;

	if (pi->_itype == ITYPE_NONE)
		return true;

	PkItemStruct pkItem;

	PackPkItem(&pkItem, pi);
	x = plr._px + offset_x[dir];
	y = plr._py + offset_y[dir];
	pr = delta_put_item(&pkItem, plr._pDunLevel, x, y);
	if (pr < 0)
		return false;
	if (pr == 0 && currLvl._dLevelIdx == plr._pDunLevel) {
		SyncPutItem(pnum, x, y, pi, true);
	}
	pi->_itype = ITYPE_NONE;
	return true;
}

static unsigned On_PLRDEAD(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;
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
		copy_cstr(ear._iPlrName, plr._pName);
		const int earSets[NUM_CLASSES] = {
				ICURS_EAR_WARRIOR, ICURS_EAR_ROGUE, ICURS_EAR_SORCERER
#ifdef HELLFIRE
				, ICURS_EAR_SORCERER, ICURS_EAR_ROGUE, ICURS_EAR_WARRIOR
#endif
		};
		ear._iCurs = earSets[plr._pClass];
		//ear._iCreateInfo = SwapLE16(*(WORD *)&ear._iPlrName[0]);
		//ear._iSeed = SwapLE32(*(DWORD *)&ear._iPlrName[2]);
		ear._ivalue = plr._pLevel;

		PlrDeadItem(pnum, &ear, DIR_S);
	}

	SyncPlrKill(pnum);

	check_update_plr(pnum);

	return sizeof(*cmd);
}

static unsigned On_PLRRESURRECT(const TCmd* pCmd, int pnum)
{
	SyncPlrResurrect(pnum);

	check_update_plr(pnum);

	return sizeof(*pCmd);
}

static unsigned On_DOOROPEN(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncDoorOpen(cmd->wParam1);
	delta_sync_object(cmd->wParam1, CMD_DOOROPEN, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_DOORCLOSE(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncDoorClose(cmd->wParam1);
	delta_sync_object(cmd->wParam1, CMD_DOORCLOSE, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPDISABLE(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapDisable(cmd->wParam1);
	delta_sync_object(cmd->wParam1, CMD_TRAPDISABLE, plr._pDunLevel);

	return sizeof(*cmd);
}

/*static unsigned On_TRAPOPEN(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapOpen(cmd->wParam1);
	delta_sync_object(cmd->wParam1, CMD_TRAPOPEN, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_TRAPCLOSE(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncTrapClose(cmd->wParam1);
	delta_sync_object(cmd->wParam1, CMD_TRAPCLOSE, plr._pDunLevel);

	return sizeof(*cmd);
}*/

static unsigned On_OPERATEOBJ(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	//if (pnum != mypnum && currLvl._dLevelIdx == plr._pDunLevel)
	//	SyncOpObject(pnum, cmd->wParam1);
	delta_sync_object(cmd->wParam1, CMD_OPERATEOBJ, plr._pDunLevel);

	return sizeof(*cmd);
}

/**
 * Sync Shrine effect with every player
 */
static unsigned On_SHRINE(const TCmd* pCmd, int pnum)
{
	const TCmdShrine* cmd = (const TCmdShrine*)pCmd;

	if (plr._pmode != PM_DEATH) {
		const BYTE type = cmd->shType;

		net_assert(type < NUM_SHRINETYPE);

		SyncShrineCmd(pnum, type, cmd->shSeed);
	}

	return sizeof(*cmd);
}

static unsigned On_SPLITPLRGOLD(const TCmd* pCmd, int pnum)
{
	const TCmdParamBW* cmd = (const TCmdParamBW*)pCmd;
	const BYTE r = cmd->byteParam;

	net_assert(r < NUM_INV_GRID_ELEM);

	// if (plr._pmode != PM_DEATH)
		SyncSplitGold(pnum, r, cmd->wordParam);

	return sizeof(*cmd);
}

static unsigned On_PASTEPLRITEM(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	if (plr._pmode != PM_DEATH) {
		const BYTE r = cmd->bParam1;

		net_assert(r < SLOTXY_BELT_FIRST);

		InvPasteItem(pnum, r);
	}

	return sizeof(*cmd);
}

static unsigned On_PASTEPLRBELTITEM(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	if (plr._pmode != PM_DEATH) {
		const BYTE r = cmd->bParam1;

		net_assert(r < MAXBELTITEMS);

		InvPasteBeltItem(pnum, r);
	}

	return sizeof(*cmd);
}

static unsigned On_CUTPLRITEM(const TCmd* pCmd, int pnum)
{
	const TCmdBParam2* cmd = (const TCmdBParam2*)pCmd;

	if (plr._pmode != PM_DEATH) {
		const BYTE cii = cmd->bParam1;

		net_assert(cii < NUM_INVELEM);

		InvCutItem(pnum, cii, cmd->bParam2);
	}

	return sizeof(*cmd);
}

static unsigned On_DELPLRITEM(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;
	const BYTE cii = cmd->bParam1;

	net_assert(cii < NUM_INVELEM);

	SyncPlrItemRemove(pnum, cii);

	return sizeof(*cmd);
}

static unsigned On_USEPLRITEM(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	if (plr._pmode != PM_DEATH) {
		const BYTE cii = cmd->bParam1;

		net_assert(cii < NUM_INVELEM);

		SyncUseItem(pnum, cii, SPL_INVALID);
	}

	return sizeof(*cmd);
}

static unsigned On_USEPLRMAP(const TCmd* pCmd, int pnum)
{
	const TCmdBParam2* cmd = (const TCmdBParam2*)pCmd;

	if (plr._pmode != PM_DEATH) {
		const BYTE cii = cmd->bParam1;
		const BYTE mIdx = cmd->bParam2;

		net_assert(cii < NUM_INVELEM);
		net_assert(mIdx < MAXCAMPAIGNSIZE);

		SyncUseMapItem(pnum, cii, mIdx);
	}

	return sizeof(*cmd);
}

static unsigned On_PLRINFO(const TCmd* pCmd, int pnum)
{
	const TMsgLarge* cmd = (const TMsgLarge*)pCmd;

	net_assert((unsigned)pnum < MAX_PLRS);

	if (geBufferMsgs == MSG_GAME_DELTA_LOAD || geBufferMsgs == MSG_GAME_DELTA_WAIT)
		DeltaQueuePacket(pnum, cmd, cmd->tpHdr.wBytes + sizeof(cmd->tpHdr));
	else if (pnum != mypnum)
		multi_recv_plrinfo_msg(pnum, cmd);

	return cmd->tpHdr.wBytes + sizeof(cmd->tpHdr);
}

static unsigned On_JOINLEVEL(const TCmd* pCmd, int pnum)
{
	const TCmdJoinLevel* cmd = (const TCmdJoinLevel*)pCmd;

	// reqister request only if not processing level-delta
	//if (geBufferMsgs != MSG_LVL_DELTA_PROC) { -- does not cover all cases...
		guSendLevelData |= (1 << pnum);
		guRequestLevelData[pnum] = gdwLastGameTurn;
	//}
	if (geBufferMsgs == MSG_LVL_DELTA_WAIT) {
		return sizeof(*cmd);
	}
	// should not be the case if priority is respected
	net_assert(geBufferMsgs != MSG_LVL_DELTA_SKIP_JOIN || currLvl._dLevelIdx != cmd->lLevel);
	// if (geBufferMsgs != MSG_LVL_DELTA_WAIT &&
	//// (geBufferMsgs != MSG_LVL_DELTA_SKIP_JOIN || currLvl._dLevelIdx != cmd->lLevel)) {
		plr._pLvlChanging = FALSE;
		//if (plr._pmode != PM_DEATH)
			plr._pInvincible = 40;
		net_assert(cmd->lLevel < NUM_LEVELS);
		net_assert(IN_ACTIVE_AREA(cmd->px, cmd->py));
		plr._pDunLevel = cmd->lLevel;
		plr._px = cmd->px;
		plr._py = cmd->py;
	// }

	if (pnum != mypnum) {
		if (!plr._pActive) {
			if (geBufferMsgs == MSG_LVL_DELTA_PROC) {
				// joined and left while waiting for level-delta
				return sizeof(*cmd);
			}
			if (!multi_plrinfo_received(pnum)) {
				// plrinfo_msg did not arrive -> drop the player
				SNetDropPlayer(pnum);
				return sizeof(*cmd);
			}
			// TODO: validate data from internet
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
			plr._pHPBase = cmd->php;
			plr._pManaBase = cmd->pmp;
			plr._pTimer[PLTR_INFRAVISION] = cmd->lTimer1;
			plr._pTimer[PLTR_RAGE] = cmd->lTimer2;
			plr._pManaShield = cmd->pManaShield;

			ImportItemDurabilities(pnum, cmd->itemsDur);

			InitLvlPlayer(pnum, true);
			ProcessVisionList();
		} else {
			delta_reserve_items(cmd);
		}
	}

	return sizeof(*cmd);
}

static unsigned On_DISCONNECT(const TCmd* pCmd, int pnum)
{
	if (geBufferMsgs == MSG_LVL_DELTA_WAIT) {
		guOweLevelDelta &= ~(1 << pnum);
		return sizeof(*pCmd);
	}
	multi_deactivate_player(pnum);

	return sizeof(*pCmd);
}

static unsigned On_REQDELTA(const TCmd* pCmd, int pnum)
{
	if (pnum != mypnum) {
		guSendGameDelta |= 1 << pnum;
	}

	return sizeof(*pCmd);
}

static void DoTelekinesis(int pnum, int x, int y, int8_t from, int id)
{
	CmdSkillUse su;

	su.from = from;
	su.skill = SPL_TELEKINESIS;

	if (CheckPlrSkillUse(pnum, su)) {
		plr._pDestAction = ACTION_SPELL;
		plr._pDestParam1 = x;
		plr._pDestParam2 = y;
		// plr._pDestParam3 = SPL_TELEKINESIS; // spell
		plr._pDestParam4 = id;              // fake spllvl
	}
}

static unsigned On_TELEKINITM(const TCmd* pCmd, int pnum)
{
	const TCmdLocBParam2* cmd = (const TCmdLocBParam2*)pCmd;

	DoTelekinesis(pnum, cmd->x, cmd->y, cmd->bParam1, (MTT_ITEM << 16) | cmd->bParam2);

	return sizeof(*cmd);
}

static unsigned On_TELEKINMON(const TCmd* pCmd, int pnum)
{
	const TCmdParamBW* cmd = (const TCmdParamBW*)pCmd;
	int mnum = cmd->wordParam;

	net_assert(mnum < MAXMONSTERS);

	DoTelekinesis(pnum, monsters[mnum]._mx, monsters[mnum]._my, cmd->byteParam, (MTT_MONSTER << 16) | mnum);

	return sizeof(*cmd);
}

static unsigned On_TELEKINPLR(const TCmd* pCmd, int pnum)
{
	const TCmdBParam2* cmd = (const TCmdBParam2*)pCmd;
	int tnum = cmd->bParam2;

	net_assert(tnum < MAX_PLRS);

	DoTelekinesis(pnum, players[tnum]._px, players[tnum]._py, cmd->bParam1, (MTT_PLAYER << 16) | tnum);

	return sizeof(*cmd);
}

static unsigned On_TELEKINOBJ(const TCmd* pCmd, int pnum)
{
	const TCmdParamBW* cmd = (const TCmdParamBW*)pCmd;
	int oi = cmd->wordParam;

	net_assert(oi < MAXOBJECTS);

	DoTelekinesis(pnum, objects[oi]._ox, objects[oi]._oy, cmd->byteParam, (MTT_OBJECT << 16) | oi);

	return sizeof(*cmd);
}

static unsigned On_ACTIVATEPORTAL(const TCmd* pCmd, int pnum)
{
	const TCmdLocBParam1* cmd = (const TCmdLocBParam1*)pCmd;
	BYTE bLevel = cmd->bParam1;

	net_assert(bLevel != DLV_TOWN);
	// net_assert(bLevel < NUM_LEVELS);
	net_assert(IN_ACTIVE_AREA(cmd->x, cmd->y));

	static_assert(MAXPORTAL == MAX_PLRS, "On_ACTIVATEPORTAL uses pnum as portal-id.");
	if (currLvl._dLevelIdx == DLV_TOWN)
		AddInTownPortal(pnum);
	else if (currLvl._dLevelIdx != bLevel)
		RemovePortalMissile(pnum);

	ActivatePortal(pnum, cmd->x, cmd->y, bLevel);

	return sizeof(*cmd);
}

static unsigned On_RETOWN(const TCmd* pCmd, int pnum)
{
	RestartTownLvl(pnum);

	return sizeof(*pCmd);
}

static unsigned On_STRING(const TCmd* pCmd, int pnum)
{
	const TMsgString* cmd = (const TMsgString*)pCmd;

	//if (geBufferMsgs != MSG_GAME_DELTA_LOAD && geBufferMsgs != MSG_GAME_DELTA_WAIT) {
		net_assert(cmd->bsLen < MAX_SEND_STR_LEN && cmd->str[cmd->bsLen] == '\0');
		if (pnum < MAX_PLRS) {
			if (!(guTeamMute & (1 << pnum))) {
				ReceivePlrMsg(pnum, cmd->str);
			}
		} else {
			EventPlrMsg(cmd->str);
		}
	//}

	return cmd->bsLen + 3; // length of string + nul terminator + sizeof(cmd->bCmd) + sizeof(cmd->bsLen)
}

static unsigned On_INVITE(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	// TODO: check (cmd->bParam1 == mypnum) should not be necessary in a server/client solution
	if (cmd->bParam1 == mypnum && plr._pTeam == pnum) {
		guTeamInviteRec |= (1 << pnum);
		EventPlrMsg("%s invited to their team.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_ACK_INVITE(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

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

static unsigned On_DEC_INVITE(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteSent &= ~(1 << pnum);

		EventPlrMsg("%s rejected your invitation.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_REV_INVITE(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	if (cmd->bParam1 == mypnum) { // TODO: check should not be necessary in a server/client solution
		guTeamInviteRec &= ~(1 << pnum);

		EventPlrMsg("%s revoked the invitation.", plr._pName);
	}

	return sizeof(*cmd);
}

static unsigned On_KICK_PLR(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;
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
static unsigned On_STORE_1(const TCmd* pCmd, int pnum)
{
	const TCmdStore1* cmd = (const TCmdStore1*)pCmd;
	BYTE c = cmd->stCmd, cii = cmd->stLoc;

	net_assert(plr._pmode != PM_DEATH);
	net_assert(plr._pDunLevel == DLV_TOWN);
	net_assert(c == STORE_SSELL || c == STORE_SIDENTIFY || c == STORE_SREPAIR || c == STORE_WRECHARGE || c == STORE_PEGBOY);
	net_assert(cii < NUM_INVELEM);

	SyncStoreCmd(pnum, c, cii, cmd->stValue);

	return sizeof(*cmd);
}

/**
 * Execute store transactions II.
 */
static unsigned On_STORE_2(const TCmd* pCmd, int pnum)
{
	const TCmdStore2* cmd = (const TCmdStore2*)pCmd;
	BYTE c = cmd->stCmd;

	net_assert(plr._pmode != PM_DEATH);
	net_assert(plr._pDunLevel == DLV_TOWN);
	net_assert(c == STORE_HBUY || c == STORE_SBUY || c == STORE_SPBUY || c == STORE_WBUY || c == STORE_PBUY);

	UnPackPkItem(&cmd->item);
	SyncStoreCmd(pnum, c, MAXITEMS, cmd->stValue);

	return sizeof(*cmd);
}

/* Sync item rewards in town. */
static unsigned On_QTOWNER(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	net_assert(plr._pmode != PM_DEATH);
	net_assert(plr._pDunLevel == DLV_TOWN);

	SyncTownerQ(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

/* Sync item rewards in dungeon. */
static unsigned On_QMONSTER(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	if (plr._pmode != PM_DEATH)
		SyncMonsterQ(pnum, cmd->wParam1);

	return sizeof(*cmd);
}

/**
 * Sync Quest with every player.
 */
static unsigned On_SYNCQUEST(const TCmd* pCmd, int pnum)
{
	const TCmdQuest* cmd = (const TCmdQuest*)pCmd;

	if (pnum != mypnum)
		SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);

	return sizeof(*cmd);
}

/**
 * Same as On_SYNCQUEST, but sends the message only to players on other levels.
 */
static unsigned On_SYNCQUESTEXT(const TCmd* pCmd, int pnum)
{
	const TCmdQuest* cmd = (const TCmdQuest*)pCmd;

	if (currLvl._dLevelIdx != plr._pDunLevel)
		SetMultiQuest(cmd->q, cmd->qstate, cmd->qlog, cmd->qvar1);

	return sizeof(*cmd);
}

#if DEV_MODE
#define msg_errorf(fmt, ...) EventPlrMsg(fmt, __VA_ARGS__);
static unsigned On_DUMP_MONSTERS(const TCmd* pCmd, int pnum)
{
	int mnum;
	MonsterStruct* mon;

	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		mon = &monsters[mnum];
		// clang-format off
		LogErrorF("mnum:%d "
	"mo:%d "
	"sq:%d "
	"idx:%d "
	//"pc:%d "
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
	//"df:%d "
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
	"nc:%d "
	"lid:%d "
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
	//mon->_mpathcount,
	//mon->_mAlign_1,
	mon->_mgoal,
	mon->_mgoalvar1,
	mon->_mgoalvar2,
	mon->_mgoalvar3,
	mon->_mx,
	mon->_my,
	mon->_mfutx,
	mon->_mfuty,
	mon->_moldx,
	mon->_moldy,
	mon->_mxoff,
	mon->_myoff,
	mon->_mdir,
	mon->_menemy,
	mon->_menemyx,
	mon->_menemyy,
	mon->_mListener,
	mon->_mAnimFrameLen,
	mon->_mAnimCnt,
	mon->_mAnimLen,
	mon->_mAnimFrame,
	//mon->_mDelFlag,
	mon->_mVar1,
	mon->_mVar2,
	mon->_mVar3,
	mon->_mVar4,
	mon->_mVar5,
	mon->_mVar6,
	mon->_mVar7,
	mon->_mVar8,
	mon->_mmaxhp,
	mon->_mhitpoints,
	mon->_mlastx,
	mon->_mlasty,
	mon->_mRndSeed,
	mon->_mAISeed,
	mon->_muniqtype,
	mon->_muniqtrans,
	mon->_mNameColor,
	mon->_mlid,
	mon->_mleader,
	mon->_mleaderflag,
	mon->_mpacksize,
	mon->_mvid,
	mon->_mLevel,
	mon->_mSelFlag,
	mon->_mAI.aiType,
	mon->_mAI.aiInt,
	mon->_mFlags,
	mon->_mHit,
	mon->_mMinDamage,
	mon->_mMaxDamage,
	mon->_mHit2,
	mon->_mMinDamage2,
	mon->_mMaxDamage2,
	mon->_mMagic,
	mon->_mArmorClass,
	mon->_mEvasion,
	mon->_mAFNum,
	mon->_mAFNum2,
	mon->_mMagicRes,
	mon->_mExp,
	mon->_mType,
	mon->_mAnimWidth,
	mon->_mAnimXOffset);
		// clang-format on
		DDMonster* mstr = &gsDeltaData.ddLevel[myplr._pDunLevel].monster[mnum];
		if (mstr->dmCmd != DCMD_MON_INVALID) {
			// clang-format off
			LogErrorF("delta ",
		"dmCmd:%d "
		"dmx:%d "
		"dmy:%d "
		"dmdir:%d "
		"dmactive:%d "
		"dmhitpoints:%d "
		"dmWhoHit:%d ",

		mstr->dmCmd,
		mstr->dmx,
		mstr->dmy,
		mstr->dmdir,
		mstr->dmactive,
		(int)mstr->dmhitpoints,
		mstr->dmWhoHit);
			// clang-format on
		} else {
			LogErrorF("delta dmCmd:0");
		}
	}

	return sizeof(*pCmd);
}

static unsigned On_REQUEST_PLRCHECK(const TCmd* pCmd, int pnum)
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

		*buf = plx(i)._pDestAction;
		buf++;

		*(INT*)buf = plx(i)._pDestParam1;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pDestParam2;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pDestParam3;
		buf += sizeof(INT);
		*(INT*)buf = plx(i)._pDestParam4;
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
		//plr._pxoff = tplr->spxoff;
		//plr._pyoff = tplr->spyoff;
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

		// LogErrorF("Player base-data %d", (size_t)buf - (size_t)plrdata);

		assert((size_t)buf - (size_t)plrdata == 114);
		NetSendChunk(plrdata, 114); // (size_t)buf - (size_t)plrdata);

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
		*(uint64_t*)buf = plx(i)._pInvSkills;
		buf += sizeof(uint64_t);

		//LogErrorF("Player skill-data I. %d", (size_t)buf - (size_t)plrdata);
		assert((size_t)buf - (size_t)plrdata == 211);
		NetSendChunk(plrdata, 211); // (size_t)buf - (size_t)plrdata);

		// skill attributes II.
		buf = &plrdata[1];
		*buf = i;
		buf++;

		*buf = 2; // chunk-index
		buf++;

		memcpy(buf, plx(i)._pSkillExp, sizeof(plx(i)._pSkillExp) / 2);
		buf += sizeof(plx(i)._pSkillExp) / 2;

		// LogErrorF("Player skill-data II. %d", (size_t)buf - (size_t)plrdata);
		assert((size_t)buf - (size_t)plrdata == 131);
		NetSendChunk(plrdata, 131); // (size_t)buf - (size_t)plrdata);

		// skill attributes III.
		buf = &plrdata[1];
		*buf = i;
		buf++;

		*buf = 3; // chunk-index
		buf++;

		memcpy(buf, &plx(i)._pSkillExp[32], sizeof(plx(i)._pSkillExp) / 2);
		buf += sizeof(plx(i)._pSkillExp) / 2;

		//LogErrorF("Player skill-data III. %d", (size_t)buf - (size_t)plrdata);
		assert((size_t)buf - (size_t)plrdata == 131);
		NetSendChunk(plrdata, 131); // (size_t)buf - (size_t)plrdata);
		/*PlrAnimStruct _pAnims[NUM_PGXS];*/
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
		int8_t _pMagResist;
		int8_t _pFireResist;
		int8_t _pLghtResist;
		int8_t _pAcidResist;
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
		int _pIAbsAnyHit; // absorbed hit damage
		int _pIAbsPhyHit; // absorbed physical hit damage
		int8_t _pIBaseAttackSpeed;
		int8_t _pIArrowVelBonus; // _pISplCost in vanilla code
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

static void PrintPlrMismatch64(const char* field, uint64_t myval, uint64_t extval, int sp, int pnum)
{
	msg_errorf("%d received %s (%ull vs. %ull) from %d for plr%d", mypnum, field, myval, extval, sp, pnum);
}

static void CmpPlrArray(const char* field, const void* src, const void* data, int size, int len, int ip, int pnum)
{
	const BYTE* sbuf = (const BYTE*)src;
	const BYTE* dbuf = (const BYTE*)data;
	for (int i = 0; i < len; i++) {

		if (size == 1) {
			if (*sbuf != *dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch(tmp, *dbuf, *sbuf, pnum, ip);
			}
		} else if (size == 2) {
			if (*(const uint16_t*)sbuf != *(const uint16_t*)dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch(tmp, *(const uint16_t*)dbuf, *(const uint16_t*)sbuf, pnum, ip);
			}
		} else if (size == 4) {
			if (*(const uint32_t*)sbuf != *(const uint32_t*)dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch(tmp, *(const uint32_t*)dbuf, *(const uint32_t*)sbuf, pnum, ip);
			}
		} else if (size == 8) {
			if (*(const uint64_t*)sbuf != *(const uint64_t*)dbuf) {
				char tmp[256];
				snprintf(tmp, 256, "%s[%d]", field, i);
				PrintPlrMismatch64(tmp, *(const uint64_t*)dbuf, *(const uint64_t*)sbuf, pnum, ip);
			}
		}
		sbuf += size;
		dbuf += size;
	}
}

static unsigned On_DO_PLRCHECK(const TCmd* pCmd, int pnum)
{
	const BYTE* src = (const BYTE*)pCmd;
	src++;
	int i = *src, k;
	src++;
	k = *src;
	src++;

	// LogErrorF("ItemCheck %d. for %d running data from %d.", k, i, pnum);
	if (!plx(i)._pActive)
		msg_errorf("%d received inactive plr%d from %d", mypnum, i, pnum);

	switch (k) {
	case 0: // base params

		if (plx(i)._pmode != *src)
			PrintPlrMismatch("mode", plx(i)._pmode, *src, pnum, i);
		src++;

		// walkpath

		if (plx(i)._pDestAction != *src)
			PrintPlrMismatch("destaction", plx(i)._pDestAction, *src, pnum, i);
		src++;

		if (plx(i)._pDestAction != ACTION_NONE && plx(i)._pDestParam1 != *(const INT*)src)
			PrintPlrMismatch("destparam1", plx(i)._pDestParam1, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pDestAction != ACTION_NONE && plx(i)._pDestParam2 != *(const INT*)src)
			PrintPlrMismatch("destparam2", plx(i)._pDestParam2, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pDestAction != ACTION_NONE && plx(i)._pDestParam3 != *(const INT*)src)
			PrintPlrMismatch("destparam3", plx(i)._pDestParam3, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pDestAction != ACTION_NONE && plx(i)._pDestParam4 != *(const INT*)src)
			PrintPlrMismatch("destparam4", plx(i)._pDestParam4, *(const INT*)src, pnum, i);
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

		if (plx(i)._pStatPts != *(const WORD*)src)
			PrintPlrMismatch("statpts", plx(i)._pStatPts, *(const WORD*)src, pnum, i);
		src += sizeof(WORD);

		//BYTE _pLightRad;
		if (plx(i)._pManaShield != *src)
			PrintPlrMismatch("manashield", plx(i)._pManaShield, *src, pnum, i);
		src++;

		//memcpy(buf, plx(i)._pTimer, sizeof(plx(i)._pTimer));
		//buf += sizeof(plx(i)._pTimer);

		if (plx(i)._pExperience != *(const UINT*)src)
			PrintPlrMismatch("expr", plx(i)._pExperience, *(const UINT*)src, pnum, i);
		src += sizeof(UINT);
		if (plx(i)._pNextExper != *(const UINT*)src)
			PrintPlrMismatch("nexpr", plx(i)._pNextExper, *(const UINT*)src, pnum, i);
		src += sizeof(UINT);

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
		if (plx(i)._pBaseStr != *(const WORD*)src)
			PrintPlrMismatch("basestr", plx(i)._pBaseStr, *(const WORD*)src, pnum, i);
		src += sizeof(WORD);
		if (plx(i)._pBaseMag != *(const WORD*)src)
			PrintPlrMismatch("basemag", plx(i)._pBaseMag, *(const WORD*)src, pnum, i);
		src += sizeof(WORD);
		if (plx(i)._pBaseDex != *(const WORD*)src)
			PrintPlrMismatch("basedex", plx(i)._pBaseDex, *(const WORD*)src, pnum, i);
		src += sizeof(WORD);
		if (plx(i)._pBaseVit != *(const WORD*)src)
			PrintPlrMismatch("basevit", plx(i)._pBaseVit, *(const WORD*)src, pnum, i);
		src += sizeof(WORD);

		if (plx(i)._pHPBase != *(const INT*)src)
			PrintPlrMismatch("HPBase", plx(i)._pHPBase, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pMaxHPBase != *(const INT*)src)
			PrintPlrMismatch("MaxHPBase", plx(i)._pMaxHPBase, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pManaBase != *(const INT*)src)
			PrintPlrMismatch("ManaBase", plx(i)._pManaBase, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pMaxManaBase != *(const INT*)src)
			PrintPlrMismatch("MaxManaBase", plx(i)._pMaxManaBase, *(const INT*)src, pnum, i);
		src += sizeof(INT);

		if (plx(i)._pVar1 != *(const INT*)src)
			PrintPlrMismatch("Var1", plx(i)._pVar1, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar2 != *(const INT*)src)
			PrintPlrMismatch("Var2", plx(i)._pVar2, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar3 != *(const INT*)src)
			PrintPlrMismatch("Var3", plx(i)._pVar3, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar4 != *(const INT*)src)
			PrintPlrMismatch("Var4", plx(i)._pVar4, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar5 != *(const INT*)src)
			PrintPlrMismatch("Var5", plx(i)._pVar5, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar6 != *(const INT*)src)
			PrintPlrMismatch("Var6", plx(i)._pVar6, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar7 != *(const INT*)src)
			PrintPlrMismatch("Var7", plx(i)._pVar7, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		if (plx(i)._pVar8 != *(const INT*)src)
			PrintPlrMismatch("Var8", plx(i)._pVar8, *(const INT*)src, pnum, i);
		src += sizeof(INT);

		if (plx(i)._pGold != *(const INT*)src)
			PrintPlrMismatch("Gold", plx(i)._pGold, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		//int _pStrength;
		//int _pMagic;
		//int _pDexterity;
		//int _pVitality;
		if (plx(i)._pHitPoints != *(const INT*)src)
			PrintPlrMismatch("HitPoints", plx(i)._pHitPoints, *(const INT*)src, pnum, i);
		src += sizeof(INT);
		//int _pMaxHP;     // the maximum hp of the player
		if (plx(i)._pMana != *(const INT*)src)
			PrintPlrMismatch("Mana", plx(i)._pMana, *(const INT*)src, pnum, i);
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

		if (plx(i)._pMemSkills != *(const uint64_t*)src)
			PrintPlrMismatch64("MemSkills", plx(i)._pMemSkills, *(const uint64_t*)src, pnum, i);
		src += sizeof(uint64_t);
		if (plx(i)._pInvSkills != *(const uint64_t*)src)
			PrintPlrMismatch64("InvSkills", plx(i)._pInvSkills, *(const uint64_t*)src, pnum, i);
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

	return (unsigned)((size_t)src - (size_t)pCmd);
}

static const int ITEMCHECK_LEN = 4 + 2 + 2 + 1 + 1 + 1 + 1 + 4 + 4 + 4;
static BYTE* SendItem(const ItemStruct* is, BYTE* dst)
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

static unsigned On_REQUEST_ITEMCHECK(const TCmd* pCmd, int pnum)
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

static void PrintItemMismatch(const ItemStruct* is, const char* field, int myval, int extval, int sp, int pnum, int locId, int subloc)
{
	const char* loc = locId == 0 ? (subloc < 0 ? "hand" : "body") : (locId == 1 ? "belt" : "inv");
	int row = locId >= 2 ? locId - 2 : 0;
	if (plx(pnum)._pActive)
		msg_errorf("%d received %s (%d vs. %d) from %d for plr%d %s item at %d:%d", mypnum, field, myval, extval, sp, pnum, loc, row, subloc);
}

static const BYTE* CheckItem(const ItemStruct* is, const BYTE* src, int pnum, int loc, int subloc, int sp)
{
	const BYTE* srcStart = src;
	bool placeholder, none;

	if (is->_itype != *(const BYTE*)src) {
		PrintItemMismatch(is, "type", is->_itype, *(const BYTE*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(BYTE);

	placeholder = is->_itype == ITYPE_PLACEHOLDER;
	none = is->_itype == ITYPE_NONE;

	if (!none && !placeholder && is->_iSeed != *(const INT*)src) {
		PrintItemMismatch(is, "seed", is->_iSeed, *(const INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);

	if (!none && !placeholder && is->_iIdx != *(const WORD*)src) {
		PrintItemMismatch(is, "idx", is->_iIdx, *(const WORD*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(WORD);
	if (!none && !placeholder && is->_iCreateInfo != *(const WORD*)src) {
		PrintItemMismatch(is, "ci", is->_iCreateInfo, *(const WORD*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(WORD);

	if (placeholder && is->_ix != *(const BYTE*)src) {
		PrintItemMismatch(is, "x", is->_ix, *(const BYTE*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(BYTE);
	//if (!none && !placeholder && is->_iy != *(const BYTE*)src) {
	//	PrintItemMismatch(is, "x", is->_iy, *(const BYTE*)src, sp, pnum, loc, subloc);
	//}
	src += sizeof(BYTE);
	if (!none && !placeholder && is->_iIdentified != *(const BOOLEAN*)src) {
		PrintItemMismatch(is, "iden", is->_iIdentified, *(const BOOLEAN*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(BOOLEAN);

	if (!none && !placeholder && is->_iCharges != *(const INT*)src) {
		PrintItemMismatch(is, "chg", is->_iCharges, *(const INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);
	if (!none && !placeholder && is->_iDurability != *(const INT*)src) {
		PrintItemMismatch(is, "dur", is->_iDurability, *(const INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);
	if (!none && !placeholder && is->_ivalue != *(const INT*)src) {
		PrintItemMismatch(is, "value", is->_ivalue, *(const INT*)src, sp, pnum, loc, subloc);
	}
	src += sizeof(INT);

	assert((size_t)src - (size_t)srcStart == ITEMCHECK_LEN);
	return src;
}

static unsigned On_DO_ITEMCHECK(const TCmd* pCmd, int pnum)
{
	const BYTE* src = (const BYTE*)pCmd;
	src++;
	int i = *src, k;
	src++;
	k = *src;
	src++;

	// LogErrorF("ItemCheck %d. for %d running data from %d.", k, i, pnum);

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

	//	LogErrorF("ItemCheck done. %d", (size_t)src - (size_t)pCmd);
	return (unsigned)((size_t)src - (size_t)pCmd);
}

#endif

static unsigned On_SETSHIELD(const TCmd* pCmd, int pnum)
{
	const TCmdBParam1* cmd = (const TCmdBParam1*)pCmd;

	plr._pManaShield = cmd->bParam1;

	return sizeof(*cmd);
}

static unsigned On_REMSHIELD(const TCmd* pCmd, int pnum)
{
	plr._pManaShield = 0;

	return sizeof(*pCmd);
}

static unsigned On_BLOODPASS(const TCmd* pCmd, int pnum)
{
	const TCmdParam1* cmd = (const TCmdParam1*)pCmd;

	if (SyncBloodPass(pnum, cmd->wParam1))
		delta_sync_object(cmd->wParam1, CMD_OPERATEOBJ, plr._pDunLevel);

	return sizeof(*cmd);
}

static unsigned On_OPENSPIL(const TCmd* pCmd, int pnum)
{
	net_assert(quests[Q_BANNER]._qactive != QUEST_NOTAVAIL);

	quests[Q_BANNER]._qactive = QUEST_DONE;
	quests[Q_BANNER]._qvar1 = QV_BANNER_ATTACK;

	//if (QuestStatus(Q_BANNER))
	if (currLvl._dLevelIdx == questlist[Q_BANNER]._qdlvl) {
		// TODO: ResyncQuests?
		ResyncBanner();
		//RedoLightAndVision();
	}
	return sizeof(*pCmd);
}
#ifdef HELLFIRE
static unsigned On_OPENNAKRUL(const TCmd* pCmd, int pnum)
{
	// net_assert(quests[Q_NAKRUL]._qactive != QUEST_NOTAVAIL);
	net_assert(plr._pDunLevel == questlist[Q_NAKRUL]._qdlvl);

	quests[Q_NAKRUL]._qactive = QUEST_DONE;
	quests[Q_NAKRUL]._qvar1 = QV_NAKRUL_BOOKOPEN;

	//if (QuestStatus(Q_NAKRUL))
	if (currLvl._dLevelIdx == questlist[Q_NAKRUL]._qdlvl) {
		OpenNakrulRoom();
		WakeNakrul();
		//RedoLightAndVision();
	}
	return sizeof(*pCmd);
}
#endif
unsigned ParseMsg(int pnum, const TCmd* pCmd)
{
//#ifndef NOHOSTING
	if ((unsigned)pnum >= MAX_PLRS && pnum != SNPLAYER_MASTER) {
//#else
//	if ((unsigned)pnum >= MAX_PLRS) {
//#endif
		dev_fatal("ParseMsg: illegal player %d", pnum);
	}
	switch (pCmd->bCmd) {
	case NMSG_PLRINFO:
		return On_PLRINFO(pCmd, pnum);
	case NMSG_DLEVEL_DATA:
	case NMSG_DLEVEL_JUNK:
	case NMSG_DLEVEL_PLR:
	case NMSG_DLEVEL_END:
		return On_DLEVEL(pCmd, pnum);
	case NMSG_LVL_DELTA:
	case NMSG_LVL_DELTA_END:
		return On_LVL_DELTA(pCmd, pnum);
	case NMSG_STRING:
		return On_STRING(pCmd, pnum);
	}

	SNetDropPlayer(pnum);
	return 0;
}

unsigned ParseCmd(int pnum, const TCmd* pCmd)
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
	case CMD_WALKDIR:
		return On_WALKDIR(pCmd, pnum);
	case CMD_SKILLXY:
		return On_SKILLXY(pCmd, pnum);
	case CMD_OPOBJXY:
		return On_OPOBJXY(pCmd, pnum);
	case CMD_DISARMXY:
		return On_DISARMXY(pCmd, pnum);
	case CMD_SKILLMON:
		return On_SKILLMON(pCmd, pnum);
	case CMD_SKILLPLR:
		return On_SKILLPLR(pCmd, pnum);
	case CMD_TURN:
		return On_TURN(pCmd, pnum);
	case CMD_BLOCK:
		return On_BLOCK(pCmd, pnum);
	case CMD_TALKMON:
		return On_TALKMON(pCmd, pnum);
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
	case CMD_DECHP:
		return On_DECHP(pCmd, pnum);
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
	case CMD_USEPLRMAP:
		return On_USEPLRMAP(pCmd, pnum);
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
	//case CMD_TRAPOPEN:
	//	return On_TRAPOPEN(pCmd, pnum);
	//case CMD_TRAPCLOSE:
	//	return On_TRAPCLOSE(pCmd, pnum);
	case CMD_SHRINE:
		return On_SHRINE(pCmd, pnum);
	case CMD_TELEKINITM:
		return On_TELEKINITM(pCmd, pnum);
	case CMD_TELEKINMON:
		return On_TELEKINMON(pCmd, pnum);
	case CMD_TELEKINPLR:
		return On_TELEKINPLR(pCmd, pnum);
	case CMD_TELEKINOBJ:
		return On_TELEKINOBJ(pCmd, pnum);
	case CMD_ACTIVATEPORTAL:
		return On_ACTIVATEPORTAL(pCmd, pnum);
	case CMD_NEWLVL:
		return On_NEWLVL(pCmd, pnum);
	case CMD_CREATELVL:
		return On_CREATELVL(pCmd, pnum);
	case CMD_USEPORTAL:
		return On_USEPORTAL(pCmd, pnum);
	case CMD_RETOWN:
		return On_RETOWN(pCmd, pnum);
	case CMD_JOINLEVEL:
		return On_JOINLEVEL(pCmd, pnum);
	case CMD_DISCONNECT:
		return On_DISCONNECT(pCmd, pnum);
	case CMD_REQDELTA:
		return On_REQDELTA(pCmd, pnum);
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
#ifdef HELLFIRE
	case CMD_OPENNAKRUL:
		return On_OPENNAKRUL(pCmd, pnum);
#endif
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
	}

	SNetDropPlayer(pnum);
	return NET_TURN_MSG_SIZE;
}

DEVILUTION_END_NAMESPACE
