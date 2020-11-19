/**
 * @file monster.h
 *
 * Interface of monster functionality, AI, actions, spawning, loading, etc.
 */
#ifndef __MONSTER_H__
#define __MONSTER_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int monstkills[MAXMONSTERS];
extern int monstactive[MAXMONSTERS];
extern int nummonsters;
extern BOOLEAN sgbSaveSoundOn;
extern MonsterStruct monster[MAXMONSTERS];
extern CMonster Monsters[MAX_LVLMTYPES];
extern int nummtypes;

void InitLevelMonsters();
void GetLevelMTypes();
void InitMonsterGFX(int midx);
void InitMonster(int mnum, int dir, int mtype, int x, int y);
void ClrAllMonsters();
void monster_some_crypt();
void PlaceGroup(int mtype, int num, int leaderf, int leader);
void InitMonsters();
void SetMapMonsters(BYTE *pMap, int startx, int starty);
void DeleteMonster(int i);
int AddMonster(int x, int y, int dir, int mtype, BOOL InMap);
#ifdef HELLFIRE
void monster_43C785(int mnum);
#endif
BOOL MonTalker(int mnum);
void MonStartStand(int mnum, int md);
void MonClearSquares(int mnum);
void MonGetKnockback(int mnum);
void MonStartHit(int mnum, int pnum, int dam);
void MonStartKill(int mnum, int pnum);
void MonSyncStartKill(int mnum, int x, int y, int pnum);
BOOL MonDoStand(int mnum);
BOOL MonDoWalk(int mnum);
BOOL MonDoWalk2(int mnum);
BOOL MonDoWalk3(int mnum);
BOOL MonDoAttack(int mnum);
void MonTeleport(int mnum);
BOOL MonDoGotHit(int mnum);
void MonUpdateLeader(int mnum);
void DoEnding();
void PrepDoEnding();
void MonWalkDir(int mnum, int md);
void MAI_Zombie(int mnum);
void MAI_SkelSd(int mnum);
void MAI_Snake(int mnum);
void MAI_Bat(int mnum);
void MAI_SkelBow(int mnum);
void MAI_Fat(int mnum);
void MAI_Sneak(int mnum);
void MAI_Fireman(int mnum);
void MAI_Fallen(int mnum);
void MAI_Cleaver(int mnum);
void MAI_Round(int mnum, BOOL special);
void MAI_GoatMc(int mnum);
void MAI_Ranged(int mnum, int mitype, BOOL special);
void MAI_GoatBow(int mnum);
void MAI_Succ(int mnum);
#ifdef HELLFIRE
void mai_ranged_441649(int mnum);
void mai_ranged_441654(int mnum);
void mai_ranged_44165F(int mnum);
void mai_ranged_44166A(int mnum);
#endif
void MAI_AcidUniq(int mnum);
#ifdef HELLFIRE
void mai_ranged_441680(int mnum);
void mai_ranged_44168B(int mnum);
#endif
void MAI_Scav(int mnum);
void MAI_Garg(int mnum);
void MAI_RoundRanged(int mnum, int mitype, BOOL checkdoors, int dam, int lessmissiles);
void MAI_Magma(int mnum);
void MAI_Storm(int mnum);
void MAI_Storm2(int mnum);
#ifdef HELLFIRE
void mai_roundranged_441EA0(int mnum);
#endif
void MAI_Acid(int mnum);
void MAI_Diablo(int mnum);
void MAI_Mega(int mnum);
void MAI_Golum(int mnum);
void MAI_SkelKing(int mnum);
void MAI_Rhino(int mnum);
#ifdef HELLFIRE
void mai_horkdemon(int mnum);
#endif
void MAI_Counselor(int mnum);
void MAI_Garbud(int mnum);
void MAI_Zhar(int mnum);
void MAI_SnotSpil(int mnum);
void MAI_Lazurus(int mnum);
void MAI_Lazhelp(int mnum);
void MAI_Lachdanan(int mnum);
void MAI_Warlord(int mnum);
void DeleteMonsterList();
void ProcessMonsters();
void FreeMonsters();
BOOL DirOK(int mnum, int mdir);
BOOL PosOkMissile(int x, int y);
BOOL CheckNoSolid(int x, int y);
BOOL LineClearF(BOOL (*Clear)(int, int), int x1, int y1, int x2, int y2);
BOOL LineClear(int x1, int y1, int x2, int y2);
BOOL LineClearF1(BOOL (*Clear)(int, int, int), int mnum, int x1, int y1, int x2, int y2);
void SyncMonsterAnim(int i);
void MonFallenFear(int x, int y);
void PrintMonstHistory(int mt);
void PrintUniqueHistory(int mnum);
void MissToMonst(int mnum, int x, int y);
BOOL monster_posok(int mnum, int x, int y);
BOOL PosOkMonst(int mnum, int x, int y);
BOOL PosOkMonst2(int mnum, int x, int y);
BOOL PosOkMonst3(int mnum, int x, int y);
BOOL IsSkel(int mt);
BOOL IsGoat(int mt);
int MonSpawnSkel(int x, int y, int dir);
BOOL SpawnSkeleton(int mnum, int x, int y);
int PreSpawnSkeleton();
void TalktoMonster(int mnum);
void SpawnGolum(int mnum, int x, int y, int mi);
BOOL CanTalkToMonst(int mnum);
BOOL CheckMonsterHit(int mnum, BOOL *ret);
int encode_enemy(int mnum);
void decode_enemy(int mnum, int enemy);

/* data */

extern int opposite[8];
extern int offset_x[8];
extern int offset_y[8];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MONSTER_H__ */
