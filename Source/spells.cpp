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

void UseMana(int pnum, int sn, int sf)
{
	ItemStruct *is;
	int ma; // mana cost

	if (pnum == myplr) {
		if (sf == SPLFROM_MANA) {
#ifdef _DEBUG
			if (debug_mode_key_inverted_v)
				return;
#endif
			ma = GetManaAmount(pnum, sn);
			plr[pnum]._pMana -= ma;
			plr[pnum]._pManaBase -= ma;
			gbRedrawFlags |= REDRAW_MANA_FLASK;
		} else if (sf != SPLFROM_SKILL) {
			static_assert(NUM_INVLOC == INVITEM_INV_FIRST, "Equipped items must preceed INV items in UseMana.");
			static_assert(INVITEM_INV_FIRST < INVITEM_BELT_FIRST, "INV items must preceed BELT items in UseMana.");
			if (sf < INVITEM_INV_FIRST) {
				is = &plr[pnum].InvBody[sf];
				assert(is->_itype != ITYPE_NONE);
				assert(is->_iSpell == sn);
				assert(is->_iCharges > 0);
				is->_iCharges--;
				CalcPlrStaff(pnum);
			} else if (sf < INVITEM_BELT_FIRST) {
				is = &plr[pnum].InvList[sf - INVITEM_INV_FIRST];
				assert(is->_itype != ITYPE_NONE);
				assert(is->_iSpell == sn);
				assert(is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE);
				RemoveInvItem(pnum, sf - INVITEM_INV_FIRST);
			} else {
				is = &plr[pnum].SpdList[sf - INVITEM_BELT_FIRST];
				assert(is->_itype != ITYPE_NONE);
				assert(is->_iSpell == sn);
				assert(is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE);
				RemoveSpdBarItem(pnum, sf - INVITEM_BELT_FIRST);
			}
		}
	}
}

BOOL HasMana(int pnum, int sn, int sf)
{
	ItemStruct *is;

	if (pnum == myplr) {
		if (sf == SPLFROM_MANA) {
#ifdef _DEBUG
			if (debug_mode_key_inverted_v)
				return TRUE;
#endif
			return plr[pnum]._pMana >= GetManaAmount(pnum, sn);
		} else if (sf != SPLFROM_SKILL) {
			static_assert(NUM_INVLOC == INVITEM_INV_FIRST, "Equipped items must preceed INV items in HasMana.");
			static_assert(INVITEM_INV_FIRST < INVITEM_BELT_FIRST, "INV items must preceed BELT items in HasMana.");
			if (sf < INVITEM_INV_FIRST) {
				is = &plr[pnum].InvBody[sf];
				return is->_itype != ITYPE_NONE && is->_iSpell == sn && is->_iCharges > 0;
			} else if (sf < INVITEM_BELT_FIRST) {
				is = &plr[pnum].InvList[sf - INVITEM_INV_FIRST];
				return is->_itype != ITYPE_NONE && is->_iSpell == sn && (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE);
			} else {
				is = &plr[pnum].SpdList[sf - INVITEM_BELT_FIRST];
				return is->_itype != ITYPE_NONE && is->_iSpell == sn && (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE);
			}
		}
	}
	return TRUE;
}

int SpellSourceInv(int sn)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	p = &plr[myplr];
	static_assert(INVITEM_INV_FIRST > SPLFROM_INVALID || INVITEM_INV_LAST < SPLFROM_INVALID, "SpellSourceInv expects the INV indices to be distinct from SPLFROM_INVALID.");
	static_assert(INVITEM_INV_FIRST > SPLFROM_MANA || INVITEM_INV_LAST < SPLFROM_MANA, "SpellSourceInv expects the INV indices to be distinct from SPL_MANA.");
	static_assert(INVITEM_INV_FIRST > SPLFROM_SKILL || INVITEM_INV_LAST < SPLFROM_SKILL, "SpellSourceInv expects the INV indices to be distinct from SPLFROM_SKILL.");
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++) {
		if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL && pi->_iSpell == sn)
			return INVITEM_INV_FIRST + i;
	}
	static_assert(INVITEM_BELT_FIRST > SPLFROM_INVALID || INVITEM_BELT_LAST < SPLFROM_INVALID, "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_INVALID.");
	static_assert(INVITEM_BELT_FIRST > SPLFROM_MANA || INVITEM_BELT_LAST < SPLFROM_MANA, "SpellSourceInv expects the BELT indices to be distinct from SPL_MANA.");
	static_assert(INVITEM_BELT_FIRST > SPLFROM_SKILL || INVITEM_BELT_LAST < SPLFROM_SKILL, "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_SKILL.");
	pi = p->SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++) {
		if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL && pi->_iSpell == sn)
			return INVITEM_BELT_FIRST + i;
	}

	return SPLFROM_INVALID;
}

int SpellSourceEquipment(int sn)
{
	ItemStruct *pi;

	static_assert(INVITEM_HAND_LEFT != SPLFROM_INVALID, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPLFROM_INVALID.");
	static_assert(INVITEM_HAND_LEFT != SPLFROM_MANA, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPL_MANA.");
	static_assert(INVITEM_HAND_LEFT != SPLFROM_SKILL, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPLFROM_SKILL.");
	pi = &plr[myplr].InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iSpell == sn && pi->_iCharges > 0) {
		return INVITEM_HAND_LEFT;
	}

	return SPLFROM_INVALID;
}

BOOL CheckSpell(int pnum, int sn)
{
#ifdef _DEBUG
	if (debug_mode_key_inverted_v)
		return TRUE;
#endif

	return GetSpellLevel(pnum, sn) > 0 && plr[pnum]._pMana >= GetManaAmount(pnum, sn);
}

/*
 * @brief Find a place for the given player starting from its current location.
 *
 * TODO: In the original code it was possible to auto-townwarp after resurrection.
 *       The new solution prevents this, but in some cases it could be useful
 *       (in some cases it is annoying).
 *
 * @return TRUE if the player had to be displaced.
 */
BOOL PlacePlayer(int pnum)
{
	int i, nx, ny, x, y;
	BOOL done;

	for (i = 0; i < lengthof(plrxoff2); i++) {
		nx = plr[pnum]._px + plrxoff2[i];
		ny = plr[pnum]._py + plryoff2[i];

		if (PosOkPlayer(pnum, nx, ny) && PosOkPortal(nx, ny)) {
			break;
		}
	}

	if (i == 0)
		return FALSE;

	if (i == lengthof(plrxoff2)) {
		done = FALSE;

		for (i = 1; i < 50 && !done; i++) {
			for (y = -i; y <= i && !done; y++) {
				ny = plr[pnum]._py + y;

				for (x = -i; x <= i && !done; x++) {
					nx = plr[pnum]._px + x;

					if (PosOkPlayer(pnum, nx, ny) && PosOkPortal(nx, ny)) {
						done = TRUE;
					}
				}
			}
		}
	}

	plr[pnum]._px = nx;
	plr[pnum]._py = ny;
	return TRUE;
}

/**
 * @param pnum player index
 * @param tnum target player index
 */
void DoResurrect(int pnum, int tnum)
{
	PlayerStruct *tp;

	if ((DWORD)tnum >= MAX_PLRS)
		return;

	tp = &plr[tnum];
	AddMissile(tp->_px, tp->_py, tp->_px, tp->_py, 0, MIS_RESURRECTBEAM, 0, pnum, 0, 0);

	if (tp->_pHitPoints == 0) {
		if (tnum == myplr) {
			deathflag = FALSE;
			gamemenu_off();
		}

		ClrPlrPath(tnum);
		tp->destAction = ACTION_NONE;
		tp->_pInvincible = FALSE;

		PlrSetHp(tnum, std::min(10 << 6, tp->_pMaxHPBase));
		PlrSetMana(tnum, 0);

		CalcPlrInv(tnum, TRUE);

		if (tp->plrlevel == currlevel) {
			PlacePlayer(tnum);
			PlrStartStand(tnum, tp->_pdir);
		} else {
			tp->_pmode = PM_STAND;
		}
	}
}

void DoHealOther(int pnum, int tnum, int spllvl)
{
	int i, hp;

	if ((DWORD)tnum >= MAX_PLRS)
		return;

	if (plr[tnum]._pHitPoints < (1 << 6))
		return; // too late, the target is dead

	hp = RandRange(1, 10);

	for (i = plr[pnum]._pLevel; i > 0; i--) {
		hp += RandRange(1, 4);
	}

	for (i = spllvl; i > 0; i--) {
		hp += RandRange(1, 6);
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
	case PC_SORCERER: break;
	default:
		ASSUME_UNREACHABLE
	}
	PlrIncHp(tnum, hp);
}

DEVILUTION_END_NAMESPACE
