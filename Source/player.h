/**
 * @file player.h
 *
 * Interface of player functionality, leveling, actions, creation, loading, etc.
 */
#ifndef __PLAYER_H__
#define __PLAYER_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int plr_lframe_size;
extern int plr_wframe_size;
extern BYTE plr_gfx_flag;
extern int plr_aframe_size;
extern int myplr;
extern PlayerStruct plr[MAX_PLRS];
extern int plr_fframe_size;
extern int plr_qframe_size;
extern BOOL deathflag;
extern int plr_hframe_size;
extern int plr_bframe_size;
extern BYTE plr_gfx_bflag;
extern int plr_sframe_size;
extern int deathdelay;
extern int plr_dframe_size;

void SetPlayerGPtrs(BYTE *pData, BYTE **pAnim);
void LoadPlrGFX(int pnum, player_graphic gfxflag);
void InitPlayerGFX(int pnum);
void InitPlrGFXMem(int pnum);
DWORD GetPlrGFXSize(const char *szCel);
void FreePlayerGFX(int pnum);
void NewPlrAnim(int pnum, BYTE *Peq, int numFrames, int Delay, int width);
void ClearPlrPVars(int pnum);
void SetPlrAnims(int pnum);
void ClearPlrRVars(PlayerStruct *p);
void CreatePlayer(int pnum, char c);
int CalcStatDiff(int pnum);
void NextPlrLevel(int pnum);
void AddPlrExperience(int pnum, int lvl, int exp);
void AddPlrMonstExper(int lvl, int exp, char pmask);
void InitPlayer(int pnum, BOOL FirstTime);
void InitMultiView();
void CheckEFlag(int pnum, BOOL flag);
BOOL PlrDirOK(int pnum, int dir);
void PlrClrTrans(int x, int y);
void PlrDoTrans(int x, int y);
void SetPlayerOld(int pnum);
void FixPlayerLocation(int pnum, int dir);
void PlrStartStand(int pnum, int dir);
void StartWalkStand(int pnum);
void PlrChangeLightOff(int pnum);
void PlrChangeOffset(int pnum);
void StartWalk(int pnum, int xvel, int yvel, int xadd, int yadd, int EndDir, int sdir);
void StartWalk2(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int EndDir, int sdir);
void StartWalk3(int pnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, int EndDir, int sdir);
void StartAttack(int pnum, int dir);
void StartRangeAttack(int pnum, int dir, int cx, int cy);
void PlrStartBlock(int pnum, int dir);
void StartSpell(int pnum, int dir, int cx, int cy);
void FixPlrWalkTags(int pnum);
void RemovePlrFromMap(int pnum);
void StartPlrHit(int pnum, int dam, BOOL forcehit);
void RespawnDeadItem(ItemStruct *is, int x, int y);
void StartPlrKill(int pnum, int earflag);
void PlrDeadItem(int pnum, ItemStruct *is, int xx, int yy);
void DropHalfPlayersGold(int pnum);
#ifdef HELLFIRE
void StripTopGold(int pnum);
#endif
void SyncPlrKill(int pnum, int earflag);
void RemovePlrMissiles(int pnum);
void InitLevelChange(int pnum);
void StartNewLvl(int pnum, int fom, int lvl);
void RestartTownLvl(int pnum);
void StartWarpLvl(int pnum, int pidx);
BOOL PlrDoStand(int pnum);
BOOL PlrDoNewLvl(int pnum);
BOOL PlrDoWalk(int pnum);
BOOL PlrDoWalk2(int pnum);
BOOL PlrDoWalk3(int pnum);
BOOL WeaponDur(int pnum, int durrnd);
BOOL PlrHitMonst(int pnum, int mnum);
BOOL PlrHitPlr(int offp, char defp);
BOOL PlrHitObj(int pnum, int mx, int my);
BOOL PlrDoAttack(int pnum);
BOOL PlrDoRangeAttack(int pnum);
void ShieldDur(int pnum);
BOOL PlrDoBlock(int pnum);
BOOL PlrDoSpell(int pnum);
BOOL PlrDoGotHit(int pnum);
void ArmorDur(int pnum);
BOOL PlrDoDeath(int pnum);
void CheckNewPath(int pnum);
BOOL PlrDeathModeOK(int pnum);
void ValidatePlayer();
void ProcessPlayers();
void CheckCheatStats(int pnum);
void ClrPlrPath(int pnum);
BOOL PosOkPlayer(int pnum, int x, int y);
void MakePlrPath(int pnum, int xx, int yy, BOOL endspace);
void CheckPlrSpell();
void SyncPlrAnim(int pnum);
void SyncInitPlrPos(int pnum);
void SyncInitPlr(int pnum);
void CheckStats(int pnum);
void ModifyPlrStr(int pnum, int v);
void ModifyPlrMag(int pnum, int v);
void ModifyPlrDex(int pnum, int v);
void ModifyPlrVit(int pnum, int v);
void SetPlayerHitPoints(int pnum, int val);
void SetPlrStr(int pnum, int v);
void SetPlrMag(int pnum, int v);
void SetPlrDex(int pnum, int v);
void SetPlrVit(int pnum, int v);
void InitDungMsgs(int pnum);
void PlayDungMsgs();
#ifdef HELLFIRE
int player_45EFA1(int pc);
int player_45EFAB(int pc);
int player_45EFB5(int pc);
#endif

/* rdata */

extern const char ArmourChar[4];
extern const char WepChar[10];
extern const char CharChar[];

/* data */

extern int plrxoff[9];
extern int plryoff[9];
extern int plrxoff2[9];
extern int plryoff2[9];
extern char PlrGFXAnimLens[NUM_CLASSES][11];
extern int PWVel[NUM_CLASSES][3];
extern int AnimLenFromClass[NUM_CLASSES];
extern int StrengthTbl[NUM_CLASSES];
extern int MagicTbl[NUM_CLASSES];
extern int DexterityTbl[NUM_CLASSES];
extern int VitalityTbl[NUM_CLASSES];
extern int ToBlkTbl[NUM_CLASSES];
extern const char *const ClassStrTblOld[];
extern int MaxStats[NUM_CLASSES][4];
extern int ExpLvlsTbl[MAXCHARLEVEL + 1];
extern const char *const ClassStrTbl[NUM_CLASSES];
extern BYTE fix[9];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLAYER_H__ */
