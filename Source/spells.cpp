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
		ma += 2 * plr._pLevel;
	}

	sl = plr._pSkillLvl[sn] + plr._pISplLvlAdd - 1;
	if (sl < 0)
		sl = 0;
	adj = sl * spelldata[sn].sManaAdj;
	adj >>= 1;
	ma -= adj;
	mm = spelldata[sn].sMinMana;
	if (mm > ma)
		ma = mm;
	ma <<= 6;

	//return ma * (100 - plr._pISplCost) / 100;
	return ma;
}

char SpellSourceInv(int sn)
{
	ItemStruct *pi;
	int i;

	static_assert(!SPLFROM_INVALID(INVITEM_INV_FIRST), "SpellSourceInv expects the INV indices to be distinct from SPLFROM_INVALID I.");
	static_assert(!SPLFROM_INVALID(INVITEM_INV_LAST), "SpellSourceInv expects the INV indices to be distinct from SPLFROM_INVALID II.");
	static_assert((int)INVITEM_INV_FIRST > (int)SPLFROM_MANA || (int)INVITEM_INV_LAST < (int)SPLFROM_MANA, "SpellSourceInv expects the INV indices to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_INV_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_INV_LAST < (int)SPLFROM_ABILITY, "SpellSourceInv expects the INV indices to be distinct from SPLFROM_ABILITY.");
	pi = myplr._pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
		if (pi->_iMiscId == IMISC_SCROLL && pi->_iSpell == sn
		 && pi->_itype != ITYPE_NONE && pi->_itype != ITYPE_PLACEHOLDER)
			return INVITEM_INV_FIRST + i;
	}
	static_assert(!SPLFROM_INVALID(INVITEM_BELT_FIRST), "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_INVALID I.");
	static_assert(!SPLFROM_INVALID(INVITEM_BELT_LAST), "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_INVALID II.");
	static_assert((int)INVITEM_BELT_FIRST > (int)SPLFROM_MANA || (int)INVITEM_BELT_LAST < (int)SPLFROM_MANA, "SpellSourceInv expects the BELT indices to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_BELT_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_BELT_LAST < (int)SPLFROM_ABILITY, "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_ABILITY.");
	pi = myplr._pSpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++) {
		if (pi->_iMiscId == IMISC_SCROLL && pi->_iSpell == sn
		 && pi->_itype != ITYPE_NONE)
			return INVITEM_BELT_FIRST + i;
	}

	return SPLFROM_INVALID_SOURCE;
}

char SpellSourceEquipment(int sn)
{
	ItemStruct *pi;

	static_assert(!SPLFROM_INVALID(INVITEM_HAND_LEFT), "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPLFROM_INVALID.");
	static_assert((int)INVITEM_HAND_LEFT != (int)SPLFROM_MANA, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_HAND_LEFT != (int)SPLFROM_ABILITY, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPLFROM_ABILITY.");
	pi = &myplr._pInvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iSpell == sn && pi->_iCharges > 0) {
		return INVITEM_HAND_LEFT;
	}

	return SPLFROM_INVALID_SOURCE;
}

int GetSpellLevel(int pnum, int sn)
{
	int result;

	result = plr._pISplLvlAdd + plr._pSkillLvl[sn];
	if (result < 0)
		result = 0;
	return result;
}

bool CheckSpell(int pnum, int sn)
{
#ifdef _DEBUG
	if (debug_mode_key_inverted_v)
		return true;
#endif

	return GetSpellLevel(pnum, sn) > 0 && plr._pMana >= GetManaAmount(pnum, sn);
}

DEVILUTION_END_NAMESPACE
