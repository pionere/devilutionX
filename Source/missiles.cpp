/**
 * @file missiles.cpp
 *
 * Implementation of missile functionality.
 */
#include "all.h"
#include "plrctrls.h"
#include "misproc.h"
#include "engine/render/cl2_render.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

DEVILUTION_BEGIN_NAMESPACE

/*
 * Similar to walk offsetx/y with PLR/MON_WALK_SHIFT, missile velocity values
 * are shifted with MIS_*VELO_SHIFT to the higher range for better precision.
 * if MIS_VELO_SHIFT is set to 0 and MIS_BASE_VELO_SHIFT to 16:
 *    the result is reduced code size with slightly slower runtime speed.
 * if MIS_VELO_SHIFT is set to 16, MIS_BASE_VELO_SHIFT to 0:
 *    the result is increased code size with slightly better runtime speed.
 */
#define MIS_VELO_SHIFT	0
#define MIS_BASE_VELO_SHIFT	16
#define MIS_SHIFTEDVEL(x) ((x) << MIS_VELO_SHIFT)

int missileactive[MAXMISSILES];
MissileStruct missile[MAXMISSILES];
int nummissiles;

// TODO: merge XDirAdd/YDirAdd, offset_x/offset_y, bxadd/byadd, pathxdir/pathydir, plrxoff2/plryoff2, trm3x/trm3y
/** Maps from direction to X-offset. */
static const int XDirAdd[NUM_DIRS] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to Y-offset. */
static const int YDirAdd[NUM_DIRS] = { 1, 1, 1, 0, -1, -1, -1, 0 };

static_assert(MAX_LIGHT_RAD >= 9, "FireWallLight needs at least light-radius of 9.");
static const int FireWallLight[14] = { 2, 2, 3, 4, 5, 5, 6, 7, 8, 9, 9, 8, 9, 9 };

void GetDamageAmt(int sn, int sl, int *minv, int *maxv)
{
	int k, magic, mind, maxd;

	assert((unsigned)mypnum < MAX_PLRS);
	assert((unsigned)sn < NUM_SPELLS);
	magic = myplr._pMagic;
#ifdef HELLFIRE
	if (SPELL_RUNE(sn))
		sl += myplr._pDexterity >> 3;
#endif
	switch (sn) {
	case SPL_FIREBOLT:
	case SPL_GUARDIAN:
		k = (magic >> 3) + sl;
		mind = k + 1;
		maxd = k + 10;
		break;
#ifdef HELLFIRE
	case SPL_RUNELIGHT:
#endif
	case SPL_LIGHTNING:
		mind = 1;
		maxd = ((magic + (sl << 3)) * (6 + (sl >> 1))) >> 3;
		break;
	case SPL_FLASH:
		mind = magic >> 1;
		for (k = 0; k < sl; k++)
			mind += mind >> 3;

		mind *= misfiledata[MFILE_BLUEXFR].mfAnimLen[0];
		maxd = mind << 3;
		mind >>= 6;
		maxd >>= 6;
		break;
	case SPL_NULL:
	case SPL_WALK:
	case SPL_BLOCK:
	case SPL_ATTACK:
	case SPL_SWIPE:
	case SPL_RATTACK:
	case SPL_POINT_BLANK:
	case SPL_FAR_SHOT:
	case SPL_CHARGE:
	case SPL_RAGE:
	case SPL_STONE:
	case SPL_INFRA:
	case SPL_MANASHIELD:
	case SPL_ATTRACT:
	case SPL_TELEKINESIS:
	case SPL_TELEPORT:
	case SPL_RNDTELEPORT:
	case SPL_TOWN:
	case SPL_HEAL:
	case SPL_HEALOTHER:
	case SPL_RESURRECT:
	case SPL_IDENTIFY:
	case SPL_REPAIR:
	case SPL_RECHARGE:
	case SPL_DISARM:
#ifdef HELLFIRE
	case SPL_BUCKLE:
	case SPL_WHITTLE:
	case SPL_RUNESTONE:
#endif
		mind = -1;
		maxd = -1;
		break;
#ifdef HELLFIRE
	case SPL_FIRERING:
#endif
	case SPL_FIREWALL:
		mind = ((magic >> 3) + sl + 5) << (-3 + 5);
		maxd = ((magic >> 3) + sl * 2 + 10) << (-3 + 5);
		break;
	case SPL_FIREBALL:
		mind = (magic >> 2) + 10;
		maxd = mind + 10;
		for (k = 0; k < sl; k++) {
			mind += mind >> 3;
			maxd += maxd >> 3;
		}
		break;
	case SPL_METEOR:
		mind = (magic >> 2) + (sl << 3) + 40;
		maxd = (magic >> 2) + (sl << 4) + 40;
		break;
	case SPL_CHAIN:
		mind = 1;
		maxd = magic;
		break;
#ifdef HELLFIRE
	case SPL_RUNEWAVE:
#endif
	case SPL_WAVE:
		mind = ((magic >> 3) + 2 * sl + 1) * 4;
		maxd = ((magic >> 3) + 4 * sl + 2) * 4;
		break;
#ifdef HELLFIRE
	case SPL_RUNENOVA:
#endif
	case SPL_NOVA:
		mind = 1;
		maxd = (magic >> 1) + (sl << 5);
		break;
	case SPL_INFERNO:
		mind = (magic * 20) >> 6;
		maxd = ((magic + (sl << 4)) * 30) >> 6;
		break;
	case SPL_GOLEM:
		mind = 2 * sl + 8;
		maxd = 2 * sl + 16;
		break;
	case SPL_ELEMENTAL:
		mind = (magic >> 3) + 2 * sl + 4;
		maxd = (magic >> 3) + 4 * sl + 20;
		for (k = 0; k < sl; k++) {
			mind += mind >> 3;
			maxd += maxd >> 3;
		}
		break;
	case SPL_CBOLT:
		mind = 1;
		maxd = (magic >> 2) + (sl << 2);
		break;
	case SPL_HBOLT:
		mind = (magic >> 2) + sl;
		maxd = mind + 9;
		break;
	case SPL_FLARE:
		mind = (magic * (sl + 1)) >> 3;
		maxd = mind;
		break;
#ifdef HELLFIRE
	/*case SPL_LIGHTWALL:
		mind = 1;
		maxd = ((magic >> 1) + sl) << (-3 + 5);
		break;
	case SPL_RUNEWAVE:
	case SPL_IMMOLAT:
		mind = 1 + (magic >> 3);
		maxd = mind + 4;
		for (k = 0; k < sl; k++) {
			mind += mind >> 3;
			maxd += maxd >> 3;
		}
		break;*/
	case SPL_RUNEFIRE:
		mind = 1 + (magic >> 1) + 16 * sl;
		maxd = 1 + (magic >> 1) + 32 * sl;
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}

	*minv = mind;
	*maxv = maxd;
}

/*
 * Check if an active (missile-)entity can be placed at the given position.
 */
static bool PosOkMissile(int x, int y)
{
	if (!PosOkActor(x, y))
		return false;
	// nSolidTable is checked -> ignore the few additional tiles from nMissileTable
	return (dMissile[x][y] /*| nMissileTable[dPiece[x][y]]*/) == 0;
}

static bool FindClosest(int sx, int sy, int &dx, int &dy)
{
	int j, i, mid, mnum, tx, ty;
	const char* cr;
	MonsterStruct* mon;

	mid = dMonster[sx][sy];
	mid = mid >= 0 ? mid - 1 : -(mid + 1);

	static_assert(DBORDERX >= 16 && DBORDERY >= 16, "FindClosest expects a large enough border.");
	for (i = 1; i < 16; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mnum = dMonster[tx][ty] - 1;
			if (mnum < 0 || mnum == mid)
				continue;
			mon = &monsters[mnum];
			if (mon->_mhitpoints < (1 << 6))
				continue;
			tx = mon->_mfutx;
			ty = mon->_mfuty;
			if (LineClear(sx, sy, tx, ty)) {
				dx = tx;
				dy = ty;
				return true;
			}
		}
	}
	return false;
}

static bool FindClosestChain(int sx, int sy, int &dx, int &dy)
{
	int j, i, mid, mnum, tx, ty;
	const char* cr;
	MonsterStruct* mon;

	mid = dMonster[sx][sy];
	mid = mid >= 0 ? mid - 1 : -(mid + 1);

	static_assert(DBORDERX >= 8 && DBORDERY >= 8, "FindClosestChain expects a large enough border.");
	for (i = 1; i < 8; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mnum = dMonster[tx][ty] - 1;
			if (mnum < 0 || mnum == mid)
				continue;
			mon = &monsters[mnum];
			if (mon->_mhitpoints < (1 << 6)
			 || (mon->_mMagicRes & MORS_LIGHTNING_IMMUNE) == MORS_LIGHTNING_IMMUNE)
				continue;
			tx = mon->_mfutx;
			ty = mon->_mfuty;
			if (LineClear(sx, sy, tx, ty)) {
				dx = tx;
				dy = ty;
				return true;
			}
		}
	}
	return false;
}

/**
 * @brief Returns the direction a vector from p1(x1, y1) to p2(x2, y2) is pointing to.
 *
 *      W    SW     S
 *            ^
 *            |	
 *     NW ----+---> SE
 *            |
 *            |
 *      N    NE     E
 *
 * @param x1 the x coordinate of p1
 * @param y1 the y coordinate of p1
 * @param x2 the x coordinate of p2
 * @param y2 the y coordinate of p2
 * @return the direction of the p1->p2 vector
*/
static int GetDirection8(int x1, int y1, int x2, int y2)
{
#if UNOPTIMIZED_DIRECTION
	int mx, my, md;

	mx = x2 - x1;
	my = y2 - y1;
	if (mx >= 0) {
		if (my >= 0) {
			if (5 * mx <= (my << 1)) // mx/my <= 0.4, approximation of tan(22.5)
				return 1;            // DIR_SW
			md = 0;                  // DIR_S
		} else {
			my = -my;
			if (5 * mx <= (my << 1))
				return 5; // DIR_NE
			md = 6;       // DIR_E
		}
		if (5 * my <= (mx << 1)) // my/mx <= 0.4
			md = 7;              // DIR_SE
	} else {
		mx = -mx;
		if (my >= 0) {
			if (5 * mx <= (my << 1))
				return 1; // DIR_SW
			md = 2;       // DIR_W
		} else {
			my = -my;
			if (5 * mx <= (my << 1))
				return 5; // DIR_NE
			md = 4;       // DIR_N
		}
		if (5 * my <= (mx << 1))
			md = 3; // DIR_NW
	}
	return md;
#else
	int dx = x2 - x1;
	int dy = y2 - y1;
	unsigned adx = abs(dx);
	unsigned ady = abs(dy);
	//                        SE  NE  SW  NW
	const int BaseDirs[4] = {  7,  5,  1,  3 };
	int dir = BaseDirs[2 * (dx < 0) + (dy < 0)];
	//const int DeltaDirs[2][4] = {{0, 1, 2}, {2, 1, 0}};
	const int DeltaDirs[2][4] = {{1, 0, 2}, {1, 2, 0}};
	const int (&DeltaDir)[4] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	//dir += DeltaDir[5 * adx <= (ady << 1) ? 2 : (5 * ady <= (adx << 1) ? 0 : 1)];
	dir += DeltaDir[5 * adx <= (ady << 1) ? 2 : (5 * ady <= (adx << 1) ? 1 : 0)];
	return dir & 7;
#endif
}

/**
 * @brief Returns the direction a vector from p1(x1, y1) to p2(x2, y2) is pointing to.
 *
 *      W  sW  SW   Sw  S
 *              ^
 *     nW       |       Se
 *              |
 *     NW ------+-----> SE
 *              |
 *     Nw       |       sE
 *              |
 *      N  Ne  NE   nE  E
 *
 * @param x1 the x coordinate of p1
 * @param y1 the y coordinate of p1
 * @param x2 the x coordinate of p2
 * @param y2 the y coordinate of p2
 * @return the direction of the p1->p2 vector
*/
static int GetDirection16(int x1, int y1, int x2, int y2)
{
#if UNOPTIMIZED_DIRECTION
	int mx, my, md;
	mx = x2 - x1;
	my = y2 - y1;
	if (mx >= 0) {
		if (my >= 0) {
			if (3 * mx <= (my << 1)) { // mx/my <= 2/3, approximation of tan(33.75)
				if (5 * mx < my)       // mx/my < 0.2, approximation of tan(11.25)
					return 2;          // DIR_SW;
				return 1;              // DIR_Sw;
			}
			md = 0; // DIR_S;
		} else {
			my = -my;
			if (3 * mx <= (my << 1)) {
				if (5 * mx < my)
					return 10; // DIR_NE;
				return 11;     // DIR_nE;
			}
			md = 12; // DIR_E;
		}
		if (3 * my <= (mx << 1)) {    // my/mx <= 2/3
			if (5 * my < mx)          // my/mx < 0.2
				return 14;            // DIR_SE;
			return md == 0 ? 15 : 13; // DIR_S ? DIR_Se : DIR_sE;
		}
	} else {
		mx = -mx;
		if (my >= 0) {
			if (3 * mx <= (my << 1)) {
				if (5 * mx < my)
					return 2; // DIR_SW;
				return 3;     // DIR_sW;
			}
			md = 4; // DIR_W;
		} else {
			my = -my;
			if (3 * mx <= (my << 1)) {
				if (5 * mx < my)
					return 10; // DIR_NE;
				return 9;      // DIR_Ne;
			}
			md = 8; // DIR_N;
		}
		if (3 * my <= (mx << 1)) {
			if (5 * my < mx)
				return 6;           // DIR_NW;
			return md == 4 ? 5 : 7; // DIR_W ? DIR_nW : DIR_Nw;
		}
	}
	return md;
#else
	int dx = x2 - x1;
	int dy = y2 - y1;
	unsigned adx = abs(dx);
	unsigned ady = abs(dy);
	//                        SE  NE  SW  NW
	//const int BaseDirs[4] = { 14, 10,  2,  6 };
	const int BaseDirs[4] = { 14 + 2, 10 + 2,  2 + 2,  6 + 2 };
	int dir = BaseDirs[2 * (dx < 0) + (dy < 0)];
	//const int DeltaDirs[2][8] = {{ 0, 1, 2, 3, 4 }, { 4, 3, 2, 1, 0 }};
	const int DeltaDirs[2][4] = {{ 0 - 2, 1 - 2, 3 - 2, 4 - 2 }, { 4 - 2, 3 -2 , 1 - 2, 0 - 2 }};
	//const int (&DeltaDir)[8] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	const int (&DeltaDir)[4] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	if (3 * adx <= (ady << 1)) {
		//dir += DeltaDir[5 * adx < ady ? 4 : 3];
		dir += DeltaDir[5 * adx < ady ? 3 : 2];
	} else if (3 * ady <= (adx << 1)) {
		dir += DeltaDir[5 * ady < adx ? 0 : 1];
	}// else
	//	dir += DeltaDir[2];
	return dir & 15;
#endif
}

void DeleteMissile(int mi, int idx)
{
	nummissiles--;
	assert(missileactive[idx] == mi);
	missileactive[idx] = missileactive[nummissiles];
	missileactive[nummissiles] = mi;

}

static void PutMissile(int mi)
{
	int x, y;

	x = missile[mi]._mix;
	y = missile[mi]._miy;
	assert(IN_DUNGEON_AREA(x, y));
	if (!missile[mi]._miDelFlag) {
		dMissile[x][y] = dMissile[x][y] == 0 ? mi + 1 : MIS_MULTI;
		assert(!missile[mi]._miPreFlag);
		//if (missile[mi]._miPreFlag)
		//	dFlags[x][y] |= BFLAG_MISSILE_PRE;
	}
}

static void PutMissileF(int mi, BYTE flag)
{
	int x, y;

	x = missile[mi]._mix;
	y = missile[mi]._miy;
	assert(IN_DUNGEON_AREA(x, y));
	assert(!missile[mi]._miDelFlag);
	//if (!missile[mi]._miDelFlag) {
		dMissile[x][y] = dMissile[x][y] == 0 ? mi + 1 : MIS_MULTI;
		dFlags[x][y] |= flag;
	//}
}

static void GetMissileVel(int mi, int sx, int sy, int dx, int dy, int v)
{
	double dxp, dyp, dr;

	dx -= sx;
	dy -= sy;
	assert(dx != 0 || dy != 0);
	dxp = (dx - dy);
	dyp = (dy + dx);
	dr = sqrt(dxp * dxp + dyp * dyp);
	missile[mi]._mixvel = (dxp * (v << MIS_BASE_VELO_SHIFT)) / dr;
	missile[mi]._miyvel = (dyp * (v << MIS_BASE_VELO_SHIFT)) / dr;
}

static void GetMissilePos(int mi)
{
	MissileStruct *mis;
	int mx, my, dx, dy, lx, ly;

	mis = &missile[mi];
	mx = mis->_mitxoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);
	my = mis->_mityoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);

	dx = mx + my;
	dy = my - mx;
	lx = dx / 8;
	dx = dx / 64;
	ly = dy / 8;
	dy = dy / 64;

	mis->_mix = dx + mis->_misx;
	mis->_miy = dy + mis->_misy;
	mis->_mixoff = (mx + (dy * 32) - (dx * 32)) * ASSET_MPL;
	mis->_miyoff = ((my >> 1) - (dx * 16) - (dy * 16)) * ASSET_MPL;
	ChangeLightOff(mis->_miLid, lx - (dx * 8), ly - (dy * 8));
}

/**
 * Shift the missiles coordinates to place it in front of the view.
 * @param mi the missile to be shifted
 */
static void ShiftMissilePos(int mi)
{
	MissileStruct *mis;
	int dx, dy, x, y;

	mis = &missile[mi];
	switch (mis->_miDir) {
	case DIR_S:
	case DIR_SW:
	case DIR_SE:
		dx = 1;
		dy = 1;
		break;
	case DIR_W:
		dx = 0;
		dy = 1;
		break;
	case DIR_NW:
	case DIR_N:
	case DIR_NE:
		return;
		//dx = 0;
		//dy = 0;
		//break;
	case DIR_E:
		dx = 1;
		dy = 0;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	x = mis->_mix + dx;
	y = mis->_miy + dy;
	if (PosOkActor(x, y)) {
		mis->_mix += dx;
		mis->_miy += dy;
		mis->_mixoff += ((dy * 32) - (dx * 32)) * ASSET_MPL;
		mis->_miyoff -= ((dy * 16) + (dx * 16)) * ASSET_MPL;
	}
}

unsigned CalcMonsterDam(uint16_t mor, BYTE mRes, unsigned mindam, unsigned maxdam, bool penetrates)
{
	unsigned dam;
	BYTE resist;

	switch (mRes) {
	case MISR_NONE:
		resist = MORT_NONE;
		break;
	case MISR_SLASH:
		mor &= MORS_SLASH_IMMUNE;
		resist = mor >> MORS_IDX_SLASH;
		break;
	case MISR_BLUNT:
		mor &= MORS_BLUNT_IMMUNE;
		resist = mor >> MORS_IDX_BLUNT;
		break;
	case MISR_PUNCTURE:
		mor &= MORS_PUNCTURE_IMMUNE;
		resist = mor >> MORS_IDX_PUNCTURE;
		break;
	case MISR_FIRE:
		mor &= MORS_FIRE_IMMUNE;
		resist = mor >> MORS_IDX_FIRE;
		break;
	case MISR_LIGHTNING:
		mor &= MORS_LIGHTNING_IMMUNE;
		resist = mor >> MORS_IDX_LIGHTNING;
		break;
	case MISR_MAGIC:
		mor &= MORS_MAGIC_IMMUNE;
		resist = mor >> MORS_IDX_MAGIC;
		break;
	case MISR_ACID:
		mor &= MORS_ACID_IMMUNE;
		resist = mor >> MORS_IDX_ACID;
		break;
	default:
		ASSUME_UNREACHABLE
		resist = MORT_NONE;
		break;
	}
	dam = RandRange(mindam, maxdam);
	switch (resist) {
	case MORT_NONE:
		break;
	case MORT_PROTECTED:
		if (!penetrates) {
			dam >>= 1;
			dam += dam >> 2;
		}
		break;
	case MORT_RESIST:
		dam >>= penetrates ? 1 : 2;
		break;
	case MORT_IMMUNE:
		dam = 0;
		break;
	default: ASSUME_UNREACHABLE;
	}
	return dam;
}

void AddElementalExplosion(int dx, int dy, int fdam, int ldam, int mdam, int adam)
{
	int mtype;

	if ((fdam | ldam) >= (mdam | adam)) {
		mtype = fdam >= ldam ? MIS_EXFIRE : MIS_EXLGHT;
	} else {
		mtype = mdam >= adam ? MIS_EXMAGIC : MIS_EXACID;
	}
	AddMissile(dx, dy, -1, 0, 0, mtype, MST_NA, 0, 0);
	/*int gfx = random_(8, dam);
	if (gfx >= dam - (fdam + ldam)) {
		if (gfx < dam - ldam) {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPFEXP, MST_NA, 0);
		} else {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPLEXP, MST_NA, 0);
		}
	}*/
}

static bool MonsterTrapHit(int mnum, int mi)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	int hper, dam;
	bool ret;

	mon = &monsters[mnum];
	mis = &missile[mi];
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = GetRndSeed();
	if (mis->_miSubType == 0) {
		hper = 100 + (2 * currLvl._dLevel)
		    - mon->_mArmorClass;
		hper -= mis->_miDist << 1;
	} else {
		hper = 40;
	}
	if (!CheckHit(hper) && mon->_mmode != MM_STONE)
#if DEBUG_MODE
		if (!debug_mode_god_mode)
#endif
			return false;

	dam = CalcMonsterDam(mon->_mMagicRes, mis->_miResist, mis->_miMinDam, mis->_miMaxDam, false);
	if (dam == 0)
		return false;

	if (CheckMonsterHit(mnum, &ret)) {
		return ret;
	}

	mon->_mhitpoints -= dam;
#if DEBUG_MODE
	if (debug_mode_god_mode)
		mon->_mhitpoints = 0;
#endif
	if (mon->_mhitpoints < (1 << 6)) {
		MonStartKill(mnum, -1);
	} else {
		/*if (resist != MORT_NONE) {
			PlayEffect(mnum, MS_GOTHIT);
		} else {*/
			MonStartHit(mnum, -1, dam, 0);
		//}
	}
	if (mon->_msquelch != SQUELCH_MAX) {
		mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
		// lead the monster to a fixed location
		mon->_lastx = mis->_misx;
		mon->_lasty = mis->_misy;
	}
	return true;
}

static bool MonsterMHit(int mnum, int mi)
{
	MonsterStruct* mon;
	MissileStruct* mis;
	int pnum, hper, dam, lx, ly;
	unsigned hitFlags;
	bool tmac, ret;

	mon = &monsters[mnum];
	mis = &missile[mi];
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = GetRndSeed();
	pnum = mis->_miSource;
	//assert((unsigned)pnum < MAX_PLRS);
	if (mis->_miSubType == 0) {
		hper = plr._pIHitChance - mon->_mArmorClass
		    - (mis->_miDist * mis->_miDist >> 1);
	} else {
		if (mis->_miFlags & MIF_AREA) {
			hper = 40
				+ (plr._pLevel << 1)
				- (mon->_mLevel << 1);
		} else {
			hper = 50 + plr._pMagic
				- (mon->_mLevel << 1)
				- mon->_mEvasion
				/*- dist*/; // TODO: either don't care about it, or set it!
		}
	}
	if (!CheckHit(hper) && mon->_mmode != MM_STONE)
#if DEBUG_MODE
		if (!debug_mode_god_mode)
#endif
			return false;

	if (mis->_miSubType == 0) {
		// calculcate arrow-damage
		dam = 0;
		tmac = (plr._pIFlags & ISPL_PENETRATE_PHYS) != 0;
		int sldam = plr._pISlMaxDam;
		if (sldam != 0) {
			dam += CalcMonsterDam(mon->_mMagicRes, MISR_SLASH, plr._pISlMinDam, sldam, tmac);
		}
		int bldam = plr._pIBlMaxDam;
		if (bldam != 0) {
			dam += CalcMonsterDam(mon->_mMagicRes, MISR_BLUNT, plr._pIBlMinDam, bldam, tmac);
		}
		int pcdam = plr._pIPcMaxDam;
		if (pcdam != 0) {
			dam += CalcMonsterDam(mon->_mMagicRes, MISR_PUNCTURE, plr._pIPcMinDam, pcdam, tmac);
		}
		if (random_(6, 200) < plr._pICritChance) {
			dam <<= 1;
		}
		// add modifiers from arrow-type
		if (mis->_miType == MIS_PBARROW) {
			dam = (dam * (64 + 32 - 16 * mis->_miDist + mis->_miSpllvl)) >> 6;
		} else if (mis->_miType == MIS_ASARROW) {
			dam = (dam * (8 * mis->_miDist - 16 + mis->_miSpllvl)) >> 5;
		}
		int fdam = plr._pIFMaxDam;
		if (fdam != 0) {
			fdam = CalcMonsterDam(mon->_mMagicRes, MISR_FIRE, plr._pIFMinDam, fdam, false);
		}
		int ldam = plr._pILMaxDam;
		if (ldam != 0) {
			ldam = CalcMonsterDam(mon->_mMagicRes, MISR_LIGHTNING, plr._pILMinDam, ldam, false);
		}
		int mdam = plr._pIMMaxDam;
		if (mdam != 0) {
			mdam = CalcMonsterDam(mon->_mMagicRes, MISR_MAGIC, plr._pIMMinDam, mdam, false);
		}
		int adam = plr._pIAMaxDam;
		if (adam != 0) {
			adam = CalcMonsterDam(mon->_mMagicRes, MISR_ACID, plr._pIAMinDam, adam, false);
		}
		if ((ldam | fdam | mdam | adam) != 0) {
			dam += fdam + ldam + mdam + adam;
			AddElementalExplosion(mon->_mx, mon->_my, fdam, ldam, mdam, adam);
		}
	} else {
		dam = CalcMonsterDam(mon->_mMagicRes, mis->_miResist, mis->_miMinDam, mis->_miMaxDam, false);
	}
	if (dam == 0)
		return false;

	if (CheckMonsterHit(mnum, &ret))
		return ret;

	//if (pnum == mypnum) {
		mon->_mhitpoints -= dam;
	//}
	if (mis->_miSubType == 0) {
		if (plr._pILifeSteal != 0) {
			PlrIncHp(pnum, (dam * plr._pILifeSteal) >> 7);
		}
		if (plr._pIManaSteal != 0) {
			PlrIncMana(pnum, (dam * plr._pIManaSteal) >> 7);
		}
	}

	if (mon->_mhitpoints < (1 << 6)) {
		MonStartKill(mnum, pnum);
	} else {
		/*if (resist != MORT_NONE) {
			PlayEffect(mnum, MS_GOTHIT);
		} else {*/
			hitFlags = plr._pIFlags;
			if (mis->_miSubType == 0) {
				//if (hitFlags & ISPL_NOHEALMON)
				//	mon->_mFlags |= MFLAG_NOHEAL;

				if (hitFlags & ISPL_KNOCKBACK)
					MonGetKnockback(mnum, mis->_misx, mis->_misy);
			}
			MonStartHit(mnum, pnum, dam, hitFlags);
		//}
	}

	if (mon->_msquelch != SQUELCH_MAX) {
		mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
		// lead the monster to the player
		lx = mis->_misx;
		ly = mis->_misy;
		if (mis->_miFlags & MIF_LEAD) {
#if HELLFIRE
			if (mis->_miCaster == MST_PLAYER) // only if the missile is not from a rune
#endif
			{
				lx = plr._px;
				ly = plr._py;
			}
		}
		mon->_lastx = lx;
		mon->_lasty = ly;
	}
	return true;
}

unsigned CalcPlrDam(int pnum, BYTE mRes, unsigned mindam, unsigned maxdam)
{
	int dam;
	char resist;

	switch (mRes) {
	case MISR_NONE:
	case MISR_SLASH:  // TODO: add plr._pSlash/Blunt/PunctureResist
	case MISR_BLUNT:
	case MISR_PUNCTURE:
		resist = 0;
		break;
	case MISR_FIRE:
		resist = plr._pFireResist;
		break;
	case MISR_LIGHTNING:
		resist = plr._pLghtResist;
		break;
	case MISR_MAGIC:
		resist = plr._pMagResist;
		break;
	case MISR_ACID:
		resist = plr._pAcidResist;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	dam = RandRange(mindam, maxdam);
	if (resist != 0)
		dam -= dam * resist / 100;
	return dam;
}

static bool PlayerTrapHit(int pnum, int mi)
{
	MissileStruct *mis;
	int hper, tmp, dam;

	if (plr._pInvincible) {
		return false;
	}

	mis = &missile[mi];
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = GetRndSeed();
	if (mis->_miSubType == 0) {
		hper = 100 + (2 * currLvl._dLevel)
		    + (2 * currLvl._dLevel)
		    - plr._pIAC;
		hper -= mis->_miDist << 1;
	} else {
		hper = 40
			+ (2 * currLvl._dLevel)
			- (2 * plr._pLevel);
	}

	if (!CheckHit(hper))
#if DEBUG_MODE
		if (!debug_mode_god_mode)
#endif
			return false;

	if (!(mis->_miFlags & MIF_NOBLOCK)) {
		tmp = plr._pIBlockChance;
		if (tmp != 0 && (plr._pmode == PM_STAND || plr._pmode == PM_BLOCK)) {
			// assert(plr._pSkillFlags & SFLAG_BLOCK);
			tmp = tmp - (currLvl._dLevel << 1);
			if (tmp > random_(73, 100)) {
				tmp = GetDirection(plr._px, plr._py, mis->_misx, mis->_misy);
				PlrStartBlock(pnum, tmp);
				return true;
			}
		}
	}

	dam = CalcPlrDam(pnum, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
	if (dam == 0)
		return false;

	tmp = DIR_NONE;
	if (!(mis->_miFlags & MIF_DOT)) {
		dam += plr._pIGetHit;
		if (dam < 64)
			dam = 64;
		tmp = GetDirection(mis->_misx, mis->_misy, plr._px, plr._py);
	}

	if (!PlrDecHp(pnum, dam, DMGTYPE_NPC))
		StartPlrHit(pnum, dam, false, tmp);
	return true;
}

static bool PlayerMHit(int pnum, int mi)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	int hper, tmp, dam;

	if (plr._pInvincible) {
		return false;
	}
	mis = &missile[mi];
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = GetRndSeed();
	mon = &monsters[mis->_miSource];
	if (mis->_miSubType == 0) {
		hper = 30 + mon->_mHit
		    + (2 * mon->_mLevel)
		    - plr._pIAC;
		hper -= mis->_miDist << 1;
	} else {
		if (mis->_miFlags & MIF_AREA) {
			hper = 40
				+ (2 * mon->_mLevel)
				- (2 * plr._pLevel);
		} else {
			hper = 50 + mon->_mMagic
				- plr._pIEvasion
				/*- dist*/; // TODO: either don't care about it, or set it!
		}
	}

	if (!CheckHit(hper))
#if DEBUG_MODE
		if (!debug_mode_god_mode)
#endif
			return false;

	if (!(mis->_miFlags & MIF_NOBLOCK)) {
		tmp = plr._pIBlockChance;
		if (tmp != 0 && (plr._pmode == PM_STAND || plr._pmode == PM_BLOCK)) {
			// assert(plr._pSkillFlags & SFLAG_BLOCK);
			tmp = tmp - (mon->_mLevel << 1);
			if (tmp > random_(73, 100)) {
				tmp = GetDirection(plr._px, plr._py, mis->_misx, mis->_misy);
				PlrStartBlock(pnum, tmp);
				return true;
			}
		}
	}

	dam = CalcPlrDam(pnum, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
	if (dam == 0)
		return false;
	tmp = DIR_NONE;
	if (!(mis->_miFlags & MIF_DOT)) {
		dam += plr._pIGetHit;
		if (dam < 64)
			dam = 64;
		tmp = GetDirection(mis->_misx, mis->_misy, plr._px, plr._py);
	}

	if (!PlrDecHp(pnum, dam, DMGTYPE_NPC))
		StartPlrHit(pnum, dam, false, tmp);
	return true;
}

static bool Plr2PlrMHit(int pnum, int mi)
{
	MissileStruct* mis;
	int offp, dam, tmp, hper;

	mis = &missile[mi];
	offp = mis->_miSource;
	if (plr._pTeam == plx(offp)._pTeam || plr._pInvincible) {
		return false;
	}
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = GetRndSeed();
	if (mis->_miSubType == 0) {
		hper = plx(offp)._pIHitChance
		    - plr._pIAC;
		hper -= (mis->_miDist * mis->_miDist >> 1);
	} else {
		if (mis->_miFlags & MIF_AREA) {
			hper = 40
				+ (2 * plx(offp)._pLevel)
				- (2 * plr._pLevel);
		} else {
			hper = 50 + plx(offp)._pMagic
				- plr._pIEvasion
				/*- dist*/; // TODO: either don't care about it, or set it!
		}
	}
	if (!CheckHit(hper))
		return false;

	if (!(mis->_miFlags & MIF_NOBLOCK)) {
		tmp = plr._pIBlockChance;
		if (tmp != 0 && (plr._pmode == PM_STAND || plr._pmode == PM_BLOCK)) {
			// assert(plr._pSkillFlags & SFLAG_BLOCK);
			tmp = tmp - (plx(offp)._pLevel << 1);
			if (tmp > random_(73, 100)) {
				tmp = GetDirection(plr._px, plr._py, mis->_misx, mis->_misy);
				PlrStartBlock(pnum, tmp);
				return true;
			}
		}
	}

	if (mis->_miSubType == 0) {
		dam = 0;
		int sldam = plx(offp)._pISlMaxDam;
		if (sldam != 0) {
			dam += CalcPlrDam(pnum, MISR_SLASH, plx(offp)._pISlMinDam, sldam);
		}
		int bldam = plx(offp)._pIBlMaxDam;
		if (bldam != 0) {
			dam += CalcPlrDam(pnum, MISR_BLUNT, plx(offp)._pIBlMinDam, bldam);
		}
		int pcdam = plx(offp)._pIPcMaxDam;
		if (pcdam != 0) {
			dam += CalcPlrDam(pnum, MISR_PUNCTURE, plx(offp)._pIPcMinDam, pcdam);
		}
		if (random_(6, 200) < plx(offp)._pICritChance) {
			dam <<= 1;
		}
		// add modifiers from arrow-type
		if (mis->_miType == MIS_PBARROW) {
			dam = (dam * (64 + 32 - 16 * mis->_miDist + mis->_miSpllvl)) >> 6;
		} else if (mis->_miType == MIS_ASARROW) {
			dam = (dam * (8 * mis->_miDist - 16 + mis->_miSpllvl)) >> 5;
		}

		if (plx(offp)._pILifeSteal != 0) {
			PlrIncHp(offp, (dam * plx(offp)._pILifeSteal) >> 7);
		}

		int fdam = plx(offp)._pIFMaxDam;
		if (fdam != 0) {
			fdam = CalcPlrDam(pnum, MISR_FIRE, plx(offp)._pIFMinDam, fdam);
		}
		int ldam = plx(offp)._pILMaxDam;
		if (ldam != 0) {
			ldam = CalcPlrDam(pnum, MISR_LIGHTNING, plx(offp)._pILMinDam, ldam);
		}
		int mdam = plx(offp)._pIMMaxDam;
		if (mdam != 0) {
			mdam = CalcPlrDam(pnum, MISR_MAGIC, plx(offp)._pIMMinDam, mdam);
		}
		int adam = plx(offp)._pIAMaxDam;
		if (adam != 0) {
			adam = CalcPlrDam(pnum, MISR_ACID, plx(offp)._pIAMinDam, adam);
		}
		if ((ldam | fdam | mdam | adam) != 0) {
			dam += ldam + fdam + mdam + adam;
			AddElementalExplosion(plr._px, plr._py, fdam, ldam, mdam, adam);
		}
	} else {
		dam = CalcPlrDam(pnum, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
		dam >>= 1;
	}

	if (dam == 0)
		return false;

	tmp = DIR_NONE;
	if (!(mis->_miFlags & MIF_DOT)) {
		dam += plr._pIGetHit;
		if (dam < 64)
			dam = 64;
		tmp = GetDirection(mis->_misx, mis->_misy, plr._px, plr._py);
	}

	if (!PlrDecHp(pnum, dam, DMGTYPE_PLAYER))
		StartPlrHit(pnum, dam, false, tmp);
	return true;
}

static bool MonMissHit(int mnum, int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miCaster & MST_PLAYER) {
		// player vs. monster
		return MonsterMHit(mnum, mi);
	} else if (mis->_miCaster == MST_MONSTER) {
		// monster vs. golem
		return mnum < MAX_MINIONS && MonsterTrapHit(mnum, mi);
	} else {
		// trap vs. monster
		return MonsterTrapHit(mnum, mi);
	}
}

static bool PlrMissHit(int pnum, int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miCaster & MST_PLAYER) {
		// player vs. player
		return Plr2PlrMHit(pnum, mi);
	} else if (mis->_miCaster == MST_MONSTER) {
		// monster vs. player
		return PlayerMHit(pnum, mi);
	} else {
		// trap vs. player
		return PlayerTrapHit(pnum, mi);
	}
}

/**
 * Check if the monster is on a given tile.
 */
int CheckMonCol(int mnum)
{
	MonsterStruct *mon;
	int mode;
	bool negate;
	bool halfOver;

	if (mnum >= 0) {
		// assert(mnum != 0);
		mnum--;
		negate = true;
	} else {
		mnum = -(mnum + 1);
		negate = false;
	}

	mon = &monsters[mnum];
	mode = mon->_mmode;
	static_assert(MM_WALK + 1 == MM_WALK2, "CheckMonCol expects ordered MM_WALKs.");
	if (mode > MM_WALK2 || mode < MM_WALK)
		return (negate || mode == MM_STONE) ? mnum : -1;
	halfOver = mon->_mAnimFrame > (mon->_mAnims[MA_WALK].aFrames >> 1);
	if (mode == MM_WALK) {
		if (negate)
			halfOver = !halfOver;
		return halfOver ? mnum : -1;
	}
	// assert(mode == MM_WALK2);
	if (negate)
		halfOver = !halfOver;
	return halfOver ? -1 : mnum;
}

int CheckPlrCol(int pnum)
{
	int mode;
	bool negate;
	bool halfOver;

	if (pnum >= 0) {
		// assert(pnum != 0);
		pnum--;
		negate = true;
	} else {
		pnum = -(pnum + 1);
		negate = false;
	}
	mode = plr._pmode;
	static_assert(PM_WALK + 1 == PM_WALK2, "CheckPlrCol expects ordered PM_WALKs.");
	if (mode > PM_WALK2 || mode < PM_WALK)
		return negate ? pnum : -1;
	halfOver = plr._pAnimFrame >= (plr._pWFrames >> 1);
	if (mode == PM_WALK) {
		if (negate)
			halfOver = !halfOver;
		return halfOver ? pnum : -1;
	}
	// assert(mode == PM_WALK2);
	if (negate)
		halfOver = !halfOver;
	return halfOver ? -1 : pnum;
}

/*
 * @param mi: index of the missile
 * @param mx: the x coordinate of the target
 * @param my: the y coordinate of the target
 * @param mode: the collision mode (missile_collision_mode)
 */
static bool CheckMissileCol(int mi, int mx, int my, missile_collision_mode mode)
{
	MissileStruct* mis;
	const MissileData* mds;
	int oi, mnum, pnum;
	int hit = 0;
	bool result;

	mnum = dMonster[mx][my];
	if (mnum != 0) {
		mnum = CheckMonCol(mnum);
		if (mnum != -1 && MonMissHit(mnum, mi))
			hit = 1;
	}

	pnum = dPlayer[mx][my];
	if (pnum != 0) {
		pnum = CheckPlrCol(pnum);
		if (pnum != -1 && PlrMissHit(pnum, mi))
			hit = 1;
	}

	oi = dObject[mx][my];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (!objects[oi]._oMissFlag) {
			if (objects[oi]._oBreak == OBM_BREAKABLE)
				OperateObject(-1, oi, false);
			hit = 2;
		}
	}
	if (nMissileTable[dPiece[mx][my]]) {
		hit = 2;
	}

	if (hit == 0)
		return false;

	result = hit == 1;
	if (mode != MICM_NONE) {
		mis = &missile[mi];
		if (mode == MICM_BLOCK_ANY || (!result /*&& mode == MICM_BLOCK_WALL*/))
			mis->_miRange = -1;
		mds = &missiledata[mis->_miType];
		if (mds->miSFX != SFX_NONE)
			PlaySfxLoc(mds->miSFX, mis->_mix, mis->_miy, mds->miSFXCnt);
	}
	return result;
}

static void CheckSplashColFull(int mi)
{
	MissileStruct* mis;
	int i, mx, my;

	mis = &missile[mi];
	mx = mis->_mix;
	my = mis->_miy;

	// monster/player/object hit -> hit everything around
	for (i = 0; i < lengthof(XDirAdd); i++) {
		CheckMissileCol(mi, mx + XDirAdd[i], my + YDirAdd[i], MICM_NONE);
	}
}

static void CheckSplashCol(int mi)
{
	MissileStruct* mis;
	int i, mx, my, lx, ly, tx, ty;

	mis = &missile[mi];
	mx = mis->_mix;
	my = mis->_miy;
	if (!nMissileTable[dPiece[mx][my]]) {
		CheckSplashColFull(mi);
		return;
	}

	// wall hit:
	//  1. move missile back a bit to indicate the displacement
	mis->_mitxoff -= mis->_mixvel;
	mis->_mityoff -= mis->_miyvel;
	GetMissilePos(mi);

	//  2. limit the explosion area
	lx = mis->_mix;
	ly = mis->_miy;

	//  3. alter offset for better visual
	if (mis->_mixoff >= TILE_WIDTH / 2) {
		mis->_mixoff -= TILE_WIDTH;
		mis->_mix++;
		mis->_miy--;
	}

	//mis->_mitxoff += mis->_mixvel;
	//mis->_mityoff += mis->_miyvel;
	//GetMissilePos(mi);

	// assert(lx != mx || ly != my);
	for (i = 0; i < lengthof(XDirAdd); i++) {
		tx = mx + XDirAdd[i];
		ty = my + YDirAdd[i];
		if (abs(tx - lx) <= 1 && abs(ty - ly) <= 1)
			CheckMissileCol(mi, tx, ty, MICM_NONE);
	}
}

static void SyncMissAnim(int mi)
{
	MissileStruct *mis;
	const MisFileData *mfd;
	int dir, animtype;

	mis = &missile[mi];
	animtype = mis->_miAnimType;
	dir = mis->_miDir;
	mis->_miAnimData = misanimdata[animtype][dir];
	mfd = &misfiledata[animtype];
	mis->_miAnimFlag = (mfd->mfFlags & MAFLAG_LOCK_ANIMATION) == 0;
	mis->_miAnimFrameLen = mfd->mfAnimFrameLen[dir];
	mis->_miAnimLen = mfd->mfAnimLen[dir];
	mis->_miAnimWidth = mfd->mfAnimWidth * ASSET_MPL;
	mis->_miAnimXOffset = mfd->mfAnimXOffset * ASSET_MPL;
}

static void SyncRhinoAnim(int mi)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	AnimStruct* anim;

	mis = &missile[mi];
	mon = &monsters[mis->_miSource];
	anim = &mon->_mAnims[
		(mon->_mType >= MT_HORNED && mon->_mType <= MT_OBLORD) ? MA_SPECIAL :
		(mon->_mType >= MT_NSNAKE && mon->_mType <= MT_GSNAKE) ? MA_ATTACK : MA_WALK];
	mis->_miAnimData = anim->aData[mis->_miDir];
	mis->_miAnimFrameLen = anim->aFrameLen;
	assert(mis->_miAnimFlag == TRUE);
	mis->_miAnimLen = anim->aFrames;
	mis->_miAnimWidth = mon->_mAnimWidth;
	mis->_miAnimXOffset = mon->_mAnimXOffset;

	mis->_miAnimAdd = mon->_mType >= MT_NSNAKE && mon->_mType <= MT_GSNAKE ? 2 : 1;
	mis->_miLid = mon->mlid;
	if (mon->_uniqtype != 0) {
		mis->_miUniqTrans = mon->_uniqtrans;
		//mis->_miLid = mon->mlid;
	}
}

static void SyncChargeAnim(int mi)
{
	MissileStruct* mis;
	int pnum;

	mis = &missile[mi];
	pnum = mis->_miSource;

	mis->_miAnimData = plr._pWAnim[mis->_miDir];
	mis->_miAnimFrameLen = PlrAnimFrameLens[PA_WALK];
	assert(mis->_miAnimFlag == TRUE);
	mis->_miAnimLen = plr._pWFrames;
	mis->_miAnimWidth = plr._pWWidth;
	mis->_miAnimXOffset = (plr._pWWidth - TILE_WIDTH) >> 1;
	mis->_miLid = plr._plid;
}

static void SetMissDir(int mi, int dir)
{
	missile[mi]._miDir = dir;
	// assert(gbGameLogicProgress < GLP_MISSILES_DONE);
	missile[mi]._miAnimCnt = -1;
	missile[mi]._miAnimFrame = 1;
	SyncMissAnim(mi);
}

void LoadMissileGFX(BYTE midx)
{
	char pszName[DATA_ARCHIVE_MAX_PATH];
	int i, n;
	BYTE **mad, *tf, *cf;
	const char *name;
	const MisFileData* mfd;

	mad = misanimdata[midx];
	if (mad[0] != NULL)
		return;
	mfd = &misfiledata[midx];
	n = mfd->mfAnimFAmt;
	name = mfd->mfName;
	if (n == 1) {
		snprintf(pszName, sizeof(pszName), "Missiles\\%s.CL2", name);
		assert(mad[0] == NULL);
		mad[0] = LoadFileInMem(pszName);
	} else {
		for (i = 0; i < n; i++) {
			snprintf(pszName, sizeof(pszName), "Missiles\\%s%d.CL2", name, i + 1);
			assert(mad[i] == NULL);
			mad[i] = LoadFileInMem(pszName);
		}
	}
	if (mfd->mfAnimTrans != NULL) {
		// TODO: copy paste from monster.cpp (InitMonsterTRN)
		tf = cf = LoadFileInMem(mfd->mfAnimTrans);
		for (i = 0; i < 256; i++) {
			if (*cf == 255) {
				*cf = 0;
			}
			cf++;
		}

		for (i = 0; i < n; i++) {
			Cl2ApplyTrans(mad[i], tf, mfd->mfAnimLen[i]);
		}
		mem_free_dbg(tf);
	}
}

void InitGameMissileGFX()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (!(misfiledata[i].mfFlags & MAFLAG_HIDDEN))
			LoadMissileGFX(i);
	}
}

static void FreeMissileGFX(int midx, int n)
{
	BYTE** mad;
	int i;

	mad = misanimdata[midx];
	for (i = 0; i < n; i++) {
		if (mad[i] != NULL) {
			MemFreeDbg(mad[i]);
		}
	}
}

void FreeGameMissileGFX()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (!(misfiledata[i].mfFlags & MAFLAG_HIDDEN))
			FreeMissileGFX(i, misfiledata[i].mfAnimFAmt);
	}
}

void FreeMonMissileGFX()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (misfiledata[i].mfFlags & MAFLAG_HIDDEN)
			FreeMissileGFX(i, misfiledata[i].mfAnimFAmt);
	}
}

void InitMissiles()
{
	int i;
	BYTE* pTmp;

	nummissiles = 0;

	//memset(missile, 0, sizeof(missile));
	for (i = 0; i < MAXMISSILES; i++) {
		missileactive[i] = i;
	}
	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in InitMissiles.");
	pTmp = &dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++)
		assert((*pTmp & (BFLAG_MISSILE_PRE | BFLAG_HAZARD)) == 0);
}

#ifdef HELLFIRE
static bool PlaceRune(int mi, int dx, int dy, int mitype, int mirange)
{
	int i, j, tx, ty;
	const char* cr;
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_miVar1 = mitype;
	mis->_miVar2 = mirange;     // trigger range
	mis->_miVar3 = 16;          // delay
	if (mis->_miCaster & MST_PLAYER) {
		mis->_miCaster |= MST_RUNE;
		mis->_miSpllvl += plx(mis->_miSource)._pDexterity >> 3;
	}
	mis->_miRange = 16 + 1584;	// delay + ttl

	for (i = 0; i < 10; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = *cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkMissile(tx, ty)) {
				mis->_mix = tx;
				mis->_miy = ty;
				static_assert(MAX_LIGHT_RAD >= 8, "PlaceRune needs at least light-radius of 8.");
				mis->_miLid = AddLight(tx, ty, 8);
				return true;
			}
		}
	}
	return false;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (PlaceRune(mi, dx, dy, MIS_FIREEXP, 0))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (PlaceRune(mi, dx, dy, MIS_LIGHTNINGC, 1))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddNovaRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (PlaceRune(mi, dx, dy, MIS_LIGHTNOVAC, 1))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddWaveRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (PlaceRune(mi, dx, dy, MIS_FIREWAVEC, 1))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (PlaceRune(mi, dx, dy, MIS_STONE, 0))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

int AddHorkSpawn(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(8));
	// missile[mi]._miMinDam = missile[mi]._miMaxDam = 0;
	missile[mi]._miRange = 9 - 1;
	//PutMissile(mi);
	return MIRES_DONE;
}

/*int AddLightwall(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	assert((unsigned)misource < MAX_PLRS);
	//if (misource != -1) {
		// TODO: bring it closer to AddFirewall? (_pISplDur, adjust damage)
		mis->_miMaxDam = ((plx(misource)._pMagic >> 1) + spllvl) << (-3 + 6);
	//} else {
	//	mis->_miMaxDam = (20 + currLvl._dLevel) << (-2 + 6);
	//}
	mis->_miMinDam = 1 << (-5 + 6);
	mis->_miRange = 255 * (spllvl + 1);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	return MIRES_DONE;
}*/

int AddFireexp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam, dam;

	mis = &missile[mi];
	mis->_miRange = misfiledata[MFILE_BIGEXP].mfAnimLen[0] * misfiledata[MFILE_BIGEXP].mfAnimFrameLen[0];

	if (misource != -1) {
		assert((unsigned)misource < MAX_PLRS);
		mindam = 1 + (plx(misource)._pMagic >> 1) + 16 * spllvl;
		maxdam = 1 + (plx(misource)._pMagic >> 1) + 32 * spllvl;
		dam = RandRange(mindam, maxdam);
	} else {
		dam = currLvl._dLevel;
	}
	dam <<= 6;
	mis->_miMinDam = mis->_miMaxDam = dam;
	CheckMissileCol(mi, sx, sy, MICM_NONE);
	// assert(mis->_mix == sx);
	// assert(mis->_miy == sy);
	// assert(!nMissileTable[dPiece[sx][sy]]);
	CheckSplashColFull(mi);
	return MIRES_DONE;
}

/**
 * Remark: expects damage to be shifted!
 *
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
/*int AddFireball2(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int av = MIS_SHIFTEDVEL(16);

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	assert((unsigned)misource < MAX_PLRS);
	av += spllvl;
	//if (av > MIS_SHIFTEDVEL(50)) {
	//	av = MIS_SHIFTEDVEL(50);
	//}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	static_assert(MAX_LIGHT_RAD >= 8, "AddFireball2 needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 1;
	return MIRES_DONE;
}*/

int AddRingC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int tx, ty, j, pn, mitype;
	const char* cr;

	mitype = MIS_FIREWALL; //mis->_miType == MIS_FIRERING ? MIS_FIREWALL : MIS_LIGHTWALL;

	static_assert(DBORDERX >= 3 && DBORDERY >= 3, "AddRingC expects a large enough border.");
	cr = &CrawlTable[CrawlNum[3]];
	for (j = *cr; j > 0; j--) {
		tx = sx + *++cr;
		ty = sy + *++cr;
		assert(IN_DUNGEON_AREA(tx, ty));
		pn = dPiece[tx][ty];
		if ((nSolidTable[pn] | dObject[tx][ty]) == 0) {
			if (LineClear(sx, sy, tx, ty)) {
				if (nMissileTable[pn])
					break;
				else
					AddMissile(tx, ty, 0, 0, 0, mitype, micaster, misource, spllvl);
			}
		}
	}

	return MIRES_DELETE;
}
#endif

/*
 * Var1: x coordinate of the missile-target of MIS_ASARROW
 * Var2: y coordinate of the missile-target of MIS_ASARROW
 */
int AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int av = MIS_SHIFTEDVEL(32), mtype;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	midir = GetDirection16(sx, sy, dx, dy);
	mtype = MFILE_ARROWS;
	if (micaster & MST_PLAYER) {
		av += MIS_SHIFTEDVEL((int)plx(misource)._pIArrowVelBonus);
		//int dam = plx(misource)._pIMaxDam + plx(misource)._pIMinDam;
		int fdam = plx(misource)._pIFMaxDam;
		int ldam = plx(misource)._pILMaxDam;
		int mdam = plx(misource)._pIMMaxDam;
		int adam = plx(misource)._pIAMaxDam;
		if ((ldam | fdam | mdam | adam) != 0) {
			// TODO: use random?
			if ((fdam | ldam) >= (mdam | adam)) {
				mtype = fdam >= ldam ? MFILE_FARROW : MFILE_LARROW;
			} else {
				mtype = mdam >= adam ? MFILE_MARROW : MFILE_PARROW;
			}
		}
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	mis = &missile[mi];
	if (mtype == MFILE_ARROWS) {
		mis->_miAnimFrame = midir + 1;
	} else {
		mis->_miAnimType = mtype;
		SetMissDir(mi, midir);
	}
	mis->_miRange = 255;
	if (micaster & MST_PLAYER) {
		// mis->_miMinDam = plx(misource)._pIPcMinDam;
		// mis->_miMaxDam = plx(misource)._pIPcMaxDam;
		if (mis->_miType == MIS_PBARROW)
			mis->_miRange = 4;
		else if (mis->_miType == MIS_ASARROW) {
			if (!LineClear(sx, sy, dx, dy))
				return MIRES_FAIL_DELETE;
			mis->_miVar1 = dx;
			mis->_miVar2 = dy;
		}
	} else if (micaster == MST_MONSTER) {
		mis->_miMinDam = monsters[misource]._mMinDamage << 6;
		mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
	} else {
		mis->_miMinDam = currLvl._dLevel << 6;
		mis->_miMaxDam = currLvl._dLevel << (1 + 6);
	}
	return MIRES_DONE;
}

int AddArrowC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int numarrows = 1, mitype;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if ((micaster & MST_PLAYER) && plx(misource)._pIFlags & ISPL_MULT_ARROWS) {
		numarrows = 3;
		// PlaySfxLoc(IS_STING1, sx, sy);
	}

	mis = &missile[mi];
	static_assert(MIS_ARROWC - 3 == MIS_ARROW, "AddArrowC optimization depends on the order of MIS_ARROWs I.");
	static_assert(MIS_PBARROWC - 3 == MIS_PBARROW, "AddArrowC optimization depends on the order of MIS_ARROWs II.");
	static_assert(MIS_ASARROWC - 3 == MIS_ASARROW, "AddArrowC optimization depends on the order of MIS_ARROWs III.");
	static_assert(MIS_ARROWC + 1 == MIS_PBARROWC, "AddArrowC optimization depends on the order of MIS_ARROWs IV.");
	static_assert(MIS_PBARROWC + 1 == MIS_ASARROWC, "AddArrowC optimization depends on the order of MIS_ARROWs V.");
	assert(mis->_miType >= MIS_ARROWC && mis->_miType <= MIS_ASARROWC);
	mitype = mis->_miType - 3;

	while (--numarrows >= 0) {
		int xoff = 0;
		int yoff = 0;
		if (numarrows != 0) {
			int angle = numarrows == 2 ? -1 : 1;
			int x = dx - sx;
			if (x != 0)
				yoff = x < 0 ? angle : -angle;
			int y = dy - sy;
			if (y != 0)
				xoff = y < 0 ? -angle : angle;
		}
		AddMissile(sx, sy, dx + xoff, dy + yoff, 0, mitype, micaster, misource, 0);
	}
	return MIRES_DELETE;
}

int AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int nTries;

	assert((unsigned)misource < MAX_PLRS);
	static_assert(DBORDERX >= 6 && DBORDERY >= 6, "AddRndTeleport expects a large enough border.");
	if ((micaster & MST_PLAYER) || (dx == 0 && dy == 0)) {
		nTries = 0;
		do {
			nTries++;
			if (nTries > 500) {
				return MIRES_DELETE;
			}
			dx = RandRange(4, 6);
			dy = RandRange(4, 6);
			if (random_(58, 2) == 1)
				dx = -dx;
			if (random_(58, 2) == 1)
				dy = -dy;
			dx += sx;
			dy += sy;
			assert(IN_DUNGEON_AREA(dx, dy));
		} while (!PosOkActor(dx, dy));
	}

	mis = &missile[mi];
	mis->_miRange = 1;
	mis->_mix = dx;
	mis->_miy = dy;
	dPlayer[dx][dy] = -(misource + 1);
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int av, i, mindam, maxdam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	if (micaster & MST_PLAYER) {
		switch (mis->_miType) {
		case MIS_FIREBOLT:
			av = MIS_SHIFTEDVEL(16 + 2 * spllvl);
			mindam = (plx(misource)._pMagic >> 3) + spllvl + 1;
			maxdam = mindam + 9;
			break;
		case MIS_FIREBALL:
			av = MIS_SHIFTEDVEL(spllvl + 16);
			mindam = (plx(misource)._pMagic >> 2) + 10;
			maxdam = mindam + 10;
			for (i = spllvl; i > 0; i--) {
				mindam += mindam >> 3;
				maxdam += maxdam >> 3;
			}
			break;
		case MIS_HBOLT:
			av = MIS_SHIFTEDVEL(16 + 2 * spllvl);
			mindam = (plx(misource)._pMagic >> 2) + spllvl;
			maxdam = mindam + 9;
			break;
		case MIS_FLARE:
			av = MIS_SHIFTEDVEL(16);
			if (!plx(misource)._pInvincible)
				PlrDecHp(misource, 50 << 6, DMGTYPE_NPC);
			mindam = maxdam = (plx(misource)._pMagic * (spllvl + 1)) >> 3;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		//if (av > MIS_SHIFTEDVEL(63))
		//	av = MIS_SHIFTEDVEL(63);
	} else if (micaster == MST_MONSTER) {
		//assert(misource >= MAX_MINIONS);
		av = MIS_SHIFTEDVEL(mis->_miType == MIS_FIREBOLT ? 26 : 16);
		mindam = monsters[misource]._mMinDamage;
		maxdam = monsters[misource]._mMaxDamage;
	} else {
		av = MIS_SHIFTEDVEL(16);
		mindam = currLvl._dLevel;
		maxdam = mindam + 2 * mindam - 1;
	}
	mis->_miMinDam = mis->_miMaxDam = RandRange(mindam, maxdam) << 6;
	GetMissileVel(mi, sx, sy, dx, dy, av);
	if (misfiledata[mis->_miAnimType].mfAnimFAmt == 16)
		SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	static_assert(MAX_LIGHT_RAD >= 8, "AddFirebolt needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 255;
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));
	mis = &missile[mi];
	mis->_mitxoff += 3 * mis->_mixvel;
	mis->_mityoff += 3 * mis->_miyvel;
	GetMissilePos(mi);
	mis->_miRange = 255;
	mis->_miMinDam = monsters[misource]._mMinDamage << 6;
	mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	static_assert(MAX_LIGHT_RAD >= 8, "AddMagmaball needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	return MIRES_DONE;
}

/*int AddKrull(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));
	missile[mi]._miRange = 255;
	missile[mi]._miMinDam = missile[mi]._miMaxDam = 4 << 6;
	//PutMissile(mi);
	return MIRES_DONE;
}*/

int AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	const char *cr;

	assert((unsigned)misource < MAX_PLRS);
	static_assert(DBORDERX >= 6 && DBORDERY >= 6, "AddTeleport expects a large enough border.");
	mis = &missile[mi];
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkActor(tx, ty)) {
				mis->_mix = tx;
				mis->_miy = ty;
				mis->_misx = tx;
				mis->_misy = ty;
				dPlayer[tx][ty] = -(misource + 1);
				mis->_miRange = 1;
				return MIRES_DONE;
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));

	mindam = 1;
	if (misource != -1) {
		maxdam = (plx(misource)._pMagic >> 1) + (spllvl << 5);
	} else {
		maxdam = 6 + currLvl._dLevel;
	}
	mis = &missile[mi];
	mis->_miRange = 3 * TILE_WIDTH / 16 + 2; // 4 normal tiles
	mis->_miMinDam = mindam << (6 - 2); // * 16 / 64
	mis->_miMaxDam = maxdam << (6 - 2); // * 16 / 64
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	return MIRES_DONE;
}

/**
 * Var1: sfx helper
 */
int AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int magic, mindam, maxdam;

	mis = &missile[mi];
	mis->_miRange = 160 * (spllvl + 1);
	if (misource != -1) {
		assert((unsigned)misource < MAX_PLRS);
		// TODO: add support for spell duration modifier
		// range += (plx(misource)._pISplDur * range) >> 7;
		magic = plx(misource)._pMagic;
		mindam = (magic >> 3) + spllvl + 5;
		maxdam = (magic >> 3) + spllvl * 2 + 10;
	} else {
		mindam = 5 + currLvl._dLevel;
		maxdam = 10 + currLvl._dLevel * 2;
	}
	mis->_miMinDam = mindam << (-3 + 6);
	mis->_miMaxDam = maxdam << (-3 + 6);
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
/*int AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, mindam, maxdam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	//assert(misource != -1);
	if (micaster & MST_PLAYER) {
		mindam = (plx(misource)._pMagic >> 2) + 10;
		maxdam = mindam + 10;
		for (i = spllvl; i > 0; i--) {
			mindam += mindam >> 3;
			maxdam += maxdam >> 3;
		}
		i = MIS_SHIFTEDVEL(spllvl + 16);
		//if (i > MIS_SHIFTEDVEL(50))
		//	i = MIS_SHIFTEDVEL(50);
	} else {
		mindam = monsters[misource]._mMinDamage;
		maxdam = monsters[misource]._mMaxDamage;
		i = MIS_SHIFTEDVEL(16);
	}
	mis->_miMinDam = mis->_miMaxDam = RandRange(mindam, maxdam) << 6;
	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	static_assert(MAX_LIGHT_RAD >= 8, "AddFireball needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 255;
	return MIRES_DONE;
}*/

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 */
int AddLightningC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(32));

	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miRange = 255;
	return MIRES_DONE;
}

int AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam, range;

	mis = &missile[mi];
	static_assert(MAX_LIGHT_RAD >= 4, "AddLightning needs at least light-radius of 4.");
	mis->_miLid = AddLight(sx, sy, 4);
	if (midir >= 0) {
		mis->_mixoff = missile[midir]._mixoff;
		mis->_miyoff = missile[midir]._miyoff;
		mis->_mitxoff = missile[midir]._mitxoff;
		mis->_mityoff = missile[midir]._mityoff;
	}
	range = 8 - 1;
	if (micaster & MST_PLAYER) {
		mindam = 1;
		maxdam = plx(misource)._pMagic + (spllvl << 3);
		range = (spllvl >> 1) + 6 - 1;
	} else if (micaster == MST_MONSTER) {
		if (spllvl == 0) {
			// standard lightning from a monster
			mindam = monsters[misource]._mMinDamage;
			maxdam = monsters[misource]._mMaxDamage << 1;
		} else {
			// lightning from a retreating MT_FAMILIAR
			mindam = 1;
			maxdam = monsters[misource]._mLevel;
		}
	} else {
		mindam = currLvl._dLevel;
		maxdam = mindam + currLvl._dLevel;
	}

	mis->_miRange = range;
	mis->_miMinDam = mindam << (6 - 3);
	mis->_miMaxDam = maxdam << (6 - 3);
	assert(mis->_miAnimLen == 8);
	// assert(misfiledata[MFILE_LGHNING].mfAnimLen[0] == misfiledata[MFILE_THINLGHT].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, 8);
	return MIRES_DONE;
}

int AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis, *bmis;

	mis = &missile[mi];
	if (dx != -1) {
		bmis = &missile[dx];
		mis->_mix = bmis->_mix;
		mis->_miy = bmis->_miy;
		//mis->_misx = bmis->_mix;
		//mis->_misy = bmis->_miy;
		mis->_mixoff = bmis->_mixoff;
		mis->_miyoff = bmis->_miyoff;
		//mis->_mitxoff = bmis->_mitxoff;
		//mis->_mityoff = bmis->_mityoff;
	}
	//mis->_mixvel = 0;
	//mis->_miyvel = 0;
	mis->_miRange = mis->_miAnimLen;
	return MIRES_DONE;
}

static bool CheckIfTrig(int x, int y)
{
	int i;

	for (i = 0; i < numtrigs; i++) {
		if (abs(trigs[i]._tx - x) < 2 && abs(trigs[i]._ty - y) < 2)
			return true;
	}
	return false;
}

/**
 * Var3: triggered
 */
int AddTown(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, j, tx, ty;
	const char *cr;
	// the position of portals in town and recreated portals are fixed
	if (currLvl._dType != DTYPE_TOWN && spllvl >= 0) {
		const int RANGE = 6;
		static_assert(DBORDERX >= RANGE && DBORDERY >= RANGE, "AddTown expects a large enough border.");
		for (i = 0; i < RANGE; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = (BYTE)*cr; j > 0; j--) {
				tx = dx + *++cr;
				ty = dy + *++cr;
				assert(IN_DUNGEON_AREA(tx, ty));
				if (PosOkMissile(tx, ty)) {
					if (!CheckIfTrig(tx, ty)) {
						i = RANGE;
						break;
					}
				}
			}
		}
		if (i == RANGE)
			return MIRES_FAIL_DELETE;
	} else {
		tx = dx;
		ty = dy;
	}
	// 'delete' previous portal of the misource
	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		if (mis->_miType == MIS_TOWN && mis->_miSource == misource)
			mis->_miRange = -1;
	}
	// setup the new portal
	return AddPortal(mi, 0, 0, tx, ty, 0, 0, misource, spllvl);
}

/**
 * Var3: triggered (only for MIS_TOWN)
 */
int AddPortal(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_mix = mis->_misx = dx;
	mis->_miy = mis->_misy = dy;
	mis->_miRange = 1;
	mis->_miLid = AddLight(dx, dy, spllvl >= 0 ? 1 : 15);
	if (spllvl >= 0) {
		PlaySfxLoc(LS_SENTINEL, dx, dy);
		if (misource == mypnum)
			NetSendCmdLocBParam1(CMD_ACTIVATEPORTAL, dx, dy, currLvl._dLevelIdx);
	} else {
		// a recreated portal (by AddWarpMissile or InitVP*Trigger)
		// make sure the portal is in its final form even on the first frame
		SetMissDir(mi, 1);
		PutMissile(mi);
	}
	return MIRES_DONE;
}

int AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, dam;

	AddMissile(sx, sy, 0, 0, 0, MIS_FLASH2, micaster, misource, spllvl);

	mis = &missile[mi];
	if (micaster & MST_PLAYER) {
		dam = plx(misource)._pMagic >> 1;
		for (i = spllvl; i > 0; i--) {
			dam += dam >> 3;
		}
		mis->_miMinDam = dam;
		mis->_miMaxDam = dam << 3;
	} else  {
		if (micaster == MST_MONSTER) {
			dam = monsters[misource]._mLevel << 1;
		} else {
			dam = currLvl._dLevel << 4;
		}
		mis->_miMinDam = mis->_miMaxDam = dam;
	}
	return MIRES_DONE;
}

int AddFlash2(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_miPreFlag = TRUE;
	return MIRES_DONE;
}

int AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);

	if (misource == mypnum) {
		if (plx(misource)._pManaShield == 0)
			NetSendCmdBParam1(CMD_SETSHIELD, spllvl);
		else
			NetSendCmd(CMD_REMSHIELD);
	}
	return MIRES_DELETE;
}

/**
 * Var3: x coordinate of the missile-light
 * Var4: y coordinate of the missile-light
 */
int AddFireWave(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int magic, mindam, maxdam;

	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));
	mis = &missile[mi];
	if (misource != -1) {
		assert((unsigned)misource < MAX_PLRS);
		magic = plx(misource)._pMagic;
		mindam = (magic >> 3) + 2 * spllvl + 1;
		maxdam = (magic >> 3) + 4 * spllvl + 2;
	} else {
		mindam = currLvl._dLevel + 1;
		maxdam = 2 * currLvl._dLevel + 2;
	}
	mis->_miMinDam = mindam << 6;
	mis->_miMaxDam = maxdam << 6;
	mis->_miRange = 255;
	//mis->_miVar3 = 0;
	//mis->_miVar4 = 0;
	/*mis->_mix++;
	mis->_miy++;
	mis->_miyoff -= TILE_HEIGHT;*/
	return MIRES_DONE;
}

int AddMeteor(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam, i, j, tx, ty;
	const char* cr;

	mis = &missile[mi];
	//if (micaster & MST_PLAYER) {
		mindam = (plx(misource)._pMagic >> 2) + (spllvl << 3) + 40;
		maxdam = (plx(misource)._pMagic >> 2) + (spllvl << 4) + 40;
	/*} else if (micaster == MST_MONSTER) {
		mindam = monsters[misource]._mMinDamage;
		maxdam = monsters[misource]._mMaxDamage;
	} else {
		mindam = currLvl._dLevel;
		maxdam = currLvl._dLevel * 2;
	}*/
	mis->_miMinDam = mis->_miMaxDam = RandRange(mindam, maxdam) << 6;

	static_assert(DBORDERX >= 6 && DBORDERY >= 6, "AddMeteor expects a large enough border.");
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (LineClear(sx, sy, tx, ty)) {
				if ((nSolidTable[dPiece[tx][ty]] | dObject[tx][ty]) == 0) {
					mis->_misx = tx;
					mis->_misy = ty;
					mis->_mix = tx;
					mis->_miy = ty;
					mis->_miAnimAdd = -1;
					mis->_miAnimFrame = misfiledata[MFILE_SHATTER1].mfAnimLen[0];
					return MIRES_DONE;
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, j, tx, ty;
	const char* cr;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];

	static_assert(DBORDERX >= 6 && DBORDERY >= 6, "AddGuardian expects a large enough border.");
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (LineClear(sx, sy, tx, ty)) {
				if (PosOkMissile(tx, ty)) {
					mis->_mix = tx;
					mis->_miy = ty;
					mis->_misx = tx;
					mis->_misy = ty;
					static_assert(MAX_LIGHT_RAD >= 1, "AddGuardian needs at least light-radius of 1.");
					mis->_miLid = AddLight(tx, ty, 1);
					mis->_miRange = spllvl + (plx(misource)._pLevel >> 1);
					return MIRES_DONE;
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: remaining jumps
 */
int AddChain(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	assert((unsigned)misource < MAX_PLRS);

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(32));

	mis = &missile[mi];
	static_assert(MAX_LIGHT_RAD >= 4, "AddChain needs at least light-radius of 4.");
	mis->_miLid = AddLight(sx, sy, 4);
	//assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miVar1 = 1 + (spllvl >> 1);
	//if (micaster & MST_PLAYER) {
		mis->_miMinDam = 1 << 6;
		mis->_miMaxDam = plx(misource)._pMagic << 6;
	//} else if (micaster == MST_MONSTER) {
	//	mindam = 1 << 6;
	//	maxdam = monsters[misource].mMaxDamage << 6;
	//} else {
	//	mindam = 1 << 6;
	//	maxdam = currLvl._dLevel << (1 + 6);
	//}
	mis->_miRange = 255;
	return MIRES_DONE;
}

int AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(18));
	//assert(dMonster[sx][sy] == misource + 1);
	dMonster[sx][sy] = -(misource + 1);
	monsters[misource]._mmode = MM_CHARGE;
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miLightFlag = TRUE;
	SyncRhinoAnim(mi);
	// mis->_miRange = 255;
	//PutMissile(mi);
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the destination
 * Var2: y coordinate of the destination
 */
int AddCharge(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int pnum = misource, chv, aa;

	dPlayer[sx][sy] = -(pnum + 1);
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}

	chv = MIS_SHIFTEDVEL(16) / M_SQRT2;
	aa = 2;
	if (plr._pIWalkSpeed != 0) {
		if (plr._pIWalkSpeed == 3) {
			// ISPL_FASTESTWALK
			chv = MIS_SHIFTEDVEL(32) / M_SQRT2;
			aa = 4;
		} else {
			// (ISPL_FASTERWALK | ISPL_FASTWALK)
			chv = MIS_SHIFTEDVEL(24) / M_SQRT2;
			aa = 3;
		}
	}
	GetMissileVel(mi, sx, sy, dx, dy, chv);
	plr._pmode = PM_CHARGE;
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miVar1 = dx;
	mis->_miVar2 = dy;
	mis->_miAnimAdd = aa;
	mis->_miLightFlag = TRUE;
	SyncChargeAnim(mi);
	if (pnum == mypnum) {
		// assert(ScrollInfo._sdx == 0);
		// assert(ScrollInfo._sdy == 0);
		ScrollInfo._sdir = 1 + OPPOSITE(midir); // == dir2sdir[midir]
	}
	//mis->_miLid = mon->mlid;
	//PutMissile(mi);
	return MIRES_DONE;
}

/**
 * Var1: target found
 * Var2: attempts to find a target
 */
/*int AddFireman(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	AnimStruct* anim;
	MonsterStruct* mon;

	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));
	mon = &monsters[misource];
	anim = &mon->_mAnims[MA_WALK];
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miAnimFlag = TRUE;
	mis->_miAnimData = anim->aData[midir];
	mis->_miAnimFrameLen = anim->aFrameLen;
	mis->_miAnimLen = anim->aFrames;
	mis->_miAnimWidth = mon->_mAnimWidth;
	mis->_miAnimXOffset = mon->_mAnimXOffset;
	mis->_miAnimAdd = 1;
	//mis->_miVar1 = FALSE;
	//mis->_miVar2 = 0;
	mis->_miLightFlag = TRUE;
	if (mon->_uniqtype != 0)
		mis->_miUniqTrans = mon->_uniqtrans + 4;
	dMonster[mon->_mx][mon->_my] = 0;
	//mis->_miRange = 255;
	//PutMissile(mi);
	return MIRES_DONE;
}*/

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
/*int AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));
#ifdef HELLFIRE
	if (misfiledata[missile[mi]._miAnimType].mfAnimFAmt == 16) {
		SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	}
#endif
	mis = &missile[mi];
	mis->_miRange = 255;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	static_assert(MAX_LIGHT_RAD >= 8, "AddFlare needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	//assert(misource != -1);
	if (micaster & MST_PLAYER) {
		if (!plx(misource)._pInvincible)
			PlrDecHp(misource, 320, DMGTYPE_NPC);
		mis->_miMinDam = mis->_miMaxDam = (plx(misource)._pMagic * (spllvl + 1)) << (-3 + 6);
	} else {
		mis->_miMinDam = monsters[misource]._mMinDamage << 6;
		mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
	}
	return MIRES_DONE;
}*/

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miRange = 5 * (monsters[misource]._mInt + 4);
	mis->_miMinDam = monsters[misource]._mMinDamage << 6;
	mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
	//mis->_miLid = NO_LIGHT;
	//PutMissile(mi);
	return MIRES_DONE;
}

int AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int dam;

	mis = &missile[mi];
	if (spllvl == 0) {
		// pud from a missile
		dam = currLvl._dLevel << 2;
	} else {
		// pud from a corpse
		dam = monsters[misource]._mmaxhp >> (1 + 6);
	}
	mis->_miMinDam = mis->_miMaxDam = dam;
	mis->_miRange = 40 * (monsters[misource]._mInt + 1) + random_(50, 16);
	mis->_miLightFlag = TRUE;
	mis->_miPreFlag = TRUE;
	return MIRES_DONE;
}

/**
 * Var1: mmode of the monster
 * Var2: mnum of the monster
 */
int AddStone(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	int i, j, tx, ty, mid, range;
	const char *cr;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	static_assert(DBORDERX >= 2 && DBORDERY >= 2, "AddStone expects a large enough border.");
	for (i = 0; i < 3; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mid = dMonster[tx][ty];
			if (mid == 0)
				continue;
			mid = mid >= 0 ? mid - 1 : -(mid + 1);
			mon = &monsters[mid];
			if (!(mon->_mFlags & MFLAG_NOSTONE)) {
				if (mon->_mmode != MM_FADEIN && mon->_mmode != MM_FADEOUT && mon->_mmode != MM_CHARGE && mon->_mmode != MM_STONE && mon->_mmode != MM_DEATH/*mon->_mhitpoints >= (1 << 6*/) {
					mis->_miVar1 = mon->_mmode;
					mis->_miVar2 = mid;
					mon->_mVar3 = mon->_mmode;
					mon->_mmode = MM_STONE;
					// ensure lastx/y are set when MI_Stone 'alerts' the monster
					if (micaster == MST_PLAYER) {
						mon->_lastx = plx(misource)._px;
						mon->_lasty = plx(misource)._py;
					//} else {
					//	assert(!MON_RELAXED);
					}

					// range = (sl * 128 - HP + 128) * 2
					range = ((spllvl + 1) << (7 + 6)) - mon->_mmaxhp;
					// TODO: add support for spell duration modifier
					//range += (range * plx(misource)._pISplDur) >> 7;
					range >>= 5;
					if (range < 15)
						return MIRES_DELETE;
					if (range > 239)
						range = 239;
					mis->_miRange = range;
					return MIRES_DONE;
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MonsterStruct* mon;
	int tx, ty, i, j;
	const char* cr;

	assert((unsigned)misource < MAX_PLRS);

	mon = &monsters[misource];
	if (mon->_mmode > MM_INGAME_LAST) {
		static_assert(DBORDERX >= 6 && DBORDERY >= 6, "AddGolem expects a large enough border.");
		for (i = 0; i < 6; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = (BYTE)*cr; j > 0; j--) {
				tx = dx + *++cr;
				ty = dy + *++cr;
				assert(IN_DUNGEON_AREA(tx, ty));
				if (LineClear(sx, sy, tx, ty)) {
					if (PosOkActor(tx, ty)) {
						SpawnGolem(misource, tx, ty, spllvl);
						return MIRES_DELETE;
					}
				}
			}
		}
		return MIRES_FAIL_DELETE;
	}

	missile[mi]._misx = missile[mi]._mix = mon->_mx;
	missile[mi]._misy = missile[mi]._miy = mon->_my;
	missile[mi]._miMaxDam = mon->_mhitpoints;
	missile[mi]._miMinDam = missile[mi]._miMaxDam >> 1;
	CheckSplashColFull(mi);

	MonStartKill(misource, misource);
	return MIRES_DELETE;
}

/**
 * Var1: target hit
 */
int AddApocaExp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	mis = &missile[mi];
	// assert(mis->_miAnimType == MFILE_FIREPLAR);
	mis->_miRange = misfiledata[MFILE_FIREPLAR].mfAnimLen[0] * misfiledata[MFILE_FIREPLAR].mfAnimFrameLen[0];
	mis->_miMinDam = mis->_miMaxDam = 40 << (6 + gnDifficulty); // assert(misource == DIABLO);
	//mis->_miVar1 = FALSE;
	return MIRES_DONE;
}

int AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i, hp;

	assert((unsigned)misource < MAX_PLRS);

	hp = RandRange(1, 10);
	for (i = spllvl; i > 0; i--) {
		hp += RandRange(1, 6);
	}
	for (i = plx(misource)._pLevel; i > 0; i--) {
		hp += RandRange(1, 4);
	}
	hp <<= 6;

	switch (plx(misource)._pClass) {
	case PC_WARRIOR: hp <<= 1;            break;
#ifdef HELLFIRE
	case PC_BARBARIAN:
	case PC_MONK:    hp <<= 1;            break;
	case PC_BARD:
#endif
	case PC_ROGUE: hp += hp >> 1; break;
	case PC_SORCERER: break;
	default:
		ASSUME_UNREACHABLE
	}
	PlrIncHp(misource, hp);
	return MIRES_DELETE;
}

int AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int pnum, i, hp;

	assert((unsigned)misource < MAX_PLRS);

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (pnum != misource
		 && plr._pActive && plr._pDunLevel == currLvl._dLevelIdx
		 && plr._px == dx && plr._py == dy
		 && plr._pHitPoints >= (1 << 6)) {
			hp = RandRange(1, 10);
			for (i = spllvl; i > 0; i--) {
				hp += RandRange(1, 6);
			}
			for (i = plx(misource)._pLevel; i > 0; i--) {
				hp += RandRange(1, 4);
			}
			hp <<= 6;

			switch (plx(misource)._pClass) {
			case PC_WARRIOR: hp <<= 1;    break;
#ifdef HELLFIRE
			case PC_MONK: hp *= 3;        break;
			case PC_BARBARIAN: hp <<= 1;  break;
			case PC_BARD:
#endif
			case PC_ROGUE: hp += hp >> 1; break;
			case PC_SORCERER: break;
			default:
				ASSUME_UNREACHABLE
			}
			PlrIncHp(pnum, hp);
			break;
		}
	}
	return MIRES_DELETE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 * Var3: destination reached
 * Var4: x coordinate of the destination
 * Var5: y coordinate of the destination
 */
int AddElemental(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int magic, i, mindam, maxdam;

	assert((unsigned)misource < MAX_PLRS);
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(16));
	SetMissDir(mi, GetDirection8(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	//mis->_miVar3 = FALSE;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	static_assert(MAX_LIGHT_RAD >= 8, "AddElemental needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);

	magic = plx(misource)._pMagic;
	mindam = (magic >> 3) + 2 * spllvl + 4;
	maxdam = (magic >> 3) + 4 * spllvl + 20;
	for (i = spllvl; i > 0; i--) {
		mindam += mindam >> 3;
		maxdam += maxdam >> 3;
	}
	mis->_miMinDam = mis->_miMaxDam = RandRange(mindam, maxdam) << 6;
	mis->_miRange = 0;
	return MIRES_DONE;
}

int AddOpItem(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	return MIRES_DELETE;
}

/**
 * Var1: x coordinate of the first wave
 * Var2: y coordinate of the first wave
 * Var3: direction of the first wave
 * Var4: direction of the second wave
 * Var5: x coordinate of the second wave
 * Var6: y coordinate of the second wave
 * Var7: first wave stopped
 * Var8: second wave stopped
 */
int AddWallC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	const char *cr;

	assert((unsigned)misource < MAX_PLRS);
	static_assert(DBORDERX >= 6 && DBORDERY >= 6, "AddWallC expects a large enough border.");
	mis = &missile[mi];
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (LineClear(sx, sy, tx, ty)) {
				if ((sx != tx || sy != ty) && (nSolidTable[dPiece[tx][ty]] | dObject[tx][ty]) == 0) {
					midir = GetDirection8(sx, sy, dx, dy);
					mis->_miVar1 = tx;
					mis->_miVar2 = ty;
					mis->_miVar3 = (midir - 2) & 7;
					mis->_miVar4 = (midir + 2) & 7;
					mis->_miVar5 = tx;
					mis->_miVar6 = ty;
					mis->_miVar7 = FALSE;
					mis->_miVar8 = FALSE;
					mis->_miRange = (spllvl >> 1);
					return MIRES_DONE;
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i, range;

	assert((unsigned)misource < MAX_PLRS);
	range = 1584;
	for (i = spllvl; i > 0; i--) {
		range += range >> 3;
	}
	// TODO: add support for spell duration modifier
	//range += range * plx(misource)._pISplDur >> 7;
	plx(misource)._pTimer[PLTR_INFRAVISION] = range;
	return MIRES_DELETE;
}

int AddFireWaveC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);
	int sd, nx, ny, dir;
	int i, j;

	sd = GetDirection8(sx, sy, dx, dy);
	sx += XDirAdd[sd];
	sy += YDirAdd[sd];
	if (!nMissileTable[dPiece[sx][sy]]) {
		AddMissile(sx, sy, sx + XDirAdd[sd], sy + YDirAdd[sd], 0, MIS_FIREWAVE, micaster, misource, spllvl);

		for (i = -2; i <= 2; i += 4) {
			dir = (sd + i) & 7;
			nx = sx;
			ny = sy;
			for (j = (spllvl >> 1) + 2; j > 0; j--) {
				nx += XDirAdd[dir];
				ny += YDirAdd[dir];
				if (!IN_DUNGEON_AREA(nx, ny))
					break;
				if (nMissileTable[dPiece[nx][ny]])
					break;
				AddMissile(nx, ny, nx + XDirAdd[sd], ny + YDirAdd[sd], 0, MIS_FIREWAVE, micaster, misource, spllvl);
			}
		}
	}

	return MIRES_DELETE;
}

int AddNovaC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i, tx, ty;
	const char* cr;

	cr = &CrawlTable[CrawlNum[3]];
	for (i = *cr; i > 0; i--) {
		tx = sx + *++cr;
		ty = sy + *++cr;
		AddMissile(sx, sy, tx, ty, 0, MIS_LIGHTBALL, micaster, misource, spllvl);
	}

	return MIRES_DELETE;
}

int AddRage(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int pnum;

	pnum = misource;
	assert((unsigned)pnum < MAX_PLRS);
	if (plr._pTimer[PLTR_RAGE] == 0) {
		plr._pTimer[PLTR_RAGE] = 32 * spllvl + 245;
		PlaySfxLoc(sgSFXSets[SFXS_PLR_70][plr._pClass], plr._px, plr._py);
		CalcPlrItemVals(pnum, false); // last parameter should not matter
	}
	return MIRES_DELETE;
}

int AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int oi = spllvl;
	int pnum = misource;

	// assert((unsigned)pnum < MAX_PLRS);
	// assert((unsigned)oi < MAXOBJECTS);
	// assert(objects[oi]._oBreak == OBM_UNBREAKABLE);
	// assert(abs(plr._px - dx) <= 1 && abs(plr._py - dy) <= 1);
	// assert(abs(dObject[dx][dy]) == oi + 1);
	DisarmObject(pnum, oi);
	OperateObject(pnum, oi, false);
	return MIRES_DELETE;
}

/**
 * Var2: animation timer
 */
int AddInferno(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	MissileStruct* bmis;
	int mindam, maxdam;

	mis = &missile[mi];
	static_assert(MAX_LIGHT_RAD >= 1, "AddInferno needs at least light-radius of 1.");
	bmis = &missile[midir];
	mis->_misx = bmis->_misx;
	mis->_misy = bmis->_misy;
	mis->_mixoff = bmis->_mixoff;
	mis->_miyoff = bmis->_miyoff;
	// mis->_mitxoff = bmis->_mitxoff;
	// mis->_mityoff = bmis->_mityoff;
	// assert(bmis->_miVar3 < 3);
	mis->_miVar2 = bmis->_miVar3 * 4;
	mis->_miRange = misfiledata[MFILE_INFERNO].mfAnimLen[0] * misfiledata[MFILE_INFERNO].mfAnimFrameLen[0];
	// assert(misource != -1);
	if (micaster & MST_PLAYER) {
		mindam = plx(misource)._pMagic;
		maxdam = mindam + (spllvl << 4);
	} else {
		mindam = monsters[misource]._mMinDamage;
		maxdam = monsters[misource]._mMaxDamage;
	}
	mis->_miMinDam = mindam << (6 - 4);
	mis->_miMinDam = maxdam << (6 - 4);
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 * Var3: timer to dissipate
 */
int AddInfernoC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(32));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	//mis->_miVar3 = 0;
	mis->_miRange = 255;
	return MIRES_DONE;
}

/*int AddFireTrap(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miMinDam = (2 + currLvl._dLevel) << (6 - 2);
	mis->_miMaxDam = mis->_miMinDam * 2;
	mis->_miRange = 8;
	return MIRES_DONE;
}*/

int AddBarrelExp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_miMinDam = 8 << (6 + gnDifficulty);
	mis->_miMaxDam = 16 << (6 + gnDifficulty);

	CheckMissileCol(mi, sx, sy, MICM_NONE);
	return MIRES_DELETE;
}

int AddCboltC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i = 3;

	// checks commented out, because spllvl is zero if the caster is not a player
	//if (misource != -1) {
	//	if (micaster & MST_PLAYER) {
			i += (spllvl >> 1);
	//	}
	//}

	while (i-- != 0) {
		AddMissile(sx, sy, dx, dy, midir, MIS_CBOLT, micaster, misource, spllvl);
	}
	return MIRES_DELETE;
}

/**
 * Var1: light strength
 * Var2: base direction
 * Var3: movement counter
 * Var4: rnd direction
 */
int AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(8));

	mis = &missile[mi];
	static_assert(MAX_LIGHT_RAD >= 5, "AddCbolt needs at least light-radius of 5.");
	mis->_miLid = AddLight(sx, sy, 5);
	mis->_miVar1 = 5;
	mis->_miVar2 = midir;
	//mis->_miVar3 = 0;
	mis->_miVar4 = random_(0, 16);
	if (micaster & MST_PLAYER) {
		mis->_miMinDam = 1;
		mis->_miMaxDam = (plx(misource)._pMagic << (-2 + 6)) + (spllvl << (2 + 6));
	} else {
		mis->_miMinDam = mis->_miMaxDam = 15 << (6 + gnDifficulty);
	}
	mis->_miRange = 255;
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_MINILTNG].mfAnimLen[0]);
	return MIRES_DONE;
}

int AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	if (spllvl == mypnum)
		NetSendCmd(CMD_PLRRESURRECT);

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	// mis->_misx = mis->_mix;
	// mis->_misy = mis->_miy;
	return MIRES_DONE;
}

int AddAttract(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	int dist, i, j, tx, ty, mnum;
	const char* cr;

	if (!LineClear(sx, sy, dx, dy))
		return MIRES_FAIL_DELETE;

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	mis->_miAnimFrame = 2;
	mis->_miAnimAdd = 2;

	dist = 4 + (spllvl >> 2);
	static_assert(DBORDERX >= 10 && DBORDERY >= 10, "AddAttract expects a large enough border.");
	if (dist > 10)
		dist = 10;
	for (i = 0; i < dist; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			mnum = dMonster[tx][ty] - 1;
			if (mnum < 0 || !LineClear(dx, dy, tx, ty))
				continue;
			mon = &monsters[mnum];
			if (mon->_msquelch != SQUELCH_MAX) {
				mon->_msquelch = SQUELCH_MAX;
				mon->_lastx = dx;
				mon->_lasty = dy;
			}	
		}
	}

	return MIRES_DONE;
}

int AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int pnum = misource;
	int target = spllvl & 0xFFFF;
	int type = spllvl >> 16;
	bool ret;
	// assert((unsigned)pnum < MAX_PLRS);

	switch (type) {
	case MTT_ITEM:
		// assert(target < MAXITEMS);
		if (pnum == mypnum && dx == items[target]._ix && dy == items[target]._iy &&
			LineClear(plr._px, plr._py, items[target]._ix, items[target]._iy))
			NetSendCmdGItem(CMD_AUTOGETITEM, target);
		break;
	case MTT_MONSTER:
		// assert(target < MAXMONSTERS);
		if (LineClear(plr._px, plr._py, monsters[target]._mx, monsters[target]._my)
		 && !CheckMonsterHit(target, &ret) && monsters[target]._mmode != MM_STONE && (monsters[target]._mmaxhp >> (6 + 1)) < plr._pMagic) {
			monsters[target]._msquelch = SQUELCH_MAX;
			monsters[target]._lastx = plr._px;
			monsters[target]._lasty = plr._py;
			MonGetKnockback(target, plr._px, plr._py);
			MonStartHit(target, pnum, 0, 0);
		}
		break;
	case MTT_OBJECT:
		// assert(target < MAXOBJECTS);
		if (LineClear(plr._px, plr._py, objects[target]._ox, objects[target]._oy))
			OperateObject(pnum, target, true);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return MIRES_DELETE;
}

int AddApocaC2(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int pnum;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr._pActive && plr._pDunLevel == currLvl._dLevelIdx
		 && LineClear(sx, sy, plr._pfutx, plr._pfuty)) {
			AddMissile(plr._pfutx, plr._pfuty, 0, 0, 0, MIS_EXAPOCA2, MST_MONSTER, misource, 0);
		}
	}
	return MIRES_DELETE;
}

int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, int micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	const MissileData *mds;
	int idx, mi, res;

	idx = nummissiles;
	if (idx >= MAXMISSILES)
		return -1;

	mi = missileactive[idx];
	nummissiles++;

	mis = &missile[mi];
	memset(mis, 0, sizeof(*mis));

	// mis->_miRndSeed = GetRndSeed();
	mis->_miCaster = micaster;
	mis->_miSource = misource;
	mis->_miSpllvl = spllvl;
	mis->_mix = sx;
	mis->_miy = sy;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_miType = mitype;
	mds = &missiledata[mitype];
	mis->_miSubType = mds->mType;
	mis->_miFlags = mds->mdFlags;
	mis->_miResist = mds->mResist;
	mis->_miAnimType = mds->mFileNum;
	mis->_miDrawFlag = mds->mDraw;

	if (misfiledata[mis->_miAnimType].mfAnimFAmt < NUM_DIRS)
		SetMissDir(mi, 0);
	else
		SetMissDir(mi, midir);

	mis->_miAnimAdd = 1;
	mis->_miLid = NO_LIGHT;

	if (mds->mlSFX != SFX_NONE) {
		PlaySfxLoc(mds->mlSFX, mis->_misx, mis->_misy, mds->mlSFXCnt);
	}

	res = mds->mAddProc(mi, sx, sy, dx, dy, midir, micaster, misource, spllvl);
	if (res != MIRES_DONE) {
		assert(res == MIRES_FAIL_DELETE || res == MIRES_DELETE);
		// DeleteMissile(mi, idx);
		nummissiles--;
		assert(missileactive[idx] == mi);
		missileactive[idx] = missileactive[nummissiles];
		missileactive[nummissiles] = mi;

		if (res == MIRES_FAIL_DELETE)
			mi = -1;
	}
	return mi;
}

static bool Sentfire(int mi, int sx, int sy)
{
	MissileStruct* mis;

	mis = &missile[mi];
	assert(mis->_miCaster == MST_PLAYER);
	if (dMonster[sx][sy] - 1 >= MAX_MINIONS
	 && monsters[dMonster[sx][sy] - 1]._mhitpoints >= (1 << 6)
	 && LineClear(mis->_mix, mis->_miy, sx, sy)) {
		// SetRndSeed(mis->_miRndSeed);
		AddMissile(mis->_mix, mis->_miy, sx, sy, 0, MIS_FIREBOLT, MST_PLAYER, mis->_miSource, mis->_miSpllvl);
		// mis->_miRndSeed = GetRndSeed();
		SetMissDir(mi, 2);
		mis->_miAnimFrame = misfiledata[MFILE_GUARD].mfAnimLen[2];
		mis->_miAnimAdd = -1;
		return true;
	}

	return false;
}

void MI_Dummy(int mi)
{
	return;
}

void MI_Arrow(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miDist++;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY);
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}

void MI_AsArrow(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miDist++;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
		PutMissile(mi);
		return;
	}
	// assert(missiledata[mis->_miType].miSFX == SFX_NONE);
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
	mis->_miDelFlag = TRUE;
}

void MI_Firebolt(int mi)
{
	MissileStruct* mis;
	//int omx, omy;
	int xptype;

	mis = &missile[mi];
	//omx = mis->_mitxoff;
	//omy = mis->_mityoff;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY);
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
			mis->_miVar1 = mis->_mix;
			mis->_miVar2 = mis->_miy;
			ChangeLightXY(mis->_miLid, mis->_miVar1, mis->_miVar2);
		}
		PutMissile(mi);
		return;
	}

	//mis->_mitxoff = omx;
	//mis->_mityoff = omy;
	//GetMissilePos(mi);
	switch (mis->_miType) {
	case MIS_FIREBOLT:
	case MIS_MAGMABALL:
		xptype = MIS_EXFIRE;
		break;
	case MIS_FIREBALL:
		mis->_miMinDam >>= 1; mis->_miMaxDam >>= 1;
		CheckSplashCol(mi);
		xptype = MIS_EXFBALL;
		break;
	case MIS_HBOLT:
		xptype = MIS_EXHOLY;
		break;
	case MIS_FLARE:
		xptype = MIS_EXFLARE;
		break;
	case MIS_SNOWWICH:
		xptype = MIS_EXSNOWWICH;
		break;
	case MIS_HLSPWN:
		xptype = MIS_EXHLSPWN;
		break;
	case MIS_SOLBRNR:
		xptype = MIS_EXSOLBRNR;
		break;
	case MIS_ACID:
		xptype = MIS_EXACIDP;
		break;
#ifdef HELLFIRE
	case MIS_PSYCHORB:
		xptype = MIS_EXPSYCHORB;
		break;
	case MIS_LICH:
		xptype = MIS_EXLICH;
		break;
	case MIS_BONEDEMON:
		xptype = MIS_EXBONEDEMON;
		break;
	case MIS_ARCHLICH:
		xptype = MIS_EXARCHLICH;
		break;
	case MIS_NECROMORB:
		xptype = MIS_EXNECROMORB;
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
	// SetRndSeed(mis->_miRndSeed); // used by MIS_EXACIDP
	AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, xptype, mis->_miCaster, mis->_miSource, 0);

	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid); // TODO: unlight in a central place?
}

void MI_Lightball(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_WALL);
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}

/*void MI_Krull(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY);
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}*/

void MI_Acidpud(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
	mis->_miRange--;
	if (mis->_miRange < 0) {
		if (mis->_miDir != 0) {
			mis->_miDelFlag = TRUE;
			return;
		} else {
			SetMissDir(mi, 1);
			mis->_miRange = misfiledata[MFILE_ACIDPUD].mfAnimLen[1] * misfiledata[MFILE_ACIDPUD].mfAnimFrameLen[1];
		}
	}
	PutMissileF(mi, BFLAG_MISSILE_PRE);
}

void MI_Firewall(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	if (mis->_miDir == 0) {
		if (mis->_miLid == NO_LIGHT) {
			mis->_miLid = AddLight(mis->_mix, mis->_miy, FireWallLight[0]);
		} else {
			// assert(mis->_miAnimLen < lengthof(FireWallLight));
			// assert(misfiledata[MFILE_FIREWAL].mfAnimLen[0] < lengthof(FireWallLight));
			ChangeLightRadius(mis->_miLid, FireWallLight[mis->_miAnimFrame]);
		}
		// assert(misfiledata[MFILE_FIREWAL].mfAnimFrameLen[0] == 1);
		if (mis->_miAnimFrame == misfiledata[MFILE_FIREWAL].mfAnimLen[0]
		// && mis->_miAnimCnt == misfiledata[MFILE_FIREWAL].mfAnimFrameLen[0] - 1
		 && mis->_miAnimAdd >= 0) {
			// start 'stand' after spawn
			SetMissDir(mi, 1);
			// assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
			mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
			mis->_miVar1 = RandRange(1, 256);
		}
	} else {
		// assert(mis->_miDir == 1);
		if (--mis->_miVar1 == 0 && mis->_miRange > 64) {
			// add random firewall sfx, but only if the fire last more than ~2s
			mis->_miVar1 = 255;
			// assert(missiledata[MIS_FIREWALL].mlSFX == LS_WALLLOOP);
			// assert(missiledata[MIS_FIREWALL].mlSFXCnt == 1);
			PlaySfxLoc(LS_WALLLOOP, mis->_mix, mis->_miy);
		} else if (mis->_miRange == misfiledata[MFILE_FIREWAL].mfAnimLen[0] - 1) {
			// start collapse
			SetMissDir(mi, 0);
			// assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[0]);
			mis->_miAnimFrame = misfiledata[MFILE_FIREWAL].mfAnimLen[0];
			mis->_miAnimAdd = -1;
		}
	}
	PutMissileF(mi, BFLAG_HAZARD); // TODO: do not place hazard if the source is a monster
}

/*void MI_Fireball(int mi)
{
	MissileStruct* mis;
	int mx, my;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy)
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY);
	mx = mis->_mix;
	my = mis->_miy;
	if (mis->_miRange >= 0) {
		if (mx != mis->_miVar1 || my != mis->_miVar2) {
			mis->_miVar1 = mx;
			mis->_miVar2 = my;
			ChangeLightXY(mis->_miLid, mx, my);
		}
		PutMissile(mi);
		return;
	}
	//CheckMissileCol(mi, mx, my, MICM_NONE);
	// TODO: mis->_miMinDam >>= 1; mis->_miMaxDam >>= 1; ?
	CheckSplashCol(mi);

	AddMissile(mis->_mix, mis->_miy, mi, 0, 0, MIS_EXFBALL, MST_NA, 0, 0);
	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid);
}*/

#ifdef HELLFIRE
void MI_HorkSpawn(int mi)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	const char *cr;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY);
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);

		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
	static_assert(DBORDERX >= 2 && DBORDERY >= 2, "MI_HorkSpawn expects a large enough border.");
	for (i = 0; i < 2; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = *cr; j > 0; j--) {
			tx = mis->_mix + *++cr;
			ty = mis->_miy + *++cr;
			// assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkActor(tx, ty)) {
				// assert(mapMonTypes[1].cmType == MT_HORKSPWN);
				SummonMonster(tx, ty, mis->_miDir, 1);
				return;
			}
		}
	}
}

void MI_Rune(int mi)
{
	MissileStruct* mis;
	int j, mnum, tx, ty;
	const char* cr;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	if (--mis->_miVar3 < 0) {
		cr = &CrawlTable[CrawlNum[mis->_miVar2]];
		for (j = *cr; j > 0; j--) {
			tx = mis->_mix + *++cr;
			ty = mis->_miy + *++cr;
			if (dPlayer[tx][ty] == 0) {
				mnum = dMonster[tx][ty];
				if (mnum == 0)
					continue;
				mnum = mnum >= 0 ? mnum - 1 : -(mnum + 1);
				if (monsters[mnum]._mmode == MM_STONE || monsters[mnum]._mmode == MM_DEATH)
					continue;
			}
			// SetRndSeed(mis->_miRndSeed);
			AddMissile(mis->_mix, mis->_miy, tx, ty, 0, mis->_miVar1, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
			mis->_miRange -= 48;
			mis->_miVar3 = 48;
			break;
		}
	} else {
		mis->_miAnimCnt--;
	}
	PutMissile(mi);
}

/*void MI_Lightwall(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_WALL);
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}*/

#endif

void MI_LightningC(int mi)
{
	MissileStruct *mis;
	int mx, my;

	mis = &missile[mi];

	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);

	mx = mis->_mix;
	my = mis->_miy;
	assert(IN_DUNGEON_AREA(mx, my));
	if (mx != mis->_miVar1 || my != mis->_miVar2) {
		mis->_miVar1 = mx;
		mis->_miVar2 = my;
		if (!nMissileTable[dPiece[mx][my]]) {
			// SetRndSeed(mis->_miRndSeed);
			AddMissile(
			    mx,
			    my,
			    0,
			    0,
			    mi,
			    mis->_miType == MIS_LIGHTNINGC ? MIS_LIGHTNING : MIS_LIGHTNING2,
			    mis->_miCaster,
			    mis->_miSource,
			    mis->_miSpllvl);
			// mis->_miRndSeed = GetRndSeed();
		} else {
			mis->_miRange = 0;
		}
	}
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
	}
}

void MI_Lightning(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_WALL);
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid);
}

void MI_Portal(int mi)
{
	MissileStruct* mis;
	static_assert(MAX_LIGHT_RAD >= 15, "MI_Portal needs at least light-radius of 15.");
	int ExpLight[17] = { 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15 };
	PlayerStruct* p;

	mis = &missile[mi];
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	if (mis->_miDir == 0) {
		// assert(mis->_miAnimLen < lengthof(ExpLight));
		// assert(misfiledata[MFILE_RPORTAL].mfAnimLen[0] < lengthof(ExpLight));
		// assert(misfiledata[MFILE_PORTAL].mfAnimLen[0] < lengthof(ExpLight));
		ChangeLightRadius(mis->_miLid, ExpLight[mis->_miAnimFrame]);
		// assert(misfiledata[MFILE_PORTAL].mfAnimLen[0] == misfiledata[MFILE_RPORTAL].mfAnimLen[0]);
		// assert(misfiledata[MFILE_PORTAL].mfAnimFrameLen[0] == 1);
		// assert(misfiledata[MFILE_RPORTAL].mfAnimFrameLen[0] == 1);
		if (mis->_miAnimFrame == misfiledata[MFILE_PORTAL].mfAnimLen[0]
		 /*&& mis->_miAnimCnt == misfiledata[MFILE_PORTAL].mfAnimFrameLen[0] - 1*/) {
			SetMissDir(mi, 1);
		}
	}

	if (mis->_miType == MIS_TOWN) {
		p = &myplr;
		if (p->_px == mis->_mix && p->_py == mis->_miy && /*!p->_pLvlChanging && */p->_pmode == PM_STAND && !mis->_miVar3) {
			mis->_miVar3 = TRUE;
			NetSendCmdBParam1(CMD_TWARP, mis->_miSource);
		}
	}

	PutMissile(mi);
}

void MI_Flash(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	// assert(!nMissileTable[dPiece[mis->_mix][mis->_miy]]);
	CheckSplashColFull(mi);
	if (mis->_miCaster == MST_OBJECT)
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
	// assert(mis->_miAnimLen == misfiledata[MFILE_BLUEXFR].mfAnimLen[0]);
	// assert(misfiledata[MFILE_BLUEXFR].mfAnimFrameLen[0] == 1);
	if (mis->_miAnimFrame == misfiledata[MFILE_BLUEXFR].mfAnimLen[0]
	 /*&& mis->_miAnimCnt == misfiledata[MFILE_BLUEXFR].mfAnimFrameLen[0] - 1*/) {
		mis->_miDelFlag = TRUE;
		return;
	}
	PutMissile(mi);
}

void MI_Flash2(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	// assert(mis->_miAnimLen == misfiledata[MFILE_BLUEXBK].mfAnimLen[0]);
	// assert(misfiledata[MFILE_BLUEXBK].mfAnimFrameLen[0] == 1);
	if (mis->_miAnimFrame == misfiledata[MFILE_BLUEXBK].mfAnimLen[0]
	 /*&& mis->_miAnimCnt == misfiledata[MFILE_BLUEXBK].mfAnimFrameLen[0] - 1*/) {
		mis->_miDelFlag = TRUE;
		return;
	}
	PutMissileF(mi, BFLAG_MISSILE_PRE);
}

void MI_FireWave(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	/*mis->_mix--;
	mis->_miy--;
	mis->_miyoff += TILE_HEIGHT;*/
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_WALL);
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	if (mis->_miDir != 0) {
		if (mis->_mix != mis->_miVar3 || mis->_miy != mis->_miVar4) {
			mis->_miVar3 = mis->_mix;
			mis->_miVar4 = mis->_miy;
			static_assert(MAX_LIGHT_RAD >= 8, "MI_FireWave needs at least light-radius of 8.");
			ChangeLight(mis->_miLid, mis->_miVar3, mis->_miVar4, 8);
		}
	} else {
		if (mis->_miLid == NO_LIGHT)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, FireWallLight[0]);
		else {
			// assert(mis->_miAnimLen < lengthof(FireWallLight));
			// assert(misfiledata[MFILE_FIREWAL].mfAnimLen[0] < lengthof(FireWallLight));
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, FireWallLight[mis->_miAnimFrame]);
		}
		// assert(misfiledata[MFILE_FIREWAL].mfAnimFrameLen[0] == 1);
		if (mis->_miAnimFrame == misfiledata[MFILE_FIREWAL].mfAnimLen[0]
		 /*&& mis->_miAnimCnt == misfiledata[MFILE_FIREWAL].mfAnimFrameLen[0] - 1*/) {
			SetMissDir(mi, 1);
			//assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
			mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
		}
	}
	/*mis->_mix++;
	mis->_miy++;
	mis->_miyoff -= TILE_HEIGHT;*/
	PutMissile(mi);
}

void MI_Meteor(int mi)
{
	MissileStruct* mis;
	int mx, my;
	const int MET_SHIFT_X = 16 * ASSET_MPL, MET_SHIFT_Y = 110 * ASSET_MPL, MET_SHIFT_UP = 26 * ASSET_MPL, MET_STEPS_UP = 2, MET_STEPS_DOWN = 10;

	mis = &missile[mi];

	if (mis->_miAnimType != MFILE_FIREBA) {
		// assert(misfiledata[MFILE_FIREBA].mfAnimFrameLen[0] == 1);
		if (mis->_miAnimFrame == 3
		 /*&& mis->_miAnimCnt == misfiledata[MFILE_FIREBA].mfAnimFrameLen[0] - 1*/) {
			mis->_miyoff -= MET_SHIFT_UP / MET_STEPS_UP;
			mis->_mixoff += MET_SHIFT_X / MET_STEPS_UP;
			if (mis->_miyoff < - MET_SHIFT_UP) {
				mis->_miAnimType = MFILE_FIREBA;
				SetMissDir(mi, 0);
				mis->_mixoff = MET_SHIFT_X;
				static_assert(BORDER_TOP - (96 - 46) * ASSET_MPL >= MET_SHIFT_Y, "MI_Meteor expects a large enough (screen-)border."); // 96: height of the sprite, 46: transparent lines on the first frame
				mis->_miyoff = -MET_SHIFT_Y;
				// TODO: adjust velocity based on spllvl?
			} else {
				mis->_miAnimFrame = 4;
				// mis->_miAnimCnt = 0;
			}
		}

		PutMissile(mi);
	} else {
		mis->_mixoff -= MET_SHIFT_X / MET_STEPS_DOWN;
		mis->_miyoff += MET_SHIFT_Y / MET_STEPS_DOWN;
		if (mis->_miyoff < 0) { // TODO: use _miRange?
			PutMissile(mi);
			return;
		}

		mis->_miDelFlag = TRUE;
		mx = mis->_mix;
		my = mis->_miy;
		CheckMissileCol(mi, mx, my, MICM_NONE);
		PlaySfxLoc(LS_FIRIMP2, mx, my);

		AddMissile(mx, my, 0, 0, 0, MIS_FIREWALL, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
	}
}

void MI_Guardian(int mi)
{
	MissileStruct* mis;
	int i, j, tx, ty;
	bool ex;
	const char* cr;

	mis = &missile[mi];
	switch (mis->_miDir) {
	case 0: // collapse/spawn
		// assert(((1 + misfiledata[MFILE_GUARD].mfAnimLen[0]) >> 1) <= MAX_LIGHT_RAD);
		ChangeLightRadius(mis->_miLid, (1 + mis->_miAnimFrame) >> 1);
		// assert(misfiledata[MFILE_GUARD].mfAnimFrameLen[0] == 1);
		if (mis->_miAnimFrame == misfiledata[MFILE_GUARD].mfAnimLen[0]
		 // && mis->_miAnimCnt == misfiledata[MFILE_GUARD].mfAnimFrameLen[0] - 1
		 && mis->_miAnimAdd >= 0) {
			// start stand after spawn
			SetMissDir(mi, 1);
		} else if (mis->_miAnimFrame == 1
		 // && mis->_miAnimCnt == misfiledata[MFILE_GUARD].mfAnimFrameLen[0] - 1
		 && mis->_miAnimAdd < 0) {
			// done after collapse
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_miLid);
			return;
		}
		break;
	case 1: // active
		if (mis->_miAnimFrame == 1 /*&& mis->_miAnimCnt == 0*/) {
			// check for an enemy
			mis->_miRange--;
			if (mis->_miRange >= 0) {
				ex = false;
				for (i = 6; i >= 0 && !ex; i--) {
					cr = &CrawlTable[CrawlNum[i]];
					for (j = *cr; j > 0; j--) {
						tx = mis->_mix + *++cr;
						ty = mis->_miy + *++cr;
						ex = Sentfire(mi, tx, ty);
						if (ex)
							break;
					}
				}
			} else {
				// start collapse
				SetMissDir(mi, 0);
				mis->_miAnimFrame = misfiledata[MFILE_GUARD].mfAnimLen[0];
				mis->_miAnimAdd = -1;
			}
		}
		break;
	case 2:
		// start stand after fire, or collapse if this was the last shot
		// assert(misfiledata[MFILE_GUARD].mfAnimFrameLen[2] == 1);
		if (mis->_miAnimFrame == 1
		 /* && mis->_miAnimCnt == misfiledata[MFILE_GUARD].mfAnimFrameLen[2] - 1*/) {
			ex = mis->_miRange != 0;
			SetMissDir(mi, ex ? 1 : 0);
			// skip check frame to add delay between attacks
			mis->_miAnimFrame = ex ? 2 : misfiledata[MFILE_GUARD].mfAnimLen[0];
			mis->_miAnimAdd = ex ? 1 : -1;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	PutMissile(mi);
}

void MI_Chain(int mi)
{
	MissileStruct* mis;
	int mx, my, sd, dx, dy;

	mis = &missile[mi];

	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);

	mx = mis->_mix;
	my = mis->_miy;
	if (mx != mis->_misx || my != mis->_misy) {
		if (!nMissileTable[dPiece[mx][my]]) {
			if (CheckMissileCol(mi, mx, my, MICM_BLOCK_ANY)) {
				if (mis->_miVar1-- != 0) {
					// set the new position as the starting point
					mis->_misx = mx;
					mis->_misy = my;
					mis->_mixoff = 0;
					mis->_miyoff = 0;
					mis->_mitxoff = 0;
					mis->_mityoff = 0;
					// restore base range
					mis->_miRange = 255;
					// find new target
					if (!FindClosestChain(mx, my, dx, dy)) {
						// create pseudo-random seed using the monster which was hit (or the first real monster)
						/*sd = dMonster[mx][my];
						if (sd != 0)
							sd = sd >= 0 ? sd - 1 : -(sd + 1);
						else
							sd = MAX_MINIONS;
						SetRndSeed(monsters[sd]._mRndSeed);*/
						sd = random_(0, lengthof(XDirAdd));
						dx = mx + XDirAdd[sd];
						dy = my + YDirAdd[sd];
					}
					//SetMissDir(mi, sd);
					GetMissileVel(mi, mx, my, dx, dy, MIS_SHIFTEDVEL(32));
				}
			}
		} else {
			mis->_miRange = 0;
		}
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid);
}

void MI_Misexp(int mi)
{
	MissileStruct *mis;
	static_assert(MAX_LIGHT_RAD >= 12, "MI_Misexp needs at least light-radius of 12.");
	int ExpLight[] = { 9, 9, 10, 11, 12, 11, 10, 8, 6, 4, 2, 1, 0, 0, 0, 0 };

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		if (mis->_miLid == NO_LIGHT)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, ExpLight[0]);
		else {
			// assert(mis->_miAnimLen < lengthof(ExpLight));
			ChangeLightRadius(mis->_miLid, ExpLight[mis->_miAnimFrame]);
		}
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid);
}

void MI_MiniExp(int mi)
{
	MissileStruct* mis;
	static_assert(MAX_LIGHT_RAD >= 8, "MI_MiniExp needs at least light-radius of 8.");
	int ExpLight[] = { 6, 6, 7, 8, 6, 4, 2, 1, 0, 0, 0 };

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		if (mis->_miLid == NO_LIGHT)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, ExpLight[0]);
		else {
			// assert(mis->_miAnimLen < lengthof(ExpLight));
			ChangeLightRadius(mis->_miLid, ExpLight[mis->_miAnimFrame]);
		}
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid);
}

void MI_Acidsplat(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	// assert(mis->_miAnimLen == misfiledata[MFILE_ACIDSPLA].mfAnimLen[0]);
	/*if (mis->_miRange == misfiledata[MFILE_ACIDSPLA].mfAnimLen[0] * misfiledata[MFILE_ACIDSPLA].mfAnimFrameLen[0]) {
		mis->_mix++;
		mis->_miy++;
		mis->_miyoff -= TILE_HEIGHT;
	}*/
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
	// SetRndSeed(mis->_miRndSeed);
	// assert(misfiledata[missiledata[MIS_ACIDPUD].mFileNum].mfAnimFAmt < NUM_DIRS);
	AddMissile(mis->_mix/* - 1*/, mis->_miy/* - 1*/, 0, 0, 0/*mis->_miDir*/, MIS_ACIDPUD, MST_MONSTER, mis->_miSource, 0);
}

void MI_Teleport(int mi)
{
	MissileStruct *mis;
	int pnum, px, py;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		return;
	}
	assert(mis->_miRange == 0);
	pnum = mis->_miSource;
	px = plr._px;
	py = plr._py;
	dPlayer[px][py] = 0;
	//PlrClrTrans(px, py);

	px = mis->_mix;
	py = mis->_miy;
	SetPlayerLoc(&plr, px, py);
	//PlrDoTrans(px, py);
	dPlayer[px][py] = pnum + 1;
	ChangeLightXY(plr._plid, px, py);
	ChangeVisionXY(plr._pvid, px, py);
	if (pnum == mypnum) {
		ViewX = px; // - ScrollInfo._sdx;
		ViewY = py; // - ScrollInfo._sdy;
	}
}

void MI_Stone(int mi)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	bool dead;

	mis = &missile[mi];
	mon = &monsters[mis->_miVar2];
	dead = mon->_mhitpoints < (1 << 6);
	// assert(mon->_mmode == MM_STONE);
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		if (!dead) {
			mon->_mmode = mis->_miVar1;
		} else {
			// TODO: RemoveMonFromGame ?
			// mon->_mAnimFrame = mon->_mAnimLen;
			// mon->_mAnimCnt = -1;
			// reset squelch value to simplify MonFallenFear, sync_all_monsters and LevelDeltaExport
			mon->_msquelch = 0;
			// assert(mnum >= MAX_MINIONS);
			// mon->_mmode = (mon->_mFlags & MFLAG_NOCORPSE) ? MM_UNUSED : MM_DEAD;
			mon->_mmode = MM_UNUSED;
			nummonsters--;
		}
		return;
	}

	if (!dead) {
		mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
	} else {
		if (mis->_miAnimType != MFILE_SHATTER1) {
			mis->_miDrawFlag = TRUE;
			mis->_miAnimType = MFILE_SHATTER1;
			mis->_miRange = misfiledata[MFILE_SHATTER1].mfAnimLen[0] * misfiledata[MFILE_SHATTER1].mfAnimFrameLen[0] - 1 + 10;
			mis->_mix = mon->_mx;
			mis->_miy = mon->_my;
			// mis->_mixoff = mon->_mxoff;
			// mis->_miyoff = mon->_myoff;
			SetMissDir(mi, 0);
		} else if (mis->_miAnimFrame == misfiledata[MFILE_SHATTER1].mfAnimLen[0]) {
			mis->_miAnimFlag = FALSE;
		}
		PutMissile(mi);
	}
}

void MI_ApocaExp(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		return;
	}
	if (!mis->_miVar1) {
		if (CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE))
			mis->_miVar1 = TRUE;
	}
	PutMissile(mi);
}

void MI_Rhino(int mi)
{
	MissileStruct* mis;
	int bx, by, mnum;

	mis = &missile[mi];
	mnum = mis->_miSource;
	if (monsters[mnum]._mmode != MM_CHARGE) {
		mis->_miDelFlag = TRUE;
		return;
	}
	// restore the real coordinates
	GetMissilePos(mi);
	//assert(dMonster[mis->_mix][mis->_miy] == -(mnum + 1));
	dMonster[mis->_mix][mis->_miy] = 0;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	if (monsters[mnum]._mAi == AI_SNAKE) {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		// assert(monfiledata[MOFILE_SNAKE].moAnimFrames[MA_ATTACK] == 13);
		// assert(monfiledata[MOFILE_SNAKE].moAnimFrameLen[MA_ATTACK] == 1);
		if (mis->_miAnimFrame == 13 || !PosOkActor(mis->_mix, mis->_miy)) {
			MissToMonst(mi);
			mis->_miDelFlag = TRUE;
			return;
		}
		mis->_mitxoff -= mis->_mixvel;
		mis->_mityoff -= mis->_miyvel;
	}
	GetMissilePos(mi);
	if (!PosOkActor(mis->_mix, mis->_miy)) {
		MissToMonst(mi);
		mis->_miDelFlag = TRUE;
		return;
	}
	bx = mis->_mix;
	by = mis->_miy;
	SetMonsterLoc(&monsters[mnum], bx, by);
	//assert(dMonster[bx][by] == 0);
	//assert(dPlayer[bx][by] == 0);
	dMonster[bx][by] = -(mnum + 1);
	monsters[mnum]._msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
	// assert(monsters[mnum]._mvid == NO_VISION);
	// assert(monsters[mnum].mlid == mis->_miLid);
	ChangeLightXY(monsters[mnum].mlid, bx, by);
	ShiftMissilePos(mi);
	PutMissile(mi);
}

void MI_Charge(int mi)
{
	MissileStruct* mis;
	int ax, ay, bx, by, pnum;

	mis = &missile[mi];
	pnum = mis->_miSource;
	if (plr._pmode != PM_CHARGE) {
		mis->_miDelFlag = TRUE;
		return;
	}
	mis->_miDist += mis->_miAnimAdd;
	// restore the real coordinates
	//GetMissilePos(mi);
	ax = mis->_mix;
	ay = mis->_miy;
	dPlayer[ax][ay] = 0;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (!PosOkActor(mis->_mix, mis->_miy)) {
		MissToPlr(mi, true);
		mis->_miDelFlag = TRUE;
		return;
	}
	bx = mis->_mix;
	by = mis->_miy;
	dPlayer[bx][by] = -(pnum + 1);
	if (ax != bx || ay != by) {
		SetPlayerLoc(&plr, bx, by);
		// assert(plr._plid == mis->_miLid);
		ChangeLightXY(plr._plid, bx, by);
		ChangeVisionXY(plr._pvid, bx, by);
		if (bx == mis->_miVar1 && by == mis->_miVar2) {
			MissToPlr(mi, false);
			mis->_miDelFlag = TRUE;
			return;
		}
	}
	if (pnum == mypnum /*&& ScrollInfo._sdir != SDIR_NONE*/) {
		assert(ScrollInfo._sdir != SDIR_NONE);
		ScrollInfo._sxoff = -mis->_mixoff;
		ScrollInfo._syoff = -mis->_miyoff;
		if (ViewX != bx || ViewY != by) {
			ViewX = bx; // - ScrollInfo._sdx;
			ViewY = by; // - ScrollInfo._sdy;
		}
	}
	//ShiftMissilePos(mi);
	PutMissile(mi);
}

/*void MI_Fireman(int mi)
{
	MissileStruct *mis;
	int mnum, ax, ay, bx, by, cx, cy, tnum;

	GetMissilePos(mi);
	mis = &missile[mi];
	ax = mis->_mix;
	ay = mis->_miy;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	mnum = mis->_miSource;
	bx = mis->_mix;
	by = mis->_miy;
	cx = monsters[mnum]._menemyx;
	cy = monsters[mnum]._menemyy;
	if ((bx != ax || by != ay)
	 && ((mis->_miVar1 && (abs(ax - cx) >= 4 || abs(ay - cy) >= 4)) || mis->_miVar2 > 1)
	 && PosOkMonst(mnum, ax, ay)) {
		MissToMonst(mi, ax, ay);
		mis->_miDelFlag = TRUE;
		return;
	}
	if (!(monsters[mnum]._mFlags & MFLAG_TARGETS_MONSTER)) {
		tnum = dPlayer[bx][by];
	} else {
		tnum = dMonster[bx][by];
	}
	if (!CheckAllowMissile(bx, by) || (tnum > 0 && !mis->_miVar1)) {
		mis->_mixvel *= -1;
		mis->_miyvel *= -1;
		mis->_miDir = OPPOSITE(mis->_miDir);
		mis->_miAnimData = monsters[mnum]._mAnims[MA_WALK].aData[mis->_miDir];
		mis->_miVar2++;
		if (tnum > 0)
			mis->_miVar1 = TRUE;
	}
	ShiftMissilePos(mi);
	PutMissile(mi);
}*/

void MI_WallC(int mi)
{
	MissileStruct *mis;
	int mitype, tx, ty;

	mis = &missile[mi];
	mis->_miDist++;
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
	}
//#ifdef HELLFIRE
//	mitype = mis->_miType == MIS_FIREWALLC ? MIS_FIREWALL : MIS_LIGHTWALL;
//#else
	mitype = MIS_FIREWALL;
//#endif
	// SetRndSeed(mis->_miRndSeed);
	if (!mis->_miVar8) {
		tx = mis->_miVar1;
		ty = mis->_miVar2;
		assert(IN_DUNGEON_AREA(tx, ty));
		if (!nMissileTable[dPiece[tx][ty]]) {
			AddMissile(tx, ty, 0, 0, 0, mitype, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
			mis->_miVar1 += XDirAdd[mis->_miVar3];
			mis->_miVar2 += YDirAdd[mis->_miVar3];
		} else {
			mis->_miVar8 = TRUE;
		}
	}
	if (!mis->_miVar7 && mis->_miDist != 1) {
		tx = mis->_miVar5;
		ty = mis->_miVar6;
		assert(IN_DUNGEON_AREA(tx, ty));
		if (!nMissileTable[dPiece[tx][ty]]) {
			AddMissile(tx, ty, 0, 0, 0, mitype, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
			mis->_miVar5 += XDirAdd[mis->_miVar4];
			mis->_miVar6 += YDirAdd[mis->_miVar4];
		} else {
			mis->_miVar7 = TRUE;
		}
	}
	// mis->_miRndSeed = GetRndSeed();
}

void MI_Inferno(int mi)
{
	MissileStruct* mis;
	int k;

	mis = &missile[mi];
	mis->_miAnimFlag = mis->_miVar2 == 0;
	if (!mis->_miAnimFlag) {
		mis->_miVar2--;
		return;
	}
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_WALL);
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	k = mis->_miAnimFrame;
	if (k > 11) {
		// assert(misfiledata[MFILE_INFERNO].mfAnimLen[0] < 24);
		k = 24 - k;
	}
	static_assert(MAX_LIGHT_RAD >= 12, "MI_Inferno needs at least light-radius of 12.");
	if (mis->_miLid == NO_LIGHT)
		mis->_miLid = AddLight(mis->_mix, mis->_miy, k);
	else
		ChangeLightRadius(mis->_miLid, k);
	PutMissile(mi);
}

void MI_InfernoC(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
		if (!nMissileTable[dPiece[mis->_mix][mis->_miy]]) {
			// SetRndSeed(mis->_miRndSeed);
			// mis->_miVar3 used by MIS_INFERNO !
			AddMissile(
			    mis->_mix,
			    mis->_miy,
			    0,
			    0,
			    mi,
			    MIS_INFERNO,
			    mis->_miCaster,
			    mis->_miSource,
			    mis->_miSpllvl);
			// mis->_miRndSeed = GetRndSeed();
		} else {
			mis->_miRange = 0;
		}
		mis->_miVar1 = mis->_mix;
		mis->_miVar2 = mis->_miy;
		mis->_miVar3++;
	}
	mis->_miRange--;
	if (mis->_miRange < 0 || mis->_miVar3 == 3)
		mis->_miDelFlag = TRUE;
}

/*void MI_FireTrap(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);

	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE;
		return;
	}
	PutMissileF(mi, BFLAG_HAZARD);
}*/

void MI_Cbolt(int mi)
{
	MissileStruct *mis;
	int md;
	int bpath[16] = { -1, 0, 1, -1, 0, 1, -1, -1, 0, 0, 1, 1, 0, 1, -1, 0 };

	mis = &missile[mi];
	if (mis->_miAnimType != MFILE_LGHNING) {
		if (mis->_miVar3 == 0) {
			md = (mis->_miVar2 + bpath[mis->_miVar4]) & 7;
			mis->_miVar4 = (mis->_miVar4 + 1) & 0xF;
			GetMissileVel(mi, 0, 0, XDirAdd[md], YDirAdd[md], MIS_SHIFTEDVEL(8));
			mis->_miVar3 = 16;
		} else {
			mis->_miVar3--;
		}
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		if ((mis->_mix != mis->_misx || mis->_miy != mis->_misy)
		 && CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY)) {
			static_assert(MAX_LIGHT_RAD >= 8, "MI_Cbolt needs at least light-radius of 8.");
			mis->_miVar1 = 8;
			mis->_miAnimType = MFILE_LGHNING;
			SetMissDir(mi, 0);
			mis->_miRange = misfiledata[MFILE_LGHNING].mfAnimLen[0] * misfiledata[MFILE_LGHNING].mfAnimFrameLen[0];
		}
		ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miVar1);
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid);
}

void MI_Elemental(int mi)
{
	MissileStruct *mis;
	int sd, cx, cy, dx, dy;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	cx = mis->_mix;
	cy = mis->_miy;
	if ((cx != mis->_misx || cy != mis->_misy)                       // not on the starting position
	 && (CheckMissileCol(mi, cx, cy, MICM_BLOCK_ANY) || mis->_miRange >= 0)   // did not hit a wall
	 && !mis->_miVar3 && cx == mis->_miVar4 && cy == mis->_miVar5) { // destination reached the first time
		mis->_miVar3 = TRUE;
		mis->_miRange = 0;
		if (FindClosest(cx, cy, dx, dy)) {
			sd = GetDirection8(cx, cy, dx, dy);
		} else {
			sd = plx(mis->_miSource)._pdir;
			dx = cx + XDirAdd[sd];
			dy = cy + YDirAdd[sd];
		}
		SetMissDir(mi, sd);
		GetMissileVel(mi, cx, cy, dx, dy, MIS_SHIFTEDVEL(16));
	}
	if (cx != mis->_miVar1 || cy != mis->_miVar2) {
		mis->_miVar1 = cx;
		mis->_miVar2 = cy;
		ChangeLightXY(mis->_miLid, cx, cy);
	}
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	//CheckMissileCol(mi, cx, cy, MICM_NONE);
	// TODO: mis->_miMinDam >>= 1; mis->_miMaxDam >>= 1; ?
	CheckSplashCol(mi);

	AddMissile(mis->_mix, mis->_miy, mi, 0, 0, MIS_EXFBALL, MST_NA, 0, 0);

	mis->_miDelFlag = TRUE;
	AddUnLight(mis->_miLid);
}

void MI_Resurrect(int mi)
{
	// assert(missile[mi]._miAnimLen == misfiledata[MFILE_RESSUR1].mfAnimLen[0]);
	if (missile[mi]._miAnimFrame < misfiledata[MFILE_RESSUR1].mfAnimLen[0]) {
		PutMissile(mi);
		return;
	}
	missile[mi]._miDelFlag = TRUE;
}

void ProcessMissiles()
{
	MissileStruct *mis;
	int i, mi;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		assert(IN_DUNGEON_AREA(mis->_mix, mis->_miy));
		dFlags[mis->_mix][mis->_miy] &= ~(BFLAG_MISSILE_PRE | BFLAG_HAZARD);
		dMissile[mis->_mix][mis->_miy] = 0;
	}

	for (i = 0; i < nummissiles; ) {
		mi = missileactive[i];
		if (missile[mi]._miDelFlag) {
			DeleteMissile(mi, i);
		} else {
			i++;
		}
	}

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		mis = &missile[mi];
		missiledata[mis->_miType].mProc(mi);
		if (mis->_miAnimFlag) {
			mis->_miAnimCnt++;
			if (mis->_miAnimCnt >= mis->_miAnimFrameLen) {
				mis->_miAnimCnt = 0;
				mis->_miAnimFrame += mis->_miAnimAdd;
				if (mis->_miAnimFrame > mis->_miAnimLen)
					mis->_miAnimFrame = 1;
				if (mis->_miAnimFrame < 1)
					mis->_miAnimFrame = mis->_miAnimLen;
			}
		}
	}

	for (i = 0; i < nummissiles; ) {
		mi = missileactive[i];
		if (missile[mi]._miDelFlag) {
			DeleteMissile(mi, i);
		} else {
			i++;
		}
	}
}

void SyncMissilesAnim()
{
	MissileStruct* mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		SyncMissAnim(missileactive[i]);
		if (mis->_miType == MIS_RHINO) {
			SyncRhinoAnim(missileactive[i]);
		} else if (mis->_miType == MIS_CHARGE) {
			SyncChargeAnim(missileactive[i]);
		}
	}
}

DEVILUTION_END_NAMESPACE
