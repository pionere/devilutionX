/**
 * @file player.h
 *
 * Interface of player functionality, leveling, actions, creation, loading, etc.
 */
#ifndef __PLAYER_H__
#define __PLAYER_H__

DEVILUTION_BEGIN_NAMESPACE

#define myplr					players[mypnum]
#define plr						players[pnum]
#define plx(x)					players[x]

#ifdef __cplusplus
extern "C" {
#endif

extern int mypnum;
extern PlayerStruct players[MAX_PLRS];
extern BYTE lvlLoad;
extern bool gbDeathflag;

void LoadPlrGFX(int pnum, unsigned gfxflag);
void InitPlayerGFX(int pnum);
void InitPlrGFXMem(int pnum);
void FreePlayerGFX(int pnum);
void NewPlrAnim(int pnum, BYTE **anims, int dir, unsigned numFrames, int frameLen, int width);
void SetPlrAnims(int pnum);
//int GetFrameToUseForPlayerRendering(int pnum);
void CreatePlayer(int pnum, BYTE c);
#ifdef _DEBUG
void NextPlrLevel(int pnum);
#endif
void AddPlrExperience(int pnum, int lvl, int exp);
void AddPlrMonstExper(int lvl, int exp, char pmask);
void InitPlayer(int pnum, bool FirstTime, bool active);
//void PlrClrTrans(int x, int y);
//void PlrDoTrans(int x, int y);
void FixPlayerLocation(int pnum);
void PlrStartStand(int pnum, int dir);
void PlrStartBlock(int pnum, int dir);
bool PlacePlayer(int pnum);
void RemovePlrFromMap(int pnum);
void StartPlrHit(int pnum, int dam, bool forcehit);
void StartPlrKill(int pnum, int dmgtype);
void SyncPlrResurrect(int pnum);
void RemovePlrMissiles(int pnum);
void StartNewLvl(int pnum, int fom, int lvl);
void RestartTownLvl(int pnum);
void StartWarpLvl(int pnum, int pidx);
void PlrFillHp(int pnum);
void PlrFillMana(int pnum);
void PlrSetHp(int pnum, int hp);
void PlrSetMana(int pnum, int mana);
void PlrIncHp(int pnum, int hp);
void PlrIncMana(int pnum, int mana);
bool PlrDecHp(int pnum, int hp, int dmgtype);
void PlrDecMana(int pnum, int mana);
void ProcessPlayers();
void ClrPlrPath(int pnum);
bool PosOkPlayer(int pnum, int x, int y);
void MakePlrPath(int pnum, int xx, int yy, bool endspace);
void SyncPlrAnim();
void SyncInitPlrPos(int pnum);
void SyncInitPlr(int pnum);
void IncreasePlrStr(int pnum);
void IncreasePlrMag(int pnum);
void IncreasePlrDex(int pnum);
void IncreasePlrVit(int pnum);
void RestorePlrHpVit(int pnum);
void InitDungMsgs();
void PlayDungMsgs();

inline void SetPlayerOld(int pnum)
{
	plr._poldx = plr._px;
	plr._poldy = plr._py;
}

/* data */

extern const int plrxoff[MAX_PLRS];
extern const int plryoff[MAX_PLRS];
extern const BYTE PlrAnimFrameLens[NUM_PLR_ANIMS];
extern const int StrengthTbl[NUM_CLASSES];
extern const int MagicTbl[NUM_CLASSES];
extern const int DexterityTbl[NUM_CLASSES];
extern const int VitalityTbl[NUM_CLASSES];
extern const int Abilities[NUM_CLASSES];
extern const char *const ClassStrTbl[NUM_CLASSES];
extern const unsigned SkillExpLvlsTbl[MAXSPLLEVEL + 1];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLAYER_H__ */
