/**
 * @file spells.h
 *
 * Interface of functionality for casting player spells.
 */
#ifndef __SPELLS_H__
#define __SPELLS_H__

DEVILUTION_BEGIN_NAMESPACE

#define SPELL_MASK(sn)     ((uint64_t)1 << ((sn) - 1))
#define SPLFROM_INVALID(x) ((int8_t)(x) <= SPLFROM_INVALID_SOURCE)
#define SPL_ABI_MASK       (SPELL_MASK(SPL_WALK) | SPELL_MASK(SPL_BLOCK) | SPELL_MASK(SPL_ATTACK) | SPELL_MASK(SPL_RATTACK))

#ifdef HELLFIRE
static_assert((int)SPL_RUNESTONE + 1 == (int)NUM_SPELLS, "SPELL_RUNE expects ordered spell_id enum");
#define SPELL_RUNE(sn) ((sn) >= SPL_RUNEFIRE)
#else
#define SPELL_RUNE(sn) (FALSE)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int GetManaAmount(int pnum, int sn);
int8_t SpellSourceInv(int sn);
int8_t SpellSourceEquipment(int sn);
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
