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

#define SPELL_NA		-1
/* Minimum level requirement of a book. */
#define BOOK_MIN		 1
/* Minimum level requirement of a staff. */
#define STAFF_MIN		 2
/* Minimum level requirement of a scroll. */
#define SCRL_MIN		 1

extern SpellData spelldata[NUM_SPELLS];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SPELLDAT_H__ */
