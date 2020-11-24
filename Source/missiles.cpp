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
BOOL MissilePreFlag;

/** Maps from direction to X-offset. */
int XDirAdd[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to Y-offset. */
int YDirAdd[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };
int CrawlNum[19] = { 0, 3, 12, 45, 94, 159, 240, 337, 450, 579, 724, 885, 1062, 1255, 1464, 1689, 1930, 2187, 2460 };

void GetDamageAmt(int sn, int *mind, int *maxd)
{
	PlayerStruct *p;
	int k, sl;

	assert((DWORD)myplr < MAX_PLRS);
	assert((DWORD)sn < MAX_SPELLS);
	p = &plr[myplr];
	sl = p->_pSplLvl[sn] + p->_pISplLvlAdd;

	switch (sn) {
	case SPL_FIREBOLT:
		k = (p->_pMagic >> 3) + sl;
		*mind = k + 1;
		*maxd = k + 10;
		break;
	case SPL_HEAL:
		*mind = -1;
		*maxd = -1;
		break;
	case SPL_LIGHTNING:
		*mind = 2;
		*maxd = p->_pLevel + 2;
		break;
	case SPL_FLASH:
		*mind = p->_pLevel;
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
		*mind = (4 * p->_pLevel + 8) >> 1;
		*maxd = (4 * p->_pLevel + 80) >> 1;
		break;
	case SPL_FIREBALL:
		*mind = 2 * p->_pLevel + 4;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*maxd = 2 * p->_pLevel + 40;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		break;
	case SPL_GUARDIAN:
		*mind = (p->_pLevel >> 1) + 1;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*maxd = (p->_pLevel >> 1) + 10;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		break;
	case SPL_CHAIN:
		*mind = 4;
		*maxd = 2 * p->_pLevel + 4;
		break;
	case SPL_WAVE:
		*mind = 6 * (p->_pLevel + 1);
		*maxd = 6 * (p->_pLevel + 10);
		break;
	case SPL_NOVA:
		*mind = (p->_pLevel + 5) >> 1;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*mind *= 5;
		*maxd = (p->_pLevel + 30) >> 1;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		*maxd *= 5;
		break;
	case SPL_FLAME:
		*mind = 3;
		*maxd = p->_pLevel + 4;
		*maxd += *maxd >> 1;
		break;
	case SPL_GOLEM:
		*mind = 2 * sl + 8;
		*maxd = 2 * sl + 16;
		break;
	case SPL_APOCA:
		k = std::max((int)p->_pLevel, 0);
		*mind = k;
		*maxd = 6 * k;
		break;
	case SPL_ELEMENT:
		*mind = 2 * p->_pLevel + 4;
		for (k = 0; k < sl; k++) {
			*mind += *mind >> 3;
		}
		*mind >>= 1;
		*maxd = 2 * p->_pLevel + 40;
		for (k = 0; k < sl; k++) {
			*maxd += *maxd >> 3;
		}
		*maxd >>= 1;
		break;
	case SPL_CBOLT:
		*mind = 1;
		*maxd = p->_pMagic >> 2;
		if (*maxd == 0)
			*maxd = 1;
		break;
	case SPL_HBOLT:
		*mind = p->_pLevel + 9;
		*maxd = p->_pLevel + 18;
		break;
	case SPL_HEALOTHER:
		*mind = -1;
		*maxd = -1;
		break;
	case SPL_FLARE:
		*mind = (p->_pMagic >> 1) + 3 * sl - (p->_pMagic >> 3);
		*maxd = *mind;
		break;
	}
}

static int FindClosest(int sx, int sy, int rad)
{
	int j, i, mid, tx, ty;
	char *cr;

	if (rad > 19)
		rad = 19;

	for (i = 1; i < rad; i++) {
		cr = &CrawlTable[CrawlNum[i]];
#ifdef HELLFIRE
		for (j = *cr; j > 0; j--) { // BUGFIX: should cast to BYTE or CrawlTable header will be wrong
#else
		for (j = (BYTE)*cr; j > 0; j--) {
#endif
			tx = sx + *++cr;
			ty = sy + *++cr;
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				mid = dMonster[tx][ty];
				if (mid > 0 && LineClearF(CheckNoSolid, sx, sy, tx, ty))
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
}

void DeleteMissile(int mi, int idx)
{
	int src;

	if (missile[mi]._miType == MIS_MANASHIELD) {
		src = missile[mi]._miSource;
		if (src == myplr)
			NetSendCmd(TRUE, CMD_REMSHIELD);
		plr[src].pManaShield = FALSE;
	}

	missileavail[MAXMISSILES - nummissiles] = mi;
	nummissiles--;
	if (nummissiles > 0 && idx != nummissiles)
		missileactive[idx] = missileactive[nummissiles];
}

static void GetMissileVel(int mi, int sx, int sy, int dx, int dy, int v)
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
			MissilePreFlag = TRUE;
	}
}

static void GetMissilePos(int mi)
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
	ChangeLightOff(mis->_miLid, lx - (dx * 8), ly - (dy * 8));
}

static void MoveMissilePos(int mi)
{
	MissileStruct *mis;
	int dx, dy, x, y;

	mis = &missile[mi];
	switch (mis->_miDir) {
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
	if (PosOkMonst(mis->_miSource, x, y)) {
		mis->_mix += dx;
		mis->_miy += dy;
		mis->_mixoff += (dy << 5) - (dx << 5);
		mis->_miyoff -= (dy << 4) + (dx << 4);
	}
}

static BOOL CheckMonsterRes(unsigned short mor, unsigned char mRes, BOOL *resist)
{
	switch (mRes) {
	case MISR_FIRE:
		if (mor & IMMUNE_FIRE)
			return TRUE;
		*resist = (mor & RESIST_FIRE) != 0;
		break;
	case MISR_LIGHTNING:
		if (mor & IMMUNE_LIGHTNING)
			return TRUE;
		*resist = (mor & RESIST_LIGHTNING) != 0;
		break;
	case MISR_MAGIC:
		if (mor & IMMUNE_MAGIC)
			return TRUE;
		*resist = (mor & RESIST_MAGIC) != 0;
		break;
	case MISR_ACID:
		if (mor & IMMUNE_ACID)
			return TRUE;
		// BUGFIX: TODO player is resistant against ACID with RESIST_MAGIC, monsters should behave the same
	default:
		*resist = FALSE;
		break;
	}
	return FALSE;
}

BOOL MonsterTrapHit(int mnum, int mindam, int maxdam, int dist, int mitype, BOOL shift)
{
	MonsterStruct *mon;
	MissileData *mds;
	int hit, hper, dam;
	BOOL resist, ret;

	mon = &monster[mnum];
	mds = &missiledata[mitype];
	if (CheckMonsterRes(mon->mMagicRes, mds->mResist, &resist))
		return FALSE;

	if (CheckMonsterHit(mnum, &ret)) {
		return ret;
	}

	hper = 90 - mon->mArmorClass - dist;
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(68, 100) >= hper && mon->_mmode != MM_STONE)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return FALSE;

	dam = RandRange(mindam, maxdam);
	if (!shift)
		dam <<= 6;
	if (resist)
		dam >>= 2;

	mon->_mhitpoints -= dam;
#ifdef _DEBUG
	if (debug_mode_god_mode)
		mon->_mhitpoints = 0;
#endif
	if (mon->_mhitpoints >> 6 <= 0) {
		MonStartKill(mnum, -1);
	} else {
		if (resist) {
			PlayEffect(mnum, 1);
		} else {
			if (mnum >= MAX_PLRS)
				MonStartHit(mnum, -1, dam);
		}
	}
	return TRUE;
}

static BOOL MonsterMHit(int mnum, int pnum, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift)
{
	PlayerStruct *p;
	MonsterStruct *mon;
	MissileData *mds;
	int hper, dam;
	BOOL resist, ret;

	mon = &monster[mnum];
	mds = &missiledata[mitype];
	if (mds->mdFlags & MIFLAG_UNDEAD && mon->MData->mMonstClass != MC_UNDEAD && mon->MType->mtype != MT_DIABLO)
		return TRUE;

	if (CheckMonsterRes(mon->mMagicRes, mds->mResist, &resist))
		return FALSE;

	if (CheckMonsterHit(mnum, &ret))
		return ret;

	p = &plr[pnum];
	if (mds->mType == 0) {
		hper = p->_pDexterity
		    + p->_pIBonusToHit
		    + p->_pLevel
		    - mon->mArmorClass
		    - (dist * dist >> 1)
		    + p->_pIEnAc
		    + 50;
		if (p->_pClass == PC_ROGUE)
			hper += 20;
		else if (p->_pClass == PC_WARRIOR)
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
	if (random_(69, 100) >= hper && mon->_mmode != MM_STONE)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return FALSE;

	if (mds->mName == MIS_BONESPIRIT) {
		dam = mon->_mhitpoints / 3;
	} else {
		dam = RandRange(mindam, maxdam);
		if (mds->mType == 0) {
			dam = p->_pIBonusDamMod + dam * p->_pIBonusDam / 100 + dam;
			if (p->_pClass == PC_ROGUE)
				dam += p->_pDamageMod;
			else
				dam += (p->_pDamageMod >> 1);
			if ((p->_pIFlags & ISPL_3XDAMVDEM) && mon->MData->mMonstClass == MC_DEMON)
				dam *= 3;
			if (p->_pIFlags & ISPL_NOHEALMON)
				mon->_mFlags |= MFLAG_NOHEAL;
		}
		if (!shift)
			dam <<= 6;
	}
	if (resist)
		dam >>= 2;
	if (pnum == myplr)
		mon->_mhitpoints -= dam;

	if (mon->_mhitpoints >> 6 <= 0) {
		MonStartKill(mnum, pnum);
	} else {
		if (resist) {
			PlayEffect(mnum, 1);
		} else {
			if (mon->_mmode != MM_STONE && mds->mType == 0 && p->_pIFlags & ISPL_KNOCKBACK) {
				MonGetKnockback(mnum);
			}
			if (mnum >= MAX_PLRS)
				MonStartHit(mnum, pnum, dam);
		}
	}

	if (mon->_msquelch == 0) {
		mon->_msquelch = UCHAR_MAX;
		mon->_lastx = p->_px;
		mon->_lasty = p->_py;
	}
	return TRUE;
}

BOOL PlayerTrapHit(int pnum, int mind, int maxd, int dist, int mitype, BOOL shift)
{
	PlayerStruct *p;
	MissileData *mds;
	int hper, tmp, dam, resper;
	BOOL blk;

	p = &plr[pnum];
	if (p->_pHitPoints >> 6 <= 0 || p->_pInvincible) {
		return FALSE;
	}
	mds = &missiledata[mitype];
	if ((p->_pSpellFlags & PSE_ETHERALIZED) && mds->mType == 0) {
		return FALSE;
	}

	if (mds->mType == 0) {
		tmp = p->_pIAC + p->_pIBonusAC + p->_pDexterity / 5;
		hper = 100 - (tmp >> 1);
		hper -= dist << 1;
	} else {
		hper = 40;
	}

	tmp = 10;
	if (currlevel == 14)
		tmp = 20;
	else if (currlevel == 15)
		tmp = 25;
	else if (currlevel == 16)
		tmp = 30;
	if (hper < tmp)
		hper = tmp;
	if (random_(72, 100) >= hper)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return FALSE;

	if ((mds->mdFlags & MIFLAG_NOBLOCK)
	 || !p->_pBlockFlag || (p->_pmode != PM_STAND && p->_pmode != PM_ATTACK)) {
		blk = FALSE;
	} else {
		tmp = p->_pBaseToBlk + p->_pDexterity;
		if (tmp <= 0)
			blk = FALSE;
		else if (tmp >= 100)
			blk = TRUE;
		else
			blk = random_(73, 100) < tmp;
	}

	if (mds->mName == MIS_BONESPIRIT) {
		dam = p->_pHitPoints / 3;
	} else {
		dam = RandRange(mind, maxd);
		if (p->_pIFlags & ISPL_ABSHALFTRAP)
			dam >>= 1;
		if (!shift)
			dam <<= 6;
		if (dam < 64)
			dam = 64;
	}

	switch (mds->mResist) {
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
	if (resper > 0) {
		dam = dam - dam * resper / 100;
		if (pnum == myplr) {
			p->_pHitPoints -= dam;
			p->_pHPBase -= dam;
		}
		if (p->_pHitPoints >> 6 <= 0) {
			SyncPlrKill(pnum, 0);
		} else {
			PlaySfxLoc(sgSFXSets[SFXS_PLR_69][p->_pClass], p->_px, p->_py, 2);
			drawhpflag = TRUE;
		}
	} else {
		if (blk) {
			PlrStartBlock(pnum, p->_pdir);
		} else {
			if (pnum == myplr) {
				p->_pHitPoints -= dam;
				p->_pHPBase -= dam;
			}
			if (p->_pHitPoints >> 6 <= 0) {
				SyncPlrKill(pnum, 0);
			} else {
				StartPlrHit(pnum, dam, FALSE);
			}
		}
	}
	return TRUE;
}

static BOOL PlayerMHit(int pnum, int mnum, int mind, int maxd, int dist, int mitype, BOOLEAN shift)
{
	PlayerStruct *p;
	MonsterStruct *mon;
	MissileData *mds;
	int hper, tmp, dam, resper;
	BOOL blk;

	p = &plr[pnum];
	if (p->_pHitPoints >> 6 <= 0 || p->_pInvincible) {
		return FALSE;
	}

	mds = &missiledata[mitype];
	if ((p->_pSpellFlags & PSE_ETHERALIZED) && mds->mType == 0) {
		return FALSE;
	}

	mon = &monster[mnum];
	if (mds->mType == 0) {
		tmp = p->_pIAC + p->_pIBonusAC + p->_pDexterity / 5;
		hper = 30 + mon->mHit
		    + (mon->mLevel << 1)
			- (p->_pLevel << 1)
		    - tmp;
		hper -= dist << 1;
	} else {
		hper = 40
			+ (mon->mLevel << 1)
			- (p->_pLevel << 1)
			- (dist << 1);
	}

	tmp = 10;
	if (currlevel == 14)
		tmp = 20;
	else if (currlevel == 15)
		tmp = 25;
	else if (currlevel == 16)
		tmp = 30;
	if (hper < tmp)
		hper = tmp;
	if (random_(72, 100) >= hper)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return FALSE;

	if ((mds->mdFlags & MIFLAG_NOBLOCK)
	 || !p->_pBlockFlag || (p->_pmode != PM_STAND && p->_pmode != PM_ATTACK)) {
		blk = FALSE;
	} else {
		tmp = p->_pBaseToBlk + p->_pDexterity;
		if (mon != NULL)
			tmp = tmp
				+ (p->_pLevel << 1)
				- (mon->mLevel << 1);
		if (tmp <= 0)
			blk = FALSE;
		else if (tmp >= 100)
			blk = TRUE;
		else
			blk = random_(73, 100) < tmp;
	}

	if (mds->mName == MIS_BONESPIRIT) {
		dam = p->_pHitPoints / 3;
	} else {
		dam = RandRange(mind, maxd);
		dam += p->_pIGetHit;
		if (!shift)
			dam <<= 6;
		if (dam < 64)
			dam = 64;
	}

	switch (mds->mResist) {
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
	if (resper > 0) {
		dam = dam - dam * resper / 100;
		if (pnum == myplr) {
			p->_pHitPoints -= dam;
			p->_pHPBase -= dam;
		}
		if (p->_pHitPoints >> 6 <= 0) {
			SyncPlrKill(pnum, 0);
		} else {
			PlaySfxLoc(sgSFXSets[SFXS_PLR_69][p->_pClass], p->_px, p->_py, 2);
			drawhpflag = TRUE;
		}
	} else {
		if (blk) {
			tmp = GetDirection(p->_px, p->_py, mon->_mx, mon->_my);
			PlrStartBlock(pnum, tmp);
		} else {
			if (pnum == myplr) {
				p->_pHitPoints -= dam;
				p->_pHPBase -= dam;
			}
			if (p->_pHitPoints >> 6 <= 0) {
				SyncPlrKill(pnum, 0);
			} else {
				StartPlrHit(pnum, dam, FALSE);
			}
		}
	}
	return TRUE;
}

static BOOL Plr2PlrMHit(int defp, int offp, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift)
{
	PlayerStruct *ops, *dps;
	MissileData *mds;
	int resper, dam, blkper, hper;
	BOOL blk;

	dps = &plr[defp];
	if (dps->_pInvincible) {
		return FALSE;
	}

	mds = &missiledata[mitype];
	if (mds->mdFlags & MIFLAG_UNDEAD) {
		return FALSE;
	}

	if ((dps->_pSpellFlags & PSE_ETHERALIZED) && mds->mType == 0) {
		return FALSE;
	}

	ops = &plr[offp];
	if (mds->mType == 0) {
		hper = ops->_pIBonusToHit
		    + ops->_pLevel
		    - (dist * dist >> 1)
		    - dps->_pDexterity / 5
		    - dps->_pIBonusAC
		    - dps->_pIAC
		    + ops->_pDexterity + 50;
		if (ops->_pClass == PC_ROGUE)
			hper += 20;
		if (ops->_pClass == PC_WARRIOR)
			hper += 10;
	} else {
		if (mds->mdFlags & MIFLAG_AREA) {
			hper = 40
				+ (ops->_pLevel << 1)
				- (dps->_pLevel << 1);
		} else {
			hper = ops->_pMagic
				- (dps->_pLevel << 1)
				- dist
				+ 50;
			if (ops->_pClass == PC_SORCERER)
				hper += 20;
		}
	}
	if (hper < 5)
		hper = 5;
	if (hper > 95)
		hper = 95;
	if (random_(69, 100) >= hper)
		return FALSE;

	if ((mds->mdFlags & MIFLAG_NOBLOCK)
	 || !dps->_pBlockFlag || (dps->_pmode != PM_STAND && dps->_pmode != PM_ATTACK)) {
		blk = FALSE;
	} else {
		blkper = dps->_pDexterity + dps->_pBaseToBlk
			+ ((dps->_pLevel - ops->_pLevel) << 1);
		if (blkper <= 0)
			blk = FALSE;
		else if (blkper >= 100)
			blk = TRUE;
		else
			blk = random_(73, 100) < blkper;
	}

	if (mds->mName == MIS_BONESPIRIT) {
		dam = dps->_pHitPoints / 3;
	} else {
		dam = RandRange(mindam, maxdam);
		if (mds->mType == 0) {
			dam += ops->_pIBonusDamMod + dam * ops->_pIBonusDam / 100;
			if (ops->_pClass == PC_ROGUE)
				dam += ops->_pDamageMod;
			else
				dam += (ops->_pDamageMod >> 1);
		}
		if (!shift)
			dam <<= 6;
	}
	if (mds->mType != 0)
		dam >>= 1;

	switch (mds->mResist) {
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
	if (resper > 0) {
		if (offp == myplr)
			NetSendCmdDamage(TRUE, defp, dam - resper * dam / 100);
		PlaySfxLoc(sgSFXSets[SFXS_PLR_69][ops->_pClass], ops->_px, ops->_py, 2);
	} else {
		if (blk) {
			PlrStartBlock(defp, GetDirection(dps->_px, dps->_py, ops->_px, ops->_py));
		} else {
			if (offp == myplr)
				NetSendCmdDamage(TRUE, defp, dam);
			StartPlrHit(defp, dam, FALSE);
		}
	}
	return TRUE;
}

static BOOL MonMissHit(int mnum, int mi, int mindam, int maxdam, int shift)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miSource != -1) {
		if (mis->_miCaster == 0) {
			// player vs. monster
			return MonsterMHit(mnum, mis->_miSource, mindam, maxdam,
					mis->_miDist, mis->_miType, shift);
		} else {
			// monster vs. golem
			return mnum < MAX_PLRS && MonsterTrapHit(mnum, mindam, maxdam,
				mis->_miDist, mis->_miType, shift);
		}
	} else {
		// trap vs. monster
		return MonsterTrapHit(mnum, mindam, maxdam,
				mis->_miDist, mis->_miType, shift);
	}
}

static BOOL PlrMissHit(int pnum, int mi, int mindam, int maxdam, int shift)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miSource != -1) {
		if (mis->_miCaster == 0) {
			// player vs. player
			return pnum != mis->_miSource
			 && Plr2PlrMHit(pnum, mis->_miSource, mindam, maxdam,
				 mis->_miDist, mis->_miType, shift);
		} else {
			// monster vs. player
			return PlayerMHit(pnum, mis->_miSource, mindam, maxdam,
					mis->_miDist, mis->_miType, shift);
		}
	} else {
		// trap vs. player
		return PlayerTrapHit(pnum, mindam, maxdam,
				mis->_miDist, mis->_miType, shift);
	}
}

static BOOL CheckMissileCol(int mi, int mindam, int maxdam, BOOL shift, int mx, int my, BOOLEAN nodel)
{
	MissileStruct *mis;
	MissileData *mds;
	int oi, mnum, pnum;
	int hit = 0;

	mnum = dMonster[mx][my];
	if (mnum != 0) {
		if (mnum > 0) {
			mnum--;
			if (MonMissHit(mnum, mi, mindam, maxdam, shift))
				hit = 1;
		} else {
			mnum = -(mnum + 1);
			if (monster[mnum]._mmode == MM_STONE) {
				if (MonMissHit(mnum, mi, mindam, maxdam, shift))
					hit = 1;
			}
		}
	}

	pnum = dPlayer[mx][my];
	if (pnum > 0) {
		pnum--;
		if (PlrMissHit(pnum, mi, mindam, maxdam, shift))
			hit = 1;
	}

	oi = dObject[mx][my];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (!object[oi]._oMissFlag) {
			if (object[oi]._oBreak == 1)
				BreakObject(-1, oi);
			hit = 2;
		}
	}
	if (nMissileTable[dPiece[mx][my]]) {
		hit = 2;
	}

	if (hit == 0)
		return FALSE;

	if (!nodel) {
		mis = &missile[mi];
		mis->_miRange = 0;
		mds = &missiledata[mis->_miType];
		if (mds->miSFX != -1)
			PlaySfxLoc(mds->miSFX, mis->_mix, mis->_miy, mds->miSFXCnt);
	}
	return hit == 1;
}

void SetMissAnim(int mi, int animtype)
{
	MissileStruct *mis;
	MisFileData *mfd;
	int dir;

	mis = &missile[mi];
	mis->_miAnimType = animtype;
	mfd = &misfiledata[animtype];
	dir = mis->_miDir;
	mis->_miAnimFlags = mfd->mfFlags;
	mis->_miAnimData = mfd->mfAnimData[dir];
	mis->_miAnimDelay = mfd->mfAnimDelay[dir];
	mis->_miAnimLen = mfd->mfAnimLen[dir];
	mis->_miAnimWidth = mfd->mfAnimWidth[dir];
	mis->_miAnimWidth2 = mfd->mfAnimWidth2[dir];
	mis->_miAnimCnt = 0;
	mis->_miAnimFrame = 1;
}

void SetMissDir(int mi, int dir)
{
	missile[mi]._miDir = dir;
	SetMissAnim(mi, missile[mi]._miAnimType);
}

void LoadMissileGFX(BYTE midx)
{
	char pszName[256];
	int i;
	BYTE *file;
	MisFileData *mfd;

	mfd = &misfiledata[midx];
	if (mfd->mfFlags & MFLAG_ALLOW_SPECIAL) {
		sprintf(pszName, "Missiles\\%s.CL2", mfd->mfName);
		file = LoadFileInMem(pszName, NULL);
		for (i = 0; i < mfd->mfAnimFAmt; i++)
			mfd->mfAnimData[i] = CelGetFrameStart(file, i);
	} else if (mfd->mfAnimFAmt == 1) {
		sprintf(pszName, "Missiles\\%s.CL2", mfd->mfName);
		if (mfd->mfAnimData[0] == NULL)
			mfd->mfAnimData[0] = LoadFileInMem(pszName, NULL);
	} else {
		for (i = 0; i < mfd->mfAnimFAmt; i++) {
			sprintf(pszName, "Missiles\\%s%i.CL2", mfd->mfName, i + 1);
			if (mfd->mfAnimData[i] == NULL) {
				file = LoadFileInMem(pszName, NULL);
				mfd->mfAnimData[i] = file;
			}
		}
	}
}

void InitMissileGFX()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (!(misfiledata[i].mfFlags & MFLAG_HIDDEN))
			LoadMissileGFX(i);
	}
}

static void FreeMissileGFX(int midx)
{
	MisFileData *mfd;
	int i;
	DWORD *p;

	mfd = &misfiledata[midx];
	if (mfd->mfFlags & MFLAG_ALLOW_SPECIAL) {
		if (mfd->mfAnimData[0] != NULL) {
			p = (DWORD *)mfd->mfAnimData[0];
			p -= mfd->mfAnimFAmt;
			MemFreeDbg(p);
			mfd->mfAnimData[0] = NULL;
		}
		return;
	}

	for (i = 0; i < mfd->mfAnimFAmt; i++) {
		if (mfd->mfAnimData[i] != NULL) {
			MemFreeDbg(mfd->mfAnimData[i]);
		}
	}
}

void FreeMissiles()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (!(misfiledata[i].mfFlags & MFLAG_HIDDEN))
			FreeMissileGFX(i);
	}
}

void FreeMissiles2()
{
	int i;

	for (i = 0; i < NUM_MFILE; i++) {
		if (misfiledata[i].mfFlags & MFLAG_HIDDEN)
			FreeMissileGFX(i);
	}
}

void InitMissiles()
{
	MissileStruct *mis;
	PlayerStruct *p;
	int i, j;

#ifdef HELLFIRE
	AutoMapShowItems = FALSE;
#endif
	p = &plr[myplr];
	p->_pSpellFlags &= ~PSE_ETHERALIZED;
	if (p->_pInfraFlag) {
		for (i = 0; i < nummissiles; ++i) {
			mis = &missile[missileactive[i]];
			if (mis->_miType == MIS_INFRA && mis->_miSource == myplr) {
				CalcPlrItemVals(myplr, TRUE);
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
				CalcPlrItemVals(myplr, TRUE);
				p->_pHitPoints -= missingHP + mis->_miVar2;
				if (p->_pHitPoints < 64) {
					p->_pHitPoints = 64;
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
void AddHiveexpC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	AddMissile(80, 62, 80, 62, midir, MIS_HIVEEXP, micaster, misource, missile[mi]._miDam, 0);
	AddMissile(80, 63, 80, 62, midir, MIS_HIVEEXP, micaster, misource, missile[mi]._miDam, 0);
	AddMissile(81, 62, 80, 62, midir, MIS_HIVEEXP, micaster, misource, missile[mi]._miDam, 0);
	AddMissile(81, 63, 80, 62, midir, MIS_HIVEEXP, micaster, misource, missile[mi]._miDam, 0);
	missile[mi]._miDelFlag = TRUE;
}

static BOOLEAN missiles_found_target(int mi, int *x, int *y, int rad)
{
	int i, j, tx, ty;
	char *cr;

	if (rad > 19)
		rad = 19;

	for (i = 0; i < rad; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = *cr; j > 0; j--) {
			tx = *x + *++cr;
			ty = *y + *++cr;
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				if ((nSolidTable[dPiece[tx][ty]] | dObject[tx][ty] | dMissile[tx][ty]) == 0) {
					missile[mi]._mix = tx;
					missile[mi]._miy = ty;
					*x = tx;
					*y = ty;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/**
 * Var1: mitype to fire upon impact
 */
void AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (misource >= 0)
			UseMana(misource, SPL_RUNEFIRE);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_HIVEEXP;
			missile[mi]._miLid = AddLight(dx, dy, 8);
		} else {
			missile[mi]._miDelFlag = TRUE;
		}
	} else {
		missile[mi]._miDelFlag = TRUE;
	}
}

/**
 * Var1: mitype to fire upon impact
 */
void AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (misource >= 0)
			UseMana(misource, SPL_RUNELIGHT);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_LIGHTBALL;
			missile[mi]._miLid = AddLight(dx, dy, 8);
			return;
		}
	}
	missile[mi]._miDelFlag = TRUE;
}

/**
 * Var1: mitype to fire upon impact
 */
void AddGreatLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (misource >= 0)
			UseMana(misource, SPL_RUNENOVA);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_LIGHTNOVAC;
			missile[mi]._miLid = AddLight(dx, dy, 8);
			return;
		}
	}
	missile[mi]._miDelFlag = TRUE;
}

/**
 * Var1: mitype to fire upon impact
 */
void AddImmolationRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (misource >= 0)
			UseMana(misource, SPL_RUNEIMMOLAT);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_FIRENOVAC;
			missile[mi]._miLid = AddLight(dx, dy, 8);
			return;
		}
	}
	missile[mi]._miDelFlag = TRUE;
}

/**
 * Var1: mitype to fire upon impact
 */
void AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (LineClear(sx, sy, dx, dy)) {
		if (misource >= 0)
			UseMana(misource, SPL_RUNESTONE);
		if (missiles_found_target(mi, &dx, &dy, 10)) {
			missile[mi]._miVar1 = MIS_STONE;
			missile[mi]._miLid = AddLight(dx, dy, 8);
			return;
		}
	}
	missile[mi]._miDelFlag = TRUE;
}

void AddReflection(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int lvl;

	if (misource >= 0) {
		lvl = spllvl;
		if (lvl == 0)
			lvl = 2;
		plr[misource].wReflection += lvl * plr[misource]._pLevel;
		UseMana(misource, SPL_REFLECT);
	}
}

void AddBerserk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MonsterStruct *mon;
	int i, j, tx, ty, dm, r;
	char *cr;

	missile[mi]._miDelFlag = TRUE;

	if (misource >= 0) {
		for (i = 0; i < 6; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = *cr; j > 0; j--) {
				tx = dx + *++cr;
				ty = dy + *++cr;
				if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
					dm = dMonster[tx][ty];
					dm = dm >= 0 ? dm - 1 : -(dm + 1);
					if (dm >= MAX_PLRS) {
						mon = &monster[dm];
						if (mon->_uniqtype == 0 && mon->_mAi != AI_DIABLO) {
							if (mon->_mmode != MM_FADEIN && mon->_mmode != MM_FADEOUT) {
								if (!(mon->mMagicRes & IMMUNE_MAGIC)) {
									if ((!(mon->mMagicRes & RESIST_MAGIC) || random_(99, 2) == 0) && mon->_mmode != MM_CHARGE) {
										double slvl = (double)GetSpellLevel(misource, SPL_BERSERK);
										mon->_mFlags |= MFLAG_UNUSED | MFLAG_GOLEM;
										mon->mMinDamage = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)mon->mMinDamage + slvl;
										mon->mMaxDamage = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)mon->mMaxDamage + slvl;
										mon->mMinDamage2 = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)mon->mMinDamage2 + slvl;
										mon->mMaxDamage2 = ((double)(random_(145, 10) + 20) / 100 - -1) * (double)mon->mMaxDamage2 + slvl;
										if (currlevel < 17 || currlevel > 20)
											r = 3;
										else
											r = 9;
										mon->mlid = AddLight(mon->_mx, mon->_my, r);
										UseMana(misource, SPL_BERSERK);
										return;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

/**
 * Var1: direction to place the spawn
 */
void AddHorkSpawn(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	GetMissileVel(mi, sx, sy, dx, dy, 8);
	missile[mi]._miRange = 9;
	missile[mi]._miVar1 = midir;
	PutMissile(mi);
}

void AddJester(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int spell;

	missile[mi]._miDelFlag = TRUE;
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
		UseMana(misource, SPL_TOWN);
		break;
	case 7:
		spell = MIS_TELEPORT;
		break;
	case 8:
		spell = MIS_APOCAC;
		break;
	default:
		spell = MIS_STONE;
		break;
	}
	AddMissile(sx, sy, dx, dy, midir, spell, micaster, misource, 0, spllvl);
}

void AddStealPots(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	ItemStruct *pi;
	int i, j, tx, ty, si, ii, pnum;
	char *cr;
	BOOL hasPlayedSFX;

	missile[mi]._miDelFlag = TRUE;
	for (i = 0; i < 3; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = *cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				pnum = dPlayer[tx][ty];
				if (pnum != 0) {
					pnum = pnum > 0 ? pnum - 1 : -(pnum + 1);

					hasPlayedSFX = FALSE;
					pi = plr[pnum].SpdList;
					for (si = 0; si < MAXBELTITEMS; si++, pi++) {
						ii = -1;
						if (pi->_itype == ITYPE_MISC) {
							if (random_(205, 2)) {
								switch (pi->_iMiscId) {
								case IMISC_FULLHEAL:
									ii = IDI_HEAL;
									break;
								case IMISC_HEAL:
								case IMISC_MANA:
									RemoveSpdBarItem(pnum, si);
									continue;
								case IMISC_FULLMANA:
									ii = IDI_MANA;
									break;
								case IMISC_REJUV:
									if (random_(205, 2)) {
										ii = IDI_MANA;
									} else {
										ii = IDI_HEAL;
									}
									break;
								case IMISC_FULLREJUV:
									switch (random_(205, 3)) {
									case 0:
										ii = IDI_FULLMANA;
										break;
									case 1:
										ii = IDI_FULLHEAL;
										break;
									default:
										ii = IDI_REJUV;
										break;
									}
									break;
								}
							}
						}
						if (ii != -1) {
							SetItemData(pi, ii);
							GetItemSeed(pi);
							pi->_iStatFlag = TRUE;
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
}

void AddManaTrap(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int i, j, tx, ty, pnum;
	char *cr;

	missile[mi]._miDelFlag = TRUE;
	for (i = 0; i < 3; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = *cr; j > 0; j--) {
			tx = sx + *++cr;
			ty = sy + *++cr;
			if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
				pnum = dPlayer[tx][ty];
				if (pnum != 0) {
					pnum = pnum >= 0 ? pnum - 1 : -(pnum + 1);
					plr[pnum]._pMana = 0;
					plr[pnum]._pManaBase = plr[pnum]._pMana + plr[pnum]._pMaxManaBase - plr[pnum]._pMaxMana;
					CalcPlrInv(pnum, FALSE);
					drawmanaflag = TRUE;
					PlaySfxLoc(TSFX_COW7, tx, ty);
				}
			}
		}
	}
}

/**
 * Var1: dx destination of the missile
 * Var2: dy destination of the missile
 * Var3: the arrow direction
 * Var4: the arrow type
 * Var5: the arrow velocity bonus
 */
void AddSpecArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int av, mitype, flags;

	mis = &missile[mi];
	mis->_miRange = 1;
	mis->_miVar1 = dx;
	mis->_miVar2 = dy;
	mis->_miVar3 = midir;

	av = 0;
	mitype = 0;
	if (misource != -1) {
		p = &plr[misource];
		switch (p->_pILMinDam) {
		case 0:
			mitype = MIS_FIREBALL2;
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

		if (p->_pClass == PC_ROGUE)
			av += (p->_pLevel - 1) >> 2;
		else if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARD)
			av += (p->_pLevel - 1) >> 3;

		flags = p->_pIFlags;
		if (flags & ISPL_QUICKATTACK)
			av++;
		if (flags & ISPL_FASTATTACK)
			av += 2;
		if (flags & ISPL_FASTERATTACK)
			av += 4;
		if (flags & ISPL_FASTESTATTACK)
			av += 8;
	}
	mis->_miVar4 = mitype;
	mis->_miVar5 = av;
}

void AddWarp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int tx, ty, fx, fy, i, dist;
	TriggerStruct *trg;

	dist = INT_MAX;
	if (misource >= 0) {
		sx = plr[misource]._px;
		sy = plr[misource]._py;
	}
	tx = sx;
	ty = sy;

	trg = trigs;
	for (i = std::min(numtrigs, MAXTRIGGERS); i > 0; i--, trg++) {
		if (trg->_tmsg == WM_DIABTWARPUP || trg->_tmsg == WM_DIABPREVLVL || trg->_tmsg == WM_DIABNEXTLVL || trg->_tmsg == WM_DIABRTNLVL) {
			fx = trg->_tx;
			fy = trg->_ty;
			if ((leveltype == DTYPE_CATHEDRAL || leveltype == DTYPE_CATACOMBS) && (trg->_tmsg == WM_DIABNEXTLVL || trg->_tmsg == WM_DIABPREVLVL || trg->_tmsg == WM_DIABRTNLVL)) {
				fy++;
			} else {
				fx++;
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
	mis = &missile[mi];
	mis->_miRange = 2;
	mis->_mix = tx;
	mis->_miy = ty;
	if (micaster == 0)
		UseMana(misource, SPL_WARP);
}

void AddLightwall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_miRange = 255 * (spllvl + 1);
	mis->_miAnimFrame = random_(63, 8) + 1;
}

void AddHiveexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	mis = &missile[mi];
	mis->_miLid = AddLight(sx, sy, 8);
	if (micaster == 0) {
		dam = 2 * (plr[misource]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
		for (i = spllvl; i > 0; i--) {
			dam += dam >> 3;
		}
		mis->_miDam = dam;
		CheckMissileCol(mi, dam, dam, 0, mis->_mix - 1, mis->_miy - 1, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix, mis->_miy - 1, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix + 1, mis->_miy - 1, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix - 1, mis->_miy, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix, mis->_miy, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix + 1, mis->_miy, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix - 1, mis->_miy + 1, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix, mis->_miy + 1, 1);
		CheckMissileCol(mi, dam, dam, 0, mis->_mix + 1, mis->_miy + 1, 1);
	}
	SetMissDir(mi, 0);
	mis->_miRange = mis->_miAnimLen - 1;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 * Var6: animation + direction
 * Var7: animation + direction
 */
void AddFireball3(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	if (micaster == 0) {
		dam = 2 * (plr[misource]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
		for (i = spllvl; i > 0; i--) {
			dam += dam >> 3;
		}
		i = 2 * spllvl + 16;
		if (i > 50)
			i = 50;
		UseMana(misource, SPL_FIREBALL);
	} else {
		dam = RandRange(monster[misource].mMinDamage, monster[misource].mMaxDamage);
		i = 16;
	}
	mis->_miDam = dam;

	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar6 = 2;
	mis->_miVar7 = 2;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddFireball2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int av = 16;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (micaster == 0) {
		av += spllvl;
		if (av > 50) {
			av = 50;
		}
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 */
void AddLightArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 32);
	mis = &missile[mi];
	if (misource >= 0) {
		sx = plr[misource]._px;
		sy = plr[misource]._py;
	}
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miDam <<= 6;
	mis->_miRange = 255;
	mis->_miAnimFrame = random_(52, 8) + 1;
}

void AddFlashfr(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
}

void AddFlashbk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int lvl;

	if (micaster == 0 && misource != -1) {
		missile[mi]._miDam = 0;
		if (misource > 0)
			lvl = plr[misource]._pLevel;
		else
			lvl = 1;
		missile[mi]._miRange = lvl + 10 * spllvl + 245;
	}
}

void AddManaRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	PlayerStruct *p;
	int i, ManaAmount;

	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_MANA);

	p = &plr[misource];
	if (p->_pIFlags & ISPL_NOMANA)
		return;

	ManaAmount = random_(57, 10) + 1;
	for (i = p->_pLevel; i > 0; i--) {
		ManaAmount += random_(57, 4) + 1;
	}
	for (i = spllvl; i > 0; i--) {
		ManaAmount += random_(57, 6) + 1;
	}
	ManaAmount <<= 6;
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
	drawmanaflag = TRUE;
}

void AddMagiRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	PlayerStruct *p;

	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_MAGI);
	p = &plr[misource];
	if (p->_pIFlags & ISPL_NOMANA)
		return;

	p->_pMana = p->_pMaxMana;
	p->_pManaBase = p->_pMaxManaBase;
	drawmanaflag = TRUE;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 */
void AddElementalRing(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (micaster == 0)
		UseMana(misource, SPL_FIRERING);
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miRange = 7;
}

void AddSearch(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	int lvl, i, range;
	MissileStruct *mis, *tmis;

	mis = &missile[mi];
	AutoMapShowItems = TRUE;
	if (misource > 0)
		lvl = plr[misource]._pLevel;
	else
		lvl = 1;
	range = lvl + 10 * spllvl + 245;
	mis->_miRange = range;
	if (micaster == 0)
		UseMana(misource, SPL_SEARCH);

	for (i = 0; i < nummissiles; i++) {
		tmis = &missile[missileactive[i]];
		if (tmis->_miType == MIS_SEARCH && mis != tmis && tmis->_miSource == misource) {
			if (tmis->_miRange < INT_MAX - range)
				tmis->_miRange += range;
			mis->_miDelFlag = TRUE;
			break;
		}
	}
}

/**
 * Var1: light strength
 * Var2: base direction
 * Var3: movement counter
 */
void AddCboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
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
	mis->_miRange = 256;
	mis->_miRnd = random_(63, 15) + 1;
	mis->_miAnimFrame = random_(63, 8) + 1;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddHboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int av = 16;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}

	if (misource != -1) {
		av += spllvl << 1;
		if (av > 63) {
			av = 63;
		}
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
}
#endif

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 * Var3: min damage of the physical arrow
 * Var4: max damage of the physical arrow
 * Var5: min damage of the elemental hit
 * Var6: max damage of the elemental hit
 * Var7: the mitype of the elemental hit
 */
void AddLArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int av = 31, flags;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (micaster == 0) {
		p = &plr[misource];
		flags = p->_pIFlags;
		if (flags & ISPL_RNDARROWVEL)
			av = RandRange(16, 46);
#ifdef HELLFIRE
		if (flags & ISPL_QUICKATTACK)
			av++;
		if (flags & ISPL_FASTATTACK)
			av += 2;
		if (flags & ISPL_FASTERATTACK)
			av += 4;
		if (flags & ISPL_FASTESTATTACK)
			av += 8;

		if (p->_pClass == PC_ROGUE)
			av += p->_pLevel >> 2;
		else if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARD)
			av += p->_pLevel >> 3;
#else
		if (p->_pClass == PC_ROGUE)
			av += p->_pLevel >> 2;
		else if (p->_pClass == PC_WARRIOR)
			av += p->_pLevel >> 3;
#endif
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);

	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 5);
	mis->_miRange = 256;
	if (misource != -1) {
		if (micaster == 0) {
			p = &plr[misource];
			mis->_miVar3 = p->_pIMinDam;
			mis->_miVar4 = p->_pIMaxDam;
			if (mis->_miType == MIS_LARROW) {
				mis->_miVar5 = p->_pILMinDam;
				mis->_miVar6 = p->_pILMaxDam;
			} else { // mis->_miType == MIS_FARROW
				mis->_miVar5 = p->_pIFMinDam;
				mis->_miVar6 = p->_pIFMaxDam;
			}
		} else {
			mis->_miVar3 = monster[misource].mMinDamage;
			mis->_miVar4 = monster[misource].mMaxDamage;
			// BUGFIX: use mMin/MaxDamage(2) of monsters?
			mis->_miVar5 = 1 + currlevel;
			mis->_miVar6 = 10 + currlevel * 2;
		}
	} else {
		mis->_miVar3 = mis->_miVar5 = 1 + currlevel;
		mis->_miVar4 = mis->_miVar6 = 10 + currlevel * 2;
	}
	mis->_miVar7 = mis->_miType == MIS_LARROW ? MFILE_MINILTNG : MFILE_MAGBLOS;
}

/**
 * Var1: min damage of the arrow
 * Var2: max damage of the arrow
 */
void AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int av = 32, flags;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (micaster == 0) {
		p = &plr[misource];
		flags = p->_pIFlags;
		if (flags & ISPL_RNDARROWVEL) {
			av = RandRange(16, 47);
		}
#ifdef HELLFIRE
		if (flags & ISPL_QUICKATTACK)
			av++;
		if (flags & ISPL_FASTATTACK)
			av += 2;
		if (flags & ISPL_FASTERATTACK)
			av += 4;
		if (flags & ISPL_FASTESTATTACK)
			av += 8;

		if (p->_pClass == PC_ROGUE)
			av += (p->_pLevel - 1) >> 2;
		else if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARD)
			av += (p->_pLevel - 1) >> 3;
#else
		if (p->_pClass == PC_ROGUE)
			av += (p->_pLevel - 1) >> 2;
		else if (p->_pClass == PC_WARRIOR)
			av += (p->_pLevel - 1) >> 3;
#endif
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	mis = &missile[mi];
	mis->_miAnimFrame = GetDirection16(sx, sy, dx, dy) + 1;
	mis->_miRange = 256;
	if (misource != -1) {
		if (micaster == 0) {
			mis->_miVar1 = plr[misource]._pIMinDam;
			mis->_miVar2 = plr[misource]._pIMaxDam;
		} else {
			mis->_miVar1 = monster[misource].mMinDamage;
			mis->_miVar2 = monster[misource].mMaxDamage;
		}
	} else {
		mis->_miVar1 = currlevel;
		mis->_miVar2 = 2 * currlevel;
	}
}

void AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int pn, nTries;

	if (micaster == 0 || (dx == 0 && dy == 0)) {
		nTries = 0;
		do {
			nTries++;
			if (nTries > 500) {
				dx = 0;
				dy = 0;
				break; //BUGFIX: warps player to 0/0 in hellfire, change to return or use 1.09's version of the code
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
		} while ((nSolidTable[pn] | dObject[dx][dy] | dMonster[dx][dy]) != 0);
	}

	mis = &missile[mi];
	mis->_miRange = 2;
	mis->_mix = dx;
	mis->_miy = dy;
	if (micaster == 0)
		UseMana(misource, SPL_RNDTELEPORT);
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, mx, av;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (micaster == 0) {
		for (i = 0; i < nummissiles; i++) {
			mx = missileactive[i];
			if (missile[mx]._miType == MIS_GUARDIAN && missile[mx]._miSource == misource && missile[mx]._miVar3 == mi)
				break;
		}
		if (i == nummissiles)
			UseMana(misource, SPL_FIREBOLT);
		if (misource != -1) {
			av = 2 * spllvl + 16;
			if (av > 63)
				av = 63;
		} else {
			av = 16;
		}
	} else {
		av = 26;
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_mitxoff += 3 * mis->_mixvel;
	mis->_mityoff += 3 * mis->_miyvel;
	GetMissilePos(mi);
#ifdef HELLFIRE
	if (mis->_mixvel & 0xFFFF0000 || mis->_miyvel & 0xFFFF0000)
		mis->_miRange = 256;
	else
		mis->_miRange = 1;
#else
	mis->_miRange = 256;
#endif
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
}

void AddKrull(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	missile[mi]._miRange = 256;
	PutMissile(mi);
}

void AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	char *cr;

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
					UseMana(misource, SPL_TELEPORT);
					mis->_miRange = 2;
					return;
				}
			}
		}
	}
	mis->_miDelFlag = TRUE;
}

/**
 * Var1: origin to check for a shrine
 * Var2: origin to check for a shrine
 */
void AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	if (misource >= 0) {
		sx = plr[misource]._px;
		sy = plr[misource]._py;
	}
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miRange = 255;
	mis->_miAnimFrame = random_(63, 8) + 1;
}

/**
 * Var1: range of the wall
 * Var2: light strength
 */
void AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, range;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_miDam = (random_(53, 10) + random_(53, 10) + plr[misource]._pLevel + 2) << 4;
	mis->_miDam >>= 1;
	range = 10;
	i = spllvl;
	if (i > 0)
		range = 10 * (i + 1);
	range += (plr[misource]._pISplDur * range) >> 7;
	range <<= 4;
	mis->_miRange = range;
	mis->_miVar1 = range - mis->_miAnimLen;
	//mis->_miVar2 = 0;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	mis = &missile[mi];
	if (micaster == 0) {
		dam = 2 * (plr[misource]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
		for (i = spllvl; i > 0; i--) {
			dam += dam >> 3;
		}
		i = 2 * spllvl + 16;
		if (i > 50)
			i = 50;
		UseMana(misource, SPL_FIREBALL);
	} else {
		dam = RandRange(monster[misource].mMinDamage, monster[misource].mMaxDamage);
		i = 16;
	}
	mis->_miDam = dam;
	GetMissileVel(mi, sx, sy, dx, dy, i);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 */
void AddLightningC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 32);
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	if (mis->_miDam == 0 && micaster == 0)
		UseMana(misource, SPL_LIGHTNING);
	mis->_miRange = 256;
	mis->_miAnimFrame = random_(52, 8) + 1;
}

void AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
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
	mis->_miAnimFrame = random_(52, 8) + 1;
}

/**
 * Var1: light strength
 */
void AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis, *bmis;

	if (micaster != 0 && misource > 0) {
		switch (monster[misource].MType->mtype) {
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
	mis->_miRange = mis->_miAnimLen;
	//mis->_miVar1 = 0;
}

/**
 * Var1: light strength
 * Var2: min damage
 * Var3: max damage
 */
void AddWeapFExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;

	mis = &missile[mi];
	//mis->_miVar1 = 0;
	p = &plr[misource];
	mis->_miVar2 = p->_pIFMinDam;
	mis->_miVar3 = p->_pIFMaxDam;
	mis->_miDir = 0;
	mis->_miRange = mis->_miAnimLen - 1;
}

/**
 * Var1: light strength
 * Var2: min damage
 * Var3: max damage
 */
void AddWeapLExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;

	mis = &missile[mi];
	//mis->_miVar1 = 0;
	p = &plr[misource];
	mis->_miVar2 = p->_pILMinDam;
	mis->_miVar3 = p->_pILMaxDam;
	mis->_miDir = 0;
	mis->_miRange = mis->_miAnimLen - 1;
}

static BOOL CheckIfTrig(int x, int y)
{
	int i;

	for (i = 0; i < numtrigs; i++) {
		if ((x == trigs[i]._tx && y == trigs[i]._ty) || (abs(trigs[i]._tx - x) < 2 && abs(trigs[i]._ty - y) < 2))
			return TRUE;
	}
	return FALSE;
}

/**
 * Var1: animation
 * Var2: light strength
 */
void AddTown(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis, *bmis;
	int i, j, tx, ty, pn;
	char *cr;

	mis = &missile[mi];
	if (currlevel != 0) {
		mis->_miDelFlag = TRUE;
		for (i = 0; i < 6; i++) {
			cr = &CrawlTable[CrawlNum[i]];
#ifdef HELLFIRE
			for (j = *cr; j > 0; j--) { // BUGFIX: should cast to BYTE or CrawlTable header will be wrong
#else
			for (j = (BYTE)*cr; j > 0; j--) {
#endif
				tx = dx + *++cr;
				ty = dy + *++cr;
				if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
					pn = dPiece[tx][ty];
					if ((dMissile[tx][ty] | nSolidTable[pn] | nMissileTable[pn] | dObject[tx][ty] | dPlayer[tx][ty]) == 0) {
						if (!CheckIfTrig(tx, ty)) {
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
	}
	mis->_mix = mis->_misx = tx;
	mis->_miy = mis->_misy = ty;
	mis->_miRange = 100;
	mis->_miVar1 = mis->_miRange - mis->_miAnimLen;
	//mis->_miVar2 = 0;
	for (i = 0; i < nummissiles; i++) {
		bmis = &missile[missileactive[i]];
		if (bmis->_miType == MIS_TOWN && bmis != mis && bmis->_miSource == misource)
			bmis->_miRange = 0;
	}
	PutMissile(mi);
	if (misource == myplr && !mis->_miDelFlag && currlevel != 0) {
		if (!setlevel) {
			NetSendCmdLocParam3(TRUE, CMD_ACTIVATEPORTAL, tx, ty, currlevel, leveltype, FALSE);
		} else {
			NetSendCmdLocParam3(TRUE, CMD_ACTIVATEPORTAL, tx, ty, setlvlnum, leveltype, TRUE);
		}
	}
}

void AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	mis = &missile[mi];
	if (micaster == 0) {
		if (misource != -1) {
			UseMana(misource, SPL_FLASH);
			dam = 0;
			for (i = plr[misource]._pLevel; i >= 0; i--) {
				dam += random_(55, 20) + 1;
			}
			for (i = spllvl; i > 0; i--) {
				dam += dam >> 3;
			}
			dam += dam >> 1;
		} else {
			dam = currlevel >> 1;
		}
	} else {
		dam = monster[misource].mLevel << 1;
	}
	mis->_miDam = dam;
	mis->_miRange = 19;
}

void AddFlash2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	mis = &missile[mi];
	if (micaster == 0) {
		if (misource != -1) {
			dam = 0;
			for (i = plr[misource]._pLevel; i >= 0; i--) {
				dam += random_(56, 2) + 1;
			}
			for (i = spllvl; i > 0; i--) {
				dam += dam >> 3;
			}
			dam += dam >> 1;
		} else {
			dam = currlevel >> 1;
		}
		mis->_miDam = dam;
	}
	mis->_miPreFlag = TRUE;
	mis->_miRange = 19;
}

/**
 * Var1: last hp
 * Var2: last base hp
 * Var8: earflag
 */
void AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;

	if (micaster == 0)
		UseMana(misource, SPL_MANASHIELD);
	if (misource == myplr)
		NetSendCmd(TRUE, CMD_SETSHIELD);
	mis = &missile[mi];
	p = &plr[misource];
	mis->_miRange = 48 * p->_pLevel;
	mis->_miVar1 = p->_pHitPoints;
	mis->_miVar2 = p->_pHPBase;
	mis->_miVar8 = -1;
	p->pManaShield = TRUE;
}

/**
 * Var1: animation
 * Var2: light strength
 * Var3: x coordinate of the missile-light
 * Var4: y coordinate of the missile-light
 */
void AddFireWave(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mis = &missile[mi];
	mis->_miDam = random_(59, 10) + plr[misource]._pLevel + 1;
	mis->_miRange = 255;
	//mis->_miVar1 = 0;
	//mis->_miVar2 = 0;
	//mis->_miVar3 = 0;
	//mis->_miVar4 = 0;
	mis->_mix++;
	mis->_miy++;
	mis->_miyoff -= 32;
}

/**
 * Var1: animation
 * Var2: animation
 * Var3: light strength
 */
void AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, pn, j, tx, ty, dam, range;
	char *cr;

	mis = &missile[mi];

	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
#ifdef HELLFIRE
		for (j = *cr; j > 0; j--) { // BUGFIX: should cast to BYTE or CrawlTable header will be wrong
#else
		for (j = (BYTE)*cr; j > 0; j--) {
#endif
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

						UseMana(misource, SPL_GUARDIAN);
						dam = random_(62, 10) + (plr[misource]._pLevel >> 1) + 1;
						for (i = spllvl; i > 0; i--) {
							dam += dam >> 3;
						}
						mis->_miDam = dam;

						range = spllvl + (plr[misource]._pLevel >> 1);
						range += (range * plr[misource]._pISplDur) >> 7;
						if (range > 30)
							range = 30;
						range <<= 4;
						if (range < 30)
							range = 30;

						mis->_miRange = range;
						mis->_miVar1 = range - mis->_miAnimLen;
						//mis->_miVar2 = 0;
						mis->_miVar3 = 1;
						return;
					}
				}
			}
		}
	}

	mis->_miDelFlag = TRUE;
}

/**
 * Var1: dx direction of the missile
 * Var2: dy direction of the missile
 */
void AddChain(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	UseMana(misource, SPL_CHAIN);

	mis = &missile[mi];
	mis->_miVar1 = dx;
	mis->_miVar2 = dy;
	mis->_miRange = 1;
}

void miss_null_11(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	SetMissDir(mi, dx);
	mis = &missile[mi];
	mis->_miDam = 0;
	mis->_miLightFlag = TRUE;
	mis->_miRange = 250;
}

void miss_null_12(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (dx > 3)
		dx = 2;
	SetMissDir(mi, dx);
	mis = &missile[mi];
	mis->_miDam = 0;
	mis->_miLightFlag = TRUE;
	mis->_miRange = 250;
}

void miss_null_13(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (dx > 3)
		dx = 2;
	SetMissDir(mi, dx);
	mis = &missile[mi];
	mis->_miDam = 0;
	mis->_miLightFlag = TRUE;
	mis->_miRange = mis->_miAnimLen;
}

void AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	AnimStruct *anim;

	mon = &monster[misource];
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
	mis->_miDir = midir;
	mis->_miAnimFlags = 0;
	mis->_miAnimData = anim->Data[midir];
	mis->_miAnimDelay = anim->Rate;
	mis->_miAnimLen = anim->Frames;
	mis->_miAnimWidth = mon->MType->width;
	mis->_miAnimWidth2 = mon->MType->width2;
	mis->_miAnimAdd = 1;
	if (mon->MType->mtype >= MT_NSNAKE && mon->MType->mtype <= MT_GSNAKE)
		mis->_miAnimFrame = 7;
	mis->_miLightFlag = TRUE;
	if (mon->_uniqtype != 0) {
		mis->_miUniqTrans = mon->_uniqtrans + 1;
		mis->_miLid = mon->mlid;
	}
	mis->_miRange = 256;
	PutMissile(mi);
}

/**
 * Var1: target found
 * Var2: attempts to find a target
 */
void AddFireman(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	AnimStruct *anim;
	MonsterStruct *mon;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	mon = &monster[misource];
	anim = &mon->MType->Anims[MA_WALK];
	mis = &missile[mi];
	mis->_miDir = midir;
	mis->_miAnimFlags = 0;
	mis->_miAnimData = anim->Data[midir];
	mis->_miAnimDelay = anim->Rate;
	mis->_miAnimLen = anim->Frames;
	mis->_miAnimWidth = mon->MType->width;
	mis->_miAnimWidth2 = mon->MType->width2;
	mis->_miAnimAdd = 1;
	//mis->_miVar1 = FALSE;
	//mis->_miVar2 = 0;
	mis->_miLightFlag = TRUE;
	if (mon->_uniqtype != 0)
		mis->_miUniqTrans = mon->_uniqtrans + 1;
	dMonster[mon->_mx][mon->_my] = 0;
	mis->_miRange = 256;
	PutMissile(mi);
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	missile[mi]._miRange = 256;
	missile[mi]._miVar1 = sx;
	missile[mi]._miVar2 = sy;
	missile[mi]._miLid = AddLight(sx, sy, 8);
	if (micaster == 0) {
		UseMana(misource, SPL_FLARE);
		plr[misource]._pHitPoints -= 320;
		plr[misource]._pHPBase -= 320;
		drawhpflag = TRUE;
		if (plr[misource]._pHitPoints <= 0)
			SyncPlrKill(misource, 0);
	} else {
		if (misource > 0) {
			switch (monster[misource].MType->mtype) {
			case MT_SUCCUBUS:
				SetMissAnim(mi, MFILE_FLARE);
				break;
			case MT_SNOWWICH:
				SetMissAnim(mi, MFILE_SCUBMISB);
				break;
			case MT_HLSPWN:
				SetMissAnim(mi, MFILE_SCUBMISD);
				break;
			case MT_SOLBRNR:
				SetMissAnim(mi, MFILE_SCUBMISC);
				break;
			}
		}
	}
#ifdef HELLFIRE
	if (misfiledata[missile[mi]._miAnimType].mfAnimFAmt == 16) {
		SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	}
#endif
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
#ifdef HELLFIRE
	if (mis->_mixvel & 0xFFFF0000 || mis->_miyvel & 0xFFFF0000)
		mis->_miRange = 5 * (monster[misource]._mint + 4);
	else
		mis->_miRange = 1;
#else
	mis->_miRange = 5 * (monster[misource]._mint + 4);
#endif
	//mis->_miLid = -1;
	PutMissile(mi);
}

/**
 * Var1: animation
 * Var2: light strength
 */
void miss_null_1D(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange = 50;
	mis->_miVar1 = mis->_miRange - mis->_miAnimLen;
	//mis->_miVar2 = 0;
}

void AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MonsterStruct *mon;

	mis = &missile[mi];
	mon = &monster[misource];
	mis->_miRange = 40 * (mon->_mint + 1) + random_(50, 15);
	mis->_miLightFlag = TRUE;
	mis->_miPreFlag = TRUE;
}

/**
 * Var1: mmode of the monster
 * Var2: mnum of the monster
 */
void AddStone(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MonsterStruct *mon;
	int i, j, tx, ty, mid, range;
	char *cr;

	mis = &missile[mi];
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
#ifdef HELLFIRE
		for (j = *cr; j > 0; j--) { // BUGFIX: should cast to BYTE or CrawlTable header will be wrong
#else
		for (j = (BYTE)*cr; j > 0; j--) {
#endif
			tx = dx + *++cr;
			ty = dy + *++cr;
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
				mid = dMonster[tx][ty];
				mid = mid >= 0 ? mid - 1 : -1 - mid;
				if (mid < MAX_PLRS)
					continue;
				mon = &monster[mid];
				if (mon->_mAi != AI_DIABLO) {
#ifdef HELLFIRE
					if (mon->MType->mtype == MT_NAKRUL)
						continue;
#endif
					if (mon->_mmode != MM_FADEIN && mon->_mmode != MM_FADEOUT && mon->_mmode != MM_CHARGE) {
						mis->_miVar1 = mon->_mmode;
						mis->_miVar2 = mid;
						mon->_mmode = MM_STONE;
						mis->_mix = tx;
						mis->_miy = ty;
						mis->_misx = mis->_mix;
						mis->_misy = mis->_miy;

						range = spllvl + 6;
						range += (range * plr[misource]._pISplDur) >> 7;
						if (range > 15)
							range = 15;
						range <<= 4;

						mis->_miRange = range;
						UseMana(misource, SPL_STONE);
						return;
					}
				}
			}
		}
	}

	mis->_miDelFlag = TRUE;
}

/**
 * Var1: x coordinate of the source
 * Var2: y coordinate of the source
 * Var4: x coordinate of the destination
 * Var5: y coordinate of the destination
 */
void AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis, *tmis;
	int i;

	mis = &missile[mi];
	for (i = 0; i < nummissiles; i++) {
		tmis = &missile[missileactive[i]];
		if (tmis->_miType == MIS_GOLEM && tmis != mis && tmis->_miSource == misource) {
			mis->_miDelFlag = TRUE;
			return;
		}
	}
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	if ((monster[misource]._mx != 1 || monster[misource]._my) && misource == myplr)
		MonStartKill(misource, misource);
	UseMana(misource, SPL_GOLEM);
}

void AddEtherealize(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int i, range;

	if (micaster == 0)
		UseMana(misource, SPL_ETHEREALIZE);
	mis = &missile[mi];
	p = &plr[misource];
	range = p->_pLevel << 3;
	for (i = spllvl; i > 0; i--) {
		range += range >> 3;
	}
	range += range * p->_pISplDur >> 7;
	mis->_miRange = range;
}

void miss_null_1F(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
}

void miss_null_23(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miDam == 1)
		SetMissDir(mi, 0);
	else
		SetMissDir(mi, 1);
	mis->_miLightFlag = TRUE;
	mis->_miRange = mis->_miAnimLen;
}

/**
 * Var1: target hit
 */
void AddApocaExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	mis->_misx = dx;
	mis->_misy = dy;
	mis->_miRange = mis->_miAnimLen;
	//mis->_miVar1 = FALSE;
}

void AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	PlayerStruct *p;
	int i, HealAmount;

	missile[mi]._miDelFlag = TRUE;

	UseMana(misource, SPL_HEAL);

	p = &plr[misource];
	HealAmount = random_(57, 10) + 1;
	for (i = p->_pLevel; i > 0; i--) {
		HealAmount += random_(57, 4) + 1;
	}
	for (i = spllvl; i > 0; i--) {
		HealAmount += random_(57, 6) + 1;
	}
	HealAmount <<= 6;

	switch (p->_pClass) {
	case PC_WARRIOR: HealAmount <<= 1;            break;
#ifdef HELLFIRE
	case PC_BARBARIAN:
	case PC_MONK:    HealAmount <<= 1;            break;
	case PC_BARD:
#endif
	case PC_ROGUE: HealAmount += HealAmount >> 1; break;
	}

	p->_pHitPoints += HealAmount;
	if (p->_pHitPoints > p->_pMaxHP)
		p->_pHitPoints = p->_pMaxHP;

	p->_pHPBase += HealAmount;
	if (p->_pHPBase > p->_pMaxHPBase)
		p->_pHPBase = p->_pMaxHPBase;

	drawhpflag = TRUE;
}

void AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_HEALOTHER);
	if (misource == myplr) {
		NewCursor(CURSOR_HEALOTHER);
		if (sgbControllerActive)
			TryIconCurs(FALSE);
	}
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 * Var3: destination reached
 * Var4: x coordinate of the destination
 * Var5: y coordinate of the destination
 */
void AddElement(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection8(sx, sy, dx, dy));
	UseMana(misource, SPL_ELEMENT);
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	//mis->_miVar3 = FALSE;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	mis->_miLid = AddLight(sx, sy, 8);
	dam = 2 * (plr[misource]._pLevel + random_(60, 10) + random_(60, 10)) + 4;
	for (i = spllvl; i > 0; i--) {
		dam += dam >> 3;
	}
	dam >>= 1;
	mis->_miDam = dam;
	mis->_miRange = 256;
}

extern void FocusOnInventory();

void AddIdentify(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_IDENTIFY);
	if (misource == myplr) {
		if (sbookflag)
			sbookflag = FALSE;
		if (!invflag) {
			invflag = TRUE;
			if (sgbControllerActive)
				FocusOnInventory();
		}
		NewCursor(CURSOR_IDENTIFY);
	}
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
void AddFirewallC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, j, tx, ty;
	char *cr;

	mis = &missile[mi];
	for (i = 0; i < 6; i++) {
		cr = &CrawlTable[CrawlNum[i]];
#ifdef HELLFIRE
		for (j = *cr; j > 0; j--) { // BUGFIX: should cast to BYTE or CrawlTable header will be wrong
#else
		for (j = (BYTE)*cr; j > 0; j--) {
#endif
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
						mis->_miRange = 7;
						UseMana(misource, SPL_FIREWALL);
						return;
					}
				}
			}
		}
	}

	mis->_miDelFlag = TRUE;
}

void AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, range;

	mis = &missile[mi];
	range = 1584;
	for (i = spllvl; i > 0; i--) {
		range += range >> 3;
	}
	range += range * plr[misource]._pISplDur >> 7;
	mis->_miRange = range;
	if (micaster == 0)
		UseMana(misource, SPL_INFRA);
}

/**
 * Var1: dx coordinate of the wave
 * Var2: dy coordinate of the wave
 */
void AddFireWaveC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miVar1 = dx;
	mis->_miVar2 = dy;
	mis->_miRange = 1;
	mis->_miAnimFrame = 4;
	UseMana(misource, SPL_WAVE);
}

void AddNovaC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i, dam;

	mis = &missile[mi];
	if (misource != -1) {
		if (micaster == 0)
			UseMana(misource, SPL_NOVA);
		dam = (random_(66, 6) + random_(66, 6) + random_(66, 6) + random_(66, 6) + random_(66, 6));
		dam += plr[misource]._pLevel + 5;
		dam >>= 1;
		for (i = spllvl; i > 0; i--) {
			dam += dam >> 3;
		}
	} else {
		dam = ((DWORD)currlevel >> 1) + random_(66, 3) + random_(66, 3) + random_(66, 3);
	}
	mis->_miDam = dam;
	mis->_miRange = 1;
}

#ifdef HELLFIRE
/**
 * Var1: length of the effect
 * Var2: hp penalty
 */
void AddBloodboil(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int lvl;

	mis = &missile[mi];
	p = &plr[misource];
	if (misource == -1 || p->_pSpellFlags & (PSE_BLOOD_BOIL | PSE_LETHARGY) || p->_pHitPoints <= p->_pLevel << 6) {
		mis->_miDelFlag = TRUE;
	} else {
		PlaySfxLoc(sgSFXSets[SFXS_PLR_70][p->_pClass], p->_px, p->_py);
		p->_pSpellFlags |= PSE_BLOOD_BOIL;
		UseMana(misource, SPL_BLODBOIL);
		lvl = p->_pLevel;
		mis->_miVar2 = (3 * lvl) << 7;
		if (misource <= 0)
			lvl = 1;
		mis->_miVar1 = mis->_miRange = lvl + 10 * spllvl + 245;
		CalcPlrItemVals(misource, TRUE);
		force_redraw = 255;
	}
}
#endif

void AddRepair(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_REPAIR);
	if (misource == myplr) {
		if (sbookflag)
			sbookflag = FALSE;
		if (!invflag) {
			invflag = TRUE;
			if (sgbControllerActive)
				FocusOnInventory();
		}
		NewCursor(CURSOR_REPAIR);
	}
}

void AddRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_RECHARGE);
	if (misource == myplr) {
		if (sbookflag)
			sbookflag = FALSE;
		if (!invflag) {
			invflag = TRUE;
			if (sgbControllerActive)
				FocusOnInventory();
		}
		NewCursor(CURSOR_RECHARGE);
	}
}

void AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_DISARM);
	if (misource == myplr) {
		NewCursor(CURSOR_DISARM);
		if (sgbControllerActive) {
			if (pcursobj != -1)
				NetSendCmdLocParam1(TRUE, CMD_DISARMXY, cursmx, cursmy, pcursobj);
			else
				NewCursor(CURSOR_HAND);
		}
	}
}

/**
 * Var2: y0 of the affected area (incremented)
 * Var3: y1 of the affected area
 * Var4: x0 of the affected area (incremented)
 * Var5: x1 of the affected area
 * Var6: x0 of the affected area
 */
void AddApocaC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int i;
	const int RAD = 8;

	mis = &missile[mi];
	mis->_miVar2 = std::max(sy - RAD, 1);
	mis->_miVar3 = std::min(sy + RAD, MAXDUNY - 1);
	mis->_miVar4 = std::max(sx - RAD, 1);
	mis->_miVar5 = std::min(sx + RAD, MAXDUNX - 1);
	mis->_miVar6 = mis->_miVar4;
	for (i = plr[misource]._pLevel; i > 0; i--) {
		mis->_miDam += random_(67, 6) + 1;
	}
	mis->_miRange = 255;
	UseMana(misource, SPL_APOCA);
}

/**
 * Var2: animation timer
 */
void AddFlame(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	MissileStruct *bmis;
	int i, dam;

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
	for (i = mis->_miDam; i > 0; i--) {
		mis->_miVar2 += 5;
	}
	mis->_miRange = mis->_miVar2 + 20;
	if (micaster == 0) {
		dam = random_(79, plr[misource]._pLevel) + random_(79, 2);
		dam = 8 * dam + 16;
		dam += dam >> 1;
	} else {
		dam = RandRange(monster[misource].mMinDamage, monster[misource].mMaxDamage);
	}
	mis->_miDam = dam;
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 * Var3: timer to dissipate
 */
void AddFlameC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 32);
	if (micaster == 0)
		UseMana(misource, SPL_FLAME);
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	//mis->_miVar3 = 0;
	mis->_miRange = 256;
}

/**
 * Var1: light strength
 * Var2: base direction
 * Var3: movement counter
 */
void AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	assert((DWORD)mi < MAXMISSILES);

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
	if (micaster == 0) {
		mis->_miDam = random_(68, plr[misource]._pMagic >> 2) + 1;
	} else {
		mis->_miDam = 15;
	}
	mis->_miRange = 256;
	mis->_miRnd = random_(63, 15) + 1;
	mis->_miAnimFrame = random_(63, 8) + 1;
}

/**
 * Var1: x coordinate of the missile-light
 * Var2: y coordinate of the missile-light
 */
void AddHbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;
	int av = 16;

	UseMana(misource, SPL_HBOLT);

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	if (misource != -1) {
		av += 2 * spllvl;
		if (av > 63) {
			av = 63;
		}
	}
	GetMissileVel(mi, sx, sy, dx, dy, av);
	SetMissDir(mi, GetDirection16(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
	mis->_miDam = random_(69, 10) + plr[misource]._pLevel + 9;
}

void AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_RESURRECT);
	if (misource == myplr) {
		NewCursor(CURSOR_RESURRECT);
		if (sgbControllerActive)
			TryIconCurs(FALSE);
	}
}

void AddResurrectBeam(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_mix = dx;
	mis->_miy = dy;
	mis->_misx = mis->_mix;
	mis->_misy = mis->_miy;
	mis->_miRange = misfiledata[MFILE_RESSUR1].mfAnimLen[0];
}

void AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	missile[mi]._miDelFlag = TRUE;
	UseMana(misource, SPL_TELEKINESIS);
	if (misource == myplr)
		NewCursor(CURSOR_TELEKINESIS);
}

/**
 * Var1: x coordinate of the missile
 * Var2: y coordinate of the missile
 * Var3: destination reached
 * Var4: x coordinate of the destination
 * Var5: y coordinate of the destination
 */
void AddBoneSpirit(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	if (sx == dx && sy == dy) {
		dx += XDirAdd[midir];
		dy += YDirAdd[midir];
	}
	GetMissileVel(mi, sx, sy, dx, dy, 16);
	SetMissDir(mi, GetDirection8(sx, sy, dx, dy));
	mis = &missile[mi];
	mis->_miVar1 = sx;
	mis->_miVar2 = sy;
	mis->_miVar3 = FALSE;
	mis->_miVar4 = dx;
	mis->_miVar5 = dy;
	mis->_miLid = AddLight(sx, sy, 8);
	mis->_miRange = 256;
	if (micaster == 0) {
		UseMana(misource, SPL_BONESPIRIT);
		plr[misource]._pHitPoints -= 384;
		plr[misource]._pHPBase -= 384;
		drawhpflag = TRUE;
		if (plr[misource]._pHitPoints <= 0)
			SyncPlrKill(misource, 0);
	}
}

/**
 * Var1: animation
 * Var2: light strength
 */
void AddRportal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange = 100;
	mis->_miVar1 = 100 - mis->_miAnimLen;
	//mis->_miVar2 = 0;
	PutMissile(mi);
}

void AddApocaC2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl)
{
	PlayerStruct *p;
	int pnum, dam;

	missile[mi]._miDelFlag = TRUE;
	dam = missile[mi]._miDam;
	for (pnum = 0; pnum < gbMaxPlayers; pnum++) {
		p = &plr[pnum];
		if (p->plractive) {
			if (LineClear(sx, sy, p->_pfutx, p->_pfuty)) {
				AddMissile(0, 0, p->_pfutx, p->_pfuty, 0, MIS_EXAPOCA2, micaster, misource, dam, 0);
			}
		}
	}
}

int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, char micaster, int misource, int midam, int spllvl)
{
	MissileStruct *mis;
	MissileData *mds;
	int i, mi;

#ifdef HELLFIRE
	if (nummissiles >= MAXMISSILES - 1)
#else
	if (nummissiles >= MAXMISSILES)
#endif
		return -1;

	if (mitype == MIS_MANASHIELD && plr[misource].pManaShield) {
		if (currlevel != plr[misource].plrlevel)
			return -1;

		for (i = 0; i < nummissiles; i++) {
			mis = &missile[missileactive[i]];
			if (mis->_miType == MIS_MANASHIELD && mis->_miSource == misource)
				return -1;
		}
	}

	mi = missileavail[0];

	missileavail[0] = missileavail[MAXMISSILES - nummissiles - 1];
	missileactive[nummissiles] = mi;
	nummissiles++;

	mis = &missile[mi];
	memset(mis, 0, sizeof(*mis));
	mds = &missiledata[mitype];

	mis->_miType = mitype;
	mis->_miCaster = micaster;
	mis->_miSource = misource;
	mis->_miAnimType = mds->mFileNum;
	mis->_miDrawFlag = mds->mDraw;
	mis->_miSpllvl = spllvl;
	mis->_miDir = midir;
	mis->_mix = sx;
	mis->_miy = sy;
	mis->_misx = sx;
	mis->_misy = sy;
	mis->_miDam = midam;

	if (misfiledata[mis->_miAnimType].mfAnimFAmt < 8)
		SetMissDir(mi, 0);
	else
		SetMissDir(mi, midir);

	mis->_miAnimAdd = 1;
	mis->_miLid = -1;

	if (mds->mlSFX != -1) {
		PlaySfxLoc(mds->mlSFX, mis->_misx, mis->_misy, mds->mlSFXCnt);
	}

	mds->mAddProc(mi, sx, sy, dx, dy, midir, micaster, misource, spllvl);

	return mi;
}

static BOOL Sentfire(int mi, int sx, int sy)
{
	MissileStruct *mis;
	int dir;

	mis = &missile[mi];
	if (LineClear(mis->_mix, mis->_miy, sx, sy)) {
		if (dMonster[sx][sy] > 0 && monster[dMonster[sx][sy] - 1]._mhitpoints >> 6 > 0 && dMonster[sx][sy] - 1 >= MAX_PLRS) {
			dir = GetDirection(mis->_mix, mis->_miy, sx, sy);
			mis->_miVar3 = missileavail[0];
			AddMissile(mis->_mix, mis->_miy, sx, sy, dir, MIS_FIREBOLT, 0, mis->_miSource, mis->_miDam, GetSpellLevel(mis->_miSource, SPL_FIREBOLT));
			SetMissDir(mi, 2);
			mis->_miVar2 = 3;
			return TRUE;
		}
	}

	return FALSE;
}

void MI_Dummy(int mi)
{
	return;
}

void MI_Golem(int mi)
{
	MissileStruct *mis;
	int tx, ty, i, j, src;
	char *cr;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	src = mis->_miSource;
	if (monster[src]._mx == 1 && monster[src]._my == 0) {
		for (i = 0; i < 6; i++) {
			cr = &CrawlTable[CrawlNum[i]];
#ifdef HELLFIRE
			for (j = *cr; j > 0; j--) { // BUGFIX: should cast to BYTE or CrawlTable header will be wrong
#else
			for (j = (BYTE)*cr; j > 0; j--) {
#endif
				tx = mis->_miVar4 + *++cr;
				ty = mis->_miVar5 + *++cr;
				if (0 < tx && tx < MAXDUNX && 0 < ty && ty < MAXDUNY) {
					if (LineClear(mis->_miVar1, mis->_miVar2, tx, ty)) {
						if ((dMonster[tx][ty] | nSolidTable[dPiece[tx][ty]] | dObject[tx][ty] | dPlayer[tx][ty]) == 0) {
							SpawnGolum(src, tx, ty, mi);
							return;
						}
					}
				}
			}
		}
	}
}

void MI_SetManashield(int mi)
{
	ManashieldFlag = TRUE;
}

void MI_LArrow(int mi)
{
	MissileStruct *mis;
	int rst;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miAnimType == mis->_miVar7) {
		ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miAnimFrame + 5);
		CheckMissileCol(mi, mis->_miVar5, mis->_miVar6, FALSE, mis->_mix, mis->_miy, TRUE);
	} else {
		mis->_miDist++;
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);

		if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
			rst = missiledata[mis->_miType].mResist;
			missiledata[mis->_miType].mResist = 0;
			CheckMissileCol(mi, mis->_miVar3, mis->_miVar4, FALSE, mis->_mix, mis->_miy, FALSE);
			missiledata[mis->_miType].mResist = rst;
		}
		if (mis->_miRange == 0) {
			mis->_miDir = 0;
			mis->_mitxoff -= mis->_mixvel;
			mis->_mityoff -= mis->_miyvel;
			GetMissilePos(mi);
			SetMissAnim(mi, mis->_miVar7);
			mis->_miRange = mis->_miAnimLen - 1;
		} else {
			if (mis->_mix != mis->_miVar1 || mis->_miy != mis->_miVar2) {
				mis->_miVar1 = mis->_mix;
				mis->_miVar2 = mis->_miy;
				ChangeLight(mis->_miLid, mis->_miVar1, mis->_miVar2, 5);
			}
		}
	}
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
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
		CheckMissileCol(mi, mis->_miVar1, mis->_miVar2, FALSE, mis->_mix, mis->_miy, FALSE);
	}
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Firebolt(int mi)
{
	MissileStruct *mis;
	int omx, omy;
	int mpnum, mindam, maxdam;

	mis = &missile[mi];
	mis->_miRange--;
	omx = mis->_mitxoff;
	omy = mis->_mityoff;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	if (mis->_mix != mis->_misx || mis->_miy != mis->_misy) {
		mpnum = mis->_miSource;
		if (mpnum != -1) {
			if (mis->_miCaster == 0) {
				switch (mis->_miType) {
				case MIS_FIREBOLT:
					mindam = (plr[mpnum]._pMagic >> 3) + mis->_miSpllvl + 1;
					maxdam = mindam + 9;
					break;
				case MIS_FLARE:
					mindam = maxdam = 3 * mis->_miSpllvl - (plr[mpnum]._pMagic >> 3) + (plr[mpnum]._pMagic >> 1);
					break;
				}
			} else {
				mindam = monster[mpnum].mMinDamage;
				maxdam = monster[mpnum].mMaxDamage;
			}
		} else {
			mindam = currlevel;
			maxdam = mindam + 2 * currlevel - 1;
		}
		CheckMissileCol(mi, mindam, maxdam, FALSE, mis->_mix, mis->_miy, FALSE);
	}
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		mis->_mitxoff = omx;
		mis->_mityoff = omy;
		GetMissilePos(mi);
		switch (mis->_miType) {
		case MIS_FIREBOLT:
		case MIS_MAGMABALL:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXFIRE, mis->_miCaster, mis->_miSource, 0, 0);
			break;
		case MIS_FLARE:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXFLARE, mis->_miCaster, mis->_miSource, 0, 0);
			break;
		case MIS_ACID:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXACID, mis->_miCaster, mis->_miSource, 0, 0);
			break;
#ifdef HELLFIRE
		case MIS_LICH:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXORA1, mis->_miCaster, mis->_miSource, 0, 0);
			break;
		case MIS_PSYCHORB:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXBL2, mis->_miCaster, mis->_miSource, 0, 0);
			break;
		case MIS_NECROMORB:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXRED3, mis->_miCaster, mis->_miSource, 0, 0);
			break;
		case MIS_ARCHLICH:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXYEL2, mis->_miCaster, mis->_miSource, 0, 0);
			break;
		case MIS_BONEDEMON:
			AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_EXBL3, mis->_miCaster, mis->_miSource, 0, 0);
			break;
#endif
		}
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
	int tx, ty, range, oi;

	mis = &missile[mi];
	tx = mis->_miVar1;
	ty = mis->_miVar2;
	mis->_miRange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	range = mis->_miRange;
	if (CheckMissileCol(mi, mis->_miDam, mis->_miDam, FALSE, mis->_mix, mis->_miy, FALSE))
		mis->_miRange = range;
	oi = dObject[tx][ty];
	if (oi != 0 && tx == mis->_mix && ty == mis->_miy) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._otype == OBJ_SHRINEL || object[oi]._otype == OBJ_SHRINER)
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
	CheckMissileCol(mi, mis->_miDam, mis->_miDam, FALSE, mis->_mix, mis->_miy, FALSE);
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Acidpud(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	CheckMissileCol(mi, mis->_miDam, mis->_miDam, TRUE, mis->_mix, mis->_miy, TRUE);
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
	int ExpLight[14] = { 2, 3, 4, 5, 5, 6, 7, 8, 9, 10, 11, 12, 12 };

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == mis->_miVar1) {
		SetMissDir(mi, 1);
		mis->_miAnimFrame = random_(83, 11) + 1;
	}
	if (mis->_miRange == mis->_miAnimLen - 1) {
		SetMissDir(mi, 0);
		mis->_miAnimFrame = 13;
		mis->_miAnimAdd = -1;
	}
	CheckMissileCol(mi, mis->_miDam, mis->_miDam, TRUE, mis->_mix, mis->_miy, TRUE);
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	} else if (mis->_miDir != 0 && mis->_miAnimAdd != -1 && mis->_miVar2 < 12) {
		if (mis->_miVar2 == 0)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, ExpLight[0]);
		else
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}
	PutMissile(mi);
}

void MI_Fireball(int mi)
{
	MissileStruct *mis;
	int dam, mx, my;

	mis = &missile[mi];
	dam = mis->_miDam;
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
			CheckMissileCol(mi, dam, dam, 0, mis->_mix, mis->_miy, 0);
		mx = mis->_mix;
		my = mis->_miy;
		if (mis->_miRange == 0) {
			ChangeLight(mis->_miLid, mx, my, mis->_miAnimFrame);
			if (CheckNoSolid(mx, my))
				CheckMissileCol(mi, dam, dam, 0, mx, my, 1);
			if (CheckNoSolid(mx, my + 1))
				CheckMissileCol(mi, dam, dam, 0, mx, my + 1, 1);
			if (CheckNoSolid(mx, my - 1))
				CheckMissileCol(mi, dam, dam, 0, mx, my - 1, 1);
			if (CheckNoSolid(mx + 1, my))
				CheckMissileCol(mi, dam, dam, 0, mx + 1, my, 1);
			if (CheckNoSolid(mx + 1, my - 1))
				CheckMissileCol(mi, dam, dam, 0, mx + 1, my - 1, 1);
			if (CheckNoSolid(mx + 1, my + 1))
				CheckMissileCol(mi, dam, dam, 0, mx + 1, my + 1, 1);
			if (CheckNoSolid(mx - 1, my))
				CheckMissileCol(mi, dam, dam, 0, mx - 1, my, 1);
			if (CheckNoSolid(mx - 1, my + 1))
				CheckMissileCol(mi, dam, dam, 0, mx - 1, my + 1, 1);
			if (CheckNoSolid(mx - 1, my - 1))
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
	char *cr;

	mis = &missile[mi];
	mis->_miRange--;
	CheckMissileCol(mi, 0, 0, FALSE, mis->_mix, mis->_miy, FALSE);
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
						int mon = AddMonster(tx, ty, mis->_miVar1, 1, TRUE);
						MonStartStand(mon, mis->_miVar1);
						break;
					}
				}
			}
		}
	} else {
		mis->_miDist++;
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
			mid = mid >= 0 ? mid - 1 : -(mid + 1);
			dir = GetDirection(mx, my, monster[mid]._mx, monster[mid]._my);
		} else {
			pid = pid >= 0 ? pid - 1 : -(pid + 1);
			dir = GetDirection(mx, my, plr[pid]._px, plr[pid]._py);
		}
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
		AddMissile(mx, my, mx, my, dir, mis->_miVar1, mis->_miCaster, mis->_miSource, mis->_miDam, mis->_miSpllvl);
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
	if (CheckMissileCol(mi, mis->_miDam, mis->_miDam, TRUE, mis->_mix, mis->_miy, FALSE))
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

void MI_Fireball3(int mi)
{
	MissileStruct *mis;
	int dam, mx, my;

	mis = &missile[mi];
	dam = mis->_miDam;

	if (mis->_miVar7 < 0) {
		int v = 2 * mis->_miVar6;
		mis->_miVar6 = v;
		mis->_miVar7 = v;
		mis->_miDir--;
		if (mis->_miDir < 0)
			mis->_miDir = 7;
	} else {
		mis->_miVar7--;
	}

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
			CheckMissileCol(mi, dam, dam, FALSE, mis->_mix, mis->_miy, FALSE);
		mx = mis->_mix;
		my = mis->_miy;
		if (mis->_miRange == 0) {
			ChangeLight(mis->_miLid, mx, my, mis->_miAnimFrame);
			if (CheckNoSolid(mx, my))
				CheckMissileCol(mi, dam, dam, FALSE, mx, my, TRUE);
			if (CheckNoSolid(mx, my + 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx, my + 1, TRUE);
			if (CheckNoSolid(mx, my - 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx, my - 1, TRUE);
			if (CheckNoSolid(mx + 1, my))
				CheckMissileCol(mi, dam, dam, FALSE, mx + 1, my, TRUE);
			if (CheckNoSolid(mx + 1, my - 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx + 1, my - 1, TRUE);
			if (CheckNoSolid(mx + 1, my + 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx + 1, my + 1, TRUE);
			if (CheckNoSolid(mx - 1, my))
				CheckMissileCol(mi, dam, dam, FALSE, mx - 1, my, TRUE);
			if (CheckNoSolid(mx - 1, my + 1))
				CheckMissileCol(mi, dam, dam, FALSE, mx - 1, my + 1, TRUE);
			if (CheckNoSolid(mx - 1, my - 1))
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
			mis->_miDir = 0;
			SetMissAnim(mi, MFILE_BIGEXP);
			mis->_miRange = mis->_miAnimLen - 1;
		} else if (mx != mis->_miVar1 || my != mis->_miVar2) {
			mis->_miVar1 = mx;
			mis->_miVar2 = my;
			ChangeLight(mis->_miLid, mx, my, 8);
		}
		mis->_miDelFlag = TRUE;
	}

	PutMissile(mi);
}

void MI_LightArrow(int mi)
{
	MissileStruct *mis;
	int dam, mx, my;

	mis = &missile[mi];
	mis->_miRange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);

	mx = mis->_mix;
	my = mis->_miy;
	/// ASSERT: assert((DWORD)mx < MAXDUNX);
	/// ASSERT: assert((DWORD)my < MAXDUNY);
	dam = mis->_miDam;
	if (!nMissileTable[dPiece[mx][my]]) {
		if ((mx != mis->_miVar1 || my != mis->_miVar2) && mx > 0 && my > 0 && mx < MAXDUNX && my < MAXDUNY) {
			AddMissile(
				mx,
				my,
				0,
				0,
				mi,
				mis->_miType == MIS_LIGHTNINGC ? MIS_LIGHTNING : MIS_LIGHTNING2,
				mis->_miCaster,
				mis->_miSource,
				dam,
				mis->_miSpllvl);
			mis->_miVar1 = mis->_mix;
			mis->_miVar2 = mis->_miy;
		}
	} else {
		if (mis->_miSource != -1 || mx != mis->_misx || my != mis->_misy)
			mis->_miRange = 0;
	}

	if (mis->_miRange == 0 || mx <= 0 || my <= 0 || mx >= MAXDUNX || my > MAXDUNY) { // BUGFIX my >= MAXDUNY
		mis->_miDelFlag = TRUE;
	}
}

void MI_Flashfr(int mi)
{
	MissileStruct *mis;
	int pnum;

	mis = &missile[mi];
	pnum = mis->_miSource;
	if (mis->_miCaster == 0 && pnum != -1) {
		mis->_mix = plr[pnum]._px;
		mis->_miy = plr[pnum]._py;
		mis->_mitxoff = plr[pnum]._pxoff << 16;
		mis->_mityoff = plr[pnum]._pyoff << 16;
	}
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		if (mis->_miCaster == 0 && pnum != -1) {
			plr[pnum]._pBaseToBlk -= 50;
		}
	}
	PutMissile(mi);
}

void MI_Flashbk(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	if (mis->_miCaster == 0) {
		if (mis->_miSource != -1) {
			mis->_mix = plr[mis->_miSource]._pfutx;
			mis->_miy = plr[mis->_miSource]._pfuty;
		}
	}
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Reflect(int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;

	mis = &missile[mi];
	p = &plr[mis->_miSource];
	mis->_mitxoff = p->_pxoff << 16;
	mis->_mityoff = p->_pyoff << 16;
	if (p->_pmode == PM_WALK3) {
		mis->_misx = p->_pfutx + 2;
		mis->_misy = p->_pfuty - 1;
	} else {
		mis->_misx = p->_px + 2;
		mis->_misy = p->_py - 1;
	}
	GetMissilePos(mi);
	if (p->_pmode == PM_WALK3) {
		if (p->_pdir == DIR_W)
			mis->_mix++;
		else
			mis->_miy++;
	}
	if (mis->_miSource != myplr && currlevel != p->plrlevel)
		mis->_miDelFlag = TRUE;
	if ((WORD)p->wReflection <= 0) {
		mis->_miDelFlag = TRUE;
		NetSendCmd(TRUE, CMD_ENDREFLECT);
	}
	PutMissile(mi);
}

void MI_FireRing(int mi)
{
	MissileStruct *mis;
	int src, lvl, tx, ty, dam, j, pn;
	char *cr;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	src = mis->_miSource;
	if (src != -1)
		lvl = plr[src]._pLevel;
	else
		lvl = currlevel;
	dam = (random_(53, 10) + random_(53, 10) + lvl + 2) << 3;
	cr = &CrawlTable[CrawlNum[3]];
	for (j = *cr; j > 0; j--) {
		tx = mis->_miVar1 + *++cr;
		ty = mis->_miVar2 + *++cr;
		if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
			pn = dPiece[tx][ty];
			if ((nSolidTable[pn] | dObject[tx][ty]) == 0) {
				if (LineClear(mis->_mix, mis->_miy, tx, ty)) {
					if (nMissileTable[pn])
						break;
					else
						AddMissile(tx, ty, tx, ty, 0, MIS_FIREWALL, mis->_miCaster, src, dam, mis->_miSpllvl);
				}
			}
		}
	}
}

void MI_LightRing(int mi)
{
	MissileStruct *mis;
	int src, lvl, tx, ty, dam, j, pn;
	char *cr;

	mis = &missile[mi];
	mis->_miDelFlag = TRUE;
	src = mis->_miSource;
	if (src != -1)
		lvl = plr[src]._pLevel;
	else
		lvl = currlevel;
	dam = (random_(53, 10) + random_(53, 10) + lvl + 2) << 3;
	cr = &CrawlTable[CrawlNum[3]];
	for (j = *cr; j > 0; j--) {
		tx = mis->_miVar1 + *++cr;
		ty = mis->_miVar2 + *++cr;
		if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY) {
			pn = dPiece[tx][ty];
			if ((nSolidTable[pn] | dObject[tx][ty]) == 0) {
				if (LineClear(mis->_mix, mis->_miy, tx, ty)) {
					if (nMissileTable[pn])
						break;
					else
						AddMissile(tx, ty, tx, ty, 0, MIS_LIGHTWALL, mis->_miCaster, src, dam, mis->_miSpllvl);
				}
			}
		}
	}
}

void MI_Search(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		PlaySfxLoc(IS_CAST7, plr[mis->_miSource]._px, plr[mis->_miSource]._py);
		AutoMapShowItems = FALSE;
	}
}

void MI_LightwallC(int mi)
{
	MissileStruct *mis;
	int src, lvl, dam, tx, ty;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
	} else {
		src = mis->_miSource;
		if (src != -1)
			lvl = plr[src]._pLevel;
		else
			lvl = currlevel;
		dam = 16 * (random_(53, 10) + random_(53, 10) + lvl + 2);
		if (!mis->_miVar8) {
			tx = mis->_miVar1;
			ty = mis->_miVar2;
			if (IN_DUNGEON_AREA(tx, ty) && !nMissileTable[dPiece[tx][ty]]) {
				AddMissile(tx, ty, tx, ty, 0, MIS_LIGHTWALL, mis->_miCaster, src, dam, mis->_miSpllvl);
				mis->_miVar1 += XDirAdd[mis->_miVar3];
				mis->_miVar2 += YDirAdd[mis->_miVar3];
			} else {
				mis->_miVar8 = TRUE;
			}
		}
		if (!mis->_miVar7) {
			tx = mis->_miVar5;
			ty = mis->_miVar6;
			if (IN_DUNGEON_AREA(tx, ty) && !nMissileTable[dPiece[tx][ty]]) {
				AddMissile(tx, ty, tx, ty, 0, MIS_LIGHTWALL, mis->_miCaster, src, dam, mis->_miSpllvl);
				mis->_miVar5 += XDirAdd[mis->_miVar4];
				mis->_miVar6 += YDirAdd[mis->_miVar4];
			} else {
				mis->_miVar7 = TRUE;
			}
		}
	}
}

void MI_FireNovaC(int mi)
{
	MissileStruct *mis;
	int i, sx, sy, cx, cy, caster, source, dam, spllvl;

	mis = &missile[mi];
	caster = mis->_miCaster;
	source = mis->_miSource;
	dam = mis->_miDam;
	spllvl = mis->_miSpllvl;
	sx = mis->_mix;
	sy = mis->_miy;
	cx = 0;
	cy = 0;
	for (i = 0; i < 23; i++) {
		if (cx != vCrawlTable[i][6] || cy != vCrawlTable[i][7]) {
			cx = vCrawlTable[i][6];
			cy = vCrawlTable[i][7];
			AddMissile(sx, sy, sx + cx, sy + cy, 0, MIS_FIREBALL2, caster, source, dam, spllvl);
			AddMissile(sx, sy, sx - cx, sy - cy, 0, MIS_FIREBALL2, caster, source, dam, spllvl);
			AddMissile(sx, sy, sx - cx, sy + cy, 0, MIS_FIREBALL2, caster, source, dam, spllvl);
			AddMissile(sx, sy, sx + cx, sy - cy, 0, MIS_FIREBALL2, caster, source, dam, spllvl);
		}
	}
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
}

void MI_SpecArrow(int mi)
{
	MissileStruct *mis;
	int sx, sy, dx, dy, dir, mitype, caster, source, dam, spllvl;

	mis = &missile[mi];
	caster = mis->_miCaster;
	source = mis->_miSource;
	dam = mis->_miDam;
	sx = mis->_mix;
	sy = mis->_miy;
	dx = mis->_miVar1;
	dy = mis->_miVar2;
	dir = mis->_miVar3;
	mitype = mis->_miVar4;
	spllvl = mis->_miVar5;
	AddMissile(sx, sy, dx, dy, dir, mitype, caster, source, dam, spllvl);
	if (mitype == MIS_CBOLTARROW) {
		AddMissile(sx, sy, dx, dy, dir, mitype, caster, source, dam, spllvl);
		AddMissile(sx, sy, dx, dy, dir, mitype, caster, source, dam, spllvl);
	}
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
}
#endif

void MI_LightningC(int mi)
{
	MissileStruct *mis;
	int dam, mpnum, mx, my;

	assert((DWORD)mi < MAXMISSILES);
	mis = &missile[mi];
	mis->_miRange--;

	mpnum = mis->_miSource;
	if (mpnum != -1) {
		if (mis->_miCaster == 0) {
			dam = (random_(79, 2) + random_(79, plr[mpnum]._pLevel) + 2) << 6;
		} else {
			dam = 2 * RandRange(monster[mpnum].mMinDamage, monster[mpnum].mMaxDamage);
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
	if (!nMissileTable[dPiece[mx][my]]) {
		if ((mx != mis->_miVar1 || my != mis->_miVar2) && mx > 0 && my > 0 && mx < MAXDUNX && my < MAXDUNY) {
			AddMissile(
			    mx,
			    my,
			    0,
			    0,
			    mi,
			    mis->_miType == MIS_LIGHTNINGC ? MIS_LIGHTNING : MIS_LIGHTNING2,
			    mis->_miCaster,
			    mis->_miSource,
			    dam,
			    mis->_miSpllvl);
			mis->_miVar1 = mx;
			mis->_miVar2 = my;
		}
	} else {
		if (mis->_miSource != -1 || mx != mis->_misx || my != mis->_misy)
			mis->_miRange = 0;
	}
	if (mis->_miRange == 0 || mx <= 0 || my <= 0 || mx >= MAXDUNX || my > MAXDUNY) {
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
	if (CheckMissileCol(mi, mis->_miDam, mis->_miDam, TRUE, mis->_mix, mis->_miy, FALSE))
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
	int i;

	mis = &missile[mi];
	if (mis->_miRange > 1)
		mis->_miRange--;
	if (mis->_miRange == mis->_miVar1)
		SetMissDir(mi, 1);
	if (currlevel != 0 && mis->_miDir != 1 && mis->_miRange != 0) {
		if (mis->_miVar2 == 0)
			mis->_miLid = AddLight(mis->_mix, mis->_miy, 1);
		else
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, ExpLight[mis->_miVar2]);
		mis->_miVar2++;
	}

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && currlevel == plr[i].plrlevel && !plr[i]._pLvlChanging && plr[i]._pmode == PM_STAND && plr[i]._px == mis->_mix && plr[i]._py == mis->_miy) {
			ClrPlrPath(i);
			if (i == myplr) {
				NetSendCmdParam1(TRUE, CMD_WARP, mis->_miSource);
				plr[i]._pmode = PM_NEWLVL;
			}
		}
	}

	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_Flash(int mi)
{
	MissileStruct *mis;
	int dam, mx, my;

	mis = &missile[mi];
	if (mis->_miCaster == 0) {
		if (mis->_miSource != -1)
			plr[mis->_miSource]._pInvincible = TRUE;
	}
	mis->_miRange--;
	dam = mis->_miDam;
	mx = mis->_mix;
	my = mis->_miy;
	CheckMissileCol(mi, dam, dam, TRUE, mx - 1, my, TRUE);
	CheckMissileCol(mi, dam, dam, TRUE, mx, my, TRUE);
	CheckMissileCol(mi, dam, dam, TRUE, mx + 1, my, TRUE);
	CheckMissileCol(mi, dam, dam, TRUE, mx - 1, my + 1, TRUE);
	CheckMissileCol(mi, dam, dam, TRUE, mx, my + 1, TRUE);
	CheckMissileCol(mi, dam, dam, TRUE, mx + 1, my + 1, TRUE);
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		if (mis->_miCaster == 0) {
			if (mis->_miSource != -1)
				plr[mis->_miSource]._pInvincible = FALSE;
		}
	}
	PutMissile(mi);
}

void MI_Flash2(int mi)
{
	MissileStruct *mis;
	int dam, mx, my;

	mis = &missile[mi];
	if (mis->_miCaster == 0) {
		if (mis->_miSource != -1)
			plr[mis->_miSource]._pInvincible = TRUE;
	}
	mis->_miRange--;
	dam = mis->_miDam;
	mx = mis->_mix;
	my = mis->_miy;
	CheckMissileCol(mi, dam, dam, TRUE, mx - 1, my - 1, TRUE);
	CheckMissileCol(mi, dam, dam, TRUE, mx, my - 1, TRUE);
	CheckMissileCol(mi, dam, dam, TRUE, mx + 1, my - 1, TRUE);
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		if (mis->_miCaster == 0) {
			if (mis->_miSource != -1)
				plr[mis->_miSource]._pInvincible = FALSE;
		}
	}
	PutMissile(mi);
}

static void MI_Manashield(int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;
	int pnum, diff, div;

	mis = &missile[mi];
	pnum = mis->_miSource;
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
			mis->_miRange = 0;
		diff = mis->_miVar1 - p->_pHitPoints;
		if (diff > 0) {
#ifdef HELLFIRE
			div = 19 - (std::min(mis->_miSpllvl, 8) << 1);
#else
			div = 3;
#endif
			diff -= diff / div;

			drawmanaflag = TRUE;
			drawhpflag = TRUE;

			if (p->_pMana >= diff) {
				p->_pHitPoints = mis->_miVar1;
				p->_pHPBase = mis->_miVar2;
				p->_pMana -= diff;
				p->_pManaBase -= diff;
			} else {
				p->_pHitPoints = p->_pMana + mis->_miVar1 - diff;
				p->_pHPBase = p->_pMana + mis->_miVar2 - diff;
				p->_pMana = 0;
				p->_pManaBase = p->_pMaxManaBase - p->_pMaxMana;
				mis->_miRange = 0;
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
			mis->_miRange = 0;
			mis->_miDelFlag = TRUE;
			SyncPlrKill(pnum, -1);
		}
#endif
		mis->_miVar1 = p->_pHitPoints;
		mis->_miVar2 = p->_pHPBase;
		if (mis->_miRange == 0) {
			mis->_miDelFlag = TRUE;
			NetSendCmd(TRUE, CMD_ENDSHIELD);
		}
	}
	PutMissile(mi);
}

void MI_Etherealize(int mi)
{
	MissileStruct *mis;
	PlayerStruct *p;

	mis = &missile[mi];
	mis->_miRange--;
	p = &plr[mis->_miSource];
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
	if (mis->_miRange == 0 || p->_pHitPoints <= 0) {
		mis->_miDelFlag = TRUE;
		p->_pSpellFlags &= ~PSE_ETHERALIZED;
	} else {
		p->_pSpellFlags |= PSE_ETHERALIZED;
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
		mis->_miAnimFrame = random_(82, 11) + 1;
	}
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	range = mis->_miRange;
	if (CheckMissileCol(mi, mis->_miDam, mis->_miDam, FALSE, mis->_mix, mis->_miy, FALSE))
		mis->_miRange = range;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	if (mis->_miDir != 0 || mis->_miRange == 0) {
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
	int i, j, cx, cy;
	BOOL ex;

	assert((DWORD)mi < MAXMISSILES);

	mis = &missile[mi];
	mis->_miRange--;

	if (mis->_miVar2 > 0) {
		mis->_miVar2--;
	}
	if (mis->_miRange == mis->_miVar1 || mis->_miDir == MFILE_GUARD && mis->_miVar2 == 0) {
		SetMissDir(mi, 1);
	}

	if (!(mis->_miRange % 16)) {
		ex = FALSE;
		for (i = 0; i < 23 && !ex; i++) {
			for (j = 10; j >= 0; j -= 2) {
				cx = vCrawlTable[i][j];
				cy = vCrawlTable[i][j + 1];
				if (cx == 0 && cy == 0)
					continue;
				ex = Sentfire(mi, mis->_mix + cx, mis->_miy + cy);
				if (ex) {
					break;
				}
				ex = Sentfire(mi, mis->_mix - cx, mis->_miy - cy);
				if (ex) {
					break;
				}
				ex = Sentfire(mi, mis->_mix + cx, mis->_miy - cy);
				if (ex) {
					break;
				}
				ex = Sentfire(mi, mis->_mix - cx, mis->_miy + cy);
				if (ex) {
					break;
				}
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
	int sx, sy, src, i, j, rad, tx, ty, dir;
	char *cr;

	mis = &missile[mi];
	src = mis->_miSource;
	sx = mis->_mix;
	sy = mis->_miy;
	dir = GetDirection(sx, sy, mis->_miVar1, mis->_miVar2);
	AddMissile(sx, sy, mis->_miVar1, mis->_miVar2, dir, MIS_LIGHTNINGC, 0, src, 1, mis->_miSpllvl);
	rad = mis->_miSpllvl + 3;
	if (rad > 19)
		rad = 19;
	for (i = 1; i < rad; i++) {
		cr = &CrawlTable[CrawlNum[i]];
#ifdef HELLFIRE
		for (j = *cr; j > 0; j--) { // BUGFIX: should cast to BYTE or CrawlTable header will be wrong
#else
		for (j = (BYTE)*cr; j > 0; j--) {
#endif
			tx = sx + *++cr;
			ty = sy + *++cr;
			if (tx > 0 && tx < MAXDUNX && ty > 0 && ty < MAXDUNY && dMonster[tx][ty] > 0) {
				dir = GetDirection(sx, sy, tx, ty);
				AddMissile(sx, sy, tx, ty, dir, MIS_LIGHTNINGC, 0, src, 1, mis->_miSpllvl);
			}
		}
	}
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
}

void mi_null_11(int mi)
{
	MissileStruct *mis;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	if (mis->_miAnimFrame == mis->_miAnimLen)
		mis->_miPreFlag = TRUE;
	PutMissile(mi);
}

void MI_WeapExp(int mi)
{
	MissileStruct *mis;
	int ExpLight[10] = { 9, 10, 11, 12, 11, 10, 8, 6, 4, 2 };

	mis = &missile[mi];
	mis->_miRange--;
	CheckMissileCol(mi, mis->_miVar2, mis->_miVar3, FALSE, mis->_mix, mis->_miy, FALSE);
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

void MI_Misexp(int mi)
{
	MissileStruct *mis;
#ifdef HELLFIRE
	int ExpLight[] = { 9, 10, 11, 12, 11, 10, 8, 6, 4, 2, 1, 0, 0, 0, 0 };
#else
	int ExpLight[10] = { 9, 10, 11, 12, 11, 10, 8, 6, 4, 2 };
#endif

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
		AddMissile(mis->_mix, mis->_miy, mi, 0, mis->_miDir, MIS_ACIDPUD, 1, mis->_miSource, (monster[mis->_miSource].MData->mLevel >= 2) + 1, mis->_miSpllvl);
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
	if (mis->_miRange <= 0) {
		mis->_miDelFlag = TRUE;
		return;
	}
	p = &plr[mis->_miSource];
	px = p->_px;
	py = p->_py;
	dPlayer[px][py] = 0;
	PlrClrTrans(px, py);

	px = mis->_mix;
	py = mis->_miy;
	p->_px = p->_pfutx = p->_poldx = px;
	p->_py = p->_pfuty = p->_poldy = py;
	PlrDoTrans(px, py);
	dPlayer[px][py] = mis->_miSource + 1;
	if (leveltype != DTYPE_TOWN) {
		ChangeLightXY(p->_plid, px, py);
		ChangeVisionXY(p->_pvid, px, py);
	}
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
	if (mon->_mmode != MM_STONE) {
		mis->_miDelFlag = TRUE;
		return;
	}
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		if (mon->_mhitpoints > 0)
			mon->_mmode = mis->_miVar1;
		else
			AddDead(mis->_miVar2);
		return;
	}

	if (mon->_mhitpoints == 0) {
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
	if (!mis->_miVar1) {
		if (CheckMissileCol(mi, mis->_miDam, mis->_miDam, FALSE, mis->_mix, mis->_miy, TRUE))
			mis->_miVar1 = TRUE;
	}
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
	PutMissile(mi);
}

void MI_Rhino(int mi)
{
	MissileStruct *mis;
	int mix, miy, mix2, miy2, omx, omy, mnum;

	mis = &missile[mi];
	mnum = mis->_miSource;
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
	if (mis->_miLid != -1)
		ChangeLightXY(mis->_miLid, omx, omy);
	MoveMissilePos(mi);
	PutMissile(mi);
}

void MI_Fireman(int mi)
{
	MissileStruct *mis;
	int mnum, enemy, ax, ay, bx, by, cx, cy, j;

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
	enemy = monster[mnum]._menemy;
	if (!(monster[mnum]._mFlags & MFLAG_TARGETS_MONSTER)) {
		cx = plr[enemy]._px;
		cy = plr[enemy]._py;
	} else {
		cx = monster[enemy]._mx;
		cy = monster[enemy]._my;
	}
	if ((bx != ax || by != ay) && (mis->_miVar1 && (abs(ax - cx) >= 4 || abs(ay - cy) >= 4) || mis->_miVar2 > 1) && PosOkMonst(mnum, ax, ay)) {
		MissToMonst(mi, ax, ay);
		mis->_miDelFlag = TRUE;
	} else if (!(monster[mnum]._mFlags & MFLAG_TARGETS_MONSTER)) {
		j = dPlayer[bx][by];
	} else {
		j = dMonster[bx][by];
	}
	if (!PosOkMissile(bx, by) || j > 0 && !mis->_miVar1) {
		mis->_mixvel *= -1;
		mis->_miyvel *= -1;
		mis->_miDir = opposite[mis->_miDir];
		mis->_miAnimData = monster[mnum].MType->Anims[MA_WALK].Data[mis->_miDir];
		mis->_miVar2++;
		if (j > 0)
			mis->_miVar1 = TRUE;
	}
	MoveMissilePos(mi);
	PutMissile(mi);
}

void MI_FirewallC(int mi)
{
	MissileStruct *mis;
	int tx, ty, pnum;

	mis = &missile[mi];
	pnum = mis->_miSource;
	mis->_miRange--;
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
	} else {
		if (!mis->_miVar8) {
			tx = mis->_miVar1;
			ty = mis->_miVar2;
			if (IN_DUNGEON_AREA(tx, ty) && !nMissileTable[dPiece[tx][ty]]) {
				AddMissile(tx, ty, tx, ty, plr[pnum]._pdir, MIS_FIREWALL, 0, pnum, 0, mis->_miSpllvl);
				mis->_miVar1 += XDirAdd[mis->_miVar3];
				mis->_miVar2 += YDirAdd[mis->_miVar3];
			} else {
				mis->_miVar8 = TRUE;
			}
		}
		if (!mis->_miVar7) {
			tx = mis->_miVar5;
			ty = mis->_miVar6;
			if (IN_DUNGEON_AREA(tx, ty) && !nMissileTable[dPiece[tx][ty]]) {
				AddMissile(tx, ty, tx, ty, plr[pnum]._pdir, MIS_FIREWALL, 0, pnum, 0, mis->_miSpllvl);
				mis->_miVar5 += XDirAdd[mis->_miVar4];
				mis->_miVar6 += YDirAdd[mis->_miVar4];
			} else {
				mis->_miVar7 = TRUE;
			}
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
		CalcPlrItemVals(mis->_miSource, TRUE);
	} else {
		plr[mis->_miSource]._pInfraFlag = TRUE;
	}
}

void MI_ApocaC(int mi)
{
	MissileStruct *mis;
	int i, j, pnum;

	mis = &missile[mi];
	for (j = mis->_miVar2; j < mis->_miVar3; j++) {
		for (i = mis->_miVar4; i < mis->_miVar5; i++) {
			if (dMonster[i][j] > MAX_PLRS && !nSolidTable[dPiece[i][j]]) {
#ifdef HELLFIRE
				if (!LineClear(mis->_mix, mis->_miy, i, j))
					continue;
#endif
				pnum = mis->_miSource;
				AddMissile(i, j, i, j, plr[pnum]._pdir, MIS_EXAPOCA, 0, pnum, mis->_miDam, 0);
				mis->_miVar2 = j;
				mis->_miVar4 = i + 1;
				return;
			}
		}
		mis->_miVar4 = mis->_miVar6;
	}

	mis->_miDelFlag = TRUE;
}

void MI_FireWaveC(int mi)
{
	MissileStruct *mis;
	int sx, sy, sd, nx, ny, dir;
	int i, j, pnum;

	assert((DWORD)mi < MAXMISSILES);

	mis = &missile[mi];
	pnum = mis->_miSource;
	sx = mis->_mix;
	sy = mis->_miy;
	sd = GetDirection(sx, sy, mis->_miVar1, mis->_miVar2);
	sx += XDirAdd[sd];
	sy += YDirAdd[sd];
	if (!nMissileTable[dPiece[sx][sy]]) {
		AddMissile(sx, sy, sx + XDirAdd[sd], sy + YDirAdd[sd], plr[pnum]._pdir, MIS_FIREWAVE, 0, pnum, 0, mis->_miSpllvl);

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
				AddMissile(nx, ny, nx + XDirAdd[sd], ny + YDirAdd[sd], plr[pnum]._pdir, MIS_FIREWAVE, 0, pnum, 0, mis->_miSpllvl);
			}
		}
	}
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
}

void MI_LightNovaC(int mi)
{
	MissileStruct *mis;
	int i, sx, sy, cx, cy, micaster, misource, midam, spllvl;

	mis = &missile[mi];
	micaster = mis->_miCaster;
	misource = mis->_miSource;
	midam = mis->_miDam;
	spllvl = mis->_miSpllvl;
	sx = mis->_mix;
	sy = mis->_miy;
	cx = 0;
	cy = 0;
	for (i = 0; i < 23; i++) {
		if (cx != vCrawlTable[i][6] || cy != vCrawlTable[i][7]) {
			cx = vCrawlTable[i][6];
			cy = vCrawlTable[i][7];
			AddMissile(sx, sy, sx + cx, sy + cy, 0, MIS_LIGHTBALL, micaster, misource, midam, spllvl);
			AddMissile(sx, sy, sx - cx, sy - cy, 0, MIS_LIGHTBALL, micaster, misource, midam, spllvl);
			AddMissile(sx, sy, sx - cx, sy + cy, 0, MIS_LIGHTBALL, micaster, misource, midam, spllvl);
			AddMissile(sx, sy, sx + cx, sy - cy, 0, MIS_LIGHTBALL, micaster, misource, midam, spllvl);
		}
	}
	mis->_miRange--;
	if (mis->_miRange == 0)
		mis->_miDelFlag = TRUE;
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
		CalcPlrItemVals(pnum, TRUE);
		p->_pHitPoints -= hpdif;
		if (p->_pHitPoints < 64)
			p->_pHitPoints = 64;
		force_redraw = 255;
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
	if (CheckMissileCol(mi, mis->_miDam, mis->_miDam, TRUE, mis->_mix, mis->_miy, FALSE))
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
			AddMissile(
			    mis->_mix,
			    mis->_miy,
			    0,
			    0,
			    mi,
			    MIS_FLAME,
			    mis->_miCaster,
			    mis->_miSource,
			    mis->_miVar3,
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

void MI_Cbolt(int mi)
{
	MissileStruct *mis;
	int md;
	int bpath[16] = { -1, 0, 1, -1, 0, 1, -1, -1, 0, 0, 1, 1, 0, 1, -1, 0 };

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miAnimType != MFILE_LGHNING) {
		if (mis->_miVar3 == 0) {
			md = (mis->_miVar2 + bpath[mis->_miRnd]) & 7;
			mis->_miRnd = (mis->_miRnd + 1) & 0xF;
			GetMissileVel(mi, mis->_mix, mis->_miy, mis->_mix + XDirAdd[md], mis->_miy + YDirAdd[md], 8);
			mis->_miVar3 = 16;
		} else {
			mis->_miVar3--;
		}
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		if (CheckMissileCol(mi, mis->_miDam, mis->_miDam, FALSE, mis->_mix, mis->_miy, FALSE)) {
			mis->_miVar1 = 8;
			mis->_miDir = 0;
			mis->_mixoff = 0;
			mis->_miyoff = 0;
			SetMissAnim(mi, MFILE_LGHNING);
			mis->_miRange = mis->_miAnimLen;
			GetMissilePos(mi);
		}
		ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miVar1);
	}
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_Hbolt(int mi)
{
	MissileStruct *mis;
	int mx, my;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miAnimType != MFILE_HOLYEXPL) {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		mx = mis->_mix;
		my = mis->_miy;
		if (mx != mis->_misx || my != mis->_misy) {
			CheckMissileCol(mi, mis->_miDam, mis->_miDam, FALSE, mx, my, FALSE);
		}
		if (mis->_miRange == 0) {
			mis->_mitxoff -= mis->_mixvel;
			mis->_mityoff -= mis->_miyvel;
			GetMissilePos(mi);
			mis->_miDir = 0;
			SetMissAnim(mi, MFILE_HOLYEXPL);
			mis->_miRange = mis->_miAnimLen - 1;
		} else {
			if (mx != mis->_miVar1 || my != mis->_miVar2) {
				mis->_miVar1 = mx;
				mis->_miVar2 = my;
				ChangeLight(mis->_miLid, mx, my, 8);
			}
		}
	} else {
		if (mis->_miRange == 0) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_miLid);
		} else {
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miAnimFrame + 7);
		}
	}
	PutMissile(mi);
}

void MI_Element(int mi)
{
	MissileStruct *mis;
	int mid, sd, cx, cy;

	mis = &missile[mi];
	mis->_miRange--;
	mis->_mitxoff += mis->_mixvel;
	mis->_mityoff += mis->_miyvel;
	GetMissilePos(mi);
	cx = mis->_mix;
	cy = mis->_miy;
	CheckMissileCol(mi, mis->_miDam, mis->_miDam, FALSE, cx, cy, FALSE);
	if (!mis->_miVar3 && cx == mis->_miVar4 && cy == mis->_miVar5) {
		mis->_miVar3 = TRUE;
		mis->_miRange = 255;
		mid = FindClosest(cx, cy, 19);
		if (mid > 0) {
			sd = GetDirection8(cx, cy, monster[mid]._mx, monster[mid]._my);
			SetMissDir(mi, sd);
			GetMissileVel(mi, cx, cy, monster[mid]._mx, monster[mid]._my, 16);
		} else {
			sd = plr[mis->_miSource]._pdir;
			SetMissDir(mi, sd);
			GetMissileVel(mi, cx, cy, cx + XDirAdd[sd], cy + YDirAdd[sd], 16);
		}
	}
	if (cx != mis->_miVar1 || cy != mis->_miVar2) {
		mis->_miVar1 = cx;
		mis->_miVar2 = cy;
		ChangeLight(mis->_miLid, cx, cy, 8);
	}
	if (mis->_miRange == 0) {
		AddMissile(mis->_mix, mis->_miy, mi, 0, 0, MIS_EXELE, mis->_miCaster, mis->_miSource, mis->_miDam, 0);
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_EleExp(int mi)
{
	MissileStruct *mis;
	int dam, cx, cy;

	mis = &missile[mi];
	cx = mis->_mix;
	cy = mis->_miy;
	if (mis->_miVar1++ == 0)
		mis->_miLid = AddLight(cx, cy, mis->_miAnimFrame);
	else
		ChangeLight(mis->_miLid, cx, cy, mis->_miAnimFrame);
	mis->_miRange--;
	dam = mis->_miDam;
	if (CheckNoSolid(cx, cy))
		CheckMissileCol(mi, dam, dam, TRUE, cx, cy, TRUE);
	if (CheckNoSolid(cx, cy + 1))
		CheckMissileCol(mi, dam, dam, TRUE, cx, cy + 1, TRUE);
	if (CheckNoSolid(cx, cy - 1))
		CheckMissileCol(mi, dam, dam, TRUE, cx, cy - 1, TRUE);
	if (CheckNoSolid(cx + 1, cy))
		CheckMissileCol(mi, dam, dam, TRUE, cx + 1, cy, TRUE); /* check x/y */
	if (CheckNoSolid(cx + 1, cy - 1))
		CheckMissileCol(mi, dam, dam, TRUE, cx + 1, cy - 1, TRUE);
	if (CheckNoSolid(cx + 1, cy + 1))
		CheckMissileCol(mi, dam, dam, TRUE, cx + 1, cy + 1, TRUE);
	if (CheckNoSolid(cx - 1, cy))
		CheckMissileCol(mi, dam, dam, TRUE, cx - 1, cy, TRUE);
	if (CheckNoSolid(cx - 1, cy + 1))
		CheckMissileCol(mi, dam, dam, TRUE, cx - 1, cy + 1, TRUE);
	if (CheckNoSolid(cx - 1, cy - 1))
		CheckMissileCol(mi, dam, dam, TRUE, cx - 1, cy - 1, TRUE);
	if (mis->_miRange == 0) {
		mis->_miDelFlag = TRUE;
		AddUnLight(mis->_miLid);
	}
	PutMissile(mi);
}

void MI_Bonespirit(int mi)
{
	MissileStruct *mis;
	int mid, sd;
	int cx, cy;

	mis = &missile[mi];
	mis->_miRange--;
	if (mis->_miDir == DIR_OMNI) {
		if (mis->_miRange == 0) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_miLid);
		} else {
			ChangeLight(mis->_miLid, mis->_mix, mis->_miy, mis->_miAnimFrame);
		}
	} else {
		mis->_mitxoff += mis->_mixvel;
		mis->_mityoff += mis->_miyvel;
		GetMissilePos(mi);
		cx = mis->_mix;
		cy = mis->_miy;
		CheckMissileCol(mi, 0, 0, FALSE, cx, cy, FALSE);
		if (!mis->_miVar3 && cx == mis->_miVar4 && cy == mis->_miVar5) {
			mis->_miVar3 = TRUE;
			mis->_miRange = 255;
			mid = FindClosest(cx, cy, 19);
			if (mid > 0) {
				SetMissDir(mi, GetDirection8(cx, cy, monster[mid]._mx, monster[mid]._my));
				GetMissileVel(mi, cx, cy, monster[mid]._mx, monster[mid]._my, 16);
			} else {
				sd = plr[mis->_miSource]._pdir;
				SetMissDir(mi, sd);
				GetMissileVel(mi, cx, cy, cx + XDirAdd[sd], cy + YDirAdd[sd], 16);
			}
		}
		if (cx != mis->_miVar1 || cy != mis->_miVar2) {
			mis->_miVar1 = cx;
			mis->_miVar2 = cy;
			ChangeLight(mis->_miLid, cx, cy, 8);
		}
		if (mis->_miRange == 0) {
			SetMissDir(mi, DIR_OMNI);
			mis->_miRange = 7;
		}
	}
	PutMissile(mi);
}

void MI_ResurrectBeam(int mi)
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
	} else if (currlevel != 0 && mis->_miDir != 1) {
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
			i = 0;
		} else {
			i++;
		}
	}

	MissilePreFlag = FALSE;
	ManashieldFlag = FALSE;

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		mis = &missile[mi];
		missiledata[mis->_miType].mProc(mi);
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
			if (missile[missileactive[i]]._miType == MIS_MANASHIELD) {
				MI_Manashield(missileactive[i]);
			}
		}
	}

	for (i = 0; i < nummissiles; ) {
		mi = missileactive[i];
		if (missile[mi]._miDelFlag) {
			DeleteMissile(mi, i);
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
		mis->_miAnimData = misfiledata[mis->_miAnimType].mfAnimData[mis->_miDir];
		if (mis->_miType == MIS_RHINO) {
			mon = monster[mis->_miSource].MType;
			if (mon->mtype >= MT_HORNED && mon->mtype <= MT_OBLORD) {
				anim = &mon->Anims[MA_SPECIAL];
			} else {
				if (mon->mtype >= MT_NSNAKE && mon->mtype <= MT_GSNAKE)
					anim = &mon->Anims[MA_ATTACK];
				else
					anim = &mon->Anims[MA_WALK];
			}
			mis->_miAnimData = anim->Data[mis->_miDir];
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
