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
extern DrlgMem drlg;
extern SetPieceStruct pSetPieces[4];
extern WarpStruct pWarps[NUM_DWARP];
extern BYTE* pSpecialsCel;
/* Maps from subtile_id to automap type (_automap_subtypes). */
extern BYTE automaptype[MAXSUBTILES + 1];
extern BYTE nTrnShadowTable[MAXTILES + 1];
extern uint16_t pSubtiles[MAXSUBTILES + 1][16 * ASSET_MPL * ASSET_MPL];
extern BYTE* pMicrosCel;
extern BYTE microFlags[MAXSUBTILES + 1];
extern bool nSolidTable[MAXSUBTILES + 1];
extern bool nBlockTable[MAXSUBTILES + 1];
extern bool nMissileTable[MAXSUBTILES + 1];
extern BYTE nSpecTrapTable[MAXSUBTILES + 1];
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
extern int8_t dPlayer[MAXDUNX][MAXDUNY];
extern int dMonster[MAXDUNX][MAXDUNY];
extern BYTE dDead[MAXDUNX][MAXDUNY];
extern int8_t dObject[MAXDUNX][MAXDUNY];
extern BYTE dItem[MAXDUNX][MAXDUNY];
extern BYTE dMissile[MAXDUNX][MAXDUNY];

void InitLvlDungeon();
void FreeSetPieces();
void FreeLvlDungeon();
void InitLvlMap();
void DRLG_PlaceRndTile(BYTE search, BYTE replace, BYTE rndper);
POS32 DRLG_PlaceMiniSet(const BYTE* miniset);
void DRLG_PlaceMegaTiles(int idx);
void DRLG_DrawMiniSet(const BYTE* miniset, int sx, int sy);
void DRLG_DrawMap(int idx);
void DRLG_InitTrans();
void DRLG_RedoTrans();
//void DRLG_MRectTrans(int x1, int y1, int x2, int y2, int tv);
//void DRLG_RectTrans(int x1, int y1, int x2, int y2);
//void DRLG_ListTrans(int num, const BYTE* List);
//void DRLG_AreaTrans(int num, const BYTE* List);
void DRLG_FloodTVal();
void DRLG_LoadSP(int idx, BYTE bv);
void DRLG_PlaceThemeRooms(int minSize, int maxSize, const BYTE (&themeTiles)[NUM_DRT_TYPES], int rndSkip);
bool InThemeRoom(int x, int y);
void DRLG_ChangeMap(int x1, int y1, int x2, int y2 /*, bool hasNewObjPiece*/);

inline void DRLG_CopyTrans(int sx, int sy, int dx, int dy)
{
	dTransVal[dx][dy] = dTransVal[sx][sy];
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GENDUNG_H__ */
