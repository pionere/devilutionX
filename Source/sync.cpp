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

	if (!IsMultiGame || gbLvlLoad /*|| nummonsters < 1*/) { // nummonsters is always >= MAX_MINIONS
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
			symon->nmndx = idx;
			symon->nmx = mon->_mx;
			symon->nmy = mon->_my;
			symon->nmdir = mon->_mdir;
			symon->nmleaderflag = mon->_mleaderflag;
			symon->nmhitpoints = mon->_mhitpoints;
			symon->nmactive = mon->_msquelch;

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
	sync_mnum = symon->nmndx;

	pHdr->bCmd = CMD_SYNCDATA;
	pHdr->bLevel = currLvl._dLevelIdx;
	pHdr->wLen = static_cast<uint16_t>(wLen);
	return pbBuf;
}

void InitSync()
{
	sync_mnum = 16 * mypnum;
}

DEVILUTION_END_NAMESPACE
