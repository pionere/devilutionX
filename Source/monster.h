/**
 * @file monster.h
 *
 * Interface of monster functionality, AI, actions, spawning, loading, etc.
 */
#ifndef __MONSTER_H__
#define __MONSTER_H__

DEVILUTION_BEGIN_NAMESPACE

#define MINION_INACTIVE(x)		((x->_mx | x->_my) == 0)
#define MINION_NR_INACTIVE(x)		((monster[x]._mx | monster[x]._my) == 0)

#ifdef __cplusplus
extern "C" {
#endif

extern int monstactive[MAXMONSTERS];
extern int nummonsters;
extern BOOLEAN sgbSaveSoundOn;
extern MonsterStruct monster[MAXMONSTERS];
extern CMonster Monsters[MAX_LVLMTYPES];
extern int nummtypes;

void InitLevelMonsters();
void GetLevelMTypes();
void InitMonsterGFX(int midx);
#ifdef HELLFIRE
void WakeUberDiablo();
#endif
void PlaceGroup(int mtype, int num, int leaderf, int leader);
void InitMonsters();
void SetMapMonsters(BYTE *pMap, int startx, int starty);
int AddMonster(int x, int y, int dir, int mtype, BOOL InMap);
BOOL MonTalker(int mnum);
void MonStartStand(int mnum, int md);
void MonClearSquares(int mnum);
void MonGetKnockback(int mnum);
void MonStartHit(int mnum, int pnum, int dam);
void MonStartKill(int mnum, int pnum);
void MonSyncStartKill(int mnum, int x, int y, int pnum);
void MonTeleport(int mnum);
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
void MAI_GoatMc(int mnum);
void MAI_GoatBow(int mnum);
void MAI_Succ(int mnum);
void MAI_AcidUniq(int mnum);
#ifdef HELLFIRE
void MAI_Firebat(int mnum);
void MAI_Torchant(int mnum);
void MAI_Horkdemon(int mnum);
void MAI_Lich(int mnum);
void MAI_ArchLich(int mnum);
void MAI_PsychOrb(int mnum);
void MAI_NecromOrb(int mnum);
void MAI_BoneDemon(int mnum);
#endif
void MAI_Scav(int mnum);
void MAI_Garg(int mnum);
void MAI_Magma(int mnum);
void MAI_Storm(int mnum);
void MAI_Storm2(int mnum);
void MAI_Acid(int mnum);
void MAI_Diablo(int mnum);
void MAI_Mega(int mnum);
void MAI_Golum(int mnum);
void MAI_SkelKing(int mnum);
void MAI_Rhino(int mnum);
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
void SyncMonsterAnim(int mnum);
void MonFallenFear(int x, int y);
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
void SpawnGolum(int mnum, int x, int y, int level);
BOOL CanTalkToMonst(int mnum);
BOOL CheckMonsterHit(int mnum, BOOL *ret);
int encode_enemy(int mnum);
void decode_enemy(int mnum, int enemy);

/* data */

extern const int opposite[8];
extern const int offset_x[8];
extern const int offset_y[8];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MONSTER_H__ */
