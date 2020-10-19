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

extern int MissileFileFlag;
extern int monstkills[MAXMONSTERS];
extern int monstactive[MAXMONSTERS];
extern int nummonsters;
extern BOOLEAN sgbSaveSoundOn;
extern MonsterStruct monster[MAXMONSTERS];
extern int totalmonsters;
extern CMonster Monsters[MAX_LVLMTYPES];
#ifdef HELLFIRE
extern int GraphicTable[NUMLEVELS][MAX_LVLMTYPES];
#else
extern BYTE GraphicTable[NUMLEVELS][MAX_LVLMTYPES];
#endif
extern int monstimgtot;
extern int uniquetrans;
extern int nummtypes;

void InitMonsterTRN(int midx, BOOL special);
void InitLevelMonsters();
int AddMonsterType(int type, int placeflag);
void GetLevelMTypes();
void InitMonsterGFX(int midx);
void ClearMVars(int i);
void InitMonster(int mnum, int rd, int mtype, int x, int y);
void ClrAllMonsters();
BOOL MonstPlace(int xp, int yp);
void monster_some_crypt();
void PlaceMonster(int mnum, int mtype, int x, int y);
void PlaceUniqueMonst(int uniqindex, int miniontype, int unpackfilesize);
void PlaceQuestMonsters();
void PlaceGroup(int mtype, int num, int leaderf, int leader);
void LoadDiabMonsts();
void InitMonsters();
void PlaceUniques();
void SetMapMonsters(BYTE *pMap, int startx, int starty);
void DeleteMonster(int i);
int AddMonster(int x, int y, int dir, int mtype, BOOL InMap);
#ifdef HELLFIRE
void monster_43C785(int mnum);
#endif
void NewMonsterAnim(int mnum, AnimStruct *anim, int md);
BOOL M_Ranged(int mnum);
BOOL M_Talker(int mnum);
void M_Enemy(int mnum);
int M_GetDir(int mnum);
void M_CheckEFlag(int i);
void M_StartStand(int mnum, int md);
void M_StartDelay(int mnum, int len);
void M_StartSpStand(int mnum, int md);
void M_StartWalk(int mnum, int xvel, int yvel, int xadd, int yadd, int EndDir);
void M_StartWalk2(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int EndDir);
void M_StartWalk3(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, int EndDir);
void M_StartAttack(int mnum);
void M_StartRAttack(int mnum, int mitype, int dam);
void M_StartRSpAttack(int mnum, int mitype, int dam);
void M_StartSpAttack(int mnum);
void M_StartEat(int mnum);
void M_ClearSquares(int mnum);
void M_GetKnockback(int mnum);
void M_StartHit(int mnum, int pnum, int dam);
void M_DiabloDeath(int mnum, BOOL sendmsg);
#ifdef HELLFIRE
void SpawnLoot(int mnum, BOOL sendmsg);
#endif
void M2MStartHit(int defm, int offm, int dam);
void MonstStartKill(int mnum, int pnum, BOOL sendmsg);
void M2MStartKill(int offm, int defm);
void M_StartKill(int mnum, int pnum);
void M_SyncStartKill(int mnum, int x, int y, int pnum);
void M_StartFadein(int mnum, int md, BOOL backwards);
void M_StartFadeout(int mnum, int md, BOOL backwards);
void M_StartHeal(int mnum);
void M_ChangeLightOffset(int mnum);
BOOL M_DoStand(int mnum);
BOOL M_DoWalk(int mnum);
BOOL M_DoWalk2(int mnum);
BOOL M_DoWalk3(int mnum);
void M_TryM2MHit(int offm, int defm, int hper, int mind, int maxd);
void M_TryH2HHit(int mnum, int pnum, int Hit, int MinDam, int MaxDam);
BOOL M_DoAttack(int mnum);
BOOL M_DoRAttack(int mnum);
int M_DoRSpAttack(int mnum);
BOOL M_DoSAttack(int mnum);
BOOL M_DoFadein(int mnum);
BOOL M_DoFadeout(int mnum);
int M_DoHeal(int mnum);
int M_DoTalk(int mnum);
void M_Teleport(int mnum);
BOOL M_DoGotHit(int mnum);
void M_UpdateLeader(int mnum);
void DoEnding();
void PrepDoEnding();
BOOL M_DoDeath(int mnum);
BOOL M_DoSpStand(int mnum);
BOOL M_DoDelay(int mnum);
BOOL M_DoStone(int mnum);
void M_WalkDir(int mnum, int md);
void GroupUnity(int mnum);
BOOL M_CallWalk(int mnum, int md);
BOOL M_PathWalk(int mnum);
BOOL M_CallWalk2(int mnum, int md);
BOOL M_DumbWalk(int mnum, int md);
BOOL M_RoundWalk(int mnum, int md, int *dir);
void MAI_Zombie(int mnum);
void MAI_SkelSd(int mnum);
BOOL MAI_Path(int mnum);
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
#ifdef HELLFIRE
void mai_roundranged_441EA0(int mnum);
#endif
void MAI_Acid(int mnum);
void MAI_Diablo(int mnum);
void MAI_RR2(int mnum, int mitype, int dam);
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
void M_FallenFear(int x, int y);
void PrintMonstHistory(int mt);
void PrintUniqueHistory();
void MissToMonst(int mnum, int x, int y);
BOOL PosOkMonst(int mnum, int x, int y);
#ifdef HELLFIRE
BOOLEAN monster_posok(int mnum, int x, int y);
#endif
BOOL PosOkMonst2(int mnum, int x, int y);
BOOL PosOkMonst3(int mnum, int x, int y);
BOOL IsSkel(int mt);
BOOL IsGoat(int mt);
int M_SpawnSkel(int x, int y, int dir);
void ActivateSpawn(int mnum, int x, int y, int dir);
BOOL SpawnSkeleton(int mnum, int x, int y);
int PreSpawnSkeleton();
void TalktoMonster(int mnum);
void SpawnGolum(int mnum, int x, int y, int mi);
BOOL CanTalkToMonst(int mnum);
BOOL CheckMonsterHit(int mnum, BOOL *ret);
int encode_enemy(int mnum);
void decode_enemy(int mnum, int enemy);

/* rdata */

extern const char plr2monst[9];
extern const BYTE counsmiss[4];

/* data */

extern int MWVel[24][3];
extern char animletter[7];
extern int left[8];
extern int right[8];
extern int opposite[8];
extern int offset_x[8];
extern int offset_y[8];

/** unused */
extern int rnd5[4];
extern int rnd10[4];
extern int rnd20[4];
extern int rnd60[4];
//

extern void (*AiProc[])(int i);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MONSTER_H__ */
