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
		result = skill->_suFrom;
		switch (result) {
		case SPLFROM_ABILITY:
			// assert(spelldata[sn].sManaCost == 0);
			break;
		case SPLFROM_MANA:
			result = plr._pSkillLvl[sn] > 0 ? (plr._pMana >= GetManaAmount(pnum, sn) ? SPLFROM_MANA : SPLFROM_INVALID_MANA) : SPLFROM_INVALID_LEVEL;
			break;
		case SPLFROM_INVALID_TYPE:
			break;
		default: {
			const ItemStruct* pi = &plr._pInvBody[result];
			if (pi->_itype == ITYPE_NONE || pi->_iSpell != sn || !pi->_iStatFlag)
				result = SPLFROM_INVALID_SOURCE;
		} break;
		}
	}
	if (SPLFROM_INVALID(result)) {
		skill->_suSkill = SPL_NULL;
	}
	skill->_suFrom = result;
}

DEVILUTION_END_NAMESPACE
