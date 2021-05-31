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

void UseMana(int pnum, int sn, int sf)
{
	int ma; // mana cost

	if (pnum != mypnum)
		return;

	if (sf == SPLFROM_MANA) {
#ifdef _DEBUG
		if (debug_mode_key_inverted_v)
			return;
#endif
		ma = GetManaAmount(pnum, sn);
		plr._pMana -= ma;
		plr._pManaBase -= ma;
		plr._pSkillActivity[sn] = std::min((ma >> (6 + 1)) + plr._pSkillActivity[sn], UCHAR_MAX);
		gbRedrawFlags |= REDRAW_MANA_FLASK;
	} else if (sf != SPLFROM_ABILITY) {
		NetSendCmdBParam1(true, CMD_USEPLRITEM, sf);
	}
}

bool HasMana(int pnum, int sn, int sf)
{
	ItemStruct *is;

	if (sf == SPLFROM_MANA) {
		if (pnum != mypnum)
			return true;
#ifdef _DEBUG
		if (debug_mode_key_inverted_v)
			return true;
#endif
		return plr._pMana >= GetManaAmount(pnum, sn);
	}
	if (sf != SPLFROM_ABILITY) {
		static_assert((int)NUM_INVLOC == (int)INVITEM_INV_FIRST, "Equipped items must preceed INV items in HasMana.");
		static_assert(INVITEM_INV_FIRST < INVITEM_BELT_FIRST, "INV items must preceed BELT items in HasMana.");
		if (sf < INVITEM_INV_FIRST) {
			is = &plr.InvBody[sf];
			return is->_itype != ITYPE_NONE && is->_iSpell == sn && is->_iCharges > 0;
		} else if (sf < INVITEM_BELT_FIRST) {
			is = &plr.InvList[sf - INVITEM_INV_FIRST];
		} else {
			is = &plr.SpdList[sf - INVITEM_BELT_FIRST];
		}
		return is->_itype != ITYPE_NONE && is->_iSpell == sn && (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE);
	}
	return true;
}

int SpellSourceInv(int sn)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	p = &myplr;
	static_assert((int)INVITEM_INV_FIRST > (int)SPLFROM_INVALID || (int)INVITEM_INV_LAST < (int)SPLFROM_INVALID, "SpellSourceInv expects the INV indices to be distinct from SPLFROM_INVALID.");
	static_assert((int)INVITEM_INV_FIRST > (int)SPLFROM_MANA || (int)INVITEM_INV_LAST < (int)SPLFROM_MANA, "SpellSourceInv expects the INV indices to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_INV_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_INV_LAST < (int)SPLFROM_ABILITY, "SpellSourceInv expects the INV indices to be distinct from SPLFROM_ABILITY.");
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++) {
		if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL && pi->_iSpell == sn)
			return INVITEM_INV_FIRST + i;
	}
	static_assert((int)INVITEM_BELT_FIRST > (int)SPLFROM_INVALID || (int)INVITEM_BELT_LAST < (int)SPLFROM_INVALID, "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_INVALID.");
	static_assert((int)INVITEM_BELT_FIRST > (int)SPLFROM_MANA || (int)INVITEM_BELT_LAST < (int)SPLFROM_MANA, "SpellSourceInv expects the BELT indices to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_BELT_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_BELT_LAST < (int)SPLFROM_ABILITY, "SpellSourceInv expects the BELT indices to be distinct from SPLFROM_ABILITY.");
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

	static_assert((int)INVITEM_HAND_LEFT != (int)SPLFROM_INVALID, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPLFROM_INVALID.");
	static_assert((int)INVITEM_HAND_LEFT != (int)SPLFROM_MANA, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPL_MANA.");
	static_assert((int)INVITEM_HAND_LEFT != (int)SPLFROM_ABILITY, "SpellSourceEquipment expects the LEFT_HAND index to be distinct from SPLFROM_ABILITY.");
	pi = &myplr.InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iSpell == sn && pi->_iCharges > 0) {
		return INVITEM_HAND_LEFT;
	}

	return SPLFROM_INVALID;
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
