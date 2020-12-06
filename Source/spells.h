/**
 * @file spells.h
 *
 * Interface of functionality for casting player spells.
 */
#ifndef __SPELLS_H__
#define __SPELLS_H__

DEVILUTION_BEGIN_NAMESPACE

#define SPELL_MASK(x)			((__int64)1 << (x - 1))

#ifdef __cplusplus
extern "C" {
#endif

int GetManaAmount(int pnum, int sn);
void UseMana(int pnum, int sn);
BOOL CheckSpell(int pnum, int sn, char st, BOOL manaonly);
void CastSpell(int mpnum, int sn, int sx, int sy, int dx, int dy, int caster, int spllvl);
BOOL PlacePlayer(int pnum);
void DoResurrect(int pnum, int tnum);
void DoHealOther(int pnum, int tnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SPELLS_H__ */
