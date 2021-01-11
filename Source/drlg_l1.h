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

#ifdef HELLFIRE
extern int UberRow;
extern int UberCol;
extern BOOL IsUberRoomOpened;
extern int UberLeverRow;
extern int UberLeverCol;
extern BOOL IsUberLeverActivated;
extern int UberDiabloMonsterIndex;
#endif

void LoadL1Dungeon(const char *sFileName, int vx, int vy);
void LoadPreL1Dungeon(const char *sFileName);
void CreateL1Dungeon(DWORD rseed, int entry);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DRLG_L1_H__ */
