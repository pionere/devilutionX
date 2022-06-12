/**
 * @file spelldat.h
 *
 * Interface of all spell data.
 */
#ifndef __SPELLDAT_H__
#define __SPELLDAT_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define SPELL_NA		0
/* Minimum level requirement of a book. */
#define BOOK_MIN		1
/* Minimum level requirement of a staff. */
#define STAFF_MIN		2
/* Minimum level requirement of a scroll. */
#define SCRL_MIN		1
/* Minimum level requirement of a rune. */
#define RUNE_MIN		1
/* The cooldown period of the rage skill. */
#define RAGE_COOLDOWN_TICK		1200

extern const SpellData spelldata[NUM_SPELLS];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SPELLDAT_H__ */
