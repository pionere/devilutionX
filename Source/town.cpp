/**
 * @file town.h
 *
 * Implementation of functionality for rendering the town, towners and calling other render routines.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef HELLFIRE
void T_HiveOpen()
{
	/*int miniset[9][6] = {
		//  50    51    52    53    54    55
		{  286,  288, 1262, 1264,  282,  284}, // 68
		{  295, 1294, 1296, 1297, 1298,  285}, // 69
		{ 1299, 1301, 1307, 1309, 1311, 1313}, // 70
		{ 1300, 1302, 1308, 1310, 1312, 1314}, // 71
		{ 1303, 1305, 1315, 1317, 1319, 1321}, // 72
		{ 1304, 1306, 1316, 1318, 1320, 1322}, // 73
		{    8,  280,  280,  280,  280,    0}, // 74
		{  280,  280,   19,    8,    8,    0}, // 75
		{  217,   24,    0,    0,    0,    0}, // 76
	};*/
	// dPiece[68 + DBORDERX][50 + DBORDERY] = 187; // 286
	dPiece[69 + DBORDERX][50 + DBORDERY] = 195; // 1163 // 295
	// dPiece[68 + DBORDERX][51 + DBORDERY] = 189; // 288
	dPiece[69 + DBORDERX][51 + DBORDERY] = 822; // 1294
	dPiece[68 + DBORDERX][52 + DBORDERY] = 799; // 1262
	dPiece[69 + DBORDERX][52 + DBORDERY] = 823; // 1296
	dPiece[68 + DBORDERX][53 + DBORDERY] = 801; // 1264
	dPiece[69 + DBORDERX][53 + DBORDERY] = 824; // 1297
	// dPiece[68 + DBORDERX][54 + DBORDERY] = 183; // 282
	// dPiece[69 + DBORDERX][54 + DBORDERY] = ???; // 1298
	// dPiece[68 + DBORDERX][55 + DBORDERY] = 186; // 285
	dPiece[69 + DBORDERX][55 + DBORDERY] = 186; // 285
	dPiece[70 + DBORDERX][50 + DBORDERY] = 825; // 1299
	dPiece[71 + DBORDERX][50 + DBORDERY] = 826; // 1300
	dPiece[70 + DBORDERX][51 + DBORDERY] = 827; // 1301
	dPiece[71 + DBORDERX][51 + DBORDERY] = 828; // 1302
	dPiece[72 + DBORDERX][50 + DBORDERY] = 829; // 1303
	dPiece[73 + DBORDERX][50 + DBORDERY] = 830; // 1304
	dPiece[72 + DBORDERX][51 + DBORDERY] = 831; // 1305
	dPiece[73 + DBORDERX][51 + DBORDERY] = 832; // 1306
	dPiece[70 + DBORDERX][52 + DBORDERY] = 833; // 1307
	dPiece[71 + DBORDERX][52 + DBORDERY] = 834; // 1308
	dPiece[70 + DBORDERX][53 + DBORDERY] = 835; // 1309
	dPiece[71 + DBORDERX][53 + DBORDERY] = 836; // 1310
	dPiece[70 + DBORDERX][54 + DBORDERY] = 837; // 1311
	dPiece[71 + DBORDERX][54 + DBORDERY] = 838; // 1312
	dPiece[70 + DBORDERX][55 + DBORDERY] = 839; // 1313
	dPiece[71 + DBORDERX][55 + DBORDERY] = 840; // 1314
	dPiece[72 + DBORDERX][54 + DBORDERY] = 845; // 1319
	dPiece[73 + DBORDERX][54 + DBORDERY] = 846; // 1320
	dPiece[72 + DBORDERX][55 + DBORDERY] = 847; // 1321
	// dPiece[73 + DBORDERX][55 + DBORDERY] = ???; // 1322
	dPiece[72 + DBORDERX][52 + DBORDERY] = 841; // 1315
	dPiece[73 + DBORDERX][52 + DBORDERY] = 842; // 1316
	dPiece[72 + DBORDERX][53 + DBORDERY] = 843; // 1317
	dPiece[73 + DBORDERX][53 + DBORDERY] = 844; // 1318
	dPiece[74 + DBORDERX][50 + DBORDERY] = 8; // new // 8
	dPiece[74 + DBORDERX][51 + DBORDERY] = 1; // 1
	dPiece[74 + DBORDERX][52 + DBORDERY] = 328; // 489
	dPiece[74 + DBORDERX][53 + DBORDERY] = 1; // 1
	//dPiece[75 + DBORDERX][50 + DBORDERY] = ???; // 280
	//dPiece[75 + DBORDERX][51 + DBORDERY] = ???; // 280
	//dPiece[75 + DBORDERX][53 + DBORDERY] = 8; // 8
	//dPiece[75 + DBORDERX][54 + DBORDERY] = 8; // 8
	//dPiece[76 + DBORDERX][50 + DBORDERY] = 142; // 217
	//dPiece[76 + DBORDERX][51 + DBORDERY] = 24; // 24
	//dPiece[75 + DBORDERX][52 + DBORDERY] = 19; // 19
	dPiece[74 + DBORDERX][54 + DBORDERY] = 328; // 489
}

static void T_HiveClosed()
{
	/*int miniset[9][6] = {
		//  50    51    52    53    54    55
		{ 1162, 1260, 1262, 1264, 1266, 1268}, // 68
		{ 1259, 1261, 1263, 1265, 1267,    0}, // 69
		{ 1269, 1271, 1277, 1279, 1281, 1283}, // 70
		{ 1270, 1272, 1278, 1280, 1282, 1284}, // 71
		{ 1273, 1275, 1285, 1287, 1289, 1291}, // 72
		{ 1274, 1276, 1286, 1288, 1290, 1292}, // 73
		{    8,  280,  280,  280,  280,    0}, // 74
		{  280,  280,   19,    8,    8,    0}, // 75
		{  217,   24,    0,    0,    0,    0}, // 76
	};*/
	// dPiece[68 + DBORDERX][50 + DBORDERY] = ???; // 1162
	dPiece[69 + DBORDERX][50 + DBORDERY] = 797; // 1259
	// dPiece[68 + DBORDERX][51 + DBORDERY] = ???; // 1260
	dPiece[69 + DBORDERX][51 + DBORDERY] = 798; // 1261
	dPiece[68 + DBORDERX][52 + DBORDERY] = 799; // 1262
	dPiece[69 + DBORDERX][52 + DBORDERY] = 800; // 1263
	dPiece[68 + DBORDERX][53 + DBORDERY] = 801; // 1264
	dPiece[69 + DBORDERX][53 + DBORDERY] = 802; // 1265
	// dPiece[68 + DBORDERX][54 + DBORDERY] = ???;  // 1266
	// dPiece[69 + DBORDERX][54 + DBORDERY] = ???;  // 1267
	// dPiece[68 + DBORDERX][55 + DBORDERY] = ???;  // 1268
	dPiece[70 + DBORDERX][50 + DBORDERY] = 803; // 1269
	dPiece[71 + DBORDERX][50 + DBORDERY] = 804; // 1270
	dPiece[70 + DBORDERX][51 + DBORDERY] = 805; // 1271
	dPiece[71 + DBORDERX][51 + DBORDERY] = 806; // 1272
	dPiece[72 + DBORDERX][50 + DBORDERY] = 807; // 1273
	dPiece[73 + DBORDERX][50 + DBORDERY] = 4; // new (1274) // 4
	dPiece[72 + DBORDERX][51 + DBORDERY] = 808; // 1275
	dPiece[73 + DBORDERX][51 + DBORDERY] = 809; // 1276
	dPiece[70 + DBORDERX][52 + DBORDERY] = 810; // 1277
	dPiece[71 + DBORDERX][52 + DBORDERY] = 811; // 1278
	dPiece[70 + DBORDERX][53 + DBORDERY] = 812; // 1279
	dPiece[71 + DBORDERX][53 + DBORDERY] = 813; // 1280
	dPiece[70 + DBORDERX][54 + DBORDERY] = 814; // 1281
	dPiece[71 + DBORDERX][54 + DBORDERY] = 815; // 1282
	// dPiece[70 + DBORDERX][55 + DBORDERY] = ???; // 1283
	// dPiece[71 + DBORDERX][55 + DBORDERY] = ???; // 1284
	dPiece[72 + DBORDERX][54 + DBORDERY] = 820; // 1289
	dPiece[73 + DBORDERX][54 + DBORDERY] = 821; // 1290
	// dPiece[72 + DBORDERX][55 + DBORDERY] = ???; // 1291
	// dPiece[73 + DBORDERX][55 + DBORDERY] = ???; // 1292
	dPiece[72 + DBORDERX][52 + DBORDERY] = 816; // 1285
	dPiece[73 + DBORDERX][52 + DBORDERY] = 817; // 1286
	dPiece[72 + DBORDERX][53 + DBORDERY] = 818; // 1287
	dPiece[73 + DBORDERX][53 + DBORDERY] = 819; // 1288
	dPiece[74 + DBORDERX][50 + DBORDERY] = 8; // new // 8
	dPiece[74 + DBORDERX][51 + DBORDERY] = 1; // (280) // 1
	dPiece[74 + DBORDERX][52 + DBORDERY] = 328; // (280) // 489
	dPiece[74 + DBORDERX][53 + DBORDERY] = 1; // (280) // 1
	//dPiece[75 + DBORDERX][50 + DBORDERY] = ???; // 280
	//dPiece[75 + DBORDERX][51 + DBORDERY] = ???; // 280
	//dPiece[75 + DBORDERX][53 + DBORDERY] = 8; // 8
	//dPiece[75 + DBORDERX][54 + DBORDERY] = 8; // 8
	//dPiece[76 + DBORDERX][50 + DBORDERY] = 142; // 217
	//dPiece[76 + DBORDERX][51 + DBORDERY] = 24; // 24
	//dPiece[75 + DBORDERX][52 + DBORDERY] = 19; // 19
	dPiece[74 + DBORDERX][54 + DBORDERY] = 328; // (280) // 489
}

static void T_CryptClosed()
{
	/*int miniset[4][4] = {
		//  11    12    13    14
		{ 1340,    0,    0,    0 }, // 24
		{ 1339,    0,    0,    0 }, // 25
		{ 1323, 1325, 1327, 1329 }, // 26
		{ 1324, 1326, 1328, 1330 }, // 27
	};*/
	dPiece[26 + DBORDERX][11 + DBORDERY] = 848; // 1323
	dPiece[27 + DBORDERX][11 + DBORDERY] = 849; // 1324
	dPiece[26 + DBORDERX][12 + DBORDERY] = 850; // 1325
	dPiece[27 + DBORDERX][12 + DBORDERY] = 851; // 1326
	dPiece[26 + DBORDERX][13 + DBORDERY] = 852; // 1327
	dPiece[27 + DBORDERX][13 + DBORDERY] = 853; // 1328
	dPiece[26 + DBORDERX][14 + DBORDERY] = 854; // 1329
	dPiece[27 + DBORDERX][14 + DBORDERY] = 855; // 1330
	dPiece[25 + DBORDERX][11 + DBORDERY] = 864; // 1339
	dPiece[24 + DBORDERX][11 + DBORDERY] = 865; // 1340
}

void T_CryptOpen()
{
	/*char miniset[4][4] = {
		//  11    12    13    14
		{ 1340,    0,    0,    0 }, // 24
		{ 1339,    0,    0,    0 }, // 25
		{ 1331, 1333, 1335, 1337 }, // 26
		{ 1332, 1334, 1336, 1338 }, // 27
	};*/
	dPiece[26 + DBORDERX][11 + DBORDERY] = 856; // 1331
	dPiece[27 + DBORDERX][11 + DBORDERY] = 857; // 1332
	dPiece[26 + DBORDERX][12 + DBORDERY] = 858; // 1333
	dPiece[27 + DBORDERX][12 + DBORDERY] = 859; // 1334
	dPiece[26 + DBORDERX][13 + DBORDERY] = 860; // 1335
	dPiece[27 + DBORDERX][13 + DBORDERY] = 861; // 1336
	dPiece[26 + DBORDERX][14 + DBORDERY] = 862; // 1337
	dPiece[27 + DBORDERX][14 + DBORDERY] = 863; // 1338
	dPiece[25 + DBORDERX][11 + DBORDERY] = 864; // 1339
	dPiece[24 + DBORDERX][11 + DBORDERY] = 865; // 1340
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

static void LoadTown()
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
		dPiece[50 + DBORDERX][60 + DBORDERY] = 796; // 1257
		// dPiece[50 + DBORDERX + 1][60 + DBORDERY] = 162; // 259
		// dPiece[50 + DBORDERX][60 + DBORDERY + 1] = 163; // 260
		// dPiece[50 + DBORDERX + 1][60 + DBORDERY + 1] = 164; // 261
	}

	gbOpenWarps = GetOpenWarps();
	if (!(gbOpenWarps & (1 << TWARP_CATACOMB))) {
		dPiece[38 + DBORDERX][10 + DBORDERY] = 745; // 1171
		dPiece[38 + DBORDERX + 1][10 + DBORDERY] = 746; // 1172
		dPiece[38 + DBORDERX][10 + DBORDERY + 1] = 747; // 1173
		dPiece[38 + DBORDERX + 1][10 + DBORDERY + 1] = 748; // 1174
	}
	if (!(gbOpenWarps & (1 << TWARP_CAVES))) {
		dPiece[6 + DBORDERX][58 + DBORDERY] = 777; // 1217
		// dPiece[6 + DBORDERX + 1][58 + DBORDERY] = ???; // 1218
		dPiece[6 + DBORDERX][58 + DBORDERY + 1] = 778; // 1219
		dPiece[6 + DBORDERX + 1][58 + DBORDERY + 1] = 779; // 1220
		dPiece[4 + DBORDERX + 1][58 + DBORDERY] = 285; // 427
		dPiece[6 + DBORDERX + 1][58 + DBORDERY + 2] = 776; // 1215
	}
	if (!(gbOpenWarps & (1 << TWARP_HELL))) {
		for (x = 0; x < 10; x++) {
			dPiece[26 + DBORDERX + x][68 + DBORDERY + 0] = dPiece[40 + DBORDERX + x][18 + DBORDERY + 0];
			dPiece[26 + DBORDERX + x][68 + DBORDERY + 1] = dPiece[40 + DBORDERX + x][18 + DBORDERY + 1];
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
	LoadTown();

	DRLG_InitTrans();
}

DEVILUTION_END_NAMESPACE
