/**
 * @file gendung.h
 *
 * Interface of general dungeon generation code.
 */
#ifndef __GENDUNG_H__
#define __GENDUNG_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern BYTE dungeon[DMAXX][DMAXY];
extern BYTE pdungeon[DMAXX][DMAXY];
extern char dflags[DMAXX][DMAXY];
extern int setpc_x;
extern int setpc_y;
extern int setpc_w;
extern int setpc_h;
extern BYTE *pSetPiece;
extern BOOL setloadflag;
extern BYTE *pSpecialCels;
extern BYTE *pMegaTiles;
extern BYTE *pLevelPieces;
extern BYTE *pDungeonCels;
extern char block_lvid[MAXTILES + 1];
extern BOOLEAN nBlockTable[MAXTILES + 1];
extern BOOLEAN nSolidTable[MAXTILES + 1];
extern BOOLEAN nTransTable[MAXTILES + 1];
extern BOOLEAN nMissileTable[MAXTILES + 1];
extern BOOLEAN nTrapTable[MAXTILES + 1];
extern int gnDifficulty;
extern BYTE leveltype;
extern BYTE currlevel;
extern BOOLEAN setlevel;
extern BYTE setlvlnum;
extern int ViewX;
extern int ViewY;
extern ScrollStruct ScrollInfo;
extern int MicroTileLen;
extern char TransVal;
extern BOOLEAN TransList[256];
extern int dPiece[MAXDUNX][MAXDUNY];
extern MICROS dpiece_defs_map_2[MAXDUNX][MAXDUNY];
extern char dTransVal[MAXDUNX][MAXDUNY];
extern char dLight[MAXDUNX][MAXDUNY];
extern char dPreLight[MAXDUNX][MAXDUNY];
extern char dFlags[MAXDUNX][MAXDUNY];
extern char dPlayer[MAXDUNX][MAXDUNY];
extern int dMonster[MAXDUNX][MAXDUNY];
extern char dDead[MAXDUNX][MAXDUNY];
extern char dObject[MAXDUNX][MAXDUNY];
extern char dItem[MAXDUNX][MAXDUNY];
extern char dMissile[MAXDUNX][MAXDUNY];
extern char dSpecial[MAXDUNX][MAXDUNY];
extern int themeCount;
extern THEME_LOC themeLoc[MAXTHEMES];

void DRLG_Init_Globals();
void FillSolidBlockTbls();
void SetDungeonMicros(int x1, int y1, int x2, int y2);
void DRLG_PlaceMegaTiles(int idx);
void DRLG_InitTrans();
void DRLG_MRectTrans(int x1, int y1, int x2, int y2);
void DRLG_RectTrans(int x1, int y1, int x2, int y2);
void DRLG_ListTrans(int num, const BYTE *List);
void DRLG_AreaTrans(int num, const BYTE *List);
void DRLG_InitSetPC();
void DRLG_SetPC();
void Make_SetPC(int x, int y, int w, int h);
void DRLG_PlaceThemeRooms(int minSize, int maxSize, int floor, int freq, BOOL rndSize);
void DRLG_HoldThemeRooms();
BOOL SkipThemeRoom(int x, int y);
void InitLevels();

inline void DRLG_CopyTrans(int sx, int sy, int dx, int dy)
{
	dTransVal[dx][dy] = dTransVal[sx][sy];
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GENDUNG_H__ */
