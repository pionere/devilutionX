/**
 * @file sync.cpp
 *
 * Implementation of functionality for syncing game state with other players.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/* last synchronized monster */
int sync_mnum;

BYTE* sync_all_monsters(BYTE* pbBuf, unsigned size)
{
	TSyncHeader* pHdr;
	int i, idx;
	unsigned remsize = size;
	size_t wLen;
	MonsterStruct* mon;
	TSyncMonster* symon;

	if (!IsMultiGame || gbLvlLoad == 10 /*|| nummonsters < 1*/) { // nummonsters is always >= MAX_MINIONS
		return pbBuf;
	}
	if (remsize < sizeof(*pHdr) + sizeof(TSyncMonster)) {
		return pbBuf;
	}
	assert(remsize <= 0xFFFF);

	pHdr = (TSyncHeader*)pbBuf;
	pbBuf += sizeof(*pHdr);
	remsize -= sizeof(*pHdr);

	idx = sync_mnum;
	for (i = 0; i < MAXMONSTERS; i++) {
		if (++idx >= MAXMONSTERS) {
			idx = 0;
		}
		mon = &monsters[idx];
		if (mon->_msquelch != 0 && mon->_mhitpoints != 0) {
			// assert(mon->_mmode <= MM_INGAME_LAST);
			symon = (TSyncMonster*)pbBuf;
			static_assert(MAXMONSTERS <= UCHAR_MAX, "Monster indices are transferred as BYTEs in sync_all_monsters.");
			symon->_mndx = idx;
			symon->_mx = mon->_mx;
			symon->_my = mon->_my;
			symon->_mdir = mon->_mdir;
			symon->_mleaderflag = mon->leaderflag;
			symon->_mhitpoints = SDL_SwapLE32(mon->_mhitpoints);
			symon->_mactive = SDL_SwapLE32(mon->_msquelch);

			pbBuf += sizeof(TSyncMonster);
			remsize -= sizeof(TSyncMonster);
			if (remsize < sizeof(TSyncMonster))
				break;
		}
	}
	wLen = (size_t)pbBuf - (size_t)&pHdr[1];
	if (wLen == 0)
		return (BYTE*)pHdr;
	//*size = remsize;
	symon = (TSyncMonster*)(pbBuf - sizeof(TSyncMonster));
	sync_mnum = symon->_mndx;

	pHdr->bCmd = CMD_SYNCDATA;
	pHdr->bLevel = currLvl._dLevelIdx;
	pHdr->wLen = SwapLE16(wLen);
	return pbBuf;
}

void InitSync()
{
	sync_mnum = 16 * mypnum;
}

DEVILUTION_END_NAMESPACE
