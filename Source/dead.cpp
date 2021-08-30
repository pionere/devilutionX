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
BYTE stonendx;

void InitDead()
{
	MonsterStruct *mon;
	MapMonData *cmon;
	int i, d, nd;
	bool mtypes[NUM_MTYPES];

	static_assert(false == 0, "InitDead fills mtypes with 0 instead of false values.");
	memset(mtypes, 0, sizeof(mtypes));

	nd = 0;
	cmon = mapMonTypes;
	for (i = nummtypes; i > 0; i--, cmon++) {
		if (!mtypes[cmon->cmType]) {
			for (d = 0; d < lengthof(dead[nd]._deadData); d++)
				dead[nd]._deadData[d] = cmon->cmAnims[MA_DEATH].aData[d];
			dead[nd]._deadFrame = cmon->cmAnims[MA_DEATH].aFrames;
			dead[nd]._deadWidth = cmon->cmWidth;
			dead[nd]._deadXOffset = cmon->cmXOffset;
			dead[nd]._deadtrans = 0;
			cmon->cmDeadval = nd + 1;
			mtypes[cmon->cmType] = true;
			nd++;
		}
	}

	/*for (d = 0; d < lengthof(dead[nd]._deadData); d++)
		dead[nd]._deadData[d] = misanimdata[MFILE_BLODBUR][0];
	dead[nd]._deadFrame = 8;
	dead[nd]._deadWidth = 128;
	dead[nd]._deadXOffset = 32;
	dead[nd]._deadtrans = 0;
	spurtndx = nd + 1;
	nd++;*/

	for (d = 0; d < lengthof(dead[nd]._deadData); d++)
		dead[nd]._deadData[d] = misanimdata[MFILE_SHATTER1][0];
	dead[nd]._deadFrame = 12;
	dead[nd]._deadWidth = 128;
	dead[nd]._deadXOffset = 32;
	dead[nd]._deadtrans = 0;
	stonendx = nd + 1;
	nd++;

	for (i = MAX_MINIONS; i < nummonsters; i++) {
		mon = &monsters[monstactive[i]];
		if (mon->_uniqtype != 0) {
			for (d = 0; d < lengthof(dead[nd]._deadData); d++)
				dead[nd]._deadData[d] = mon->_mAnims[MA_DEATH].aData[d];
			dead[nd]._deadFrame = mon->_mAnims[MA_DEATH].aFrames;
			dead[nd]._deadWidth = mon->_mAnimWidth;
			dead[nd]._deadXOffset = mon->_mAnimXOffset;
			dead[nd]._deadtrans = mon->_uniqtrans;
			mon->_udeadval = nd + 1;
			nd++;
		}
	}

	assert(nd <= MAXDEAD);
}

void AddDead(int mnum, bool sync)
{
	MonsterStruct *mon;
	int dx, dy;
	BYTE dv;

	if (mnum >= MAX_MINIONS)
		MonUpdateLeader(mnum);

	mon = &monsters[mnum];
	mon->_mDelFlag = TRUE;

	static_assert(MAXDEAD <= 0x1F, "Encoding of dDead requires the maximum number of deads to be low.");
	dv = mon->_mmode == MM_STONE ? stonendx : (mon->_uniqtype == 0 ? mon->MType->cmDeadval : mon->_udeadval);
	dv = (dv & 0x1F) + (mon->_mdir << 5);
	dx = mon->_mx;
	dy = mon->_my;
	if (!sync)
		dMonster[dx][dy] = 0;
	dDead[dx][dy] = dv;
}

void SyncDeadLight()
{
	MonsterStruct *mon;
	int i;
	int dx, dy;

	for (i = 0; i < nummonsters; i++) {
		mon = &monsters[monstactive[i]];
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
