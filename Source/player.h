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

extern int myplr;
extern PlayerStruct plr[MAX_PLRS];
extern BOOL deathflag;

void LoadPlrGFX(int pnum, unsigned gfxflag);
void InitPlayerGFX(int pnum);
void InitPlrGFXMem(int pnum);
void FreePlayerGFX(int pnum);
void NewPlrAnim(int pnum, BYTE **anims, int dir, unsigned numFrames, int Delay, int width);
void SetPlrAnims(int pnum);
void CreatePlayer(int pnum, char c);
#ifdef _DEBUG
void NextPlrLevel(int pnum);
#endif
void AddPlrExperience(int pnum, int lvl, int exp);
void AddPlrMonstExper(int lvl, int exp, char pmask);
void InitPlayer(int pnum, BOOL FirstTime);
void InitMultiView();
void PlrClrTrans(int x, int y);
void PlrDoTrans(int x, int y);
void FixPlayerLocation(int pnum);
void PlrStartStand(int pnum, int dir);
void PlrStartBlock(int pnum, int dir);
void RemovePlrFromMap(int pnum);
void StartPlrHit(int pnum, int dam, BOOL forcehit);
void StartPlrKill(int pnum, int earflag);
void DropHalfPlayersGold(int pnum);
#ifdef HELLFIRE
void StripTopGold(int pnum);
#endif
void SyncPlrKill(int pnum, int earflag);
void RemovePlrMissiles(int pnum);
void StartNewLvl(int pnum, int fom, int lvl);
void RestartTownLvl(int pnum);
void StartWarpLvl(int pnum, int pidx);
int PlrAtkDam(int pnum);
void PlrFillHp(int pnum);
void PlrFillMana(int pnum);
void PlrSetHp(int pnum, int hp);
void PlrSetMana(int pnum, int mana);
void PlrIncHp(int pnum, int hp);
void PlrIncMana(int pnum, int mana);
BOOL PlrDecHp(int pnum, int hp, int earflag);
void PlrDecMana(int pnum, int mana);
void ProcessPlayers();
void ClrPlrPath(int pnum);
BOOL PosOkPlayer(int pnum, int x, int y);
void MakePlrPath(int pnum, int xx, int yy, BOOL endspace);
void SyncPlrAnim(int pnum);
void SyncInitPlrPos(int pnum);
void SyncInitPlr(int pnum);
void ModifyPlrStr(int pnum, int v);
void ModifyPlrMag(int pnum, int v);
void ModifyPlrDex(int pnum, int v);
void ModifyPlrVit(int pnum, int v);
void SetPlrStr(int pnum, int v);
void SetPlrMag(int pnum, int v);
void SetPlrDex(int pnum, int v);
void SetPlrVit(int pnum, int v);
void InitDungMsgs(int pnum);
void PlayDungMsgs();

inline void SetPlayerOld(PlayerStruct *p)
{
	p->_poldx = p->_px;
	p->_poldy = p->_py;
}

/* data */

extern const int plrxoff[MAX_PLRS];
extern const int plryoff[MAX_PLRS];
extern const int plrxoff2[9];
extern const int plryoff2[9];
extern const int StrengthTbl[NUM_CLASSES];
extern const int MagicTbl[NUM_CLASSES];
extern const int DexterityTbl[NUM_CLASSES];
extern const int VitalityTbl[NUM_CLASSES];
extern const int MaxStats[NUM_CLASSES][4];
extern const int Abilities[NUM_CLASSES];
extern const char *const ClassStrTbl[NUM_CLASSES];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLAYER_H__ */
