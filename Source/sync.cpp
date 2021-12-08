/**
 * @file sync.cpp
 *
 * Implementation of functionality for syncing game state with other players.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

WORD monster_dists[MAXMONSTERS];
WORD monster_prio[MAXMONSTERS];
int sync_mnum;	// next monster to be synchronized
//int sync_inum;	// next active item to be synchronized
//int sync_pinum;	// next item to be synchronized on the body of the current player

static void sync_init_monsters()
{
	int i, mnum, px, py;

	px = myplr._px;
	py = myplr._py;
	for (i = 0; i < nummonsters; i++) {
		mnum = monstactive[i];
		static_assert(MAXDUNX + MAXDUNY + 0x1000 < 0xFFFE, "sync_init_monsters expects a dungeon to fit to 16-bit I.");
		monster_dists[mnum] = abs(px - monsters[mnum]._mx) + abs(py - monsters[mnum]._my);
		if (monsters[mnum]._msquelch == 0) {
			static_assert(MAXDUNX + MAXDUNY < 0x1000, "sync_init_monsters expects a dungeon to fit to 16-bit I/a.");
			monster_dists[mnum] |= 0x1000;
		} else if (monster_prio[mnum] != 0) {
			monster_prio[mnum]--;
		}
	}
}

static bool sync_monster_pos(TSyncMonster* symon, int mnum)
{
	MonsterStruct* mon = &monsters[mnum];

	static_assert(MAXDUNX + MAXDUNY + 0x1000 < 0xFFFE, "sync_init_monsters expects a dungeon to fit to 16-bit II.");
	monster_dists[mnum] = 0xFFFE;
	monster_prio[mnum] = mon->_msquelch == 0 ? 0xFFFF : 0xFFFE;

	if (mon->_mhitpoints < (1 << 6))
		return false;

	symon->_mndx = mnum;
	symon->_mx = mon->_mx;
	symon->_my = mon->_my;
	symon->_mdir = mon->_mdir;
	symon->_menemy = encode_enemy(mnum);
	symon->_mhitpoints = SDL_SwapLE32(mon->_mhitpoints);
	symon->_mactive = SDL_SwapLE32(mon->_msquelch);

	return true;
}

static int sync_closest_monster()
{
	int i, mnum, ndx;
	WORD minDist;

	ndx = -1;
	minDist = 0xFFFF;

	for (i = 0; i < nummonsters; i++) {
		mnum = monstactive[i];
		//assert(monster_dists[mnum] < 0xFFFF);
		if (monster_dists[mnum] < minDist && monster_prio[mnum] < 0xFFFE) {
			minDist = monster_dists[mnum];
			ndx = mnum;
		}
	}

	return ndx;
}

static int sync_prio_monster()
{
	int i, mnum, ndx;
	WORD minPrio;

	ndx = -1;
	minPrio = 0xFFFE;

	for (i = 0; i < nummonsters; i++) {
		if (++sync_mnum >= nummonsters) {
			sync_mnum = 0;
		}
		mnum = monstactive[sync_mnum];
		if (monster_prio[mnum] < minPrio) {
			minPrio = monster_prio[mnum];
			ndx = mnum;
		}
	}

	return ndx;
}

/*static void SyncPlrInv(TSyncHeader* pHdr)
{
	int ii;
	ItemStruct* is;

	if (numitems > 0) {
		if (sync_inum >= numitems) {
			sync_inum = 0;
		}
		ii = itemactive[sync_inum++];
		pHdr->bItemI = ii;
		is = &items[ii];
		pHdr->bItemX = is->_ix;
		pHdr->bItemY = is->_iy;
		pHdr->wItemIndx = is->_iIdx;
		if (is->_iIdx == IDI_EAR) {
			pHdr->wItemCI = (is->_iName[7] << 8) | is->_iName[8];
			pHdr->dwItemSeed = (is->_iName[9] << 24) | (is->_iName[10] << 16) | (is->_iName[11] << 8) | is->_iName[12];
			pHdr->bItemId = is->_iName[13];
			pHdr->bItemDur = is->_iName[14];
			pHdr->bItemMDur = is->_iName[15];
			pHdr->bItemCh = is->_iName[16];
			pHdr->bItemMCh = is->_iName[17];
			pHdr->wItemVal = (is->_iName[18] << 8) | ((is->_iCurs - ICURS_EAR_SORCERER) << 6) | is->_ivalue;
			pHdr->dwItemBuff = (is->_iName[19] << 24) | (is->_iName[20] << 16) | (is->_iName[21] << 8) | is->_iName[22];
		} else {
			pHdr->wItemCI = is->_iCreateInfo;
			pHdr->dwItemSeed = is->_iSeed;
			pHdr->bItemId = is->_iIdentified;
			pHdr->bItemDur = is->_iDurability;
			pHdr->bItemMDur = is->_iMaxDur;
			pHdr->bItemCh = is->_iCharges;
			pHdr->bItemMCh = is->_iMaxCharges;
			if (is->_iIdx == IDI_GOLD) {
				pHdr->wItemVal = is->_ivalue;
			}
		}
	} else {
		pHdr->bItemI = -1;
	}

	assert((unsigned)sync_pinum < NUM_INVLOC);
	is = &myplr._pInvBody[sync_pinum];
	if (is->_itype != ITYPE_NONE) {
		pHdr->bPInvLoc = sync_pinum;
		pHdr->wPInvIndx = is->_iIdx;
		pHdr->wPInvCI = is->_iCreateInfo;
		pHdr->dwPInvSeed = is->_iSeed;
		pHdr->bPInvId = is->_iIdentified;
	} else {
		pHdr->bPInvLoc = -1;
	}

	sync_pinum++;
	if (sync_pinum >= NUM_INVLOC) {
		sync_pinum = 0;
	}
}*/

unsigned sync_all_monsters(const BYTE* pbBuf, unsigned dwMaxLen)
{
	TSyncHeader* pHdr;
	int i, idx;
	WORD wLen;

	if (!IsMultiGame || gbLvlLoad == 10 /*|| nummonsters < 1*/) { // nummonsters is always >= MAX_MINIONS
		return dwMaxLen;
	}
	if (dwMaxLen < sizeof(*pHdr) + sizeof(TSyncMonster)) {
		return dwMaxLen;
	}

	sync_init_monsters();

	pHdr = (TSyncHeader*)pbBuf;
	pbBuf += sizeof(*pHdr);
	dwMaxLen -= sizeof(*pHdr);

	wLen = 0;
	//SyncPlrInv(pHdr);
	assert(dwMaxLen <= 0xffff);

	for (i = 0; i < nummonsters && dwMaxLen >= sizeof(TSyncMonster); i++) {
		idx = -1;
		if (i < 2)
			idx = sync_prio_monster();
		if (idx == -1)
			idx = sync_closest_monster();
		if (idx == -1)
			break;
		if (!sync_monster_pos((TSyncMonster *)pbBuf, idx))
			continue;
		pbBuf += sizeof(TSyncMonster);
		wLen += sizeof(TSyncMonster);
		dwMaxLen -= sizeof(TSyncMonster);
	}
	pHdr->bCmd = CMD_SYNCDATA;
	pHdr->bLevel = currLvl._dLevelIdx;
	pHdr->wLen = SwapLE16(wLen);
	return dwMaxLen;
}

/*static void sync_monster(int pnum, const TSyncMonster *symon)
{
	/*MonsterStruct *mon;
	int mnum, md;
	DWORD delta;

	mnum = symon->_mndx;
	mon = &monsters[mnum];

	delta = abs(myplr._px - mon->_mx) + abs(myplr._py - mon->_my);
	if (delta > 255) {
		delta = 255;
	}

	if (delta < symon->_mdelta || (delta == symon->_mdelta && pnum > mypnum)) {
		return;
	}
	if (mon->_mfutx == symon->_mx && mon->_mfuty == symon->_my) {
		return;
	}
	if (mon->_mhitpoints <= 0 || mon->_mmode == MM_DEATH || mon->_mmode == MM_CHARGE || mon->_mmode == MM_STONE) {
		return;
	}

	decode_enemy(mnum, symon->_menemy);

	if (abs(mon->_mx - symon->_mx) <= 2 && abs(mon->_my - symon->_my) <= 2) {
		if (mon->_mmode < MM_WALK || mon->_mmode > MM_WALK2) {
			md = GetDirection(mon->_mx, mon->_my, symon->_mx, symon->_my);
			if (DirOK(mnum, md)) {
				RemoveMonFromMap(mnum);
				dMonster[mon->_mx][mon->_my] = mnum + 1;
				MonWalkDir(mnum, md);
				mon->_msquelch = SQUELCH_MAX;
			}
		}
	} else if (dMonster[symon->_mx][symon->_my] == 0) {
		RemoveMonFromMap(mnum);
		dMonster[symon->_mx][symon->_my] = mnum + 1;
		mon->_mx = symon->_mx;
		mon->_my = symon->_my;
		md = GetDirection(symon->_mx, symon->_my, mon->_menemyx, mon->_menemyy);
		MonStartStand(mnum, md);
		mon->_msquelch = SQUELCH_MAX;
	}*/
}

void sync_update(int pnum, const TSyncHeader *pHdr)
{
	const BYTE *pbBuf;
	unsigned wLen;

	pbBuf = (const BYTE *)&pHdr[1];

	//assert(pHdr->bCmd == CMD_SYNCDATA);
	//assert(currLvl._dLevelIdx == pHdr->bLevel);
	/// ASSERT: assert(geBufferMsgs != MSG_RUN_DELTA);
	//assert(geBufferMsgs != MSG_GAME_DELTA && pnum != mypnum);
	for (wLen = SwapLE16(pHdr->wLen); wLen >= sizeof(TSyncMonster); wLen -= sizeof(TSyncMonster)) {
		sync_monster(pnum, (TSyncMonster *)pbBuf);
		pbBuf += sizeof(TSyncMonster);
	}
	//assert(wLen == 0);
}*/

void InitSync()
{
	sync_mnum = 16 * mypnum;
	memset(monster_prio, 255, sizeof(monster_prio));
}

DEVILUTION_END_NAMESPACE
