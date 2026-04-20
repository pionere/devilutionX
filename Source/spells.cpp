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

void SpellCheck(PlrSkillUse* skill)
{
	int8_t result = SPLFROM_INVALID_TYPE;
	int sn = skill->_suSkill;
	int pnum = mypnum;
	if (sn != SPL_NULL && (spelldata[sn].sUseFlags & myplr._pSkillFlags) == spelldata[sn].sUseFlags) {
		switch (skill->_suType) {
		case RSPLTYPE_ABILITY:
			// assert(spelldata[sn].sManaCost == 0);
			result = SPLFROM_ABILITY;
			break;
		case RSPLTYPE_SPELL:
			result = plr._pSkillLvl[sn] > 0 ? (plr._pMana >= GetManaAmount(pnum, sn) ? SPLFROM_MANA : SPLFROM_INVALID_MANA) : SPLFROM_INVALID_LEVEL;
			break;
		case RSPLTYPE_CHARGES:
			result = InvGetChargeIdx(pnum, sn);
			static_assert(!SPLFROM_INVALID(INVITEM_BODY_FIRST), "SpellCheck expects the BODY indices to be distinct from SPLFROM_INVALID I.");
			static_assert(!SPLFROM_INVALID(INVITEM_BODY_LAST), "SpellCheck expects the BODY indices to be distinct from SPLFROM_INVALID II.");
			static_assert((int)INVITEM_BODY_FIRST > (int)SPLFROM_MANA || (int)INVITEM_BODY_LAST < (int)SPLFROM_MANA, "SpellCheck expects the BODY indices to be distinct from SPL_MANA.");
			static_assert((int)INVITEM_BODY_FIRST > (int)SPLFROM_ABILITY || (int)INVITEM_BODY_LAST < (int)SPLFROM_MANA, "SpellCheck expects the BODY indices to be distinct from SPLFROM_ABILITY.");
			if ((BYTE)result == INVITEM_NONE)
				result = SPLFROM_INVALID_SOURCE;
			break;
		case RSPLTYPE_INVALID:
			result = SPLFROM_INVALID_TYPE;
			break;
		default:
			result = SPLFROM_ABILITY;
			ASSUME_UNREACHABLE
			break;
		}
	}
	if (SPLFROM_INVALID(result)) {
		skill->_suSkill = SPL_NULL;
	}
	skill->_suType = (BYTE)result;
}

DEVILUTION_END_NAMESPACE
