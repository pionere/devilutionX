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

	sl = plr._pSkillLvl[sn] - 1;
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

int8_t SpellSourceInv(int sn)
{

	static_assert(!SPLFROM_INVALID(INVITEM_INV_FIRST), "SpellSourceInv expects the INV indices to be distinct from SPLFROM_INVALID I.");
	static_assert(!SPLFROM_INVALID(INVITEM_INV_LAST), "SpellSourceInv expects the INV indices to be distinct from SPLFROM_INVALID II.");
	static_assert((int)INVITEM_INV_FIRST > (int)SPLFROM_MANA || (int)INVITEM_INV_LAST < (int)SPLFROM_MANA, "SpellSourceInv expects the INV indices to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_INV_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_INV_LAST < (int)SPLFROM_ABILITY, "SpellSourceInv expects the INV indices to be distinct from SPLFROM_ABILITY.");

	static_assert(!SPLFROM_INVALID(INVITEM_BELT_FIRST), "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_INVALID I.");
	static_assert(!SPLFROM_INVALID(INVITEM_BELT_LAST), "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_INVALID II.");
	static_assert((int)INVITEM_BELT_FIRST > (int)SPLFROM_MANA || (int)INVITEM_BELT_LAST < (int)SPLFROM_MANA, "SpellSourceInv expects the BELT indices to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_BELT_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_BELT_LAST < (int)SPLFROM_ABILITY, "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_ABILITY.");

	int idx = InvGetScrollIdx(mypnum, sn);
	return idx != INVITEM_NONE ? idx : SPLFROM_INVALID_SOURCE;
}

int8_t SpellSourceEquipment(int sn)
{
	static_assert(!SPLFROM_INVALID(INVITEM_BODY_FIRST), "SpellSourceEquipment expects the BODY indices to be distinct from SPLFROM_INVALID I.");
	static_assert(!SPLFROM_INVALID(INVITEM_BODY_LAST), "SpellSourceEquipment expects the BODY indices to be distinct from SPLFROM_INVALID II.");
	static_assert((int)INVITEM_BODY_FIRST > (int)SPLFROM_MANA || (int)INVITEM_BODY_LAST < (int)SPLFROM_MANA, "SpellSourceEquipment expects the BODY indices to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_BODY_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_BODY_LAST < (int)SPLFROM_MANA, "SpellSourceEquipment expects the BODY indices to be distinct from SPLFROM_ABILITY.");

	int idx = InvGetChargeIdx(mypnum, sn);
	return idx != INVITEM_NONE ? idx : SPLFROM_INVALID_SOURCE;
}

int8_t SpellSourceMem(int sn)
{
	int pnum = mypnum;
	return plr._pSkillLvl[sn] > 0 ? (plr._pMana >= GetManaAmount(pnum, sn) ? SPLFROM_MANA : SPLFROM_INVALID_MANA) : SPLFROM_INVALID_LEVEL;
}

DEVILUTION_END_NAMESPACE
