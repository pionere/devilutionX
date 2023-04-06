/**
 * @file drlg_l1.h
 *
 * Interface of the cathedral level generation algorithms.
 */
#ifndef __DRLG_L1_H__
#define __DRLG_L1_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void DRLG_InitL1Specials(int x1, int y1, int x2, int y2);
#ifdef HELLFIRE
void DRLG_InitL5Specials(int x1, int y1, int x2, int y2);
#endif
void DRLG_L1InitTransVals();
void LoadL1Dungeon(const LevelData* lds);
void CreateL1Dungeon();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLG_L1_H__ */
