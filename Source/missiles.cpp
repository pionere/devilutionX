/**
 * @file missiles.cpp
 *
 * Implementation of missile functionality.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int missileactive[MAXMISSILES];
int missileavail[MAXMISSILES];
MissileStruct missile[MAXMISSILES];
int nummissiles;
BOOL ManashieldFlag;
ChainStruct chain[MAXMISSILES];
BOOL MissilePreFlag;
int numchains;

/** Maps from direction to X-offset. */
int XDirAdd[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to Y-offset. */
int YDirAdd[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };
int CrawlNum[19] = { 0, 3, 12, 45, 94, 159, 240, 337, 450, 579, 724, 885, 1062, 1255, 1464, 1689, 1930, 2187, 2460 };

void GetDamageAmt(int sn, int *mind, int *maxd)
{
	int k, sl;

	assert((DWORD)myplr < MAX_PLRS);
	assert((DWORD)i < 64);
	sl = plr[myplr]._pSplLvl[sn] + plr[myplr]._pISplLvlAdd;

	switch (sn) {
	case SPL_FIREBOLT:
		*mind = (plr[myplr]._pMagic >> 3) + sl + 1;
		*maxd = (plr[myplr]._pMagic >> 3) + sl + 10;
		break;
	case SPL_HEAL: /// BUGFIX: healing calculation is unused
		*mind = plr[myplr]._pLevel + sl + 1;
#ifdef HELLFIRE
		if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_MONK || plr[myplr]._pClass == PC_BARBARIAN) {
#else
		if (plr[myplr]._pClass == PC_WARRIOR) {
#endif
			*mind <<= 1;
		}
#ifdef HELLFIRE
		else if (plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_BARD) {
#else
		if (plr[myplr]._pClass == PC_ROGUE) {
#endif
			*mind += *mind >> 1;
		}
		*maxd = 10;
		for (k = 0; k < plr[myplr]._pLevel; k++) {
			*maxd += 4;
		}
		for (k = 0; k < sl; k++) {
			*maxd += 6;
		}
#ifdef HELLFIRE
		if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_MONK || plr[myplr]._pClass == PC_BARBARIAN) {
#else
		if (plr[myplr]._pClass == PC_WARRIOR) {
#endif
			*maxd <<= 1;
		}
#ifdef HELLFIRE
		else if (plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_BARD) {
#else
		if (plr[myplr]._pClass == PC_ROGUE) {
#endif
			*maxd += *maxd >> 1;
		}
		*mind = -1;
		*maxd = -1;
		break;
	case SPL_LIGHTNING:
		*mind = 2;
		*maxd = plr[myplr]._pLevel + 2;
		break;
	case SPL_FLASH:
		*mind = plr[myplr]._pLevel;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*mind += *mind >> 1;
		*maxd = *mind * 2;
		break;
	case SPL_IDENTIFY:
	case SPL_TOWN:
	case SPL_STONE:
	case SPL_INFRA:
	case SPL_RNDTELEPORT:
	case SPL_MANASHIELD:
	case SPL_DOOMSERP:
	case SPL_BLODRIT:
	case SPL_INVISIBIL:
	case SPL_BLODBOIL:
	case SPL_TELEPORT:
	case SPL_ETHEREALIZE:
	case SPL_REPAIR:
	case SPL_RECHARGE:
	case SPL_DISARM:
	case SPL_RESURRECT:
	case SPL_TELEKINESIS:
	case SPL_BONESPIRIT:
#ifdef HELLFIRE
	case SPL_WARP:
	case SPL_REFLECT:
	case SPL_BERSERK:
	case SPL_SEARCH:
	case SPL_RUNESTONE:
#endif
		*mind = -1;
		*maxd = -1;
		break;
	case SPL_FIREWALL:
		*mind = (4 * plr[myplr]._pLevel + 8) >> 1;
		*maxd = (4 * plr[myplr]._pLevel + 80) >> 1;
		break;
	case SPL_FIREBALL:
		*mind = 2 * plr[myplr]._pLevel + 4;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*maxd = 2 * plr[myplr]._pLevel + 40;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		break;
	case SPL_GUARDIAN:
		*mind = (plr[myplr]._pLevel >> 1) + 1;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*maxd = (plr[myplr]._pLevel >> 1) + 10;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		break;
	case SPL_CHAIN:
		*mind = 4;
		*maxd = 2 * plr[myplr]._pLevel + 4;
		break;
	case SPL_WAVE:
		*mind = 6 * (plr[myplr]._pLevel + 1);
		*maxd = 6 * (plr[myplr]._pLevel + 10);
		break;
	case SPL_NOVA:
		*mind = (plr[myplr]._pLevel + 5) >> 1;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*mind *= 5;
		*maxd = (plr[myplr]._pLevel + 30) >> 1;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		*maxd *= 5;
		break;
	case SPL_FLAME:
		*mind = 3;
		*maxd = plr[myplr]._pLevel + 4;
		*maxd += *maxd >> 1;
		break;
	case SPL_GOLEM:
		*mind = 11;
		*maxd = 17;
		break;
	case SPL_APOCA:
		*mind = 0;
		for (k = 0; k < plr[myplr]._pLevel; k++) {
			*mind += 1;
		}
		*maxd = 0;
		for (k = 0; k < plr[myplr]._pLevel; k++) {
			*maxd += 6;
		}
		break;
	case SPL_ELEMENT:
		*mind = 2 * plr[myplr]._pLevel + 4;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		/// BUGFIX: add here '*mind >>= 1;'
		*maxd = 2 * plr[myplr]._pLevel + 40;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		/// BUGFIX: add here '*maxd >>= 1;'
		break;
	case SPL_CBOLT:
		*mind = 1;
		*maxd = (plr[myplr]._pMagic >> 2) + 1;
		break;
	case SPL_HBOLT:
		*mind = plr[myplr]._pLevel + 9;
		*maxd = plr[myplr]._pLevel + 18;
		break;
	case SPL_HEALOTHER: /// BUGFIX: healing calculation is unused
		*mind = plr[myplr]._pLevel + sl + 1;
#ifdef HELLFIRE
		if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_MONK || plr[myplr]._pClass == PC_BARBARIAN) {
#else
		if (plr[myplr]._pClass == PC_WARRIOR) {
#endif
			*mind <<= 1;
		}
#ifdef HELLFIRE
		if (plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_BARD) {
#else
		if (plr[myplr]._pClass == PC_ROGUE) {
#endif
			*mind += *mind >> 1;
		}
		*maxd = 10;
		for (k = 0; k < plr[myplr]._pLevel; k++) {
			*maxd += 4;
		}
		for (k = 0; k < sl; k++) {
			*maxd += 6;
		}
#ifdef HELLFIRE
		if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_MONK || plr[myplr]._pClass == PC_BARBARIAN) {
#else
		if (plr[myplr]._pClass == PC_WARRIOR) {
#endif
			*maxd <<= 1;
		}
#ifdef HELLFIRE
		if (plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_BARD) {
#else
		if (plr[myplr]._pClass == PC_ROGUE) {
#endif
			*maxd += *maxd >> 1;
		}
		*mind = -1;
		*maxd = -1;
		break;
	case SPL_FLARE:
		*mind = (plr[myplr]._pMagic >> 1) + 3 * sl - (plr[myplr]._pMagic >> 3);
		*maxd = *mind;
		break;
	}
}

BOOL CheckBlock(int fx, int fy, int tx, int ty)
{
	int pn;
	BOOL coll;

	coll = FALSE;
	while (fx != tx || fy != ty) {
		pn = GetDirection(fx, fy, tx, ty);
		fx += XDirAdd[pn];
		fy += YDirAdd[pn];
		if (nSolidTable[dPiece[fx][fy]])
			coll = TRUE;
	}

	return coll;
}

int FindClosest(int sx, int sy, int rad)
{
	int j, i, mid, tx, ty, cr;

	if (rad > 19)
		rad = 19;

	for (i = 1; i < rad; i++) {
		cr = CrawlNum[i] + 1;
#ifdef HELLFIRE
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#else
		for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#endif
			tx = sx + CrawlTable[cr];
			ty = sy + CrawlTable[cr + 1];
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				mid = dMonster[tx][ty];
				if (mid > 0 && !CheckBlock(sx, sy, tx, ty))
					return mid - 1;
			}
		}
	}
	return -1;
}

int GetSpellLevel(int pnum, int sn)
{
	int result;

	if (pnum == myplr)
		result = plr[pnum]._pISplLvlAdd + plr[pnum]._pSplLvl[sn];
	else
		result = 1;

	if (result < 0)
		result = 0;

	return result;
}

int GetDirection8(int x1, int y1, int x2, int y2)
{
	BYTE Dirs[16][16] = {
		{ 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
		{ 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
		{ 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
	};
	int mx, my, md;
	ALIGN_BY_1 BYTE urtoll[] = { 3, 4, 5 },
	                ultolr[] = { 3, 2, 1 },
	                lrtoul[] = { 7, 6, 5 },
	                lltour[] = { 7, 0, 1 };

	mx = abs(x2 - x1);
	if (mx > 15)
		mx = 15;
	my = abs(y2 - y1);
	if (my > 15)
		my = 15;
	md = Dirs[my][mx];
	if (x1 > x2) {
		if (y1 > y2)
			md = urtoll[md];
		else
			md = ultolr[md];
	} else if (y1 > y2)
		md = lrtoul[md];
	else
		md = lltour[md];
	return md;
}

int GetDirection16(int x1, int y1, int x2, int y2)
{
	BYTE Dirs[16][16] = {
		{ 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 4, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 4, 3, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 4, 4, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // BUGFIX: should be `{ 4, 4, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },`
		{ 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 4, 4, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1 }, // BUGFIX: should be `{ 4, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1 },`
		{ 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2 }
	};
	// The correct quadrant of direction indices is presented below:
	/*
		{ 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 4, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 4, 3, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 4, 4, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 4, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1 },
		{ 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1 },
		{ 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2 },
		{ 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2 }
	*/

	BYTE urtoll[5] = { 6, 7, 8, 9, 10 };
	BYTE ultolr[5] = { 6, 5, 4, 3, 2 };
	BYTE lltour[5] = { 14, 13, 12, 11, 10 };
	BYTE lrtoul[5] = { 14, 15, 0, 1, 2 };
	int mx, my, md;

	mx = abs(x2 - x1);
	if (mx > 15)
		mx = 15;
	my = abs(y2 - y1);
	if (my > 15)
		my = 15;
	md = Dirs[my][mx];
	if (x1 > x2) {
		if (y1 > y2)
			md = urtoll[md];
		else
			md = ultolr[md];
	} else if (y1 > y2) {
		md = lltour[md];
	} else {
		md = lrtoul[md];
	}
	return md;
}

void DeleteMissile(int mi, int i)
{
	int src;

	if (missile[mi]._mitype == MIS_MANASHIELD) {
		src = missile[mi]._misource;
		if (src == myplr)
			NetSendCmd(TRUE, CMD_REMSHIELD);
		plr[src].pManaShield = FALSE;
	}

	missileavail[MAXMISSILES - nummissiles] = mi;
	nummissiles--;
	if (nummissiles > 0 && i != nummissiles)
		missileactive[i] = missileactive[nummissiles];
}

void GetMissileVel(int mi, int sx, int sy, int dx, int dy, int v)
{
	double dxp, dyp, dr;

	if (dx != sx || dy != sy) {
		dxp = (dx + sy - sx - dy) * (1 << 21);
		dyp = (dy + dx - sx - sy) * (1 << 21);
		dr = sqrt(dxp * dxp + dyp * dyp);
		missile[mi]._mixvel = (dxp * (v << 16)) / dr;
		missile[mi]._miyvel = (dyp * (v << 15)) / dr;
	} else {
		missile[mi]._mixvel = 0;
		missile[mi]._miyvel = 0;
	}
}

void PutMissile(int mi)
{
	int x, y;

	x = missile[mi]._mix;
	y = missile[mi]._miy;
	if (x <= 0 || y <= 0 || x >= MAXDUNX || y >= MAXDUNY)
		missile[mi]._miDelFlag = TRUE;
	if (!missile[mi]._miDelFlag) {
		dFlags[x][y] |= BFLAG_MISSILE;
		if (dMissile[x][y] == 0)
			dMissile[x][y] = mi + 1;
		else
			dMissile[x][y] = -1;
		if (missile[mi]._miPreFlag)
			MissilePreFlag = TRUE;
	}
}

void GetMissilePos(int mi)
{
	MissileStruct *mis;
	int mx, my, dx, dy, lx, ly;

	mis = &missile[mi];
	mx = mis->_mitxoff >> 16;
	my = mis->_mityoff >> 16;
	dx = mx + 2 * my;
	dy = 2 * my - mx;
	if (dx < 0) {
		lx = -(-dx / 8);
		dx = -(-dx / 64);
	} else {
		lx = dx / 8;
		dx = dx / 64;
	}
	if (dy < 0) {
		ly = -(-dy / 8);
		dy = -(-dy / 64);
	} else {
		ly = dy / 8;
		dy = dy / 64;
	}
	mis->_mix = dx + mis->_misx;
	mis->_miy = dy + mis->_misy;
	mis->_mixoff = mx + (dy * 32) - (dx * 32);
	mis->_miyoff = my - (dx * 16) - (dy * 16);
	ChangeLightOff(mis->_mlid, lx - (dx * 8), ly - (dy * 8));
}

void MoveMissilePos(int mi)
{
	MissileStruct *mis;
	int dx, dy, x, y;

	mis = &missile[mi];
	switch (mis->_mimfnum) {
	case DIR_S:
		dx = 1;
		dy = 1;
		break;
	case DIR_SW:
		dx = 1;
		dy = 1;
		break;
	case DIR_W:
		dx = 0;
		dy = 1;
		break;
	case DIR_NW:
		dx = 0;
		dy = 0;
		break;
	case DIR_N:
		dx = 0;
		dy = 0;
		break;
	case DIR_NE:
		dx = 0;
		dy = 0;
		break;
	case DIR_E:
		dx = 1;
		dy = 0;
		break;
	case DIR_SE:
		dx = 1;
		dy = 1;
		break;
	}
	x = mis->_mix + dx;
	y = mis->_miy + dy;
	if (PosOkMonst(mis->_misource, x, y)) {
		mis->_mix += dx;
		mis->_miy += dy;
		mis->_mixoff += (dy << 5) - (dx << 5);
		mis->_miyoff -= (dy << 4) + (dx << 4);
	}
}

BOOL MonsterTrapHit(int mnum, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift)
{
	MonsterStruct *mon;
	int hit, hper, dam, mor, mir;
	BOOL resist, ret;

	resist = FALSE;
	mon = &monster[mnum];
	if (mon->mtalkmsg) {
		return FALSE;
	}
	if (mon->_mhitpoints >> 6 <= 0) {
		return FALSE;
	}
	if (mon->MType->mtype == MT_ILLWEAV && mon->_mgoal == MGOAL_RETREAT)
		return FALSE;
	if (mon->_mmode == MM_CHARGE)
		return FALSE;

	mir = missiledata[mitype].mResist;
	mor = mon->mMagicRes;
	if (mor & IMUNE_MAGIC && mir == MISR_MAGIC
	    || mor & IMUNE_FIRE && mir == MISR_FIRE
	    || mor & IMUNE_LIGHTNING && mir == MISR_LIGHTNING) {
		return FALSE;
	}

	if ((mor & RESIST_MAGIC && mir == MISR_MAGIC)
	    || (mor & RESIST_FIRE && mir == MISR_FIRE)
	    || (mor & RESIST_LIGHTNING && mir == MISR_LIGHTNING)) {
		resist = TRUE;
	}

	hit = random_(68, 100);
#ifdef HELLFIRE
	hper = 90 - (char)mon->mArmorClass - dist;
#else
	hper = 90 - (BYTE)mon->mArmorClass - dist;
#endif
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (CheckMonsterHit(mnum, &ret)) {
		return ret;
	}
#ifdef _DEBUG
	else if (hit < hper || debug_mode_dollar_sign || debug_mode_key_inverted_v || mon->_mmode == MM_STONE) {
#else
	else if (hit < hper || mon->_mmode == MM_STONE) {
#endif
		dam = mindam + random_(68, maxdam - mindam + 1);
		if (!shift)
			dam <<= 6;
		if (resist)
			mon->_mhitpoints -= dam >> 2;
		else
			mon->_mhitpoints -= dam;
#ifdef _DEBUG
		if (debug_mode_dollar_sign || debug_mode_key_inverted_v)
			mon->_mhitpoints = 0;
#endif
		if (mon->_mhitpoints >> 6 <= 0) {
			if (mon->_mmode == MM_STONE) {
				MonStartKill(mnum, -1);
				mon->_mmode = MM_STONE;
			} else {
				MonStartKill(mnum, -1);
			}
		} else {
			if (resist) {
				PlayEffect(mnum, 1);
			} else if (mon->_mmode == MM_STONE) {
				if (mnum > MAX_PLRS - 1)
					MonStartHit(mnum, -1, dam);
				mon->_mmode = MM_STONE;
			} else {
				if (mnum > MAX_PLRS - 1)
					MonStartHit(mnum, -1, dam);
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL MonsterMHit(int pnum, int mnum, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift)
{
	PlayerStruct *p;
	MonsterStruct *mon;
	int hit, hper, dam, mor, mir;
	BOOL resist, ret;

	resist = FALSE;
	mon = &monster[mnum];
	if (mon->mtalkmsg
	    || mon->_mhitpoints >> 6 <= 0
	    || mitype == MIS_HBOLT && mon->MType->mtype != MT_DIABLO && mon->MData->mMonstClass != MC_UNDEAD) {
		return FALSE;
	}
	if (mon->MType->mtype == MT_ILLWEAV && mon->_mgoal == MGOAL_RETREAT)
		return FALSE;
	if (mon->_mmode == MM_CHARGE)
		return FALSE;

	mor = mon->mMagicRes;
	mir = missiledata[mitype].mResist;

	if (mor & IMUNE_MAGIC && mir == MISR_MAGIC
	    || mor & IMUNE_FIRE && mir == MISR_FIRE
	    || mor & IMUNE_LIGHTNING && mir == MISR_LIGHTNING
	    || (mor & IMUNE_ACID) && mir == MISR_ACID)
		return FALSE;

	if (mor & RESIST_MAGIC && mir == MISR_MAGIC
	    || mor & RESIST_FIRE && mir == MISR_FIRE
	    || mor & RESIST_LIGHTNING && mir == MISR_LIGHTNING)
		resist = TRUE;

	p = &plr[pnum];
	hit = random_(69, 100);
	if (!missiledata[mitype].mType) {
		hper = p->_pDexterity
		    + p->_pIBonusToHit
		    + p->_pLevel
		    - mon->mArmorClass
		    - (dist * dist >> 1)
		    + p->_pIEnAc
		    + 50;
		if (p->_pClass == PC_ROGUE)
			hper += 20;
		if (p->_pClass == PC_WARRIOR)
			hper += 10;
	} else {
		hper = p->_pMagic - (mon->mLevel << 1) - dist + 50;
		if (p->_pClass == PC_SORCERER)
			hper += 20;
	}
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (mon->_mmode == MM_STONE)
		hit = 0;
	if (CheckMonsterHit(mnum, &ret))
		return ret;
#ifdef _DEBUG
	if (hit >= hper && !debug_mode_key_inverted_v && !debug_mode_dollar_sign)
		return FALSE;
#else
	if (hit >= hper)
		return FALSE;
#endif
	if (mitype == MIS_BONESPIRIT) {
		dam = mon->_mhitpoints / 3 >> 6;
	} else {
		dam = mindam + random_(70, maxdam - mindam + 1);
	}
	if (!missiledata[mitype].mType) {
		dam = p->_pIBonusDamMod + dam * p->_pIBonusDam / 100 + dam;
		if (p->_pClass == PC_ROGUE)
			dam += p->_pDamageMod;
		else
			dam += (p->_pDamageMod >> 1);
	}
	if (!shift)
		dam <<= 6;
	if (resist)
		dam >>= 2;
	if (pnum == myplr)
		mon->_mhitpoints -= dam;
	if (p->_pIFlags & ISPL_FIRE_ARROWS)
		mon->_mFlags |= MFLAG_NOHEAL;

	if (mon->_mhitpoints >> 6 <= 0) {
		if (mon->_mmode == MM_STONE) {
			MonStartKill(mnum, pnum);
			mon->_mmode = MM_STONE;
		} else {
			MonStartKill(mnum, pnum);
		}
	} else {
		if (resist) {
			PlayEffect(mnum, 1);
		} else if (mon->_mmode == MM_STONE) {
			if (mnum > MAX_PLRS - 1)
				MonStartHit(mnum, pnum, dam);
			mon->_mmode = MM_STONE;
		} else {
			if (!missiledata[mitype].mType && p->_pIFlags & ISPL_KNOCKBACK) {
				MonGetKnockback(mnum);
			}
			if (mnum > MAX_PLRS - 1)
				MonStartHit(mnum, pnum, dam);
		}
	}

	if (!mon->_msquelch) {
		mon->_msquelch = UCHAR_MAX;
		mon->_lastx = p->_px;
		mon->_lasty = p->_py;
	}
	return TRUE;
}

BOOL PlayerMHit(int pnum, int mnum, int dist, int mind, int maxd, int mitype, BOOLEAN shift, int earflag)
{
	PlayerStruct *p;
	int hit, hper, tac, dam, blk, blkper, resper;

	p = &plr[pnum];
	if (p->_pHitPoints >> 6 <= 0) {
		return FALSE;
	}

	if (p->_pInvincible) {
		return FALSE;
	}

	if (p->_pSpellFlags & 1 && !missiledata[mitype].mType) {
		return FALSE;
	}

	hit = random_(72, 100);
#ifdef _DEBUG
	if (debug_mode_dollar_sign || debug_mode_key_inverted_v)
		hit = 1000;
#endif
	if (!missiledata[mitype].mType) {
		tac = p->_pIAC + p->_pIBonusAC + p->_pDexterity / 5;
		if (mnum != -1) {
			hper = monster[mnum].mHit
			    + ((monster[mnum].mLevel - p->_pLevel) * 2)
			    + 30
			    - (dist << 1) - tac;
		} else {
			hper = 100 - (tac >> 1) - (dist << 1);
		}
	} else {
		if (mnum != -1) {
			hper = +40 - (p->_pLevel << 1) - (dist << 1) + (monster[mnum].mLevel << 1);
		} else {
			hper = 40;
		}
	}

	if (hper < 10)
		hper = 10;
	if (currlevel == 14 && hper < 20) {
		hper = 20;
	}
	if (currlevel == 15 && hper < 25) {
		hper = 25;
	}
	if (currlevel == 16 && hper < 30) {
		hper = 30;
	}

	if ((p->_pmode == PM_STAND || p->_pmode == PM_ATTACK) && p->_pBlockFlag) {
		blk = random_(73, 100);
	} else {
		blk = 100;
	}

	if (shift == TRUE)
		blk = 100;
	if (mitype == MIS_ACIDPUD)
		blk = 100;
	if (mnum != -1)
		blkper = p->_pBaseToBlk + (p->_pLevel << 1) - (monster[mnum].mLevel << 1) + p->_pDexterity;
	else
		blkper = p->_pBaseToBlk + p->_pDexterity;
	if (blkper < 0)
		blkper = 0;
	if (blkper > 100)
		blkper = 100;

	switch (missiledata[mitype].mResist) {
	case MISR_FIRE:
		resper = p->_pFireResist;
		break;
	case MISR_LIGHTNING:
		resper = p->_pLghtResist;
		break;
	case MISR_MAGIC:
	case MISR_ACID:
		resper = p->_pMagResist;
		break;
	default:
		resper = 0;
		break;
	}

	if (hit < hper) {
		if (mitype == MIS_BONESPIRIT) {
			dam = p->_pHitPoints / 3;
		} else {
			if (shift == FALSE) {

				dam = (mind << 6) + random_(75, (maxd - mind + 1) << 6);
				if (mnum == -1 && p->_pIFlags & ISPL_ABSHALFTRAP)
					dam >>= 1;
				dam += (p->_pIGetHit * 64);
			} else {
				dam = mind + random_(75, maxd - mind + 1);
				if (mnum == -1 && p->_pIFlags & ISPL_ABSHALFTRAP)
					dam >>= 1;
				dam += p->_pIGetHit;
			}

			if (dam < 64)
				dam = 64;
		}
		if (resper > 0) {

			dam = dam - dam * resper / 100;
			if (pnum == myplr) {
				p->_pHitPoints -= dam;
				p->_pHPBase -= dam;
			}
			if (p->_pHitPoints > p->_pMaxHP) {
				p->_pHitPoints = p->_pMaxHP;
				p->_pHPBase = p->_pMaxHPBase;
			}

			if (p->_pHitPoints >> 6 <= 0) {
				SyncPlrKill(pnum, earflag);
			} else {
				if (p->_pClass == PC_WARRIOR) {
					PlaySfxLoc(PS_WARR69, p->_px, p->_py);
				} else if (p->_pClass == PC_ROGUE) {
					PlaySfxLoc(PS_ROGUE69, p->_px, p->_py);
				} else if (p->_pClass == PC_SORCERER) {
					PlaySfxLoc(PS_MAGE69, p->_px, p->_py);
				}
				drawhpflag = TRUE;
			}
		} else {
			if (blk < blkper) {
				if (mnum != -1) {
					tac = GetDirection(p->_px, p->_py, monster[mnum]._mx, monster[mnum]._my);
				} else {
					tac = p->_pdir;
				}
				PlrStartBlock(pnum, tac);
			} else {
				if (pnum == myplr) {
					p->_pHitPoints -= dam;
					p->_pHPBase -= dam;
				}
				if (p->_pHitPoints > p->_pMaxHP) {
					p->_pHitPoints = p->_pMaxHP;
					p->_pHPBase = p->_pMaxHPBase;
				}
				if (p->_pHitPoints >> 6 <= 0) {
					SyncPlrKill(pnum, earflag);
				} else {
					StartPlrHit(pnum, dam, FALSE);
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL Plr2PlrMHit(int offp, int defp, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift)
{
	PlayerStruct *ops, *dps;
	int tac, resper, dam, blk, blkper, hper, hit;

	dps = &plr[defp];
	if (dps->_pInvincible) {
		return FALSE;
	}

	if (mitype == MIS_HBOLT) {
		return FALSE;
	}

	if (dps->_pSpellFlags & 1 && !missiledata[mitype].mType) {
		return FALSE;
	}

	switch (missiledata[mitype].mResist) {
	case MISR_FIRE:
		resper = dps->_pFireResist;
		break;
	case MISR_LIGHTNING:
		resper = dps->_pLghtResist;
		break;
	case MISR_MAGIC:
	case MISR_ACID:
		resper = dps->_pMagResist;
		break;
	default:
		resper = 0;
		break;
	}
	hper = random_(69, 100);
	ops = &plr[offp];
	if (!missiledata[mitype].mType) {
		hit = ops->_pIBonusToHit
		    + ops->_pLevel
		    - (dist * dist >> 1)
		    - dps->_pDexterity / 5
		    - dps->_pIBonusAC
		    - dps->_pIAC
		    + ops->_pDexterity + 50;
		if (ops->_pClass == PC_ROGUE)
			hit += 20;
		if (ops->_pClass == PC_WARRIOR)
			hit += 10;
	} else {
		hit = ops->_pMagic
		    - (dps->_pLevel << 1)
		    - dist
		    + 50;
		if (ops->_pClass == PC_SORCERER)
			hit += 20;
	}
	if (hit < 5)
		hit = 5;
	if (hit > 95)
		hit = 95;
	if (hper < hit) {
		if ((dps->_pmode == PM_STAND || dps->_pmode == PM_ATTACK) && dps->_pBlockFlag) {
			blkper = random_(73, 100);
		} else {
			blkper = 100;
		}
		if (shift == TRUE)
			blkper = 100;
		blk = dps->_pDexterity + dps->_pBaseToBlk + (dps->_pLevel << 1) - (ops->_pLevel << 1);

		if (blk < 0) {
			blk = 0;
		}
		if (blk > 100) {
			blk = 100;
		}

		if (mitype == MIS_BONESPIRIT) {
			dam = dps->_pHitPoints / 3;
		} else {
			dam = mindam + random_(70, maxdam - mindam + 1);
			if (!missiledata[mitype].mType)
				dam += ops->_pIBonusDamMod + ops->_pDamageMod + dam * ops->_pIBonusDam / 100;
			if (!shift)
				dam <<= 6;
		}
		if (missiledata[mitype].mType)
			dam >>= 1;
		if (resper > 0) {
			if (offp == myplr)
				NetSendCmdDamage(TRUE, defp, dam - resper * dam / 100);
			if (ops->_pClass == PC_WARRIOR) {
				tac = PS_WARR69;
			} else if (ops->_pClass == PC_ROGUE) {
				tac = PS_ROGUE69;
			} else if (ops->_pClass == PC_SORCERER) {
				tac = PS_MAGE69;
			} else {
				return TRUE;
			}
			PlaySfxLoc(tac, ops->_px, ops->_py);
		} else {
			if (blkper < blk) {
				PlrStartBlock(defp, GetDirection(dps->_px, dps->_py, ops->_px, ops->_py));
			} else {
				if (offp == myplr)
					NetSendCmdDamage(TRUE, defp, dam);
				StartPlrHit(defp, dam, FALSE);
			}
		}
		return TRUE;
	}
	return FALSE;
}

void CheckMissileCol(int mi, int mindam, int maxdam, BOOL shift, int mx, int my, BOOLEAN nodel)
{
	MissileStruct *mis;
	int oi;

	mis = &missile[mi];
	if (mis->_miAnimType != MFILE_FIREWAL && mis->_misource != -1) {
		if (mis->_micaster == 0) {
			if (dMonster[mx][my] > 0) {
				if (MonsterMHit(
				        mis->_misource,
				        dMonster[mx][my] - 1,
				        mindam,
				        maxdam,
				        mis->_midist,
				        mis->_mitype,
				        shift)) {
					if (!nodel)
						mis->_mirange = 0;
					mis->_miHitFlag = TRUE;
				}
			} else {
				if (dMonster[mx][my] < 0
				    && monster[-(dMonster[mx][my] + 1)]._mmode == MM_STONE
				    && MonsterMHit(
				           mis->_misource,
				           -(dMonster[mx][my] + 1),
				           mindam,
				           maxdam,
				           mis->_midist,
				           mis->_mitype,
				           shift)) {
					if (!nodel)
						mis->_mirange = 0;
					mis->_miHitFlag = TRUE;
				}
			}
			if (dPlayer[mx][my] > 0
			    && dPlayer[mx][my] - 1 != mis->_misource
			    && Plr2PlrMHit(
			           mis->_misource,
			           dPlayer[mx][my] - 1,
			           mindam,
			           maxdam,
			           mis->_midist,
			           mis->_mitype,
			           shift)) {
				if (!nodel)
					mis->_mirange = 0;
				mis->_miHitFlag = TRUE;
			}
		} else {
			if (monster[mis->_misource]._mFlags & MFLAG_TARGETS_MONSTER
			    && dMonster[mx][my] > 0
			    && monster[dMonster[mx][my] - 1]._mFlags & MFLAG_GOLEM
			    && MonsterTrapHit(dMonster[mx][my] - 1, mindam, maxdam, mis->_midist, mis->_mitype, shift)) {
				if (!nodel)
					mis->_mirange = 0;
				mis->_miHitFlag = TRUE;
			}
			if (dPlayer[mx][my] > 0
			    && PlayerMHit(
			           dPlayer[mx][my] - 1,
			           mis->_misource,
			           mis->_midist,
			           mindam,
			           maxdam,
			           mis->_mitype,
			           shift,
			           0)) {
				if (!nodel)
					mis->_mirange = 0;
				mis->_miHitFlag = TRUE;
			}
		}
	} else {
		if (dMonster[mx][my] > 0) {
			if (mis->_miAnimType == MFILE_FIREWAL) {
				if (MonsterMHit(
				        mis->_misource,
				        dMonster[mx][my] - 1,
				        mindam,
				        maxdam,
				        mis->_midist,
				        mis->_mitype,
				        shift)) {
					if (!nodel)
						mis->_mirange = 0;
					mis->_miHitFlag = TRUE;
				}
			} else if (MonsterTrapHit(dMonster[mx][my] - 1, mindam, maxdam, mis->_midist, mis->_mitype, shift)) {
				if (!nodel)
					mis->_mirange = 0;
				mis->_miHitFlag = TRUE;
			}
		}
		if (dPlayer[mx][my] > 0
		    && PlayerMHit(dPlayer[mx][my] - 1, -1, mis->_midist, mindam, maxdam, mis->_mitype, shift, mis->_miAnimType == MFILE_FIREWAL)) {
			if (!nodel)
				mis->_mirange = 0;
			mis->_miHitFlag = TRUE;
		}
	}
	if (dObject[mx][my]) {
		oi = dObject[mx][my] > 0 ? dObject[mx][my] - 1 : -(dObject[mx][my] + 1);
		if (!object[oi]._oMissFlag) {
			if (object[oi]._oBreak == 1)
				BreakObject(-1, oi);
			if (!nodel)
				mis->_mirange = 0;
			mis->_miHitFlag = FALSE;
		}
	}
	if (nMissileTable[dPiece[mx][my]]) {
		if (!nodel)
			mis->_mirange = 0;
		mis->_miHitFlag = FALSE;
	}
	if (mis->_mirange == 0 && missiledata[mis->_mitype].miSFX != -1)
		PlaySfxLoc(missiledata[mis->_mitype].miSFX, mis->_mix, mis->_miy);
}

void SetMissAnim(int mi, int animtype)
{
	MissileStruct *mis;
	MisFileData *mfd;

	mis = &missile[mi];
	int dir = mis->_mimfnum;
	if (animtype > MFILE_NULL) {
		animtype = MFILE_NULL;
	}
	mis->_miAnimType = animtype;
	mfd = &misfiledata[animtype];
	mis->_miAnimFlags = mfd->mFlags;
	mis->_miAnimData = mfd->mAnimData[dir];
	mis->_miAnimDelay = mfd->mAnimDelay[dir];
	mis->_miAnimLen = mfd->mAnimLen[dir];
	mis->_miAnimWidth = mfd->mAnimWidth[dir];
	mis->_miAnimWidth2 = mfd->mAnimWidth2[dir];
	mis->_miAnimCnt = 0;
	mis->_miAnimFrame = 1;
}

void SetMissDir(int mi, int dir)
{
	missile[mi]._mimfnum = dir;
	SetMissAnim(mi, missile[mi]._miAnimType);
}

void LoadMissileGFX(BYTE midx)
{
	char pszName[256];
	int i;
	BYTE *file;
	MisFileData *mfd;

	mfd = &misfiledata[midx];
	if (mfd->mFlags & MFLAG_ALLOW_SPECIAL) {
		sprintf(pszName, "Missiles\\%s.CL2", mfd->mName);
		file = LoadFileInMem(pszName, NULL);
		for (i = 0; i < mfd->mAnimFAmt; i++)
			mfd->mAnimData[i] = CelGetFrameStart(file, i);
	} else if (mfd->mAnimFAmt == 1) {
		sprintf(pszName, "Missiles\\%s.CL2", mfd->mName);
		if (!mfd->mAnimData[0])
			mfd->mAnimData[0] = LoadFileInMem(pszName, NULL);
	} else {
		for (i = 0; i < mfd->mAnimFAmt; i++) {
			sprintf(pszName, "Missiles\\%s%i.CL2", mfd->mName, i + 1);
			if (!mfd->mAnimData[i]) {
				file = LoadFileInMem(pszName, NULL);
				mfd->mAnimData[i] = file;
			}
		}
	}
}

void InitMissileGFX()
{
	int i;

	for (i = 0; misfiledata[i].mAnimFAmt; i++) {
		if (!(misfiledata[i].mFlags & MFLAG_HIDDEN))
			LoadMissileGFX(i);
	}
}

void FreeMissileGFX(int midx)
{
	int i;
	DWORD *p;

	if (misfiledata[midx].mFlags & MFLAG_ALLOW_SPECIAL) {
		if (misfiledata[midx].mAnimData[0]) {
			p = (DWORD *)misfiledata[midx].mAnimData[0];
			p -= misfiledata[midx].mAnimFAmt;
			MemFreeDbg(p);
			misfiledata[midx].mAnimData[0] = NULL;
		}
		return;
	}

	for (i = 0; i < misfiledata[midx].mAnimFAmt; i++) {
		if (misfiledata[midx].mAnimData[i]) {
			MemFreeDbg(misfiledata[midx].mAnimData[i]);
		}
	}
}

void FreeMissiles()
{
	int i;

	for (i = 0; misfiledata[i].mAnimFAmt; i++) {
		if (!(misfiledata[i].mFlags & MFLAG_HIDDEN))
			FreeMissileGFX(i);
	}
}

void FreeMissiles2()
{
	int i;

	for (i = 0; misfiledata[i].mAnimFAmt; i++) {
		if (misfiledata[i].mFlags & MFLAG_HIDDEN)
			FreeMissileGFX(i);
	}
}

void InitMissiles()
{
	MissileStruct *mis;
	PlayerStruct *p;
	int src, i, j;

#ifdef HELLFIRE
	AutoMapShowItems = FALSE;
#endif
	p = &plr[myplr];
	p->_pSpellFlags &= ~0x1;
	if (p->_pInfraFlag == TRUE) {
		for (i = 0; i < nummissiles; ++i) {
			mis = &missile[missileactive[i]];
			if (mis->_mitype == MIS_INFRA) {
				src = mis->_misource;
				if (src == myplr)
					CalcPlrItemVals(src, TRUE);
			}
		}
	}

#ifdef HELLFIRE
	if ((p->_pSpellFlags & 2) == 2 || (p->_pSpellFlags & 4) == 4) {
		p->_pSpellFlags &= ~0x2;
		p->_pSpellFlags &= ~0x4;
		for (i = 0; i < nummissiles; ++i) {
			mis = &missile[missileactive[i]];
			if (mis->_mitype == MIS_BLODBOIL) {
				if (mis->_misource == myplr) {
					int missingHP = p->_pMaxHP - p->_pHitPoints;
					CalcPlrItemVals(myplr, TRUE);
					p->_pHitPoints -= missingHP + mis->_miVar2;
					if (p->_pHitPoints < 64) {
						p->_pHitPoints = 64;
					}
				}
			}
		}
	}
#endif

	nummissiles = 0;
	for (i = 0; i < MAXMISSILES; i++) {
		missileavail[i] = i;
		missileactive[i] = 0;
	}
	numchains = 0;
	for (i = 0; i < MAXMISSILES; i++) {
		chain[i].idx = -1;
		chain[i]._mitype = 0;
		chain[i]._mirange = 0;
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			dFlags[i][j] &= ~BFLAG_MISSILE;
		}
	}
#ifdef HELLFIRE
	p->wReflection = FALSE;
#endif
}

#ifdef HELLFIRE
void AddHiveExplosion(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	AddMissile(80, 62, 80, 62, midir, MIS_HIVEEXP, micaster, id, dam, 0);
	AddMissile(80, 63, 80, 62, midir, MIS_HIVEEXP, micaster, id, dam, 0);
	AddMissile(81, 62, 80, 62, midir, MIS_HIVEEXP, micaster, id, dam, 0);
	AddMissile(81, 63, 80, 62, midir, MIS_HIVEEXP, micaster, id, dam, 0);
	missile[mi]._miDelFlag = TRUE;
}

void AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (id >= 0)
			UseMana(id, SPL_RUNEFIRE);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_HIVEEXP;
			missile[mi]._miDelFlag = FALSE;
			missile[mi]._mlid = AddLight(dx, dy, 8);
		} else {
			missile[mi]._miDelFlag = TRUE;
		}
	} else {
		missile[mi]._miDelFlag = TRUE;
	}
}

BOOLEAN missiles_found_target(int mi, int *x, int *y, int rad)
{
	BOOLEAN found;
	int i, j, cr, tx, ty;

	found = FALSE;

	if (rad > 19)
		rad = 19;

	for (i = 0; i < rad; i++) {
		if (found) {
			break;
		}
		cr = CrawlNum[i] + 1;
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
			tx = *x + CrawlTable[cr];
			ty = *y + CrawlTable[cr + 1];
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				if (!nSolidTable[dPiece[tx][ty]] && !dObject[tx][ty] && !dMissile[tx][ty]) {
					missile[mi]._mix = tx;
					missile[mi]._miy = ty;
					*x = tx;
					*y = ty;
					found = TRUE;
					break;
				}
			}
		}
	}
	return found;
}

void AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (id >= 0)
			UseMana(id, SPL_RUNELIGHT);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_LIGHTBALL;
			missile[mi]._miDelFlag = FALSE;
			missile[mi]._mlid = AddLight(dx, dy, 8);
		} else {
			missile[mi]._miDelFlag = TRUE;
		}
	} else {
		missile[mi]._miDelFlag = TRUE;
	}
}

void AddGreatLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (id >= 0)
			UseMana(id, SPL_RUNENOVA);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_NOVA;
			missile[mi]._miDelFlag = FALSE;
			missile[mi]._mlid = AddLight(dx, dy, 8);
		} else {
			missile[mi]._miDelFlag = TRUE;
		}
	} else {
		missile[mi]._miDelFlag = TRUE;
	}
}

void AddImmolationRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (id >= 0)
			UseMana(id, SPL_RUNEIMMOLAT);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_IMMOLATION;
			missile[mi]._miDelFlag = FALSE;
			missile[mi]._mlid = AddLight(dx, dy, 8);
		} else {
			missile[mi]._miDelFlag = TRUE;
		}
	} else {
		missile[mi]._miDelFlag = TRUE;
	}
}

void AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (id >= 0)
			UseMana(id, SPL_RUNESTONE);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_STONE;
			missile[mi]._miDelFlag = FALSE;
			missile[mi]._mlid = AddLight(dx, dy, 8);
		} else {
			missile[mi]._miDelFlag = TRUE;
		}
	} else {
		missile[mi]._miDelFlag = TRUE;
	}
}

void AddReflection(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int lvl;

	if (id >= 0) {
		if (missile[mi]._mispllvl)
			lvl = missile[mi]._mispllvl;
		else
			lvl = 2;
		plr[id].wReflection += lvl * plr[id]._pLevel;
		UseMana(id, SPL_REFLECT);
	}
	missile[mi]._mirange = 0;
	missile[mi]._miDelFlag = 0;
}

void AddBerserk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int i, j, cr, tx, ty, dm, r;

	if (id >= 0) {
		missile[mi]._misource = id;
		for (i = 0; i < 6; i++) {
			cr = CrawlNum[i] + 1;
			for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
				tx = dx + CrawlTable[cr];
				ty = dy + CrawlTable[cr + 1];
				if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
					dm = dMonster[tx][ty];
					dm = dm > 0 ? dm - 1 : -(dm + 1);
					if (dm > 3) {
						if (!monster[dm]._uniqtype && monster[dm]._mAi != AI_DIABLO) {
							if (monster[dm]._mmode != MM_FADEIN && monster[dm]._mmode != MM_FADEOUT) {
								if (!(monster[dm].mMagicRes & IMUNE_MAGIC)) {
									if ((!(monster[dm].mMagicRes & RESIST_MAGIC) || (monster[dm].mMagicRes & RESIST_MAGIC) == 1 && !random_(99, 2)) && monster[dm]._mmode != MM_CHARGE) {
										i = 6;
										double slvl = (double)GetSpellLevel(id, SPL_BERSERK);
										monster[dm]._mFlags |= MFLAG_UNUSED | MFLAG_GOLEM;
										monster[dm].mMinDamage = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)monster[dm].mMinDamage + slvl;
										monster[dm].mMaxDamage = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)monster[dm].mMaxDamage + slvl;
										monster[dm].mMinDamage2 = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)monster[dm].mMinDamage2 + slvl;
										monster[dm].mMaxDamage2 = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)monster[dm].mMaxDamage2 + slvl;
										if (currlevel < 17 || currlevel > 20)
											r = 3;
										else
											r = 9;
										monster[dm].mlid = AddLight(monster[dm]._mx, monster[dm]._my, r);
										UseMana(id, SPL_BERSERK);
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	missile[mi]._mirange = 0;
	missile[mi]._miDelFlag = TRUE;
}

void AddHorkSpawn(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	GetMissileVel(mi, sx, sy, dx, dy, 8);
	missile[mi]._mirange = 9;
	missile[mi]._miVar1 = midir;
	PutMissile(mi);
}

void AddJester(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int spell;

	spell = MIS_FIREBOLT;
	switch (random_(255, 10)) {
	case 0:
	case 1:
		spell = MIS_FIREBOLT;
		break;
	case 2:
		spell = MIS_FIREBALL;
		break;
	case 3:
		spell = MIS_FIREWALLC;
		break;
	case 4:
		spell = MIS_GUARDIAN;
		break;
	case 5:
		spell = MIS_CHAIN;
		break;
	case 6:
		spell = MIS_TOWN;
		UseMana(id, SPL_TOWN);
		break;
	case 7:
		spell = MIS_TELEPORT;
		break;
	case 8:
		spell = MIS_APOCA;
		break;
	case 9:
		spell = MIS_STONE;
		break;
	}
	AddMissile(sx, sy, dx, dy, midir, spell, missile[mi]._micaster, missile[mi]._misource, 0, missile[mi]._mispllvl);
	missile[mi]._miDelFlag = TRUE;
	missile[mi]._mirange = 0;
}

void AddStealPots(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int i, cr, j, tx, ty, si, ii, pnum;
	BOOL hasPlayedSFX;

	missile[mi]._misource = id;
	for (i = 0; i < 3; i++) {
		cr = CrawlNum[i] + 1;
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
			tx = sx + CrawlTable[cr];
			ty = sy + CrawlTable[cr + 1];
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				pnum = dPlayer[tx][ty];
				if (pnum) {
					pnum = pnum > 0 ? pnum - 1 : -(pnum + 1);

					hasPlayedSFX = FALSE;
					for (si = 0; si < MAXBELTITEMS; si++) {
						ii = -1;
						if (plr[pnum].SpdList[si]._itype == ITYPE_MISC) {
							if (random_(205, 2)) {
								switch (plr[pnum].SpdList[si]._iMiscId) {
								case IMISC_FULLHEAL:
									ii = ItemMiscIdIdx(IMISC_HEAL);
									break;
								case IMISC_HEAL:
								case IMISC_MANA:
									RemoveSpdBarItem(pnum, si);
									continue;
								case IMISC_FULLMANA:
									ii = ItemMiscIdIdx(IMISC_MANA);
									break;
								case IMISC_REJUV:
									if (random_(205, 2)) {
										ii = ItemMiscIdIdx(IMISC_MANA);
									} else {
										ii = ItemMiscIdIdx(IMISC_HEAL);
									}
									ii = ItemMiscIdIdx(IMISC_HEAL);
									break;
								case IMISC_FULLREJUV:
									switch (random_(205, 3)) {
									case 0:
										ii = ItemMiscIdIdx(IMISC_FULLMANA);
										break;
									case 1:
										ii = ItemMiscIdIdx(IMISC_FULLHEAL);
										break;
									default:
										ii = ItemMiscIdIdx(IMISC_REJUV);
										break;
									}
									break;
								}
							}
						}
						if (ii != -1) {
							SetPlrHandItem(&plr[pnum].HoldItem, ii);
							GetPlrHandSeed(&plr[pnum].HoldItem);
							plr[pnum].HoldItem._iStatFlag = 1;
							plr[pnum].SpdList[si] = plr[pnum].HoldItem;
						}
						if (!hasPlayedSFX) {
							PlaySfxLoc(IS_POPPOP2, tx, ty);
							hasPlayedSFX = TRUE;
						}
					}
					force_redraw = 255;
				}
			}
		}
	}
	missile[mi]._mirange = 0;
	missile[mi]._miDelFlag = TRUE;
}

void AddManaTrap(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int i, cr, j, tx, ty, pid;

	missile[mi]._misource = id;
	for (i = 0; i < 3; i++) {
		cr = CrawlNum[i] + 1;
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
			tx = sx + CrawlTable[cr];
			ty = sy + CrawlTable[cr + 1];
			if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
				pid = dPlayer[tx][ty];
				if (pid != 0) {
					if (pid > 0)
						pid = pid - 1;
					else
						pid = -(pid + 1);
					plr[pid]._pMana = 0;
					plr[pid]._pManaBase = plr[pid]._pMana + plr[pid]._pMaxManaBase - plr[pid]._pMaxMana;
					CalcPlrInv(pid, FALSE);
					drawmanaflag = TRUE;
					PlaySfxLoc(TSFX_COW7, tx, ty);
				}
			}
		}
	}
	missile[mi]._mirange = 0;
	missile[mi]._miDelFlag = TRUE;
}

void AddSpecArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	PlayerStruct *p;
	int av;

	av = 0;
	if (!micaster) {
		p = &plr[id];
		if (p->_pClass == PC_ROGUE)
			av += (p->_pLevel - 1) >> 2;
		else if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARD)
			av += (p->_pLevel - 1) >> 3;

		if (p->_pIFlags & ISPL_QUICKATTACK)
			av++;
		if (p->_pIFlags & ISPL_FASTATTACK)
			av += 2;
		if (p->_pIFlags & ISPL_FASTERATTACK)
			av += 4;
		if (p->_pIFlags & ISPL_FASTESTATTACK)
			av += 8;
	}
	missile[mi]._mirange = 1;
	missile[mi]._miVar1 = dx;
	missile[mi]._miVar2 = dy;
	missile[mi]._miVar3 = av;
}

void AddWarp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int tx, ty, fx, fy, i, dist;
	TriggerStruct *trg;

	dist = INT_MAX;
	if (id >= 0) {
		sx = plr[id]._px;
		sy = plr[id]._py;
	}
	tx = sx;
	ty = sy;

	for (i = 0; i < numtrigs && i < MAXTRIGGERS; i++) {
		trg = &trigs[i];
		if (trg->_tmsg == 1032 || trg->_tmsg == 1027 || trg->_tmsg == 1026 || trg->_tmsg == 1028) {
			if ((leveltype == 1 || leveltype == 2) && (trg->_tmsg == 1026 || trg->_tmsg == 1027 || trg->_tmsg == 1028)) {
				fx = trg->_tx;
				fy = trg->_ty + 1;
			} else {
				fx = trg->_tx + 1;
				fy = trg->_ty;
			}
			int dify = (sy - fy);
			int difx = (sx - fx);
			int dif = dify * dify + difx * difx;
			if (dif < dist) {
				dist = dif;
				tx = fx;
				ty = fy;
			}
		}
	}
	missile[mi]._mirange = 2;
	missile[mi]._miVar1 = 0;
	missile[mi]._mix = tx;
	missile[mi]._miy = ty;
	if (!micaster)
		UseMana(id, SPL_WARP);
}

void AddLightWall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_midam = dam;
	mis->_miAnimFrame = random_(63, 8) + 1;
	mis->_mirange = 255 * (mis->_mispllvl + 1);
	if (id < 0) {
		mis->_miVar1 = sx;
		mis->_miVar2 = sy;
	} else {
		mis->_miVar1 = plr[id]._px;
		mis->_miVar2 = plr[id]._py;
	}
}

void AddHivectrl(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i, dmg;

	mis = &missile[mi];
	if (!micaster || micaster == 2) {
		mis->_midam = 2 * (plr[id]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
		for (i = mis->_mispllvl; i > 0; i--) {
			mis->_midam += mis->_midam >> 3;
		}

		dmg = mis->_midam;
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix - 1, mis->_miy - 1, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix, mis->_miy - 1, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix + 1, mis->_miy - 1, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix - 1, mis->_miy, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix, mis->_miy, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix + 1, mis->_miy, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix - 1, mis->_miy + 1, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix, mis->_miy + 1, 1);
		CheckMissileCol(mi, dmg, dmg, 0, mis->_mix + 1, mis->_miy + 1, 1);
	}
	mis->_mlid = AddLight(sx, sy, 8);
	SetMissDir(mi, 0);
	mis->_miDelFlag = 0;
	mis->_mirange = mis->_miAnimLen - 1;
}

void AddImmolation(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	if (!micaster) {
		mis->_midam = 2 * (plr[id]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
		for (i = mis->_mispllvl; i > 0; i--) {
			mis->_midam += mis->_midam >> 3;
		}
		i = 2 * mis->_mispllvl + 16;
		if (i > 50)
			i = 50;
		UseMana(id, SPL_FIREBALL);
	} else {
		i = 16;
	}
	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_mirange = 256;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar3 = 0;
	mis->_miVar4 = sx;
	mis->_miVar5 = sy;
	mis->_miVar6 = 2;
	mis->_miVar7 = 2;
	mis->_mlid = AddLight(sx, sy, 8);
}

void AddFireNova(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	if (!micaster) {
		i = mis->_mispllvl + 16;
		if (i > 50) {
			i = 50;
		}
	} else {
		i = 16;
	}
	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_mirange = 256;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar3 = 0;
	mis->_miVar4 = sx;
	mis->_miVar5 = sy;
	mis->_mlid = AddLight(sx, sy, 8);
}

void AddLightArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 32);
	mis = &missile[mi];
	mis->_miAnimFrame = random_(52, 8) + 1;
	mis->_mirange = 255;
	if (id < 0) {
		mis->_miVar1 = sx;
		mis->_miVar2 = sy;
	} else {
		mis->_miVar1 = plr[id]._px;
		mis->_miVar2 = plr[id]._py;
	}
	mis->_midam <<= 6;
}

void AddFlashfr(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
}

void AddFlashbk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int lvl;

	if (!micaster && id != -1) {
		missile[mi]._midam = 0;
		if (2 * (id > 0))
			lvl = plr[id]._pLevel;
		else
			lvl = 1;
		missile[mi]._mirange = lvl + 10 * missile[mi]._mispllvl + 245;
	}
}

void AddManaRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	PlayerStruct *p;
	int i, ManaAmount;

	p = &plr[id];
	ManaAmount = (random_(57, 10) + 1) << 6;
	for (i = 0; i < p->_pLevel; i++) {
		ManaAmount += (random_(57, 4) + 1) << 6;
	}
	for (i = 0; i < missile[mi]._mispllvl; i++) {
		ManaAmount += (random_(57, 6) + 1) << 6;
	}
	if (p->_pClass == PC_SORCERER)
		ManaAmount <<= 1;
	if (p->_pClass == PC_ROGUE || p->_pClass == PC_BARD)
		ManaAmount += ManaAmount >> 1;
	p->_pMana += ManaAmount;
	if (p->_pMana > p->_pMaxMana)
		p->_pMana = p->_pMaxMana;
	p->_pManaBase += ManaAmount;
	if (p->_pManaBase > p->_pMaxManaBase)
		p->_pManaBase = p->_pMaxManaBase;
	UseMana(id, SPL_MANA);
	missile[mi]._miDelFlag = TRUE;
	drawmanaflag = TRUE;
}

void AddMagiRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	plr[id]._pMana = plr[id]._pMaxMana;
	plr[id]._pManaBase = plr[id]._pMaxManaBase;
	UseMana(id, SPL_MAGI);
	missile[mi]._miDelFlag = TRUE;
	drawmanaflag = TRUE;
}

void AddElementalRing(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	if (!micaster)
		UseMana(id, SPL_FIRERING);
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miDelFlag = FALSE;
	mis->_miVar3 = 0;
	mis->_miVar4 = 0;
	mis->_miVar5 = 0;
	mis->_miVar6 = 0;
	mis->_miVar7 = 0;
	mis->_miVar8 = 0;
	mis->_mirange = 7;
}

void AddSearch(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int lvl, i, mx, r1, r2;
	MissileStruct *mis, *tmis;

	mis = &missile[mi];
	mis->_miDelFlag = FALSE;
	mis->_miVar1 = id;
	mis->_miVar2 = 0;
	mis->_miVar3 = 0;
	mis->_miVar4 = 0;
	mis->_miVar5 = 0;
	mis->_miVar6 = 0;
	mis->_miVar7 = 0;
	mis->_miVar8 = 0;
	AutoMapShowItems = TRUE;
	if (2 * (id > 0))
		lvl = plr[id]._pLevel;
	else
		lvl = 1;
	mis->_mirange = lvl + 10 * mis->_mispllvl + 245;
	if (!micaster)
		UseMana(id, SPL_SEARCH);

	for (i = 0; i < nummissiles; i++) {
		tmis = &missile[missileactive[i]];
		if (tmis->_mitype == 85 && mis != tmis && tmis->_miVar1 == id) {
			r1 = mis->_mirange;
			r2 = tmis->_mirange;
			if (r2 < INT_MAX - r1)
				tmis->_mirange = r1 + r2;
			mis->_miDelFlag = TRUE;
			break;
		}
	}
}

void AddCboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (!micaster) {
		if (id == myplr) {
			mis->_mirnd = random_(63, 15) + 1;
		} else {
			mis->_mirnd = random_(63, 15) + 1;
		}
	} else {
		mis->_mirnd = random_(63, 15) + 1;
		mis->_midam = 15;
	}
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis->_miAnimFrame = random_(63, 8) + 1;
	mis->_mlid = AddLight(sx, sy, 5);
	GetMissileVel(mi, sx, sy, dx, dy, 8);
	mis->_miVar1 = 5;
	mis->_miVar2 = midir;
	mis->_miVar3 = 0;
	mis->_mirange = 256;
}

void AddHboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int i;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}

	if (id != -1) {
		i = 2 * missile[mi]._mispllvl + 16;
		if (i >= 63) {
			i = 63;
		}
	} else {
		i = 16;
	}

	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	missile[mi]._mirange = 256;
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	missile[mi]._mlid = AddLight(sx, sy, 8);
}
#endif

void AddLArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (!micaster) {
#ifdef HELLFIRE
		int av = 32;

		if (plr[id]._pClass == PC_ROGUE)
			av += (plr[id]._pLevel) >> 2;
		else if (plr[id]._pClass == PC_WARRIOR || plr[id]._pClass == PC_BARD)
			av += (plr[id]._pLevel) >> 3;

		if (plr[id]._pIFlags & ISPL_QUICKATTACK)
			av++;
		if (plr[id]._pIFlags & ISPL_FASTATTACK)
			av += 2;
		if (plr[id]._pIFlags & ISPL_FASTERATTACK)
			av += 4;
		if (plr[id]._pIFlags & ISPL_FASTESTATTACK)
			av += 8;
		GetMissileVel(mi, sx, sy, dx, dy, av);
#else
		if (plr[id]._pClass == PC_ROGUE)
			GetMissileVel(mi, sx, sy, dx, dy, (plr[id]._pLevel >> 2) + 31);
		else if (plr[id]._pClass == PC_WARRIOR)
			GetMissileVel(mi, sx, sy, dx, dy, (plr[id]._pLevel >> 3) + 31);
		else
			GetMissileVel(mi, sx, sy, dx, dy, 32);
#endif
	} else
		GetMissileVel(mi, sx, sy, dx, dy, 32);

	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	missile[mi]._mirange = 256;
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	missile[mi]._mlid = AddLight(sx, sy, 5);
}

void AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int av;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (!micaster) {
		av = 32;
		if (plr[id]._pIFlags & ISPL_RNDARROWVEL) {
			av = random_(64, 32) + 16;
		}
#ifdef HELLFIRE
		if (plr[id]._pClass == PC_ROGUE)
			av += (plr[id]._pLevel - 1) >> 2;
		else if (plr[id]._pClass == PC_WARRIOR || plr[id]._pClass == PC_BARD)
			av += (plr[id]._pLevel - 1) >> 3;

		if (plr[id]._pIFlags & ISPL_QUICKATTACK)
			av++;
		if (plr[id]._pIFlags & ISPL_FASTATTACK)
			av += 2;
		if (plr[id]._pIFlags & ISPL_FASTERATTACK)
			av += 4;
		if (plr[id]._pIFlags & ISPL_FASTESTATTACK)
			av += 8;
#else
		if (plr[id]._pClass == PC_ROGUE)
			av += (plr[id]._pLevel - 1) >> 2;
		if (plr[id]._pClass == PC_WARRIOR)
			av += (plr[id]._pLevel - 1) >> 3;
#endif
		GetMissileVel(mi, sx, sy, dx, dy, av);
	} else {
		GetMissileVel(mi, sx, sy, dx, dy, 32);
	}
	missile[mi]._miAnimFrame = GetDirection16(sx, sy, dx, dy) + 1;
	missile[mi]._mirange = 256;
}

void GetVileMissPos(int mi, int dx, int dy)
{
	int xx, yy, k, j, i;

	for (k = 1; k < 50; k++) {
		for (j = -k; j <= k; j++) {
			yy = j + dy;
			for (i = -k; i <= k; i++) {
				xx = i + dx;
				if (PosOkPlayer(myplr, xx, yy)) {
					missile[mi]._mix = xx;
					missile[mi]._miy = yy;
					return;
				}
			}
		}
	}
	missile[mi]._mix = dx;
	missile[mi]._miy = dy;
}

void AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int pn, r1, r2, nTries;

	nTries = 0;
	do {
		nTries++;
		if (nTries > 500) {
			r1 = 0;
			r2 = 0;
			break;
		}
		r1 = random_(58, 3) + 4;
		r2 = random_(58, 3) + 4;
		if (random_(58, 2) == 1)
			r1 = -r1;
		if (random_(58, 2) == 1)
			r2 = -r2;

#ifdef HELLFIRE
		r1 += sx;
		r2 += sy;
		if (r1 <= MAXDUNX && r1 >= 0 && r2 <= MAXDUNY && r2 >= 0) { ///BUGFIX: < MAXDUNX / < MAXDUNY
			pn = dPiece[r1][r2];
		}
	} while (nSolidTable[pn] || dObject[r1][r2] || dMonster[r1][r2]);
#else
		pn = dPiece[r1 + sx][sy + r2];
	} while (nSolidTable[pn] || dObject[r1 + sx][sy + r2] || dMonster[r1 + sx][sy + r2]);
#endif

	missile[mi]._mirange = 2;
	missile[mi]._miVar1 = 0;
	if (!setlevel || setlvlnum != SL_VILEBETRAYER) {
#ifdef HELLFIRE
		missile[mi]._mix = r1;
		missile[mi]._miy = r2;
#else
		missile[mi]._mix = sx + r1;
		missile[mi]._miy = sy + r2;
#endif
		if (!micaster)
			UseMana(id, SPL_RNDTELEPORT);
	} else {
		pn = dObject[dx][dy] - 1;
		// BUGFIX: should only run magic circle check if dObject[dx][dy] is non-zero.
		if (object[pn]._otype == OBJ_MCIRCLE1 || object[pn]._otype == OBJ_MCIRCLE2) {
			missile[mi]._mix = dx;
			missile[mi]._miy = dy;
			if (!PosOkPlayer(myplr, dx, dy))
				GetVileMissPos(mi, dx, dy);
		}
	}
}

void AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int i, mx, sp;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (!micaster) {
		for (i = 0; i < nummissiles; i++) {
			mx = missileactive[i];
			if (missile[mx]._mitype == MIS_GUARDIAN && missile[mx]._misource == id && missile[mx]._miVar3 == mi)
				break;
		}
		if (i == nummissiles)
			UseMana(id, SPL_FIREBOLT);
		if (id != -1) {
			sp = 2 * missile[mi]._mispllvl + 16;
			if (sp >= 63)
				sp = 63;
		} else {
			sp = 16;
		}
	} else {
		sp = 26;
	}
	GetMissileVel(mi, sx, sy, dx, dy, sp);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	missile[mi]._mirange = 256;
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	missile[mi]._mlid = AddLight(sx, sy, 8);
}

void AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_mitxoff += 3 * mis->_mixvel;
	mis->_mityoff += 3 * mis->_miyvel;
	GetMissilePos(mi);
#ifdef HELLFIRE
	if (mis->_mixvel & 0xFFFF0000 || mis->_miyvel & 0xFFFF0000)
		mis->_mirange = 256;
	else
		mis->_mirange = 1;
#else
	mis->_mirange = 256;
#endif
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_mlid = AddLight(sx, sy, 8);
}

void miss_null_33(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	missile[mi]._mirange = 256;
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	PutMissile(mi);
}

void AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i, cr, j, tx, ty;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	for (i = 0; i < 6; i++) {
		cr = CrawlNum[i] + 1;
		for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
			tx = dx + CrawlTable[cr];
			ty = dy + CrawlTable[cr + 1];
			if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
				if (!(nSolidTable[dPiece[tx][ty]] | dMonster[tx][ty] | dObject[tx][ty] | dPlayer[tx][ty])) {
					mis->_mix = tx;
					mis->_miy = ty;
					mis->_misx = tx;
					mis->_misy = ty;
					mis->_miDelFlag = FALSE;
					i = 6;
					break;
				}
			}
		}
	}

	if (!mis->_miDelFlag) {
		UseMana(id, SPL_TELEPORT);
		mis->_mirange = 2;
	}
}

void AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_midam = dam;
	mis->_miAnimFrame = random_(63, 8) + 1;
	mis->_mirange = 255;
	if (id < 0) {
		mis->_miVar1 = sx;
		mis->_miVar2 = sy;
	} else {
		mis->_miVar1 = plr[id]._px;
		mis->_miVar2 = plr[id]._py;
	}
}

void AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	mis = &missile[mi];
	mis->_midam = (random_(53, 10) + random_(53, 10) + plr[id]._pLevel + 2) << 4;
	mis->_midam >>= 1;
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis->_mirange = 10;
	i = mis->_mispllvl;
	if (i > 0)
		mis->_mirange = 10 * (i + 1);
	mis->_mirange += (plr[id]._pISplDur * mis->_mirange) >> 7;
	mis->_mirange <<= 4;
	mis->_miVar1 = mis->_mirange - mis->_miAnimLen;
	mis->_miVar2 = 0;
}

void AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	if (!micaster) {
		mis->_midam = 2 * (plr[id]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
		for (i = mis->_mispllvl; i > 0; i--) {
			mis->_midam += mis->_midam >> 3;
		}
		i = 2 * mis->_mispllvl + 16;
		if (i > 50)
			i = 50;
		UseMana(id, SPL_FIREBALL);
	} else {
		i = 16;
	}
	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_mirange = 256;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar3 = 0;
	mis->_miVar4 = sx;
	mis->_miVar5 = sy;
	mis->_mlid = AddLight(sx, sy, 8);
}

void AddLightctrl(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (!dam && !micaster)
		UseMana(id, SPL_LIGHTNING);
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	GetMissileVel(mi, sx, sy, dx, dy, 32);
	missile[mi]._miAnimFrame = random_(52, 8) + 1;
	missile[mi]._mirange = 256;
}

void AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_misx = dx;
	mis->_misy = dy;
	if (midir >= 0) {
		mis->_mixoff = missile[midir]._mixoff;
		mis->_miyoff = missile[midir]._miyoff;
		mis->_mitxoff = missile[midir]._mitxoff;
		mis->_mityoff = missile[midir]._mityoff;
	}
	mis->_miAnimFrame = random_(52, 8) + 1;

	if (midir < 0 || micaster == 1 || id == -1) {
		if (midir < 0 || id == -1)
			mis->_mirange = 8;
		else
			mis->_mirange = 10;
	} else {
		mis->_mirange = (mis->_mispllvl >> 1) + 6;
	}
	mis->_mlid = AddLight(mis->_mix, mis->_miy, 4);
}

void AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis, *bmis;
	CMonster *mon;

	if (micaster && id > 0) {
		mon = monster[id].MType;
#ifdef HELLFIRE
		switch (mon->mtype) {
		case MT_SUCCUBUS:
			SetMissAnim(mi, MFILE_FLAREEXP);
			break;
		case MT_SNOWWICH:
			SetMissAnim(mi, MFILE_SCBSEXPB);
			break;
		case MT_HLSPWN:
			SetMissAnim(mi, MFILE_SCBSEXPD);
			break;
		case MT_SOLBRNR:
			SetMissAnim(mi, MFILE_SCBSEXPC);
			break;
		}
#else
		if (mon->mtype == MT_SUCCUBUS)
			SetMissAnim(mi, MFILE_FLAREEXP);
		if (mon->mtype == MT_SNOWWICH)
			SetMissAnim(mi, MFILE_SCBSEXPB);
		if (mon->mtype == MT_HLSPWN)
			SetMissAnim(mi, MFILE_SCBSEXPD);
		if (mon->mtype == MT_SOLBRNR)
			SetMissAnim(mi, MFILE_SCBSEXPC);
#endif
	}

	mis = &missile[mi];
	bmis = &missile[dx];
	mis->_mix = bmis->_mix;
	mis->_miy = bmis->_miy;
	mis->_misx = bmis->_misx;
	mis->_misy = bmis->_misy;
	mis->_mixoff = bmis->_mixoff;
	mis->_miyoff = bmis->_miyoff;
	mis->_mitxoff = bmis->_mitxoff;
	mis->_mityoff = bmis->_mityoff;
	mis->_mixvel = 0;
	mis->_miyvel = 0;
	mis->_mirange = mis->_miAnimLen;
	mis->_miVar1 = 0;
}

void AddWeapexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mix = sx;
	mis->_miy = sy;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_mixvel = 0;
	mis->_miyvel = 0;
	mis->_miVar1 = 0;
	mis->_miVar2 = dx;
	mis->_mimfnum = 0;
	if (dx == 1)
		SetMissAnim(mi, MFILE_MAGBLOS);
	else
		SetMissAnim(mi, MFILE_MINILTNG);
	mis->_mirange = mis->_miAnimLen - 1;
}

BOOL CheckIfTrig(int x, int y)
{
	int i;

	for (i = 0; i < numtrigs; i++) {
		if ((x == trigs[i]._tx && y == trigs[i]._ty) || (abs(trigs[i]._tx - x) < 2 && abs(trigs[i]._ty - y) < 2))
			return TRUE;
	}
	return FALSE;
}

void AddTown(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i, j, cr, mx, tx, ty, dp;

	mis = &missile[mi];
	if (currlevel != 0) {
		mis->_miDelFlag = TRUE;
		for (i = 0; i < 6; i++) {
			cr = CrawlNum[i] + 1;
#ifdef HELLFIRE
			for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#else
			for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#endif
				tx = dx + CrawlTable[cr];
				ty = dy + CrawlTable[cr + 1];
				if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
					dp = dPiece[tx][ty];
					if (!(dMissile[tx][ty] | nSolidTable[dp] | nMissileTable[dp] | dObject[tx][ty] | dPlayer[tx][ty])) {
						if (!CheckIfTrig(tx, ty)) {
							mis->_mix = tx;
							mis->_miy = ty;
							mis->_misx = tx;
							mis->_misy = ty;
							mis->_miDelFlag = FALSE;
							i = 6;
							break;
						}
					}
				}
			}
		}
	} else {
		tx = dx;
		ty = dy;
		mis->_mix = tx;
		mis->_miy = ty;
		mis->_misx = tx;
		mis->_misy = ty;
		mis->_miDelFlag = FALSE;
	}
	mis->_mirange = 100;
	mis->_miVar1 = mis->_mirange - mis->_miAnimLen;
	mis->_miVar2 = 0;
	for (i = 0; i < nummissiles; i++) {
		mx = missileactive[i];
		if (missile[mx]._mitype == MIS_TOWN && mx != mi && missile[mx]._misource == id)
			missile[mx]._mirange = 0;
	}
	PutMissile(mi);
	if (id == myplr && !mis->_miDelFlag && currlevel != 0) {
		if (!setlevel) {
			NetSendCmdLocParam3(TRUE, CMD_ACTIVATEPORTAL, tx, ty, currlevel, leveltype, 0);
		} else {
			NetSendCmdLocParam3(TRUE, CMD_ACTIVATEPORTAL, tx, ty, setlvlnum, leveltype, 1);
		}
	}
}

void AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	mis = &missile[mi];
	if (!micaster) {
		if (id != -1) {
			mis->_midam = 0;
			for (i = 0; i <= plr[id]._pLevel; i++) {
				mis->_midam += random_(55, 20) + 1;
			}
			for (i = mis->_mispllvl; i > 0; i--) {
				mis->_midam += mis->_midam >> 3;
			}
			mis->_midam += mis->_midam >> 1;
			UseMana(id, SPL_FLASH);
		} else {
			mis->_midam = currlevel >> 1;
		}
	} else {
		mis->_midam = monster[id].mLevel << 1;
	}
	mis->_mirange = 19;
}

void AddFlash2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	mis = &missile[mi];
	if (!micaster) {
		if (id != -1) {
			mis->_midam = 0;
			for (i = 0; i <= plr[id]._pLevel; i++) {
				mis->_midam += random_(56, 2) + 1;
			}
			for (i = mis->_mispllvl; i > 0; i--) {
				mis->_midam += mis->_midam >> 3;
			}
			mis->_midam += mis->_midam >> 1;
		} else {
			mis->_midam = currlevel >> 1;
		}
	}
	mis->_miPreFlag = TRUE;
	mis->_mirange = 19;
}

void AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mirange = 48 * plr[id]._pLevel;
	mis->_miVar1 = plr[id]._pHitPoints;
	mis->_miVar2 = plr[id]._pHPBase;
	mis->_miVar8 = -1;
	if (!micaster)
		UseMana(id, SPL_MANASHIELD);
	if (id == myplr)
		NetSendCmd(TRUE, CMD_SETSHIELD);
	plr[id].pManaShield = TRUE;
}

void AddFiremove(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_midam = random_(59, 10) + plr[id]._pLevel + 1;
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis->_mirange = 255;
	mis->_miVar1 = 0;
	mis->_miVar2 = 0;
	mis->_mix++;
	mis->_miy++;
	mis->_miyoff -= 32;
}

void AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i, cr, pn, j, tx, ty;

	mis = &missile[mi];
	mis->_midam = random_(62, 10) + (plr[id]._pLevel >> 1) + 1;
	for (i = mis->_mispllvl; i > 0; i--) {
		mis->_midam += mis->_midam >> 3;
	}

	mis->_miDelFlag = TRUE;
	for (i = 0; i < 6; i++) {
		cr = CrawlNum[i] + 1;
#ifdef HELLFIRE
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#else
		for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#endif
			tx = dx + CrawlTable[cr];
			ty = dy + CrawlTable[cr + 1];
			pn = dPiece[tx][ty];
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				if (LineClear(sx, sy, tx, ty)) {
					if (!(dMonster[tx][ty] | nSolidTable[pn] | nMissileTable[pn] | dObject[tx][ty] | dMissile[tx][ty])) {
						mis->_mix = tx;
						mis->_miy = ty;
						mis->_misx = tx;
						mis->_misy = ty;
						mis->_miDelFlag = FALSE;
						UseMana(id, SPL_GUARDIAN);
						i = 6;
						break;
					}
				}
			}
		}
	}

	if (mis->_miDelFlag != TRUE) {
		mis->_misource = id;
		mis->_mlid = AddLight(mis->_mix, mis->_miy, 1);
		mis->_mirange = mis->_mispllvl + (plr[id]._pLevel >> 1);
		mis->_mirange += (mis->_mirange * plr[id]._pISplDur) >> 7;

		if (mis->_mirange > 30)
			mis->_mirange = 30;
		mis->_mirange <<= 4;
		if (mis->_mirange < 30)
			mis->_mirange = 30;

		mis->_miVar1 = mis->_mirange - mis->_miAnimLen;
		mis->_miVar2 = 0;
		mis->_miVar3 = 1;
	}
}

void AddChain(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miVar1 = dx;
	missile[mi]._miVar2 = dy;
	missile[mi]._mirange = 1;
	UseMana(id, SPL_CHAIN);
}

void miss_null_11(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	SetMissDir(mi, dx);
	missile[mi]._midam = 0;
	missile[mi]._miLightFlag = TRUE;
	missile[mi]._mirange = 250;
}

void miss_null_12(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (dx > 3)
		dx = 2;
	SetMissDir(mi, dx);
	missile[mi]._midam = 0;
	missile[mi]._miLightFlag = TRUE;
	missile[mi]._mirange = 250;
}

void miss_null_13(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (dx > 3)
		dx = 2;
	SetMissDir(mi, dx);
	missile[mi]._midam = 0;
	missile[mi]._miLightFlag = TRUE;
	missile[mi]._mirange = missile[mi]._miAnimLen;
}

void AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	AnimStruct *anim;

	mon = &monster[id];
	if (mon->MType->mtype < MT_HORNED || mon->MType->mtype > MT_OBLORD) {
		if (mon->MType->mtype < MT_NSNAKE || mon->MType->mtype > MT_GSNAKE) {
			anim = &mon->MType->Anims[MA_WALK];
		} else {
			anim = &mon->MType->Anims[MA_ATTACK];
		}
	} else {
		anim = &mon->MType->Anims[MA_SPECIAL];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 18);
	mis = &missile[mi];
	mis->_mimfnum = midir;
	mis->_miAnimFlags = 0;
	mis->_miAnimData = anim->Data[midir];
	mis->_miAnimDelay = anim->Rate;
	mis->_miAnimLen = anim->Frames;
	mis->_miAnimWidth = mon->MType->width;
	mis->_miAnimWidth2 = mon->MType->width2;
	mis->_miAnimAdd = 1;
	if (mon->MType->mtype >= MT_NSNAKE && mon->MType->mtype <= MT_GSNAKE)
		mis->_miAnimFrame = 7;
	mis->_miVar1 = 0;
	mis->_miVar2 = 0;
	mis->_miLightFlag = TRUE;
	if (mon->_uniqtype != 0) {
		mis->_miUniqTrans = mon->_uniqtrans + 1;
		mis->_mlid = mon->mlid;
	}
	mis->_mirange = 256;
	PutMissile(mi);
}

void miss_null_32(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	AnimStruct *anim;
	MonsterStruct *mon;

	anim = &monster[id].MType->Anims[MA_WALK];
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mon = &monster[id];
	mis = &missile[mi];
	mis->_mimfnum = midir;
	mis->_miAnimFlags = 0;
	mis->_miAnimData = anim->Data[midir];
	mis->_miAnimDelay = anim->Rate;
	mis->_miAnimLen = anim->Frames;
	mis->_miAnimWidth = mon->MType->width;
	mis->_miAnimWidth2 = mon->MType->width2;
	mis->_miAnimAdd = 1;
	mis->_miVar1 = 0;
	mis->_miVar2 = 0;
	mis->_miLightFlag = TRUE;
	if (mon->_uniqtype != 0)
		mis->_miUniqTrans = mon->_uniqtrans + 1;
	dMonster[mon->_mx][mon->_my] = 0;
	mis->_mirange = 256;
	PutMissile(mi);
}

void AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	missile[mi]._mirange = 256;
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	missile[mi]._mlid = AddLight(sx, sy, 8);
	if (!micaster) {
		UseMana(id, SPL_FLARE);
		plr[id]._pHitPoints -= 320;
		plr[id]._pHPBase -= 320;
		drawhpflag = TRUE;
		if (plr[id]._pHitPoints <= 0)
			SyncPlrKill(id, 0);
	} else {
		if (id > 0) {
			if (monster[id].MType->mtype == MT_SUCCUBUS)
				SetMissAnim(mi, MFILE_FLARE);
			if (monster[id].MType->mtype == MT_SNOWWICH)
				SetMissAnim(mi, MFILE_SCUBMISB);
			if (monster[id].MType->mtype == MT_HLSPWN)
				SetMissAnim(mi, MFILE_SCUBMISD);
			if (monster[id].MType->mtype == MT_SOLBRNR)
				SetMissAnim(mi, MFILE_SCUBMISC);
		}
	}
#ifdef HELLFIRE
	if (misfiledata[missile[mi]._miAnimType].mAnimFAmt == 16) {
		SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	}
#endif
}

void AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
#ifdef HELLFIRE
	if (mis->_mixvel & 0xFFFF0000 || mis->_miyvel & 0xFFFF0000)
		mis->_mirange = 5 * (monster[id]._mint + 4);
	else
		mis->_mirange = 1;
#else
	mis->_mirange = 5 * (monster[id]._mint + 4);
#endif
	mis->_mlid = -1;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	PutMissile(mi);
}

void miss_null_1D(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_midam = dam;
	mis->_mixvel = 0;
	mis->_miyvel = 0;
	mis->_mirange = 50;
	mis->_miVar1 = mis->_mirange - mis->_miAnimLen;
	mis->_miVar2 = 0;
}

void AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int mnum;

	mis = &missile[mi];
	mis->_mixvel = 0;
	mis->_miyvel = 0;
	mis->_mixoff = 0;
	mis->_miyoff = 0;
	mis->_miLightFlag = TRUE;
	mnum = mis->_misource;
	mis->_mirange = random_(50, 15) + 40 * (monster[mnum]._mint + 1);
	mis->_miPreFlag = TRUE;
}

void AddStone(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i, j, cr, tx, ty, mid;

	mis = &missile[mi];
	mis->_misource = id;
	for (i = 0; i < 6; i++) {
		cr = CrawlNum[i] + 1;
#ifdef HELLFIRE
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#else
		for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#endif
			tx = dx + CrawlTable[cr];
			ty = dy + CrawlTable[cr + 1];
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				mid = dMonster[tx][ty];
				mid = mid > 0 ? mid - 1 : -1 - mid;
#ifdef HELLFIRE
				if (mid > MAX_PLRS - 1 && monster[mid]._mAi != AI_DIABLO && monster[mid].MType->mtype != MT_NAKRUL) {
#else
				if (mid > MAX_PLRS - 1 && monster[mid]._mAi != AI_DIABLO) {
#endif
					if (monster[mid]._mmode != MM_FADEIN && monster[mid]._mmode != MM_FADEOUT && monster[mid]._mmode != MM_CHARGE) {
						j = -99;
						i = 6;
						mis->_miVar1 = monster[mid]._mmode;
						mis->_miVar2 = mid;
						monster[mid]._mmode = MM_STONE;
						break;
					}
				}
			}
		}
	}

	if (j != -99) {
		mis->_miDelFlag = TRUE;
	} else {
		mis->_mix = tx;
		mis->_miy = ty;
		mis->_misx = mis->_mix;
		mis->_misy = mis->_miy;
		mis->_mirange = mis->_mispllvl + 6;
		mis->_mirange += (mis->_mirange * plr[id]._pISplDur) >> 7;

		if (mis->_mirange > 15)
			mis->_mirange = 15;
		mis->_mirange <<= 4;
		UseMana(id, SPL_STONE);
	}
}

void AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis, *tmis;
	int i;

	mis = &missile[mi];
	mis->_miDelFlag = FALSE;
	for (i = 0; i < nummissiles; i++) {
		tmis = &missile[missileactive[i]];
		if (tmis->_mitype == MIS_GOLEM && tmis != mis && tmis->_misource == id) {
			mis->_miDelFlag = TRUE;
			return;
		}
	}
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	if ((monster[id]._mx != 1 || monster[id]._my) && id == myplr)
		MonStartKill(id, id);
	UseMana(id, SPL_GOLEM);
}

void AddEtherealize(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	mis = &missile[mi];
	mis->_mirange = 16 * plr[id]._pLevel >> 1;
	for (i = mis->_mispllvl; i > 0; i--) {
		mis->_mirange += mis->_mirange >> 3;
	}
	mis->_mirange += mis->_mirange * plr[id]._pISplDur >> 7;
	mis->_miVar1 = plr[id]._pHitPoints;
	mis->_miVar2 = plr[id]._pHPBase;
	if (!micaster)
		UseMana(id, SPL_ETHEREALIZE);
}

void miss_null_1F(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miDelFlag = TRUE;
}

void miss_null_23(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_midam = dam;
	mis->_mix = sx;
	mis->_miy = sy;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_misource = id;
	if (dam == 1)
		SetMissDir(mi, 0);
	else
		SetMissDir(mi, 1);
	mis->_miLightFlag = TRUE;
	mis->_mirange = mis->_miAnimLen;
}

void AddBoom(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	mis->_misx = dx;
	mis->_misy = dy;
	mis->_mixvel = 0;
	mis->_miyvel = 0;
	mis->_midam = dam;
	mis->_mirange = mis->_miAnimLen;
	mis->_miVar1 = 0;
}

void AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int i;
	int HealAmount;

	HealAmount = (random_(57, 10) + 1) << 6;
	for (i = 0; i < plr[id]._pLevel; i++) {
		HealAmount += (random_(57, 4) + 1) << 6;
	}
	for (i = 0; i < missile[mi]._mispllvl; i++) {
		HealAmount += (random_(57, 6) + 1) << 6;
	}

	if (plr[id]._pClass == PC_WARRIOR)
		HealAmount <<= 1;
#ifdef HELLFIRE
	else if (plr[id]._pClass == PC_BARBARIAN || plr[id]._pClass == PC_MONK)
		HealAmount <<= 1;
#endif

	if (plr[id]._pClass == PC_ROGUE)
		HealAmount += HealAmount >> 1;
#ifdef HELLFIRE
	else if (plr[id]._pClass == PC_BARD)
		HealAmount += HealAmount >> 1;
#endif

	plr[id]._pHitPoints += HealAmount;
	if (plr[id]._pHitPoints > plr[id]._pMaxHP)
		plr[id]._pHitPoints = plr[id]._pMaxHP;

	plr[id]._pHPBase += HealAmount;
	if (plr[id]._pHPBase > plr[id]._pMaxHPBase)
		plr[id]._pHPBase = plr[id]._pMaxHPBase;

	UseMana(id, SPL_HEAL);
	missile[mi]._miDelFlag = TRUE;
	drawhpflag = TRUE;
}

void AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(id, SPL_HEALOTHER);
	if (id == myplr) {
		SetCursor_(CURSOR_HEALOTHER);
		if (sgbControllerActive)
			TryIconCurs();
	}
}

void AddElement(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	mis->_midam = 2 * (plr[id]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
	for (i = mis->_mispllvl; i > 0; i--) {
		mis->_midam += mis->_midam >> 3;
	}
	mis->_midam >>= 1;
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection8(sx, sy, dx, dy));
	mis->_mirange = 256;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar3 = 0;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	mis->_mlid = AddLight(sx, sy, 8);
	UseMana(id, SPL_ELEMENT);
}

extern void FocusOnInventory();

void AddIdentify(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(id, SPL_IDENTIFY);
	if (id == myplr) {
		if (sbookflag)
			sbookflag = FALSE;
		if (!invflag) {
			invflag = TRUE;
			if (sgbControllerActive)
				FocusOnInventory();
		}
		SetCursor_(CURSOR_IDENTIFY);
	}
}

void AddFirewallC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i, j, cr, tx, ty;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	for (i = 0; i < 6; i++) {
		cr = CrawlNum[i] + 1;
#ifdef HELLFIRE
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#else
		for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#endif
			tx = dx + CrawlTable[cr];
			ty = dy + CrawlTable[cr + 1];
			if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
				if (LineClear(sx, sy, tx, ty)) {
					if ((sx != tx || sy != ty) && !(nSolidTable[dPiece[tx][ty]] | dObject[tx][ty])) {
						mis->_miVar1 = tx;
						mis->_miVar2 = ty;
						mis->_miVar5 = tx;
						mis->_miVar6 = ty;
						mis->_miDelFlag = FALSE;
						i = 6;
						break;
					}
				}
			}
		}
	}

	if (mis->_miDelFlag != TRUE) {
		mis->_miVar7 = 0;
		mis->_miVar8 = 0;
		mis->_miVar3 = (midir - 2) & 7;
		mis->_miVar4 = (midir + 2) & 7;
		mis->_mirange = 7;
		UseMana(id, SPL_FIREWALL);
	}
}

void AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	mis = &missile[mi];
	mis->_mirange = 1584;
	for (i = mis->_mispllvl; i > 0; i--) {
		mis->_mirange += mis->_mirange >> 3;
	}
	mis->_mirange += mis->_mirange * plr[id]._pISplDur >> 7;
	if (!micaster)
		UseMana(id, SPL_INFRA);
}

void AddWave(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miVar1 = dx;
	mis->_miVar2 = dy;
	mis->_miVar3 = 0;
	mis->_miVar4 = 0;
	mis->_mirange = 1;
	mis->_miAnimFrame = 4;
	UseMana(id, SPL_WAVE);
}

void AddNova(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int k;

	mis = &missile[mi];
	mis->_miVar1 = dx;
	mis->_miVar2 = dy;
	if (id != -1) {
		mis->_midam = (random_(66, 6) + random_(66, 6) + random_(66, 6) + random_(66, 6) + random_(66, 6));
		mis->_midam += plr[id]._pLevel + 5;
		mis->_midam >>= 1;
		for (k = mis->_mispllvl; k > 0; k--) {
			mis->_midam += mis->_midam >> 3;
		}
		if (!micaster)
			UseMana(id, SPL_NOVA);
	} else {
		mis->_midam = ((DWORD)currlevel >> 1) + random_(66, 3) + random_(66, 3) + random_(66, 3);
	}
	mis->_mirange = 1;
}

void AddBloodboil(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
#ifdef HELLFIRE
	int lvl;

	if (id == -1 || plr[id]._pSpellFlags & 6 || plr[id]._pHitPoints <= plr[id]._pLevel << 6) {
		missile[mi]._miDelFlag = TRUE;
	} else {
		int blodboilSFX[NUM_CLASSES] = {
			PS_WARR70,
			PS_ROGUE70,
			PS_MAGE70,
			PS_MAGE70,  // BUGFIX: PS_MONK70?
			PS_ROGUE70,
			PS_WARR70
		};
		UseMana(id, 22);
		missile[mi]._miVar1 = id;
		int tmp = 3 * plr[id]._pLevel;
		tmp <<= 7;
		plr[id]._pSpellFlags |= 2u;
		missile[mi]._miVar2 = tmp;
		if (2 * (id > 0))
			lvl = plr[id]._pLevel;
		else
			lvl = 1;
		missile[mi]._mirange = lvl + 10 * missile[mi]._mispllvl + 245;
		CalcPlrItemVals(id, TRUE);
		force_redraw = 255;
		PlaySfxLoc(blodboilSFX[plr[id]._pClass], plr[id]._px, plr[id]._py);
	}
#else
	missile[mi]._miDelFlag = 1;
#endif
}

void AddRepair(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(id, SPL_REPAIR);
	if (id == myplr) {
		if (sbookflag)
			sbookflag = FALSE;
		if (!invflag) {
			invflag = TRUE;
			if (sgbControllerActive)
				FocusOnInventory();
		}
		SetCursor_(CURSOR_REPAIR);
	}
}

void AddRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(id, SPL_RECHARGE);
	if (id == myplr) {
		if (sbookflag)
			sbookflag = FALSE;
		if (!invflag) {
			invflag = TRUE;
			if (sgbControllerActive)
				FocusOnInventory();
		}
		SetCursor_(CURSOR_RECHARGE);
	}
}

void AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(id, SPL_DISARM);
	if (id == myplr) {
		SetCursor_(CURSOR_DISARM);
		if (sgbControllerActive) {
			if (pcursobj != -1)
				NetSendCmdLocParam1(true, CMD_DISARMXY, cursmx, cursmy, pcursobj);
			else
				SetCursor_(CURSOR_HAND);
		}
	}
}

void AddApoca(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	mis = &missile[mi];
	mis->_miVar1 = 8;
	mis->_miVar2 = sy - mis->_miVar1;
	mis->_miVar3 = mis->_miVar1 + sy;
	mis->_miVar4 = sx - mis->_miVar1;
	mis->_miVar5 = mis->_miVar1 + sx;
	mis->_miVar6 = mis->_miVar4;
	if (mis->_miVar2 <= 0)
		mis->_miVar2 = 1;
	if (mis->_miVar3 >= MAXDUNY)
		mis->_miVar3 = MAXDUNY - 1;
	if (mis->_miVar4 <= 0)
		mis->_miVar4 = 1;
	if (mis->_miVar5 >= MAXDUNX)
		mis->_miVar5 = MAXDUNX - 1;
	for (i = 0; i < plr[id]._pLevel; i++) {
		mis->_midam += random_(67, 6) + 1;
	}
	mis->_mirange = 255;
	mis->_miDelFlag = FALSE;
	UseMana(id, SPL_APOCA);
}

void AddFlame(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	int i;

	mis = &missile[mi];
	mis->_miVar2 = 0;
	for (i = dam; i > 0; i--) {
		mis->_miVar2 += 5;
	}
	mis->_misx = dx;
	mis->_misy = dy;
	mis->_mixoff = missile[midir]._mixoff;
	mis->_miyoff = missile[midir]._miyoff;
	mis->_mitxoff = missile[midir]._mitxoff;
	mis->_mityoff = missile[midir]._mityoff;
	mis->_mirange = mis->_miVar2 + 20;
	mis->_mlid = AddLight(sx, sy, 1);
	if (!micaster) {
		i = random_(79, plr[id]._pLevel) + random_(79, 2);
		mis->_midam = 8 * i + 16 + ((8 * i + 16) >> 1);
	} else {
		mis->_midam = monster[id].mMinDamage + random_(77, monster[id].mMaxDamage - monster[id].mMinDamage + 1);
	}
}

void AddFlamec(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 32);
	if (!micaster)
		UseMana(id, SPL_FLAME);
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	missile[mi]._miVar3 = 0;
	missile[mi]._mirange = 256;
}

void AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;
	assert((DWORD)mi < MAXMISSILES);

	mis = &missile[mi];
	if (micaster == 0) {
		if (id == myplr) {
			mis->_mirnd = random_(63, 15) + 1;
			mis->_midam = random_(68, plr[id]._pMagic >> 2) + 1;
		} else {
			mis->_mirnd = random_(63, 15) + 1;
			mis->_midam = random_(68, plr[id]._pMagic >> 2) + 1;
		}
	} else {
		mis->_mirnd = random_(63, 15) + 1;
		mis->_midam = 15;
	}

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}

	mis->_miAnimFrame = random_(63, 8) + 1;
	mis->_mlid = AddLight(sx, sy, 5);

	GetMissileVel(mi, sx, sy, dx, dy, 8);
	mis->_miVar1 = 5;
	mis->_miVar2 = midir;
	mis->_miVar3 = 0;
	mis->_mirange = 256;
}

void AddHbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int sp;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (id != -1) {
		sp = 2 * missile[mi]._mispllvl + 16;
		if (sp >= 63) {
			sp = 63;
		}
	} else {
		sp = 16;
	}
	GetMissileVel(mi, sx, sy, dx, dy, sp);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	missile[mi]._mirange = 256;
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	missile[mi]._mlid = AddLight(sx, sy, 8);
	missile[mi]._midam = random_(69, 10) + plr[id]._pLevel + 9;
	UseMana(id, SPL_HBOLT);
}

void AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	UseMana(id, SPL_RESURRECT);
	if (id == myplr) {
		SetCursor_(CURSOR_RESURRECT);
		if (sgbControllerActive)
			TryIconCurs();
	}
	missile[mi]._miDelFlag = TRUE;
}

void AddResurrectBeam(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	mis->_misx = mis->_mix;
	mis->_misy = mis->_miy;
	mis->_mixvel = 0;
	mis->_miyvel = 0;
	mis->_mirange = misfiledata[MFILE_RESSUR1].mAnimLen[0];
}

void AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(id, SPL_TELEKINESIS);
	if (id == myplr)
		SetCursor_(CURSOR_TELEKINESIS);
}

void AddBoneSpirit(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx = XDirAdd[midir] + dx;
		dy = YDirAdd[midir] + dy;
	}
	mis = &missile[mi];
	mis->_midam = 0;
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection8(sx, sy, dx, dy));
	mis->_mirange = 256;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar3 = 0;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	mis->_mlid = AddLight(sx, sy, 8);
	if (!micaster) {
		UseMana(id, SPL_BONESPIRIT);
		plr[id]._pHitPoints -= 384;
		plr[id]._pHPBase -= 384;
		drawhpflag = TRUE;
		if (plr[id]._pHitPoints <= 0)
			SyncPlrKill(id, 0);
	}
}

void AddRportal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mix = sx;
	mis->_miy = sy;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_mirange = 100;
	mis->_miVar1 = 100 - mis->_miAnimLen;
	mis->_miVar2 = 0;
	PutMissile(mi);
}

void AddDiabApoca(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam)
{
	int pnum;

	for (pnum = 0; pnum < gbMaxPlayers; pnum++) {
		if (plr[pnum].plractive) {
			if (LineClear(sx, sy, plr[pnum]._pfutx, plr[pnum]._pfuty)) {
				AddMissile(0, 0, plr[pnum]._pfutx, plr[pnum]._pfuty, 0, MIS_BOOM2, micaster, id, dam, 0);
			}
		}
	}
	missile[mi]._miDelFlag = TRUE;
}

int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, char micaster, int id, int midam, int spllvl)
{
	MissileStruct *mis;
	int i, mi;

#ifdef HELLFIRE
	if (nummissiles >= MAXMISSILES - 1)
#else
	if (nummissiles >= MAXMISSILES)
#endif
		return -1;

	if (mitype == MIS_MANASHIELD && plr[id].pManaShield == TRUE) {
		if (currlevel != plr[id].plrlevel)
			return -1;

		for (i = 0; i < nummissiles; i++) {
			mis = &missile[missileactive[i]];
			if (mis->_mitype == MIS_MANASHIELD && mis->_misource == id)
				return -1;
		}
	}

	mi = missileavail[0];

	missileavail[0] = missileavail[MAXMISSILES - nummissiles - 1];
	missileactive[nummissiles] = mi;
	nummissiles++;

	mis = &missile[mi];
#ifdef HELLFIRE
	memset(mis, 0, sizeof(*mis));
#endif

	mis->_mitype = mitype;
	mis->_micaster = micaster;
	mis->_misource = id;
	mis->_miAnimType = missiledata[mitype].mFileNum;
	mis->_miDrawFlag = missiledata[mitype].mDraw;
	mis->_mispllvl = spllvl;
	mis->_mimfnum = midir;

	if (mis->_miAnimType == MFILE_NONE || misfiledata[mis->_miAnimType].mAnimFAmt < 8)
		SetMissDir(mi, 0);
	else
		SetMissDir(mi, midir);

	mis->_mix = sx;
	mis->_miy = sy;
	mis->_mixoff = 0;
	mis->_miyoff = 0;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_mitxoff = 0;
	mis->_mityoff = 0;
	mis->_miDelFlag = FALSE;
	mis->_miAnimAdd = 1;
	mis->_miLightFlag = FALSE;
	mis->_miPreFlag = FALSE;
	mis->_miUniqTrans = 0;
	mis->_midam = midam;
	mis->_miHitFlag = FALSE;
	mis->_midist = 0;
	mis->_mlid = -1;
	mis->_mirnd = 0;

	if (missiledata[mitype].mlSFX != -1) {
		PlaySfxLoc(missiledata[mitype].mlSFX, mis->_misx, mis->_misy);
	}

	missiledata[mitype].mAddProc(mi, sx, sy, dx, dy, midir, micaster, id, midam);

	return mi;
}

int Sentfire(int mi, int sx, int sy)
{
	MissileStruct *mis;
	int ex, dir;

	mis = &missile[mi];
	ex = 0;
	if (LineClear(mis->_mix, mis->_miy, sx, sy)) {
		if (dMonster[sx][sy] > 0 && monster[dMonster[sx][sy] - 1]._mhitpoints >> 6 > 0 && dMonster[sx][sy] - 1 > MAX_PLRS - 1) {
			dir = GetDirection(mis->_mix, mis->_miy, sx, sy);
			mis->_miVar3 = missileavail[0];
			AddMissile(mis->_mix, mis->_miy, sx, sy, dir, MIS_FIREBOLT, 0, mis->_misource, mis->_midam, GetSpellLevel(mis->_misource, SPL_FIREBOLT));
			ex = -1;
		}
	}
	if (ex == -1) {
		SetMissDir(mi, 2);
		mis->_miVar2 = 3;
	}

	return ex;
}

void MI_Dummy(int mi)
{
	return;
}

void MI_Golem(int mi)
{
	MissileStruct *mis;
	int tx, ty, i, j, src, cr;
	char *ct;

	mis = &missile[mi];
	src = mis->_misource;
	if (monster[src]._mx == 1 && !monster[src]._my) {
		for (i = 0; i < 6; i++) {
			cr = CrawlNum[i] + 1;
#ifdef HELLFIRE
			for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#else
			for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#endif
				tx = mis->_miVar4 + CrawlTable[cr];
				ty = mis->_miVar5 + CrawlTable[cr + 1];
				if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
					if (LineClear(mis->_miVar1, mis->_miVar2, tx, ty)) {
						if (!(dMonster[tx][ty] | nSolidTable[dPiece[tx][ty]] | dObject[tx][ty])) {
							i = 6;
							SpawnGolum(src, tx, ty, mi);
							break;
						}
					}
				}
			}
		}
	}
	mis->_miDelFlag = TRUE;
}

void MI_SetManashield(int mi)
{
	ManashieldFlag = TRUE;
}

void MI_LArrow(int mi)
{
	MissileStruct *mis;
	int mpnum, mind, maxd, rst;

	mis = &missile[mi];
	mis->_mirange--;
	mpnum = mis->_misource;
	if (mis->_miAnimType == MFILE_MINILTNG || mis->_miAnimType == MFILE_MAGBLOS) {
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, mis->_miAnimFrame + 5);
		rst = missiledata[mis->_mitype].mResist;
		if (mis->_mitype == MIS_LARROW) {
			if (mpnum != -1) {
				mind = plr[mpnum]._pILMinDam;
				maxd = plr[mpnum]._pILMaxDam;
			} else {
				mind = random_(68, 10) + 1 + currlevel;
				maxd = random_(68, 10) + 1 + currlevel * 2;
			}
			missiledata[MIS_LARROW].mResist = MISR_LIGHTNING;
			CheckMissileCol(mi, mind, maxd, FALSE, mis->_mix, mis->_miy, TRUE);
		}
		if (mis->_mitype == MIS_FARROW) {
			if (mpnum != -1) {
				mind = plr[mpnum]._pIFMinDam;
				maxd = plr[mpnum]._pIFMaxDam;
			} else {
				mind = random_(68, 10) + 1 + currlevel;
				maxd = random_(68, 10) + 1 + currlevel * 2;
			}
			missiledata[MIS_FARROW].mResist = MISR_FIRE;
			CheckMissileCol(mi, mind, maxd, FALSE, mis->_mix, mis->_miy, TRUE);
		}
		missiledata[mis->_mitype].mResist = rst;
	} else {
		mis->_midist++;
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);

		if (mpnum != -1) {
			if (mis->_micaster == 0) {
				mind = plr[mpnum]._pIMinDam;
				maxd = plr[mpnum]._pIMaxDam;
			} else {
				mind = monster[mpnum].mMinDamage;
				maxd = monster[mpnum].mMaxDamage;
			}
		} else {
			mind = random_(68, 10) + 1 + currlevel;
			maxd = random_(68, 10) + 1 + currlevel * 2;
		}

		if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
			rst = missiledata[mis->_mitype].mResist;
			missiledata[mis->_mitype].mResist = 0;
			CheckMissileCol(mi, mind, maxd, FALSE, mis->_mix, mis->_miy, FALSE);
			missiledata[mis->_mitype].mResist = rst;
		}
		if (mis->_mirange == 0) {
			mis->_mimfnum = 0;
			mis->_mitxoff -= mis->_mixvel;
			mis->_mityoff -= mis->_miyvel;
			GetMissilePos(mi);
			if (mis->_mitype == MIS_LARROW)
				SetMissAnim(mi, MFILE_MINILTNG);
			else
				SetMissAnim(mi, MFILE_MAGBLOS);
			mis->_mirange = mis->_miAnimLen - 1;
		} else {
			if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
				mis->_miVar1 = mis->_mix;
				mis->_miVar2 = mis->_miy;
				ChangeLight(mis->_mlid, mis->_miVar1, mis->_miVar2, 5);
			}
		}
	}
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	PutMissile(mi);
}

void MI_Arrow(int mi)
{
	MissileStruct *mis;
	int mpnum, mind, maxd;

	mis = &missile[mi];
	mis->_mirange--;
	mis->_midist++;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	mpnum = mis->_misource;
	if (mpnum != -1) {
		if (mis->_micaster == 0) {
			mind = plr[mpnum]._pIMinDam;
			maxd = plr[mpnum]._pIMaxDam;
		} else {
			mind = monster[mpnum].mMinDamage;
			maxd = monster[mpnum].mMaxDamage;
		}
	} else {
		mind = currlevel;
		maxd = 2 * currlevel;
	}
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy)
		CheckMissileCol(mi, mind, maxd, FALSE, mis->_mix, mis->_miy, FALSE);
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Firebolt(int mi)
{
	MissileStruct *mis;
	int omx, omy;
	int dam, mpnum;

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_mitype != MIS_BONESPIRIT || mis->_mimfnum != 8) {
		omx = mis->_mitxoff;
		omy = mis->_mityoff;
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		mpnum = mis->_misource;
		if (mpnum != -1) {
			if (mis->_micaster == 0) {
				switch (mis->_mitype) {
				case MIS_FIREBOLT:
					dam = random_(75, 10) + (plr[mpnum]._pMagic >> 3) + mis->_mispllvl + 1;
					break;
				case MIS_FLARE:
					dam = 3 * mis->_mispllvl - (plr[mpnum]._pMagic >> 3) + (plr[mpnum]._pMagic >> 1);
					break;
				case MIS_BONESPIRIT:
					dam = 0;
					break;
				}
			} else {
				dam = monster[mpnum].mMinDamage + random_(77, monster[mpnum].mMaxDamage - monster[mpnum].mMinDamage + 1);
			}
		} else {
			dam = currlevel + random_(78, 2 * currlevel);
		}
		if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
			CheckMissileCol(mi, dam, dam, FALSE, mis->_mix, mis->_miy, FALSE);
		}
		if (mis->_mirange == 0) {
			mis->_miDelFlag = TRUE;
			mis->_mitxoff = omx;
			mis->_mityoff = omy;
			GetMissilePos(mi);
			switch (mis->_mitype) {
			case MIS_FIREBOLT:
			case MIS_MAGMABALL:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_MISEXP, mis->_micaster, mis->_misource, 0, 0);
				break;
			case MIS_FLARE:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_MISEXP2, mis->_micaster, mis->_misource, 0, 0);
				break;
			case MIS_ACID:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_MISEXP3, mis->_micaster, mis->_misource, 0, 0);
				break;
			case MIS_BONESPIRIT:
				SetMissDir(mi, 8);
				mis->_mirange = 7;
				mis->_miDelFlag = FALSE;
				PutMissile(mi);
				return;
#ifdef HELLFIRE
			case MIS_LICH:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_EXORA1, mis->_micaster, mis->_misource, 0, 0);
				break;
			case MIS_PSYCHORB:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_EXBL2, mis->_micaster, mis->_misource, 0, 0);
				break;
			case MIS_NECROMORB:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_EXRED3, mis->_micaster, mis->_misource, 0, 0);
				break;
			case MIS_ARCHLICH:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_EXYEL2, mis->_micaster, mis->_misource, 0, 0);
				break;
			case MIS_BONEDEMON:
				AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_EXBL3, mis->_micaster, mis->_misource, 0, 0);
				break;
#endif
			}
			if (mis->_mlid >= 0)
				AddUnLight(mis->_mlid);
			PutMissile(mi);
		} else {
			if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
				mis->_miVar1 = mis->_mix;
				mis->_miVar2 = mis->_miy;
				if (mis->_mlid >= 0)
					ChangeLight(mis->_mlid, mis->_miVar1, mis->_miVar2, 8);
			}
			PutMissile(mi);
		}
	} else if (mis->_mirange == 0) {
		if (mis->_mlid >= 0)
			AddUnLight(mis->_mlid);
		mis->_miDelFlag = TRUE;
		PlaySfxLoc(LS_BSIMPCT, mis->_mix, mis->_miy);
		PutMissile(mi);
	} else
		PutMissile(mi);
}

void MI_Lightball(int mi)
{
	MissileStruct *mis;
	int tx, ty, range, oi;
	char obj;

	mis = &missile[mi];
	tx = mis->_miVar1;
	ty = mis->_miVar2;
	mis->_mirange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	range = mis->_mirange;
	CheckMissileCol(mi, mis->_midam, mis->_midam, FALSE, mis->_mix, mis->_miy, FALSE);
	if (mis->_miHitFlag == TRUE)
		mis->_mirange = range;
	obj = dObject[tx][ty];
	if (obj && tx == mis->_mix && ty == mis->_miy) {
		if (obj > 0) {
			oi = obj - 1;
		} else {
			oi = -1 - obj;
		}
		if (object[oi]._otype == OBJ_SHRINEL || object[oi]._otype == OBJ_SHRINER)
			mis->_mirange = range;
	}
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void mi_null_33(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mirange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	CheckMissileCol(mi, mis->_midam, mis->_midam, FALSE, mis->_mix, mis->_miy, FALSE);
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Acidpud(int mi)
{
	MissileStruct *mis;
	int range;

	mis = &missile[mi];
	mis->_mirange--;
	range = mis->_mirange;
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix, mis->_miy, FALSE);
	mis->_mirange = range;
	if (range == 0) {
		if (mis->_mimfnum != 0) {
			mis->_miDelFlag = TRUE;
		} else {
			SetMissDir(mi, 1);
			mis->_mirange = mis->_miAnimLen;
		}
	}
	PutMissile(mi);
}

void MI_Firewall(int mi)
{
	MissileStruct *mis;
	int ExpLight[14] = { 2, 3, 4, 5, 5, 6, 7, 8, 9, 10, 11, 12, 12 };

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_mirange == mis->_miVar1) {
		SetMissDir(mi, 1);
		mis->_miAnimFrame = random_(83, 11) + 1;
	}
	if (mis->_mirange == mis->_miAnimLen - 1) {
		SetMissDir(mi, 0);
		mis->_miAnimFrame = 13;
		mis->_miAnimAdd = -1;
	}
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix, mis->_miy, TRUE);
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	if (mis->_mimfnum != 0 && mis->_mirange != 0 && mis->_miAnimAdd != -1 && mis->_miVar2 < 12) {
		if (mis->_miVar2 == 0)
			mis->_mlid = AddLight(mis->_mix, mis->_miy, ExpLight[0]);
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}
	PutMissile(mi);
}

void MI_Fireball(int mi)
{
	MissileStruct *mis;
	int dam, mpnum, px, py, mx, my;

	mis = &missile[mi];
	mpnum = mis->_misource;
	dam = mis->_midam;
	mis->_mirange--;

	if (mis->_micaster == 0) {
		px = plr[mpnum]._px;
		py = plr[mpnum]._py;
	} else {
		px = monster[mpnum]._mx;
		py = monster[mpnum]._my;
	}

	if (mis->_miAnimType == MFILE_BIGEXP) {
		if (mis->_mirange == 0) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_mlid);
		}
	} else {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		if (mis->_mix != mis->_misx || mis->_miy != mis->_misy)
			CheckMissileCol(mi, dam, dam, 0, mis->_mix, mis->_miy, 0);
		if (!mis->_mirange) {
			mx = mis->_mix;
			my = mis->_miy;
			ChangeLight(mis->_mlid, mis->_mix, my, mis->_miAnimFrame);
			if (!CheckBlock(px, py, mx, my))
				CheckMissileCol(mi, dam, dam, 0, mx, my, 1);
			if (!CheckBlock(px, py, mx, my + 1))
				CheckMissileCol(mi, dam, dam, 0, mx, my + 1, 1);
			if (!CheckBlock(px, py, mx, my - 1))
				CheckMissileCol(mi, dam, dam, 0, mx, my - 1, 1);
			if (!CheckBlock(px, py, mx + 1, my))
				CheckMissileCol(mi, dam, dam, 0, mx + 1, my, 1);
			if (!CheckBlock(px, py, mx + 1, my - 1))
				CheckMissileCol(mi, dam, dam, 0, mx + 1, my - 1, 1);
			if (!CheckBlock(px, py, mx + 1, my + 1))
				CheckMissileCol(mi, dam, dam, 0, mx + 1, my + 1, 1);
			if (!CheckBlock(px, py, mx - 1, my))
				CheckMissileCol(mi, dam, dam, 0, mx - 1, my, 1);
			if (!CheckBlock(px, py, mx - 1, my + 1))
				CheckMissileCol(mi, dam, dam, 0, mx - 1, my + 1, 1);
			if (!CheckBlock(px, py, mx - 1, my - 1))
				CheckMissileCol(mi, dam, dam, 0, mx - 1, my - 1, 1);
			if (!TransList[dTransVal[mx][my]]
			    || (mis->_mixvel < 0 && ((TransList[dTransVal[mx][my + 1]] && nSolidTable[dPiece[mx][my + 1]]) || (TransList[dTransVal[mx][my - 1]] && nSolidTable[dPiece[mx][my - 1]])))) {
				mis->_mix++;
				mis->_miy++;
				mis->_miyoff -= 32;
			}
			if (mis->_miyvel > 0
			    && (TransList[dTransVal[mx + 1][my]] && nSolidTable[dPiece[mx + 1][my]]
			           || TransList[dTransVal[mx - 1][my]] && nSolidTable[dPiece[mx - 1][my]])) {
				mis->_miyoff -= 32;
			}
			if (mis->_mixvel > 0
			    && (TransList[dTransVal[mx][my + 1]] && nSolidTable[dPiece[mx][my + 1]]
			           || TransList[dTransVal[mx][my - 1]] && nSolidTable[dPiece[mx][my - 1]])) {
				mis->_mixoff -= 32;
			}
			mis->_mimfnum = 0;
			SetMissAnim(mi, MFILE_BIGEXP);
			mis->_mirange = mis->_miAnimLen - 1;
		} else if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
			mis->_miVar1 = mis->_mix;
			mis->_miVar2 = mis->_miy;
			ChangeLight(mis->_mlid, mis->_miVar1, mis->_miVar2, 8);
		}
	}

	PutMissile(mi);
}

#ifdef HELLFIRE
void MI_HorkSpawn(int mi)
{
	MissileStruct *mis;
	int i, j, cr, tx, ty;

	mis = &missile[mi];
	mis->_mirange--;
	CheckMissileCol(mi, 0, 0, 0, mis->_mix, mis->_miy, 0);
	if (mis->_mirange <= 0) {
		mis->_miDelFlag = TRUE;
		for (i = 0; i < 2; i++) {
			cr = CrawlNum[i] + 1;
			for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
				tx = mis->_mix + CrawlTable[cr];
				ty = mis->_miy + CrawlTable[cr + 1];
				if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
					if (!nSolidTable[dPiece[tx][ty]] && !dMonster[tx][ty] && !dPlayer[tx][ty] && !dObject[tx][ty]) {
						i = 6;
						int mon = AddMonster(tx, ty, mis->_miVar1, 1, TRUE);
						MonStartStand(mon, mis->_miVar1);
						break;
					}
				}
			}
		}
	} else {
		mis->_midist++;
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
	}
	PutMissile(mi);
}

void MI_Rune(int mi)
{
	MissileStruct *mis;
	int mid, pid, dir, mx, my;

	mis = &missile[mi];
	mx = mis->_mix;
	my = mis->_miy;
	mid = dMonster[mx][my];
	pid = dPlayer[mx][my];
	if (mid != 0 || pid != 0) {
		if (mid != 0) {
			if (mid > 0)
				mid = mid - 1;
			else
				mid = -(mid + 1);
			dir = GetDirection(mis->_mix, mis->_miy, monster[mid]._mx, monster[mid]._my);
		} else {
			if (pid > 0)
				pid = pid - 1;
			else
				pid = -(pid + 1);
			dir = GetDirection(mis->_mix, mis->_miy, plr[pid]._px, plr[pid]._py);
		}
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
		AddMissile(mx, my, mx, my, dir, mis->_miVar1, 2, mis->_misource, mis->_midam, mis->_mispllvl);
	}
	PutMissile(mi);
}

void MI_LightWall(int mi)
{
	MissileStruct *mis;
	int range;

	mis = &missile[mi];
	mis->_mirange--;
	range = mis->_mirange;
	CheckMissileCol(mi, mis->_midam, mis->_midam, 1, mis->_mix, mis->_miy, 0);
	if (mis->_miHitFlag == TRUE)
		mis->_mirange = range;
	if (!mis->_mirange)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Hivectrl(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_mirange <= 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	PutMissile(mi);
}

void MI_Immolation(int mi)
{
	MissileStruct *mis;
	int dam, mpnum, px, py, mx, my, xof, yof;

	mis = &missile[mi];
	mpnum = mis->_misource;
	dam = mis->_midam;

	if (mis->_miVar7 < 0) {
		int v = 2 * mis->_miVar6;
		mis->_miVar6 = v;
		mis->_miVar7 = v;
		mis->_mimfnum--;
		if (mis->_mimfnum < 0)
			mis->_mimfnum = 7;
	} else {
		mis->_miVar7--;
	}

	switch (mis->_mimfnum) {
	case DIR_S:
		xof = mis->_mixvel;
		yof = 0;
		break;
	case DIR_SW:
		xof = mis->_mixvel;
		yof = mis->_miyvel;
		break;
	case DIR_W:
		xof = 0;
		yof = mis->_miyvel;
		break;
	case DIR_NW:
		xof = mis->_mixvel;
		yof = mis->_miyvel;
		break;
	case DIR_N:
		xof = mis->_mixvel;
		yof = 0;
		break;
	case DIR_NE:
		xof = mis->_mixvel;
		yof = mis->_miyvel;
		break;
	case DIR_E:
		xof = 0;
		yof = mis->_miyvel;
		break;
	case DIR_SE:
		xof = mis->_mixvel;
		yof = mis->_miyvel;
		break;
	}
	mis->_mirange--;

	if (mis->_micaster == 0) {
		px = plr[mpnum]._px;
		py = plr[mpnum]._py;
	} else {
		px = monster[mpnum]._mx;
		py = monster[mpnum]._my;
	}

	if (mis->_miAnimType == MFILE_BIGEXP) {
		if (!mis->_mirange) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_mlid);
		}
	} else {
		mis->_mitxoff += xof;
		mis->_mityoff += yof;
		GetMissilePos(mi);
		if (mis->_mix != mis->_misx || mis->_miy != mis->_misy)
			CheckMissileCol(mi, dam, dam, FALSE, mis->_mix, mis->_miy, FALSE);
		if (mis->_mirange == 0) {
			mx = mis->_mix;
			my = mis->_miy;
			ChangeLight(mis->_mlid, mis->_mix, my, mis->_miAnimFrame);
			if (!CheckBlock(px, py, mx, my))
				CheckMissileCol(mi, dam, dam, FALSE, mx, my, TRUE);
			if (!CheckBlock(px, py, mx, my + 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx, my + 1, TRUE);
			if (!CheckBlock(px, py, mx, my - 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx, my - 1, TRUE);
			if (!CheckBlock(px, py, mx + 1, my))
				CheckMissileCol(mi, dam, dam, FALSE, mx + 1, my, TRUE);
			if (!CheckBlock(px, py, mx + 1, my - 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx + 1, my - 1, TRUE);
			if (!CheckBlock(px, py, mx + 1, my + 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx + 1, my + 1, TRUE);
			if (!CheckBlock(px, py, mx - 1, my))
				CheckMissileCol(mi, dam, dam, FALSE, mx - 1, my, TRUE);
			if (!CheckBlock(px, py, mx - 1, my + 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx - 1, my + 1, TRUE);
			if (!CheckBlock(px, py, mx - 1, my - 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx - 1, my - 1, TRUE);
			if (!TransList[dTransVal[mx][my]]
			    || (mis->_mixvel < 0 && ((TransList[dTransVal[mx][my + 1]] && nSolidTable[dPiece[mx][my + 1]]) || (TransList[dTransVal[mx][my - 1]] && nSolidTable[dPiece[mx][my - 1]])))) {
				mis->_mix++;
				mis->_miy++;
				mis->_miyoff -= 32;
			}
			if (mis->_miyvel > 0
			    && (TransList[dTransVal[mx + 1][my]] && nSolidTable[dPiece[mx + 1][my]]
			           || TransList[dTransVal[mx - 1][my]] && nSolidTable[dPiece[mx - 1][my]])) {
				mis->_miyoff -= 32;
			}
			if (mis->_mixvel > 0
			    && (TransList[dTransVal[mx][my + 1]] && nSolidTable[dPiece[mx][my + 1]]
			           || TransList[dTransVal[mx][my - 1]] && nSolidTable[dPiece[mx][my - 1]])) {
				mis->_mixoff -= 32;
			}
			mis->_mimfnum = 0;
			SetMissAnim(mi, MFILE_BIGEXP);
			mis->_mirange = mis->_miAnimLen - 1;
		} else if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
			mis->_miVar1 = mis->_mix;
			mis->_miVar2 = mis->_miy;
			ChangeLight(mis->_mlid, mis->_miVar1, mis->_miVar2, 8);
		}
		mis->_miDelFlag = TRUE;
	}

	PutMissile(mi);
}

void MI_LightArrow(int mi)
{
	MissileStruct *mis;
	int pn, dam, mx, my;

	mis = &missile[mi];
	mis->_mirange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);

	mx = mis->_mix;
	my = mis->_miy;
	/// ASSERT: assert((DWORD)mx < MAXDUNX);
	/// ASSERT: assert((DWORD)my < MAXDUNY);
	pn = dPiece[mx][my];
	/// ASSERT: assert((DWORD)pn <= MAXTILES);

	if (mis->_misource == -1) {
		if ((mx != mis->_misx || my != mis->_misy) && nMissileTable[pn]) {
			mis->_mirange = 0;
		}
	} else if (nMissileTable[pn]) {
		mis->_mirange = 0;
	}

	if (!nMissileTable[pn]) {
		if ((mx != mis->_miVar1 || my != mis->_miVar2) && mx > 0 && my > 0 && mx < MAXDUNX && my < MAXDUNY) {
			if (mis->_misource != -1) {
				if (mis->_micaster == 1
				    && monster[mis->_misource].MType->mtype >= MT_STORM
				    && monster[mis->_misource].MType->mtype <= MT_MAEL) {
					AddMissile(
					    mx,
					    my,
					    mis->_misx,
					    mis->_misy,
					    mi,
					    MIS_LIGHTNING2,
					    mis->_micaster,
					    mis->_misource,
					    dam,
					    mis->_mispllvl);
				} else {
					AddMissile(
					    mx,
					    my,
					    mis->_misx,
					    mis->_misy,
					    mi,
					    MIS_LIGHTNING,
					    mis->_micaster,
					    mis->_misource,
					    dam,
					    mis->_mispllvl);
				}
			} else {
				AddMissile(
				    mx,
				    my,
				    mis->_misx,
				    mis->_misy,
				    mi,
				    MIS_LIGHTNING,
				    mis->_micaster,
				    mis->_misource,
				    dam,
				    mis->_mispllvl);
			}
			mis->_miVar1 = mis->_mix;
			mis->_miVar2 = mis->_miy;
		}
	}

	if (mis->_mirange == 0 || mx <= 0 || my <= 0 || mx >= MAXDUNX || my > MAXDUNY) { // BUGFIX my >= MAXDUNY
		mis->_miDelFlag = TRUE;
	}
}

void MI_Flashfr(int mi)
{
	MissileStruct *mis;
	int pnum;

	mis = &missile[mi];
	pnum = mis->_misource;
	if (!mis->_micaster && pnum != -1) {
		mis->_mix = plr[pnum]._px;
		mis->_miy = plr[pnum]._py;
		mis->_mitxoff = plr[pnum]._pxoff << 16;
		mis->_mityoff = plr[pnum]._pyoff << 16;
	}
	mis->_mirange--;
	if (!mis->_mirange) {
		mis->_miDelFlag = TRUE;
		if (!mis->_micaster) {
			pnum = mis->_misource;
			if (pnum != -1)
				plr[pnum]._pBaseToBlk -= 50;
		}
	}
	PutMissile(mi);
}

void MI_Flashbk(int mi)
{
	MissileStruct *mis;
	mis = &missile[mi];
	if (!mis->_micaster) {
		if (mis->_misource != -1) {
			mis->_mix = plr[mis->_misource]._pfutx;
			mis->_miy = plr[mis->_misource]._pfuty;
		}
	}
	mis->_mirange--;
	if (!mis->_mirange)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Reflect(int mi)
{
	MissileStruct *mis;
	int pnum;

	mis = &missile[mi];
	pnum = mis->_misource;
	mis->_mitxoff = plr[pnum]._pxoff << 16;
	mis->_mityoff = plr[pnum]._pyoff << 16;
	if (plr[pnum]._pmode == PM_WALK3) {
		mis->_misx = plr[pnum]._pfutx + 2;
		mis->_misy = plr[pnum]._pfuty - 1;
	} else {
		mis->_misx = plr[pnum]._px + 2;
		mis->_misy = plr[pnum]._py - 1;
	}
	GetMissilePos(mi);
	if (plr[pnum]._pmode == PM_WALK3) {
		if (plr[pnum]._pdir == DIR_W)
			mis->_mix++;
		else
			mis->_miy++;
	}
	if (pnum != myplr && currlevel != plr[pnum].plrlevel)
		mis->_miDelFlag = TRUE;
	if ((WORD)plr[pnum].wReflection <= 0) {
		mis->_miDelFlag = TRUE;
		NetSendCmd(TRUE, CMD_REFLECT);
	}
	PutMissile(mi);
}

void MI_FireRing(int mi)
{
	MissileStruct *mis;
	int src, tx, ty, dam, j, cr, pn;
	BYTE lvl;

	mis = &missile[mi];
	mis->_miDelFlag = 1;
	src = mis->_micaster;
	cr = CrawlNum[3] + 1;
	if (src > 0)
		lvl = plr[src]._pLevel;
	else
		lvl = currlevel;
	dam = 16 * (random_(53, 10) + random_(53, 10) + lvl + 2) >> 1;
	for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
		tx = mis->_miVar1 + CrawlTable[cr - 1];
		ty = mis->_miVar2 + CrawlTable[cr];
		if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
			pn = dPiece[tx][ty];
			if (!nSolidTable[pn] && !dObject[tx][ty]) {
				if (LineClear(mis->_mix, mis->_miy, tx, ty)) {
					if (nMissileTable[pn] || mis->_miVar8)
						mis->_miVar8 = 1;
					else
						AddMissile(tx, ty, tx, ty, 0, MIS_FIREWALL, 2, src, dam, mis->_mispllvl);
				}
			}
		}
	}
}

void MI_LightRing(int mi)
{
	MissileStruct *mis;
	int src, tx, ty, dam, cr, j, pn;
	BYTE lvl;

	mis = &missile[mi];
	mis->_miDelFlag = 1;
	src = mis->_micaster;
	cr = CrawlNum[3] + 1;
	if (src > 0)
		lvl = plr[src]._pLevel;
	else
		lvl = currlevel;
	dam = 16 * (random_(53, 10) + random_(53, 10) + lvl + 2) >> 1;
	for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
		tx = mis->_miVar1 + CrawlTable[cr - 1];
		ty = mis->_miVar2 + CrawlTable[cr];
		if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
			pn = dPiece[tx][ty];
			if (!nSolidTable[pn] && !dObject[tx][ty]) {
				if (LineClear(mis->_mix, mis->_miy, tx, ty)) {
					if (nMissileTable[pn] || mis->_miVar8)
						mis->_miVar8 = 1;
					else
						AddMissile(tx, ty, tx, ty, 0, MIS_LIGHTWALL, 2, src, dam, mis->_mispllvl);
				}
			}
		}
	}
}

void MI_Search(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mirange--;
	if (!mis->_mirange) {
		mis->_miDelFlag = TRUE;
		PlaySfxLoc(IS_CAST7, plr[mis->_miVar1]._px, plr[mis->_miVar1]._py);
		AutoMapShowItems = FALSE;
	}
}

void MI_LightningWall(int mi)
{
	MissileStruct *mis;
	int src, lvl, dam, tx, ty, dp;

	mis = &missile[mi];
	mis->_mirange--;
	src = mis->_misource;
	if (src > 0)
		lvl = plr[src]._pLevel;
	else
		lvl = 0;
	dam = 16 * (random_(53, 10) + random_(53, 10) + lvl + 2);
	if (!mis->_mirange) {
		mis->_miDelFlag = TRUE;
	} else {
		dp = dPiece[mis->_miVar1][mis->_miVar2];
		if (dp || 1) {
			tx = mis->_miVar1 + XDirAdd[mis->_miVar3];
			ty = mis->_miVar2 + YDirAdd[mis->_miVar3];
			if (!nMissileTable[dp] && !mis->_miVar8 && tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				AddMissile(mis->_miVar1, mis->_miVar2, mis->_miVar1, mis->_miVar2, plr[src]._pdir, MIS_LIGHTWALL, 2, src, dam, mis->_mispllvl);
				mis->_miVar1 = tx;
				mis->_miVar2 = ty;
			} else {
				mis->_miVar8 = 1;
			}
		} else {
			mis->_miVar8 = 1;
		}
		dp = dPiece[mis->_miVar5][mis->_miVar6];
		if (dp || 1) {
			tx = mis->_miVar5 + XDirAdd[mis->_miVar4];
			ty = mis->_miVar6 + YDirAdd[mis->_miVar4];
			if (!nMissileTable[dp] && !mis->_miVar7 && tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				AddMissile(mis->_miVar5, mis->_miVar6, mis->_miVar5, mis->_miVar6, plr[src]._pdir, MIS_LIGHTWALL, 2, src, dam, mis->_mispllvl);
				mis->_miVar5 = tx;
				mis->_miVar6 = ty;
			} else {
				mis->_miVar7 = 1;
			}
		} else {
			mis->_miVar7 = 1;
		}
	}
}

void MI_FireNova(int mi)
{
	MissileStruct *mis;
	int k, src, sx, sy, dir, en, sx1, sy1, dam;

	sx1 = 0;
	sy1 = 0;
	mis = &missile[mi];
	src = mis->_misource;
	dam = mis->_midam;
	sx = mis->_mix;
	sy = mis->_miy;
	if (src != -1) {
		en = 0;
		dir = plr[src]._pdir;
	} else {
		dir = 0;
		en = 1;
	}
	for (k = 0; k < 23; k++) {
		if (sx1 != vCrawlTable[k][6] || sy1 != vCrawlTable[k][7]) {
			AddMissile(sx, sy, sx + vCrawlTable[k][6], sy + vCrawlTable[k][7], dir, MIS_FIRENOVA, en, src, dam, mis->_mispllvl);
			AddMissile(sx, sy, sx - vCrawlTable[k][6], sy - vCrawlTable[k][7], dir, MIS_FIRENOVA, en, src, dam, mis->_mispllvl);
			AddMissile(sx, sy, sx - vCrawlTable[k][6], sy + vCrawlTable[k][7], dir, MIS_FIRENOVA, en, src, dam, mis->_mispllvl);
			AddMissile(sx, sy, sx + vCrawlTable[k][6], sy - vCrawlTable[k][7], dir, MIS_FIRENOVA, en, src, dam, mis->_mispllvl);
			sx1 = vCrawlTable[k][6];
			sy1 = vCrawlTable[k][7];
		}
	}
	mis->_mirange--;
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
}

void MI_SpecArrow(int mi)
{
	MissileStruct *mis;
	int dir, src, dam, sx, sy, dx, dy, spllvl, mitype, micaster;

	dir = 0;
	mis = &missile[mi];
	src = mis->_misource;
	dam = mis->_midam;
	sx = mis->_mix;
	sy = mis->_miy;
	dx = mis->_miVar1;
	dy = mis->_miVar2;
	spllvl = mis->_miVar3;
	mitype = 0;
	if (src != -1) {
		dir = plr[src]._pdir;
		micaster = 0;

		switch (plr[src]._pILMinDam) {
		case 0:
			mitype = MIS_FIRENOVA;
			break;
		case 1:
			mitype = MIS_LIGHTARROW;
			break;
		case 2:
			mitype = MIS_CBOLTARROW;
			break;
		case 3:
			mitype = MIS_HBOLTARROW;
			break;
		}
	} else {
		micaster = 1;
	}
	AddMissile(sx, sy, dx, dy, dir, mitype, micaster, src, dam, spllvl);
	if (mitype == MIS_CBOLTARROW) {
		AddMissile(sx, sy, dx, dy, dir, mitype, micaster, src, dam, spllvl);
		AddMissile(sx, sy, dx, dy, dir, mitype, micaster, src, dam, spllvl);
	}
	mis->_mirange--;
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
}

#endif
void MI_Lightctrl(int mi)
{
	MissileStruct *mis;
	int pn, dam, mpnum, mx, my;

	assert((DWORD)mi < MAXMISSILES);
	mis = &missile[mi];
	mis->_mirange--;

	mpnum = mis->_misource;
	if (mpnum != -1) {
		if (mis->_micaster == 0) {
			dam = (random_(79, 2) + random_(79, plr[mpnum]._pLevel) + 2) << 6;
		} else {
			dam = 2 * (monster[mpnum].mMinDamage + random_(80, monster[mpnum].mMaxDamage - monster[mpnum].mMinDamage + 1));
		}
	} else {
		dam = random_(81, currlevel) + 2 * currlevel;
	}

	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);

	mx = mis->_mix;
	my = mis->_miy;
	assert((DWORD)mx < MAXDUNX);
	assert((DWORD)my < MAXDUNY);
	pn = dPiece[mx][my];
	assert((DWORD)pn <= MAXTILES);

	if (mis->_misource == -1) {
		if ((mx != mis->_misx || my != mis->_misy) && nMissileTable[pn]) {
			mis->_mirange = 0;
		}
	} else if (nMissileTable[pn]) {
		mis->_mirange = 0;
	}
	if (!nMissileTable[pn]) {
		if ((mx != mis->_miVar1 || my != mis->_miVar2) && mx > 0 && my > 0 && mx < MAXDUNX && my < MAXDUNY) {
			if (mis->_misource != -1) {
				if (mis->_micaster == 1
				    && monster[mis->_misource].MType->mtype >= MT_STORM
				    && monster[mis->_misource].MType->mtype <= MT_MAEL) {
					AddMissile(
					    mx,
					    my,
					    mis->_misx,
					    mis->_misy,
					    mi,
					    MIS_LIGHTNING2,
					    mis->_micaster,
					    mis->_misource,
					    dam,
					    mis->_mispllvl);
				} else {
					AddMissile(
					    mx,
					    my,
					    mis->_misx,
					    mis->_misy,
					    mi,
					    MIS_LIGHTNING,
					    mis->_micaster,
					    mis->_misource,
					    dam,
					    mis->_mispllvl);
				}
			} else {
				AddMissile(
				    mx,
				    my,
				    mis->_misx,
				    mis->_misy,
				    mi,
				    MIS_LIGHTNING,
				    mis->_micaster,
				    mis->_misource,
				    dam,
				    mis->_mispllvl);
			}
			mis->_miVar1 = mis->_mix;
			mis->_miVar2 = mis->_miy;
		}
	}
	if (mis->_mirange == 0 || mx <= 0 || my <= 0 || mx >= MAXDUNX || my > MAXDUNY) {
		mis->_miDelFlag = TRUE;
	}
}

void MI_Lightning(int mi)
{
	MissileStruct *mis;
	int range;

	mis = &missile[mi];
	mis->_mirange--;
	range = mis->_mirange;
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy)
		CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix, mis->_miy, FALSE);
	if (mis->_miHitFlag == TRUE)
		mis->_mirange = range;
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	PutMissile(mi);
}

void MI_Town(int mi)
{
	MissileStruct *mis;
	int ExpLight[17] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15 };
	int i;

	mis = &missile[mi];
	if (mis->_mirange > 1)
		mis->_mirange--;
	if (mis->_mirange == mis->_miVar1)
		SetMissDir(mi, 1);
	if (currlevel != 0 && mis->_mimfnum != 1 && mis->_mirange != 0) {
		if (mis->_miVar2 == 0)
			mis->_mlid = AddLight(mis->_mix, mis->_miy, 1);
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && currlevel == plr[i].plrlevel && !plr[i]._pLvlChanging && plr[i]._pmode == PM_STAND && plr[i]._px == mis->_mix && plr[i]._py == mis->_miy) {
			ClrPlrPath(i);
			if (i == myplr) {
				NetSendCmdParam1(TRUE, CMD_WARP, mis->_misource);
				plr[i]._pmode = PM_NEWLVL;
			}
		}
	}

	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	PutMissile(mi);
}

void MI_Flash(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_micaster == 0) {
		if (mis->_misource != -1)
			plr[mis->_misource]._pInvincible = TRUE;
	}
	mis->_mirange--;
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix - 1, mis->_miy, TRUE);
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix, mis->_miy, TRUE);
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix + 1, mis->_miy, TRUE);
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix - 1, mis->_miy + 1, TRUE);
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix, mis->_miy + 1, TRUE);
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix + 1, mis->_miy + 1, TRUE);
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		if (mis->_micaster == 0) {
			if (mis->_misource != -1)
				plr[mis->_misource]._pInvincible = FALSE;
		}
	}
	PutMissile(mi);
}

void MI_Flash2(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_micaster == 0) {
		if (mis->_misource != -1)
			plr[mis->_misource]._pInvincible = TRUE;
	}
	mis->_mirange--;
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix - 1, mis->_miy - 1, TRUE);
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix, mis->_miy - 1, TRUE);
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix + 1, mis->_miy - 1, TRUE);
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		if (mis->_micaster == 0) {
			if (mis->_misource != -1)
				plr[mis->_misource]._pInvincible = FALSE;
		}
	}
	PutMissile(mi);
}

void MI_Manashield(int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int pnum, diff;

	mis = &missile[mi];
	pnum = mis->_misource;
	p = &plr[pnum];
	mis->_mix = p->_px;
	mis->_miy = p->_py;
	mis->_mitxoff = p->_pxoff << 16;
	mis->_mityoff = p->_pyoff << 16;
	if (p->_pmode == PM_WALK3) {
		mis->_misx = p->_pfutx;
		mis->_misy = p->_pfuty;
	} else {
		mis->_misx = p->_px;
		mis->_misy = p->_py;
	}
	GetMissilePos(mi);
	if (p->_pmode == PM_WALK3) {
		if (p->_pdir == DIR_W)
			mis->_mix++;
		else
			mis->_miy++;
	}
	if (pnum != myplr) {
		if (currlevel != p->plrlevel)
			mis->_miDelFlag = TRUE;
	} else {
		if (p->_pMana <= 0 || !p->plractive)
			mis->_mirange = 0;
		if (p->_pHitPoints < mis->_miVar1) {
			diff = mis->_miVar1 - p->_pHitPoints;
#ifdef HELLFIRE
			int div = 0;
			for (int m = 0; m < mis->_mispllvl && m < 7; m++) {
				div += 3;
			}
			if (div > 0)
				diff -= diff / div;
#else
			if (mis->_mispllvl > 0) {
				diff += diff / -3;
			}
#endif

			if (diff < 0)
				diff = 0;
			drawmanaflag = TRUE;
			drawhpflag = TRUE;

			if (p->_pMana >= diff) {
				p->_pHitPoints = mis->_miVar1;
				p->_pHPBase = mis->_miVar2;
				p->_pMana -= diff;
				p->_pManaBase -= diff;
			} else {
#ifdef HELLFIRE
				p->_pHitPoints += p->_pMana - diff;
				p->_pHPBase += p->_pMana - diff;
#else
				p->_pHitPoints = p->_pMana + mis->_miVar1 - diff;
				p->_pHPBase = p->_pMana + mis->_miVar2 - diff;
#endif
				p->_pMana = 0;
				p->_pManaBase = p->_pMaxManaBase - p->_pMaxMana;
				mis->_mirange = 0;
				mis->_miDelFlag = TRUE;
				if (p->_pHitPoints < 0)
					SetPlayerHitPoints(pnum, 0);
				if ((p->_pHitPoints >> 6) == 0 && pnum == myplr) {
					SyncPlrKill(pnum, mis->_miVar8);
				}
			}
		}

#ifndef HELLFIRE
		if (pnum == myplr && p->_pHitPoints == 0 && mis->_miVar1 == 0 && p->_pmode != PM_DEATH) {
			mis->_mirange = 0;
			mis->_miDelFlag = TRUE;
			SyncPlrKill(pnum, -1);
		}
#endif
		mis->_miVar1 = p->_pHitPoints;
		mis->_miVar2 = p->_pHPBase;
		if (mis->_mirange == 0) {
			mis->_miDelFlag = TRUE;
			NetSendCmd(TRUE, CMD_ENDSHIELD);
		}
	}
	PutMissile(mi);
}

void MI_Etherealize(int mi)
{
	MissileStruct *mis;
	int pnum;

	mis = &missile[mi];
	mis->_mirange--;
	pnum = mis->_misource;
	mis->_mix = plr[pnum]._px;
	mis->_miy = plr[pnum]._py;
	mis->_mitxoff = plr[pnum]._pxoff << 16;
	mis->_mityoff = plr[pnum]._pyoff << 16;
	if (plr[pnum]._pmode == PM_WALK3) {
		mis->_misx = plr[pnum]._pfutx;
		mis->_misy = plr[pnum]._pfuty;
	} else {
		mis->_misx = plr[pnum]._px;
		mis->_misy = plr[pnum]._py;
	}
	GetMissilePos(mi);
	if (plr[pnum]._pmode == PM_WALK3) {
		if (plr[pnum]._pdir == DIR_W)
			mis->_mix++;
		else
			mis->_miy++;
	}
	plr[pnum]._pSpellFlags |= 1;
	if (mis->_mirange == 0 || plr[pnum]._pHitPoints <= 0) {
		mis->_miDelFlag = TRUE;
		plr[pnum]._pSpellFlags &= ~0x1;
	}
	PutMissile(mi);
}

void MI_Firemove(int mi)
{
	MissileStruct *mis;
	int range;
	int ExpLight[14] = { 2, 3, 4, 5, 5, 6, 7, 8, 9, 10, 11, 12, 12 };

	mis = &missile[mi];
	mis->_mix--;
	mis->_miy--;
	mis->_miyoff += 32;
	mis->_miVar1++;
	if (mis->_miVar1 == mis->_miAnimLen) {
		SetMissDir(mi, 1);
		mis->_miAnimFrame = random_(82, 11) + 1;
	}
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	range = mis->_mirange;
	CheckMissileCol(mi, mis->_midam, mis->_midam, FALSE, mis->_mix, mis->_miy, FALSE);
	if (mis->_miHitFlag == TRUE)
		mis->_mirange = range;
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	if (mis->_mimfnum != 0 || mis->_mirange == 0) {
		if (mis->_mix != mis->_miVar3 || mis->_miy != mis->_miVar4) {
			mis->_miVar3 = mis->_mix;
			mis->_miVar4 = mis->_miy;
			ChangeLight(mis->_mlid, mis->_miVar3, mis->_miVar4, 8);
		}
	} else {
		if (mis->_miVar2 == 0)
			mis->_mlid = AddLight(mis->_mix, mis->_miy, ExpLight[0]);
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}
	mis->_mix++;
	mis->_miy++;
	mis->_miyoff -= 32;
	PutMissile(mi);
}

void MI_Guardian(int mi)
{
	MissileStruct *mis;
	int j, k, sx, sy, sx1, sy1, ex;

	assert((DWORD)mi < MAXMISSILES);

#ifndef HELLFIRE
	sx1 = 0;
	sy1 = 0;
#endif
	mis = &missile[mi];
	mis->_mirange--;

	if (mis->_miVar2 > 0) {
		mis->_miVar2--;
	}
	if (mis->_mirange == mis->_miVar1 || mis->_mimfnum == MFILE_GUARD && mis->_miVar2 == 0) {
		SetMissDir(mi, 1);
	}

	if (!(mis->_mirange % 16)) {
		ex = 0;
		for (j = 0; j < 23 && ex != -1; j++) {
			for (k = 10; k >= 0 && ex != -1 && (vCrawlTable[j][k] != 0 || vCrawlTable[j][k + 1] != 0); k -= 2) {
				if (sx1 == vCrawlTable[j][k] && sy1 == vCrawlTable[j][k + 1]) {
					continue;
				}
				sx = mis->_mix + vCrawlTable[j][k];
				sy = mis->_miy + vCrawlTable[j][k + 1];
				ex = Sentfire(mi, sx, sy);
				if (ex == -1) {
					break;
				}
				sx = mis->_mix - vCrawlTable[j][k];
				sy = mis->_miy - vCrawlTable[j][k + 1];
				ex = Sentfire(mi, sx, sy);
				if (ex == -1) {
					break;
				}
				sx = mis->_mix + vCrawlTable[j][k];
				sy = mis->_miy - vCrawlTable[j][k + 1];
				ex = Sentfire(mi, sx, sy);
				if (ex == -1) {
					break;
				}
				sx = mis->_mix - vCrawlTable[j][k];
				sy = mis->_miy + vCrawlTable[j][k + 1];
				ex = Sentfire(mi, sx, sy);
				if (ex == -1) {
					break;
				}
				sx1 = vCrawlTable[j][k];
				sy1 = vCrawlTable[j][k + 1];
			}
		}
	}

	if (mis->_mirange == 14) {
		SetMissDir(mi, 0);
		mis->_miAnimFrame = 15;
		mis->_miAnimAdd = -1;
	}

	mis->_miVar3 += mis->_miAnimAdd;

	if (mis->_miVar3 > 15) {
		mis->_miVar3 = 15;
	} else if (mis->_miVar3 > 0) {
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, mis->_miVar3);
	}

	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}

	PutMissile(mi);
}

void MI_Chain(int mi)
{
	MissileStruct *mis;
	int sx, sy, src, cr, i, j, rad, tx, ty, dir;

	mis = &missile[mi];
	src = mis->_misource;
	sx = mis->_mix;
	sy = mis->_miy;
	dir = GetDirection(sx, sy, mis->_miVar1, mis->_miVar2);
	AddMissile(sx, sy, mis->_miVar1, mis->_miVar2, dir, MIS_LIGHTCTRL, 0, src, 1, mis->_mispllvl);
	rad = mis->_mispllvl + 3;
	if (rad > 19)
		rad = 19;
	for (i = 1; i < rad; i++) {
		cr = CrawlNum[i] + 1;
#ifdef HELLFIRE
		for (j = CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#else
		for (j = (BYTE)CrawlTable[cr - 1]; j > 0; j--, cr += 2) {
#endif
			tx = sx + CrawlTable[cr];
			ty = sy + CrawlTable[cr + 1];
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY && dMonster[tx][ty] > 0) {
				dir = GetDirection(sx, sy, tx, ty);
				AddMissile(sx, sy, tx, ty, dir, MIS_LIGHTCTRL, 0, src, 1, mis->_mispllvl);
			}
		}
	}
	mis->_mirange--;
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
}

void mi_null_11(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
	if (mis->_miAnimFrame == mis->_miAnimLen)
		mis->_miPreFlag = TRUE;
	PutMissile(mi);
}

void MI_Weapexp(int mi)
{
	MissileStruct *mis;
	int pnum, mind, maxd;
	int ExpLight[10] = { 9, 10, 11, 12, 11, 10, 8, 6, 4, 2 };

	mis = &missile[mi];
	mis->_mirange--;
	pnum = mis->_misource;
	if (mis->_miVar2 == 1) {
		mind = plr[pnum]._pIFMinDam;
		maxd = plr[pnum]._pIFMaxDam;
		missiledata[mis->_mitype].mResist = MISR_FIRE;
	} else {
		mind = plr[pnum]._pILMinDam;
		maxd = plr[pnum]._pILMaxDam;
		missiledata[mis->_mitype].mResist = MISR_LIGHTNING;
	}
	CheckMissileCol(mi, mind, maxd, FALSE, mis->_mix, mis->_miy, FALSE);
	if (mis->_miVar1 == 0) {
		mis->_mlid = AddLight(mis->_mix, mis->_miy, 9);
	} else {
		if (mis->_mirange != 0)
			ChangeLight(mis->_mlid, mis->_mix, mis->_miy, ExpLight[mis->_miVar1]);
	}
	mis->_miVar1++;
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	} else {
		PutMissile(mi);
	}
}

void MI_Misexp(int mi)
{
	MissileStruct *mis;
#ifdef HELLFIRE
	int ExpLight[] = { 9, 10, 11, 12, 11, 10, 8, 6, 4, 2, 1, 0, 0, 0, 0 };
#else
	int ExpLight[10] = { 9, 10, 11, 12, 11, 10, 8, 6, 4, 2 };
#endif

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	} else {
		if (mis->_miVar1 == 0)
			mis->_mlid = AddLight(mis->_mix, mis->_miy, 9);
		else
			ChangeLight(mis->_mlid, mis->_mix, mis->_miy, ExpLight[mis->_miVar1]);
		mis->_miVar1++;
		PutMissile(mi);
	}
}

void MI_Acidsplat(int mi)
{
	MissileStruct *mis;
	int mnum, dam;

	mis = &missile[mi];
	if (mis->_mirange == mis->_miAnimLen) {
		mis->_mix++;
		mis->_miy++;
		mis->_miyoff -= 32;
	}
	mis->_mirange--;
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		mnum = mis->_misource;
		dam = mis->_mispllvl;
		AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_mimfnum, MIS_ACIDPUD, 1, mnum, (monster[mnum].MData->mLevel >= 2) + 1, dam);
	} else {
		PutMissile(mi);
	}
}

void MI_Teleport(int mi)
{
	MissileStruct *mis;
	int pnum;

	mis = &missile[mi];
	pnum = mis->_misource;
	mis->_mirange--;
	if (mis->_mirange <= 0) {
		mis->_miDelFlag = TRUE;
	} else {
		dPlayer[plr[pnum]._px][plr[pnum]._py] = 0;
		PlrClrTrans(plr[pnum]._px, plr[pnum]._py);
		plr[pnum]._px = mis->_mix;
		plr[pnum]._py = mis->_miy;
		plr[pnum]._pfutx = plr[pnum]._px;
		plr[pnum]._pfuty = plr[pnum]._py;
		plr[pnum]._poldx = plr[pnum]._px;
		plr[pnum]._poldy = plr[pnum]._py;
		PlrDoTrans(plr[pnum]._px, plr[pnum]._py);
		mis->_miVar1 = 1;
		dPlayer[plr[pnum]._px][plr[pnum]._py] = pnum + 1;
		if (leveltype != DTYPE_TOWN) {
			ChangeLightXY(plr[pnum]._plid, plr[pnum]._px, plr[pnum]._py);
			ChangeVisionXY(plr[pnum]._pvid, plr[pnum]._px, plr[pnum]._py);
		}
		if (pnum == myplr) {
			ViewX = plr[pnum]._px - ScrollInfo._sdx;
			ViewY = plr[pnum]._py - ScrollInfo._sdy;
		}
	}
}

void MI_Stone(int mi)
{
	MissileStruct *mis;
	int mnum;

	mis = &missile[mi];
	mis->_mirange--;
	mnum = mis->_miVar2;
	if (monster[mnum]._mhitpoints == 0 && mis->_miAnimType != MFILE_SHATTER1) {
#ifndef HELLFIRE
		mis->_mimfnum = 0;
		mis->_miDrawFlag = TRUE;
#endif
		SetMissAnim(mi, MFILE_SHATTER1);
		mis->_mirange = 11;
	}
	if (monster[mnum]._mmode != MM_STONE) {
		mis->_miDelFlag = TRUE;
		return;
	}

	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		if (monster[mnum]._mhitpoints > 0)
			monster[mnum]._mmode = mis->_miVar1;
		else
			AddDead(monster[mnum]._mx, monster[mnum]._my, stonendx, (direction)monster[mnum]._mdir);
	}
	if (mis->_miAnimType == MFILE_SHATTER1)
		PutMissile(mi);
}

void MI_Boom(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_miVar1 == 0)
		CheckMissileCol(mi, mis->_midam, mis->_midam, FALSE, mis->_mix, mis->_miy, TRUE);
	if (mis->_miHitFlag == TRUE)
		mis->_miVar1 = 1;
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Rhino(int mi)
{
	MissileStruct *mis;
	int mix, miy, mix2, miy2, omx, omy, mnum;

	mis = &missile[mi];
	mnum = mis->_misource;
	if (monster[mnum]._mmode != MM_CHARGE) {
		mis->_miDelFlag = TRUE;
		return;
	}
	GetMissilePos(mi);
	mix = mis->_mix;
	miy = mis->_miy;
	dMonster[mix][miy] = 0;
	if (monster[mnum]._mAi == AI_SNAKE) {
		mis->_mitxoff += 2 * mis->_mixvel;
		mis->_mityoff += 2 * mis->_miyvel;
		GetMissilePos(mi);
		mix2 = mis->_mix;
		miy2 = mis->_miy;
		mis->_mitxoff -= mis->_mixvel;
		mis->_mityoff -= mis->_miyvel;
	} else {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
	}
	GetMissilePos(mi);
	omx = mis->_mix;
	omy = mis->_miy;
	if (!PosOkMonst(mnum, mis->_mix, mis->_miy) || (monster[mnum]._mAi == AI_SNAKE && !PosOkMonst(mnum, mix2, miy2))) {
		MissToMonst(mi, mix, miy);
		mis->_miDelFlag = TRUE;
		return;
	}
	monster[mnum]._mfutx = omx;
	monster[mnum]._moldx = omx;
	dMonster[omx][omy] = -1 - mnum;
	monster[mnum]._mx = omx;
	monster[mnum]._mfuty = omy;
	monster[mnum]._moldy = omy;
	monster[mnum]._my = omy;
	if (monster[mnum]._uniqtype != 0)
		ChangeLightXY(mis->_mlid, omx, omy);
	MoveMissilePos(mi);
	PutMissile(mi);
}

void mi_null_32(int mi)
{
	MissileStruct *mis;
	int src, enemy, ax, ay, bx, by, cx, cy, j;

	GetMissilePos(mi);
	mis = &missile[mi];
	ax = mis->_mix;
	ay = mis->_miy;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	src = mis->_misource;
	bx = mis->_mix;
	by = mis->_miy;
	enemy = monster[src]._menemy;
	if (!(monster[src]._mFlags & MFLAG_TARGETS_MONSTER)) {
		cx = plr[enemy]._px;
		cy = plr[enemy]._py;
	} else {
		cx = monster[enemy]._mx;
		cy = monster[enemy]._my;
	}
	if ((bx != ax || by != ay) && (mis->_miVar1 & 1 && (abs(ax - cx) >= 4 || abs(ay - cy) >= 4) || mis->_miVar2 > 1) && PosOkMonst(mis->_misource, ax, ay)) {
		MissToMonst(mi, ax, ay);
		mis->_miDelFlag = TRUE;
	} else if (!(monster[src]._mFlags & MFLAG_TARGETS_MONSTER)) {
		j = dPlayer[bx][by];
	} else {
		j = dMonster[bx][by];
	}
	if (!PosOkMissile(bx, by) || j > 0 && !(mis->_miVar1 & 1)) {
		mis->_mixvel *= -1;
		mis->_miyvel *= -1;
		mis->_mimfnum = opposite[mis->_mimfnum];
		mis->_miAnimData = monster[src].MType->Anims[MA_WALK].Data[mis->_mimfnum];
		mis->_miVar2++;
		if (j > 0)
			mis->_miVar1 |= 1;
	}
	MoveMissilePos(mi);
	PutMissile(mi);
}

void MI_FirewallC(int mi)
{
	MissileStruct *mis;
	int tx, ty, pnum;

	mis = &missile[mi];
	pnum = mis->_misource;
	mis->_mirange--;
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
	} else {
		tx = mis->_miVar1 + XDirAdd[mis->_miVar3];
		ty = mis->_miVar2 + YDirAdd[mis->_miVar3];
		if (!nMissileTable[dPiece[mis->_miVar1][mis->_miVar2]] && mis->_miVar8 == 0 && tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
			AddMissile(mis->_miVar1, mis->_miVar2, mis->_miVar1, mis->_miVar2, plr[pnum]._pdir, MIS_FIREWALL, 0, pnum, 0, mis->_mispllvl);
			mis->_miVar1 = tx;
			mis->_miVar2 = ty;
		} else {
			mis->_miVar8 = 1;
		}
		tx = mis->_miVar5 + XDirAdd[mis->_miVar4];
		ty = mis->_miVar6 + YDirAdd[mis->_miVar4];
		if (!nMissileTable[dPiece[mis->_miVar5][mis->_miVar6]] && mis->_miVar7 == 0 && tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
			AddMissile(mis->_miVar5, mis->_miVar6, mis->_miVar5, mis->_miVar6, plr[pnum]._pdir, MIS_FIREWALL, 0, pnum, 0, mis->_mispllvl);
			mis->_miVar5 = tx;
			mis->_miVar6 = ty;
		} else {
			mis->_miVar7 = 1;
		}
	}
}

void MI_Infra(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mirange--;
	plr[mis->_misource]._pInfraFlag = TRUE;
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		CalcPlrItemVals(mis->_misource, TRUE);
	}
}

void MI_Apoca(int mi)
{
	MissileStruct *mis;
	int j, k, pnum;
	BOOL exit;

	mis = &missile[mi];
	pnum = mis->_misource;
	exit = FALSE;
	for (j = mis->_miVar2; j < mis->_miVar3 && !exit; j++) {
		for (k = mis->_miVar4; k < mis->_miVar5 && !exit; k++) {
			if (dMonster[k][j] > MAX_PLRS - 1 && !nSolidTable[dPiece[k][j]]) {
#ifdef HELLFIRE
				if (LineClear(mis->_mix, mis->_miy, k, j)) {
					AddMissile(k, j, k, j, plr[pnum]._pdir, MIS_BOOM, 0, pnum, mis->_midam, 0);
					exit = TRUE;
				}
#else
				AddMissile(k, j, k, j, plr[pnum]._pdir, MIS_BOOM, 0, pnum, mis->_midam, 0);
				exit = TRUE;
#endif
			}
		}
		if (!exit) {
			mis->_miVar4 = mis->_miVar6;
		}
	}

	if (exit == TRUE) {
		mis->_miVar2 = j - 1;
		mis->_miVar4 = k;
	} else {
		mis->_miDelFlag = TRUE;
	}
}

void MI_Wave(int mi)
{
	MissileStruct *mis;
	int sx, sy, sd, nxa, nxb, nya, nyb, dira, dirb;
	int j, pnum, pn;
	BOOL f1, f2;
	int v1, v2;

	f1 = FALSE;
	f2 = FALSE;
	assert((DWORD)mi < MAXMISSILES);

	mis = &missile[mi];
	pnum = mis->_misource;
	sx = mis->_mix;
	sy = mis->_miy;
	v1 = mis->_miVar1;
	v2 = mis->_miVar2;
	sd = GetDirection(sx, sy, v1, v2);
	dira = (sd - 2) & 7;
	dirb = (sd + 2) & 7;
	nxa = sx + XDirAdd[sd];
	nya = sy + YDirAdd[sd];
	pn = dPiece[nxa][nya];
	assert((DWORD)pn <= MAXTILES);
	if (!nMissileTable[pn]) {
		AddMissile(nxa, nya, nxa + XDirAdd[sd], nya + YDirAdd[sd], plr[pnum]._pdir, MIS_FIREMOVE, 0, pnum, 0, mis->_mispllvl);
		nxa += XDirAdd[dira];
		nya += YDirAdd[dira];
		nxb = sx + XDirAdd[sd] + XDirAdd[dirb];
		nyb = sy + YDirAdd[sd] + YDirAdd[dirb];
		for (j = 0; j < (mis->_mispllvl >> 1) + 2; j++) {
			pn = dPiece[nxa][nya]; // BUGFIX: dPiece is accessed before check against dungeon size and 0
			assert((DWORD)pn <= MAXTILES);
			if (nMissileTable[pn] || f1 || nxa <= 0 || nxa >= MAXDUNX || nya <= 0 || nya >= MAXDUNY) {
				f1 = TRUE;
			} else {
				AddMissile(nxa, nya, nxa + XDirAdd[sd], nya + YDirAdd[sd], plr[pnum]._pdir, MIS_FIREMOVE, 0, pnum, 0, mis->_mispllvl);
				nxa += XDirAdd[dira];
				nya += YDirAdd[dira];
			}
			pn = dPiece[nxb][nyb]; // BUGFIX: dPiece is accessed before check against dungeon size and 0
			assert((DWORD)pn <= MAXTILES);
			if (nMissileTable[pn] || f2 || nxb <= 0 || nxb >= MAXDUNX || nyb <= 0 || nyb >= MAXDUNY) {
				f2 = TRUE;
			} else {
				AddMissile(nxb, nyb, nxb + XDirAdd[sd], nyb + YDirAdd[sd], plr[pnum]._pdir, MIS_FIREMOVE, 0, pnum, 0, mis->_mispllvl);
				nxb += XDirAdd[dirb];
				nyb += YDirAdd[dirb];
			}
		}
	}
	mis->_mirange--;
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
}

void MI_Nova(int mi)
{
	MissileStruct *mis;
	int k, src, sx, sy, dir, en, sx1, sy1, dam;

	sx1 = 0;
	sy1 = 0;
	mis = &missile[mi];
	src = mis->_misource;
	dam = mis->_midam;
	sx = mis->_mix;
	sy = mis->_miy;
	if (src != -1) {
		en = 0;
		dir = plr[src]._pdir;
	} else {
		dir = 0;
		en = 1;
	}
	for (k = 0; k < 23; k++) {
		if (sx1 != vCrawlTable[k][6] || sy1 != vCrawlTable[k][7]) {
			AddMissile(sx, sy, sx + vCrawlTable[k][6], sy + vCrawlTable[k][7], dir, MIS_LIGHTBALL, en, src, dam, mis->_mispllvl);
			AddMissile(sx, sy, sx - vCrawlTable[k][6], sy - vCrawlTable[k][7], dir, MIS_LIGHTBALL, en, src, dam, mis->_mispllvl);
			AddMissile(sx, sy, sx - vCrawlTable[k][6], sy + vCrawlTable[k][7], dir, MIS_LIGHTBALL, en, src, dam, mis->_mispllvl);
			AddMissile(sx, sy, sx + vCrawlTable[k][6], sy - vCrawlTable[k][7], dir, MIS_LIGHTBALL, en, src, dam, mis->_mispllvl);
			sx1 = vCrawlTable[k][6];
			sy1 = vCrawlTable[k][7];
		}
	}
	mis->_mirange--;
	if (mis->_mirange == 0)
		mis->_miDelFlag = TRUE;
}

#ifdef HELLFIRE
void MI_Bloodboil(int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int lvl, pnum, hpdif;

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_mirange == 0) {
		pnum = mis->_miVar1;
		p = &plr[pnum];
		if ((p->_pSpellFlags & 2) == 2) {
			int blodboilSFX[NUM_CLASSES] = {
				PS_WARR72,
				PS_ROGUE72,
				PS_MAGE72,
				PS_MAGE72, // BUGFIX: should be PS_MONK72?
				PS_ROGUE72,
				PS_WARR72
			};
			p->_pSpellFlags &= ~0x2;
			p->_pSpellFlags |= 4;
			if (2 * (pnum > 0))
				lvl = p->_pLevel;
			else
				lvl = 1;
			mis->_mirange = lvl + 10 * mis->_mispllvl + 245;
			hpdif = p->_pMaxHP - p->_pHitPoints;
			CalcPlrItemVals(pnum, TRUE);
			p->_pHitPoints -= hpdif;
			if (p->_pHitPoints < 64)
				p->_pHitPoints = 64;
			force_redraw = 255;
			PlaySfxLoc(blodboilSFX[p->_pClass], p->_px, p->_py);
		} else {
			int blodboilSFX[NUM_CLASSES] = {
				PS_WARR72,
				PS_ROGUE72,
				PS_MAGE72,
				PS_MAGE72, // BUGFIX: should be PS_MONK72?
				PS_ROGUE72,
				PS_WARR72
			};
			mis->_miDelFlag = TRUE;
			p->_pSpellFlags &= ~0x4;
			hpdif = p->_pMaxHP - p->_pHitPoints;
			CalcPlrItemVals(pnum, TRUE);
			p->_pHitPoints -= hpdif + mis->_miVar2;
			if (p->_pHitPoints < 64)
				p->_pHitPoints = 64;
			force_redraw = 255;
			PlaySfxLoc(blodboilSFX[p->_pClass], p->_px, p->_py);
		}
	}
}
#else
void MI_Bloodboil(int mi)
{
	missile[mi]._miDelFlag = TRUE;
}
#endif

void MI_Flame(int mi)
{
	MissileStruct *mis;
	int k;

	mis = &missile[mi];
	mis->_mirange--;
	mis->_miVar2--;
	k = mis->_mirange;
	CheckMissileCol(mi, mis->_midam, mis->_midam, TRUE, mis->_mix, mis->_miy, FALSE);
	if (mis->_mirange == 0 && mis->_miHitFlag == TRUE)
		mis->_mirange = k;
	if (mis->_miVar2 == 0)
		mis->_miAnimFrame = 20;
	if (mis->_miVar2 <= 0) {
		k = mis->_miAnimFrame;
		if (k > 11)
			k = 24 - k;
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, k);
	}
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	if (mis->_miVar2 <= 0)
		PutMissile(mi);
}

void MI_Flamec(int mi)
{
	MissileStruct *mis;
	int id, src;

	mis = &missile[mi];
	mis->_mirange--;
	src = mis->_misource;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
		id = dPiece[mis->_mix][mis->_miy];
		if (!nMissileTable[id]) {
			AddMissile(
			    mis->_mix,
			    mis->_miy,
			    mis->_misx,
			    mis->_misy,
			    mi,
			    MIS_FLAME,
			    mis->_micaster,
			    src,
			    mis->_miVar3,
			    mis->_mispllvl);
		} else {
			mis->_mirange = 0;
		}
		mis->_miVar1 = mis->_mix;
		mis->_miVar2 = mis->_miy;
		mis->_miVar3++;
	}
	if (mis->_mirange == 0 || mis->_miVar3 == 3)
		mis->_miDelFlag = TRUE;
}

void MI_Cbolt(int mi)
{
	MissileStruct *mis;
	int md;
	int bpath[16] = { -1, 0, 1, -1, 0, 1, -1, -1, 0, 0, 1, 1, 0, 1, -1, 0 };

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_miAnimType != MFILE_LGHNING) {
		if (mis->_miVar3 == 0) {
			md = (mis->_miVar2 + bpath[mis->_mirnd]) & 7;
			mis->_mirnd = (mis->_mirnd + 1) & 0xF;
			GetMissileVel(mi, mis->_mix, mis->_miy, mis->_mix + XDirAdd[md], mis->_miy + YDirAdd[md], 8);
			mis->_miVar3 = 16;
		} else {
			mis->_miVar3--;
		}
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		CheckMissileCol(mi, mis->_midam, mis->_midam, FALSE, mis->_mix, mis->_miy, FALSE);
		if (mis->_miHitFlag == TRUE) {
			mis->_miVar1 = 8;
			mis->_mimfnum = 0;
			mis->_mixoff = 0;
			mis->_miyoff = 0;
			SetMissAnim(mi, MFILE_LGHNING);
			mis->_mirange = mis->_miAnimLen;
			GetMissilePos(mi);
		}
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, mis->_miVar1);
	}
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	PutMissile(mi);
}

void MI_Hbolt(int mi)
{
	MissileStruct *mis;
	int dam;

	mis = &missile[mi];
	mis->_mirange--;
	if (mis->_miAnimType != MFILE_HOLYEXPL) {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		dam = mis->_midam;
		if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
			CheckMissileCol(mi, dam, dam, FALSE, mis->_mix, mis->_miy, FALSE);
		}
		if (mis->_mirange == 0) {
			mis->_mitxoff -= mis->_mixvel;
			mis->_mityoff -= mis->_miyvel;
			GetMissilePos(mi);
			mis->_mimfnum = 0;
			SetMissAnim(mi, MFILE_HOLYEXPL);
			mis->_mirange = mis->_miAnimLen - 1;
		} else {
			if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
				mis->_miVar1 = mis->_mix;
				mis->_miVar2 = mis->_miy;
				ChangeLight(mis->_mlid, mis->_miVar1, mis->_miVar2, 8);
			}
		}
	} else {
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, mis->_miAnimFrame + 7);
		if (mis->_mirange == 0) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_mlid);
		}
	}
	PutMissile(mi);
}

void MI_Element(int mi)
{
	MissileStruct *mis;
	int mid, sd, dam, cx, cy, px, py, pnum;

	mis = &missile[mi];
	mis->_mirange--;
	dam = mis->_midam;
	pnum = mis->_misource;
	if (mis->_miAnimType == MFILE_BIGEXP) {
		cx = mis->_mix;
		cy = mis->_miy;
		px = plr[pnum]._px;
		py = plr[pnum]._py;
		ChangeLight(mis->_mlid, cx, cy, mis->_miAnimFrame);
		if (!CheckBlock(px, py, cx, cy))
			CheckMissileCol(mi, dam, dam, TRUE, cx, cy, TRUE);
		if (!CheckBlock(px, py, cx, cy + 1))
			CheckMissileCol(mi, dam, dam, TRUE, cx, cy + 1, TRUE);
		if (!CheckBlock(px, py, cx, cy - 1))
			CheckMissileCol(mi, dam, dam, TRUE, cx, cy - 1, TRUE);
		if (!CheckBlock(px, py, cx + 1, cy))
			CheckMissileCol(mi, dam, dam, TRUE, cx + 1, cy, TRUE); /* check x/y */
		if (!CheckBlock(px, py, cx + 1, cy - 1))
			CheckMissileCol(mi, dam, dam, TRUE, cx + 1, cy - 1, TRUE);
		if (!CheckBlock(px, py, cx + 1, cy + 1))
			CheckMissileCol(mi, dam, dam, TRUE, cx + 1, cy + 1, TRUE);
		if (!CheckBlock(px, py, cx - 1, cy))
			CheckMissileCol(mi, dam, dam, TRUE, cx - 1, cy, TRUE);
		if (!CheckBlock(px, py, cx - 1, cy + 1))
			CheckMissileCol(mi, dam, dam, TRUE, cx - 1, cy + 1, TRUE);
		if (!CheckBlock(px, py, cx - 1, cy - 1))
			CheckMissileCol(mi, dam, dam, TRUE, cx - 1, cy - 1, TRUE);
		if (mis->_mirange == 0) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_mlid);
		}
	} else {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		cx = mis->_mix;
		cy = mis->_miy;
		CheckMissileCol(mi, dam, dam, FALSE, cx, cy, FALSE);
		if (mis->_miVar3 == 0 && cx == mis->_miVar4 && cy == mis->_miVar5)
			mis->_miVar3 = 1;
		if (mis->_miVar3 == 1) {
			mis->_miVar3 = 2;
			mis->_mirange = 255;
			mid = FindClosest(cx, cy, 19);
			if (mid > 0) {
				sd = GetDirection8(cx, cy, monster[mid]._mx, monster[mid]._my);
				SetMissDir(mi, sd);
				GetMissileVel(mi, cx, cy, monster[mid]._mx, monster[mid]._my, 16);
			} else {
				sd = plr[pnum]._pdir;
				SetMissDir(mi, sd);
				GetMissileVel(mi, cx, cy, cx + XDirAdd[sd], cy + YDirAdd[sd], 16);
			}
		}
		if (cx != mis->_miVar1 || cy != mis->_miVar2) {
			mis->_miVar1 = cx;
			mis->_miVar2 = cy;
			ChangeLight(mis->_mlid, cx, cy, 8);
		}
		if (mis->_mirange == 0) {
			mis->_mimfnum = 0;
			SetMissAnim(mi, MFILE_BIGEXP);
			mis->_mirange = mis->_miAnimLen - 1;
		}
	}
	PutMissile(mi);
}

void MI_Bonespirit(int mi)
{
	MissileStruct *mis;
	int pnum, mid, sd, dam;
	int cx, cy;

	mis = &missile[mi];
	mis->_mirange--;
	dam = mis->_midam;
	pnum = mis->_misource;
	if (mis->_mimfnum == 8) {
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, mis->_miAnimFrame);
		if (mis->_mirange == 0) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_mlid);
		}
		PutMissile(mi);
	} else {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		cx = mis->_mix;
		cy = mis->_miy;
		CheckMissileCol(mi, dam, dam, FALSE, cx, cy, FALSE);
		if (mis->_miVar3 == 0 && cx == mis->_miVar4 && cy == mis->_miVar5)
			mis->_miVar3 = 1;
		if (mis->_miVar3 == 1) {
			mis->_miVar3 = 2;
			mis->_mirange = 255;
			mid = FindClosest(cx, cy, 19);
			if (mid > 0) {
				mis->_midam = monster[mid]._mhitpoints >> 7;
				SetMissDir(mi, GetDirection8(cx, cy, monster[mid]._mx, monster[mid]._my));
				GetMissileVel(mi, cx, cy, monster[mid]._mx, monster[mid]._my, 16);
			} else {
				sd = plr[pnum]._pdir;
				SetMissDir(mi, sd);
				GetMissileVel(mi, cx, cy, cx + XDirAdd[sd], cy + YDirAdd[sd], 16);
			}
		}
		if (cx != mis->_miVar1 || cy != mis->_miVar2) {
			mis->_miVar1 = cx;
			mis->_miVar2 = cy;
			ChangeLight(mis->_mlid, cx, cy, 8);
		}
		if (mis->_mirange == 0) {
			SetMissDir(mi, DIR_OMNI);
			mis->_mirange = 7;
		}
		PutMissile(mi);
	}
}

void MI_ResurrectBeam(int mi)
{
	missile[mi]._mirange--;
	if (missile[mi]._mirange == 0)
		missile[mi]._miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Rportal(int mi)
{
	MissileStruct *mis;
	int ExpLight[17] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15 };

	mis = &missile[mi];
	if (mis->_mirange > 1)
		mis->_mirange--;
	if (mis->_mirange == mis->_miVar1)
		SetMissDir(mi, 1);

	if (currlevel != 0 && mis->_mimfnum != 1 && mis->_mirange != 0) {
		if (mis->_miVar2 == 0)
			mis->_mlid = AddLight(mis->_mix, mis->_miy, 1);
		ChangeLight(mis->_mlid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}
	if (mis->_mirange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_mlid);
	}
	PutMissile(mi);
}

void ProcessMissiles()
{
	MissileStruct *mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		dFlags[mis->_mix][mis->_miy] &= ~BFLAG_MISSILE;
		dMissile[mis->_mix][mis->_miy] = 0;
#ifdef HELLFIRE
		if (mis->_mix < 0 || mis->_mix >= MAXDUNX - 1 || mis->_miy < 0 || mis->_miy >= MAXDUNY - 1)
			mis->_miDelFlag = TRUE;
#endif
	}

	i = 0;
	while (i < nummissiles) {
		if (missile[missileactive[i]]._miDelFlag) {
			DeleteMissile(missileactive[i], i);
			i = 0;
		} else {
			i++;
		}
	}

	MissilePreFlag = FALSE;
	ManashieldFlag = FALSE;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		missiledata[mis->_mitype].mProc(missileactive[i]);
		if (!(mis->_miAnimFlags & MFLAG_LOCK_ANIMATION)) {
			mis->_miAnimCnt++;
			if (mis->_miAnimCnt >= mis->_miAnimDelay) {
				mis->_miAnimCnt = 0;
				mis->_miAnimFrame += mis->_miAnimAdd;
				if (mis->_miAnimFrame > mis->_miAnimLen)
					mis->_miAnimFrame = 1;
				if (mis->_miAnimFrame < 1)
					mis->_miAnimFrame = mis->_miAnimLen;
			}
		}
	}

	if (ManashieldFlag) {
		for (i = 0; i < nummissiles; i++) {
			if (missile[missileactive[i]]._mitype == MIS_MANASHIELD) {
				MI_Manashield(missileactive[i]);
			}
		}
	}

	i = 0;
	while (i < nummissiles) {
#ifdef HELLFIRE
		if (missile[missileactive[i]]._miDelFlag == TRUE) {
#else
		if (missile[missileactive[i]]._miDelFlag) {
#endif
			DeleteMissile(missileactive[i], i);
			i = 0;
		} else {
			i++;
		}
	}
}

void missiles_process_charge()
{
	CMonster *mon;
	AnimStruct *anim;
	MissileStruct *mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		mis->_miAnimData = misfiledata[mis->_miAnimType].mAnimData[mis->_mimfnum];
		if (mis->_mitype == MIS_RHINO) {
			mon = monster[mis->_misource].MType;
			if (mon->mtype >= MT_HORNED && mon->mtype <= MT_OBLORD) {
				anim = &mon->Anims[MA_SPECIAL];
			} else {
				if (mon->mtype >= MT_NSNAKE && mon->mtype <= MT_GSNAKE)
					anim = &mon->Anims[MA_ATTACK];
				else
					anim = &mon->Anims[MA_WALK];
			}
			mis->_miAnimData = anim->Data[mis->_mimfnum];
		}
	}
}

void ClearMissileSpot(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	dFlags[mis->_mix][mis->_miy] &= ~BFLAG_MISSILE;
	dMissile[mis->_mix][mis->_miy] = 0;
}

DEVILUTION_END_NAMESPACE
