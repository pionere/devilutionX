/**
 * @file dead.cpp
 *
 * Implementation of functions for placing dead monsters.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** unused, this was probably for blood boil/burn */
int spurtndx;
DeadStruct dead[MAXDEAD];
int stonendx;

void InitDead()
{
	MonsterStruct *mon;
	CMonster *cmon;
	int i, d, nd;
	int mtypes[MAXMONSTERS];

	memset(mtypes, 0, sizeof(mtypes));

	nd = 0;
	cmon = Monsters;
	for (i = nummtypes; i > 0; i--, cmon++) {
		if (mtypes[cmon->mtype] == 0) {
			for (d = 0; d < 8; d++)
				dead[nd]._deadData[d] = cmon->Anims[MA_DEATH].Data[d];
			dead[nd]._deadFrame = cmon->Anims[MA_DEATH].Frames;
			dead[nd]._deadWidth = cmon->width;
			dead[nd]._deadWidth2 = cmon->width2;
			dead[nd]._deadtrans = 0;
			cmon->mdeadval = nd + 1;
			mtypes[cmon->mtype] = nd + 1;
			nd++;
		}
	}

	for (d = 0; d < 8; d++)
		dead[nd]._deadData[d] = misfiledata[MFILE_BLODBUR].mfAnimData[0];
	dead[nd]._deadFrame = 8;
	dead[nd]._deadWidth = 128;
	dead[nd]._deadWidth2 = 32;
	dead[nd]._deadtrans = 0;
	spurtndx = nd + 1;
	nd++;

	for (d = 0; d < 8; d++)
		dead[nd]._deadData[d] = misfiledata[MFILE_SHATTER1].mfAnimData[0];
	dead[nd]._deadFrame = 12;
	dead[nd]._deadWidth = 128;
	dead[nd]._deadWidth2 = 32;
	dead[nd]._deadtrans = 0;
	stonendx = nd + 1;
	nd++;

	for (i = 0; i < nummonsters; i++) {
		mon = &monster[monstactive[i]];
		if (mon->_uniqtype != 0) {
			for (d = 0; d < 8; d++)
				dead[nd]._deadData[d] = mon->MType->Anims[MA_DEATH].Data[d];
			dead[nd]._deadFrame = mon->MType->Anims[MA_DEATH].Frames;
			dead[nd]._deadWidth = mon->MType->width;
			dead[nd]._deadWidth2 = mon->MType->width2;
			dead[nd]._deadtrans = mon->_uniqtrans + 4;
			mon->_udeadval = nd + 1;
			nd++;
		}
	}

	assert(nd <= MAXDEAD);
}

void AddDead(int dx, int dy, char dv, int ddir)
{
	dDead[dx][dy] = (dv & 0x1F) + (ddir << 5);
}

void SetDead()
{
	int i, mi;
	int dx, dy;

	for (i = 0; i < nummonsters; i++) {
		mi = monstactive[i];
		if (monster[mi]._uniqtype != 0) {
			for (dx = 0; dx < MAXDUNX; dx++) {
				for (dy = 0; dy < MAXDUNY; dy++) {
					if ((dDead[dx][dy] & 0x1F) == monster[mi]._udeadval)
						ChangeLightXY(monster[mi].mlid, dx, dy);
				}
			}
		}
	}
}

DEVILUTION_END_NAMESPACE
