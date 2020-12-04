/**
 * @file town.h
 *
 * Implementation of functionality for rendering the town, towners and calling other render routines.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Load level data into dPiece
 * @param P3Tiles Tile set
 * @param pSector Sector data
 * @param xi upper left destination
 * @param yi upper left destination
 * @param w width of sector
 * @param h height of sector
 */
void T_FillSector(BYTE *P3Tiles, BYTE *pSector, int xi, int yi, int w, int h)
{
	int i, j, xx, yy, nMap;
	long v1, v2, v3, v4, ii;
	WORD *Sector;

	ii = 4;
	yy = yi;
	for (j = 0; j < h; j++) {
		xx = xi;
		for (i = 0; i < w; i++) {
			WORD *Map;

			Map = (WORD *)&pSector[ii];
			nMap = SDL_SwapLE16(*Map);
			if (nMap) {
				Sector = (((WORD *)&P3Tiles[(nMap - 1) * 8]));
				v1 = SDL_SwapLE16(*(Sector + 0)) + 1;
				v2 = SDL_SwapLE16(*(Sector + 1)) + 1;
				v3 = SDL_SwapLE16(*(Sector + 2)) + 1;
				v4 = SDL_SwapLE16(*(Sector + 3)) + 1;

			} else {
				v1 = 0;
				v2 = 0;
				v3 = 0;
				v4 = 0;
			}
			dPiece[xx][yy] = v1;
			dPiece[xx + 1][yy] = v2;
			dPiece[xx][yy + 1] = v3;
			dPiece[xx + 1][yy + 1] = v4;
			xx += 2;
			ii += 2;
		}
		yy += 2;
	}
}

/**
 * @brief Load a tile in to dPiece
 * @param P3Tiles Tile set
 * @param xx upper left destination
 * @param yy upper left destination
 * @param t tile id
 */
void T_FillTile(BYTE *P3Tiles, int xx, int yy, int t)
{
	long v1, v2, v3, v4;
	WORD *Tiles;

	Tiles = ((WORD *)&P3Tiles[(t - 1) * 8]);
	v1 = SDL_SwapLE16(*(Tiles + 0)) + 1;
	v2 = SDL_SwapLE16(*(Tiles + 1)) + 1;
	v3 = SDL_SwapLE16(*(Tiles + 2)) + 1;
	v4 = SDL_SwapLE16(*(Tiles + 3)) + 1;

	dPiece[xx][yy] = v1;
	dPiece[xx + 1][yy] = v2;
	dPiece[xx][yy + 1] = v3;
	dPiece[xx + 1][yy + 1] = v4;
}

void T_HiveOpen()
{
	dPiece[68 + DBORDERX][50 + DBORDERY] = 0x48a;
	dPiece[69 + DBORDERX][50 + DBORDERY] = 0x48b;
	dPiece[68 + DBORDERX][51 + DBORDERY] = 0x48c;
	dPiece[69 + DBORDERX][51 + DBORDERY] = 0x50e;
	dPiece[68 + DBORDERX][52 + DBORDERY] = 0x4ee;
	dPiece[68 + DBORDERX][53 + DBORDERY] = 0x4f0;
	dPiece[69 + DBORDERX][52 + DBORDERY] = 0x510;
	dPiece[69 + DBORDERX][53 + DBORDERY] = 0x511;
	dPiece[69 + DBORDERX][54 + DBORDERY] = 0x512;
	dPiece[68 + DBORDERX][54 + DBORDERY] = 0x11a;
	dPiece[68 + DBORDERX][55 + DBORDERY] = 0x11c;
	dPiece[69 + DBORDERX][55 + DBORDERY] = 0x11d;
	dPiece[70 + DBORDERX][50 + DBORDERY] = 0x513;
	dPiece[70 + DBORDERX][51 + DBORDERY] = 0x515;
	dPiece[71 + DBORDERX][51 + DBORDERY] = 0x516;
	dPiece[72 + DBORDERX][50 + DBORDERY] = 0x517;
	dPiece[73 + DBORDERX][50 + DBORDERY] = 0x518;
	dPiece[72 + DBORDERX][51 + DBORDERY] = 0x519;
	dPiece[73 + DBORDERX][51 + DBORDERY] = 0x51a;
	dPiece[70 + DBORDERX][52 + DBORDERY] = 0x51b;
	dPiece[71 + DBORDERX][52 + DBORDERY] = 0x51c;
	dPiece[70 + DBORDERX][53 + DBORDERY] = 0x51d;
	dPiece[71 + DBORDERX][53 + DBORDERY] = 0x51e;
	dPiece[70 + DBORDERX][54 + DBORDERY] = 0x51f;
	dPiece[71 + DBORDERX][54 + DBORDERY] = 0x520;
	dPiece[70 + DBORDERX][55 + DBORDERY] = 0x521;
	dPiece[71 + DBORDERX][55 + DBORDERY] = 0x522;
	dPiece[72 + DBORDERX][54 + DBORDERY] = 0x527;
	dPiece[73 + DBORDERX][54 + DBORDERY] = 0x528;
	dPiece[72 + DBORDERX][55 + DBORDERY] = 0x529;
	dPiece[73 + DBORDERX][55 + DBORDERY] = 0x52a;
	dPiece[72 + DBORDERX][52 + DBORDERY] = 0x523;
	dPiece[73 + DBORDERX][52 + DBORDERY] = 0x524;
	dPiece[72 + DBORDERX][53 + DBORDERY] = 0x525;
	dPiece[73 + DBORDERX][53 + DBORDERY] = 0x526;
	dPiece[74 + DBORDERX][51 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][52 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][53 + DBORDERY] = 0x118;
	dPiece[75 + DBORDERX][50 + DBORDERY] = 0x118;
	dPiece[75 + DBORDERX][51 + DBORDERY] = 0x118;
	dPiece[75 + DBORDERX][53 + DBORDERY] = 8;
	dPiece[75 + DBORDERX][54 + DBORDERY] = 8;
	dPiece[76 + DBORDERX][50 + DBORDERY] = 0xd9;
	dPiece[76 + DBORDERX][51 + DBORDERY] = 0x18;
	dPiece[75 + DBORDERX][52 + DBORDERY] = 0x13;
	dPiece[74 + DBORDERX][54 + DBORDERY] = 0x118;
	SetDungeonMicros();
}

static void T_HiveClosed()
{
	dPiece[68 + DBORDERX][50 + DBORDERY] = 0x48a;
	dPiece[69 + DBORDERX][50 + DBORDERY] = 0x4eb;
	dPiece[68 + DBORDERX][51 + DBORDERY] = 0x4ec;
	dPiece[69 + DBORDERX][51 + DBORDERY] = 0x4ed;
	dPiece[68 + DBORDERX][52 + DBORDERY] = 0x4ee;
	dPiece[69 + DBORDERX][52 + DBORDERY] = 0x4ef;
	dPiece[68 + DBORDERX][53 + DBORDERY] = 0x4f0;
	dPiece[69 + DBORDERX][53 + DBORDERY] = 0x4f1;
	dPiece[68 + DBORDERX][54 + DBORDERY] = 0x4f2;
	dPiece[69 + DBORDERX][54 + DBORDERY] = 0x4f3;
	dPiece[68 + DBORDERX][55 + DBORDERY] = 0x4f4;
	dPiece[70 + DBORDERX][50 + DBORDERY] = 0x4f5;
	dPiece[71 + DBORDERX][50 + DBORDERY] = 0x4f6;
	dPiece[70 + DBORDERX][51 + DBORDERY] = 0x4f7;
	dPiece[71 + DBORDERX][51 + DBORDERY] = 0x4f8;
	dPiece[72 + DBORDERX][50 + DBORDERY] = 0x4f9;
	dPiece[73 + DBORDERX][50 + DBORDERY] = 0x4fa;
	dPiece[72 + DBORDERX][51 + DBORDERY] = 0x4fb;
	dPiece[73 + DBORDERX][51 + DBORDERY] = 0x4fc;
	dPiece[70 + DBORDERX][52 + DBORDERY] = 0x4fd;
	dPiece[71 + DBORDERX][52 + DBORDERY] = 0x4fe;
	dPiece[70 + DBORDERX][53 + DBORDERY] = 0x4ff;
	dPiece[71 + DBORDERX][53 + DBORDERY] = 0x500;
	dPiece[70 + DBORDERX][54 + DBORDERY] = 0x501;
	dPiece[71 + DBORDERX][54 + DBORDERY] = 0x502;
	dPiece[70 + DBORDERX][55 + DBORDERY] = 0x503;
	dPiece[71 + DBORDERX][55 + DBORDERY] = 0x504;
	dPiece[72 + DBORDERX][54 + DBORDERY] = 0x509;
	dPiece[73 + DBORDERX][54 + DBORDERY] = 0x50a;
	dPiece[72 + DBORDERX][55 + DBORDERY] = 0x50b;
	dPiece[73 + DBORDERX][55 + DBORDERY] = 0x50c;
	dPiece[72 + DBORDERX][52 + DBORDERY] = 0x505;
	dPiece[73 + DBORDERX][52 + DBORDERY] = 0x506;
	dPiece[72 + DBORDERX][53 + DBORDERY] = 0x507;
	dPiece[73 + DBORDERX][53 + DBORDERY] = 0x508;
	dPiece[74 + DBORDERX][51 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][52 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][53 + DBORDERY] = 0x118;
	dPiece[75 + DBORDERX][50 + DBORDERY] = 0x118;
	dPiece[75 + DBORDERX][51 + DBORDERY] = 0x118;
	dPiece[75 + DBORDERX][53 + DBORDERY] = 8;
	dPiece[75 + DBORDERX][54 + DBORDERY] = 8;
	dPiece[76 + DBORDERX][50 + DBORDERY] = 0xd9;
	dPiece[76 + DBORDERX][51 + DBORDERY] = 0x18;
	dPiece[75 + DBORDERX][52 + DBORDERY] = 0x13;
	dPiece[74 + DBORDERX][54 + DBORDERY] = 0x118;
	SetDungeonMicros();
}

static void T_CryptClosed()
{
	dPiece[26 + DBORDERX][11 + DBORDERY] = 0x52b;
	dPiece[27 + DBORDERX][11 + DBORDERY] = 0x52c;
	dPiece[26 + DBORDERX][12 + DBORDERY] = 0x52d;
	dPiece[27 + DBORDERX][12 + DBORDERY] = 0x52e;
	dPiece[26 + DBORDERX][13 + DBORDERY] = 0x52f;
	dPiece[27 + DBORDERX][13 + DBORDERY] = 0x530;
	dPiece[26 + DBORDERX][14 + DBORDERY] = 0x531;
	dPiece[27 + DBORDERX][14 + DBORDERY] = 0x532;
	dPiece[25 + DBORDERX][11 + DBORDERY] = 0x53b;
	dPiece[24 + DBORDERX][11 + DBORDERY] = 0x53c;
	SetDungeonMicros();
}

void T_CryptOpen()
{
	dPiece[26 + DBORDERX][11 + DBORDERY] = 0x533;
	dPiece[27 + DBORDERX][11 + DBORDERY] = 0x534;
	dPiece[26 + DBORDERX][12 + DBORDERY] = 0x535;
	dPiece[27 + DBORDERX][12 + DBORDERY] = 0x536;
	dPiece[26 + DBORDERX][13 + DBORDERY] = 0x537;
	dPiece[27 + DBORDERX][13 + DBORDERY] = 0x538;
	dPiece[26 + DBORDERX][14 + DBORDERY] = 0x539;
	dPiece[27 + DBORDERX][14 + DBORDERY] = 0x53a;
	dPiece[25 + DBORDERX][11 + DBORDERY] = 0x53b;
	dPiece[24 + DBORDERX][11 + DBORDERY] = 0x53c;
	SetDungeonMicros();
}

/** Return the available town-warps for the current player
*/
unsigned char GetOpenWarps()
{
	unsigned char twarps;
#ifdef SPAWN
	townwarps = 0;
#else
	if (gbMaxPlayers == 1) {
		twarps = plr[myplr].pTownWarps;
#ifdef HELLFIRE
		if (plr[myplr]._pLevel >= 10)
			twarps |= 1;
		if (plr[myplr]._pLevel >= 15)
			twarps |= 2;
		if (plr[myplr]._pLevel >= 20)
			twarps |= 4;
#endif
	} else {
		twarps = 0xFF;
	}
#endif
	return twarps;
}

/**
 * @brief Initialize all of the levels data
 */
void T_Pass3()
{
	int x;
	BYTE *P3Tiles, *pSector;
	unsigned char twarps;

	memset(dPiece, 0, sizeof(dPiece));

	P3Tiles = LoadFileInMem("Levels\\TownData\\Town.TIL", NULL);
	pSector = LoadFileInMem("Levels\\TownData\\Sector1s.DUN", NULL);
	T_FillSector(P3Tiles, pSector, 36 + DBORDERX, 36 + DBORDERY, 25, 25);
	mem_free_dbg(pSector);
	pSector = LoadFileInMem("Levels\\TownData\\Sector2s.DUN", NULL);
	T_FillSector(P3Tiles, pSector, 36 + DBORDERX, -10 + DBORDERY, 25, 23);
	mem_free_dbg(pSector);
	pSector = LoadFileInMem("Levels\\TownData\\Sector3s.DUN", NULL);
	T_FillSector(P3Tiles, pSector, -10 + DBORDERX, 36 + DBORDERY, 23, 25);
	mem_free_dbg(pSector);
	pSector = LoadFileInMem("Levels\\TownData\\Sector4s.DUN", NULL);
	T_FillSector(P3Tiles, pSector, -10 + DBORDERX, -10 + DBORDERY, 23, 23);
	mem_free_dbg(pSector);

#ifdef HELLFIRE
	if (quests[Q_FARMER]._qactive == QUEST_DONE || quests[Q_FARMER]._qactive == 10
	 || quests[Q_JERSEY]._qactive == QUEST_DONE || quests[Q_JERSEY]._qactive == 10)
		T_HiveOpen();
	else
		T_HiveClosed();
	if (quests[Q_GRAVE]._qactive == QUEST_DONE || plr[myplr]._pLvlVisited[21])
		T_CryptOpen();
	else
		T_CryptClosed();
#endif

	twarps = GetOpenWarps();
	if (!(twarps & 1))
		T_FillTile(P3Tiles, 38 + DBORDERX, 10 + DBORDERY, 320);
	if (!(twarps & 2)) {
		T_FillTile(P3Tiles, 6 + DBORDERX, 58 + DBORDERY, 332);
		T_FillTile(P3Tiles, 6 + DBORDERX, 60 + DBORDERY, 331);
	}
	if (!(twarps & 4)) {
		for (x = 26 + DBORDERX; x < 36 + DBORDERX; x++) {
			T_FillTile(P3Tiles, x, 68 + DBORDERY, random_(0, 4) + 1);
		}
	}

	if (quests[Q_PWATER]._qactive != QUEST_DONE && quests[Q_PWATER]._qactive) {
		T_FillTile(P3Tiles, 50 + DBORDERX, 60 + DBORDERY, 342);
	} else {
		T_FillTile(P3Tiles, 50 + DBORDERX, 60 + DBORDERY, 71);
	}

	mem_free_dbg(P3Tiles);
}

/**
 * @brief Initialize town level
 * @param entry Methode of entry
 */
void CreateTown(int entry)
{
	int x, y, pn;

	DRLG_InitTrans();
	DRLG_Init_Globals();

	if (entry == ENTRY_MAIN) { // New game
		ViewX = 65 + DBORDERX;
		ViewY = 58 + DBORDERY;
	} else if (entry == ENTRY_PREV) { // Cathedral
		ViewX = 15 + DBORDERX;
		ViewY = 21 + DBORDERY;
	} else if (entry == ENTRY_TWARPUP) {
		if (TWarpFrom == 5) {
			ViewX = 39 + DBORDERX;
			ViewY = 12 + DBORDERY;
		} else if (TWarpFrom == 9) {
			ViewX = 8 + DBORDERX;
			ViewY = 59 + DBORDERY;
		} else if (TWarpFrom == 13) {
			ViewX = 31 + DBORDERX;
			ViewY = 71 + DBORDERY;
#ifdef HELLFIRE
		} else if (TWarpFrom == 17) {
			ViewX = 69 + DBORDERX;
			ViewY = 52 + DBORDERY;
		} else if (TWarpFrom == 21) {
			ViewX = 26 + DBORDERX;
			ViewY = 15 + DBORDERY;
#endif
		}
	}

	T_Pass3();
	memset(dLight, 0, sizeof(dLight));
	memset(dFlags, 0, sizeof(dFlags));
	memset(dPlayer, 0, sizeof(dPlayer));
	memset(dMonster, 0, sizeof(dMonster));
	memset(dObject, 0, sizeof(dObject));
	memset(dItem, 0, sizeof(dItem));
	memset(dSpecial, 0, sizeof(dSpecial));

	for (y = 0; y < MAXDUNY; y++) {
		for (x = 0; x < MAXDUNX; x++) {
			pn = dPiece[x][y];
			if (pn == 360) {
				dSpecial[x][y] = 1;
			} else if (pn == 358) {
				dSpecial[x][y] = 2;
			} else if (pn == 129) {
				dSpecial[x][y] = 6;
			} else if (pn == 130) {
				dSpecial[x][y] = 7;
			} else if (pn == 128) {
				dSpecial[x][y] = 8;
			} else if (pn == 117) {
				dSpecial[x][y] = 9;
			} else if (pn == 157) {
				dSpecial[x][y] = 10;
			} else if (pn == 158) {
				dSpecial[x][y] = 11;
			} else if (pn == 156) {
				dSpecial[x][y] = 12;
			} else if (pn == 162) {
				dSpecial[x][y] = 13;
			} else if (pn == 160) {
				dSpecial[x][y] = 14;
			} else if (pn == 214) {
				dSpecial[x][y] = 15;
			} else if (pn == 212) {
				dSpecial[x][y] = 16;
			} else if (pn == 217) {
				dSpecial[x][y] = 17;
			} else if (pn == 216) {
				dSpecial[x][y] = 18;
			}
		}
	}
}

DEVILUTION_END_NAMESPACE
