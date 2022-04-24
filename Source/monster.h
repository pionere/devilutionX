/**
 * @file monster.h
 *
 * Interface of monster functionality, AI, actions, spawning, loading, etc.
 */
#ifndef __MONSTER_H__
#define __MONSTER_H__

DEVILUTION_BEGIN_NAMESPACE

// Ticks necessary to finish the current action and add the result to the delta
// ~ ACTION_LENGTH + (gbNetUpdateRate * gbEmptyTurns) * (MAXMONSTERS / (NET_NORMAL_MSG_SIZE / sizeof(TSyncMonster)))
#define SQUELCH_LOW					127
#define SQUELCH_MAX					(SQUELCH_LOW + 240)
#define OPPOSITE(x)					(((x) + 4) & 7)
/** Maps from direction to the opposite direction. */
//const int opposite[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };

#ifdef __cplusplus
extern "C" {
#endif

extern int nummonsters;
extern MonsterStruct monsters[MAXMONSTERS];
extern MapMonData mapMonTypes[MAX_LVLMTYPES];
extern int nummtypes;
extern BYTE numSkelTypes;
/* The number of goat-monster types on the current level. */
extern BYTE numGoatTypes;
/* Skeleton-monster types on the current level. */
extern BYTE mapSkelTypes[MAX_LVLMTYPES];
/* Goat-monster types on the current level. */
extern BYTE mapGoatTypes[MAX_LVLMTYPES];

void InitLevelMonsters();
void GetLevelMTypes();
#ifdef HELLFIRE
void WakeUberDiablo();
#endif
void InitMonsters();
void SetMapMonsters(BYTE *pMap, int startx, int starty);
void MonChangeMap();
void InitMonster(int mnum, int dir, int mtidx, int x, int y);
void AddMonster(int x, int y, int dir, int mtidx);
int SummonMonster(int x, int y, int dir, int mtidx);
void RemoveMonFromMap(int mnum);
void MonGetKnockback(int mnum, int sx, int sy);
void MonStartHit(int mnum, int pnum, int dam, unsigned hitflags);
void MonStartKill(int mnum, int pnum);
void MonSyncStartKill(int mnum, int x, int y, int pnum);
void MonUpdateLeader(int mnum);
void DoEnding();
void MonWalkDir(int mnum, int md);
void ProcessMonsters();
void FreeMonsters();
bool MonDirOK(int mnum, int mdir);
bool CheckAllowMissile(int x, int y);
bool CheckNoSolid(int x, int y);
bool LineClearF(bool (*Clear)(int, int), int x1, int y1, int x2, int y2);
bool LineClear(int x1, int y1, int x2, int y2);
bool LineClearF1(bool (*Clear)(int, int, int), int mnum, int x1, int y1, int x2, int y2);
void SyncMonsterAnim(int mnum);
void MissToMonst(int mnum);
/* Check if the monster can be displaced to the given position. (unwillingly) */
bool PosOkMonster(int mnum, int x, int y);
/* Check if the monster can be placed to the given position. (willingly) */
bool PosOkMonst(int mnum, int x, int y);
/* Check if the monster could walk on the given position. (ignoring players/monsters) */
bool PosOkMonst2(int mnum, int x, int y);
/* Check if the monster could walk on the given position. (ignoring doors) */
bool PosOkMonst3(int mnum, int x, int y);
void SpawnSkeleton(int mnum, int x, int y, int dir);
int PreSpawnSkeleton();
void SyncMonsterQ(int pnum, int idx);
void TalktoMonster(int mnum, int pnum);
void InitGolemStats(int mnum, int level);
void SpawnGolem(int mnum, int x, int y, int level);
bool CanTalkToMonst(int mnum);
bool CheckMonsterHit(int mnum, bool *ret);

inline void SetMonsterLoc(MonsterStruct* mon, int x, int y)
{
	mon->_mx = mon->_mfutx = mon->_moldx = x;
	mon->_my = mon->_mfuty = mon->_moldy = y;
}

/* data */

extern const int offset_x[NUM_DIRS];
extern const int offset_y[NUM_DIRS];
//extern const char walk2dir[9];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MONSTER_H__ */
