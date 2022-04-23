/**
 * @file dead.cpp
 *
 * Implementation of functions for placing dead monsters.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** unused, this was probably for blood boil/burn */
//int spurtndx;
DeadStruct dead[MAXDEAD];

void InitDead()
{
	MonsterStruct* mon;
	MapMonData* cmon;
	int i, d, nd;
#if DEBUG_MODE
	bool mtypes[NUM_MTYPES];
#endif

	nd = 0;
	for (d = 0; d < lengthof(dead[nd]._deadData); d++)
		dead[nd]._deadData[d] = misanimdata[MFILE_SHATTER1][0];
	dead[nd]._deadFrame = misfiledata[MFILE_SHATTER1].mfAnimLen[0];
	dead[nd]._deadWidth = misfiledata[MFILE_SHATTER1].mfAnimWidth;
	dead[nd]._deadXOffset = misfiledata[MFILE_SHATTER1].mfAnimXOffset;
	dead[nd]._deadtrans = 0;
	//stonendx = ++nd;
	++nd;
	assert(nd == STONENDX);
#if DEBUG_MODE
	static_assert(false == 0, "InitDead fills mtypes with 0 instead of false values.");
	memset(mtypes, 0, sizeof(mtypes));
#endif
	cmon = mapMonTypes;
	for (i = nummtypes; i > 0; i--, cmon++) {
#if DEBUG_MODE
		assert(!mtypes[cmon->cmType]);
		mtypes[cmon->cmType] = true;
#endif
		for (d = 0; d < lengthof(dead[nd]._deadData); d++)
			dead[nd]._deadData[d] = cmon->cmAnims[MA_DEATH].aData[d];
		dead[nd]._deadFrame = cmon->cmAnims[MA_DEATH].aFrames;
		dead[nd]._deadWidth = cmon->cmWidth;
		dead[nd]._deadXOffset = cmon->cmXOffset;
		dead[nd]._deadtrans = 0;
		cmon->cmDeadval = ++nd;
	}

	/*for (d = 0; d < lengthof(dead[nd]._deadData); d++)
		dead[nd]._deadData[d] = misanimdata[MFILE_BLODBUR][0];
	dead[nd]._deadFrame = 8;
	dead[nd]._deadWidth = 128;
	dead[nd]._deadXOffset = 32;
	dead[nd]._deadtrans = 0;
	spurtndx = nd + 1;
	nd++;*/

	for (i = 0; i < MAXMONSTERS; i++) {
		mon = &monsters[i];
		if (mon->_uniqtype != 0) {
			for (d = 0; d < lengthof(dead[nd]._deadData); d++)
				dead[nd]._deadData[d] = mon->_mAnims[MA_DEATH].aData[d];
			dead[nd]._deadFrame = mon->_mAnims[MA_DEATH].aFrames;
			dead[nd]._deadWidth = mon->_mAnimWidth;
			dead[nd]._deadXOffset = mon->_mAnimXOffset;
			dead[nd]._deadtrans = mon->_uniqtrans;
			mon->_udeadval = ++nd;
		}
	}

	assert(nd <= MAXDEAD);
}

/*
 * Add the corpse of the monster.
 *
 * @param mnum: the monster which died
 * @param stone: force a 'stone'-corpse
 */
void AddDead(int mnum, bool stone)
{
	MonsterStruct* mon;
	BYTE dv;

	mon = &monsters[mnum];
	static_assert(MAXDEAD < (1 << 5), "Encoding of dDead requires the maximum number of deads to be low.");
	if (!stone && mon->_mType != MT_GOLEM) {
		dv = mon->_uniqtype == 0 ? mon->MType->cmDeadval : mon->_udeadval;
		dv |= (mon->_mdir << 5);
	} else {
		dv = STONENDX;
	}
	// assert(dv < MAXDEAD);
	dDead[mon->_mx][mon->_my] = dv;
}

void SyncDeadLight()
{
	MonsterStruct* mon;
	int i;
	int dx, dy;

	for (i = 0; i < MAXMONSTERS; i++) {
		mon = &monsters[i];
		if (mon->mlid != NO_LIGHT) {
			for (dx = 0; dx < MAXDUNX; dx++) {
				for (dy = 0; dy < MAXDUNY; dy++) {
					if ((dDead[dx][dy] & 0x1F) == mon->_udeadval)
						ChangeLightXY(mon->mlid, dx, dy);
				}
			}
		}
	}
}

DEVILUTION_END_NAMESPACE
