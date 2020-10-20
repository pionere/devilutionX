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
int sync_inum;	// next active item to be synchronized
int sync_pinum;	// next item to be synchronized on the body of the current player

DWORD sync_all_monsters(const BYTE *pbBuf, DWORD dwMaxLen)
{
	TSyncHeader *pHdr;
	int i;
	BOOL sync;

	if (nummonsters < 1) {
		return dwMaxLen;
	}
	if (dwMaxLen < sizeof(*pHdr) + sizeof(TSyncMonster)) {
		return dwMaxLen;
	}

	pHdr = (TSyncHeader *)pbBuf;
	pbBuf += sizeof(*pHdr);
	dwMaxLen -= sizeof(*pHdr);

	pHdr->bCmd = CMD_SYNCDATA;
	pHdr->bLevel = currlevel;
	pHdr->wLen = 0;
	SyncPlrInv(pHdr);
	assert(dwMaxLen <= 0xffff);
	sync_init_monsters();

	for (i = 0; i < nummonsters && dwMaxLen >= sizeof(TSyncMonster); i++) {
		sync = FALSE;
		if (i < 2) {
			sync = sync_prio_monster((TSyncMonster *)pbBuf);
		}
		if (!sync) {
			sync = sync_closest_monster((TSyncMonster *)pbBuf);
		}
		if (!sync) {
			break;
		}
		pbBuf += sizeof(TSyncMonster);
		pHdr->wLen += sizeof(TSyncMonster);
		dwMaxLen -= sizeof(TSyncMonster);
	}

	return dwMaxLen;
}

void sync_init_monsters()
{
	int i, m;

	for (i = 0; i < nummonsters; i++) {
		m = monstactive[i];
		monster_dists[m] = abs(plr[myplr]._px - monster[m]._mx) + abs(plr[myplr]._py - monster[m]._my);
		if (monster[m]._msquelch == 0) {
			monster_dists[m] += 0x1000;
		} else if (monster_prio[m] != 0) {
			monster_prio[m]--;
		}
	}
}

BOOL sync_closest_monster(TSyncMonster *symon)
{
	int i, mnum, ndx;
	DWORD lru;

	ndx = -1;
	lru = 0xFFFFFFFF;

	for (i = 0; i < nummonsters; i++) {
		mnum = monstactive[i];
		if (monster_dists[mnum] < lru && monster_prio[mnum] < 0xFFFE) {
			lru = monster_dists[mnum];
			ndx = monstactive[i];
		}
	}

	if (ndx == -1) {
		return FALSE;
	}

	sync_monster_pos(symon, ndx);
	return TRUE;
}

void sync_monster_pos(TSyncMonster *symon, int mnum)
{
	symon->_mndx = mnum;
	symon->_mx = monster[mnum]._mx;
	symon->_my = monster[mnum]._my;
	symon->_menemy = encode_enemy(mnum);
	symon->_mdelta = monster_dists[mnum] > 255 ? 255 : monster_dists[mnum];

	monster_dists[mnum] = 0xFFFF;
	monster_prio[mnum] = monster[mnum]._msquelch == 0 ? 0xFFFF : 0xFFFE;
}

BOOL sync_prio_monster(TSyncMonster *symon)
{
	int i, mnum, ndx;
	DWORD lru;

	ndx = -1;
	lru = 0xFFFE;

	for (i = 0; i < nummonsters; i++) {
		if (sync_mnum >= nummonsters) {
			sync_mnum = 0;
		}
		mnum = monstactive[sync_mnum];
		if (monster_prio[mnum] < lru) {
			lru = monster_prio[mnum];
			ndx = monstactive[sync_mnum];
		}
		sync_mnum++;
	}

	if (ndx == -1) {
		return FALSE;
	}

	sync_monster_pos(symon, ndx);
	return TRUE;
}

void SyncPlrInv(TSyncHeader *pHdr)
{
	int ii;
	ItemStruct *is;

	if (numitems > 0) {
		if (sync_inum >= numitems) {
			sync_inum = 0;
		}
		ii = itemactive[sync_inum++];
		pHdr->bItemI = ii;
		pHdr->bItemX = item[ii]._ix;
		pHdr->bItemY = item[ii]._iy;
		pHdr->wItemIndx = item[ii].IDidx;
		if (item[ii].IDidx == IDI_EAR) {
			pHdr->wItemCI = (item[ii]._iName[7] << 8) | item[ii]._iName[8];
			pHdr->dwItemSeed = (item[ii]._iName[9] << 24) | (item[ii]._iName[10] << 16) | (item[ii]._iName[11] << 8) | item[ii]._iName[12];
			pHdr->bItemId = item[ii]._iName[13];
			pHdr->bItemDur = item[ii]._iName[14];
			pHdr->bItemMDur = item[ii]._iName[15];
			pHdr->bItemCh = item[ii]._iName[16];
			pHdr->bItemMCh = item[ii]._iName[17];
			pHdr->wItemVal = (item[ii]._iName[18] << 8) | ((item[ii]._iCurs - ICURS_EAR_SORCEROR) << 6) | item[ii]._ivalue;
			pHdr->dwItemBuff = (item[ii]._iName[19] << 24) | (item[ii]._iName[20] << 16) | (item[ii]._iName[21] << 8) | item[ii]._iName[22];
		} else {
			pHdr->wItemCI = item[ii]._iCreateInfo;
			pHdr->dwItemSeed = item[ii]._iSeed;
			pHdr->bItemId = item[ii]._iIdentified;
			pHdr->bItemDur = item[ii]._iDurability;
			pHdr->bItemMDur = item[ii]._iMaxDur;
			pHdr->bItemCh = item[ii]._iCharges;
			pHdr->bItemMCh = item[ii]._iMaxCharges;
			if (item[ii].IDidx == IDI_GOLD) {
				pHdr->wItemVal = item[ii]._ivalue;
			}
		}
	} else {
		pHdr->bItemI = -1;
	}

	assert((DWORD)sync_pinum < NUM_INVLOC);
	is = &plr[myplr].InvBody[sync_pinum];
	if (is->_itype != ITYPE_NONE) {
		pHdr->bPInvLoc = sync_pinum;
		pHdr->wPInvIndx = is->IDidx;
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
}

DWORD sync_update(int pnum, const BYTE *pbBuf)
{
	TSyncHeader *pHdr;
	WORD wLen;

	pHdr = (TSyncHeader *)pbBuf;
	pbBuf += sizeof(*pHdr);

	if (pHdr->bCmd != CMD_SYNCDATA) {
		app_fatal("bad sync command");
	}

	/// ASSERT: assert(gbBufferMsgs != BUFFER_PROCESS);

	if (gbBufferMsgs == 1) {
		return pHdr->wLen + sizeof(*pHdr);
	}
	if (pnum == myplr) {
		return pHdr->wLen + sizeof(*pHdr);
	}

	for (wLen = pHdr->wLen; wLen >= sizeof(TSyncMonster); wLen -= sizeof(TSyncMonster)) {
		if (currlevel == pHdr->bLevel) {
			sync_monster(pnum, (TSyncMonster *)pbBuf);
		}
		delta_sync_monster((TSyncMonster *)pbBuf, pHdr->bLevel);
		pbBuf += sizeof(TSyncMonster);
	}

	assert(wLen == 0);

	return pHdr->wLen + sizeof(*pHdr);
}

void sync_monster(int pnum, const TSyncMonster *symon)
{
	int i, mnum, md, mdx, mdy;
	DWORD delta;

	mnum = symon->_mndx;

#ifdef HELLFIRE
	if (monster[mnum]._mhitpoints <= 0) {
#else
	if (monster[mnum]._mhitpoints == 0) {
#endif
		return;
	}

	for (i = 0; i < nummonsters; i++) {
		if (monstactive[i] == mnum) {
			break;
		}
	}

	delta = abs(plr[myplr]._px - monster[mnum]._mx) + abs(plr[myplr]._py - monster[mnum]._my);
	if (delta > 255) {
		delta = 255;
	}

	if (delta < symon->_mdelta || (delta == symon->_mdelta && pnum > myplr)) {
		return;
	}
	if (monster[mnum]._mfutx == symon->_mx && monster[mnum]._mfuty == symon->_my) {
		return;
	}
	if (monster[mnum]._mmode == MM_CHARGE || monster[mnum]._mmode == MM_STONE) {
		return;
	}

	mdx = abs(monster[mnum]._mx - symon->_mx);
	mdy = abs(monster[mnum]._my - symon->_my);
	if (mdx <= 2 && mdy <= 2) {
		if (monster[mnum]._mmode < MM_WALK || monster[mnum]._mmode > MM_WALK3) {
			md = GetDirection(monster[mnum]._mx, monster[mnum]._my, symon->_mx, symon->_my);
			if (DirOK(mnum, md)) {
				MonClearSquares(mnum);
				dMonster[monster[mnum]._mx][monster[mnum]._my] = mnum + 1;
				MonWalkDir(mnum, md);
				monster[mnum]._msquelch = UCHAR_MAX;
			}
		}
	} else if (dMonster[symon->_mx][symon->_my] == 0) {
		MonClearSquares(mnum);
		dMonster[symon->_mx][symon->_my] = mnum + 1;
		monster[mnum]._mx = symon->_mx;
		monster[mnum]._my = symon->_my;
		decode_enemy(mnum, symon->_menemy);
		md = GetDirection(symon->_mx, symon->_my, monster[mnum]._menemyx, monster[mnum]._menemyy);
		MonStartStand(mnum, md);
		monster[mnum]._msquelch = UCHAR_MAX;
	}

	decode_enemy(mnum, symon->_menemy);
}

void sync_init()
{
	sync_mnum = 16 * myplr;
	memset(monster_prio, 255, sizeof(monster_prio));
}

DEVILUTION_END_NAMESPACE
