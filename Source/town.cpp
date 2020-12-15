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
	/*int miniset[9][6] = {
		//  50     51     52     53     54     55
		{0x48a, 0x48b, 0x4ee, 0x4f0, 0x11a, 0x11c}, // 68
		{0x48c, 0x50e, 0x510, 0x511, 0x512, 0x11d}, // 69
		{0x513, 0x515, 0x51b, 0x51d, 0x51f, 0x521}, // 70
		{    0, 0x516, 0x51c, 0x51e, 0x520, 0x522}, // 71
		{0x517, 0x519, 0x523, 0x525, 0x527, 0x529}, // 72
		{0x518, 0x51a, 0x524, 0x526, 0x528, 0x52a}, // 73
		{    0, 0x118, 0x118, 0x118, 0x118,     0}, // 74
		{0x118, 0x118, 0x013, 0x008, 0x008,     0}, // 75
		{0x0d9, 0x018,     0,     0,     0,     0}, // 76
	};*/
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
	/*int miniset[9][6] = {
		//  50     51     52     53     54     55
		{0x48a, 0x4ec, 0x4ee, 0x4f0, 0x4f2, 0x4f4}, // 68
		{0x4eb, 0x4ed, 0x4ef, 0x4f1, 0x4f3,     0}, // 69
		{0x4f5, 0x4f7, 0x4fd, 0x4ff, 0x501, 0x503}, // 70
		{0x4f6, 0x4f8, 0x4fe, 0x500, 0x502, 0x504}, // 71
		{0x4f9, 0x4fb, 0x505, 0x507, 0x509, 0x50b}, // 72
		{0x4fa, 0x4fc, 0x506, 0x508, 0x50a, 0x50c}, // 73
		{    0, 0x118, 0x118, 0x118, 0x118,     0}, // 74
		{0x118, 0x118, 0x013, 0x008, 0x008,     0}, // 75
		{0x0d9, 0x018,     0,     0,     0,     0}, // 76
	};*/
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
	/*int miniset[4][4] = {
		//  11     12     13     14
		{0x53c,     0,     0,     0}, // 24
		{0x53b,     0,     0,     0}, // 25
		{0x52b, 0x52d, 0x52f, 0x531}, // 26
		{0x52c, 0x52e, 0x530, 0x532}, // 27
	};*/
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
	/*char miniset[4][4] = {
		//  11     12     13     14
		{0x53c,     0,     0,     0}, // 24
		{0x53b,     0,     0,     0}, // 25
		{0x533, 0x535, 0x537, 0x539}, // 26
		{0x534, 0x536, 0x538, 0x53a}, // 27
	};*/
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

#define REPLACEMENT 341
void InitTown()
{
	int i, j;

	// make the whole town visible
	for (i = 0; i < MAXDUNX; i++) {
		for (j = 0; j < MAXDUNY; j++)
			dFlags[i][j] |= BFLAG_LIT;
	}

	// prevent the player from teleporting inside of buildings
	nSolidTable[521] = TRUE;
	nSolidTable[522] = TRUE;
	nSolidTable[523] = TRUE;
	nSolidTable[524] = TRUE;
	nSolidTable[539] = TRUE;
	nSolidTable[551] = TRUE;
	nSolidTable[552] = TRUE;

	// player's home
	dPiece[DBORDERX + 62][DBORDERY + 60] = REPLACEMENT;
	dPiece[DBORDERX + 62][DBORDERY + 61] = REPLACEMENT;
	dPiece[DBORDERX + 63][DBORDERY + 60] = REPLACEMENT;
	dPiece[DBORDERX + 63][DBORDERY + 61] = REPLACEMENT;

	// beggar
	for (i = DBORDERX + 58; i <= DBORDERX + 61; i++)
		for (j = DBORDERY + 70; j <= DBORDERY + 71; j++)
			dPiece[i][j] = REPLACEMENT;
	dPiece[DBORDERX + 60][DBORDERY + 69] = REPLACEMENT;
	dPiece[DBORDERX + 61][DBORDERY + 69] = REPLACEMENT;

	// healer
	dPiece[DBORDERX + 40][DBORDERY + 68] = REPLACEMENT;
	dPiece[DBORDERX + 40][DBORDERY + 69] = REPLACEMENT;
	dPiece[DBORDERX + 41][DBORDERY + 68] = REPLACEMENT;
	dPiece[DBORDERX + 41][DBORDERY + 69] = REPLACEMENT;
	dPiece[DBORDERX + 42][DBORDERY + 66] = REPLACEMENT;
	dPiece[DBORDERX + 42][DBORDERY + 67] = REPLACEMENT;
	dPiece[DBORDERX + 43][DBORDERY + 66] = REPLACEMENT;
	dPiece[DBORDERX + 43][DBORDERY + 67] = REPLACEMENT;

	// barmaid
	for (i = DBORDERX + 28; i <= DBORDERX + 31; i++)
		for (j = DBORDERY + 56; j <= DBORDERY + 57; j++)
			dPiece[i][j] = REPLACEMENT;
	dPiece[DBORDERX + 30][DBORDERY + 54] = REPLACEMENT;
	dPiece[DBORDERX + 30][DBORDERY + 55] = REPLACEMENT;
	dPiece[DBORDERX + 31][DBORDERY + 54] = REPLACEMENT;
	dPiece[DBORDERX + 31][DBORDERY + 55] = REPLACEMENT;

	// ogden
	for (i = DBORDERX + 38; i <= DBORDERX + 41; i++)
		for (j = DBORDERY + 48; j <= DBORDERY + 49; j++)
			dPiece[i][j] = REPLACEMENT;
	dPiece[DBORDERX + 38][DBORDERY + 50] = REPLACEMENT;
	dPiece[DBORDERX + 38][DBORDERY + 51] = REPLACEMENT;
	dPiece[DBORDERX + 39][DBORDERY + 50] = REPLACEMENT;
	dPiece[DBORDERX + 39][DBORDERY + 51] = REPLACEMENT;

	// smith
	for (i = DBORDERX + 53; i <= DBORDERX + 59; i++)
		for (j = DBORDERY + 48; j <= DBORDERY + 49; j++)
			dPiece[i][j] = REPLACEMENT;
	dPiece[DBORDERY + 51][DBORDERY + 51] = REPLACEMENT;

	dPiece[DBORDERX + 51][DBORDERY + 50] = REPLACEMENT;
	dPiece[DBORDERX + 50][DBORDERY + 50] = REPLACEMENT;
	dPiece[DBORDERX + 50][DBORDERY + 51] = REPLACEMENT;

	// house at the portals
	for (i = DBORDERX + 36; i <= DBORDERX + 43; i++)
		for (j = DBORDERY + 34; j <= DBORDERY + 35; j++)
			dPiece[i][j] = REPLACEMENT;
	dPiece[DBORDERX + 38][DBORDERY + 32] = REPLACEMENT;
	dPiece[DBORDERX + 38][DBORDERY + 33] = REPLACEMENT;
	dPiece[DBORDERX + 39][DBORDERY + 32] = REPLACEMENT;
	dPiece[DBORDERX + 39][DBORDERY + 33] = REPLACEMENT;

	// witch
	for (i = DBORDERX + 66; i <= DBORDERX + 67; i++)
		for (j = DBORDERY + 8; j <= DBORDERY + 9; j++)
			dPiece[i][j] = REPLACEMENT;

	// cathedral - base
	for (i = DBORDERX + 10; i <= DBORDERX + 19; i++)
		for (j = DBORDERY + 6; j <= DBORDERY + 15; j++)
			// skip (DBORDERX + 10:DBORDERY + 10)-(DBORDERX + 11:DBORDERY + 11),
			//      (DBORDERX + 12:DBORDERX +  6)-(DBORDERX + 13:DBORDERY + 9),
			//      (DBORDERX + 18:DBORDERY +  6)-(DBORDERX + 19:DBORDERY + 7)
			if (dPiece[i][j] == 0) {
				dPiece[i][j] = REPLACEMENT;
			}
	//           - front
	for (i = DBORDERX + 12; i <= DBORDERX + 17; i++)
		for (j = DBORDERY + 16; j <= DBORDERY + 17; j++)
			dPiece[i][j] = REPLACEMENT;

	//           - back
	for (i = DBORDERX + 14; i <= DBORDERX + 15; i++)
		for (j = DBORDERY + 3; j <= DBORDERY + 5; j++)
			dPiece[i][j] = REPLACEMENT;
	for (i = DBORDERX + 6; i <= DBORDERX + 11; i++)
		for (j = DBORDERY + 3; j <= DBORDERY + 5; j++)
			dPiece[i][j] = REPLACEMENT;

	//            - tower
	for (i = DBORDERX + 2; i <= DBORDERX + 4; i++)
		for (j = DBORDERY; j <= DBORDERY + 1; j++)
			dPiece[i][j] = 1018;
	for (i = DBORDERX + 1; i <= DBORDERX + 3; i++)
		for (j = DBORDERY + 2; j <= DBORDERY + 3; j++)
			dPiece[i][j] = 1018;
	for (i = DBORDERX + 4; i <= DBORDERX + 5; i++)
		for (j = DBORDERY + 4; j <= DBORDERY + 5; j++)
			dPiece[i][j] = 958;
	for (i = DBORDERX + 6; i <= DBORDERX + 9; i++)
		for (j = DBORDERY + 6; j <= DBORDERY + 7; j++)
			dPiece[i][j] = REPLACEMENT;
	dPiece[DBORDERX + 0][DBORDERY + 2] = 1026;
	dPiece[DBORDERX + 5][DBORDERY + 1] = 1026;
	dPiece[DBORDERX + 6][DBORDERY + 2] = 1027;
	dPiece[DBORDERX + 4][DBORDERY + 6] = 958;
	dPiece[DBORDERX + 5][DBORDERY + 6] = 958;
	dPiece[DBORDERX + 7][DBORDERY + 8] = REPLACEMENT;

	SetDungeonMicros();

	InitTowners();
}

/**
 * Return the available town-warps for the current player
 */
unsigned char GetOpenWarps()
{
	unsigned char twarps = 1 << TWARP_CATHEDRAL;
#ifndef SPAWN
	if (gbMaxPlayers == 1) {
		twarps |= plr[myplr].pTownWarps << 1;
#ifdef HELLFIRE
		if (plr[myplr]._pLevel >= 10)
			twarps |= 1 << TWARP_CATACOMB;
		if (plr[myplr]._pLevel >= 15)
			twarps |= 1 << TWARP_CAVES;
		if (plr[myplr]._pLevel >= 20)
			twarps |= 1 << TWARP_HELL;
#endif
	} else {
		twarps |= (1 << TWARP_CATACOMB) | (1 << TWARP_CAVES) | (1 << TWARP_HELL);
	}
#endif
#ifdef HELLFIRE
	if (quests[Q_FARMER]._qactive == QUEST_DONE || quests[Q_FARMER]._qactive == 10
	 || quests[Q_JERSEY]._qactive == QUEST_DONE || quests[Q_JERSEY]._qactive == 10)
		twarps |= 1 << TWARP_HIVE;
	if (quests[Q_GRAVE]._qactive == QUEST_DONE || plr[myplr]._pLvlVisited[21])
		twarps |= 1 << TWARP_CRYPT;
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

	twarps = GetOpenWarps();
	if (!(twarps & (1 << TWARP_CATACOMB)))
		T_FillTile(P3Tiles, 38 + DBORDERX, 10 + DBORDERY, 320);
	if (!(twarps & (1 << TWARP_CAVES))) {
		T_FillTile(P3Tiles, 6 + DBORDERX, 58 + DBORDERY, 332);
		T_FillTile(P3Tiles, 6 + DBORDERX, 60 + DBORDERY, 331);
	}
	if (!(twarps & (1 << TWARP_HELL))) {
		for (x = 26 + DBORDERX; x < 36 + DBORDERX; x++) {
			T_FillTile(P3Tiles, x, 68 + DBORDERY, RandRange(1, 4));
		}
	}
#ifdef HELLFIRE
	if (!(twarps & (1 << TWARP_HIVE)))
		T_HiveClosed();
	else
		T_HiveOpen();
	if (!(twarps & (1 << TWARP_CRYPT)))
		T_CryptClosed();
	else
		T_CryptOpen();
#endif

	if (quests[Q_PWATER]._qactive != QUEST_DONE && quests[Q_PWATER]._qactive != QUEST_NOTAVAIL) {
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
