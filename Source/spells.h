/**
 * @file spells.h
 *
 * Interface of functionality for casting player spells.
 */
#ifndef __SPELLS_H__
#define __SPELLS_H__

DEVILUTION_BEGIN_NAMESPACE

#define SPELL_MASK(x)			((uint64_t)1 << (x - 1))

#ifdef __cplusplus
extern "C" {
#endif

int GetManaAmount(int pnum, int sn);
void UseMana(int pnum, int sn, int sf);
BOOL HasMana(int pnum, int sn, int sf);
int SpellSourceInv(int sn);
int SpellSourceEquipment(int sn);
int GetSpellLevel(int pnum, int sn);
BOOL CheckSpell(int pnum, int sn);
BOOL PlacePlayer(int pnum);
void DoResurrect(int pnum, int tnum);
void DoHealOther(int pnum, int tnum, int spllvl);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SPELLS_H__ */
