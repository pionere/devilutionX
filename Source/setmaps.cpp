/**
 * @file setmaps.cpp
 *
 * Implementation of functionality the special quest dungeons.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

// BUGFIX: constant data should be const
const BYTE BoneChamTrans1[] = {
	DBORDERX + 27, DBORDERY + 3, DBORDERX + 34, DBORDERY + 10,
	DBORDERX + 35, DBORDERY + 3, DBORDERX + 43, DBORDERY + 10,
	DBORDERX + 19, DBORDERY + 11, DBORDERX + 26, DBORDERY + 18,
	DBORDERX + 27, DBORDERY + 11, DBORDERX + 33, DBORDERY + 18,
	DBORDERX + 34, DBORDERY + 11, DBORDERX + 43, DBORDERY + 18
};

const BYTE BoneChamTrans2[] = {
	DBORDERX + 3, DBORDERY + 15, DBORDERX + 18, DBORDERY + 31,
	DBORDERX + 18, DBORDERY + 19, DBORDERX + 26, DBORDERY + 26
};

const BYTE BoneChamTrans3[] = {
	DBORDERX + 27, DBORDERY + 19, DBORDERX + 34, DBORDERY + 26,
	DBORDERX + 35, DBORDERY + 19, DBORDERX + 46, DBORDERY + 26,
	DBORDERX + 47, DBORDERY + 15, DBORDERX + 50, DBORDERY + 30,
	DBORDERX + 51, DBORDERY + 15, DBORDERX + 62, DBORDERY + 18,
	DBORDERX + 51, DBORDERY + 19, DBORDERX + 62, DBORDERY + 26,
	DBORDERX + 51, DBORDERY + 27, DBORDERX + 62, DBORDERY + 30,
	DBORDERX + 19, DBORDERY + 27, DBORDERX + 26, DBORDERY + 35,
	DBORDERX + 27, DBORDERY + 27, DBORDERX + 33, DBORDERY + 35,
	DBORDERX + 34, DBORDERY + 27, DBORDERX + 43, DBORDERY + 35
};

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
		LoadL1Dungeon(lds);
		// gbInitObjFlag = true;
		AddL1Objs(0, 0, MAXDUNX, MAXDUNY);
		break;
	case DTYPE_CATACOMBS:
		LoadL2Dungeon(lds);
		// gbInitObjFlag = true;
		AddL2Objs(0, 0, MAXDUNX, MAXDUNY);
		break;
	case DTYPE_CAVES:
		LoadL3Dungeon(lds);
		break;
	//case DTYPE_HELL:
	//	LoadL4Dungeon(lds);
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
		numtrans = 1;
		DRLG_ListTrans(sizeof(BoneChamTrans1) / 4, &BoneChamTrans1[0]);
		DRLG_AreaTrans(sizeof(BoneChamTrans2) / 4, &BoneChamTrans2[0]);
		DRLG_ListTrans(sizeof(BoneChamTrans3) / 4, &BoneChamTrans3[0]);
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
	LoadPalette(lds->dPalName);
	InitTriggers();
}

DEVILUTION_END_NAMESPACE
