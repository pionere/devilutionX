/**
 * @file monster.h
 *
 * Interface of monster functionality, AI, actions, spawning, loading, etc.
 */
#ifndef __MONSTER_H__
#define __MONSTER_H__

DEVILUTION_BEGIN_NAMESPACE

#define MON_WALK_SHIFT 8

// Ticks necessary to finish the current action and add the result to the delta
// ~ ACTION_LENGTH + (gbNetUpdateRate * gbEmptyTurns) * (MAXMONSTERS / (NET_TURN_MSG_SIZE / sizeof(TSyncMonster)))
#define SQUELCH_LOW 127
#define SQUELCH_MAX (SQUELCH_LOW + 240)
#define OPPOSITE(x) (((x) + 4) & 7)
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

void InitLvlMonsters();
void GetLevelMTypes();
#ifdef HELLFIRE
void WakeNakrul();
#endif
void InitMonsters();
void MonChangeMap();
void InitMonster(int mnum, int dir, int mtidx, int x, int y);
void AddMonster(int mtidx, int x, int y);
void InitSummonedMonster(int mnum, int dir, int mtidx, int x, int y);
int SummonMonster(int x, int y, int dir, int mtidx);
void RemoveMonFromMap(int mnum);
void MonHitByPlr(int mnum, int pnum, int dam, unsigned hitflags, int dir);
void MonHitByMon(int defm, int offm, int dam, int dir);
void MonKill(int mnum, int mpnum);
void MonSyncKill(int mnum, int x, int y, int pnum);
void MonLeaveLeader(int mnum);
void MonUpdateLeader(int mnum);
void MonAddDead(int mnum);
void DoEnding();
void ProcessMonsters();
void FreeMonsters();
//bool CheckAllowMissile(int x, int y);
bool LineClear(int x1, int y1, int x2, int y2);
void SyncMonsterAnim(int mnum);
void MonHinder(int mnum, int spllvl, unsigned tick);
void MissToMonst(int mi);
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
void PreSpawnGolem(int mnum, int level);
void SpawnGolem(int mnum, int x, int y, int level);
bool CanTalkToMonst(int mnum);
bool CheckMonsterHit(int mnum, bool* ret);
void MonCallToArms(int mnum);

inline void SetMonsterLoc(MonsterStruct* mon, int x, int y)
{
	mon->_mx = mon->_mfutx = mon->_moldx = x;
	mon->_my = mon->_mfuty = mon->_moldy = y;
}

/* data */

extern const int offset_x[NUM_DIRS];
extern const int offset_y[NUM_DIRS];
extern const int MWVel[24];
//extern const int8_t walk2dir[9];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MONSTER_H__ */
