/**
 * @file spells.cpp
 *
 * Implementation of functionality for casting player spells.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int GetManaAmount(int pnum, int sn)
{
	// mana amount, spell level, adjustment, min mana
	int ma, sl, adj, mm;

	ma = spelldata[sn].sManaCost;
	if (sn == SPL_HEAL || sn == SPL_HEALOTHER) {
		ma += 2 * plr[pnum]._pLevel;
	} else if (ma == 255) {
		ma = (BYTE)plr[pnum]._pMaxManaBase;
	}

	sl = plr[pnum]._pSplLvl[sn] + plr[pnum]._pISplLvlAdd - 1;
	if (sl < 0)
		sl = 0;
	if (sn == SPL_RESURRECT) {
		adj = sl * (ma >> 3);
	} else {
		adj = sl * spelldata[sn].sManaAdj;
		if (sn == SPL_FIREBOLT) {
			adj >>= 1;
		}
	}
	ma -= adj;
	if (ma < 0)
		ma = 0;
	ma <<= 6;

	if (plr[pnum]._pClass == PC_ROGUE)
		ma -= ma >> 2;
#ifdef HELLFIRE
	else if (plr[pnum]._pClass == PC_SORCERER)
		ma >>= 1;
	else if (plr[pnum]._pClass == PC_MONK || plr[pnum]._pClass == PC_BARD)
		ma -= ma >> 2;
#endif

	mm = spelldata[sn].sMinMana << 6;
	if (mm > ma)
		ma = mm;

	return ma * (100 - plr[pnum]._pISplCost) / 100;
}

void UseMana(int pnum, int sn)
{
	int ma; // mana cost

	if (pnum == myplr) {
		switch (plr[pnum]._pSplType) {
		case RSPLTYPE_SKILL:
		case RSPLTYPE_INVALID:
			break;
		case RSPLTYPE_SCROLL:
			RemoveScroll(pnum);
			break;
		case RSPLTYPE_CHARGES:
			UseStaffCharge(pnum);
			break;
		case RSPLTYPE_SPELL:
#ifdef _DEBUG
			if (!debug_mode_key_inverted_v) {
#endif
				ma = GetManaAmount(pnum, sn);
				plr[pnum]._pMana -= ma;
				plr[pnum]._pManaBase -= ma;
				drawmanaflag = TRUE;
#ifdef _DEBUG
			}
#endif
			break;
		}
	}
}

BOOL CheckSpell(int pnum, int sn, char st, BOOL manaonly)
{
	BOOL result;

#ifdef _DEBUG
	if (debug_mode_key_inverted_v)
		return TRUE;
#endif

	result = TRUE;
	if (!manaonly && pcurs != CURSOR_HAND) {
		result = FALSE;
	} else {
		if (st != RSPLTYPE_SKILL) {
			if (GetSpellLevel(pnum, sn) <= 0) {
				result = FALSE;
			} else {
				result = plr[pnum]._pMana >= GetManaAmount(pnum, sn);
			}
		}
	}

	return result;
}

void CastSpell(int mpnum, int sn, int sx, int sy, int dx, int dy, int caster, int spllvl)
{
	int i;
	int dir; // missile direction

	if (caster == 0) {
#ifdef HELLFIRE
		if (sn == SPL_FIREWALL || sn == SPL_LIGHTWALL)
#else
		if (sn == SPL_FIREWALL)
#endif
			dir = plr[mpnum]._pVar3;
		else
			dir = plr[mpnum]._pdir;
	} else {
		dir = monster[mpnum]._mdir;
	}

	for (i = 0; spelldata[sn].sMissiles[i] != MIS_ARROW && i < 3; i++) {
		AddMissile(sx, sy, dx, dy, dir, spelldata[sn].sMissiles[i], caster, mpnum, 0, spllvl);
	}

	if (spelldata[sn].sMissiles[0] == MIS_TOWN) {
		UseMana(mpnum, SPL_TOWN);
	}
	if (spelldata[sn].sMissiles[0] == MIS_CBOLT) {
		UseMana(mpnum, SPL_CBOLT);

		for (i = (spllvl >> 1) + 3; i > 0; i--) {
			AddMissile(sx, sy, dx, dy, dir, MIS_CBOLT, caster, mpnum, 0, spllvl);
		}
	}
}

static void PlacePlayer(int pnum)
{
	int i, nx, ny, x, y;
	BOOL done;

	if (plr[pnum].plrlevel == currlevel) {
		for (i = 0; i < 8; i++) {
			nx = plr[pnum]._px + plrxoff2[i];
			ny = plr[pnum]._py + plryoff2[i];

			if (PosOkPlayer(pnum, nx, ny)) {
				break;
			}
		}

		if (i == 8) {
			done = FALSE;

			for (i = 1; i < 50 && !done; i++) {
				for (y = -i; y <= i && !done; y++) {
					ny = plr[pnum]._py + y;

					for (x = -i; x <= i && !done; x++) {
						nx = plr[pnum]._px + x;

						if (PosOkPlayer(pnum, nx, ny)) {
							done = TRUE;
						}
					}
				}
			}
		}

		plr[pnum]._px = nx;
		plr[pnum]._py = ny;

		dPlayer[nx][ny] = pnum + 1;

		if (pnum == myplr) {
			ViewX = nx;
			ViewY = ny;
		}
	}
}

/**
 * @param pnum player index
 * @param tnum target player index
 */
void DoResurrect(int pnum, int tnum)
{
	PlayerStruct *tp;
	int hp;

	if ((DWORD)tnum >= MAX_PLRS)
		return;

	tp = &plr[tnum];
	AddMissile(tp->_px, tp->_py, tp->_px, tp->_py, 0, MIS_RESURRECTBEAM, 0, pnum, 0, 0);

	if (tp->_pHitPoints == 0) {
		if (tnum == myplr) {
			deathflag = FALSE;
			gamemenu_off();
			drawhpflag = TRUE;
			drawmanaflag = TRUE;
		}

		ClrPlrPath(tnum);
		tp->destAction = ACTION_NONE;
		tp->_pInvincible = FALSE;
#ifndef HELLFIRE
		PlacePlayer(tnum);
#endif

		hp = 640;
#ifndef HELLFIRE
		if (tp->_pMaxHPBase < 640) {
			hp = tp->_pMaxHPBase;
		}
#endif
		SetPlayerHitPoints(tnum, hp);

		tp->_pHPBase = tp->_pHitPoints + (tp->_pMaxHPBase - tp->_pMaxHP);
		tp->_pMana = 0;
		tp->_pManaBase = tp->_pMana + (tp->_pMaxManaBase - tp->_pMaxMana);

		CalcPlrInv(tnum, TRUE);

		if (tp->plrlevel == currlevel) {
			PlrStartStand(tnum, tp->_pdir);
		} else {
			tp->_pmode = PM_STAND;
		}
	}
}

void DoHealOther(int pnum, int tnum)
{
	PlayerStruct *tp;
	int i, hp;

	if ((DWORD)tnum >= MAX_PLRS)
		return;

	tp = &plr[tnum];
	if ((tp->_pHitPoints >> 6) <= 0)
		return; // too late, the target is dead

	hp = random_(57, 10) + 1;

	for (i = plr[pnum]._pLevel; i > 0; i--) {
		hp += random_(57, 4) + 1;
	}

	for (i = GetSpellLevel(pnum, SPL_HEALOTHER); i > 0; i--) {
		hp += random_(57, 6) + 1;
	}
	hp <<= 6;

	switch (plr[pnum]._pClass) {
	case PC_WARRIOR: hp <<= 1;    break;
#ifdef HELLFIRE
	case PC_MONK: hp *= 3;        break;
	case PC_BARBARIAN: hp <<= 1;  break;
	case PC_BARD:
#endif
	case PC_ROGUE: hp += hp >> 1; break;
	}
	tp->_pHitPoints += hp;
	if (tp->_pHitPoints > tp->_pMaxHP) {
		tp->_pHitPoints = tp->_pMaxHP;
	}

	tp->_pHPBase += hp;
	if (tp->_pHPBase > tp->_pMaxHPBase) {
		tp->_pHPBase = tp->_pMaxHPBase;
	}

	drawhpflag = TRUE;
}

DEVILUTION_END_NAMESPACE
