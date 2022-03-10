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
extern BYTE gbLvlLoad;

void LoadPlrGFX(int pnum, unsigned gfxflag);
void InitPlayerGFX(int pnum);
void InitPlrGFXMem(int pnum);
void FreePlayerGFX(int pnum);
void SetPlrAnims(int pnum);
void CreatePlayer(const _uiheroinfo &heroinfo);
#if DEBUG_MODE
void NextPlrLevel(int pnum);
#endif
void AddPlrExperience(int pnum, int lvl, unsigned exp);
void InitPlayer(int pnum);
void InitLvlPlayer(int pnum, bool entering);
void RemoveLvlPlayer(int pnum);
//void PlrClrTrans(int x, int y);
//void PlrDoTrans(int x, int y);
void FixPlayerLocation(int pnum);
void PlrStartStand(int pnum);
void PlrStartBlock(int pnum, int dir);
void KnockbackPlr(int pnum, int dir);
void RemovePlrFromMap(int pnum);
void StartPlrHit(int pnum, int dam, bool forcehit, int dir);
void SyncPlrKill(int pnum, int dmgtype);
void SyncPlrResurrect(int pnum);
void StartNewLvl(int pnum, int fom, int lvl);
void RestartTownLvl(int pnum);
void StartTWarp(int pnum, int pidx);
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
void MissToPlr(int mi, bool hit);
bool PosOkPlayer(int pnum, int x, int y);
bool MakePlrPath(int pnum, int xx, int yy, bool endspace);
void SyncPlrAnim(int pnum);
void IncreasePlrStr(int pnum);
void IncreasePlrMag(int pnum);
void IncreasePlrDex(int pnum);
void IncreasePlrVit(int pnum);
void RestorePlrHpVit(int pnum);

// Set each location to the input location.
// Oldx/y could be set to an invalid value so RemovePlrFromMap could check if the player was placed on the map earlier,
//  but it is not worth it at the moment.
inline void SetPlayerLoc(PlayerStruct* p, int x, int y)
{
	p->_px = p->_pfutx = p->_poldx = x;
	p->_py = p->_pfuty = p->_poldy = y;
}

/* data */

extern const int plrxoff[MAX_PLRS];
extern const int plryoff[MAX_PLRS];
extern const BYTE PlrAnimFrameLens[NUM_PLR_ANIMS];
extern const int StrengthTbl[NUM_CLASSES];
extern const int MagicTbl[NUM_CLASSES];
extern const int DexterityTbl[NUM_CLASSES];
extern const int VitalityTbl[NUM_CLASSES];
extern const BYTE Abilities[NUM_CLASSES];
extern const char *const ClassStrTbl[NUM_CLASSES];
extern const unsigned PlrExpLvlsTbl[MAXCHARLEVEL + 1];
extern const unsigned SkillExpLvlsTbl[MAXSPLLEVEL + 1];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLAYER_H__ */
