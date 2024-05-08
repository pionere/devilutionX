/**
 * @file town.h
 *
 * Implementation of functionality for rendering the town, towners and calling other render routines.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef HELLFIRE
#define TOWN_PREDUN "NLevels\\TownData\\Town.RDUN"
#define TOWN_DUN    "NLevels\\TownData\\Town.DUN"
#else
#define TOWN_PREDUN "Levels\\TownData\\Town.RDUN"
#define TOWN_DUN    "Levels\\TownData\\Town.DUN"
#endif

void OpenNest()
{
	DRLG_ChangeMap(34, 25, 36, 27);
}

void OpenCrypt()
{
	DRLG_ChangeMap(13, 5, 13, 7);
}

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
	// load the changing tiles of the town
	pSetPieces[0]._spx = 0;
	pSetPieces[0]._spy = 0;
	// pSetPieces[0]._sptype = lds->dSetLvlPiece;
	pSetPieces[0]._spData = LoadFileInMem(TOWN_DUN);
	// DRLG_L1FixMap();
	// memset(drlgFlags, 0, sizeof(drlgFlags));
	// static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL1DungeonData.");
	// memset(dungeon, BASE_MEGATILE_L1, sizeof(dungeon));

	// DRLG_LoadSP(0, DEFAULT_MEGATILE_L1);
	DRLG_DrawMap(0);
	memcpy(pdungeon, dungeon, sizeof(pdungeon));

#if INT_MAX == INT32_MAX && SDL_BYTEORDER != SDL_BIG_ENDIAN
	LoadFileWithMem(TOWN_PREDUN, (BYTE*)&dPiece[0][0]);
#else
	uint32_t* pBuf = (uint32_t*)LoadFileInMem(TOWN_PREDUN);
	int* dp = &dPiece[0][0];
	uint32_t* pTmp = pBuf;
	for (int x = 0; x < MAXDUNX * MAXDUNY; x++, dp++, pTmp++)
		*dp = SwapLE32(*pTmp);

	mem_free_dbg(pBuf);
#endif

	if (quests[Q_PWATER]._qvar1 == QV_PWATER_CLEAN) {
		DRLG_ChangeMap(25, 30, 25, 30);
	}

	gbOpenWarps = GetOpenWarps();
	if (gbOpenWarps & (1 << TWARP_CATACOMB)) {
		DRLG_ChangeMap(19, 5, 19, 5);
	}
	if (gbOpenWarps & (1 << TWARP_CAVES)) {
		DRLG_ChangeMap(3, 29, 3, 30);
	}
	if (gbOpenWarps & (1 << TWARP_HELL)) {
		DRLG_ChangeMap(13, 34, 17, 34);
	}
#ifdef HELLFIRE
	if (gbOpenWarps & (1 << TWARP_NEST)) {
		OpenNest();
	}
	if (gbOpenWarps & (1 << TWARP_CRYPT)) {
		OpenCrypt();
	}
#endif
	// release prematurely to counter the incomplete map
	MemFreeDbg(pSetPieces[0]._spData);
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
