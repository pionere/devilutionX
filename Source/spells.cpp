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
	}

	sl = plr[pnum]._pSplLvl[sn] + plr[pnum]._pISplLvlAdd - 1;
	if (sl < 0)
		sl = 0;
	adj = sl * spelldata[sn].sManaAdj;
	adj >>= 1;
	ma -= adj;
	mm = spelldata[sn].sMinMana;
	if (mm > ma)
		ma = mm;
	ma <<= 6;

	//return ma * (100 - plr[pnum]._pISplCost) / 100;
	return ma;
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
			ma = 0; // REMOVEME GetManaAmount(pnum, sn);
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
			} else {
				is = &plr[pnum].SpdList[sf - INVITEM_BELT_FIRST];
			}
			return is->_itype != ITYPE_NONE && is->_iSpell == sn && (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE);
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

int GetSpellLevel(int pnum, int sn)
{
	int result;

	result = plr[pnum]._pISplLvlAdd + plr[pnum]._pSplLvl[sn];
	if (result < 0)
		result = 0;
	return result;
}

BOOL CheckSpell(int pnum, int sn)
{
#ifdef _DEBUG
	if (debug_mode_key_inverted_v)
		return TRUE;
#endif

	return GetSpellLevel(pnum, sn) > 0 && plr[pnum]._pMana >= GetManaAmount(pnum, sn);
}

DEVILUTION_END_NAMESPACE
