/**
 * @file monster.h
 *
 * Interface of monster functionality, AI, actions, spawning, loading, etc.
 */
#ifndef __MONSTER_H__
#define __MONSTER_H__

DEVILUTION_BEGIN_NAMESPACE

#define SQUELCH_MAX					UCHAR_MAX
#define MINION_INACTIVE(x)			((x->_mx | x->_my) == 0)
#define MINION_NR_INACTIVE(x)		((monsters[x]._mx | monsters[x]._my) == 0)
#define OPPOSITE(x)					(((x) + 4) & 7)
/** Maps from direction to the opposite direction. */
//const int opposite[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };

#ifdef __cplusplus
extern "C" {
#endif

extern int monstactive[MAXMONSTERS];
extern int nummonsters;
extern MonsterStruct monsters[MAXMONSTERS];
extern MapMonData mapMonTypes[MAX_LVLMTYPES];
extern int nummtypes;

void InitLevelMonsters();
void GetLevelMTypes();
void InitMonsterGFX(int midx);
#ifdef HELLFIRE
void WakeUberDiablo();
#endif
void InitMonsters();
void SetMapMonsters(BYTE *pMap, int startx, int starty);
int AddMonster(int x, int y, int dir, int mtidx, bool InMap);
void MonStartStand(int mnum, int md);
void RemoveMonFromMap(int mnum);
void MonGetKnockback(int mnum, int sx, int sy);
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
void SyncMonsterQ(int pnum, int idx);
void TalktoMonster(int mnum, int pnum);
void InitGolemStats(int mnum, int level);
void SpawnGolem(int mnum, int x, int y, int level);
bool CanTalkToMonst(int mnum);
bool CheckMonsterHit(int mnum, bool *ret);
int encode_enemy(int mnum);
void decode_enemy(int mnum, int enemy);

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
