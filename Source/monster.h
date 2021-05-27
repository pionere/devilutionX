/**
 * @file monster.h
 *
 * Interface of monster functionality, AI, actions, spawning, loading, etc.
 */
#ifndef __MONSTER_H__
#define __MONSTER_H__

DEVILUTION_BEGIN_NAMESPACE

#define MINION_INACTIVE(x)			((x->_mx | x->_my) == 0)
#define MINION_NR_INACTIVE(x)		((monster[x]._mx | monster[x]._my) == 0)
#define OPPOSITE(x)					(((x) + 4) & 7)
/** Maps from direction to the opposite direction. */
//const int opposite[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };

#ifdef __cplusplus
extern "C" {
#endif

extern int monstactive[MAXMONSTERS];
extern int nummonsters;
extern MonsterStruct monster[MAXMONSTERS];
extern CMonster Monsters[MAX_LVLMTYPES];
extern int nummtypes;

void InitLevelMonsters();
void GetLevelMTypes();
void InitMonsterGFX(int midx);
#ifdef HELLFIRE
void WakeUberDiablo();
#endif
void InitMonsters();
void SetMapMonsters(BYTE *pMap, int startx, int starty);
int AddMonster(int x, int y, int dir, int mtype, bool InMap);
bool MonTalker(int mnum);
void MonStartStand(int mnum, int md);
void MonClearSquares(int mnum);
void MonGetKnockback(int mnum);
void MonStartHit(int mnum, int pnum, int dam);
void MonStartKill(int mnum, int pnum);
void MonSyncStartKill(int mnum, int x, int y, int pnum);
void MonUpdateLeader(int mnum);
void DoEnding();
void PrepDoEnding(bool soundOn);
void MonWalkDir(int mnum, int md);
void DeleteMonsterList();
void ProcessMonsters();
void FreeMonsters();
bool DirOK(int mnum, int mdir);
bool PosOkMissile(int x, int y);
bool CheckNoSolid(int x, int y);
bool LineClearF(bool (*Clear)(int, int), int x1, int y1, int x2, int y2);
bool LineClear(int x1, int y1, int x2, int y2);
bool LineClearF1(bool (*Clear)(int, int, int), int mnum, int x1, int y1, int x2, int y2);
void SyncMonsterAnim(int mnum);
void MissToMonst(int mnum, int x, int y);
bool PosOkMonst(int mnum, int x, int y);
bool PosOkMonst2(int mnum, int x, int y);
bool PosOkMonst3(int mnum, int x, int y);
bool IsSkel(int mt);
bool IsGoat(int mt);
void SpawnSkeleton(int mnum, int x, int y, int dir);
int PreSpawnSkeleton();
void TalktoMonster(int mnum, int pnum);
void SpawnGolum(int mnum, int x, int y, int level);
bool CanTalkToMonst(int mnum);
bool CheckMonsterHit(int mnum, bool *ret);
int encode_enemy(int mnum);
void decode_enemy(int mnum, int enemy);

/* data */

extern const int offset_x[8];
extern const int offset_y[8];
//extern const char walk2dir[9];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MONSTER_H__ */
