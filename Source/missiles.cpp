/**
 * @file missiles.cpp
 *
 * Implementation of missile functionality.
 */
#include "all.h"
#include "plrctrls.h"
#include "misproc.h"

DEVILUTION_BEGIN_NAMESPACE

int missileactive[MAXMISSILES];
int missileavail[MAXMISSILES];
MissileStruct missile[MAXMISSILES];
int nummissiles;
bool gbMissilePreFlag;

/** Maps from direction to X-offset. */
const int XDirAdd[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to Y-offset. */
const int YDirAdd[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };

void GetDamageAmt(int sn, int *mind, int *maxd)
{
	PlayerStruct *p;
	int k, sl;

	assert((unsigned)myplr < MAX_PLRS);
	assert((unsigned)sn < NUM_SPELLS);
	sl = GetSpellLevel(myplr, sn);
	p = &plr[myplr];

	switch (sn) {
	case SPL_FIREBOLT:
		k = (p->_pMagic >> 3) + sl;
		*mind = k + 1;
		*maxd = k + 10;
		break;
#ifdef HELLFIRE
	case SPL_RUNELIGHT:
#endif
	case SPL_LIGHTNING:
		*mind = 1;
		*maxd = ((p->_pMagic + (sl << 3)) * (6 + (sl >> 1))) >> 3;
		break;
	case SPL_FLASH:
		*mind = p->_pMagic >> 1;
		for (k = 0; k < sl; k++)
			*mind += *mind >> 3;

		*mind *= 19;
		*maxd = *mind << 3;
		*mind >>= 6;
		*maxd >>= 6;
		break;
	case SPL_NULL:
	case SPL_WALK:
	case SPL_BLOCK:
	case SPL_ATTACK:
	case SPL_SWIPE:
	case SPL_RATTACK:
	case SPL_POINT_BLANK:
	case SPL_FAR_SHOT:
	case SPL_STONE:
	case SPL_INFRA:
	case SPL_MANASHIELD:
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
	case SPL_BLODBOIL:
	case SPL_WHITTLE:
	case SPL_RUNESTONE:
#endif
		*mind = -1;
		*maxd = -1;
		break;
#ifdef HELLFIRE
	case SPL_FIRERING:
#endif
	case SPL_FIREWALL:
		*mind = ((p->_pMagic >> 3) + sl + 5) << (-3 + 5);
		*maxd = ((p->_pMagic >> 3) + sl * 2 + 10) << (-3 + 5);
		break;
	case SPL_FIREBALL:
		*mind = (p->_pMagic >> 2) + 10;
		*maxd = *mind + 10;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
			*maxd += *maxd >> 3;
		}
		break;
	case SPL_GUARDIAN:
		k = (p->_pMagic >> 3) + sl;
		*mind = k + 1;
		*maxd = k + 10;
		break;
	case SPL_CHAIN:
		*mind = 1;
		*maxd = p->_pMagic;
		break;
	case SPL_WAVE:
		*mind = ((p->_pMagic >> 3) + sl + 1) * 4;
		*maxd = ((p->_pMagic >> 3) + 2 * sl + 2) * 4;
		break;
#ifdef HELLFIRE
	case SPL_RUNENOVA:
#endif
	case SPL_NOVA:
		*mind = 1;
		*maxd = (p->_pMagic >> 1) + (sl << 4);
		break;
	case SPL_FLAME:
		*mind = (p->_pMagic * 20) >> 6;
		*maxd = ((p->_pMagic + (sl << 4)) * 30) >> 6;
		break;
	case SPL_GOLEM:
		*mind = 2 * sl + 8;
		*maxd = 2 * sl + 16;
		break;
	case SPL_ELEMENT:
		*mind = (p->_pMagic >> 3) + 2 * sl + 4;
		*maxd = (p->_pMagic >> 3) + 4 * sl + 20;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
			*maxd += *maxd >> 3;
		}
		break;
	case SPL_CBOLT:
		*mind = 1;
		*maxd = (p->_pMagic >> 2) + (sl << 2);
		break;
	case SPL_HBOLT:
		*mind = (p->_pMagic >> 2) + sl;
		*maxd = *mind + 9;
		break;
	case SPL_FLARE:
		*mind = (p->_pMagic * (sl + 1)) >> 3;
		*maxd = *mind;
		break;
#ifdef HELLFIRE
	case SPL_LIGHTWALL:
		*mind = 1;
		*maxd = ((p->_pMagic >> 1) + sl) << (-3 + 5);
		break;
	case SPL_RUNEIMMOLAT:
	case SPL_IMMOLAT:
		*mind = 1 + (p->_pMagic >> 3);
		*maxd = *mind + 4;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
			*maxd += *maxd >> 3;
		}
		break;
	case SPL_RUNEFIRE:
		*mind = 2 * p->_pLevel + 4;
		*maxd = *mind + 18;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
			*maxd += *maxd >> 3;
		}
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static bool PosOkMissile1(int x, int y)
{
	return !nMissileTable[dPiece[x][y]];
}

static bool FindClosest(int sx, int sy, int &dx, int &dy)
{
	int j, i, mid, tx, ty;
	const char *cr;

	for (i = 1; i < 16; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mid = dMonster[tx][ty];
			if (mid > 0
			 && monster[mid - 1]._mhitpoints >= (1 << 6)
			 && LineClearF(CheckNoSolid, sx, sy, tx, ty)) {
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
	int j, i, mid, tx, ty;
	const char *cr;
	
	for (i = 1; i < 8; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			assert(IN_DUNGEON_AREA(tx, ty));
			mid = dMonster[tx][ty];
			if (mid > 0
			 && (monster[mid - 1].mMagicRes & MORS_LIGHTNING_IMMUNE) != MORS_LIGHTNING_IMMUNE
			 && monster[mid - 1]._mhitpoints >= (1 << 6)
			 && LineClearF(CheckNoSolid, sx, sy, tx, ty)) {
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
	/*int mx, my, md;

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
	return md;*/
	// The implementation of above with fewer branches
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
	/*int mx, my, md;
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
	return md;*/
	// The implementation of above with fewer branches
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
}

void DeleteMissile(int mi, int idx)
{
	missileavail[MAXMISSILES - nummissiles] = mi;
	nummissiles--;
	if (nummissiles > 0 && idx != nummissiles)
		missileactive[idx] = missileactive[nummissiles];
}

static void GetMissileVel(int mi, int sx, int sy, int dx, int dy, int v)
{
	double dxp, dyp, dr;

	dx -= sx;
	dy -= sy;
	if (dx != 0 || dy != 0) {
		dxp = (dx - dy) << 21;
		dyp = (dy + dx) << 21;
		dr = sqrt(dxp * dxp + dyp * dyp);
		missile[mi]._mixvel = (dxp * (v << 16)) / dr;
		missile[mi]._miyvel = (dyp * (v << 15)) / dr;
	} else {
		missile[mi]._mixvel = 0;
		missile[mi]._miyvel = 0;
	}
}

static void PutMissile(int mi)
{
	int x, y;

	x = missile[mi]._mix;
	y = missile[mi]._miy;
	if (x <= 0 || y <= 0 || x >= MAXDUNX || y >= MAXDUNY)
		missile[mi]._miDelFlag = TRUE;
	else if (!missile[mi]._miDelFlag) {
		dFlags[x][y] |= BFLAG_MISSILE;
		if (dMissile[x][y] == 0)
			dMissile[x][y] = mi + 1;
		else
			dMissile[x][y] = -1;
		if (missile[mi]._miPreFlag)
			gbMissilePreFlag = true;
	}
}

static void GetMissilePos(int mi)
{
	MissileStruct *mis;
	int mx, my, dx, dy, lx, ly;

	mis = &missile[mi];
	mx = mis->_mitxoff >> 16;
	my = mis->_mityoff >> 15;

	dx = mx + my;
	dy = my - mx;
	lx = dx / 8;
	dx = dx / 64;
	ly = dy / 8;
	dy = dy / 64;

	mis->_mix = dx + mis->_misx;
	mis->_miy = dy + mis->_misy;
	mis->_mixoff = mx + (dy * 32) - (dx * 32);
	mis->_miyoff = (my >> 1) - (dx * 16) - (dy * 16);
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
	if (PosOkMonst(mis->_miSource, x, y)) {
		mis->_mix += dx;
		mis->_miy += dy;
		mis->_mixoff += (dy << 5) - (dx << 5);
		mis->_miyoff -= (dy << 4) + (dx << 4);
	}
}

unsigned CalcMonsterDam(unsigned short mor, unsigned char mRes, unsigned mindam, unsigned maxdam)
{
	unsigned dam;
	BYTE resist;

	switch (mRes) {
	case MISR_NONE:
		resist = MORT_NONE;
		break;
	case MISR_SLASH:
		mor &= MORS_SLASH_IMMUNE;
		if (mor == MORS_SLASH_IMMUNE)
			return 0;
		resist = mor >> MORS_IDX_SLASH;
		break;
	case MISR_BLUNT:
		mor &= MORS_BLUNT_IMMUNE;
		if (mor == MORS_BLUNT_IMMUNE)
			return 0;
		resist = mor >> MORS_IDX_BLUNT;
		break;
	case MISR_PUNCTURE:
		mor &= MORS_PUNCTURE_IMMUNE;
		if (mor == MORS_PUNCTURE_IMMUNE)
			return 0;
		resist = mor >> MORS_IDX_PUNCTURE;
		break;
	case MISR_FIRE:
		mor &= MORS_FIRE_IMMUNE;
		if (mor == MORS_FIRE_IMMUNE)
			return 0;
		resist = mor >> MORS_IDX_FIRE;
		break;
	case MISR_LIGHTNING:
		mor &= MORS_LIGHTNING_IMMUNE;
		if (mor == MORS_LIGHTNING_IMMUNE)
			return 0;
		resist = mor >> MORS_IDX_LIGHTNING;
		break;
	case MISR_MAGIC:
		mor &= MORS_MAGIC_IMMUNE;
		if (mor == MORS_MAGIC_IMMUNE)
			return 0;
		resist = mor >> MORS_IDX_MAGIC;
		break;
	case MISR_ACID:
		mor &= MORS_ACID_IMMUNE;
		if (mor == MORS_ACID_IMMUNE)
			return 0;
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
		dam >>= 1;
		dam += dam >> 2;
		break;
	case MORT_RESIST:
		dam >>= 2;
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
	AddMissile(dx, dy, -1, 0, 0, mtype, 0, 0, 0, 0, 0);
	/*int gfx = random_(8, dam);
	if (gfx >= dam - (fdam + ldam)) {
		if (gfx < dam - ldam) {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPFEXP, 0, 0, 0, 0);
		} else {
			AddMissile(dx, dy, 0, 0, 0, MIS_WEAPLEXP, 0, 0, 0, 0);
		}
	}*/
}

static bool MonsterTrapHit(int mnum, int mi)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	int hper, dam;
	bool ret;

	mon = &monster[mnum];
	mis = &missile[mi];
	hper = 90 - mon->mArmorClass - mis->_miDist;
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(68, 100) >= hper && mon->_mmode != MM_STONE)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return false;

	dam = CalcMonsterDam(mon->mMagicRes, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
	if (dam == 0)
		return false;

	if (CheckMonsterHit(mnum, &ret)) {
		return ret;
	}

	mon->_mhitpoints -= dam;
#ifdef _DEBUG
	if (debug_mode_god_mode)
		mon->_mhitpoints = 0;
#endif
	if (mon->_mhitpoints < (1 << 6)) {
		MonStartKill(mnum, -1);
	} else {
		/*if (resist) {
			PlayEffect(mnum, 1);
		} else {*/
			MonStartHit(mnum, -1, dam);
		//}
	}
	return true;
}

static bool MonsterMHit(int mnum, int mi)
{
	PlayerStruct *p;
	MonsterStruct *mon;
	MissileStruct *mis;
	int pnum, tmac, hper, dam;
	bool ret;

	mon = &monster[mnum];
	mis = &missile[mi];
	pnum = mis->_miSource;
	p = &plr[pnum];
	if (mis->_miSubType == 0) {
		tmac = mon->mArmorClass;
		if (p->_pIEnAc > 0) {
			int _pIEnAc = p->_pIEnAc - 1;
			if (_pIEnAc > 0)
				tmac >>= _pIEnAc;
			else
				tmac -= tmac >> 2;
		}
		hper = p->_pIHitChance - tmac
		    - (mis->_miDist * mis->_miDist >> 1);
	} else {
		hper = 50 + p->_pMagic
			- (mon->mLevel << 1)
			/*- dist*/; // TODO: either don't care about it, or set it!
	}
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(69, 100) >= hper && mon->_mmode != MM_STONE)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return false;

	if (CheckMonsterHit(mnum, &ret))
		return ret;

	if (mis->_miSubType == 0) {
		// calculcate arrow-damage
		dam = 0;
		int sldam = p->_pISlMaxDam;
		if (sldam != 0) {
			dam += CalcMonsterDam(mon->mMagicRes, MISR_SLASH, p->_pISlMinDam, sldam);
		}
		int bldam = p->_pIBlMaxDam;
		if (bldam != 0) {
			dam += CalcMonsterDam(mon->mMagicRes, MISR_BLUNT, p->_pIBlMinDam, bldam);
		}
		int pcdam = p->_pIPcMaxDam;
		if (pcdam != 0) {
			dam += CalcMonsterDam(mon->mMagicRes, MISR_PUNCTURE, p->_pIPcMinDam, pcdam);
		}
		// add modifiers from arrow-type
		if (mis->_miType == MIS_PBARROW) {
			dam = (dam * (64 + 32 - 16 * mis->_miDist + mis->_miSpllvl)) >> 6;
		} else if (mis->_miType == MIS_ASARROW) {
			dam = (dam * (8 * mis->_miDist - 16 + mis->_miSpllvl)) >> 5;
		}
		int fdam = p->_pIFMaxDam;
		if (fdam != 0) {
			fdam = CalcMonsterDam(mon->mMagicRes, MISR_FIRE, p->_pIFMinDam, fdam);
		}
		int ldam = p->_pILMaxDam;
		if (ldam != 0) {
			ldam = CalcMonsterDam(mon->mMagicRes, MISR_LIGHTNING, p->_pILMinDam, ldam);
		}
		int mdam = p->_pIMMaxDam;
		if (mdam != 0) {
			mdam = CalcMonsterDam(mon->mMagicRes, MISR_MAGIC, p->_pIMMinDam, mdam);
		}
		int adam = p->_pIAMaxDam;
		if (adam != 0) {
			adam = CalcMonsterDam(mon->mMagicRes, MISR_ACID, p->_pIAMinDam, adam);
		}
		if ((ldam | fdam | mdam | adam) != 0) {
			dam += fdam + ldam + mdam + adam;
			AddElementalExplosion(mon->_mx, mon->_my, fdam, ldam, mdam, adam);
		}

		if (p->_pILifeSteal != 0) {
			PlrIncHp(pnum, (dam * p->_pILifeSteal) >> 7);
		}
		if (p->_pIManaSteal != 0) {
			PlrIncMana(pnum, (dam * p->_pIManaSteal) >> 7);
		}
	} else {
		dam = CalcMonsterDam(mon->mMagicRes, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
		if (dam == 0)
			return false;
	}

	if (pnum == myplr)
		mon->_mhitpoints -= dam;

	if (mon->_mhitpoints < (1 << 6)) {
		MonStartKill(mnum, pnum);
	} else {
		/*if (resist != MORT_NONE) {
			PlayEffect(mnum, 1);
		} else {*/
			if (mis->_miSubType == 0) {
				if (p->_pIFlags & ISPL_NOHEALMON)
					mon->_mFlags |= MFLAG_NOHEAL;
				if ((p->_pIFlags & ISPL_KNOCKBACK) && mon->_mmode != MM_STONE)
					MonGetKnockback(mnum);
			}
			MonStartHit(mnum, pnum, dam);
		//}
	}

	if (mon->_msquelch == 0) {
		mon->_msquelch = UCHAR_MAX;
		mon->_lastx = p->_px;
		mon->_lasty = p->_py;
	}
	return true;
}

int CalcPlrDam(PlayerStruct *p, BYTE mRes, unsigned mindam, unsigned maxdam)
{
	int dam;
	char resist;

	switch (mRes) {
	case MISR_NONE:
	case MISR_SLASH:  // TODO: add dps->_pSlash/Blunt/PunctureResist
	case MISR_BLUNT:
	case MISR_PUNCTURE:
		resist = 0;
		break;
	case MISR_FIRE:
		resist = p->_pFireResist;
		break;
	case MISR_LIGHTNING:
		resist = p->_pLghtResist;
		break;
	case MISR_MAGIC:
		resist = p->_pMagResist;
		break;
	case MISR_ACID:
		resist = p->_pAcidResist;
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
	PlayerStruct *p;
	int hper, tmp, dam;

	p = &plr[pnum];
	if (p->_pInvincible) {
		return false;
	}

	mis = &missile[mi];
	if (mis->_miSubType == 0) {
		hper = 100 + (2 * currLvl._dLevel)
		    + (2 * currLvl._dLevel)
			- (2 * p->_pLevel)
		    - p->_pIAC;
		hper -= mis->_miDist << 1;
	} else {
		hper = 40
			+ (2 * currLvl._dLevel)
			- (2 * p->_pLevel);
	}

	if (random_(72, 100) >= hper)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return false;

	if (!(mis->_miFlags & MIFLAG_NOBLOCK)) {
		tmp = p->_pIBlockChance;
		if (tmp != 0 && (p->_pmode == PM_STAND || p->_pmode == PM_BLOCK)) {
			// assert(p->_pSkillFlags & SFLAG_BLOCK);
			tmp = tmp - (currLvl._dLevel << 1);
			if (tmp > random_(73, 100)) {
				PlrStartBlock(pnum, p->_pdir);
				return true;
			}
		}
	}

	dam = CalcPlrDam(p, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
	if (dam == 0)
		return false;

	dam += p->_pIGetHit;
	if (dam < 64)
		dam = 64;

	if (pnum != myplr || !PlrDecHp(pnum, dam, 0))
		StartPlrHit(pnum, dam, false);
	return true;
}

static bool PlayerMHit(int pnum, int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;
	MonsterStruct *mon;
	int hper, tmp, dam;

	p = &plr[pnum];
	if (p->_pInvincible) {
		return false;
	}
	mis = &missile[mi];
	mon = &monster[mis->_miSource];
	if (mis->_miSubType == 0) {
		hper = 30 + mon->mHit
		    + (2 * mon->mLevel)
			- (2 * p->_pLevel)
		    - p->_pIAC;
		hper -= mis->_miDist << 1;
	} else {
		hper = 40
			+ (2 * mon->mLevel)
			- (2 * p->_pLevel)
			/*- (dist << 1)*/; // TODO: either don't care about it, or set it!
	}

	if (random_(72, 100) >= hper)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return false;

	if (!(mis->_miFlags & MIFLAG_NOBLOCK)) {
		tmp = p->_pIBlockChance;
		if (tmp != 0 && (p->_pmode == PM_STAND || p->_pmode == PM_BLOCK)) {
			// assert(p->_pSkillFlags & SFLAG_BLOCK);
			tmp = tmp - (mon->mLevel << 1);
			if (tmp > random_(73, 100)) {
				tmp = GetDirection(p->_px, p->_py, mon->_mx, mon->_my);
				PlrStartBlock(pnum, tmp);
				return true;
			}
		}
	}

	dam = CalcPlrDam(p, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
	if (dam == 0)
		return false;
	dam += p->_pIGetHit;
	if (dam < 64)
		dam = 64;

	if (pnum != myplr || !PlrDecHp(pnum, dam, 0))
		StartPlrHit(pnum, dam, false);
	return true;
}

static bool Plr2PlrMHit(int defp, int mi)
{
	MissileStruct *mis;
	PlayerStruct *ops, *dps;
	int offp, dam, blkper, hper;

	dps = &plr[defp];
	if (dps->_pInvincible) {
		return false;
	}

	mis = &missile[mi];
	offp = mis->_miSource;
	ops = &plr[offp];
	if (mis->_miSubType == 0) {
		hper = ops->_pIHitChance
		    - (mis->_miDist * mis->_miDist >> 1)
		    - dps->_pIAC;
	} else {
		if (mis->_miFlags & MIFLAG_AREA) {
			hper = 40
				+ (ops->_pLevel << 1)
				- (dps->_pLevel << 1);
		} else {
			hper = 50 + ops->_pMagic
				- (dps->_pLevel << 1)
				/*- dist*/; // TODO: either don't care about it, or set it!
		}
	}
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(69, 100) >= hper)
		return false;

	if (!(mis->_miFlags & MIFLAG_NOBLOCK)) {
		blkper = dps->_pIBlockChance;
		if (blkper != 0 && (dps->_pmode == PM_STAND || dps->_pmode == PM_BLOCK)) {
			// assert(dps->_pSkillFlags & SFLAG_BLOCK);
			blkper = blkper - (ops->_pLevel << 1);
			if (blkper > random_(73, 100)) {
				PlrStartBlock(defp, GetDirection(dps->_px, dps->_py, ops->_px, ops->_py));
				return true;
			}
		}
	}

	if (mis->_miSubType == 0) {
		dam = 0;
		int sldam = ops->_pISlMaxDam;
		if (sldam != 0) {
			dam += CalcPlrDam(dps, MISR_SLASH, ops->_pISlMinDam, sldam);
		}
		int bldam = ops->_pIBlMaxDam;
		if (bldam != 0) {
			dam += CalcPlrDam(dps, MISR_BLUNT, ops->_pIBlMinDam, bldam);
		}
		int pcdam = ops->_pIPcMaxDam;
		if (pcdam != 0) {
			dam += CalcPlrDam(dps, MISR_PUNCTURE, ops->_pIPcMinDam, pcdam);
		}
		// add modifiers from arrow-type
		if (mis->_miType == MIS_PBARROW) {
			dam = (dam * (64 + 32 - 16 * mis->_miDist + mis->_miSpllvl)) >> 6;
		} else if (mis->_miType == MIS_ASARROW) {
			dam = (dam * (8 * mis->_miDist - 16 + mis->_miSpllvl)) >> 5;
		}

		if (ops->_pILifeSteal != 0) {
			PlrIncHp(offp, (dam * ops->_pILifeSteal) >> 7);
		}

		int fdam = ops->_pIFMaxDam;
		if (fdam != 0) {
			fdam = CalcPlrDam(dps, MISR_FIRE, ops->_pIFMinDam, fdam);
		}
		int ldam = ops->_pILMaxDam;
		if (ldam != 0) {
			ldam = CalcPlrDam(dps, MISR_LIGHTNING, ops->_pILMinDam, ldam);
		}
		int mdam = ops->_pIMMaxDam;
		if (mdam != 0) {
			mdam = CalcPlrDam(dps, MISR_MAGIC, ops->_pIMMinDam, mdam);
		}
		int adam = ops->_pIAMaxDam;
		if (adam != 0) {
			adam = CalcPlrDam(dps, MISR_ACID, ops->_pIAMinDam, adam);
		}
		if ((ldam | fdam | mdam | adam) != 0) {
			dam += ldam + fdam + mdam + adam;
			AddElementalExplosion(dps->_px, dps->_py, fdam, ldam, mdam, adam);
		}
	} else {
		dam = CalcPlrDam(dps, mis->_miResist, mis->_miMinDam, mis->_miMaxDam);
		if (dam == 0)
			return false;
		dam >>= 1;
	}

	if (offp == myplr)
		NetSendCmdDwParam2(true, CMD_PLRDAMAGE, defp, dam);
	StartPlrHit(defp, dam, false);
	return true;
}

static bool MonMissHit(int mnum, int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miSource != -1) {
		if (mis->_miCaster == 0) {
			// player vs. monster
			return MonsterMHit(mnum, mi);
		} else {
			// monster vs. golem
			return mnum < MAX_MINIONS && MonsterTrapHit(mnum, mi);
		}
	} else {
		// trap vs. monster
		return MonsterTrapHit(mnum, mi);
	}
}

static bool PlrMissHit(int pnum, int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miSource != -1) {
		if (mis->_miCaster == 0) {
			// player vs. player
			return plr[pnum]._pTeam != plr[mis->_miSource]._pTeam && Plr2PlrMHit(pnum, mi);
		} else {
			// monster vs. player
			return PlayerMHit(pnum, mi);
		}
	} else {
		// trap vs. player
		return PlayerTrapHit(pnum, mi);
	}
}

/**
 * Check if the monster is on a given tile.
 * Uses _mVar8 of MonsterStruct! -> TODO: move to monster.cpp?
 */
static int CheckMonCol(int mnum, int mx, int my)
{
	MonsterStruct *mon;
	int mode;
	bool negate;
	bool halfOver;

	if (mnum > 0) {
		mnum--;
		negate = true;
	} else {
		mnum = -(mnum + 1);
		negate = false;
	}

	mon = &monster[mnum];
	mode = mon->_mmode;
	static_assert(MM_WALK3 - MM_WALK == 2, "CheckMonCol expects ordered MM_WALKs I.");
	static_assert(MM_WALK + 1 == MM_WALK2, "CheckMonCol expects ordered MM_WALKs II.");
	if (mode > MM_WALK3 || mode < MM_WALK)
		return (negate || mode == MM_STONE) ? mnum : -1;
	halfOver = mon->_mVar8 >= (mon->_mAnims[MA_WALK].Frames >> 1);
	if (mode == MM_WALK) {
		if (negate)
			halfOver = !halfOver;
		return halfOver ? mnum : -1;
	}
	if (mode == MM_WALK2) {
		if (negate)
			halfOver = !halfOver;
		return halfOver ? -1 : mnum;
	}
	assert(!negate);
	if (halfOver) {
		return (mon->_mfutx == mx && mon->_mfuty == my) ? mnum : -1;
	} else {
		return (mon->_mx == mx && mon->_my == my) ? mnum : -1;
	}
}

static int CheckPlrCol(int pnum, int mx, int my)
{
	PlayerStruct *p;
	int mode;
	bool negate;
	bool halfOver;

	if (pnum > 0) {
		pnum--;
		negate = true;
	} else {
		pnum = -(pnum + 1);
		negate = false;
	}
	p = &plr[pnum];
	mode = p->_pmode;
	static_assert(PM_WALK3 - PM_WALK == 2, "CheckPlrCol expects ordered PM_WALKs I.");
	static_assert(PM_WALK + 1 == PM_WALK2, "CheckPlrCol expects ordered PM_WALKs II.");
	if (mode > PM_WALK3 || mode < PM_WALK)
		return negate ? pnum : -1;
	halfOver = p->_pAnimFrame >= (p->_pWFrames >> 1);
	if (mode == PM_WALK) {
		if (negate)
			halfOver = !halfOver;
		return halfOver ? pnum : -1;
	}
	if (mode == PM_WALK2) {
		if (negate)
			halfOver = !halfOver;
		return halfOver ? -1 : pnum;
	}
	assert(!negate);
	if (halfOver) {
		return (p->_pfutx == mx && p->_pfuty == my) ? pnum : -1;
	} else {
		return (p->_px == mx && p->_py == my) ? pnum : -1;
	}
}

static bool CheckMissileCol(int mi, int mx, int my, bool nodel)
{
	MissileStruct *mis;
	const MissileData *mds;
	int oi, mnum, pnum;
	int hit = 0;

	mnum = dMonster[mx][my];
	if (mnum != 0) {
		mnum = CheckMonCol(mnum, mx, my);
		if (mnum != -1 && MonMissHit(mnum, mi))
			hit = 1;
	}

	pnum = dPlayer[mx][my];
	if (pnum != 0) {
		pnum = CheckPlrCol(pnum, mx, my);
		if (pnum != -1 && PlrMissHit(pnum, mi))
			hit = 1;
	}

	oi = dObject[mx][my];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (!object[oi]._oMissFlag) {
			if (object[oi]._oBreak == 1)
				OperateObject(-1, oi, false);
			hit = 2;
		}
	}
	if (nMissileTable[dPiece[mx][my]]) {
		hit = 2;
	}

	if (hit == 0)
		return false;

	if (!nodel) {
		mis = &missile[mi];
		mis->_miRange = 0;
		mds = &missiledata[mis->_miType];
		if (mds->miSFX != -1)
			PlaySfxLoc(mds->miSFX, mis->_mix, mis->_miy, mds->miSFXCnt);
	}
	return hit == 1;
}

static void SetMissAnim(int mi, int animtype)
{
	MissileStruct *mis;
	const MisFileData *mfd;
	int dir;

	mis = &missile[mi];
	mis->_miAnimType = animtype;
	dir = mis->_miDir;
	mis->_miAnimData = misanimdata[animtype][dir];
	mfd = &misfiledata[animtype];
	mis->_miAnimFlags = mfd->mfFlags;
	mis->_miAnimDelay = mfd->mfAnimDelay[dir];
	mis->_miAnimLen = mfd->mfAnimLen[dir];
	mis->_miAnimWidth = mfd->mfAnimWidth[dir];
	mis->_miAnimWidth2 = mfd->mfAnimWidth2[dir];
	mis->_miAnimCnt = 0;
	mis->_miAnimFrame = 1;
}

static void SetMissDir(int mi, int dir)
{
	missile[mi]._miDir = dir;
	SetMissAnim(mi, missile[mi]._miAnimType);
}

void LoadMissileGFX(BYTE midx)
{
	char pszName[256];
	int i, n;
	unsigned char **mad;
	const char *name;

	n = misfiledata[midx].mfAnimFAmt;
	name = misfiledata[midx].mfName;
	mad = misanimdata[midx];
	if (n == 1) {
		snprintf(pszName, sizeof(pszName), "Missiles\\%s.CL2", name);
		if (mad[0] == NULL)
			mad[0] = LoadFileInMem(pszName, NULL);
	} else {
		for (i = 0; i < n; i++) {
			snprintf(pszName, sizeof(pszName), "Missiles\\%s%i.CL2", name, i + 1);
			if (mad[i] == NULL) {
				mad[i] = LoadFileInMem(pszName, NULL);
			}
		}
	}
}

void InitMissileGFX()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (!(misfiledata[i].mfFlags & MAFLAG_HIDDEN))
			LoadMissileGFX(i);
	}
}

static void FreeMissileGFX(int midx, int n)
{
	unsigned char **mad;
	int i;

	mad = misanimdata[midx];
	for (i = 0; i < n; i++) {
		if (mad[i] != NULL) {
			MemFreeDbg(mad[i]);
		}
	}
}

void FreeMissiles()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (!(misfiledata[i].mfFlags & MAFLAG_HIDDEN))
			FreeMissileGFX(i, misfiledata[i].mfAnimFAmt);
	}
}

void FreeMissiles2()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (misfiledata[i].mfFlags & MAFLAG_HIDDEN)
			FreeMissileGFX(i, misfiledata[i].mfAnimFAmt);
	}
}

void InitMissiles()
{
	MissileStruct *mis;
	PlayerStruct *p;
	int i;
	BYTE *pTmp;

	p = &plr[myplr];
	if (p->_pInfraFlag) {
		for (i = 0; i < nummissiles; ++i) {
			mis = &missile[missileactive[i]];
			if (mis->_miType == MIS_INFRA && mis->_miSource == myplr) {
				CalcPlrItemVals(myplr, true);
			}
		}
	}

#ifdef HELLFIRE
	if (p->_pSpellFlags & (PSE_BLOOD_BOIL | PSE_LETHARGY)) {
		p->_pSpellFlags &= ~(PSE_BLOOD_BOIL | PSE_LETHARGY);
		for (i = 0; i < nummissiles; ++i) {
			mis = &missile[missileactive[i]];
			if (mis->_miType == MIS_BLODBOIL && mis->_miSource == myplr) {
				int missingHP = p->_pMaxHP - p->_pHitPoints;
				CalcPlrItemVals(myplr, true);
				p->_pHitPoints -= missingHP + mis->_miVar2;
				if (p->_pHitPoints < (1 << 6)) {
					p->_pHitPoints = (1 << 6);
				}
			}
		}
	}
#endif

	nummissiles = 0;
	memset(missileactive, 0, sizeof(missileactive));
	for (i = 0; i < MAXMISSILES; i++) {
		missileavail[i] = i;
	}
	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in InitMissiles.");
	pTmp = &dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++)
		*pTmp &= ~BFLAG_MISSILE;
}

#ifdef HELLFIRE
int AddHiveexpC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int i, j;

	for (i = sx; i <= dx; i++)
		for (j = sy; j <= dy; j++)
			AddMissile(i, j, 0, 0, 0, MIS_HIVEEXP, micaster, misource, 0, 0, 0);
	return MIRES_DELETE;
}

static bool place_rune(int mi, int dx, int dy, int mitype, int mirange)
{
	int i, j, tx, ty;
	const char *cr;

	for (i = 0; i < 10; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = *cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			if (IN_DUNGEON_AREA(tx, ty)
			 && ((nSolidTable[dPiece[tx][ty]] | dObject[tx][ty] | dMissile[tx][ty]) == 0)) {
				missile[mi]._mix = tx;
				missile[mi]._miy = ty;
				missile[mi]._miVar1 = mitype;
				missile[mi]._miVar2 = mirange;		// trigger range
				missile[mi]._miRange = 16 + 1584;	// delay + ttl
				missile[mi]._miLid = AddLight(tx, ty, 8);
				return true;
			}
		}
	}
	return false;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 */
int AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (place_rune(mi, dx, dy, MIS_HIVEEXP, 0))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 */
int AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (place_rune(mi, dx, dy, MIS_LIGHTNINGC, 1))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 */
int AddGreatLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (place_rune(mi, dx, dy, MIS_LIGHTNOVAC, 1))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 */
int AddImmolationRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (place_rune(mi, dx, dy, MIS_FIRENOVAC, 1))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: mitype to fire upon impact
 * Var2: range of the rune
 */
int AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (place_rune(mi, dx, dy, MIS_STONE, 0))
			return MIRES_DONE;
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: direction to place the spawn
 */
int AddHorkSpawn(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	GetMissileVel(mi, sx, sy, dx, dy, 8);
	// missile[mi]._miMinDam = missile[mi]._miMaxDam = 0;
	missile[mi]._miRange = 9;
	missile[mi]._miVar1 = midir;
	//PutMissile(mi);
	return MIRES_DONE;
}

int AddLightwall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	assert((unsigned)misource < MAX_PLRS);
	//if (misource != -1) {
		// TODO: bring it closer to AddFirewall? (_pISplDur, adjust damage)
		mis->_miMaxDam = ((plr[misource]._pMagic >> 1) + spllvl) << (-3 + 6);
	//} else {
	//	mis->_miMaxDam = (20 + currLvl._dLevel) << (-2 + 6);
	//}
	mis->_miMinDam = 1 << (-5 + 6);
	mis->_miRange = 255 * (spllvl + 1);
	mis->_miAnimFrame = RandRange(1, 8);
	return MIRES_DONE;
}

int AddHiveexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	mis = &missile[mi];
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = mis->_miAnimLen - 1;

	dam = 2 * (plr[misource]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
	for (i = spllvl; i > 0; i--) {
		dam += dam >> 3;
	}
	dam <<= 6;
	mis->_miMinDam = mis->_miMaxDam = dam;
	CheckMissileCol(mi, sx - 1, sy - 1, true);
	CheckMissileCol(mi, sx, sy - 1, true);
	CheckMissileCol(mi, sx + 1, sy - 1, true);
	CheckMissileCol(mi, sx - 1, sy, true);
	CheckMissileCol(mi, sx, sy, true);
	CheckMissileCol(mi, sx + 1, sy, true);
	CheckMissileCol(mi, sx - 1, sy + 1, true);
	CheckMissileCol(mi, sx, sy + 1, true);
	CheckMissileCol(mi, sx + 1, sy + 1, true);
	return MIRES_DONE;
}

/**
 * Remark: expects damage to be shifted!
 *
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddFireball2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int av = 16;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	assert((unsigned)misource < MAX_PLRS);
	av += spllvl;
	if (av > 50) {
		av = 50;
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
	return MIRES_DONE;
}

int AddRingC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	return MIRES_DONE;
}
#endif

/*
 * Var1: x coordinate of the missile-target of MIS_ASARROW
 * Var2: y coordinate of the missile-target of MIS_ASARROW
 */
int AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int av = 32, mtype;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	midir = GetDirection16(sx, sy, dx, dy);
	if (micaster == 0) {
		p = &plr[misource];
		av += p->_pIArrowVelBonus;
		//int dam = p->_pIMaxDam + p->_pIMinDam;
		int fdam = p->_pIFMaxDam;
		int ldam = p->_pILMaxDam;
		int mdam = p->_pIMMaxDam;
		int adam = p->_pIAMaxDam;
		if ((ldam | fdam | mdam | adam) != 0) {
			if ((fdam | ldam) >= (mdam | adam)) {
				mtype = fdam >= ldam ? MFILE_FARROW : MFILE_LARROW;
			} else {
				mtype = mdam >= adam ? MFILE_MARROW : MFILE_PARROW;
			}
			missile[mi]._miDir = midir;
			SetMissAnim(mi, mtype);
			/*dam += fdam + ldam;
			int gfx = random_(8, dam);
			if (gfx >= dam - (fdam + ldam)) {
				if (gfx < dam - ldam) {
					SetMissAnim(mi, MFILE_FARROW);
				} else {
					SetMissAnim(mi, MFILE_LARROW);
				}
			}*/
		}
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	mis = &missile[mi];
	mis->_miAnimFrame = midir + 1; // only for normal arrows
	mis->_miRange = 256;
	if (misource != -1) {
		if (micaster == 0) {
			// mis->_miMinDam = plr[misource]._pIPcMinDam;
			// mis->_miMaxDam = plr[misource]._pIPcMaxDam;
			if (mis->_miType == MIS_PBARROW)
				mis->_miRange = 1 + 4;
			else if (mis->_miType == MIS_ASARROW) {
				if (!LineClearF(PosOkMissile1, sx, sy, dx, dy))
					return MIRES_FAIL_DELETE;
				mis->_miVar1 = dx;
				mis->_miVar2 = dy;
			}
		} else {
			mis->_miMinDam = monster[misource].mMinDamage << 6;
			mis->_miMaxDam = monster[misource].mMaxDamage << 6;
		}
	} else {
		mis->_miMinDam = currLvl._dLevel << 6;
		mis->_miMaxDam = currLvl._dLevel << (1 + 6);
	}
	return MIRES_DONE;
}

int AddArrowC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int numarrows = 1, mitype;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (micaster == 0 && plr[misource]._pIFlags & ISPL_MULT_ARROWS) {
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
		AddMissile(sx, sy, dx + xoff, dy + yoff, 0, mitype, micaster, misource, 0, 0, 0);
	}
	return MIRES_DELETE;
}

int AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int pn, nTries;

	assert((unsigned)misource < MAX_PLRS);
	if (micaster == 0 || (dx == 0 && dy == 0)) {
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
#ifdef HELLFIRE
			if (dx <= MAXDUNX && dx >= 0 && dy <= MAXDUNY && dy >= 0) ///BUGFIX: < MAXDUNX / < MAXDUNY
#endif
				pn = dPiece[dx][dy];
		} while ((nSolidTable[pn] | dObject[dx][dy] | dMonster[dx][dy] | dPlayer[dx][dy]) != 0);
	}

	mis = &missile[mi];
	mis->_miRange = 2;
	mis->_mix = dx;
	mis->_miy = dy;
	dPlayer[dx][dy] = -(misource + 1);
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int av, mindam, maxdam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (micaster == 0) {
		if (misource != -1) {
			av = 2 * spllvl + 16;
			if (av > 63)
				av = 63;
			if (missile[mi]._miType == MIS_FIREBOLT) {
				mindam = (plr[misource]._pMagic >> 3) + spllvl + 1;
				maxdam = mindam + 9;
			} else {
				//assert(missile[mi]._miType == MIS_HBOLT);
				mindam = (plr[misource]._pMagic >> 2) + spllvl;
				maxdam = mindam + 9;
			}
		} else {
			av = 16;
			mindam = currLvl._dLevel;
			maxdam = mindam + 2 * mindam - 1;
		}
	} else {
		av = 26;
		mindam = monster[misource].mMinDamage;
		maxdam = monster[misource].mMaxDamage;
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miMinDam = mindam << 6;
	mis->_miMaxDam = maxdam << 6;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_mitxoff += 3 * mis->_mixvel;
	mis->_mityoff += 3 * mis->_miyvel;
	GetMissilePos(mi);
	mis->_miRange = 256;
	mis->_miMinDam = monster[misource].mMinDamage << 6;
	mis->_miMaxDam = monster[misource].mMaxDamage << 6;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	return MIRES_DONE;
}

int AddKrull(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	missile[mi]._miRange = 256;
	missile[mi]._miMinDam = missile[mi]._miMaxDam = 4 << 6;
	//PutMissile(mi);
	return MIRES_DONE;
}

int AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	const char *cr;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
				if ((nSolidTable[dPiece[tx][ty]] | dMonster[tx][ty] | dObject[tx][ty] | dPlayer[tx][ty]) == 0) {
					mis->_mix = tx;
					mis->_miy = ty;
					mis->_misx = tx;
					mis->_misy = ty;
					dPlayer[tx][ty] = -(misource + 1);
					mis->_miRange = 2;
					return MIRES_DONE;
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

/*
 * Remark: expects damage to be shifted!
 */
int AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_miRange = 255;
	mis->_miMinDam >>= 2; // * 16 / 64
	mis->_miMaxDam >>= 2; // * 16 / 64
	mis->_miAnimFrame = RandRange(1, 8);
	return MIRES_DONE;
}

/**
 * Var1: animation helper
 * Var2: light strength
 */
int AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;

	mis = &missile[mi];
	mis->_miRange = 160 * (spllvl + 1);
	if (misource != -1) {
		assert((unsigned)misource < MAX_PLRS);
		p = &plr[misource];
		// TODO: add support for spell duration modifier
		// range += (p->_pISplDur * range) >> 7;
		mis->_miMinDam = ((p->_pMagic >> 3) + spllvl + 5) << (-3 + 6);
		mis->_miMaxDam = ((p->_pMagic >> 3) + spllvl * 2 + 10) << (-3 + 6);
	} else {
		mis->_miMinDam = (5 + currLvl._dLevel) << (-3 + 6);
		mis->_miMaxDam = (10 + currLvl._dLevel) << (-3 + 6);
	}
	mis->_miVar1 = mis->_miRange - mis->_miAnimLen;
	//mis->_miVar2 = 0;
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, mindam, maxdam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	if (micaster == 0) {
		mindam = (plr[misource]._pMagic >> 2) + 10;
		maxdam = mindam + 10;
		for (i = spllvl; i > 0; i--) {
			mindam += mindam >> 3;
			maxdam += maxdam >> 3;
		}
		i = 2 * spllvl + 16;
		if (i > 50)
			i = 50;
	} else {
		mindam = monster[misource].mMinDamage;
		maxdam = monster[misource].mMaxDamage;
		i = 16;
	}
	mis->_miMinDam = mis->_miMaxDam = RandRange(mindam, maxdam) << 6;
	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 */
int AddLightningC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int mindam, maxdam;

	if (misource != -1) {
		if (micaster == 0) {
			mindam = 1;
			maxdam = plr[misource]._pMagic + (spllvl << 3);
		} else {
			mindam = monster[misource].mMinDamage;
			maxdam = monster[misource].mMaxDamage;
		}
	} else {
		mindam = currLvl._dLevel;
		maxdam = mindam + currLvl._dLevel;
	}

	GetMissileVel(mi, sx, sy, dx, dy, 32);

	mis = &missile[mi];
	mis->_miMinDam = mindam;
	mis->_miMaxDam = maxdam;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miRange = 256;
	return MIRES_DONE;
}

int AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int range;

	mis = &missile[mi];
	mis->_miLid = AddLight(sx, sy, 4);
	if (midir >= 0) {
		mis->_mixoff = missile[midir]._mixoff;
		mis->_miyoff = missile[midir]._miyoff;
		mis->_mitxoff = missile[midir]._mitxoff;
		mis->_mityoff = missile[midir]._mityoff;
	}
	if (midir < 0 || misource == -1)
		range = 8;
	else if (micaster == 1)
		range = 10;
	else
		range = (spllvl >> 1) + 6;
	mis->_miRange = range;
	mis->_miMinDam <<= 3;
	mis->_miMaxDam <<= 3;
	mis->_miAnimFrame = RandRange(1, 8);
	return MIRES_DONE;
}

/**
 * Var1: light strength
 */
int AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis, *bmis;

	mis = &missile[mi];
	if (dx != -1) {
		bmis = &missile[dx];
		mis->_mix = bmis->_mix;
		mis->_miy = bmis->_miy;
		mis->_misx = bmis->_misx;
		mis->_misy = bmis->_misy;
		mis->_mixoff = bmis->_mixoff;
		mis->_miyoff = bmis->_miyoff;
		mis->_mitxoff = bmis->_mitxoff;
		mis->_mityoff = bmis->_mityoff;
	}
	mis->_mixvel = 0;
	mis->_miyvel = 0;
	mis->_miRange = mis->_miAnimLen;
	//mis->_miVar1 = 0;
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
 * Var1: animation
 * Var2: light strength
 */
int AddTown(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis, *bmis;
	int i, j, tx, ty, pn;
	const char *cr;

	mis = &missile[mi];
	if (currLvl._dType != DTYPE_TOWN) {
		mis->_miDelFlag = TRUE;
		for (i = 0; i < 6; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = (BYTE)*cr; j > 0; j--) {
				tx = dx + *++cr;
				ty = dy + *++cr;
				if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
					pn = dPiece[tx][ty];
					if ((dMissile[tx][ty] | nSolidTable[pn] | nMissileTable[pn] | dObject[tx][ty] | dPlayer[tx][ty]) == 0) {
						if (!CheckIfTrig(tx, ty)) {
							mis->_miDelFlag = FALSE;
							if (misource == myplr) {
								NetSendCmdLocBParam1(true, CMD_ACTIVATEPORTAL, tx, ty, currLvl._dLevelIdx);
							}
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
	}
	mis->_mix = mis->_misx = tx;
	mis->_miy = mis->_misy = ty;
	mis->_miRange = 100;
	if (spllvl >= 0) {
		PlaySfxLoc(LS_SENTINEL, tx, ty);
		mis->_miVar1 = mis->_miRange - mis->_miAnimLen;
		// mis->_miVar2 = 0;
	} else {
		// a portal recreated by AddWarpMissile
		mis->_miVar1 = mis->_miRange - 1;
	}
	for (i = 0; i < nummissiles; i++) {
		bmis = &missile[missileactive[i]];
		if (bmis->_miType == MIS_TOWN && bmis != mis && bmis->_miSource == misource)
			bmis->_miRange = 0;
	}
	PutMissile(mi);
	return MIRES_DONE;
}

int AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	AddMissile(sx, sy, 0, 0, 0, MIS_FLASH2, micaster, misource, 0, 0, spllvl);

	mis = &missile[mi];
	if (misource != -1) {
		if (micaster == 0) {
			dam = plr[misource]._pMagic >> 1;
			for (i = spllvl; i > 0; i--) {
				dam += dam >> 3;
			}
			mis->_miMinDam = dam;
			mis->_miMaxDam = dam << 3;
		} else {
			mis->_miMinDam = mis->_miMaxDam = monster[misource].mLevel << 1;
		}
	} else {
		mis->_miMinDam = mis->_miMaxDam = currLvl._dLevel << 1;
	}
	//assert(mis->_miAnimLen == misfiledata[MFILE_BLUEXFR].mfAnimLen[0]);
	mis->_miRange = 19;
	return MIRES_DONE;
}

int AddFlash2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	//assert(mis->_miAnimLen == misfiledata[MFILE_BLUEXBK].mfAnimLen[0]);
	mis->_miRange = 19;
	mis->_miPreFlag = TRUE;
	return MIRES_DONE;
}

int AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);

	if (misource == myplr) {
		if (plr[misource]._pManaShield == 0)
			NetSendCmdBParam1(true, CMD_SETSHIELD, spllvl);
		else
			NetSendCmd(true, CMD_REMSHIELD);
	}
	return MIRES_DELETE;
}

/**
 * Var1: animation
 * Var2: light strength
 * Var3: x coordinate of the missile-light
 * Var4: y coordinate of the missile-light
 */
int AddFireWave(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	assert((unsigned)misource < MAX_PLRS);
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_miMinDam = ((plr[misource]._pMagic >> 3) + spllvl + 1) << 6;
	mis->_miMaxDam = ((plr[misource]._pMagic >> 3) + 2 * spllvl + 2) << 6;
	mis->_miRange = 255;
	//mis->_miVar1 = 0;
	//mis->_miVar2 = 0;
	//mis->_miVar3 = 0;
	//mis->_miVar4 = 0;
	mis->_mix++;
	mis->_miy++;
	mis->_miyoff -= 32;
	return MIRES_DONE;
}

/**
 * Var1: animation
 * Var2: animation
 * Var3: light strength
 */
int AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, pn, j, tx, ty, range;
	const char *cr;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];

	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				if (LineClear(sx, sy, tx, ty)) {
					pn = dPiece[tx][ty];
					if ((dMonster[tx][ty] | nSolidTable[pn] | nMissileTable[pn] | dObject[tx][ty] | dMissile[tx][ty]) == 0) {
						mis->_mix = tx;
						mis->_miy = ty;
						mis->_misx = tx;
						mis->_misy = ty;
						mis->_miLid = AddLight(tx, ty, 1);

						range = spllvl + (plr[misource]._pLevel >> 1);
						// TODO: add support for spell duration modifier
						//range += (range * plr[misource]._pISplDur) >> 7;
						if (range > 30)
							range = 30;
						range <<= 4;
						if (range < 30)
							range = 30;

						mis->_miRange = range;
						mis->_miVar1 = range - mis->_miAnimLen;
						//mis->_miVar2 = 0;
						mis->_miVar3 = 1;
						return MIRES_DONE;
					}
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var1: remaining jumps
 */
int AddChain(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	assert((unsigned)misource < MAX_PLRS);

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 32);

	mis = &missile[mi];
	mis->_miLid = AddLight(sx, sy, 4);
	//assert(mis->_miAnimLen == misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miAnimFrame = random_(0, misfiledata[MFILE_LGHNING].mfAnimLen[0]);
	mis->_miVar1 = 1 + (spllvl >> 1);
	//if (misource != -1) {
	//	if (micaster == 0) {
			mis->_miMinDam = 1 << 6;
			mis->_miMaxDam = plr[misource]._pMagic << 6;
	//	} else {
	//		mindam = 1 << 6;
	//		maxdam = monster[misource].mMaxDamage << 6;
	//	}
	//} else {
	//	mindam = 1 << 6;
	//	maxdam = currLvl._dLevel << (1 + 6);
	//}
	mis->_miRange = 256;
	return MIRES_DONE;
}

int AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	AnimStruct *anim;

	GetMissileVel(mi, sx, sy, dx, dy, 18);
	mon = &monster[misource];
	anim = &mon->_mAnims[
		(mon->_mType >= MT_HORNED && mon->_mType <= MT_OBLORD) ? MA_SPECIAL :
		(mon->_mType >= MT_NSNAKE && mon->_mType <= MT_GSNAKE) ? MA_ATTACK : MA_WALK];
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miAnimFlags = 0;
	mis->_miAnimData = anim->Data[midir];
	mis->_miAnimDelay = anim->Rate;
	mis->_miAnimLen = anim->Frames;
	mis->_miAnimWidth = mon->_mAnimWidth;
	mis->_miAnimWidth2 = mon->_mAnimWidth2;
	mis->_miAnimAdd = 1;
	if (mon->_mType >= MT_NSNAKE && mon->_mType <= MT_GSNAKE)
		mis->_miAnimFrame = 7;
	mis->_miLightFlag = TRUE;
	if (mon->_uniqtype != 0) {
		mis->_miUniqTrans = mon->_uniqtrans + 1;
		mis->_miLid = mon->mlid;
	}
	// mis->_miRange = 256;
	//PutMissile(mi);
	return MIRES_DONE;
}

/**
 * Var1: target found
 * Var2: attempts to find a target
 */
int AddFireman(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	AnimStruct *anim;
	MonsterStruct *mon;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mon = &monster[misource];
	anim = &mon->_mAnims[MA_WALK];
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miAnimFlags = 0;
	mis->_miAnimData = anim->Data[midir];
	mis->_miAnimDelay = anim->Rate;
	mis->_miAnimLen = anim->Frames;
	mis->_miAnimWidth = mon->_mAnimWidth;
	mis->_miAnimWidth2 = mon->_mAnimWidth2;
	mis->_miAnimAdd = 1;
	//mis->_miVar1 = FALSE;
	//mis->_miVar2 = 0;
	mis->_miLightFlag = TRUE;
	if (mon->_uniqtype != 0)
		mis->_miUniqTrans = mon->_uniqtrans + 1;
	dMonster[mon->_mx][mon->_my] = 0;
	//mis->_miRange = 256;
	//PutMissile(mi);
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 16);
#ifdef HELLFIRE
	if (misfiledata[missile[mi]._miAnimType].mfAnimFAmt == 16) {
		SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	}
#endif
	mis = &missile[mi];
	mis->_miRange = 256;
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	if (micaster == 0) {
		if (!plr[misource]._pInvincible)
			PlrDecHp(misource, 320, 0);
		mis->_miMinDam = mis->_miMaxDam = (plr[misource]._pMagic * (spllvl + 1)) << (-3 + 6);
	} else {
		mis->_miMinDam = monster[misource].mMinDamage << 6;
		mis->_miMaxDam = monster[misource].mMaxDamage << 6;
	}
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
int AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miRange = 5 * (monster[misource]._mint + 4);
	mis->_miMinDam = monster[misource].mMinDamage << 6;
	mis->_miMaxDam = monster[misource].mMaxDamage << 6;
	//mis->_miLid = -1;
	//PutMissile(mi);
	return MIRES_DONE;
}

int AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange = 40 * (monster[misource]._mint + 1) + random_(50, 15);
	mis->_miLightFlag = TRUE;
	mis->_miPreFlag = TRUE;
	return MIRES_DONE;
}

/**
 * Var1: mmode of the monster
 * Var2: mnum of the monster
 */
int AddStone(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	int i, j, tx, ty, mid, range;
	const char *cr;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				mid = dMonster[tx][ty];
				mid = mid >= 0 ? mid - 1 : -1 - mid;
				if (mid < MAX_MINIONS)
					continue;
				mon = &monster[mid];
				if (mon->_mAi != AI_DIABLO) {
#ifdef HELLFIRE
					if (mon->_mType == MT_NAKRUL)
						continue;
#endif
					if (mon->_mmode != MM_FADEIN && mon->_mmode != MM_FADEOUT && mon->_mmode != MM_CHARGE && mon->_mmode != MM_STONE && mon->_mhitpoints >= (1 << 6)) {
						mis->_miVar1 = mon->_mmode;
						mis->_miVar2 = mid;
						mon->_mmode = MM_STONE;
						mis->_mix = tx;
						mis->_miy = ty;
						mis->_misx = mis->_mix;
						mis->_misy = mis->_miy;

						range = spllvl + 6;
						// TODO: add support for spell duration modifier
						//range += (range * plr[misource]._pISplDur) >> 7;
						if (range > 15)
							range = 15;
						range <<= 4;

						mis->_miRange = range;
						return MIRES_DONE;
					}
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

/**
 * Var4: x coordinate of the destination
 * Var5: y coordinate of the destination
 */
int AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis, *tmis;
	int i;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	for (i = 0; i < nummissiles; i++) {
		tmis = &missile[missileactive[i]];
		if (tmis->_miType == MIS_GOLEM && tmis != mis && tmis->_miSource == misource) {
			return MIRES_FAIL_DELETE;
		}
	}
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	if (!(MINION_NR_INACTIVE(misource)) && misource == myplr)
		MonStartKill(misource, misource);
	return MIRES_DONE;
}

/**
 * Var1: target hit
 */
int AddApocaExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange = mis->_miAnimLen;
	mis->_miMinDam = mis->_miMaxDam = 40 << 6; // assert(misource == DIABLO);
	//mis->_miVar1 = FALSE;
	return MIRES_DONE;
}

int AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int i, hp;

	assert((unsigned)misource < MAX_PLRS);

	hp = RandRange(1, 10);
	for (i = spllvl; i > 0; i--) {
		hp += RandRange(1, 6);
	}
	for (i = plr[misource]._pLevel; i > 0; i--) {
		hp += RandRange(1, 4);
	}
	hp <<= 6;

	switch (plr[misource]._pClass) {
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

int AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int pnum, i, hp;

	assert((unsigned)misource < MAX_PLRS);

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (pnum != misource
		 && plr[pnum]._px == dx && plr[pnum]._py == dy
		 && plr[pnum]._pHitPoints >= (1 << 6)) {
			hp = RandRange(1, 10);
			for (i = spllvl; i > 0; i--) {
				hp += RandRange(1, 6);
			}
			for (i = plr[misource]._pLevel; i > 0; i--) {
				hp += RandRange(1, 4);
			}
			hp <<= 6;

			switch (plr[misource]._pClass) {
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
int AddElement(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int i, mindam, maxdam;

	assert((unsigned)misource < MAX_PLRS);
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection8(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	//mis->_miVar3 = FALSE;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	mis->_miLid = AddLight(sx, sy, 8);

	p = &plr[misource];
	mindam = (p->_pMagic >> 3) + 2 * spllvl + 4;
	maxdam = (p->_pMagic >> 3) + 4 * spllvl + 20;
	for (i = spllvl; i > 0; i--) {
		mindam += mindam >> 3;
		maxdam += maxdam >> 3;
	}
	mis->_miMinDam = mis->_miMaxDam = RandRange(mindam, maxdam) << 6;
	mis->_miRange = 256;
	return MIRES_DONE;
}

int AddIdentify(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);
	CheckIdentify(misource, spllvl);
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
int AddWallC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	const char *cr;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
			tx = dx + *++cr;
			ty = dy + *++cr;
			if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
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
						mis->_miRange = 1 + (spllvl >> 1);
						return MIRES_DONE;
					}
				}
			}
		}
	}
	return MIRES_FAIL_DELETE;
}

int AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, range;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	range = 1584;
	for (i = spllvl; i > 0; i--) {
		range += range >> 3;
	}
	// TODO: add support for spell duration modifier
	//range += range * plr[misource]._pISplDur >> 7;
	mis->_miRange = range;
	return MIRES_DONE;
}

/**
 * Var1: direction of the wave
 */
int AddFireWaveC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	mis->_miVar1 = GetDirection8(sx, sy, dx, dy);
	// mis->_miRange = 1;
	return MIRES_DONE;
}

int AddLightNovaC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miMinDam = (1 << 6);
	if (misource != -1) {
		mis->_miMaxDam = ((plr[misource]._pMagic >> 1) + (spllvl << 4)) << 6;
	} else {
		mis->_miMaxDam = (6 + currLvl._dLevel) << 6;
	}
	// mis->_miRange = 1;
	return MIRES_DONE;
}

#ifdef HELLFIRE
int AddFireNovaC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, mindam, maxdam;

	mis = &missile[mi];
	mindam = 1 + (plr[misource]._pMagic >> 3);
	maxdam = mindam + 4;
	for (i = spllvl; i > 0; i--) {
		mindam += mindam >> 3;
		maxdam += maxdam >> 3;
	}
	mis->_miMaxDam = mis->_miMinDam = RandRange(mindam, maxdam) << 6;
	// mis->_miRange = 1;
	return MIRES_DONE;
}

/**
 * Var1: length of the effect
 * Var2: hp penalty
 */
int AddBloodboil(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int lvl;

	assert((unsigned)misource < MAX_PLRS);
	mis = &missile[mi];
	p = &plr[misource];
	if (p->_pSpellFlags & (PSE_BLOOD_BOIL | PSE_LETHARGY) || p->_pHitPoints <= p->_pLevel << 6) {
		return MIRES_FAIL_DELETE;
	} else {
		PlaySfxLoc(sgSFXSets[SFXS_PLR_70][p->_pClass], p->_px, p->_py);
		p->_pSpellFlags |= PSE_BLOOD_BOIL;
		lvl = p->_pLevel;
		mis->_miVar2 = (3 * lvl) << 7;
		if (misource <= 0)
			lvl = 1;
		lvl *= 2;
		mis->_miVar1 = mis->_miRange = lvl + 10 * spllvl + 245;
		CalcPlrItemVals(misource, true);
		return MIRES_DONE;
	}
}
#endif

int AddRepair(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);
	DoRepair(misource, spllvl);
	return MIRES_DELETE;
}

int AddRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);
	DoRecharge(misource, spllvl);
	return MIRES_DELETE;
}

#ifdef HELLFIRE
int AddWhittle(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);
	DoWhittle(misource, spllvl);
	return MIRES_DELETE;
}
#endif

int AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);

	if (misource == myplr) {
		NewCursor(CURSOR_DISARM);
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
		if (sgbControllerActive) {
			TryIconCurs(false);
		}
#endif
	}
	return MIRES_DELETE;
}

/**
 * Var2: animation timer
 */
int AddFlame(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MissileStruct *bmis;
	int i;

	mis = &missile[mi];
	mis->_miLid = AddLight(sx, sy, 1);
	bmis = &missile[midir];
	mis->_misx = bmis->_misx;
	mis->_misy = bmis->_misy;
	mis->_mixoff = bmis->_mixoff;
	mis->_miyoff = bmis->_miyoff;
	mis->_mitxoff = bmis->_mitxoff;
	mis->_mityoff = bmis->_mityoff;
	//mis->_miVar2 = 0;
	for (i = bmis->_miVar3; i > 0; i--) {
		mis->_miVar2 += 5;
	}
	mis->_miRange = mis->_miVar2 + 20;
	if (micaster == 0) {
		mis->_miMinDam = plr[misource]._pMagic;
		mis->_miMaxDam = mis->_miMinDam + (spllvl << 4);
	} else {
		mis->_miMinDam = monster[misource].mMinDamage << 1;
		mis->_miMaxDam = monster[misource].mMaxDamage << 1;
	}
	return MIRES_DONE;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 * Var3: timer to dissipate
 */
int AddFlameC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 32);
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	//mis->_miVar3 = 0;
	mis->_miRange = 256;
	return MIRES_DONE;
}

int AddFireTrap(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miMinDam = (2 + currLvl._dLevel) << (6 - 2);
	mis->_miMaxDam = mis->_miMinDam * 2;
	mis->_miRange = 9;
	return MIRES_DONE;
}

int AddBarrelExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miMinDam = 8 << 6;
	mis->_miMaxDam = 16 << 6;
	
	CheckMissileCol(mi, sx, sy, true);
	return MIRES_DELETE;
}

int AddCboltC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int i = 3;

	if (misource != -1) {
		if (micaster == 0) {
			i += (spllvl >> 1);
		}
	}

	while (i-- != 0) {
		AddMissile(sx, sy, dx, dy, midir, MIS_CBOLT, micaster, misource, 0, 0, spllvl);
	}
	return MIRES_DELETE;
}

/**
 * Var1: light strength
 * Var2: base direction
 * Var3: movement counter
 * Var4: rnd direction
 */
int AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 8);

	mis = &missile[mi];
	mis->_miLid = AddLight(sx, sy, 5);
	mis->_miVar1 = 5;
	mis->_miVar2 = midir;
	//mis->_miVar3 = 0;
	mis->_miVar4 = RandRange(1, 15);
	if (micaster == 0) {
		mis->_miMinDam = 1;
		mis->_miMaxDam = (plr[misource]._pMagic << (-2 + 6)) + (spllvl << (2 + 6));
	} else {
		mis->_miMinDam = mis->_miMaxDam = 15 << 6;
	}
	mis->_miRange = 256;
	mis->_miAnimFrame = RandRange(1, 8);
	return MIRES_DONE;
}

int AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *tp;
	int tnum = spllvl;

	assert((unsigned)misource < MAX_PLRS);

	if ((unsigned)tnum >= MAX_PLRS)
		return MIRES_DELETE;

	tp = &plr[tnum];
	if (tp->_pHitPoints >= (1 << 6))
		return MIRES_DELETE;

	if (tnum == myplr) {
		gbDeathflag = false;
		gamemenu_off();
	}

	ClrPlrPath(tnum);
	tp->destAction = ACTION_NONE;
	tp->_pInvincible = FALSE;

	PlrSetHp(tnum, std::min(10 << 6, tp->_pMaxHPBase));
	PlrSetMana(tnum, 0);

	CalcPlrInv(tnum, true);

	if (tp->plrlevel == currLvl._dLevelIdx) {
		PlacePlayer(tnum);
		PlrStartStand(tnum, tp->_pdir);
	} else {
		tp->_pmode = PM_STAND;
	}

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	mis->_misx = mis->_mix;
	mis->_misy = mis->_miy;
	mis->_miRange = misfiledata[MFILE_RESSUR1].mfAnimLen[0];
	return MIRES_DONE;
}

int AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	assert((unsigned)misource < MAX_PLRS);

	if (misource == myplr)
		NewCursor(CURSOR_TELEKINESIS);
	return MIRES_DELETE;
}

/**
 * Var1: animation
 * Var2: light strength
 */
int AddRportal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange = 100;
	mis->_miVar1 = 100 - mis->_miAnimLen;
	//mis->_miVar2 = 0;
	PutMissile(mi);
	return MIRES_DONE;
}

int AddApocaC2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	PlayerStruct *p;
	int pnum;

	for (pnum = 0; pnum < gbMaxPlayers; pnum++) {
		p = &plr[pnum];
		if (p->plractive) {
			if (LineClear(sx, sy, p->_pfutx, p->_pfuty)) {
				AddMissile(p->_pfutx, p->_pfuty, 0, 0, 0, MIS_EXAPOCA2, micaster, misource, 0, 0, 0);
			}
		}
	}
	return MIRES_DELETE;
}

int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, char micaster, int misource, int mindam, int maxdam, int spllvl)
{
	MissileStruct *mis;
	const MissileData *mds;
	int idx, mi, res;

	idx = nummissiles;
#ifdef HELLFIRE
	if (idx >= MAXMISSILES - 1)
#else
	if (idx >= MAXMISSILES)
#endif
		return -1;

	mi = missileavail[0];

	missileavail[0] = missileavail[MAXMISSILES - idx - 1];
	missileactive[idx] = mi;
	nummissiles++;

	mis = &missile[mi];
	memset(mis, 0, sizeof(*mis));

	mis->_miCaster = micaster;
	mis->_miSource = misource;
	mis->_miSpllvl = spllvl;
	mis->_miDir = midir;
	mis->_mix = sx;
	mis->_miy = sy;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_miMinDam = mindam;
	mis->_miMaxDam = maxdam;
	mis->_miType = mitype;
	mds = &missiledata[mitype];
	mis->_miSubType = mds->mType;
	mis->_miFlags = mds->mdFlags;
	mis->_miResist = mds->mResist;
	mis->_miAnimType = mds->mFileNum;
	mis->_miDrawFlag = mds->mDraw;

	if (misfiledata[mis->_miAnimType].mfAnimFAmt < 8)
		SetMissDir(mi, 0);
	else
		SetMissDir(mi, midir);

	mis->_miAnimAdd = 1;
	mis->_miLid = -1;

	if (mds->mlSFX != -1) {
		PlaySfxLoc(mds->mlSFX, mis->_misx, mis->_misy, mds->mlSFXCnt);
	}

	res = mds->mAddProc(mi, sx, sy, dx, dy, midir, micaster, misource, spllvl);
	if (res != MIRES_DONE) {
		assert(res == MIRES_FAIL_DELETE || res == MIRES_DELETE);
		missileavail[MAXMISSILES - nummissiles] = mi;
		nummissiles--;
		if (/* nummissiles > 0 && */idx != nummissiles)
			missileactive[idx] = missileactive[nummissiles];

		if (res == MIRES_FAIL_DELETE)
			mi = -1;
	}
	return mi;
}

static bool Sentfire(int mi, int sx, int sy)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (LineClear(mis->_mix, mis->_miy, sx, sy)) {
		if (dMonster[sx][sy] > 0 && monster[dMonster[sx][sy] - 1]._mhitpoints >= (1 << 6) && dMonster[sx][sy] - 1 >= MAX_MINIONS) {
			AddMissile(mis->_mix, mis->_miy, sx, sy, 0, MIS_FIREBOLT, mis->_miCaster, mis->_miSource, 0, 0, mis->_miSpllvl);
			SetMissDir(mi, 2);
			mis->_miVar2 = 3;
			return true;
		}
	}

	return false;
}

void MI_Dummy(int mi)
{
	return;
}

void MI_Golem(int mi)
{
	MissileStruct *mis;
	int tx, ty, i, j, src;
	const char *cr;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	src = mis->_miSource;
	if (MINION_NR_INACTIVE(src)) {
		for (i = 0; i < 6; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = (BYTE)*cr; j > 0; j--) {
				tx = mis->_miVar4 + *++cr;
				ty = mis->_miVar5 + *++cr;
				if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
					if (LineClear(mis->_misx, mis->_misy, tx, ty)) {
						if ((dMonster[tx][ty] | nSolidTable[dPiece[tx][ty]] | dObject[tx][ty] | dPlayer[tx][ty]) == 0) {
							SpawnGolum(src, tx, ty, mis->_miSpllvl);
							return;
						}
					}
				}
			}
		}
	}
}

void MI_Arrow(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	mis->_miDist++;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, false);
	}
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_AsArrow(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miDist++;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix == mis->_miVar1 && mis->_miy == mis->_miVar2) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, false);
		mis->_miDelFlag = TRUE;
		return;
	}
	PutMissile(mi);
}

void MI_Firebolt(int mi)
{
	MissileStruct *mis;
	//int omx, omy;
	int xptype;

	mis = &missile[mi];
	mis->_miRange--;
	//omx = mis->_mitxoff;
	//omy = mis->_mityoff;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		CheckMissileCol(mi, mis->_mix, mis->_miy, false);
	}
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		//mis->_mitxoff = omx;
		//mis->_mityoff = omy;
		//GetMissilePos(mi);
		switch (mis->_miType) {
		case MIS_FIREBOLT:
		case MIS_MAGMABALL:
			xptype = MIS_EXFIRE;
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
		case MIS_LICH:
			xptype = MIS_EXORA1;
			break;
		case MIS_PSYCHORB:
			xptype = MIS_EXBL2;
			break;
		case MIS_NECROMORB:
			xptype = MIS_EXRED3;
			break;
		case MIS_ARCHLICH:
			xptype = MIS_EXYEL2;
			break;
		case MIS_BONEDEMON:
			xptype = MIS_EXBL3;
			break;
#endif
		default:
			ASSUME_UNREACHABLE
			break;
		}
		AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, xptype, mis->_miCaster, mis->_miSource, 0, 0, 0);

		AddUnLight(mis->_miLid);
	} else {
		if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
			mis->_miVar1 = mis->_mix;
			mis->_miVar2 = mis->_miy;
			ChangeLight(mis->_miLid, mis->_miVar1, mis->_miVar2, 8);
		}
	}
	PutMissile(mi);
}

void MI_Lightball(int mi)
{
	MissileStruct *mis;
	int range;

	mis = &missile[mi];
	mis->_miRange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		range = mis->_miRange;
		if (CheckMissileCol(mi, mis->_mix, mis->_miy, false))
			mis->_miRange = range;
	}
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Krull(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	CheckMissileCol(mi, mis->_mix, mis->_miy, false);
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Acidpud(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, true);
	mis->_miRange--;
	if (mis->_miRange == 0) {
		if (mis->_miDir != 0) {
			mis->_miDelFlag = TRUE;
		} else {
			SetMissDir(mi, 1);
			mis->_miRange = mis->_miAnimLen;
		}
	}
	PutMissile(mi);
}

void MI_Firewall(int mi)
{
	MissileStruct *mis;
	int ExpLight[] = { 2, 3, 4, 5, 5, 6, 7, 8, 9, 10, 11, 12, 12 };

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == mis->_miVar1) {
		SetMissDir(mi, 1);
		//assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
		mis->_miAnimFrame = random_(0, misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
	}
	if (mis->_miRange == mis->_miAnimLen - 1) {
		SetMissDir(mi, 0);
		//assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[0]);
		mis->_miAnimFrame = misfiledata[MFILE_FIREWAL].mfAnimLen[0];
		mis->_miAnimAdd = -1;
	}
	CheckMissileCol(mi, mis->_mix, mis->_miy, true);
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	} else if (mis->_miDir == 0) {
		if (mis->_miVar2 == 0) {
			//assert(mis->_miLid == -1);
			mis->_miLid = AddLight(mis->_mix, mis->_miy, ExpLight[0]);
		} else {
			//assert(mis->_miVar2 < sizeof(ExpLight));
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		}
		mis->_miVar2 += mis->_miAnimAdd;
	}
	PutMissile(mi);
}

void MI_Fireball(int mi)
{
	MissileStruct *mis;
	int mx, my;

	mis = &missile[mi];
	mis->_miRange--;

	if (mis->_miAnimType == MFILE_BIGEXP) {
		if (mis->_miRange == 0) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_miLid);
		}
	} else {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		if (mis->_mix != mis->_misx || mis->_miy != mis->_misy)
			CheckMissileCol(mi, mis->_mix, mis->_miy, false);
		mx = mis->_mix;
		my = mis->_miy;
		if (mis->_miRange == 0) {
			ChangeLight(mis->_miLid, mx, my, mis->_miAnimFrame);
			//CheckMissileCol(mi, mx, my, true);
			// TODO: mis->_miMinDam >>= 1; mis->_miMaxDam >>= 1; ?
			CheckMissileCol(mi, mx, my + 1, true);
			CheckMissileCol(mi, mx, my - 1, true);
			CheckMissileCol(mi, mx + 1, my, true);
			CheckMissileCol(mi, mx + 1, my - 1, true);
			CheckMissileCol(mi, mx + 1, my + 1, true);
			CheckMissileCol(mi, mx - 1, my, true);
			CheckMissileCol(mi, mx - 1, my + 1, true);
			CheckMissileCol(mi, mx - 1, my - 1, true);
			if (!TransList[dTransVal[mx][my]]
			    || (mis->_mixvel < 0 && ((TransList[dTransVal[mx][my + 1]] && nSolidTable[dPiece[mx][my + 1]]) || (TransList[dTransVal[mx][my - 1]] && nSolidTable[dPiece[mx][my - 1]])))) {
				mis->_mix++;
				mis->_miy++;
				mis->_miyoff -= 32;
			}
			if (mis->_miyvel > 0
			 && ((TransList[dTransVal[mx + 1][my]] && nSolidTable[dPiece[mx + 1][my]])
			  || (TransList[dTransVal[mx - 1][my]] && nSolidTable[dPiece[mx - 1][my]]))) {
				mis->_miyoff -= 32;
			}
			if (mis->_mixvel > 0
			 && ((TransList[dTransVal[mx][my + 1]] && nSolidTable[dPiece[mx][my + 1]])
			  || (TransList[dTransVal[mx][my - 1]] && nSolidTable[dPiece[mx][my - 1]]))) {
				mis->_mixoff -= 32;
			}
			mis->_miDir = 0;
			SetMissAnim(mi, MFILE_BIGEXP);
			mis->_miRange = mis->_miAnimLen - 1;
		} else if (mx != mis->_miVar1 || my != mis->_miVar2) {
			mis->_miVar1 = mx;
			mis->_miVar2 = my;
			ChangeLight(mis->_miLid, mx, my, 8);
		}
	}

	PutMissile(mi);
}

#ifdef HELLFIRE
void MI_HorkSpawn(int mi)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	const char *cr;

	mis = &missile[mi];
	mis->_miRange--;
	CheckMissileCol(mi, mis->_mix, mis->_miy, false);
	if (mis->_miRange <= 0) {
		mis->_miDelFlag = TRUE;
		for (i = 0; i < 2; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = *cr; j > 0; j--) {
				tx = mis->_mix + *++cr;
				ty = mis->_miy + *++cr;
				if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
					if ((nSolidTable[dPiece[tx][ty]] | dMonster[tx][ty] | dPlayer[tx][ty] | dObject[tx][ty]) == 0) {
						i = 6;
						int mon = AddMonster(tx, ty, mis->_miVar1, 1, true);
						MonStartStand(mon, mis->_miVar1);
						break;
					}
				}
			}
		}
	} else {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
	}
	PutMissile(mi);
}

void MI_Rune(int mi)
{
	MissileStruct *mis;
	int j, tx, ty;
	const char *cr;

	mis = &missile[mi];
	if (--mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		return;
	}
	if (mis->_miRange <= 1584) { // ttl of the rune
		cr = &CrawlTable[CrawlNum[mis->_miVar2]];
		for (j = *cr; j > 0; j--) {
			tx = mis->_mix + *++cr;
			ty = mis->_miy + *++cr;
			if ((dMonster[tx][ty] | dPlayer[tx][ty]) != 0) {
				mis->_miDelFlag = TRUE;
				AddUnLight(mis->_miLid);
				AddMissile(mis->_mix, mis->_miy, tx, ty, 0, mis->_miVar1, mis->_miCaster, mis->_miSource, 0, 0, mis->_miSpllvl);
				return;
			}
		}
	}
	PutMissile(mi);
}

void MI_Lightwall(int mi)
{
	MissileStruct *mis;
	int range;

	mis = &missile[mi];
	mis->_miRange--;
	range = mis->_miRange;
	if (CheckMissileCol(mi, mis->_mix, mis->_miy, false))
		mis->_miRange = range;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Hiveexp(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange <= 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_RingC(int mi)
{
	MissileStruct *mis;
	int sx, sy, tx, ty, j, pn, mitype, caster, source, spllvl;
	const char *cr;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	mitype = mis->_miType == MIS_FIRERING ? MIS_FIREWALL : MIS_LIGHTWALL;
	caster = mis->_miCaster;
	source = mis->_miSource;
	spllvl = mis->_miSpllvl;
	sx = mis->_mix;
	sy = mis->_miy;

	cr = &CrawlTable[CrawlNum[3]];
	for (j = *cr; j > 0; j--) {
		tx = sx + *++cr;
		ty = sy + *++cr;
		if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
			pn = dPiece[tx][ty];
			if ((nSolidTable[pn] | dObject[tx][ty]) == 0) {
				if (LineClear(sx, sy, tx, ty)) {
					if (nMissileTable[pn])
						break;
					else
						AddMissile(tx, ty, 0, 0, 0, mitype, caster, source, 0, 0, spllvl);
				}
			}
		}
	}
}

#endif

void MI_NovaC(int mi)
{
	MissileStruct *mis;
	int i, sx, sy, tx, ty, mitype, caster, source, mindam, maxdam, spllvl;
	const char *cr;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
#ifdef HELLFIRE
	mitype = mis->_miType == MIS_FIRENOVAC ? MIS_FIREBALL2 : MIS_LIGHTBALL;
#else
	mitype = MIS_LIGHTBALL;
#endif
	caster = mis->_miCaster;
	source = mis->_miSource;
	mindam = mis->_miMinDam;
	maxdam = mis->_miMaxDam;
	spllvl = mis->_miSpllvl;
	sx = mis->_mix;
	sy = mis->_miy;

	cr = &CrawlTable[CrawlNum[4]];
	for (i = *cr; i > 0; i--) {
		tx = sx + *++cr;
		ty = sy + *++cr;
		AddMissile(sx, sy, tx, ty, 0, mitype, caster, source, mindam, maxdam, spllvl);
	}
}

void MI_LightningC(int mi)
{
	MissileStruct *mis;
	int mx, my;

	mis = &missile[mi];
	mis->_miRange--;

	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);

	mx = mis->_mix;
	my = mis->_miy;
	assert(IN_DUNGEON_AREA(mx, my));
	if (!nMissileTable[dPiece[mx][my]]) {
		if (mx != mis->_miVar1 || my != mis->_miVar2) {
			AddMissile(
			    mx,
			    my,
			    0,
			    0,
			    mi,
			    mis->_miType == MIS_LIGHTNINGC ? MIS_LIGHTNING : MIS_LIGHTNING2,
			    mis->_miCaster,
			    mis->_miSource,
				mis->_miMinDam,
			    mis->_miMaxDam,
			    mis->_miSpllvl);
			mis->_miVar1 = mx;
			mis->_miVar2 = my;
		}
	} else {
		if (mis->_miSource != -1 || mx != mis->_misx || my != mis->_misy)
			mis->_miRange = 0;
	}
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
	}
}

void MI_Lightning(int mi)
{
	MissileStruct *mis;
	int range;

	mis = &missile[mi];
	mis->_miRange--;
	range = mis->_miRange;
	if (CheckMissileCol(mi, mis->_mix, mis->_miy, false))
		mis->_miRange = range;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_Town(int mi)
{
	MissileStruct *mis;
	int ExpLight[17] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15 };
	PlayerStruct *p;

	mis = &missile[mi];
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	if (mis->_miRange > 1)
		mis->_miRange--;
	if (mis->_miRange == mis->_miVar1) {
		SetMissDir(mi, 1);
		if (currLvl._dType != DLV_TOWN && mis->_miLid == -1)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, 15);
	} else if (mis->_miDir != 1 && currLvl._dType != DLV_TOWN) {
		if (mis->_miVar2 == 0)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, 1);
		else
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}

	p = &plr[myplr];
	if (p->_px == mis->_mix && p->_py == mis->_miy && !p->_pLvlChanging && p->_pmode == PM_STAND) {
		NetSendCmdParam1(true, CMD_WARP, mis->_miSource);
	}

	PutMissile(mi);
}

void MI_Flash(int mi)
{
	MissileStruct *mis;
	int mx, my;

	mis = &missile[mi];
	mx = mis->_mix;
	my = mis->_miy;
	CheckMissileCol(mi, mx - 1, my, true);
	CheckMissileCol(mi, mx + 1, my, true);
	CheckMissileCol(mi, mx - 1, my + 1, true);
	CheckMissileCol(mi, mx, my + 1, true);
	CheckMissileCol(mi, mx + 1, my + 1, true);
	// Flash2
	CheckMissileCol(mi, mx - 1, my - 1, true);
	CheckMissileCol(mi, mx, my - 1, true);
	CheckMissileCol(mi, mx + 1, my - 1, true);
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
	}
	PutMissile(mi);
}

void MI_Flash2(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
	}
	PutMissile(mi);
}

void MI_FireWave(int mi)
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
		//assert(mis->_miAnimLen == misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
		mis->_miAnimFrame = random_(0, misfiledata[MFILE_FIREWAL].mfAnimLen[1]);
	}
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	range = mis->_miRange;
	if (CheckMissileCol(mi, mis->_mix, mis->_miy, false))
		mis->_miRange = range;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	if (mis->_miDir != 0) {
		if (mis->_mix != mis->_miVar3 || mis->_miy != mis->_miVar4) {
			mis->_miVar3 = mis->_mix;
			mis->_miVar4 = mis->_miy;
			ChangeLight(mis->_miLid, mis->_miVar3, mis->_miVar4, 8);
		}
	} else {
		if (mis->_miVar2 == 0)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, ExpLight[0]);
		else
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
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
	int i, j, tx, ty;
	bool ex;
	const char *cr;

	mis = &missile[mi];
	mis->_miRange--;

	if (mis->_miVar2 > 0) {
		mis->_miVar2--;
	}
	if (mis->_miRange == mis->_miVar1 || (mis->_miDir == 2 && mis->_miVar2 == 0)) {
		SetMissDir(mi, 1);
	}

	if (!(mis->_miRange % 16)) {
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
	}

	if (mis->_miRange == 14) {
		SetMissDir(mi, 0);
		mis->_miAnimFrame = 15;
		mis->_miAnimAdd = -1;
	}

	mis->_miVar3 += mis->_miAnimAdd;

	if (mis->_miVar3 > 15) {
		mis->_miVar3 = 15;
	} else if (mis->_miVar3 > 0) {
		ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miVar3);
	}

	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}

	PutMissile(mi);
}

void MI_Chain(int mi)
{
	MissileStruct *mis;
	int mx, my, range, sd, dx, dy;

	mis = &missile[mi];
	mis->_miRange--;

	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);

	mx = mis->_mix;
	my = mis->_miy;
	if (mx != mis->_misx || my != mis->_misy) {
		if (!nMissileTable[dPiece[mx][my]]) {
			range = mis->_miRange;
			if (CheckMissileCol(mi, mis->_mix, mis->_miy, false)) {
				if (mis->_miVar1-- != 0) {
					// set the new position as the starting point
					mis->_misx = mx;
					mis->_misy = my;
					mis->_mixoff = 0;
					mis->_miyoff = 0;
					mis->_mitxoff = 0;
					mis->_mityoff = 0;
					// restore base range
					mis->_miRange = 256;
					// find new target
					if (!FindClosestChain(mx, my, dx, dy)) {
						sd = random_(0, lengthof(offset_x));
						dx = mx + offset_x[sd];
						dy = my + offset_y[sd];
					}
					//SetMissDir(mi, sd);
					GetMissileVel(mi, mx, my, dx, dy, 32);
				}
			}
		} else {
			mis->_miRange = 0;
		}
	}
	if (mis->_miRange != 0)
		PutMissile(mi);
	else {
		AddUnLight(mis->_miLid);
		mis->_miDelFlag = TRUE;
	}
}

void MI_Misexp(int mi)
{
	MissileStruct *mis;
	int ExpLight[] = { 9, 10, 11, 12, 11, 10, 8, 6, 4, 2, 1, 0, 0, 0, 0 };

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	} else {
		if (mis->_miVar1 == 0)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, 9);
		else
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, ExpLight[mis->_miVar1]);
		mis->_miVar1++;
		PutMissile(mi);
	}
}

void MI_Acidsplat(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miRange == mis->_miAnimLen) {
		mis->_mix++;
		mis->_miy++;
		mis->_miyoff -= 32;
	}
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddMissile(mis->_mix - 1, mis->_miy - 1, 0, 0, mis->_miDir, MIS_ACIDPUD, 1, mis->_miSource, 2, 2, mis->_miSpllvl);
	} else {
		PutMissile(mi);
	}
}

void MI_Teleport(int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int px, py;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		return;
	}
	assert(mis->_miRange == 1);
	p = &plr[mis->_miSource];
	px = p->_px;
	py = p->_py;
	dPlayer[px][py] = 0;
	//PlrClrTrans(px, py);

	px = mis->_mix;
	py = mis->_miy;
	p->_px = p->_pfutx = p->_poldx = px;
	p->_py = p->_pfuty = p->_poldy = py;
	//PlrDoTrans(px, py);
	dPlayer[px][py] = mis->_miSource + 1;
	ChangeLightXY(p->_plid, px, py);
	ChangeVisionXY(p->_pvid, px, py);
	if (mis->_miSource == myplr) {
		ViewX = px - ScrollInfo._sdx;
		ViewY = py - ScrollInfo._sdy;
	}
}

void MI_Stone(int mi)
{
	MissileStruct *mis;
	MonsterStruct *mon;

	mis = &missile[mi];
	mon = &monster[mis->_miVar2];
	// assert(mon->_mmode == MM_STONE);
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		if (mon->_mhitpoints >= (1 << 6))
			mon->_mmode = mis->_miVar1;
		else
			AddDead(mis->_miVar2);
		return;
	}

	if (mon->_mhitpoints < (1 << 6)) {
		if (mis->_miAnimType != MFILE_SHATTER1) {
			//mis->_miDir = 0;
			mis->_miDrawFlag = TRUE;
			SetMissAnim(mi, MFILE_SHATTER1);
			mis->_miRange = 11;
		}
		PutMissile(mi);
	}
}

void MI_ApocaExp(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	if (!mis->_miVar1) {
		if (CheckMissileCol(mi, mis->_mix, mis->_miy, true))
			mis->_miVar1 = TRUE;
	}
	PutMissile(mi);
}

void MI_Rhino(int mi)
{
	MissileStruct *mis;
	int ax, ay, mix2, miy2, bx, by, mnum;

	mis = &missile[mi];
	mnum = mis->_miSource;
	if (monster[mnum]._mmode != MM_CHARGE) {
		mis->_miDelFlag = TRUE;
		return;
	}
	// restore the real coordinates
	GetMissilePos(mi);
	ax = mis->_mix;
	ay = mis->_miy;
	dMonster[ax][ay] = 0;
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
	if (!PosOkMonst(mnum, mis->_mix, mis->_miy) || (monster[mnum]._mAi == AI_SNAKE && !PosOkMonst(mnum, mix2, miy2))) {
		MissToMonst(mi, ax, ay);
		mis->_miDelFlag = TRUE;
		return;
	}
	bx = missile[mi]._mix;
	by = missile[mi]._miy;
	monster[mnum]._mfutx = bx;
	monster[mnum]._moldx = bx;
	monster[mnum]._mx = bx;
	monster[mnum]._mfuty = by;
	monster[mnum]._moldy = by;
	monster[mnum]._my = by;
	dMonster[bx][by] = -(mnum + 1);
	if (missile[mi]._miLid != -1)
		ChangeLightXY(missile[mi]._miLid, bx, by);
	ShiftMissilePos(mi);
	PutMissile(mi);
}

void MI_Fireman(int mi)
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
	cx = monster[mnum]._menemyx;
	cy = monster[mnum]._menemyy;
	if ((bx != ax || by != ay)
	 && ((mis->_miVar1 && (abs(ax - cx) >= 4 || abs(ay - cy) >= 4)) || mis->_miVar2 > 1)
	 && PosOkMonst(mnum, ax, ay)) {
		MissToMonst(mi, ax, ay);
		mis->_miDelFlag = TRUE;
		return;
	}
	if (!(monster[mnum]._mFlags & MFLAG_TARGETS_MONSTER)) {
		tnum = dPlayer[bx][by];
	} else {
		tnum = dMonster[bx][by];
	}
	if (!PosOkMissile(bx, by) || (tnum > 0 && !mis->_miVar1)) {
		mis->_mixvel *= -1;
		mis->_miyvel *= -1;
		mis->_miDir = OPPOSITE(mis->_miDir);
		mis->_miAnimData = monster[mnum]._mAnims[MA_WALK].Data[mis->_miDir];
		mis->_miVar2++;
		if (tnum > 0)
			mis->_miVar1 = TRUE;
	}
	ShiftMissilePos(mi);
	PutMissile(mi);
}

void MI_WallC(int mi)
{
	MissileStruct *mis;
	int mitype, tx, ty;

	mis = &missile[mi];
	mis->_miDist++;
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
	}
#ifdef HELLFIRE
	mitype = mis->_miType == MIS_FIREWALLC ? MIS_FIREWALL : MIS_LIGHTWALL;
#else
	mitype = MIS_FIREWALL;
#endif
	if (!mis->_miVar8) {
		tx = mis->_miVar1;
		ty = mis->_miVar2;
		assert(IN_DUNGEON_AREA(tx, ty));
		if (!nMissileTable[dPiece[tx][ty]]) {
			AddMissile(tx, ty, 0, 0, 0, mitype, mis->_miCaster, mis->_miSource, 0, 0, mis->_miSpllvl);
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
			AddMissile(tx, ty, 0, 0, 0, mitype, mis->_miCaster, mis->_miSource, 0, 0, mis->_miSpllvl);
			mis->_miVar5 += XDirAdd[mis->_miVar4];
			mis->_miVar6 += YDirAdd[mis->_miVar4];
		} else {
			mis->_miVar7 = TRUE;
		}
	}
}

void MI_Infra(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		CalcPlrItemVals(mis->_miSource, true);
	} else {
		plr[mis->_miSource]._pInfraFlag = TRUE;
	}
}

void MI_FireWaveC(int mi)
{
	MissileStruct *mis;
	int sx, sy, sd, nx, ny, dir;
	int i, j, pnum;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	pnum = mis->_miSource;
	sx = mis->_mix;
	sy = mis->_miy;
	sd = mis->_miVar1;
	sx += XDirAdd[sd];
	sy += YDirAdd[sd];
	if (!nMissileTable[dPiece[sx][sy]]) {
		AddMissile(sx, sy, sx + XDirAdd[sd], sy + YDirAdd[sd], 0, MIS_FIREWAVE, 0, pnum, 0, 0, mis->_miSpllvl);

		for (i = -2; i <= 2; i += 4) {
			dir = (sd + i) & 7;
			nx = sx;
			ny = sy;
			for (j = (mis->_miSpllvl >> 1) + 2; j > 0; j--) {
				nx += XDirAdd[dir];
				ny += YDirAdd[dir];
				if (nx <= 0 || nx >= MAXDUNX || ny <= 0 || ny >= MAXDUNY)
					break;
				if (nMissileTable[dPiece[nx][ny]])
					break;
				AddMissile(nx, ny, nx + XDirAdd[sd], ny + YDirAdd[sd], 0, MIS_FIREWAVE, 0, pnum, 0, 0, mis->_miSpllvl);
			}
		}
	}
}

#ifdef HELLFIRE
void MI_Bloodboil(int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int pnum, hpdif;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0) {
		pnum = mis->_miSource;
		p = &plr[pnum];
		hpdif = p->_pMaxHP - p->_pHitPoints;
		if (p->_pSpellFlags & PSE_BLOOD_BOIL) {
			p->_pSpellFlags &= ~PSE_BLOOD_BOIL;
			p->_pSpellFlags |= PSE_LETHARGY;
			mis->_miRange = mis->_miVar1;
		} else {
			mis->_miDelFlag = TRUE;
			p->_pSpellFlags &= ~PSE_LETHARGY;
			hpdif += mis->_miVar2;
		}
		CalcPlrItemVals(pnum, true);
		p->_pHitPoints -= hpdif;
		if (p->_pHitPoints < (1 << 6))
			p->_pHitPoints = (1 << 6);
		// unnecessary since CalcPlrItemVals always asks for a redraw of hp and mana
		// gbRedrawFlags |= REDRAW_HP_FLASK;
		PlaySfxLoc(sgSFXSets[SFXS_PLR_72][p->_pClass], p->_px, p->_py);
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
	mis->_miRange--;
	k = mis->_miRange;
	if (CheckMissileCol(mi, mis->_mix, mis->_miy, false))
		mis->_miRange = k;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		return;
	}
	mis->_miVar2--;
	if (mis->_miVar2 <= 0) {
		if (mis->_miVar2 == 0)
			mis->_miAnimFrame = 20;
		k = mis->_miAnimFrame;
		if (k > 11)
			k = 24 - k;
		ChangeLight(mis->_miLid, mis->_mix, mis->_miy, k);
		PutMissile(mi);
	}
}

void MI_FlameC(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
		if (!nMissileTable[dPiece[mis->_mix][mis->_miy]]) {
			// mis->_miVar3 used by MIS_FLAME !
			AddMissile(
			    mis->_mix,
			    mis->_miy,
			    0,
			    0,
			    mi,
			    MIS_FLAME,
			    mis->_miCaster,
			    mis->_miSource,
			    0,
				0,
			    mis->_miSpllvl);
		} else {
			mis->_miRange = 0;
		}
		mis->_miVar1 = mis->_mix;
		mis->_miVar2 = mis->_miy;
		mis->_miVar3++;
	}
	if (mis->_miRange == 0 || mis->_miVar3 == 3)
		mis->_miDelFlag = TRUE;
}

void MI_FireTrap(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_mix, mis->_miy, true);

	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
}

void MI_Cbolt(int mi)
{
	MissileStruct *mis;
	int md;
	int bpath[16] = { -1, 0, 1, -1, 0, 1, -1, -1, 0, 0, 1, 1, 0, 1, -1, 0 };

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miAnimType != MFILE_LGHNING) {
		if (mis->_miVar3 == 0) {
			md = (mis->_miVar2 + bpath[mis->_miVar4]) & 7;
			mis->_miVar4 = (mis->_miVar4 + 1) & 0xF;
			GetMissileVel(mi, mis->_mix, mis->_miy, mis->_mix + XDirAdd[md], mis->_miy + YDirAdd[md], 8);
			mis->_miVar3 = 16;
		} else {
			mis->_miVar3--;
		}
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		if ((mis->_misx != mis->_mix || mis->_misy != mis->_miy)
		 && CheckMissileCol(mi, mis->_mix, mis->_miy, false)) {
			mis->_miVar1 = 8;
			mis->_miDir = 0;
			SetMissAnim(mi, MFILE_LGHNING);
			mis->_miRange = mis->_miAnimLen;
		}
		ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miVar1);
	}
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_Element(int mi)
{
	MissileStruct *mis;
	int sd, cx, cy, dx, dy;

	mis = &missile[mi];
	mis->_miRange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	cx = mis->_mix;
	cy = mis->_miy;
	CheckMissileCol(mi, cx, cy, false);
	if (!mis->_miVar3 && cx == mis->_miVar4 && cy == mis->_miVar5) {
		mis->_miVar3 = TRUE;
		mis->_miRange = 255;
		if (FindClosest(cx, cy, dx, dy)) {
			sd = GetDirection8(cx, cy, dx, dy);
		} else {
			sd = plr[mis->_miSource]._pdir;
			dx = cx + XDirAdd[sd];
			dy = cy + YDirAdd[sd];
		}
		SetMissDir(mi, sd);
		GetMissileVel(mi, cx, cy, dx, dy, 16);
	}
	if (cx != mis->_miVar1 || cy != mis->_miVar2) {
		mis->_miVar1 = cx;
		mis->_miVar2 = cy;
		ChangeLight(mis->_miLid, cx, cy, 8);
	}
	if (mis->_miRange == 0) {
		//CheckMissileCol(mi, cx, cy, true);
		// TODO: mis->_miMinDam >>= 1; mis->_miMaxDam >>= 1; ?
		CheckMissileCol(mi, cx, cy + 1, true);
		CheckMissileCol(mi, cx, cy - 1, true);
		CheckMissileCol(mi, cx + 1, cy, true); /* check x/y */
		CheckMissileCol(mi, cx + 1, cy - 1, true);
		CheckMissileCol(mi, cx + 1, cy + 1, true);
		CheckMissileCol(mi, cx - 1, cy, true);
		CheckMissileCol(mi, cx - 1, cy + 1, true);
		CheckMissileCol(mi, cx - 1, cy - 1, true);

		AddMissile(mis->_mix, mis->_miy, mi, 0, 0, MIS_EXELE, mis->_miCaster, mis->_miSource, 0, 0, 0);
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_EleExp(int mi)
{
	MissileStruct *mis;
	int cx, cy;

	mis = &missile[mi];
	cx = mis->_mix;
	cy = mis->_miy;
	if (mis->_miVar1++ == 0)
		mis->_miLid = AddLight(cx, cy, mis->_miAnimFrame);
	else
		ChangeLight(mis->_miLid, cx, cy, mis->_miAnimFrame);
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_Resurrect(int mi)
{
	missile[mi]._miRange--;
	if (missile[mi]._miRange == 0)
		missile[mi]._miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Rportal(int mi)
{
	MissileStruct *mis;
	int ExpLight[17] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15 };

	mis = &missile[mi];
	if (mis->_miRange > 1)
		mis->_miRange--;
	if (mis->_miRange == mis->_miVar1)
		SetMissDir(mi, 1);

	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	} else if (currLvl._dType != DLV_TOWN && mis->_miDir != 1) {
		if (mis->_miVar2 == 0)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, 1);
		else
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}
	PutMissile(mi);
}

void ProcessMissiles()
{
	MissileStruct *mis;
	int i, mi;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		dFlags[mis->_mix][mis->_miy] &= ~BFLAG_MISSILE;
		dMissile[mis->_mix][mis->_miy] = 0;
#ifdef HELLFIRE
		if (mis->_mix < 0 || mis->_mix >= MAXDUNX - 1 || mis->_miy < 0 || mis->_miy >= MAXDUNY - 1)
			mis->_miDelFlag = TRUE;
#endif
	}

	for (i = 0; i < nummissiles; ) {
		mi = missileactive[i];
		if (missile[mi]._miDelFlag) {
			DeleteMissile(mi, i);
		} else {
			i++;
		}
	}

	gbMissilePreFlag = false;

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		mis = &missile[mi];
		missiledata[mis->_miType].mProc(mi);
		if (!(mis->_miAnimFlags & MAFLAG_LOCK_ANIMATION)) {
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

	for (i = 0; i < nummissiles; ) {
		mi = missileactive[i];
		if (missile[mi]._miDelFlag) {
			DeleteMissile(mi, i);
		} else {
			i++;
		}
	}
}

void missiles_process_charge()
{
	MonsterStruct *mon;
	int anim;
	MissileStruct *mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		mis->_miAnimData = misanimdata[mis->_miAnimType][mis->_miDir];
		if (mis->_miType == MIS_RHINO) {
			mon = &monster[mis->_miSource];
			if (mon->_mType >= MT_HORNED && mon->_mType <= MT_OBLORD) {
				anim = MA_SPECIAL;
			} else {
				if (mon->_mType >= MT_NSNAKE && mon->_mType <= MT_GSNAKE)
					anim = MA_ATTACK;
				else
					anim = MA_WALK;
			}
			mis->_miAnimData = mon->_mAnims[anim].Data[mis->_miDir];
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
