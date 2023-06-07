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
		{ 1162, 1163, 1262, 1264,  282,  284}, // 68
		{ 1164, 1294, 1296, 1297, 1298,  285}, // 69
		{ 1299, 1301, 1307, 1309, 1311, 1313}, // 70
		{ 1300, 1302, 1308, 1310, 1312, 1314}, // 71
		{ 1303, 1305, 1315, 1317, 1319, 1321}, // 72
		{ 1304, 1306, 1316, 1318, 1320, 1322}, // 73
		{    8,  280,  280,  280,  280,    0}, // 74
		{  280,  280,   19,    8,    8,    0}, // 75
		{  217,   24,    0,    0,    0,    0}, // 76
	};*/
	dPiece[68 + DBORDERX][50 + DBORDERY] = 1162;
	dPiece[69 + DBORDERX][50 + DBORDERY] = 1163;
	dPiece[68 + DBORDERX][51 + DBORDERY] = 1164;
	dPiece[69 + DBORDERX][51 + DBORDERY] = 1294;
	dPiece[68 + DBORDERX][52 + DBORDERY] = 1262;
	dPiece[69 + DBORDERX][52 + DBORDERY] = 1296;
	dPiece[68 + DBORDERX][53 + DBORDERY] = 1264;
	dPiece[69 + DBORDERX][53 + DBORDERY] = 1297;
	dPiece[68 + DBORDERX][54 + DBORDERY] = 282;
	dPiece[69 + DBORDERX][54 + DBORDERY] = 1298;
	dPiece[68 + DBORDERX][55 + DBORDERY] = 284;
	dPiece[69 + DBORDERX][55 + DBORDERY] = 285;
	dPiece[70 + DBORDERX][50 + DBORDERY] = 1299;
	dPiece[71 + DBORDERX][50 + DBORDERY] = 1300;
	dPiece[70 + DBORDERX][51 + DBORDERY] = 1301;
	dPiece[71 + DBORDERX][51 + DBORDERY] = 1302;
	dPiece[72 + DBORDERX][50 + DBORDERY] = 1303;
	dPiece[73 + DBORDERX][50 + DBORDERY] = 1304;
	dPiece[72 + DBORDERX][51 + DBORDERY] = 1305;
	dPiece[73 + DBORDERX][51 + DBORDERY] = 1306;
	dPiece[70 + DBORDERX][52 + DBORDERY] = 1307;
	dPiece[71 + DBORDERX][52 + DBORDERY] = 1308;
	dPiece[70 + DBORDERX][53 + DBORDERY] = 1309;
	dPiece[71 + DBORDERX][53 + DBORDERY] = 1310;
	dPiece[70 + DBORDERX][54 + DBORDERY] = 1311;
	dPiece[71 + DBORDERX][54 + DBORDERY] = 1312;
	dPiece[70 + DBORDERX][55 + DBORDERY] = 1313;
	dPiece[71 + DBORDERX][55 + DBORDERY] = 1314;
	dPiece[72 + DBORDERX][54 + DBORDERY] = 1319;
	dPiece[73 + DBORDERX][54 + DBORDERY] = 1320;
	dPiece[72 + DBORDERX][55 + DBORDERY] = 1321;
	dPiece[73 + DBORDERX][55 + DBORDERY] = 1322;
	dPiece[72 + DBORDERX][52 + DBORDERY] = 1315;
	dPiece[73 + DBORDERX][52 + DBORDERY] = 1316;
	dPiece[72 + DBORDERX][53 + DBORDERY] = 1317;
	dPiece[73 + DBORDERX][53 + DBORDERY] = 1318;
	dPiece[74 + DBORDERX][50 + DBORDERY] = 8; // new
	dPiece[74 + DBORDERX][51 + DBORDERY] = 280;
	dPiece[74 + DBORDERX][52 + DBORDERY] = 280;
	dPiece[74 + DBORDERX][53 + DBORDERY] = 280;
	//dPiece[75 + DBORDERX][50 + DBORDERY] = 280;
	//dPiece[75 + DBORDERX][51 + DBORDERY] = 280;
	//dPiece[75 + DBORDERX][53 + DBORDERY] = 8;
	//dPiece[75 + DBORDERX][54 + DBORDERY] = 8;
	//dPiece[76 + DBORDERX][50 + DBORDERY] = 217;
	//dPiece[76 + DBORDERX][51 + DBORDERY] = 24;
	//dPiece[75 + DBORDERX][52 + DBORDERY] = 19;
	dPiece[74 + DBORDERX][54 + DBORDERY] = 280;
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
	dPiece[68 + DBORDERX][50 + DBORDERY] = 1162;
	dPiece[69 + DBORDERX][50 + DBORDERY] = 1259;
	dPiece[68 + DBORDERX][51 + DBORDERY] = 1260;
	dPiece[69 + DBORDERX][51 + DBORDERY] = 1261;
	dPiece[68 + DBORDERX][52 + DBORDERY] = 1262;
	dPiece[69 + DBORDERX][52 + DBORDERY] = 1263;
	dPiece[68 + DBORDERX][53 + DBORDERY] = 1264;
	dPiece[69 + DBORDERX][53 + DBORDERY] = 1265;
	dPiece[68 + DBORDERX][54 + DBORDERY] = 1266;
	dPiece[69 + DBORDERX][54 + DBORDERY] = 1267;
	dPiece[68 + DBORDERX][55 + DBORDERY] = 1268;
	dPiece[70 + DBORDERX][50 + DBORDERY] = 1269;
	dPiece[71 + DBORDERX][50 + DBORDERY] = 1270;
	dPiece[70 + DBORDERX][51 + DBORDERY] = 1271;
	dPiece[71 + DBORDERX][51 + DBORDERY] = 1272;
	dPiece[72 + DBORDERX][50 + DBORDERY] = 1273;
	dPiece[73 + DBORDERX][50 + DBORDERY] = 1274;
	dPiece[72 + DBORDERX][51 + DBORDERY] = 1275;
	dPiece[73 + DBORDERX][51 + DBORDERY] = 1276;
	dPiece[70 + DBORDERX][52 + DBORDERY] = 1277;
	dPiece[71 + DBORDERX][52 + DBORDERY] = 1278;
	dPiece[70 + DBORDERX][53 + DBORDERY] = 1279;
	dPiece[71 + DBORDERX][53 + DBORDERY] = 1280;
	dPiece[70 + DBORDERX][54 + DBORDERY] = 1281;
	dPiece[71 + DBORDERX][54 + DBORDERY] = 1282;
	dPiece[70 + DBORDERX][55 + DBORDERY] = 1283;
	dPiece[71 + DBORDERX][55 + DBORDERY] = 1284;
	dPiece[72 + DBORDERX][54 + DBORDERY] = 1289;
	dPiece[73 + DBORDERX][54 + DBORDERY] = 1290;
	dPiece[72 + DBORDERX][55 + DBORDERY] = 1291;
	dPiece[73 + DBORDERX][55 + DBORDERY] = 1292;
	dPiece[72 + DBORDERX][52 + DBORDERY] = 1285;
	dPiece[73 + DBORDERX][52 + DBORDERY] = 1286;
	dPiece[72 + DBORDERX][53 + DBORDERY] = 1287;
	dPiece[73 + DBORDERX][53 + DBORDERY] = 1288;
	dPiece[74 + DBORDERX][50 + DBORDERY] = 8; // new
	dPiece[74 + DBORDERX][51 + DBORDERY] = 280
	dPiece[74 + DBORDERX][52 + DBORDERY] = 280
	dPiece[74 + DBORDERX][53 + DBORDERY] = 280
	//dPiece[75 + DBORDERX][50 + DBORDERY] = 280;
	//dPiece[75 + DBORDERX][51 + DBORDERY] = 280;
	//dPiece[75 + DBORDERX][53 + DBORDERY] = 8;
	//dPiece[75 + DBORDERX][54 + DBORDERY] = 8;
	//dPiece[76 + DBORDERX][50 + DBORDERY] = 217;
	//dPiece[76 + DBORDERX][51 + DBORDERY] = 24;
	//dPiece[75 + DBORDERX][52 + DBORDERY] = 19;
	dPiece[74 + DBORDERX][54 + DBORDERY] = 280
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
	dPiece[26 + DBORDERX][11 + DBORDERY] = 1323;
	dPiece[27 + DBORDERX][11 + DBORDERY] = 1324;
	dPiece[26 + DBORDERX][12 + DBORDERY] = 1325;
	dPiece[27 + DBORDERX][12 + DBORDERY] = 1326;
	dPiece[26 + DBORDERX][13 + DBORDERY] = 1327;
	dPiece[27 + DBORDERX][13 + DBORDERY] = 1328;
	dPiece[26 + DBORDERX][14 + DBORDERY] = 1329;
	dPiece[27 + DBORDERX][14 + DBORDERY] = 1330;
	dPiece[25 + DBORDERX][11 + DBORDERY] = 1339;
	dPiece[24 + DBORDERX][11 + DBORDERY] = 1340;
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
	dPiece[26 + DBORDERX][11 + DBORDERY] = 1331;
	dPiece[27 + DBORDERX][11 + DBORDERY] = 1332;
	dPiece[26 + DBORDERX][12 + DBORDERY] = 1333;
	dPiece[27 + DBORDERX][12 + DBORDERY] = 1334;
	dPiece[26 + DBORDERX][13 + DBORDERY] = 1335;
	dPiece[27 + DBORDERX][13 + DBORDERY] = 1336;
	dPiece[26 + DBORDERX][14 + DBORDERY] = 1337;
	dPiece[27 + DBORDERX][14 + DBORDERY] = 1338;
	dPiece[25 + DBORDERX][11 + DBORDERY] = 1339;
	dPiece[24 + DBORDERX][11 + DBORDERY] = 1340;
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
		dPiece[50 + DBORDERX][60 + DBORDERY] = 1257;
		// dPiece[50 + DBORDERX + 1][60 + DBORDERY] = 259;
		// dPiece[50 + DBORDERX][60 + DBORDERY + 1] = 260;
		// dPiece[50 + DBORDERX + 1][60 + DBORDERY + 1] = 261;
	}

	gbOpenWarps = GetOpenWarps();
	if (!(gbOpenWarps & (1 << TWARP_CATACOMB))) {
		dPiece[38 + DBORDERX][10 + DBORDERY] = 1171;
		dPiece[38 + DBORDERX + 1][10 + DBORDERY] = 1172;
		dPiece[38 + DBORDERX][10 + DBORDERY + 1] = 1173;
		dPiece[38 + DBORDERX + 1][10 + DBORDERY + 1] = 1174;
	}
	if (!(gbOpenWarps & (1 << TWARP_CAVES))) {
		dPiece[6 + DBORDERX][58 + DBORDERY] = 1217;
		dPiece[6 + DBORDERX + 1][58 + DBORDERY] = 1218;
		dPiece[6 + DBORDERX][58 + DBORDERY + 1] = 1219;
		dPiece[6 + DBORDERX + 1][58 + DBORDERY + 1] = 1220;
		dPiece[4 + DBORDERX + 1][58 + DBORDERY] = 427;
		dPiece[6 + DBORDERX + 1][58 + DBORDERY + 2] = 1215;
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
