/**
 * @file town.h
 *
 * Implementation of functionality for rendering the town, towners and calling other render routines.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Load a tile in to dPiece
 * @param xx upper left destination
 * @param yy upper left destination
 * @param mt mega tile id (-1)
 */
static void T_FillTile(int xx, int yy, int mt)
{
	int v1, v2, v3, v4;
	uint16_t* pTile;

	pTile = &pTiles[mt * 4];
	v1 = SwapLE16(pTile[0]) + 1;
	v2 = SwapLE16(pTile[1]) + 1;
	v3 = SwapLE16(pTile[2]) + 1;
	v4 = SwapLE16(pTile[3]) + 1;

	dPiece[xx][yy] = v1;
	dPiece[xx + 1][yy] = v2;
	dPiece[xx][yy + 1] = v3;
	dPiece[xx + 1][yy + 1] = v4;
}

#ifdef HELLFIRE
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
		{0x008, 0x118, 0x118, 0x118, 0x118,     0}, // 74
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
	dPiece[74 + DBORDERX][50 + DBORDERY] = 0x8; // new
	dPiece[74 + DBORDERX][51 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][52 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][53 + DBORDERY] = 0x118;
	//dPiece[75 + DBORDERX][50 + DBORDERY] = 0x118;
	//dPiece[75 + DBORDERX][51 + DBORDERY] = 0x118;
	//dPiece[75 + DBORDERX][53 + DBORDERY] = 8;
	//dPiece[75 + DBORDERX][54 + DBORDERY] = 8;
	//dPiece[76 + DBORDERX][50 + DBORDERY] = 0xd9;
	//dPiece[76 + DBORDERX][51 + DBORDERY] = 0x18;
	//dPiece[75 + DBORDERX][52 + DBORDERY] = 0x13;
	dPiece[74 + DBORDERX][54 + DBORDERY] = 0x118;
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
		{0x008, 0x118, 0x118, 0x118, 0x118,     0}, // 74
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
	dPiece[74 + DBORDERX][50 + DBORDERY] = 0x8; //new
	dPiece[74 + DBORDERX][51 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][52 + DBORDERY] = 0x118;
	dPiece[74 + DBORDERX][53 + DBORDERY] = 0x118;
	//dPiece[75 + DBORDERX][50 + DBORDERY] = 0x118;
	//dPiece[75 + DBORDERX][51 + DBORDERY] = 0x118;
	//dPiece[75 + DBORDERX][53 + DBORDERY] = 8;
	//dPiece[75 + DBORDERX][54 + DBORDERY] = 8;
	//dPiece[76 + DBORDERX][50 + DBORDERY] = 0xd9;
	//dPiece[76 + DBORDERX][51 + DBORDERY] = 0x18;
	//dPiece[75 + DBORDERX][52 + DBORDERY] = 0x13;
	dPiece[74 + DBORDERX][54 + DBORDERY] = 0x118;
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
}
#endif

/**
 * Return the available town-warps for the current player
 */
static BYTE GetOpenWarps()
{
	BYTE twarps = 1 << TWARP_CATHEDRAL;
	if (IsMultiGame) {
		twarps |= (1 << TWARP_CATACOMB) | (1 << TWARP_CAVES) | (1 << TWARP_HELL);
	} else {
		twarps |= gbTownWarps;
	}
#ifdef HELLFIRE
	if (quests[Q_FARMER]._qactive == QUEST_DONE || quests[Q_JERSEY]._qactive == QUEST_DONE)
		twarps |= 1 << TWARP_NEST;
	if (quests[Q_GRAVE]._qactive == QUEST_DONE)
		twarps |= 1 << TWARP_CRYPT;
#endif
	return twarps;
}

/**
 * @brief Initialize all of the levels data
 */
static void T_Pass3()
{
	int x;
#if INT_MAX == INT32_MAX && SDL_BYTEORDER != SDL_BIG_ENDIAN
	LoadFileWithMem("Levels\\TownData\\Town.RDUN", (BYTE*)&dPiece[0][0]);
#else
	uint32_t* pBuf = (uint32_t*)LoadFileInMem("Levels\\TownData\\Town.RDUN");
	int* dp = &dPiece[0][0];
	uint32_t* pTmp = pBuf;
	for (x = 0; x < MAXDUNX * MAXDUNY; x++, dp++, pTmp++)
		*dp = SwapLE32(*pTmp);

	mem_free_dbg(pBuf);
#endif

	if (quests[Q_PWATER]._qvar1 != QV_PWATER_CLEAN) {
		// T_FillTile(50 + DBORDERX, 60 + DBORDERY, 342 - 1);
		dPiece[50 + DBORDERX][60 + DBORDERY] = 1257;
		dPiece[50 + DBORDERX + 1][60 + DBORDERY] = 259;
		dPiece[50 + DBORDERX][60 + DBORDERY + 1] = 1258;
		dPiece[50 + DBORDERX + 1][60 + DBORDERY + 1] = 261;
	}

	gbOpenWarps = GetOpenWarps();
	if (!(gbOpenWarps & (1 << TWARP_CATACOMB))) {
		// T_FillTile(38 + DBORDERX, 10 + DBORDERY, 320 - 1);
		dPiece[38 + DBORDERX][10 + DBORDERY] = 1171;
		dPiece[38 + DBORDERX + 1][10 + DBORDERY] = 1172;
		dPiece[38 + DBORDERX][10 + DBORDERY + 1] = 1173;
		dPiece[38 + DBORDERX + 1][10 + DBORDERY + 1] = 1174;
	}
	if (!(gbOpenWarps & (1 << TWARP_CAVES))) {
		// T_FillTile(6 + DBORDERX, 58 + DBORDERY, 332 - 1);
		dPiece[6 + DBORDERX][58 + DBORDERY] = 1217;
		dPiece[6 + DBORDERX + 1][58 + DBORDERY] = 1218;
		dPiece[6 + DBORDERX][58 + DBORDERY + 1] = 1219;
		dPiece[6 + DBORDERX + 1][58 + DBORDERY + 1] = 1220;
		dPiece[4 + DBORDERX + 1][58 + DBORDERY] = 427;
		dPiece[6 + DBORDERX + 1][58 + DBORDERY + 2] = 1215;
		// T_FillTile(6 + DBORDERX, 60 + DBORDERY, 331 - 1);
	}
	if (!(gbOpenWarps & (1 << TWARP_HELL))) {
		for (x = 26 + DBORDERX; x < 36 + DBORDERX; x += 2) {
			T_FillTile(x, 68 + DBORDERY, random_(0, 4));
		}
	}

#ifdef HELLFIRE
	if (!(gbOpenWarps & (1 << TWARP_NEST)))
		T_HiveClosed();
	else
		T_HiveOpen();
	if (!(gbOpenWarps & (1 << TWARP_CRYPT)))
		T_CryptClosed();
	else
		T_CryptOpen();
#endif
}

/**
 * @brief Initialize town level
 */
void CreateTown()
{
	int i, *dp;
	BYTE pc, *dsp;

	DRLG_InitTrans();
	DRLG_Init_Globals();

	T_Pass3();

	// make the whole town lit
	memset(dLight, 0, sizeof(dLight));
	// make the whole town visible
	memset(dFlags, BFLAG_VISIBLE, sizeof(dFlags));

	static_assert(sizeof(dPiece) == MAXDUNX * MAXDUNY * sizeof(int), "Linear traverse of dPiece does not work in CreateTown.");
	static_assert(sizeof(dSpecial) == MAXDUNX * MAXDUNY, "Linear traverse of dSpecial does not work in CreateTown.");
	dsp = &dSpecial[0][0];
	dp = &dPiece[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, dsp++, dp++) {
		/*if (*dp == 360)
			pc = 1;
		else if (*dp == 358)
			pc = 2;
		else*/ if (*dp == 129)
			pc = 6;
		else if (*dp == 130)
			pc = 7;
		else if (*dp == 128)
			pc = 8;
		else if (*dp == 117)
			pc = 9;
		else if (*dp == 157)
			pc = 10;
		else if (*dp == 158)
			pc = 11;
		else if (*dp == 156)
			pc = 12;
		else if (*dp == 162)
			pc = 13;
		else if (*dp == 160)
			pc = 14;
		//else if (*dp == 214)
		//	pc = 15;
		else if (*dp == 212)
			pc = 16;
		//else if (*dp == 217)
		//	pc = 17;
		else if (*dp == 216)
			pc = 18;
		else
			continue;
		*dsp = pc;
	}
}

DEVILUTION_END_NAMESPACE
