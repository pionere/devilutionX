/**
 * @file spells.h
 *
 * Interface of functionality for casting player spells.
 */
#ifndef __SPELLS_H__
#define __SPELLS_H__

DEVILUTION_BEGIN_NAMESPACE

#define SPELL_MASK(x)			((uint64_t)1 << (x - 1))
#define SPLFROM_INVALID(x)		((char)x <= SPLFROM_INVALID_SOURCE)

#ifdef __cplusplus
extern "C" {
#endif

int GetManaAmount(int pnum, int sn);
char SpellSourceInv(int sn);
char SpellSourceEquipment(int sn);
bool CheckSpell(int pnum, int sn);

inline void IncreasePlrSkillLvl(int pnum, int sn)
{
	plr._pMemSkills |= SPELL_MASK(sn);
	plr._pSkillLvlBase[sn]++;
	CalcPlrItemVals(pnum, false);
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SPELLS_H__ */
