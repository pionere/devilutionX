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
int stonendx;

void InitDead()
{
	MonsterStruct *mon;
	CMonster *cmon;
	int i, d, nd;
	bool mtypes[NUM_MTYPES];

	static_assert(false == 0, "InitDead fills mtypes with 0 instead of false values.");
	memset(mtypes, 0, sizeof(mtypes));

	nd = 0;
	cmon = Monsters;
	for (i = nummtypes; i > 0; i--, cmon++) {
		if (!mtypes[cmon->mtype]) {
			for (d = 0; d < lengthof(dead[nd]._deadData); d++)
				dead[nd]._deadData[d] = cmon->Anims[MA_DEATH].Data[d];
			dead[nd]._deadFrame = cmon->Anims[MA_DEATH].Frames;
			dead[nd]._deadWidth = cmon->width;
			dead[nd]._deadWidth2 = cmon->width2;
			dead[nd]._deadtrans = 0;
			cmon->mdeadval = nd + 1;
			mtypes[cmon->mtype] = true;
			nd++;
		}
	}

	/*for (d = 0; d < lengthof(dead[nd]._deadData); d++)
		dead[nd]._deadData[d] = misanimdata[MFILE_BLODBUR][0];
	dead[nd]._deadFrame = 8;
	dead[nd]._deadWidth = 128;
	dead[nd]._deadWidth2 = 32;
	dead[nd]._deadtrans = 0;
	spurtndx = nd + 1;
	nd++;*/

	for (d = 0; d < lengthof(dead[nd]._deadData); d++)
		dead[nd]._deadData[d] = misanimdata[MFILE_SHATTER1][0];
	dead[nd]._deadFrame = 12;
	dead[nd]._deadWidth = 128;
	dead[nd]._deadWidth2 = 32;
	dead[nd]._deadtrans = 0;
	stonendx = nd + 1;
	nd++;

	for (i = MAX_MINIONS; i < nummonsters; i++) {
		mon = &monster[monstactive[i]];
		if (mon->_uniqtype != 0) {
			for (d = 0; d < lengthof(dead[nd]._deadData); d++)
				dead[nd]._deadData[d] = mon->_mAnims[MA_DEATH].Data[d];
			dead[nd]._deadFrame = mon->_mAnims[MA_DEATH].Frames;
			dead[nd]._deadWidth = mon->_mAnimWidth;
			dead[nd]._deadWidth2 = mon->_mAnimWidth2;
			dead[nd]._deadtrans = mon->_uniqtrans + 4;
			mon->_udeadval = nd + 1;
			nd++;
		}
	}

	assert(nd <= MAXDEAD);
}

void AddDead(int mnum)
{
	MonsterStruct *mon;
	int dx, dy, dir, dv;

	if (mnum >= MAX_MINIONS)
		MonUpdateLeader(mnum);

	mon = &monster[mnum];
	mon->_mDelFlag = TRUE;

	dx = mon->_mx;
	dy = mon->_my;
	dv = mon->_mmode == MM_STONE ? stonendx : (mon->_uniqtype == 0 ? mon->MType->mdeadval : mon->_udeadval);
	dir = mon->_mdir;
	dMonster[dx][dy] = 0;
	dDead[dx][dy] = (dv & 0x1F) + (dir << 5);
}

void SetDead()
{
	MonsterStruct *mon;
	int i;
	int dx, dy;

	for (i = 0; i < nummonsters; i++) {
		mon = &monster[monstactive[i]];
		if (mon->mlid != 0 && mon->_mDelFlag) {
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
