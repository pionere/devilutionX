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
extern BYTE drlgFlags[DMAXX][DMAXY];
extern int setpc_x;
extern int setpc_y;
extern int setpc_w;
extern int setpc_h;
extern BYTE *pSetPiece;
extern BYTE *pSpecialCels;
extern BYTE *pMegaTiles;
extern MICROS pMicroPieces[MAXTILES + 1];
extern BYTE* pMicroCels;
extern BYTE microFlags[MAXTILES + 1];
extern bool nBlockTable[MAXTILES + 1];
extern bool nSolidTable[MAXTILES + 1];
extern bool nTrapTable[MAXTILES + 1];
extern bool nMissileTable[MAXTILES + 1];
extern int gnDifficulty;
extern LevelStruct currLvl;
extern int MicroTileLen;
extern BYTE numtrans;
extern bool TransList[256];
extern int dPiece[MAXDUNX][MAXDUNY];
extern BYTE dTransVal[MAXDUNX][MAXDUNY];
extern BYTE dPreLight[MAXDUNX][MAXDUNY];
extern BYTE dLight[MAXDUNX][MAXDUNY];
extern BYTE dFlags[MAXDUNX][MAXDUNY];
extern char dPlayer[MAXDUNX][MAXDUNY];
extern int dMonster[MAXDUNX][MAXDUNY];
extern BYTE dDead[MAXDUNX][MAXDUNY];
extern char dObject[MAXDUNX][MAXDUNY];
extern BYTE dItem[MAXDUNX][MAXDUNY];
extern BYTE dMissile[MAXDUNX][MAXDUNY];
extern BYTE dSpecial[MAXDUNX][MAXDUNY];
extern int themeCount;
extern THEME_LOC themeLoc[MAXTHEMES];

void InitLvlDungeon();
void FreeLvlDungeon();
void DRLG_Init_Globals();
void DRLG_PlaceRndTile(BYTE search, BYTE replace, BYTE rndper);
POS32 DRLG_PlaceMiniSet(const BYTE *miniset);
void DRLG_PlaceMegaTiles(int idx);
void DRLG_InitTrans();
void DRLG_MRectTrans(int x1, int y1, int x2, int y2, int tv);
void DRLG_RectTrans(int x1, int y1, int x2, int y2);
void DRLG_ListTrans(int num, const BYTE *List);
void DRLG_AreaTrans(int num, const BYTE *List);
void DRLG_FloodTVal(BYTE floor);
void DRLG_InitSetPC();
void DRLG_SetPC();
void Make_SetPC(int x, int y, int w, int h);
void DRLG_PlaceThemeRooms(int minSize, int maxSize, int floor, int freq, bool rndSize);
void DRLG_HoldThemeRooms();
bool NearThemeRoom(int x, int y);

inline void DRLG_CopyTrans(int sx, int sy, int dx, int dy)
{
	dTransVal[dx][dy] = dTransVal[sx][sy];
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GENDUNG_H__ */
