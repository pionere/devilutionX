/**
 * @file themes.h
 *
 * Interface of the theme room placing algorithms.
 */
#ifndef __THEMES_H__
#define __THEMES_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int numthemes;
extern BOOL armorFlag;
extern BOOL ThemeGoodIn[4];
extern BOOL weaponFlag;
extern BOOL treasureFlag;
extern BOOL mFountainFlag;
extern BOOL cauldronFlag;
extern BOOL tFountainFlag;
extern int zharlib;
extern int themex;
extern int themey;
extern int themeVar1;
extern ThemeStruct themes[MAXTHEMES];
extern BOOL pFountainFlag;
extern BOOL bFountainFlag;
extern BOOL bCrossFlag;

BOOL TFit_Shrine(int tidx);
BOOL TFit_Obj5(int tidx);
BOOL TFit_SkelRoom(int tidx);
BOOL TFit_GoatShrine(int tidx);
BOOL CheckThemeObj3(int x, int y, int tidx, int rndfrq);
BOOL TFit_Obj3(int tidx);
BOOL CheckThemeReqs(int theme);
BOOL SpecialThemeFit(int tidx, int theme);
BOOL CheckThemeRoom(int tv);
void InitThemes();
void HoldThemeRooms();
void PlaceThemeMonsts(int tidx, int rndfrq);
void Theme_Barrel(int tidx);
void Theme_Shrine(int tidx);
void Theme_MonstPit(int tidx);
void Theme_SkelRoom(int tidx);
void Theme_Treasure(int tidx);
void Theme_Library(int tidx);
void Theme_Torture(int tidx);
void Theme_BloodFountain(int tidx);
void Theme_Decap(int tidx);
void Theme_PurifyingFountain(int tidx);
void Theme_ArmorStand(int tidx);
void Theme_GoatShrine(int tidx);
void Theme_Cauldron(int tidx);
void Theme_MurkyFountain(int tidx);
void Theme_TearFountain(int tidx);
void Theme_BrnCross(int tidx);
void Theme_WeaponRack(int tidx);
void UpdateL4Trans();
void CreateThemeRooms();

/* rdata */

extern int ThemeGood[4];
extern int trm5x[25];
extern int trm5y[25];
extern int trm3x[9];
extern int trm3y[9];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __THEMES_H__ */
