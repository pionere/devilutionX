/**
 * @file missiles.cpp
 *
 * Implementation of missile functionality.
 */
#include "all.h"
#include "misproc.h"
#include "engine/render/cl2_render.h"

DEVILUTION_BEGIN_NAMESPACE

/*
 * Similar to walk offsetx/y with PLR/MON_WALK_SHIFT, missile velocity values
 * are shifted with MIS_*VELO_SHIFT to the higher range for better precision.
 * if MIS_VELO_SHIFT is set to 0 and MIS_BASE_VELO_SHIFT to 16:
 *    the result is reduced code size with slightly slower runtime speed.
 * if MIS_VELO_SHIFT is set to 16, MIS_BASE_VELO_SHIFT to 0:
 *    the result is increased code size with slightly better runtime speed.
 */
#define MIS_VELO_SHIFT      0
#define MIS_BASE_VELO_SHIFT 16
#define MIS_SHIFTEDVEL(x)   ((x) << MIS_VELO_SHIFT)

int missileactive[MAXMISSILES];
MissileStruct missile[MAXMISSILES];
int nummissiles;

// container for live data of missile-animations
static BYTE* misanimdata[NUM_MFILE + 1][16] = { { 0 } };
static int misanimdim[NUM_MFILE][2] = { 0 };

// TODO: merge XDirAdd/YDirAdd, offset_x/offset_y, bxadd/byadd, pathxdir/pathydir, plrxoff2/plryoff2, trm3x/trm3y
/** Maps from direction to X-offset. */
static const int XDirAdd[NUM_DIRS] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to Y-offset. */
static const int YDirAdd[NUM_DIRS] = { 1, 1, 1, 0, -1, -1, -1, 0 };

static_assert(MAX_LIGHT_RAD >= 9, "FireWallLight needs at least light-radius of 9.");
static const BYTE FireWallLight[14] = { 2, 2, 3, 4, 5, 5, 6, 7, 8, 9, 9, 8, 8, 8 };

static const BYTE BloodBoilLocs[][2] = {
	// clang-format off
	{ 3, 4 },  { 2, 1 },  { 3, 3 },  { 1, 1 },  { 2, 3 }, { 1, 0 },  { 4, 3 },  { 2, 2 },  { 3, 0 },  { 1, 2 }, 
	{ 2, 4 },  { 0, 1 },  { 4, 2 },  { 0, 3 },  { 2, 0 }, { 3, 2 },  { 1, 4 },  { 4, 1 },  { 0, 2 },  { 3, 1 }, { 1, 3 }
	// clang-format on
};

void GetSkillDetails(int sn, int sl, SkillDetails* skd)
{
	int k, type, magic, mind, maxd;

	assert((unsigned)mypnum < MAX_PLRS);
	assert((unsigned)sn < NUM_SPELLS);
	type = SDT_DAMAGE;
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
	case SPL_PULSE:
		k = (magic >> 2) + (sl << 2);
		mind = k * 3 / 4u;
		maxd = k * 5 / 2u;
		break;
	case SPL_NULL:
	case SPL_WALK:
	case SPL_BLOCK:
	case SPL_CHARGE:
	case SPL_RAGE:
	case SPL_SHROUD:
	case SPL_SWAMP:
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
	case SPL_OIL:
	case SPL_REPAIR:
	case SPL_RECHARGE:
	case SPL_DISARM:
#ifdef HELLFIRE
	case SPL_BUCKLE:
	case SPL_WHITTLE:
	case SPL_RUNESTONE:
#endif
		type = SDT_NONE;
		break;
	case SPL_ATTACK:
	case SPL_WHIPLASH:
	case SPL_WALLOP:
	case SPL_SWIPE:
		type = SDT_DAMAGE_MELEE;
		switch (sn) {
		case SPL_ATTACK:
			mind = maxd = 128; break;
		case SPL_WHIPLASH:
			mind = maxd = (128 * (24 + sl)) >> 6; break;
		case SPL_WALLOP:
			mind = maxd = (128 * (112 + sl)) >> 6; break;
		case SPL_SWIPE:
			mind = maxd = (128 * (48 + sl)) >> 6; break;
		default:
			ASSUME_UNREACHABLE
		}
		break;
	case SPL_RATTACK:
	case SPL_POINT_BLANK:
	case SPL_FAR_SHOT:
	case SPL_PIERCE_SHOT:
	case SPL_MULTI_SHOT:
		type = SDT_DAMAGE_RANGED;
		switch (sn) {
		case SPL_RATTACK:
			mind = maxd = 128; break;
		case SPL_POINT_BLANK:
			mind = 0; maxd = (128 * (64 + /*32 +*/ sl)) >> 6; break;
		case SPL_FAR_SHOT:
			mind = (128 * (8 * 2 - 16 + sl)) >> 5; maxd = 0; break;
		case SPL_PIERCE_SHOT:
			mind = maxd = (128 * (32 + sl)) >> 6; break;
		case SPL_MULTI_SHOT:
			mind = maxd = (128 * (16 + sl)) >> 6; break;
		default:
			ASSUME_UNREACHABLE
		}
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
	case SPL_BLOODBOIL:
		mind = (magic >> 2) + (sl << 2) + 10;
		maxd = (magic >> 2) + (sl << 3) + 10;
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
		sl = sl * 4 + (magic >> 6);
		sl = sl > 0 ? sl - 1 : 0;
		k = monsterdata[MT_GOLEM].mLevel;
		sl = k + sl;
		mind = sl * monsterdata[MT_GOLEM].mMinDamage / k;
		maxd = sl * monsterdata[MT_GOLEM].mMaxDamage / k;
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
	case SPL_POISON:
		mind = ((magic >> 4) + sl + 2) << (-3 + 5);
		maxd = ((magic >> 4) + sl + 4) << (-3 + 5);
		break;
	case SPL_WIND:
		mind = (magic >> 3) + 7 * sl + 1;
		maxd = (magic >> 3) + 8 * sl + 1;
		// (dam * 2 * misfiledata[MFILE_WIND].mfAnimLen[0] / 16) << (-3 + 5)
		mind = mind * 3;
		maxd = maxd * 3;
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

	skd->type = type;
	// if (type != SDT_NONE) {
		skd->v0 = mind;
		skd->v1 = maxd;
	// }
}

void RemovePortalMissile(int pnum)
{
	MissileStruct* mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		if (mis->_miType == MIS_TOWN && mis->_miSource == pnum) {
			mis->_miDelFlag = TRUE; // + AddUnLight
		}
	}
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

/*
 * Check if a missile can be placed at the given position.
 */
static bool PosOkMis2(int x, int y)
{
	// int oi;

	if (nMissileTable[dPiece[x][y]] != 0)
		return false;

	/* commented out for consistent behavior (firewall is propagated over solid objects)
	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (objects[oi]._oSolidFlag)
			return false;
	}*/

	return true;
}

static bool FindClosest(int sx, int sy, int& dx, int& dy)
{
	int j, i, mid, mnum, tx, ty;
	const int8_t* cr;
	MonsterStruct* mon;

	mid = dMonster[sx][sy];
	mid = mid >= 0 ? mid - 1 : -(mid + 1);

	static_assert(DBORDERX >= 15 && DBORDERY >= 15, "FindClosest expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 15, "FindClosest uses CrawlTable/CrawlNum up to radius 16.");
	for (i = 1; i <= 15; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mnum = dMonster[tx][ty] - 1;
			if (mnum < 0 || mnum == mid)
				continue;
			mon = &monsters[mnum];
			if (mon->_mhitpoints == 0)
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

static bool FindClosestChain(int sx, int sy, int& dx, int& dy)
{
	int j, i, mid, mnum, tx, ty;
	const int8_t* cr;
	MonsterStruct* mon;

	mid = dMonster[sx][sy];
	mid = mid >= 0 ? mid - 1 : -(mid + 1);

	static_assert(DBORDERX >= 7 && DBORDERY >= 7, "FindClosestChain expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 7, "FindClosestChain uses CrawlTable/CrawlNum up to radius 7.");
	for (i = 1; i <= 7; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mnum = dMonster[tx][ty] - 1;
			if (mnum < 0 || mnum == mid)
				continue;
			mon = &monsters[mnum];
			if (mon->_mhitpoints == 0
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

static void DoTeleport(int pnum, int dx, int dy)
{
	int px, py;

	px = plr._px;
	py = plr._py;
	dPlayer[px][py] = 0;
	//PlrClrTrans(px, py);

	px = dx;
	py = dy;
	SetPlayerLoc(&plr, px, py);
	//PlrDoTrans(px, py);
	dPlayer[px][py] = pnum + 1;
	ChangeLightXY(plr._plid, px, py);
	ChangeVisionXY(plr._pvid, px, py);
	if (pnum == mypnum) {
		myview.x = px; // - ScrollInfo._sdx;
		myview.y = py; // - ScrollInfo._sdy;
	}
}

#define GetDirection8 GetDirection

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
	const int BaseDirs[4] = { 14 + 2, 10 + 2, 2 + 2, 6 + 2 };
	int dir = BaseDirs[2 * (dx < 0) + (dy < 0)];
	//const int DeltaDirs[2][8] = { { 0, 1, 2, 3, 4 }, { 4, 3, 2, 1, 0 } };
	const int DeltaDirs[2][4] = { { 0 - 2, 1 - 2, 3 - 2, 4 - 2 }, { 4 - 2, 3 - 2 , 1 - 2, 0 - 2 } };
	//const int(&DeltaDir)[8] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	const int(&DeltaDir)[4] = DeltaDirs[(dx < 0) ^ (dy < 0)];
	if (3 * adx <= (ady << 1)) {
		//dir += DeltaDir[5 * adx < ady ? 4 : 3];
		dir += DeltaDir[5 * adx < ady ? 3 : 2];
	} else if (3 * ady <= (adx << 1)) {
		dir += DeltaDir[5 * ady < adx ? 0 : 1];
	} // else
	//	dir += DeltaDir[2];
	return dir & 15;
#endif
}

static void DeleteMissile(int mi, int idx)
{
	AddUnLight(missile[mi]._miLid);

	nummissiles--;
	assert(missileactive[idx] == mi);
	missileactive[idx] = missileactive[nummissiles];
	missileactive[nummissiles] = mi;

}

static void DeleteMissiles()
{
	int i, mi;

	for (i = 0; i < nummissiles; ) {
		mi = missileactive[i];
		if (missile[mi]._miDelFlag) {
			DeleteMissile(mi, i);
		} else {
			i++;
		}
	}
}

static void PutMissile(int mi)
{
	int x, y;

	x = missile[mi]._mix;
	y = missile[mi]._miy;
	assert(IN_DUNGEON_AREA(x, y));
	//assert(missile[mi]._miDrawFlag); -- not really necessary, but otherwise it is pointless
	assert(!missile[mi]._miDelFlag);
	//if (!missile[mi]._miDelFlag) {
		dMissile[x][y] = dMissile[x][y] == 0 ? mi + 1 : MIS_MULTI;
		assert(!missile[mi]._miPreFlag);
		//if (missile[mi]._miPreFlag)
		//	dFlags[x][y] |= BFLAG_MISSILE_PRE;
	// }
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
	MissileStruct* mis;
	int mx, my, dx, dy, dqx, dqy;

	mis = &missile[mi];
	mx = mis->_mitxoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);
	my = mis->_mityoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT);
	if ((mis->_mitxoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT - 1) & 1))
		mx++;
	if ((mis->_mityoff >> (MIS_BASE_VELO_SHIFT + MIS_VELO_SHIFT - 1) & 1))
		my++;

	dx = mx + my;
	dy = my - mx;
	// -- keep in sync with LineClearF + do it in GetMissileVel?
	//dx += dx >= 0 ? 32 : -32;
	//dy += dy >= 0 ? 32 : -32;

	dqx = dx / 64;
	//drx = dx % 64;
	dqy = dy / 64;
	//dry = dy % 64;

	mis->_mix = dqx + mis->_misx;
	mis->_miy = dqy + mis->_misy;
	mis->_mixoff = (mx - (dqx - dqy) * 32) * ASSET_MPL;        // ((drx - dry) >> 1) * ASSET_MPL;
	mis->_miyoff = ((my >> 1) - (dqx + dqy) * 16) * ASSET_MPL; // ((drx + dry) >> 2) * ASSET_MPL;
	ChangeLightScreenOff(mis->_miLid, mis->_mixoff, mis->_miyoff);
}
#if 0
/**
 * Shift the missiles coordinates to place it in front of the view.
 * @param mi the missile to be shifted
 */
static void ShiftMissilePos(int mi)
{
	MissileStruct* mis;
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
#endif
unsigned CalcMonsterDam(unsigned mor, BYTE mRes, unsigned mindam, unsigned maxdam, bool penetrates)
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

int AddElementalExplosion(int fdam, int ldam, int mdam, int adam, bool isMonster, int mpnum)
{
	int dam = fdam + ldam + mdam + adam;
	int mtype, mx, my, mxoff, myoff, mi;

	if (dam == 0) {
		return 0;
	}

	if ((fdam | ldam) >= (mdam | adam)) {
		mtype = fdam >= ldam ? MIS_EXFIRE : MIS_EXLGHT;
	} else {
		mtype = mdam >= adam ? MIS_EXMAGIC : MIS_EXACID;
	}
	if (isMonster) {
		mx = monsters[mpnum]._mx;
		my = monsters[mpnum]._my;
		mxoff = monsters[mpnum]._mxoff;
		myoff = monsters[mpnum]._myoff;
	} else {
		mx = plx(mpnum)._px;
		my = plx(mpnum)._py;
		mxoff = plx(mpnum)._pxoff;
		myoff = plx(mpnum)._pyoff;
	}
	mi = AddMissile(mx, my, -1, 0, 0, mtype, MST_NA, 0, 0);
	if (mi >= 0) {
		missile[mi]._mixoff = mxoff;
		missile[mi]._miyoff = myoff;
	}
	/*int gfx = random_(8, dam);
	if (gfx >= dam - (fdam + ldam)) {
		if (gfx < dam - ldam) {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPFEXP, MST_NA, 0, 0);
		} else {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPLEXP, MST_NA, 0, 0);
		}
	}*/
	return dam;
}

static int MissDirection(const MissileStruct* mis, int adir, int ax, int ay)
{
	int dir;
	if (mis->_miFlags & MIF_GUIDED) {
		dir = mis->_miVar5; // MIS_DIR
	} else if ((mis->_miFlags & MIF_AREA) || (ax == mis->_misx && ay == mis->_misy)) {
		dir = OPPOSITE(adir);
	} else {
		dir = GetDirection8(mis->_misx, mis->_misy, ax, ay);
	}
	return dir;
}

static bool MissMonHitByMon(int mnum, int mi)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	int misource, hper, dir, dam;
	bool ret;

	mon = &monsters[mnum];
	mis = &missile[mi];
	if (!(mis->_miFlags & MIF_DOT)) {
		if (mis->_miVar8 == mnum + 1)
			return false;
		mis->_miVar8 = mnum + 1;
	}
	misource = mis->_miSource;
	// assert(misource == -1 || ((unsigned)misource >= MAX_MINIONS && (unsigned)misource < MAXMONSTERS));
	if (mnum >= MAX_MINIONS && misource >= MAX_MINIONS) {
		return false; // monster vs. monster
	}
	// if (mnum < MAX_MINIONS && (unsigned)misource < MAX_MINIONS && plx(mnum)._pTeam == plx(misource)._pTeam)
	//	return false; // minion vs. minion
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = NextRndSeed();
	if (mis->_miFlags & MIF_ARROW) {
		hper = mis->_miVar6; // MISHIT
		hper -= mon->_mArmorClass;
		hper -= mis->_miVar7 << 1; // MISDIST
	} else if (mis->_miFlags & MIF_AREA) {
		hper = 40 + (misource >= 0 ? 2 * monsters[misource]._mLevel : 2 * currLvl._dLevel);
		hper -= 2 * mon->_mLevel;
	} else {
		hper = 50 + (misource >= 0 ? monsters[misource]._mMagic : 2 * currLvl._dLevel);
		hper -= 2 * mon->_mLevel + mon->_mEvasion;
		// hper -= dist; // TODO: either don't care about it, or set it!
	}
	if (!CheckHit(hper) && mon->_mmode != MM_STONE)
		return false;

	dam = CalcMonsterDam(mon->_mMagicRes, mis->_miResist, mis->_miMinDam, mis->_miMaxDam, false);
	if (dam == 0)
		return false;

	if (!CheckMonsterHit(mnum, &ret)) {
		return ret;
	}

	mon->_mhitpoints -= dam;
	if (mon->_mhitpoints < (1 << 6)) {
		MonKill(mnum, misource);
	} else {
		/*if (resist != MORT_NONE) {
			PlayMonSfx(mnum, MS_GOTHIT);
		} else {*/
			dir = MissDirection(mis, mon->_mdir, mon->_mx, mon->_my);
			MonHitByMon(mnum, misource, dam, dir);
		//}
	}
	if (mon->_msquelch != SQUELCH_MAX) {
		mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
		// lead the monster to a fixed location
		mon->_mlastx = mis->_misx;
		mon->_mlasty = mis->_misy;
	}
	return true;
}

static bool MissMonHitByPlr(int mnum, int mi)
{
	MonsterStruct* mon;
	MissileStruct* mis;
	int pnum, hper, dir, dam, lx, ly;
	unsigned hitFlags;
	bool tmac, ret;

	mon = &monsters[mnum];
	mis = &missile[mi];
	if (!(mis->_miFlags & MIF_DOT)) {
		if (mis->_miVar8 == mnum + 1)
			return false;
		mis->_miVar8 = mnum + 1;
	}
	pnum = mis->_miSource;
	// assert((unsigned)pnum < MAX_PLRS);
	// if (mnum < MAX_MINIONS && plx(mnum)._pTeam == plr._pTeam)
	//	return false; // player vs. minion
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = NextRndSeed();
	if (mis->_miFlags & MIF_ARROW) {
		hper = plr._pIHitChance;
		hper -= mon->_mArmorClass;
		hper -= (mis->_miVar7 * mis->_miVar7 >> 1); // MISDIST
	} else if (mis->_miFlags & MIF_AREA) {
		hper = 40 + 2 * plr._pLevel;
		hper -= 2 * mon->_mLevel;
	} else {
		hper = 50 + plr._pMagic;
		hper -= 2 * mon->_mLevel + mon->_mEvasion;
		// hper -= dist; // TODO: either don't care about it, or set it!
	}
	if (!CheckHit(hper) && mon->_mmode != MM_STONE)
		return false;

	if (mis->_miFlags & MIF_ARROW) {
		// calculate arrow-damage
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
		switch (mis->_miType) {
		case MIS_ARROW:
			break;
		case MIS_PBARROW: // SPL_POINT_BLANK
			// assert(mis->_miVar7 <= 6); -- guaranteed by _miRange
			dam = (dam * (64 + 32 - 16 * mis->_miVar7 + mis->_miSpllvl)) >> 6; // MISDIST
			break;
		case MIS_ASARROW: // SPL_FAR_SHOT
			// assert(mis->_miVar7 >= 2);
			dam = (dam * (8 * mis->_miVar7 - 16 + mis->_miSpllvl)) >> 5; // MISDIST
			break;
		case MIS_MLARROW: // SPL_MULTI_SHOT
			dam = (dam * (16 + mis->_miSpllvl)) >> 6;
			break;
		case MIS_PCARROW: // SPL_PIERCE_SHOT
			dam = (dam * (32 + mis->_miSpllvl)) >> 6;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
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
		dam += AddElementalExplosion(fdam, ldam, mdam, adam, true, mnum);
	} else {
		dam = CalcMonsterDam(mon->_mMagicRes, mis->_miResist, mis->_miMinDam, mis->_miMaxDam, false);
	}
	if (dam <= 0) {
		dam = 1;
	}

	if (!CheckMonsterHit(mnum, &ret))
		return ret;

	//if (pnum == mypnum) {
		mon->_mhitpoints -= dam;
	//}
	if (mis->_miFlags & MIF_ARROW) {
		if (plr._pILifeSteal != 0) {
			PlrIncHp(pnum, (dam * plr._pILifeSteal) >> 7);
		}
		if (plr._pIManaSteal != 0) {
			PlrIncMana(pnum, (dam * plr._pIManaSteal) >> 7);
		}
	}

	if (mon->_mhitpoints < (1 << 6)) {
		MonKill(mnum, pnum);
	} else {
		/*if (resist != MORT_NONE) {
			PlayMonSfx(mnum, MS_GOTHIT);
		} else {*/
			hitFlags = 0;
			if (mis->_miFlags & MIF_ARROW) {
				hitFlags = (plr._pIFlags & ISPL_HITFLAGS_MASK) | ISPL_FAKE_CAN_BLEED;
				//if (hitFlags & ISPL_NOHEALMON)
				//	mon->_mFlags |= MFLAG_NOHEAL;
			}
			dir = MissDirection(mis, mon->_mdir, mon->_mx, mon->_my);
			MonHitByPlr(mnum, pnum, dam, hitFlags, dir);
		//}
	}

	if (mon->_msquelch != SQUELCH_MAX) {
		mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
		// lead the monster to the player
		lx = mis->_misx;
		ly = mis->_misy;
		if (mis->_miFlags & MIF_LEAD) {
#ifdef HELLFIRE
			if (mis->_miCaster == MST_PLAYER) // only if the missile is not from a rune
#endif
			{
				lx = plr._px;
				ly = plr._py;
			}
		}
		mon->_mlastx = lx;
		mon->_mlasty = ly;
	}
	return true;
}

unsigned CalcPlrDam(int pnum, BYTE mRes, unsigned mindam, unsigned maxdam)
{
	int dam;
	int8_t resist;

	switch (mRes) {
	case MISR_NONE:
	case MISR_SLASH: // TODO: add plr._pSlash/Blunt/PunctureResist
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

static bool MissPlrHitByMon(int pnum, int mi)
{
	MissileStruct* mis;
	int misource, hper, dir, dam;
	unsigned hitFlags;

	mis = &missile[mi];
	if (!(mis->_miFlags & MIF_DOT)) {
		if (mis->_miVar8 == -(pnum + 1))
			return false;
		mis->_miVar8 = -(pnum + 1);
	}
	misource = mis->_miSource;
	// assert(misource == -1 || ((unsigned)misource >= MAX_MINIONS && (unsigned)misource < MAXMONSTERS));
	//if ((unsigned)misource < MAX_MINIONS && plx(misource)._pTeam == plr._pTeam)
	//	return false; // minion vs. player
	if (plr._pInvincible) {
		return false;
	}
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = NextRndSeed();
	if (mis->_miFlags & MIF_ARROW) {
		hper = mis->_miVar6; // MISHIT
		hper -= plr._pIAC;
		hper -= mis->_miVar7 << 1; // MISDIST
	} else if (mis->_miFlags & MIF_AREA) {
		hper = 40 + (misource >= 0 ? 2 * monsters[misource]._mLevel : 2 * currLvl._dLevel);
		hper -= 2 * plr._pLevel;
	} else {
		hper = 50 + (misource >= 0 ? monsters[misource]._mMagic : 2 * currLvl._dLevel);
		hper -= plr._pIEvasion;
		// hper -= dist; // TODO: either don't care about it, or set it!
	}
	if (!CheckHit(hper))
		return false;

	dir = MissDirection(mis, plr._pdir, plr._px, plr._py);
	if (!(mis->_miFlags & MIF_AREA)) {
		if (PlrCheckBlock(pnum, misource >= 0 ? monsters[misource]._mLevel : currLvl._dLevel, OPPOSITE(dir)))
			return true;
	}

	dam = CalcPlrDam(pnum, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
	if (!(mis->_miFlags & MIF_DOT)) {
		dam -= plr._pIAbsAnyHit;
		// assert(mis->_miResist != MISR_SLASH && mis->_miResist != MISR_PUNCTURE);
		if (/*mis->_miResist == MISR_SLASH || */mis->_miResist == MISR_BLUNT/* || mis->_miResist == MISR_PUNCTURE*/)
			dam -= plr._pIAbsPhyHit;
	}
	if (dam <= 0) {
		dam = 1;
	}

	if (!PlrDecHp(pnum, dam, DMGTYPE_NPC)) {
		hitFlags = 0;
		if (mis->_miFlags & MIF_ARROW) {
			hitFlags = (misource >= 0 ? monsters[misource]._mFlags & ISPL_HITFLAGS_MASK : 0) | ISPL_FAKE_CAN_BLEED;
			static_assert((int)MFLAG_KNOCKBACK == (int)ISPL_KNOCKBACK, "MissPlrHitByMon uses _mFlags as hitFlags.");
		}
		PlrHitByAny(pnum, misource, dam, hitFlags, dir);
	}
	return true;
}

static bool MissPlrHitByPlr(int pnum, int mi)
{
	MissileStruct* mis;
	int offp, hper, dir, dam;
	unsigned hitFlags;

	mis = &missile[mi];
	if (!(mis->_miFlags & MIF_DOT)) {
		if (mis->_miVar8 == -(pnum + 1))
			return false;
		mis->_miVar8 = -(pnum + 1);
	}
	offp = mis->_miSource;
	// assert((unsigned)offp < MAX_PLRS);
	if (plr._pTeam == plx(offp)._pTeam || plr._pInvincible) {
		return false; // player vs. player
	}
	// SetRndSeed(mis->_miRndSeed);
	// mis->_miRndSeed = NextRndSeed();
	if (mis->_miFlags & MIF_ARROW) {
		hper = plx(offp)._pIHitChance;
		hper -= plr._pIAC;
		hper -= (mis->_miVar7 * mis->_miVar7 >> 1); // MISDIST
	} else if (mis->_miFlags & MIF_AREA) {
		hper = 40 + 2 * plx(offp)._pLevel;
		hper -= 2 * plr._pLevel;
	} else {
		hper = 50 + plx(offp)._pMagic;
		hper -= plr._pIEvasion;
		// hper -= dist; // TODO: either don't care about it, or set it!
	}
	if (!CheckHit(hper))
		return false;

	dir = MissDirection(mis, plr._pdir, plr._px, plr._py);
	if (!(mis->_miFlags & MIF_AREA)) {
		if (PlrCheckBlock(pnum, 2 * plx(offp)._pLevel, OPPOSITE(dir)))
			return true;
	}

	if (mis->_miFlags & MIF_ARROW) {
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
		switch (mis->_miType) {
		case MIS_ARROW:
			break;
		case MIS_PBARROW: // SPL_POINT_BLANK
			// assert(mis->_miVar7 <= 6); -- guaranteed by _miRange
			dam = (dam * (64 + 32 - 16 * mis->_miVar7 + mis->_miSpllvl)) >> 6; // MISDIST
			break;
		case MIS_ASARROW: // SPL_FAR_SHOT
			// assert(mis->_miVar7 >= 2);
			dam = (dam * (8 * mis->_miVar7 - 16 + mis->_miSpllvl)) >> 5; // MISDIST
			break;
		case MIS_MLARROW: // SPL_MULTI_SHOT
			dam = (dam * (16 + mis->_miSpllvl)) >> 6;
			break;
		case MIS_PCARROW: // SPL_PIERCE_SHOT
			dam = (dam * (32 + mis->_miSpllvl)) >> 6;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}

		// assert(!(mis->_miFlags & MIF_DOT));
		dam -= plr._pIAbsAnyHit;
		// assert(mis->_miResist == MISR_SLASH || mis->_miResist == MISR_BLUNT || mis->_miResist == MISR_PUNCTURE);
		dam -= plr._pIAbsPhyHit;
		if (dam > 0 && plx(offp)._pILifeSteal != 0) {
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
		dam += AddElementalExplosion(fdam, ldam, mdam, adam, false, pnum);
	} else {
		dam = CalcPlrDam(pnum, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
		dam >>= 1;
		if (!(mis->_miFlags & MIF_DOT)) {
			dam -= plr._pIAbsAnyHit;
			// assert(mis->_miResist != MISR_SLASH && mis->_miResist != MISR_PUNCTURE);
			if (/*mis->_miResist == MISR_SLASH || */mis->_miResist == MISR_BLUNT/* || mis->_miResist == MISR_PUNCTURE*/)
				dam -= plr._pIAbsPhyHit;
		}
	}

	if (dam <= 0) {
		dam = 1;
	}

	if (!PlrDecHp(pnum, dam, DMGTYPE_PLAYER)) {
		hitFlags = 0;
		if (mis->_miFlags & MIF_ARROW)
			hitFlags = (plx(mis->_miSource)._pIFlags & ISPL_HITFLAGS_MASK) | ISPL_FAKE_CAN_BLEED;
		PlrHitByAny(pnum, mis->_miSource, dam, hitFlags, dir);
	}
	return true;
}

static bool MonMissHit(int mnum, int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	if (mis->_miCaster & MST_PLAYER) {
		// player vs. monster
		return MissMonHitByPlr(mnum, mi);
	} else {
		// monster/trap vs. monster
		return MissMonHitByMon(mnum, mi);
	}
}

static bool PlrMissHit(int pnum, int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	if (mis->_miCaster & MST_PLAYER) {
		// player vs. player
		return MissPlrHitByPlr(pnum, mi);
	} else {
		// monster/trap vs. player
		return MissPlrHitByMon(pnum, mi);
	}
}

/**
 * Check if the monster is on a given tile.
 */
int CheckMonCol(int mnum)
{
	MonsterStruct* mon;
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
	// assert(mon->_mAnims[MA_WALK].maFrames == mon->_mAnimLen);
	halfOver = mon->_mAnimFrame > (mon->_mAnimLen >> 1);
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
	// assert(plr._pAnims[PGX_WALK].paFrames == plr._pAnimLen);
	halfOver = plr._pAnimFrame > (plr._pAnimLen >> 1);
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
 * @return what was hit (0: nothing, 1: actor, 2: object, 3: wall)
 */
static int CheckMissileCol(int mi, int mx, int my, missile_collision_mode mode)
{
	MissileStruct* mis;
	const MissileData* mds;
	int oi, mnum, pnum;
	int hit = 0;

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
		hit = 3;
	}

	mnum = dMonster[mx][my];
	if (mnum != 0) {
		mnum = CheckMonCol(mnum);
		if (mnum >= 0 && MonMissHit(mnum, mi))
			hit = 1;
	}

	pnum = dPlayer[mx][my];
	if (pnum != 0) {
		pnum = CheckPlrCol(pnum);
		if (pnum >= 0 && PlrMissHit(pnum, mi))
			hit = 1;
	}

	if (hit == 0)
		return hit;

	if (mode != MICM_NONE) {
		mis = &missile[mi];
		if (mode == MICM_BLOCK_ANY || (hit != 1 /*&& mode == MICM_BLOCK_WALL*/))
			mis->_miRange = -1;
		mds = &missiledata[mis->_miType];
		if (SFX_VALID(mds->miSFX))
			PlaySfxLocN(mds->miSFX, mis->_mix, mis->_miy, mds->miSFXCnt);
	}
	return hit;
}

static void CheckSplashColFull(int mi)
{
	MissileStruct* mis;
	int i, sx, sy, mx, my;

	mis = &missile[mi];
	mx = mis->_mix;
	my = mis->_miy;

	// monster/player/object hit
	//  - adjust source position for directional hit
	sx = mis->_misx;
	sy = mis->_misy;
	mis->_misx = mx;
	mis->_misy = my;
	//  - hit everything around
	for (i = 0; i < lengthof(XDirAdd); i++) {
		CheckMissileCol(mi, mx + XDirAdd[i], my + YDirAdd[i], MICM_NONE);
	}
	// - restore source position
	mis->_misx = sx;
	mis->_misy = sy;
}

static void CheckSplashCol(int mi, int hit)
{
	MissileStruct* mis;
	int i, sx, sy, mx, my, lx, ly, tx, ty;

	if (hit != 3) {
		CheckSplashColFull(mi);
		return;
	}

	// wall hit:
	mis = &missile[mi];
	mx = mis->_mix;
	my = mis->_miy;
	//  - move missile back a bit to indicate the displacement
	mis->_mitxoff -= mis->_mixvel;
	mis->_mityoff -= mis->_miyvel;
	GetMissilePos(mi);

	//  - limit the explosion area
	lx = mis->_mix;
	ly = mis->_miy;

	//  - alter offset for better visual
	if (mis->_mixoff >= TILE_WIDTH / 2) {
		mis->_mixoff -= TILE_WIDTH;
		mis->_mix++;
		mis->_miy--;
	}

	//mis->_mitxoff += mis->_mixvel;
	//mis->_mityoff += mis->_miyvel;
	//GetMissilePos(mi);

	// assert(lx != mx || ly != my);
	//  - adjust source position for directional hit
	sx = mis->_misx;
	sy = mis->_misy;
	mis->_misx = mx;
	mis->_misy = my;
	//  - hit around in a limited area
	for (i = 0; i < lengthof(XDirAdd); i++) {
		tx = mx + XDirAdd[i];
		ty = my + YDirAdd[i];
		if (abs(tx - lx) < 2 && abs(ty - ly) < 2)
			CheckMissileCol(mi, tx, ty, MICM_NONE);
	}
	// - restore source position
	mis->_misx = sx;
	mis->_misy = sy;
}

static void SyncMissAnim(int mi)
{
	MissileStruct* mis;
	const MisFileData* mfd;
	int dir, animtype;

	mis = &missile[mi];
	animtype = mis->_miFileNum;
	dir = mis->_miDir;
	mis->_miAnimData = misanimdata[animtype][dir];
	mfd = &misfiledata[animtype];
	static_assert(offsetof(MissileStruct, _miAnimFlag) == offsetof(MissileStruct, _miDrawFlag) + 1, "SyncMissAnim uses DWORD-memcpy to optimize performance I.");
	static_assert(offsetof(MissileStruct, _miLightFlag) == offsetof(MissileStruct, _miDrawFlag) + 2, "SyncMissAnim uses DWORD-memcpy to optimize performance II.");
	static_assert(offsetof(MissileStruct, _miPreFlag) == offsetof(MissileStruct, _miDrawFlag) + 3, "SyncMissAnim uses DWORD-memcpy to optimize performance III.");
	static_assert(offsetof(MisFileData, mfAnimFlag) == offsetof(MisFileData, mfDrawFlag) + 1, "SyncMissAnim uses DWORD-memcpy to optimize performance IV.");
	static_assert(offsetof(MisFileData, mfLightFlag) == offsetof(MisFileData, mfDrawFlag) + 2, "SyncMissAnim uses DWORD-memcpy to optimize performance V.");
	static_assert(offsetof(MisFileData, mfPreFlag) == offsetof(MisFileData, mfDrawFlag) + 3, "SyncMissAnim uses DWORD-memcpy to optimize performance VI.");
	*(uint32_t*)&mis->_miDrawFlag = *(uint32_t*)&mfd->mfDrawFlag;
	mis->_miAnimFrameLen = mfd->mfAnimFrameLen;
	mis->_miAnimLen = mfd->mfAnimLen[dir];
	mis->_miAnimWidth = misanimdim[animtype][0];
	mis->_miAnimXOffset = misanimdim[animtype][1];
}

static void SyncRhinoAnim(int mi)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	MonAnimStruct* anim;

	mis = &missile[mi];
	assert(mis->_miAnimFlag);

	mon = &monsters[mis->_miSource];
	anim = &mon->_mAnims[
		(mon->_mFileNum == MOFILE_RHINO) ? MA_SPECIAL :
		(mon->_mFileNum == MOFILE_SNAKE) ? MA_ATTACK : MA_WALK];

	mis->_miAnimData = anim->maAnimData[mis->_miDir];
	mis->_miAnimFrameLen = anim->maFrameLen;
	mis->_miAnimLen = anim->maFrames;
	mis->_miAnimWidth = mon->_mAnimWidth;
	mis->_miAnimXOffset = mon->_mAnimXOffset;
	mis->_miAnimAdd = mon->_mFileNum == MOFILE_SNAKE ? 2 : 1;
	// if (mon->_muniqtype != 0) {
		mis->_miUniqTrans = mon->_muniqtrans;
	// }
}

static void SyncChargeAnim(int mi)
{
	MissileStruct* mis;
	int pnum;
	PlrAnimStruct* anim;

	mis = &missile[mi];
	assert(mis->_miAnimFlag);

	pnum = mis->_miSource;
	anim = &plr._pAnims[PGX_WALK];

	mis->_miAnimData = anim->paAnimData[mis->_miDir];
	mis->_miAnimFrameLen = PlrAnimFrameLens[PGX_WALK];
	mis->_miAnimLen = anim->paFrames;
	mis->_miAnimWidth = anim->paAnimWidth;
	mis->_miAnimXOffset = (anim->paAnimWidth - TILE_WIDTH) >> 1;
}

static void SetMissAnim(int mi, int dir)
{
	missile[mi]._miDir = dir;
	// assert(gbGameLogicProgress < GLP_MISSILES_DONE);
	missile[mi]._miAnimCnt = -1;
	missile[mi]._miAnimFrame = 1;
	SyncMissAnim(mi);
}

static void LoadMissileGFX(BYTE midx)
{
	char pszName[DATA_ARCHIVE_MAX_PATH];
	int i, n;
	BYTE** mad;
	const char* name;
	const char* fmt;
	const MisFileData* mfd;

	mad = misanimdata[midx];
	if (mad[0] != NULL)
		return;
	mfd = &misfiledata[midx];
	n = mfd->mfAnimFAmt;
	name = mfd->mfName;
	fmt = n == 1 ? "Missiles\\%s.CL2" : "Missiles\\%s%d.CL2";
	for (i = 0; i < n; i++) {
		snprintf(pszName, sizeof(pszName), fmt, name, i + 1);
		assert(mad[i] == NULL);
		mad[i] = LoadFileInMem(pszName);
	}
	misanimdim[midx][0] = Cl2Width(mad[0]);
	misanimdim[midx][1] = (misanimdim[midx][0] - TILE_WIDTH) >> 1;
	if (mfd->mfAnimTrans != NULL) {
		BYTE trn[NUM_COLORS];
		LoadFileWithMem(mfd->mfAnimTrans, trn);

		for (i = 0; i < n; i++) {
			Cl2ApplyTrans(mad[i], trn);
		}
	}
}

void InitGameMissileGFX()
{
	int i;

	for (i = 0; i < NUM_FIXMFILE; i++) {
		LoadMissileGFX(i);
	}
}

void InitMissileGFX(int mitype)
{
	BYTE midx = missiledata[mitype].mFileNum;
	if (midx > NUM_FIXMFILE) {
		LoadMissileGFX(midx);
	}
	// if (mitype == MIS_FIREBOLT) {
	//	LoadMissileGFX(MFILE_MAGBLOS); // InitMissileGFX(MIS_EXFIRE)
	// }
	// if (mitype == MIS_MAGMABALL) {
	//	LoadMissileGFX(MFILE_MAGBLOS); // InitMissileGFX(MIS_EXFIRE)
	// }
	// if (mitype == MIS_FIREBALL) {
	//	LoadMissileGFX(MFILE_BIGEXP); // InitMissileGFX(MIS_EXFBALL)
	// }
	// if (mitype == MIS_HBOLT) {
	//	LoadMissileGFX(MFILE_HOLYEXPL); // InitMissileGFX(MIS_EXHOLY)
	// }
	// if (mitype == MIS_FLARE) {
	//	LoadMissileGFX(MFILE_FLAREEXP); // InitMissileGFX(MIS_EXFLARE)
	// }
	if (mitype == MIS_SNOWWICH) {
		LoadMissileGFX(MFILE_SCBSEXPB); // InitMissileGFX(MIS_EXSNOWWICH)
	}
	if (mitype == MIS_HLSPWN) {
		LoadMissileGFX(MFILE_SCBSEXPD); // InitMissileGFX(MIS_EXHLSPWN)
	}
	if (mitype == MIS_SOLBRNR) {
		LoadMissileGFX(MFILE_SCBSEXPC); // InitMissileGFX(MIS_EXSOLBRNR)
	}
	if (mitype == MIS_MAGE) {
		LoadMissileGFX(MFILE_MAGEEXP); // InitMissileGFX(MIS_EXMAGE)
	}
	if (mitype == MIS_ACID) {
		LoadMissileGFX(MFILE_ACIDSPLA); // InitMissileGFX(MIS_EXACIDP)
		LoadMissileGFX(MFILE_ACIDPUD);  // InitMissileGFX(MIS_ACIDPUD)
	}
	// if (mitype == MIS_LIGHTNINGC) {
	//	LoadMissileGFX(MFILE_LGHNING); // InitMissileGFX(MIS_LIGHTNING)
	// }
	if (mitype == MIS_LIGHTNINGC2) {
		LoadMissileGFX(MFILE_THINLGHT); // InitMissileGFX(MIS_LIGHTNING2)
	}
	// if (mitype == MIS_BLOODBOILC) {
	//	LoadMissileGFX(MFILE_BLODBURS); // InitMissileGFX(MIS_BLOODBOIL)
	// }
	// if (mitype == MIS_SWAMPC) {
	//	LoadMissileGFX(MFILE_SWAMP); // InitMissileGFX(MIS_SWAMP)
	// }
	// if (mitype == MIS_FLASH) {
	//	LoadMissileGFX(MFILE_BLUEXBK); // InitMissileGFX(MIS_FLASH2)
	// }
	// if (mitype == MIS_STONE) {
	//	LoadMissileGFX(MFILE_SHATTER1); // InitMissileGFX(MIS_EXSTONE)
	// }
	// if (mitype == MIS_FIREWALLC) {
	//	LoadMissileGFX(MFILE_FIREWAL); // InitMissileGFX(MIS_FIREWALL)
	// }
	// if (mitype == MIS_FIREWAVEC) {
	//	LoadMissileGFX(MFILE_FIREWAL); // InitMissileGFX(MIS_FIREWAVE)
	// }
	// if (mitype == MIS_METEOR) {
	//	LoadMissileGFX(MFILE_FIREBA);
	//	LoadMissileGFX(MFILE_FIREWAL); // InitMissileGFX(MIS_FIREWALL)
	// }
	// if (mitype == MIS_LIGHTNOVAC) {
	//	LoadMissileGFX(MFILE_LGHNING); // InitMissileGFX(MIS_LIGHTBALL)
	// }
	// if (mitype == MIS_INFERNOC) {
	//	LoadMissileGFX(MFILE_INFERNO); // InitMissileGFX(MIS_INFERNO)
	// }
	// if (mitype == MIS_CBOLTC) {
	//	LoadMissileGFX(MFILE_MINILTNG); // InitMissileGFX(MIS_CBOLT)
	//	LoadMissileGFX(MFILE_LGHNING);
	// }
	if (mitype == MIS_APOCAC2) {
		LoadMissileGFX(MFILE_FIREPLAR); // InitMissileGFX(MIS_EXAPOCA2)
	}
	// if (mitype == MIS_PULSE) {
	//	LoadMissileGFX(MFILE_MINILTNG);
	// }
#ifdef HELLFIRE
	// if (mitype == MIS_FIRERING) {
	//	LoadMissileGFX(MFILE_FIREWAL); // InitMissileGFX(MIS_FIREWALL)
	// }
	// if (mitype == MIS_RUNEFIRE) {
	//	LoadMissileGFX(MFILE_FIREWAL); // InitMissileGFX(MIS_FIREWAVEC)
	// }
	// if (mitype == MIS_RUNELIGHT) {
	//	LoadMissileGFX(MFILE_LGHNING); // InitMissileGFX(MIS_LIGHTNOVAC)
	// }
	// if (mitype == MIS_RUNENOVA) {
	//	LoadMissileGFX(MFILE_LGHNING); // InitMissileGFX(MIS_LIGHTNINGC)
	// }
	// if (mitype == MIS_RUNEWAVE) {
	//	LoadMissileGFX(MFILE_BIGEXP); // InitMissileGFX(MIS_FIREEXP)
	// }
	if (mitype == MIS_BONEDEMON) {
		LoadMissileGFX(MFILE_EXORA1_B); // InitMissileGFX(MIS_EXBONEDEMON)
	}
	if (mitype == MIS_PSYCHORB) {
		LoadMissileGFX(MFILE_EXORA1); // InitMissileGFX(MIS_EXPSYCHORB)
	}
	if (mitype == MIS_NECROMORB) {
		LoadMissileGFX(MFILE_EXYEL2_B); // InitMissileGFX(MIS_EXNECROMORB)
	}
	if (mitype == MIS_LICH) {
		LoadMissileGFX(MFILE_EXORA1_A); // InitMissileGFX(MIS_EXLICH)
	}
	if (mitype == MIS_ARCHLICH) {
		LoadMissileGFX(MFILE_EXYEL2_A); // InitMissileGFX(MIS_EXARCHLICH)
	}
#endif
}

static void FreeMissileGFX(int midx)
{
	BYTE** mad;
	int n, i;

	n = misfiledata[midx].mfAnimFAmt;
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

	for (i = 0; i < NUM_FIXMFILE; i++) {
		FreeMissileGFX(i);
	}
}

void FreeMonMissileGFX()
{
	int i;

	for (i = NUM_FIXMFILE; i < NUM_MFILE; i++) {
		FreeMissileGFX(i);
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
static int PlaceRune(int mi, int sx, int sy, int dx, int dy, int mitype, int mirange)
{
	int i, j, tx, ty;
	const int8_t* cr;
	MissileStruct* mis;
	// (micaster == MST_PLAYER || micaster == MST_OBJECT);
	mis = &missile[mi];
	mis->_miVar1 = mitype;
	mis->_miVar2 = mirange;    // trigger range
	mis->_miVar3 = 16;         // delay
	if (mis->_miCaster & MST_PLAYER) {
		mis->_miCaster |= MST_RUNE;
		mis->_miSpllvl += plx(mis->_miSource)._pDexterity >> 3;
	}
	mis->_miRange = 16 + 1584; // delay + ttl (48 * 17 + 48 * 16)
	static_assert(DBORDERX >= 9 && DBORDERY >= 9, "PlaceRune expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 9, "PlaceRune uses CrawlTable/CrawlNum up to radius 9.");
	for (i = 0; i <= 9; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkMissile(tx, ty) && LineClear(sx, sy, tx, ty)) {
				mis->_mix = tx;
				mis->_miy = ty;
				static_assert(MAX_LIGHT_RAD >= 8, "PlaceRune needs at least light-radius of 8.");
				mis->_miLid = AddLight(tx, ty, 8);
				return MIRES_DONE;
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	return PlaceRune(mi, sx, sy, dx, dy, MIS_FIREEXP, 0); // RUNE_RANGE
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "AddLightRune expects a large enough border.");
	return PlaceRune(mi, sx, sy, dx, dy, MIS_LIGHTNINGC, 1); // RUNE_RANGE
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddNovaRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "AddNovaRune expects a large enough border.");
	return PlaceRune(mi, sx, sy, dx, dy, MIS_LIGHTNOVAC, 1); // RUNE_RANGE
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddWaveRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "AddWaveRune expects a large enough border.");
	return PlaceRune(mi, sx, sy, dx, dy, MIS_FIREWAVEC, 1); // RUNE_RANGE
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 * Var3: fire timer
 */
int AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	return PlaceRune(mi, sx, sy, dx, dy, MIS_STONE, 0); // RUNE_RANGE
}

/*int AddLightwall(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

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
	// assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	return MIRES_DONE;
}*/

int AddFireexp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam, dam;
	// ((micaster & MST_PLAYER) || micaster == MST_OBJECT);
	mis = &missile[mi];

	if (misource != -1) {
		// assert((unsigned)misource < MAX_PLRS);
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
 */
/*int AddFireball2(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	static_assert(MAX_LIGHT_RAD >= 8, "AddFireball2 needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	return MIRES_DONE;
}*/

int AddRingC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int tx, ty, j, mitype;
	const int8_t* cr;
	// ((micaster & MST_PLAYER) || micaster == MST_OBJECT);
	mitype = MIS_FIREWALL; //mis->_miType == MIS_FIRERING ? MIS_FIREWALL : MIS_LIGHTWALL;

	static_assert(DBORDERX >= 3 && DBORDERY >= 3, "AddRingC expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 3, "AddRingC uses CrawlTable/CrawlNum radius 3.");
	cr = &CrawlTable[CrawlNum[3]];
	for (j = (BYTE)*cr; j > 0; j--) {
		tx = sx + *++cr;
		ty = sy + *++cr;
		assert(IN_DUNGEON_AREA(tx, ty));
		if (PosOkMis2(tx, ty) && LineClear(sx, sy, tx, ty)) {
			AddMissile(tx, ty, 0, 0, 0, mitype, micaster, misource, spllvl);
		}
	}

	return MIRES_DELETE;
}
#endif

int AddDone(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	return MIRES_DONE;
}

/*
 * Var1: x coordinate of the missile-target of MIS_ASARROW
 * Var2: y coordinate of the missile-target of MIS_ASARROW
 * Var6: hit chance (MISHIT)
 * Var7: the distance travelled (MISDIST)
 */
int AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mtype;

	midir = GetDirection16(sx, sy, dx, dy);
	mtype = MFILE_ARROWS;
	if (micaster & MST_PLAYER) {
		// assert((unsigned)misource < MAX_PLRS);
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
	mis = &missile[mi];
	if (mtype == MFILE_ARROWS) {
		mis->_miAnimFrame = midir + 1;
	} else {
		mis->_miFileNum = mtype;
		SetMissAnim(mi, midir);
	}
	// mis->_miVar7 = 0;
	if (micaster & MST_PLAYER) {
		// mis->_miMinDam = plx(misource)._pIPcMinDam;
		// mis->_miMaxDam = plx(misource)._pIPcMaxDam;
		if (mis->_miType == MIS_ASARROW) {
			mis->_miVar1 = dx;
			mis->_miVar2 = dy;
		}
		// mis->_miVar6 = plx(misource)._pIHitChance;
	} else if (micaster == MST_MONSTER) {
		// assert((unsigned)misource < MAXMONSTERS);
		mis->_miMinDam = monsters[misource]._mMinDamage << 6;
		mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
		mis->_miVar6 = 30 + monsters[misource]._mHit + 2 * monsters[misource]._mLevel;
	} else {
		mis->_miMinDam = currLvl._dLevel << 6;
		mis->_miMaxDam = currLvl._dLevel << (1 + 6);
		mis->_miVar6 = 100 + currLvl._dLevel * 2;
	}
	return MIRES_DONE;
}

int AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, mindam, maxdam;

	mis = &missile[mi];
	if (micaster & MST_PLAYER) {
		// assert((unsigned)misource < MAX_PLRS);
		switch (mis->_miType) {
		case MIS_FIREBOLT:
			mindam = (plx(misource)._pMagic >> 3) + spllvl + 1;
			maxdam = mindam + 9;
			break;
		case MIS_FIREBALL:
			mindam = (plx(misource)._pMagic >> 2) + 10;
			maxdam = mindam + 10;
			for (i = spllvl; i > 0; i--) {
				mindam += mindam >> 3;
				maxdam += maxdam >> 3;
			}
			break;
		case MIS_HBOLT:
			mindam = (plx(misource)._pMagic >> 2) + spllvl;
			maxdam = mindam + 9;
			break;
		case MIS_FLARE:
			if (!plx(misource)._pInvincible)
				PlrDecHp(misource, 50 << 6, DMGTYPE_NPC);
			mindam = maxdam = (plx(misource)._pMagic * (spllvl + 1)) >> 3;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else if (micaster == MST_MONSTER) {
		// assert((unsigned)misource < MAXMONSTERS);
		mindam = monsters[misource]._mMinDamage;
		maxdam = monsters[misource]._mMaxDamage;
	} else {
		mindam = currLvl._dLevel;
		maxdam = mindam + 2 * mindam - 1;
	}
	mis->_miMinDam = mis->_miMaxDam = RandRange(mindam, maxdam) << 6;
	static_assert(MAX_LIGHT_RAD >= 8, "AddFirebolt needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	return MIRES_DONE;
}

/**
 * Var1: the target player
 * Var2: turn timer
 */
int AddMage(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	constexpr int MAX_BRIGHTNESS = 10;
	// assert(micaster == MST_MONSTER);
	// assert((unsigned)misource < MAXMONSTERS);
	mis = &missile[mi];
	mis->_miVar1 = dPlayer[dx][dy];
	// mis->_miVar2 = 0;
	mis->_miVar5 = midir; // MIS_DIR
	mis->_miUniqTrans = MAX_BRIGHTNESS;
	mis->_miMinDam = monsters[misource]._mMinDamage << 6;
	mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
	static_assert(MAX_LIGHT_RAD >= 8, "AddMage needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	return MIRES_DONE;
}

int AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	// assert(micaster == MST_MONSTER);
	// assert((unsigned)misource < MAXMONSTERS);
	mis = &missile[mi];
	mis->_mitxoff += 4 * mis->_mixvel;
	mis->_mityoff += 4 * mis->_miyvel;
	GetMissilePos(mi);
	mis->_miMinDam = monsters[misource]._mMinDamage << 6;
	mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
	static_assert(MAX_LIGHT_RAD >= 8, "AddMagmaball needs at least light-radius of 8.");
	mis->_miLid = AddLight(sx, sy, 8);
	return MIRES_DONE;
}

int AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam;
	// assert((micaster & MST_PLAYER) || micaster == MST_OBJECT);
	mindam = 1;
	if (misource != -1) {
		// assert((unsigned)misource < MAX_PLRS);
		maxdam = (plx(misource)._pMagic >> 1) + (spllvl << 5);
	} else {
		maxdam = 6 + currLvl._dLevel;
	}
	mis = &missile[mi];
	mis->_miMinDam = mindam << (6 - 2);      // * 16 / 64
	mis->_miMaxDam = maxdam << (6 - 2);      // * 16 / 64
	// assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	return MIRES_DONE;
}

/**
 * Var1: the id of the affected actor [0, -(pnum + 1), (mnum + 1)]
 */
int AddPoison(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int magic, mindam, maxdam;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	mis->_miRange = 16 * spllvl + 96;
	//if (misource != -1) {
		// TODO: add support for spell duration modifier
		// range += (plx(misource)._pISplDur * range) >> 7;
		magic = plx(misource)._pMagic;
		mindam = (magic >> 4) + spllvl + 2;
		maxdam = mindam + 2;
	//} else {
	//	mindam = 5 + currLvl._dLevel;
	//	maxdam = 10 + currLvl._dLevel * 2;
	//}
	mis->_miMinDam = mindam << (-3 + 6);
	mis->_miMaxDam = maxdam << (-3 + 6);
	// mis->_miVar1 = 0;
	return MIRES_DONE;
}

/*
 * Var1: mindam increment
 * Var2: maxdam increment
 */
int AddWind(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int magic, mindam, maxdam;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	mis->_mitxoff += 4 * mis->_mixvel;
	mis->_mityoff += 4 * mis->_miyvel;
	GetMissilePos(mi);
	//if (misource != -1) {
		magic = plx(misource)._pMagic;
		mindam = (magic >> 3) + 7 * spllvl + 1;
		maxdam = (magic >> 3) + 8 * spllvl + 1;
	//} else {
	//	mindam = (5 + currLvl._dLevel) / 8;
	//	maxdam = (10 + currLvl._dLevel * 2) / 8;
	//}
	//mis->_miMinDam = 0;
	//mis->_miMaxDam = 0;
	mis->_miVar1 = mindam;
	mis->_miVar2 = maxdam;
	return MIRES_DONE;
}

int AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	// assert(micaster == MST_MONSTER);
	// assert((unsigned)misource < MAXMONSTERS);
	mis = &missile[mi];
	mis->_miRange = 5 * (monsters[misource]._mAI.aiInt + 4);
	mis->_miMinDam = monsters[misource]._mMinDamage << 6;
	mis->_miMaxDam = monsters[misource]._mMaxDamage << 6;
	//mis->_miLid = NO_LIGHT;
	//PutMissile(mi);
	return MIRES_DONE;
}

int AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int dam;
	// assert(micaster == MST_MONSTER);
	// assert((unsigned)misource < MAXMONSTERS);
	mis = &missile[mi];
	if (spllvl == 0) {
		// pud from a missile
		dam = currLvl._dLevel << 2;
	} else {
		// pud from a corpse
		dam = monsters[misource]._mmaxhp >> (1 + 6);
	}
	mis->_miMinDam = mis->_miMaxDam = dam;
	mis->_miRange = 40 * (monsters[misource]._mAI.aiInt + 1) + random_(50, 16);
	return MIRES_DONE;
}

/*int AddKrull(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	missile[mi]._miMinDam = missile[mi]._miMaxDam = 4 << 6;
	//PutMissile(mi);
	return MIRES_DONE;
}*/

int AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i, j, tx, ty, dir;
	const int8_t* cr;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	// MisInCastDistance(sx, sy, midir, dx, dy, 7);
	while (true) {
		tx = sx - dx;
		ty = sy - dy;
		i = sqrt(tx * tx + ty * ty);
		if (i <= 7)
			break;
		// assert(i < MAXDUNX + MAXDUNY);
		dir = OPPOSITE(midir);
		dx += XDirAdd[dir];
		dy += YDirAdd[dir];
	}

	static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddTeleport expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 5, "AddTeleport uses CrawlTable/CrawlNum up to radius 5.");
	for (i = 0; i <= 5; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkActor(tx, ty)) {
				DoTeleport(misource, tx, ty);
				return MIRES_DELETE;
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int nTries;
	// assert((micaster & MST_PLAYER) || micaster == MST_OBJECT);
	// assert((unsigned)misource < MAX_PLRS);
	static_assert(DBORDERX >= 6 && DBORDERY >= 6, "AddRndTeleport expects a large enough border.");
	if ((micaster & MST_PLAYER) || (dx == 0 && dy == 0)) {
		nTries = 0;
		do {
			nTries++;
			if (nTries > 500) {
				return MIRES_FAIL_DELETE;
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

	DoTeleport(misource, dx, dy);
	return MIRES_DELETE;
}

/**
 * Var1: sfx helper
 * Var3: min-damage / 4
 * Var4: max-damage / 4
 */
int AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int magic, mindam, maxdam;
	// assert((micaster & MST_PLAYER) || micaster == MST_OBJECT);
	mis = &missile[mi];
	mis->_miRange = 64 * spllvl + 134;
	if (misource != -1) {
		// assert((unsigned)misource < MAX_PLRS);
		// TODO: add support for spell duration modifier
		// range += (plx(misource)._pISplDur * range) >> 7;
		magic = plx(misource)._pMagic;
		mindam = (magic >> 3) + spllvl + 5;
		maxdam = (magic >> 3) + spllvl * 2 + 10;
	} else {
		mindam = 5 + currLvl._dLevel;
		maxdam = 10 + currLvl._dLevel * 2;
	}
	mis->_miVar3 = mindam << (-3 - 2 + 6);
	mis->_miVar4 = maxdam << (-3 - 2 + 6);
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 */
int AddLightningC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
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
		// assert((unsigned)misource < MAX_PLRS);
		mindam = 1;
		maxdam = plx(misource)._pMagic + (spllvl << 3);
		range = (spllvl >> 1) + 6 - 1;
	} else if (micaster == MST_MONSTER) {
		// assert((unsigned)misource < MAXMONSTERS);
		if (spllvl == 0) {
			// standard lightning from a monster
			mindam = monsters[misource]._mMinDamage;
			maxdam = monsters[misource]._mMaxDamage << 1;
		} else {
			// lightning from a retreating MT_XBAT
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
	// assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	return MIRES_DONE;
}

/**
 * Var1: timer to place the splashes
 * Var2: last location where the blood-splash was placed
 */
int AddBloodBoilC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	// assert((micaster & MST_PLAYER) || micaster == MST_MONSTER);
	mis = &missile[mi];
	if (micaster == MST_MONSTER) {
		// assert((unsigned)misource < MAXMONSTERS);
		spllvl = monsters[misource]._mLevel / 6; // TODO: add _mSkillLvl?
		mis->_miSpllvl = spllvl;
	}

	mis->_mix = dx - 2;
	mis->_miy = dy - 2;
	mis->_miVar1 = 0;
	mis->_miVar2 = random_(49, lengthof(BloodBoilLocs));
	mis->_miRange = (lengthof(BloodBoilLocs) + spllvl * 2) * misfiledata[MFILE_BLODBURS].mfAnimFrameLen * misfiledata[MFILE_BLODBURS].mfAnimLen[0] / 2;
	return MIRES_DONE;
}

int AddBloodBoil(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam;
	// assert((micaster & MST_PLAYER) || micaster == MST_MONSTER);
	mis = &missile[mi];
	if (micaster == MST_MONSTER) {
		// assert((unsigned)misource < MAXMONSTERS);
		mindam = monsters[misource]._mLevel >> 1; // TODO: use _mSkillLvl?
		maxdam = monsters[misource]._mLevel;
	} else {
		// assert((unsigned)misource < MAX_PLRS);
		mindam = (plx(misource)._pMagic >> 2) + (spllvl << 2) + 10;
		maxdam = (plx(misource)._pMagic >> 2) + (spllvl << 3) + 10;
	}

	mis->_miMinDam = mindam << 6;
	mis->_miMaxDam = maxdam << 6;
	return MIRES_DONE;
}

/**
 * Var1: whether the target has died
 */
int AddBleed(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int pnum;
	MonsterStruct* mon;

	mis = &missile[mi];
	static_assert(MAX_PLRS <= MAX_MINIONS, "MIS_BLEED uses a single int to store player and monster targets.");
	assert(!(monsterdata[MT_GOLEM].mFlags & MFLAG_CAN_BLEED));
	if ((unsigned)spllvl >= MAX_MINIONS) {
		// assert((unsigned)misource < MAXMONSTERS);
		mon = &monsters[spllvl];
		mis->_mix = mon->_mx;
		mis->_miy = mon->_my;
		mis->_mixoff = mon->_mxoff;
		mis->_miyoff = mon->_myoff;
		mis->_miMinDam = mon->_mmaxhp >> (2 + 4);
		mis->_miMaxDam = mon->_mmaxhp >> (1 + 4);
	} else {
		pnum = spllvl;
		mis->_mix = plr._px;
		mis->_miy = plr._py;
		mis->_mixoff = plr._pxoff;
		mis->_miyoff = plr._pyoff;
		mis->_miMinDam = plr._pMaxHP >> (2 + 4);
		mis->_miMaxDam = plr._pMaxHP >> (1 + 4);
	}

	// mis->_miVar1 = 0;
	return MIRES_DONE;
}

int AddShroud(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, j, tx, ty;
	const int8_t* cr;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];

	static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddShroud expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 5, "AddShroud uses CrawlTable/CrawlNum up to radius 5.");
	for (i = 0; i <= 5; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkMissile(tx, ty) && LineClear(sx, sy, tx, ty)) {
				mis->_mix = tx;
				mis->_miy = ty;
				//mis->_misx = tx;
				//mis->_misy = ty;
				mis->_miRange = 32 * spllvl + 160;
				return MIRES_DONE;
			}
		}
	}
	return MIRES_FAIL_DELETE;
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
		mis->_miUniqTrans = bmis->_miUniqTrans;
	}
	//mis->_mixvel = 0;
	//mis->_miyvel = 0;
	// assert(mis->_miAnimFrameLen == 1);
	return MIRES_DONE;
}

static bool CheckIfTrig(int x, int y)
{
	int i;

	for (i = 0; i < MAXPORTAL; i++) {
		// if (portals[i]._rlevel == DLV_TOWN)
		//	continue;
		if (portals[i]._rlevel == currLvl._dLevelIdx && portals[i]._rx == x && portals[i]._ry == y)
			return true;
	}

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
	int i, j, tx, ty;
	const int8_t* cr;
	// assert((micaster & MST_PLAYER) || micaster == MST_NA);
	// assert((unsigned)misource < MAX_PLRS);
	// the position of portals in town and recreated portals are fixed
	if (currLvl._dType != DTYPE_TOWN && spllvl >= 0) {
		static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddTown expects a large enough border.");
		static_assert(lengthof(CrawlNum) > 5, "AddShroud uses CrawlTable/CrawlNum up to radius 5.");
		for (i = 0; i <= 5; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = (BYTE)*cr; j > 0; j--) {
				tx = dx + *++cr;
				ty = dy + *++cr;
				assert(IN_DUNGEON_AREA(tx, ty));
				if (PosOkActor(tx, ty) && !CheckIfTrig(tx, ty) && LineClear(sx, sy, tx, ty)) {
					goto done;
				}
			}
		}
		return MIRES_FAIL_DELETE;
done:
		;
	} else {
		tx = dx;
		ty = dy;
	}
	// 'delete' previous portal of the misource
	// assert(!missile[mi]._miDelFlag);
	RemovePortalMissile(misource);
	missile[mi]._miDelFlag = FALSE; // revert delete flag of the current missile
	// setup the new portal
	return AddPortal(mi, 0, 0, tx, ty, 0, 0, misource, spllvl);
}

/**
 * Var3: triggered (only for MIS_TOWN)
 */
int AddPortal(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	// assert((micaster & MST_PLAYER) || micaster == MST_NA);
	mis = &missile[mi];
	mis->_mix = mis->_misx = dx;
	mis->_miy = mis->_misy = dy;
	static_assert(MAX_LIGHT_RAD >= 15, "AddPortal needs at least light-radius of 15.");
	mis->_miLid = AddLight(dx, dy, spllvl >= 0 ? 1 : 15);
	if (spllvl >= 0) {
		PlaySfxLoc(LS_SENTINEL, dx, dy);
		if (misource == mypnum)
			NetSendCmdLocBParam1(CMD_ACTIVATEPORTAL, dx, dy, currLvl._dLevelIdx);
	} else {
		// a recreated portal (by AddWarpMissile or InitVP*Trigger)
		// make sure the portal is in its final form even on the first frame
		SetMissAnim(mi, 1);
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
		// assert((unsigned)misource < MAX_PLRS);
		dam = plx(misource)._pMagic >> 1;
		for (i = spllvl; i > 0; i--) {
			dam += dam >> 3;
		}
		mis->_miMinDam = dam;
		mis->_miMaxDam = dam << 3;
	} else {
		if (micaster == MST_MONSTER) {
			// assert((unsigned)misource < MAXMONSTERS);
			dam = monsters[misource]._mLevel << 1;
		} else {
			dam = currLvl._dLevel << 4;
		}
		mis->_miMinDam = mis->_miMaxDam = dam;
	}
	return MIRES_DONE;
}

/*
 * Var1: sfx helper (firewall)
 * Var3: min-damage / 4
 * Var4: max-damage / 4
 */
int AddFireWave(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int magic, mindam, maxdam;
	// assert((micaster & MST_PLAYER) || micaster == MST_OBJECT);
	mis = &missile[mi];
	if (misource != -1) {
		// assert((unsigned)misource < MAX_PLRS);
		magic = plx(misource)._pMagic;
		mindam = (magic >> 3) + 2 * spllvl + 1;
		maxdam = (magic >> 3) + 4 * spllvl + 2;
	} else {
		mindam = currLvl._dLevel + 1;
		maxdam = 2 * currLvl._dLevel + 2;
	}
	mis->_miVar3 = mindam << (-2 + 6);
	mis->_miVar4 = maxdam << (-2 + 6);
	return MIRES_DONE;
}

int AddMeteor(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam, i, j, tx, ty;
	const int8_t* cr;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	//if (micaster & MST_PLAYER) {
		mindam = (plx(misource)._pMagic >> 2) + (spllvl << 3) + 40;
		maxdam = (plx(misource)._pMagic >> 2) + (spllvl << 4) + 40;
	/*} else if (micaster == MST_MONSTER) {
		// assert((unsigned)misource < MAXMONSTERS);
		mindam = monsters[misource]._mMinDamage;
		maxdam = monsters[misource]._mMaxDamage;
	} else {
		mindam = currLvl._dLevel;
		maxdam = currLvl._dLevel * 2;
	}*/
	mis->_miMinDam = mindam << 6;
	mis->_miMaxDam = maxdam << 6;

	static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddMeteor expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 5, "AddMeteor uses CrawlTable/CrawlNum up to radius 5.");
	for (i = 0; i <= 5; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkMis2(tx, ty) && LineClear(sx, sy, tx, ty)) {
				mis->_misx = tx;
				mis->_misy = ty;
				mis->_mix = tx;
				mis->_miy = ty;
				// assert(mis->_miAnimLen == misfiledata[MFILE_SHATTER1].mfAnimLen[0]);
				mis->_miAnimFrame = misfiledata[MFILE_SHATTER1].mfAnimLen[0];
				mis->_miAnimAdd = -1;
				return MIRES_DONE;
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
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	static_assert(MAX_LIGHT_RAD >= 4, "AddChain needs at least light-radius of 4.");
	mis->_miLid = AddLight(sx, sy, 4);
	// assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miVar1 = 1 + (spllvl >> 1);
	//if (micaster & MST_PLAYER) {
		mis->_miMinDam = 1 << 6;
		mis->_miMaxDam = plx(misource)._pMagic << 6;
	//} else if (micaster == MST_MONSTER) {
	//	// assert((unsigned)misource < MAXMONSTERS);
	//	mindam = 1 << 6;
	//	maxdam = monsters[misource].mMaxDamage << 6;
	//} else {
	//	mindam = 1 << 6;
	//	maxdam = currLvl._dLevel << (1 + 6);
	//}
	return MIRES_DONE;
}

int AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	// assert(micaster == MST_MONSTER);
	// assert((unsigned)misource < MAXMONSTERS);
	// assert(dMonster[sx][sy] == misource + 1);
	dMonster[sx][sy] = -(misource + 1);
	monsters[misource]._mmode = MM_CHARGE;
	mis = &missile[mi];
	mis->_miDir = midir;
	SyncRhinoAnim(mi);
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
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)pnum < MAX_PLRS);
	// assert(dPlayer[sx][sy] == pnum + 1);
	dPlayer[sx][sy] = -(pnum + 1);

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
		GetMissileVel(mi, sx, sy, dx, dy, chv);
	}
	plr._pmode = PM_CHARGE;
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miVar1 = dx;
	mis->_miVar2 = dy;
	mis->_miAnimAdd = aa;
	SyncChargeAnim(mi);
	if (pnum == mypnum) {
		// assert(ScrollInfo._sdx == 0);
		// assert(ScrollInfo._sdy == 0);
		ScrollInfo._sdir = 1 + midir; // == dir2sdir[midir]
	}
	//mis->_miLid = mon->_mlid;
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
	MonAnimStruct* anim;
	MonsterStruct* mon;

	mon = &monsters[misource];
	anim = &mon->_mAnims[MA_WALK];
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miAnimFlag = TRUE;
	mis->_miAnimData = anim->maAnimData[midir];
	mis->_miAnimFrameLen = anim->maFrameLen;
	mis->_miAnimLen = anim->maFrames;
	mis->_miAnimWidth = mon->_mAnimWidth;
	mis->_miAnimXOffset = mon->_mAnimXOffset;
	mis->_miAnimAdd = 1;
	//mis->_miVar1 = FALSE;
	//mis->_miVar2 = 0;
	if (mon->_muniqtype != 0)
		mis->_miUniqTrans = mon->_muniqtrans + 4;
	dMonster[mon->_mx][mon->_my] = 0;
	//PutMissile(mi);
	return MIRES_DONE;
}*/

/**
 * Var1: mnum of the monster
 */
int AddStone(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	int i, j, tx, ty, mid, range;
	const int8_t* cr;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	static_assert(DBORDERX >= 2 && DBORDERY >= 2, "AddStone expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 2, "AddStone uses CrawlTable/CrawlNum up to radius 2.");
	for (i = 0; i <= 2; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mid = dMonster[tx][ty] - 1;
			if (mid < MAX_MINIONS || !LineClear(sx, sy, tx, ty))
				continue;
			assert(mid < MAXMONSTERS);
			mon = &monsters[mid];
			if (!(mon->_mFlags & MFLAG_NOSTONE) && !CanTalkToMonst(mid)
			 && mon->_mmode != MM_FADEIN && mon->_mmode != MM_FADEOUT && mon->_mmode != MM_CHARGE && mon->_mmode != MM_STONE && mon->_mmode != MM_DEATH /*mon->_mhitpoints != 0*/) {
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

				MonLeaveLeader(mid);
				mis->_miVar1 = mid;
				mis->_mix = mon->_mx;
				mis->_miy = mon->_my;
				mis->_mixoff = mon->_mxoff;
				mis->_miyoff = mon->_myoff;
				mon->_mVar3 = mon->_mmode;
				mon->_mmode = MM_STONE;
				// ensure lastx/y are set when MI_Stone 'alerts' the monster
				if (micaster == MST_PLAYER) {
					mon->_mlastx = plx(misource)._px;
					mon->_mlasty = plx(misource)._py;
				//} else {
				//	assert(!MON_RELAXED);
				}

				return MIRES_DONE;
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, j, tx, ty;
	const int8_t* cr;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];

	static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddGuardian expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 5, "AddGuardian uses CrawlTable/CrawlNum up to radius 5.");
	for (i = 0; i <= 5; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkMissile(tx, ty) && LineClear(sx, sy, tx, ty)) {
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
	return MIRES_FAIL_DELETE;
}

int AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MonsterStruct* mon;
	int tx, ty, i, j;
	const int8_t* cr;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	static_assert(MAX_MINIONS == MAX_PLRS, "AddGolem requires that owner of a monster has the same id as the monster itself.");
	mon = &monsters[misource];
	if (mon->_mmode > MM_INGAME_LAST) {
		static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddGolem expects a large enough border.");
		static_assert(lengthof(CrawlNum) > 5, "AddGolem uses CrawlTable/CrawlNum up to radius 5.");
		for (i = 0; i <= 5; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = (BYTE)*cr; j > 0; j--) {
				tx = dx + *++cr;
				ty = dy + *++cr;
				assert(IN_DUNGEON_AREA(tx, ty));
				if (PosOkActor(tx, ty) && LineClear(sx, sy, tx, ty)) {
					SpawnGolem(misource, tx, ty, spllvl);
					return MIRES_DELETE;
				}
			}
		}
		return MIRES_FAIL_DELETE;
	}

	/*missile[mi]._misx = */missile[mi]._mix = mon->_mx;
	/*missile[mi]._misy = */missile[mi]._miy = mon->_my;
	missile[mi]._miMaxDam = mon->_mhitpoints;
	missile[mi]._miMinDam = missile[mi]._miMaxDam >> 1;
	CheckSplashColFull(mi);

	MonKill(misource, misource);
	return MIRES_DELETE;
}

int AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i, hp;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);

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
	int tnum, i, hp;
	MonsterStruct* mon;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	// calculate hp
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
	// select target
	tnum = dPlayer[dx][dy];
	if (tnum != 0) {
		// - player
		tnum = tnum >= 0 ? tnum - 1 : -(tnum + 1);
		if (tnum != misource)
			PlrIncHp(tnum, hp);
	} else {
		// - minion
		tnum = dMonster[dx][dy];
		if (tnum != 0) {
			tnum = tnum >= 0 ? tnum - 1 : -(tnum + 1);
			if (tnum < MAX_MINIONS) {
				mon = &monsters[tnum];
				if (mon->_mhitpoints != 0) {
					// MonIncHp(tnum, hp);
					mon->_mhitpoints += hp;
					if (mon->_mhitpoints > mon->_mmaxhp)
						mon->_mhitpoints = mon->_mmaxhp;
				}
			}
		}
	}
	return MIRES_DELETE;
}

/**
 * Var1: whether the starting position is left
 * Var2: whether the destination is reached
 * Var3: x coordinate of the destination
 * Var4: y coordinate of the destination
 */
int AddElemental(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int magic, i, mindam, maxdam;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	//mis->_miVar1 = FALSE;
	//mis->_miVar2 = FALSE;
	mis->_miVar3 = dx;
	mis->_miVar4 = dy;
	mis->_miVar5 = midir; // MIS_DIR
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
	return MIRES_DONE;
}

int AddOpItem(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	return MIRES_DELETE;
}

/**
 * Var1: the distance of the wave from the starting position
 * Var2: first wave stopped
 * Var3: second wave stopped
 */
int AddWallC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int i, j, tx, ty;
	const int8_t* cr;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddWallC expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 5, "AddWallC uses CrawlTable/CrawlNum up to radius 5.");
	mis = &missile[mi];
	for (i = 0; i <= 5; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkMis2(tx, ty) && LineClear(sx, sy, tx, ty) && (sx != tx || sy != ty)) {
				midir = GetDirection8(sx, sy, dx, dy);
				midir = (midir - 2) & 7;
				mis->_mix = tx;
				mis->_miy = ty;
				mis->_mixvel = XDirAdd[midir];
				mis->_miyvel = YDirAdd[midir];
				//mis->_miVar1 = 0;
				//mis->_miVar2 = FALSE;
				//mis->_miVar3 = FALSE;
				mis->_miRange = (spllvl >> 1);
				return MIRES_DONE;
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddFireWaveC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);
	int sd, nx, ny, dir;
	int i, j;

	sd = GetDirection8(sx, sy, dx, dy);
	// if (!nMissileTable[dPiece[sx][sy]]) {
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
	// }

	return MIRES_DELETE;
}

int AddNovaC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i, tx, ty;
	const int8_t* cr;
	// assert((micaster & MST_PLAYER) || micaster == MST_OBJECT);
	static_assert(DBORDERX >= 3 && DBORDERY >= 3, "AddNovaC expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 3, "AddNovaC uses CrawlTable/CrawlNum radius 3.");
	// assert(CrawlTable[CrawlNum[3]] == 24);  -- (total) damage depends on this
	cr = &CrawlTable[CrawlNum[3]];
	for (i = (BYTE)*cr; i > 0; i--) {
		tx = sx + *++cr;
		ty = sy + *++cr;
		AddMissile(sx, sy, tx, ty, 0, MIS_LIGHTBALL, micaster, misource, spllvl);
	}

	return MIRES_DELETE;
}

int AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int oi = spllvl;
	int pnum = misource;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)pnum < MAX_PLRS);
	// assert((unsigned)oi < MAXOBJECTS);
	// assert(objects[oi]._oBreak == OBM_UNBREAKABLE);
	// assert(abs(plr._px - dx) < 2 && abs(plr._py - dy) < 2);
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
	// assert((micaster & MST_PLAYER) || micaster == MST_MONSTER);
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
	mis->_miVar2 = (2 - bmis->_miRange) * 4;
	// assert(misource != -1);
	if (micaster & MST_PLAYER) {
		// assert((unsigned)misource < MAX_PLRS);
		mindam = plx(misource)._pMagic;
		maxdam = mindam + (spllvl << 4);
	} else {
		// assert((unsigned)misource < MAXMONSTERS);
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
 */
int AddInfernoC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	// assert((micaster & MST_PLAYER) || micaster == MST_MONSTER);
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	return MIRES_DONE;
}

/*int AddFireTrap(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_miMinDam = (2 + currLvl._dLevel) << (6 - 2);
	mis->_miMaxDam = mis->_miMinDam * 2;
	return MIRES_DONE;
}*/

int AddBarrelExp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	// assert(micaster == MST_NA);
	mis = &missile[mi];
	mis->_miMinDam = 8 << (6 + gnDifficulty);
	mis->_miMaxDam = 16 << (6 + gnDifficulty);

	CheckMissileCol(mi, sx, sy, MICM_NONE);
	return MIRES_DELETE;
}

int AddCboltC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i = 3;

	// assert((micaster & MST_PLAYER) || micaster == MST_MONSTER);

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
	int mindam, maxdam;
	// assert((micaster & MST_PLAYER) || micaster == MST_MONSTER);
	mis = &missile[mi];
	static_assert(MAX_LIGHT_RAD >= 5, "AddCbolt needs at least light-radius of 5.");
	mis->_miLid = AddLight(sx, sy, 5);
	mis->_miVar1 = 5;
	mis->_miVar2 = midir;
	//mis->_miVar3 = 0;
	mis->_miVar4 = random_(0, 16);
	if (micaster & MST_PLAYER) {
		// assert((unsigned)misource < MAX_PLRS);
		mindam = 1 << 6;
		maxdam = (plx(misource)._pMagic << (-2 + 6)) + (spllvl << (2 + 6));
	} else {
		// assert((unsigned)misource < MAXMONSTERS);
		mindam = monsters[misource]._mMinDamage << 6;
		maxdam = monsters[misource]._mMaxDamage << 6;
	}
	mis->_miMinDam = mindam;
	mis->_miMaxDam = maxdam;
	// assert(mis->_miAnimLen == misfiledata[MFILE_MINILTNG].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_MINILTNG].mfAnimLen[0]);
	return MIRES_DONE;
}

int AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;

	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);

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
	const int8_t* cr;

	// assert(micaster & MST_PLAYER);
	// assert((unsigned)misource < MAX_PLRS);

	if (!LineClear(sx, sy, dx, dy))
		return MIRES_FAIL_DELETE;

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	mis->_miAnimFrame = 2;
	mis->_miAnimAdd = 2;

	dist = 4 + (spllvl >> 2);
	static_assert(DBORDERX >= 9 && DBORDERY >= 9, "AddAttract expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 9, "AddAttract uses CrawlTable/CrawlNum up to radius 9.");
	if (dist > 9)
		dist = 9;
	for (i = 0; i <= dist; i++) {
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
				mon->_mlastx = dx;
				mon->_mlasty = dy;
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
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)pnum < MAX_PLRS);

	switch (type) {
	case MTT_ITEM:
		// assert(target < MAXITEMS);
		if (pnum == mypnum && dx == items[target]._ix && dy == items[target]._iy
		 && LineClear(plr._px, plr._py, items[target]._ix, items[target]._iy))
			NetSendCmdGItem(!gbInvflag ? CMD_AUTOGETITEM : CMD_GETITEM, target);
		break;
	case MTT_MONSTER:
		// assert(target < MAXMONSTERS);
		if (LineClear(plr._px, plr._py, monsters[target]._mx, monsters[target]._my)
		 && CheckMonsterHit(target, &ret) && monsters[target]._mmode != MM_STONE && monsters[target]._mmode <= MM_INGAME_LAST && (monsters[target]._mmaxhp >> (6 + 1)) < plr._pMagic) {
			monsters[target]._msquelch = SQUELCH_MAX;
			monsters[target]._mlastx = plr._px;
			monsters[target]._mlasty = plr._py;
			// int dir = GetDirection8(plr._px, plr._py, monsters[target]._mx, monsters[target]._my);
			MonHitByPlr(target, pnum, 0, ISPL_KNOCKBACK, plr._pdir);
		}
		break;
	case MTT_OBJECT:
		// assert(target < MAXOBJECTS);
		if (LineClear(plr._px, plr._py, objects[target]._ox, objects[target]._oy))
			OperateObject(pnum, target, true);
		break;
	case MTT_PLAYER:
		// assert(target < MAX_PLRS);
		if (LineClear(plr._px, plr._py, plx(target)._px, plx(target)._py)
		 && plx(target)._pActive && !plx(target)._pLvlChanging && plx(target)._pDunLevel == currLvl._dLevelIdx && plx(target)._pHitPoints != 0 && plx(target)._pmode != PM_BLOCK
		 && (plx(target)._pMaxHP >> (6 + 1)) < plr._pMagic) {
			// int dir = GetDirection8(plr._px, plr._py, plx(target)._px, plx(target)._py);
			PlrHitByAny(target, pnum, 0, ISPL_KNOCKBACK, plr._pdir);
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return MIRES_DELETE;
}

int AddApocaC2(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int pnum, px, py;

	// assert(micaster == MST_MONSTER);
	// assert(misource == DIABLO);

	mis = &missile[mi];
	mis->_miMinDam = mis->_miMaxDam = 40 << (6 + gnDifficulty);

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!plr._pActive || plr._pDunLevel != currLvl._dLevelIdx || plr._pLvlChanging/* || plr._pHitPoints == 0*/)
			continue; // skip player if not on the current level
		// assert(plr._pAnims[PGX_WALK].paFrames == plr._mAnimLen);
		if (plr._pAnimFrame > (plr._pAnimLen >> 1)) {
			px = plr._pfutx;
			py = plr._pfuty;
		} else {
			px = plr._poldx;
			py = plr._poldy;
		}
		if (!LineClear(sx, sy, px, py))
			continue; // skip player if not visible

		// hit-check
		CheckMissileCol(mi, px, py, MICM_NONE);

		// add explosion effect
		mis->_mix = plr._px;
		mis->_miy = plr._py;
		mis->_mixoff = plr._pxoff;
		mis->_miyoff = plr._pyoff;

		AddMissile(0, 0, mi, 0, 0, MIS_EXAPOCA2, MST_NA, 0, 0);
	}
	return MIRES_DELETE;
}

int AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	// assert((micaster & MST_PLAYER) || micaster == MST_NA);
	// assert((unsigned)misource < MAX_PLRS);

	if (misource == mypnum) {
		if (plx(misource)._pManaShield == 0)
			NetSendCmdBParam1(CMD_SETSHIELD, spllvl + 1);
		else
			NetSendCmd(CMD_REMSHIELD);
	}
	return MIRES_DELETE;
}

int AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int i, range;
	// assert((micaster & MST_PLAYER) || micaster == MST_NA);
	// assert((unsigned)misource < MAX_PLRS);
	range = 1584;
	for (i = spllvl; i > 0; i--) {
		range += range >> 3;
	}
	// TODO: add support for spell duration modifier
	//range += range * plx(misource)._pISplDur >> 7;
	plx(misource)._pTimer[PLTR_INFRAVISION] = range;
	return MIRES_DELETE;
}

int AddRage(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	int pnum = misource;
	// assert(micaster & MST_PLAYER);
	// assert((unsigned)pnum < MAX_PLRS);

	if (plr._pTimer[PLTR_RAGE] == 0) {
		plr._pTimer[PLTR_RAGE] = 32 * spllvl + 245;
		PlaySfxLoc(sgSFXSets[SFXS_PLR_70][plr._pClass], plr._px, plr._py);
		CalcPlrItemVals(pnum, false); // last parameter should not matter
	}
	return MIRES_DELETE;
}

/**
 * Var1: min-damage modifier on hit
 * Var2: max-damage modifier on hit
 */
int AddPulse(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	int mindam, maxdam, i, j, tx, ty;
	const int8_t* cr;
	mis = &missile[mi];

	// assert((micaster & MST_PLAYER) || micaster == MST_MONSTER);
	if (micaster == MST_MONSTER) {
		// assert((unsigned)misource < MAXMONSTERS);
		mindam = monsters[misource]._mMinDamage << 6;
		maxdam = monsters[misource]._mMaxDamage << 6;
	} else {
		// assert((unsigned)misource < MAX_PLRS);
		mindam = 1 << 6;
		maxdam = (plx(misource)._pMagic << (-2 + 6)) + (spllvl << (2 + 6));
	}
	mis->_miVar1 = mindam / 4u;
	mis->_miVar2 = maxdam / 4u;
	mis->_miMinDam = mindam - mis->_miVar1;
	mis->_miMaxDam = maxdam - mis->_miVar2;
	// assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_LGHNING].mfAnimLen[0]);

	static_assert(DBORDERX >= 5 && DBORDERY >= 5, "AddPulse expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 5, "AddPulse uses CrawlTable/CrawlNum up to radius 5.");
	for (i = 0; i <= 5; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			if (PosOkMis2(tx, ty) && LineClear(sx, sy, tx, ty)) {
				mis->_mix = tx;
				mis->_miy = ty;
				//mis->_misx = tx; -- unused
				//mis->_misy = ty;
				static_assert(MAX_LIGHT_RAD >= 4, "AddPulse needs at least light-radius of 4.");
				mis->_miLid = AddLight(tx, ty, 4);
				return MIRES_DONE;
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddCallToArms(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl)
{
	// assert(micaster == MST_MONSTER);
	// assert((unsigned)misource < MAXMONSTERS);
	MonCallToArms(misource);

	return MIRES_DELETE;
}

int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, int micaster, int misource, int spllvl)
{
	MissileStruct* mis;
	const MissileData* mds;
	int idx, mi, anims, animdir, res;

	idx = nummissiles;
	if (idx >= MAXMISSILES)
		return -1;

	mi = missileactive[idx];
	nummissiles++;

	mis = &missile[mi];
	memset(mis, 0, sizeof(*mis));

	// mis->_miRndSeed = NextRndSeed();
	mis->_miCaster = micaster;
	mis->_miSource = misource;
	mis->_miSpllvl = spllvl;
	mis->_mix = sx;
	mis->_miy = sy;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_miType = mitype;
	mds = &missiledata[mitype];
	mis->_miFlags = mds->mdFlags;
	mis->_miResist = mds->mResist;
	mis->_miFileNum = mds->mFileNum;
	mis->_miRange = mds->mdRange;

	mis->_miAnimAdd = 1;
	mis->_miLid = NO_LIGHT;

	if (SFX_VALID(mds->mlSFX)) {
		PlaySfxLocN(mds->mlSFX, sx, sy, mds->mlSFXCnt);
	}

	if (mds->mdPrSpeed != 0) {
		if (sx == dx && sy == dy) {
			dx += XDirAdd[midir];
			dy += YDirAdd[midir];
		}
		GetMissileVel(mi, sx, sy, dx, dy, MIS_SHIFTEDVEL(mds->mdPrSpeed));
	}

	animdir = 0;
	anims = misfiledata[mds->mFileNum].mfAnimFAmt;
	if (anims >= NUM_DIRS) {
		animdir = midir;
		if (anims != NUM_DIRS) {
			// assert(anims == 16);
			animdir = GetDirection16(sx, sy, dx, dy);
		}
	}
	SetMissAnim(mi, animdir);

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

static void ConvertMissile(int mi, int mitype)
{
	MissileStruct* mis;
	const MissileData* mds;
	const int animdir = 0;

	mis = &missile[mi];
	mis->_miType = mitype;
	mds = &missiledata[mitype];
	mis->_miFlags = mds->mdFlags;
	// mis->_miResist = mds->mResist;
	mis->_miFileNum = mds->mFileNum;
	mis->_miRange = mds->mdRange;

	// mis->_miAnimAdd = 1;

	// assert(misfiledata[mds->mFileNum].mfAnimFAmt < NUM_DIRS);
	SetMissAnim(mi, animdir);

	PutMissile(mi);
}

static bool Sentfire(int mi, int sx, int sy)
{
	int mnum;
	MissileStruct* mis;

	mis = &missile[mi];
	// assert(mis->_miCaster == MST_PLAYER);
	mnum = dMonster[sx][sy] - 1;
	if (mnum >= MAX_MINIONS
	 && monsters[mnum]._mhitpoints != 0
	 //&& !CanTalkToMonst(mnum) -- commented out to make it consistent with MI_Rune, MI_Poison, FindClosestChain, FindClosest
	 && LineClear(mis->_mix, mis->_miy, sx, sy)) {
		// SetRndSeed(mis->_miRndSeed);
		AddMissile(mis->_mix, mis->_miy, sx, sy, 0, MIS_FIREBOLT, MST_PLAYER, mis->_miSource, mis->_miSpllvl);
		// mis->_miRndSeed = NextRndSeed();
		SetMissAnim(mi, 2);
		// assert(mis->_miAnimLen == misfiledata[MFILE_GUARD].mfAnimLen[2]);
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
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_miVar7++; // MISDIST
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, mis->_miType != MIS_PCARROW ? MICM_BLOCK_ANY : MICM_BLOCK_WALL);
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
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_miVar7++; // MISDIST
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (!nMissileTable[dPiece[mis->_mix][mis->_miy]] && (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2)) {
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
	int xptype, hit = 0;

	mis = &missile[mi];
	//omx = mis->_mitxoff;
	//omy = mis->_mityoff;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		hit = CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY);
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		CondChangeLightXY(mis->_miLid, mis->_mix, mis->_miy);
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
		CheckSplashCol(mi, hit);
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
	ConvertMissile(mi, xptype);
}

void MI_Mage(int mi)
{
	MissileStruct *mis, *bmis;
	constexpr int MAX_BRIGHTNESS = 10;
	int i, bmi, xptype;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY);
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		for (i = 0; i < nummissiles; i++) {
			bmi = missileactive[i];
			if (bmi <= mi)
				continue;
			bmis = &missile[bmi];
			// check the type of the other missile
			if (bmis->_miType != MIS_MAGE) {
				continue;
			}
			// check the distance
			if (bmis->_mix != mis->_mix || bmis->_miy != mis->_miy) {
				continue;
			}
			int doff = abs(bmis->_mixoff - mis->_mixoff) + abs(bmis->_miyoff - mis->_miyoff) * (TILE_WIDTH / TILE_HEIGHT);
			if (doff > TILE_WIDTH)
				continue;
			// check target
			if (mis->_miVar1 != 0 && bmis->_miVar1 != 0 && mis->_miVar1 != bmis->_miVar1)
				continue;
			// merge the missiles
			if (mis->_miVar1 != 0)
				bmis->_miVar1 = mis->_miVar1;
			bmis->_miUniqTrans -= (MAX_BRIGHTNESS - mis->_miUniqTrans) + 1;
			if (bmis->_miUniqTrans < 0)
				bmis->_miUniqTrans = 0;
			bmis->_miMinDam += mis->_miMinDam;
			bmis->_miMaxDam += mis->_miMaxDam;
			if (bmis->_miRange < mis->_miRange + 1)
				bmis->_miRange = mis->_miRange + 1;
			// assert(mis->_miCaster == MST_MONSTER && bmis->_miCaster == MST_MONSTER);
			if (monsters[bmis->_miSource]._mMagic < monsters[mis->_miSource]._mMagic)
				bmis->_miSource = mis->_miSource;
			mis->_miDelFlag = TRUE; // + AddUnLight
			return;
		}

		if (mis->_miVar1 != 0 && (mis->_miVar2++ & 7) == 0) {
			int pnum = mis->_miVar1;
			pnum = pnum >= 0 ? pnum - 1 : -(pnum + 1);
			if (plr._pActive && plr._pDunLevel == currLvl._dLevelIdx/* && !plr._pLvlChanging*/ && plr._pHitPoints != 0 && (mis->_mix != plr._px || mis->_miy != plr._py)) {
				mis->_miVar5 = GetDirection8(mis->_mix, mis->_miy, plr._px, plr._py); // MIS_DIR
				GetMissileVel(mi, mis->_mix, mis->_miy, plr._px, plr._py, MIS_SHIFTEDVEL(missiledata[MIS_MAGE].mdPrSpeed));
			} else {
				mis->_miVar1 = 0;
			}
		}

		CondChangeLightXY(mis->_miLid, mis->_mix, mis->_miy);
		PutMissile(mi);
		return;
	}

	xptype = MIS_EXMAGE;
	ConvertMissile(mi, xptype);
}

void MI_Poison(int mi)
{
	MissileStruct* mis;
	int tnum, pnum;
	MonsterStruct* mon;

	mis = &missile[mi];
	if (mis->_miVar1 == 0) {
		// target not acquired
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		if ((mis->_mix != mis->_misx || mis->_miy != mis->_misy)
		 && CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_WALL) == 1) {
			tnum = dMonster[mis->_mix][mis->_miy];
			if (tnum != 0) {
				// monster target acquired
				tnum = tnum >= 0 ? tnum - 1 : -(tnum + 1);

				mis->_miVar1 = tnum + 1;
			} else {
				tnum = dPlayer[mis->_mix][mis->_miy];
				if (tnum != 0) {
					// player target acquired
					tnum = tnum >= 0 ? tnum - 1 : -(tnum + 1);

					mis->_miVar1 = -(tnum + 1);
				} else {
					// actor died and displaced -> done
					mis->_miRange = 0;
				}
			}
		}
	} else if (mis->_miVar1 > 0) {
		// monster target
		tnum = mis->_miVar1 - 1;
		mon = &monsters[tnum];
		if (mon->_mmode > MM_INGAME_LAST || mon->_mmode == MM_DEATH) {
			mis->_miRange = 0;
		} else {
			mis->_mix = mon->_mx;
			mis->_miy = mon->_my;
			mis->_mixoff = mon->_mxoff;
			mis->_miyoff = mon->_myoff;
			if (mon->_mSelFlag & 4) {
				if (mon->_mSelFlag & 2) {
					mis->_miyoff -= 3 * TILE_HEIGHT / 4;
				} else {
					mis->_miyoff -= TILE_HEIGHT;
				}
			} else {
				if (mon->_mSelFlag & 2) {
					mis->_miyoff -= TILE_HEIGHT / 2;
				}
			}
			// CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
			MonMissHit(tnum, mi);
		}
	} else {
		// player target
		pnum = -(mis->_miVar1 + 1);
		if (!plr._pActive || plr._pLvlChanging || plr._pHitPoints == 0) {
			mis->_miRange = 0;
		} else {
			mis->_mix = plr._px;
			mis->_miy = plr._py;
			mis->_mixoff = plr._pxoff;
			mis->_miyoff = plr._pyoff - 3 * TILE_HEIGHT / 4;
			// CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
			PlrMissHit(pnum, mi);
		}
	}

	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}

void MI_Wind(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_WALL);
	}
	if (mis->_miDir == 0) {
		mis->_miMinDam += mis->_miVar1;
		mis->_miMaxDam += mis->_miVar2;
		// assert(mis->_miAnimLen == misfiledata[MFILE_WIND].mfAnimLen[0]);
		// assert(mis->_miAnimFrameLen == 1);
		if (mis->_miAnimFrame == misfiledata[MFILE_WIND].mfAnimLen[0]) {
			SetMissAnim(mi, 1);
		}
	}

	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
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
	MissileStruct* mis;

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

void MI_Acid(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
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

	// SetRndSeed(mis->_miRndSeed); // used by MIS_EXACIDP
	ConvertMissile(mi, MIS_EXACIDP);
}

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
			mis->_miRange = misfiledata[MFILE_ACIDPUD].mfAnimLen[1] * misfiledata[MFILE_ACIDPUD].mfAnimFrameLen;
			SetMissAnim(mi, 1);
		}
	}
	PutMissileF(mi, BFLAG_MISSILE_PRE);
}

void MI_Firewall(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
	if (mis->_miDir == 0) {
		if (mis->_miLid == NO_LIGHT) {
			mis->_miLid = AddLight(mis->_mix, mis->_miy, FireWallLight[0]);
		} else {
			// assert(mis->_miAnimLen < lengthof(FireWallLight));
			ChangeLightRadius(mis->_miLid, FireWallLight[mis->_miAnimFrame]);
		}
		// assert(mis->_miAnimFrameLen == 1);
		if ((mis->_miAnimFrame & 1) == 0 && mis->_miAnimFrame <= 8) {
			mis->_miMinDam += mis->_miAnimAdd >= 0 ? mis->_miVar3 : -mis->_miVar3;
			mis->_miMaxDam += mis->_miAnimAdd >= 0 ? mis->_miVar4 : -mis->_miVar4;
		}
		// assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[0]);
		if (mis->_miAnimFrame == misfiledata[MFILE_FIREWAL].mfAnimLen[0]
		// && mis->_miAnimCnt == misfiledata[MFILE_FIREWAL].mfAnimFrameLen - 1
		 && mis->_miAnimAdd >= 0) {
			// start 'stand' after spawn
			SetMissAnim(mi, 1);
			// assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
			mis->_miAnimFrame = RandRange(1, misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
			mis->_miVar1 = RandRange(1, 256);
		} else if (mis->_miAnimAdd < 0 && mis->_miAnimFrame == 1) {
			mis->_miDelFlag = TRUE; // + AddUnLight
			return;
		}
	} else {
		// assert(mis->_miDir == 1);
		mis->_miRange--;
		if (--mis->_miVar1 == 0 && mis->_miRange > 64) {
			// add random firewall sfx, but only if the fire last more than ~2s
			mis->_miVar1 = 255;
			// assert(missiledata[MIS_FIREWALL].mlSFX == LS_WALLLOOP);
			// assert(missiledata[MIS_FIREWALL].mlSFXCnt == 1);
			PlaySfxLoc(LS_WALLLOOP, mis->_mix, mis->_miy);
		} else if (mis->_miRange < 0) {
			// start collapse
			SetMissAnim(mi, 0);
			// assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[0]);
			mis->_miAnimFrame = misfiledata[MFILE_FIREWAL].mfAnimLen[0];
			mis->_miAnimAdd = -1;
			// mis->_miRange = 0;
		}
	}
	PutMissileF(mi, BFLAG_HAZARD); // TODO: do not place hazard if the source is a monster
}

/*void MI_Fireball(int mi)
{
	MissileStruct* mis;
	int mx, my, hit = 0;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	mx = mis->_mix;
	my = mis->_miy;
	if (mx != mis->_misx || my != mis->_misy)
		hit = CheckMissileCol(mi, mx, my, MICM_BLOCK_ANY);
	if (mis->_miRange >= 0) {
		CondChangeLightXY(mis->_miLid, mx, my);
		PutMissile(mi);
		return;
	}
	//CheckMissileCol(mi, mx, my, MICM_NONE);
	// TODO: mis->_miMinDam >>= 1; mis->_miMaxDam >>= 1; ?
	CheckSplashCol(mi, hit);

	ConvertMissile(mi, MIS_EXFBALL);
}*/

#ifdef HELLFIRE
void MI_HorkSpawn(int mi)
{
	MissileStruct* mis;
	int i, j, tx, ty;
	const int8_t* cr;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		// if ((mis->_mix == mis->_misx && mis->_miy == mis->_misy) || PosOkMissile(mis->_mix, mis->_miy)) {
		// if (PosOkMonster(mis->_miSource, mis->_mix, mis->_miy)) {
			PutMissile(mi);
			return;
		// }
	}
	// assert(abs(mis->_mix - mis->_misx) < 2 && abs(mis->_miy - mis->_misy) < 2);
	mis->_miDelFlag = TRUE;
	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "MI_HorkSpawn expects a large enough border.");
	static_assert(lengthof(CrawlNum) > 1, "MI_HorkSpawn uses CrawlTable/CrawlNum up to radius 1.");
	for (i = 0; i <= 1; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
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
	int j, mnum, sx, sy, tx, ty;
	const int8_t* cr;

	mis = &missile[mi];
	if (--mis->_miVar3 <= 0) {
		sx = mis->_mix;
		sy = mis->_miy;
		static_assert(lengthof(CrawlNum) > 1, "MI_Rune uses CrawlTable/CrawlNum up to radius 1.");
		cr = &CrawlTable[CrawlNum[mis->_miVar2]]; // RUNE_RANGE
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			if (dPlayer[tx][ty] == 0) {
				mnum = dMonster[tx][ty];
				if (mnum == 0)
					continue;
				mnum = mnum >= 0 ? mnum - 1 : -(mnum + 1);
				if (monsters[mnum]._mmode == MM_STONE || monsters[mnum]._mmode == MM_DEATH)
					continue;
			}
			if (!LineClear(sx, sy, tx, ty))
				continue;
			// SetRndSeed(mis->_miRndSeed);
			AddMissile(sx, sy, tx, ty, 0, mis->_miVar1, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
			mis->_miRange -= 48;
			mis->_miVar3 = 48;
			break;
		}
	} else {
		mis->_miAnimCnt--;
	}
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE; // + AddUnLight
		return;
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
	MissileStruct* mis;
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
			// mis->_miRndSeed = NextRndSeed();
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
	mis->_miDelFlag = TRUE; // + AddUnLight
}

void MI_BloodBoilC(int mi)
{
	MissileStruct* mis;
	int mx, my;

	mis = &missile[mi];
	// assert(mis->_miType == MIS_BLOODBOILC || mis->_miType == MIS_SWAMPC);
	if ((mis->_miVar1++ & (mis->_miType == MIS_BLOODBOILC ? 7 : 3)) == 0) {
		mis->_miVar2++;
		if (mis->_miVar2 >= lengthof(BloodBoilLocs))
			mis->_miVar2 = 0;
		mx = mis->_mix + BloodBoilLocs[mis->_miVar2][0];
		my = mis->_miy + BloodBoilLocs[mis->_miVar2][1];
		if ((nMissileTable[dPiece[mx][my]] | dObject[mx][my]) == 0) {
			AddMissile(mx, my, -1, 0, 0, mis->_miType == MIS_BLOODBOILC ? MIS_BLOODBOIL : MIS_SWAMP, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
		}
	}
	mis->_miRange--;
	if (mis->_miRange < 0)
		mis->_miDelFlag = TRUE;
}

void MI_SwampC(int mi)
{
	MissileStruct *mis, *bmis;
	int i, bmi, dx, dy, mx, my, mnum, pnum;
	bool matches[16][16] = { false };
	const int bx = 8 - 2; const int by = 8 - 2;

	mis = &missile[mi];
	for (i = 0; i < nummissiles; i++) {
		bmi = missileactive[i];
		bmis = &missile[bmi];
		if (bmis->_miType != MIS_SWAMPC)
			continue;
		if (bmis->_miSpllvl < mis->_miSpllvl)
			continue;
		if (bmis->_miSpllvl == mis->_miSpllvl && mi <= bmi)
			continue;
		dx = bmis->_mix - mis->_mix;
		dy = bmis->_miy - mis->_miy;
		if (dx < -4 || dx > 4 || dy < -4 || dy > 4)
			continue;
		dx += bx;
		dy += by;
		for (int n = 0; n < lengthof(BloodBoilLocs); n++) {
			mx = dx + BloodBoilLocs[n][0];
			my = dy + BloodBoilLocs[n][1];
			matches[mx][my] = true;
		}
	}

	for (int n = 0; n < lengthof(BloodBoilLocs); n++) {
		dx = BloodBoilLocs[n][0];
		dy = BloodBoilLocs[n][1];
		if (matches[dx + bx][dy + by])
			continue;
		mx = mis->_mix + dx;
		my = mis->_miy + dy;

		mnum = dMonster[mx][my];
		if (mnum < 0) {
			mnum = -(mnum + 1);
			MonHinder(mnum, mis->_miSpllvl, mis->_miVar1);
		} else {
			pnum = dPlayer[mx][my];
			if (pnum < 0) {
				pnum = -(pnum + 1);
				PlrHinder(pnum, mis->_miSpllvl, mis->_miVar1);
			}
		}
	}

	MI_BloodBoilC(mi);
}

void MI_BloodBoil(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	// assert(mis->_miAnimLen == misfiledata[MFILE_BLODBURS].mfAnimLen[0]);
	// assert(mis->_miAnimFrameLen == misfiledata[MFILE_BLODBURS].mfAnimFrameLen);
	if (mis->_miRange == misfiledata[MFILE_BLODBURS].mfAnimFrameLen * misfiledata[MFILE_BLODBURS].mfAnimLen[0] / 2)
		CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE /* MICM_BLOCK_WALL */);
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}

void MI_Bleed(int mi)
{
	MissileStruct* mis;
	int tnum, pnum;
	MonsterStruct* mon;

	mis = &missile[mi];
	if (mis->_miVar1 == 0) {
		tnum = mis->_miSpllvl;
		static_assert(MAX_PLRS <= MAX_MINIONS, "MIS_BLEED uses a single int to store player and monster targets.");
		assert(!(monsterdata[MT_GOLEM].mFlags & MFLAG_CAN_BLEED));
		if (tnum >= MAX_MINIONS) {
			mon = &monsters[tnum];
			if (mon->_mmode > MM_INGAME_LAST || mon->_mmode == MM_DEATH) {
				mis->_miVar1 = 1;
			} else if (mon->_mmode != MM_STONE) {
				// CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
				MonMissHit(tnum, mi);
			}
		} else {
			pnum = tnum;
			if (!plr._pActive || plr._pLvlChanging || plr._pHitPoints == 0) {
				mis->_miVar1 = 1;
			} else {
				// CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE);
				PlrMissHit(pnum, mi);
			}
		}
	}

	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}

void MI_Portal(int mi)
{
	MissileStruct* mis;
	static_assert(MAX_LIGHT_RAD >= 15, "MI_Portal needs at least light-radius of 15.");
	int ExpLight[17] = { 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15 };

	mis = &missile[mi];
	if (mis->_miDelFlag) {
		return;
	}
	if (mis->_miDir == 0) {
		// assert(mis->_miAnimLen < lengthof(ExpLight));
		ChangeLightRadius(mis->_miLid, ExpLight[mis->_miAnimFrame]);
		// assert(mis->_miAnimLen == misfiledata[MFILE_PORTAL].mfAnimLen[0]);
		// assert(mis->_miAnimFrameLen == 1);
		if (mis->_miAnimFrame == misfiledata[MFILE_PORTAL].mfAnimLen[0]
		 /*&& mis->_miAnimCnt == misfiledata[MFILE_PORTAL].mfAnimFrameLen - 1*/) {
			SetMissAnim(mi, 1);
		}
	}

	if (mis->_miType == MIS_TOWN) {
		if (myplr._px == mis->_mix && myplr._py == mis->_miy && /*!myplr._pLvlChanging &&*/ myplr._pmode == PM_STAND && !mis->_miVar3) {
			mis->_miVar3 = TRUE;
			NetSendCmdBParam1(CMD_USEPORTAL, mis->_miSource);
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
	// assert(mis->_miAnimFrameLen == 1);
	if (mis->_miAnimFrame == misfiledata[MFILE_BLUEXFR].mfAnimLen[0]
	 /*&& mis->_miAnimCnt == misfiledata[MFILE_BLUEXFR].mfAnimFrameLen - 1*/) {
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
	// assert(mis->_miAnimFrameLen == 1);
	if (mis->_miAnimFrame == misfiledata[MFILE_BLUEXBK].mfAnimLen[0]
	 /*&& mis->_miAnimCnt == misfiledata[MFILE_BLUEXBK].mfAnimFrameLen - 1*/) {
		mis->_miDelFlag = TRUE;
		return;
	}
	PutMissileF(mi, BFLAG_MISSILE_PRE);
}

void MI_FireWave(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (nMissileTable[dPiece[mis->_mix][mis->_miy]]) {
		mis->_miDelFlag = TRUE;
		return;
	}
	CondChangeLightXY(mis->_miLid, mis->_mix, mis->_miy);

	MI_Firewall(mi);
}

void MI_Meteor(int mi)
{
	MissileStruct* mis;
	int mx, my;
	const int MET_SHIFT_X = 16 * ASSET_MPL, MET_SHIFT_Y = 110 * ASSET_MPL, MET_SHIFT_UP = 26 * ASSET_MPL, MET_STEPS_UP = 2, MET_STEPS_DOWN = 10;

	mis = &missile[mi];

	if (mis->_miFileNum != MFILE_FIREBA) {
		// assert(misfiledata[MFILE_FIREBA].mfAnimFrameLen == 1);
		if (mis->_miAnimFrame == 3
		 /*&& mis->_miAnimCnt == misfiledata[MFILE_FIREBA].mfAnimFrameLen - 1*/) {
			mis->_miyoff -= MET_SHIFT_UP / MET_STEPS_UP;
			mis->_mixoff += MET_SHIFT_X / MET_STEPS_UP;
			if (mis->_miyoff < -MET_SHIFT_UP) {
				mis->_miFileNum = MFILE_FIREBA;
				SetMissAnim(mi, 0);
				mis->_mixoff = MET_SHIFT_X;
				// -- 96: height of the sprite, 46: transparent lines on the first frame -- unnecessary, since Cl2DrawLightTbl is safe
				//static_assert(BORDER_TOP - (96 - 46) * ASSET_MPL >= MET_SHIFT_Y, "MI_Meteor expects a large enough (screen-)border.");
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
	const int8_t* cr;

	mis = &missile[mi];
	switch (mis->_miDir) {
	case 0: // collapse/spawn
		// assert(((1 + mis->_miAnimLen) >> 1) <= MAX_LIGHT_RAD);
		ChangeLightRadius(mis->_miLid, (1 + mis->_miAnimFrame) >> 1);
		// assert(mis->_miAnimLen == misfiledata[MFILE_GUARD].mfAnimLen[0]);
		// assert(mis->_miAnimFrameLen == 1);
		if (mis->_miAnimFrame == misfiledata[MFILE_GUARD].mfAnimLen[0]
		 // && mis->_miAnimCnt == misfiledata[MFILE_GUARD].mfAnimFrameLen - 1
		 && mis->_miAnimAdd >= 0) {
			// start stand after spawn
			SetMissAnim(mi, 1);
		} else if (mis->_miAnimFrame == 1
		 // && mis->_miAnimCnt == misfiledata[MFILE_GUARD].mfAnimFrameLen - 1
		 && mis->_miAnimAdd < 0) {
			// done after collapse
			mis->_miDelFlag = TRUE; // + AddUnLight
			return;
		}
		break;
	case 1: // active
		if (mis->_miAnimFrame == 1 /*&& mis->_miAnimCnt == 0*/) {
			// check for an enemy
			mis->_miRange--;
			if (mis->_miRange >= 0) {
				static_assert(DBORDERX >= 6 && DBORDERY >= 6, "MI_Guardian expects a large enough border.");
				static_assert(lengthof(CrawlNum) > 6, "MI_Guardian uses CrawlTable/CrawlNum up to radius 6.");
				for (i = 6; i >= 0; i--) {
					cr = &CrawlTable[CrawlNum[i]];
					for (j = (BYTE)*cr; j > 0; j--) {
						tx = mis->_mix + *++cr;
						ty = mis->_miy + *++cr;
						if (Sentfire(mi, tx, ty))
							goto done;
					}
				}
done:
				;
			} else {
collapse:
				// start collapse
				SetMissAnim(mi, 0);
				// assert(mis->_miAnimLen == misfiledata[MFILE_GUARD].mfAnimLen[0]);
				mis->_miAnimFrame = misfiledata[MFILE_GUARD].mfAnimLen[0];
				mis->_miAnimAdd = -1;
			}
		}
		break;
	case 2:
		// start stand after fire, or collapse if this was the last shot
		// assert(mis->_miAnimFrameLen == 1);
		if (mis->_miAnimFrame == 1
		 /* && mis->_miAnimCnt == misfiledata[MFILE_GUARD].mfAnimFrameLen - 1*/) {
			if (mis->_miRange <= 0)
				goto collapse;
			SetMissAnim(mi, 1);
			// skip check frame to add delay between attacks
			mis->_miAnimFrame = 2;
			mis->_miAnimAdd = 1;
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
		if (CheckMissileCol(mi, mx, my, MICM_BLOCK_ANY) == 1) {
			if (mis->_miVar1-- != 0) {
				// set the new position as the starting point
				mis->_misx = mx;
				mis->_misy = my;
				mis->_mixoff = 0;
				mis->_miyoff = 0;
				mis->_mitxoff = 0;
				mis->_mityoff = 0;
				// restore base range
				mis->_miRange = missiledata[MIS_CHAIN].mdRange;
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
				//SetMissAnim(mi, sd);
				GetMissileVel(mi, mx, my, dx, dy, MIS_SHIFTEDVEL(missiledata[MIS_CHAIN].mdPrSpeed));
			}
		}
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE; // + AddUnLight
}

void MI_Misexp(int mi)
{
	MissileStruct* mis;
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
	mis->_miDelFlag = TRUE; // + AddUnLight
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
	mis->_miDelFlag = TRUE; // + AddUnLight
}

void MI_LongExp(int mi)
{
	MissileStruct* mis;
	// TODO: add light?
	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
}

void MI_ExtExp(int mi)
{
	MissileStruct* mis;

	MI_LongExp(mi);

	mis = &missile[mi];

	// assert(mis->_miAnimLen == misfiledata[MFILE_SHATTER1].mfAnimLen[0]);
	mis->_miAnimFlag = mis->_miAnimFrame != misfiledata[MFILE_SHATTER1].mfAnimLen[0];
}

void MI_Acidsplat(int mi)
{
	MissileStruct* mis;

	mis = &missile[mi];
	// assert(mis->_miAnimLen == misfiledata[MFILE_ACIDSPLA].mfAnimLen[0]);
	/*if (mis->_miRange == misfiledata[MFILE_ACIDSPLA].mfAnimLen[0] * misfiledata[MFILE_ACIDSPLA].mfAnimFrameLen) {
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
	AddMissile(mis->_mix /* - 1*/, mis->_miy /* - 1*/, 0, 0, 0 /* mis->_miDir */, MIS_ACIDPUD, MST_MONSTER, mis->_miSource, 0);
}

void MI_Stone(int mi)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	bool dead;

	mis = &missile[mi];
	mon = &monsters[mis->_miVar1];
	dead = mon->_mhitpoints == 0;
	// assert(mon->_mmode == MM_STONE);
	mis->_miRange--;
	if (!dead) {
		if (mis->_miRange < 0) {
			mis->_miDelFlag = TRUE;
			mon->_mmode = mon->_mVar3;
			return;
		}

		mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
	} else {
		mon->_mVar3 = MM_DEATH;
		ConvertMissile(mi, MIS_EXSTONE);
	}
}

void MI_Shroud(int mi)
{
	MissileStruct *mis, *bmis;
	int i, mv;

	mis = &missile[mi];
	if (dPlayer[mis->_mix][mis->_miy] | dMonster[mis->_mix][mis->_miy])
		mis->_miRange = 0;
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		if (mis->_miDir == 0) {
			// assert(mis->_miAnimLen == misfiledata[MFILE_SHROUD].mfAnimLen[0]);
			// assert(mis->_miAnimFrameLen == 1);
			if (mis->_miAnimFrame == misfiledata[MFILE_SHROUD].mfAnimLen[0]) {
				SetMissAnim(mi, 1);
			}
		} else {
			for (i = 0; i < nummissiles; i++) {
				bmis = &missile[missileactive[i]];
				// TODO: handle MIS_FIREWALLC ?
				if (bmis->_mix != mis->_mix || bmis->_miy != mis->_miy)
					continue;
				if (bmis == mis || !(bmis->_miFlags & MIF_SHROUD))
					continue;
				mv = bmis->_miRange;
				if (mv < 0)
					continue;
				if (bmis->_miType == MIS_GUARDIAN || bmis->_miType == MIS_ELEMENTAL
#ifdef HELLFIRE
					|| bmis->_miType == MIS_HORKDMN
#endif
					)
					mv = mis->_miRange + 1;
				else if (bmis->_miType != MIS_SHROUD) {
					if (bmis->_miCaster & MST_PLAYER) {
						if (bmis->_miFlags & MIF_ARROW) {
							// physical arrow
							mv = plx(bmis->_miSource)._pLevel; // TODO: use dexterity?
						} else {
							// magical missile
							mv = bmis->_miSpllvl * 4; // TODO: use magic?
						}
					} else if (bmis->_miCaster == MST_MONSTER) {
						mv = monsters[bmis->_miSource]._mLevel;
					} else {
						mv = currLvl._dLevel;
					}
				}
				mis->_miRange -= mv;
				if (mis->_miRange < 0)
					break;
				bmis->_miRange = -1;
			}
		}
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE;
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
	//GetMissilePos(mi);
	//assert(dMonster[mis->_mix][mis->_miy] == -(mnum + 1));
	dMonster[mis->_mix][mis->_miy] = 0;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	// assert(monsters[mnum]._mFileNum != MOFILE_SNAKE || (monsters[mnum]._mAnims[MA_ATTACK].maFrames == 13 && monsters[mnum]._mAnims[MA_ATTACK].maFrameLen == 1));
	// assert(monfiledata[MOFILE_SNAKE].moAnimFrameLen[MA_ATTACK] == 1);
	if (!PosOkActor(mis->_mix, mis->_miy) || (mis->_miAnimFrame == 13 && monsters[mnum]._mFileNum == MOFILE_SNAKE)) {
		MissToMonst(mi);
		mis->_miDelFlag = TRUE;
		return;
	}
	bx = mis->_mix;
	by = mis->_miy;
	//assert(dMonster[bx][by] == 0);
	//assert(dPlayer[bx][by] == 0);
	dMonster[bx][by] = -(mnum + 1);
	monsters[mnum]._msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
	if (monsters[mnum]._mx != bx || monsters[mnum]._my != by) {
		SetMonsterLoc(&monsters[mnum], bx, by);
		// assert(monsters[mnum]._mvid == NO_VISION);
		// assert(monsters[mnum]._mlid == NO_LIGHT);
		//ChangeLightXY(monsters[mnum]._mlid, bx, by);
	}
	//ShiftMissilePos(mi);
	PutMissile(mi);
}

void MI_Charge(int mi)
{
	MissileStruct* mis;
	int bx, by, pnum;

	mis = &missile[mi];
	pnum = mis->_miSource;
	if (plr._pmode != PM_CHARGE) {
		mis->_miDelFlag = TRUE;
		return;
	}
	mis->_miRange += mis->_miAnimAdd; // MISRANGE (used in MissToPlr)
	// restore the real coordinates
	//GetMissilePos(mi);
	dPlayer[mis->_mix][mis->_miy] = 0;
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
	if (plr._px != bx || plr._py != by) {
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
		//if (myview.x != bx || myview.y != by) {
			myview.x = bx; // - ScrollInfo._sdx;
			myview.y = by; // - ScrollInfo._sdy;
		//}
	}
	//ShiftMissilePos(mi);
	PutMissile(mi);
}

/*void MI_Fireman(int mi)
{
	MissileStruct* mis;
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
	if (monsters[mnum]._menemy >= 0) {
		tnum = dPlayer[bx][by];
	} else {
		tnum = dMonster[bx][by];
	}
	if (!CheckAllowMissile(bx, by) || (tnum > 0 && !mis->_miVar1)) {
		mis->_mixvel *= -1;
		mis->_miyvel *= -1;
		mis->_miDir = OPPOSITE(mis->_miDir);
		mis->_miAnimData = monsters[mnum]._mAnims[MA_WALK].maAnimData[mis->_miDir];
		mis->_miVar2++;
		if (tnum > 0)
			mis->_miVar1 = TRUE;
	}
	ShiftMissilePos(mi);
	PutMissile(mi);
}*/

void MI_WallC(int mi)
{
	MissileStruct* mis;
	int mitype, tx, ty;

	mis = &missile[mi];
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
	if (!mis->_miVar2) {
		tx = mis->_mix + mis->_mitxoff;
		ty = mis->_miy + mis->_mityoff;
		assert(IN_DUNGEON_AREA(tx, ty));
		if (!nMissileTable[dPiece[tx][ty]]) {
			AddMissile(tx, ty, 0, 0, 0, mitype, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
		} else {
			mis->_miVar2 = TRUE;
		}
	}
	if (!mis->_miVar3 && mis->_miVar1 != 0) {
		tx = mis->_mix - mis->_mitxoff;
		ty = mis->_miy - mis->_mityoff;
		assert(IN_DUNGEON_AREA(tx, ty));
		if (!nMissileTable[dPiece[tx][ty]]) {
			AddMissile(tx, ty, 0, 0, 0, mitype, mis->_miCaster, mis->_miSource, mis->_miSpllvl);
		} else {
			mis->_miVar3 = TRUE;
		}
	}
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	mis->_miVar1++;
	// mis->_miRndSeed = NextRndSeed();
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
		mis->_miDelFlag = TRUE; // + AddUnLight
		return;
	}
	k = mis->_miAnimFrame;
	if (k > 11) {
		// assert(mis->_miAnimLen < 24);
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
	MissileStruct* mis;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
		mis->_miVar1 = mis->_mix;
		mis->_miVar2 = mis->_miy;
		if (!nMissileTable[dPiece[mis->_mix][mis->_miy]]) {
			// SetRndSeed(mis->_miRndSeed);
			// mis->_miRange used by MIS_INFERNO !
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
			// mis->_miRndSeed = NextRndSeed();
		} else {
			mis->_miRange = 0;
		}
		mis->_miRange--;
	}
	if (mis->_miRange < 0)
		mis->_miDelFlag = TRUE;
}

/*void MI_FireTrap(int mi)
{
	MissileStruct* mis;

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
	MissileStruct* mis;
	int md;
	int bpath[16] = { -1, 0, 1, -1, 0, 1, -1, -1, 0, 0, 1, 1, 0, 1, -1, 0 };

	mis = &missile[mi];
	if (mis->_miFileNum != MFILE_LGHNING) {
		if (mis->_miVar3 == 0) {
			md = (mis->_miVar2 + bpath[mis->_miVar4]) & 7;
			mis->_miVar4 = (mis->_miVar4 + 1) & 0xF;
			GetMissileVel(mi, 0, 0, XDirAdd[md], YDirAdd[md], MIS_SHIFTEDVEL(missiledata[MIS_CBOLT].mdPrSpeed));
			mis->_miVar3 = 16;
		} else {
			mis->_miVar3--;
		}
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		if ((mis->_mix != mis->_misx || mis->_miy != mis->_misy)
		 && CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_BLOCK_ANY) == 1) {
			static_assert(MAX_LIGHT_RAD >= 8, "MI_Cbolt needs at least light-radius of 8.");
			mis->_miVar1 = 8;
			mis->_miFileNum = MFILE_LGHNING;
			mis->_miRange = misfiledata[MFILE_LGHNING].mfAnimLen[0] * misfiledata[MFILE_LGHNING].mfAnimFrameLen;
			SetMissAnim(mi, 0);
		}
		ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miVar1);
	}
	mis->_miRange--;
	if (mis->_miRange >= 0) {
		PutMissile(mi);
		return;
	}
	mis->_miDelFlag = TRUE; // + AddUnLight
}

void MI_Elemental(int mi)
{
	MissileStruct* mis;
	int hit = 0, sd, cx, cy, dx, dy;

	mis = &missile[mi];
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	cx = mis->_mix;
	cy = mis->_miy;
	if (!mis->_miVar1)
		mis->_miVar1 = (cx != mis->_misx || cy != mis->_misy) ? TRUE : FALSE;
	if (mis->_miVar1)
		hit = CheckMissileCol(mi, cx, cy, MICM_BLOCK_ANY);
	if (hit == 0                                                     // did not hit anything
	 && !mis->_miVar2 && cx == mis->_miVar3 && cy == mis->_miVar4) { // destination reached the first time
		mis->_miVar2 = TRUE;
		if (FindClosest(cx, cy, dx, dy)) {
			sd = GetDirection8(cx, cy, dx, dy);
		} else {
			sd = plx(mis->_miSource)._pdir;
			dx = cx + XDirAdd[sd];
			dy = cy + YDirAdd[sd];
		}
		mis->_miVar5 = sd; // MIS_DIR
		SetMissAnim(mi, sd);
		GetMissileVel(mi, cx, cy, dx, dy, MIS_SHIFTEDVEL(missiledata[MIS_ELEMENTAL].mdPrSpeed));
	}
	if (hit == 0) {
		CondChangeLightXY(mis->_miLid, cx, cy);
		PutMissile(mi);
		return;
	}
	//CheckMissileCol(mi, cx, cy, MICM_NONE);
	// TODO: mis->_miMinDam >>= 1; mis->_miMaxDam >>= 1; ?
	CheckSplashCol(mi, hit);

	ConvertMissile(mi, MIS_EXFBALL);
}

void MI_Pulse(int mi)
{
	MissileStruct* mis;
	int dir, tmp;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange < 0) {
		mis->_miDelFlag = TRUE; // + AddUnLight
		return;
	}

	dir = mis->_miRange % 8u; // NUM_DIRS
	if (dir == 0) {
		if (CheckMissileCol(mi, mis->_mix, mis->_miy, MICM_NONE) != 0) {
			// AddMissile(mis->_mix, mis->_miy, -1, 0, 0, MIS_EXLGHT, MST_NA, 0, 0);

			mis->_miMinDam += mis->_miVar1;
			mis->_miMaxDam += mis->_miVar2;

			mis->_miVar1 *= 2;
			mis->_miVar2 *= 2;
		}
	}
	// assert(misfiledata[MFILE_MINILTNG].mfAnimLen[0] == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miFileNum = dir != 0 ? MFILE_MINILTNG : MFILE_LGHNING;
	tmp = mis->_miAnimFrame;
	if (dir == 0) {
		// assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
		tmp = ((unsigned)tmp % misfiledata[MFILE_LGHNING].mfAnimLen[0]) + 1;
	}
	SetMissAnim(mi, 0);
	mis->_miAnimFrame = tmp;
	mis->_miPreFlag = dir != 0;
	mis->_miyoff = dir != 0 ? TILE_HEIGHT/2 - ((NUM_DIRS - 1) + dir) * ASSET_MPL : 0;
	PutMissileF(mi, dir != 0 ? BFLAG_MISSILE_PRE : 0);
}

void ProcessMissiles()
{
	MissileStruct* mis;
	int i, mi;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		assert(IN_DUNGEON_AREA(mis->_mix, mis->_miy));
		dFlags[mis->_mix][mis->_miy] &= ~(BFLAG_MISSILE_PRE | BFLAG_HAZARD);
		dMissile[mis->_mix][mis->_miy] = 0;
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

	DeleteMissiles();
}

void SyncMissilesAnim()
{
	int i, mi;

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		SyncMissAnim(mi);
		if (missile[mi]._miType == MIS_RHINO) {
			SyncRhinoAnim(mi);
		} else if (missile[mi]._miType == MIS_CHARGE) {
			SyncChargeAnim(mi);
		}
	}
}

DEVILUTION_END_NAMESPACE
