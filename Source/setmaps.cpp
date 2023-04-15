/**
 * @file setmaps.cpp
 *
 * Implementation of functionality the special quest dungeons.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Maps from quest level to quest level names. */
/*const char *const quest_level_names[] = {
	"",
	"Skeleton King's Lair",
	"Chamber of Bone",
	"Maze",
	"Poisoned Water Supply",
	"Archbishop Lazarus' Lair",
};*/

static int ObjIndex(int x, int y)
{
	int oi = dObject[x][y];
#if DEBUG_MODE
	if (oi == 0) {
		app_fatal("ObjIndex: Active object not found at (%d,%d)", x, y);
	}
#endif
	oi = oi >= 0 ? oi - 1 : -(oi + 1);
	return oi;
}

static void AddSKingObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 48, DBORDERY + 18), 20, 7, 23, 10, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 48, DBORDERY + 43), 20, 14, 21, 16, 2);
	SetObjMapRange(ObjIndex(DBORDERX + 11, DBORDERY + 21), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 30, DBORDERY + 19), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 33, DBORDERY + 37), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 11, DBORDERY + 37), 8, 1, 15, 11, 3);
}

static void AddSChamObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 21, DBORDERY + 14), 17, 0, 21, 5, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 21, DBORDERY + 30), 13, 0, 16, 5, 2);
}

static void AddVileObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 10, DBORDERY + 29), 3, 4, 8, 10, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 29, DBORDERY + 30), 11, 4, 16, 10, 2);
	//SetObjMapRange(ObjIndex(DBORDERX + 19, DBORDERY + 20), 7, 11, 13, 18, 3);
}

/*static void AddMazeObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 33, DBORDERY + 25), 0?, 0?, 45?, ?, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 15, DBORDERY + 51), ?, ?, ?, ?, ?);
	SetObjMapRange(ObjIndex(DBORDERX + 27, DBORDERY + 51), ?, ?, ?, ?, ?);
	SetObjMapRange(ObjIndex(DBORDERX + 33, DBORDERY + 57), ?, ?, ?, ?, ?);
	SetObjMapRange(ObjIndex(DBORDERX + 79, DBORDERY + 51), ?, ?, ?, ?, ?);
}*/

void LoadSetMap()
{
	const LevelData* lds = &AllLevels[currLvl._dLevelIdx];

	switch (lds->dDunType) {
	case DTYPE_CATHEDRAL:
		CreateL1Dungeon();
		break;
	case DTYPE_CATACOMBS:
		CreateL2Dungeon();
		break;
	case DTYPE_CAVES:
		CreateL3Dungeon();
		break;
	//case DTYPE_HELL:
	//	CreateL4Dungeon();
	//	break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	switch (currLvl._dLevelIdx) {
	case SL_SKELKING:
		AddSKingObjs();
		// gbInitObjFlag = false;
		break;
	case SL_BONECHAMB:
		AddSChamObjs();
		// gbInitObjFlag = false;
		break;
	/*case SL_MAZE:
		break;*/
	case SL_POISONWATER:
		break;
	case SL_VILEBETRAYER:
		AddVileObjs();
		// gbInitObjFlag = false;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	InitTriggers();
	LoadRndLvlPal();
}

DEVILUTION_END_NAMESPACE
