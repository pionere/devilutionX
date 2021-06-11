/**
 * @file questdat.h
 *
 * Interface of all quest and level data.
 */
#ifndef __QUESTDAT_H__
#define __QUESTDAT_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern const LevelDataStruct AllLevels[NUM_LEVELS];
extern const int Qtalklist[STORE_TOWNERS][NUM_QUESTS];
extern const QuestData questlist[NUM_QUESTS];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __QUESTDAT_H__ */
