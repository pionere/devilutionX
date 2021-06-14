/**
 * @file setmaps.cpp
 *
 * Implementation of functionality the special quest dungeons.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

// BUGFIX: constant data should be const
const BYTE SkelKingTrans1[] = {
	DBORDERX + 3, DBORDERY + 31, DBORDERX + 10, DBORDERY + 39,
	DBORDERX + 10, DBORDERY + 33, DBORDERX + 14, DBORDERY + 37
};

const BYTE SkelKingTrans2[] = {
	DBORDERX + 17, DBORDERY + 3, DBORDERX + 31, DBORDERY + 13,
	DBORDERX + 21, DBORDERY + 13, DBORDERX + 27, DBORDERY + 23
};

const BYTE SkelKingTrans3[] = {
	DBORDERX + 11, DBORDERY + 37, DBORDERX + 19, DBORDERY + 45,
	DBORDERX + 11, DBORDERY + 19, DBORDERX + 18, DBORDERY + 26,
	DBORDERX + 29, DBORDERY + 19, DBORDERX + 37, DBORDERY + 27,
	DBORDERX + 29, DBORDERY + 37, DBORDERX + 37, DBORDERY + 45,
	DBORDERX + 15, DBORDERY + 23, DBORDERX + 33, DBORDERY + 41
};

const BYTE SkelKingTrans4[] = {
	DBORDERX + 33, DBORDERY + 29, DBORDERX + 42, DBORDERY + 35,
	DBORDERX + 41, DBORDERY + 15, DBORDERX + 46, DBORDERY + 21,
	DBORDERX + 47, DBORDERY + 15, DBORDERX + 53, DBORDERY + 24,
	DBORDERX + 43, DBORDERY + 25, DBORDERX + 57, DBORDERY + 39,
	DBORDERX + 47, DBORDERY + 39, DBORDERX + 53, DBORDERY + 49,
	DBORDERX + 57, DBORDERY + 29, DBORDERX + 62, DBORDERY + 35,
	DBORDERX + 63, DBORDERY + 27, DBORDERX + 73, DBORDERY + 37
};

const BYTE SkelChamTrans1[] = {
	DBORDERX + 27, DBORDERY + 3, DBORDERX + 34, DBORDERY + 10,
	DBORDERX + 35, DBORDERY + 3, DBORDERX + 43, DBORDERY + 10,
	DBORDERX + 19, DBORDERY + 11, DBORDERX + 26, DBORDERY + 18,
	DBORDERX + 27, DBORDERY + 11, DBORDERX + 33, DBORDERY + 18,
	DBORDERX + 34, DBORDERY + 11, DBORDERX + 43, DBORDERY + 18
};

const BYTE SkelChamTrans2[] = {
	DBORDERX + 3, DBORDERY + 15, DBORDERX + 18, DBORDERY + 31,
	DBORDERX + 18, DBORDERY + 19, DBORDERX + 26, DBORDERY + 26
};

const BYTE SkelChamTrans3[] = {
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
const char *const quest_level_names[] = {
	"",
	"Skeleton King's Lair",
	"Chamber of Bone",
	"Maze",
	"Poisoned Water Supply",
	"Archbishop Lazarus' Lair",
};

static int ObjIndex(int x, int y)
{
	int i, oi;

	for (i = 0; i < nobjects; i++) {
		oi = objectactive[i];
		if (object[oi]._ox == x && object[oi]._oy == y)
			return oi;
	}
	app_fatal("ObjIndex: Active object not found at (%d,%d)", x, y);
	return -1;
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
	SetObjMapRange(ObjIndex(DBORDERX + 19, DBORDERY + 20), 7, 11, 13, 18, 3);
}

static void DRLG_SetMapTrans(const char *sFileName)
{
	int x, y;
	int i, j;
	BYTE *pMap;
	BYTE *lm;
	unsigned dwOffset;

	pMap = LoadFileInMem(sFileName);
	lm = pMap + 2;
	x = pMap[0];
	y = *lm;
	dwOffset = (x * y + 1) * 2;
	x <<= 1;
	y <<= 1;
	dwOffset += 3 * x * y * 2;
	lm += dwOffset;

	x += DBORDERX;
	y += DBORDERY;
	for (j = DBORDERY; j < y; j++) {
		for (i = DBORDERX; i < x; i++) {
			dTransVal[i][j] = *lm;
			lm += 2;
		}
	}
	mem_free_dbg(pMap);
}

/**
 * @brief Load a 'set' map
 */
void LoadSetMap()
{
	const LevelDataStruct *lds = &AllLevels[currLvl._dLevelIdx];

	switch (currLvl._dLevelIdx) {
	case SL_SKELKING:
		LoadPreL1Dungeon(lds->dSetLvlPreDun);
		LoadL1Dungeon(lds->dSetLvlDun, lds->dSetLvlDunX, lds->dSetLvlDunY);
		DRLG_AreaTrans(sizeof(SkelKingTrans1) / 4, &SkelKingTrans1[0]);
		DRLG_ListTrans(sizeof(SkelKingTrans2) / 4, &SkelKingTrans2[0]);
		DRLG_AreaTrans(sizeof(SkelKingTrans3) / 4, &SkelKingTrans3[0]);
		DRLG_ListTrans(sizeof(SkelKingTrans4) / 4, &SkelKingTrans4[0]);
		// gbInitObjFlag = true;
		AddL1Objs(0, 0, MAXDUNX, MAXDUNY);
		AddSKingObjs();
		// gbInitObjFlag = false;
		break;
	case SL_BONECHAMB:
		LoadPreL2Dungeon(lds->dSetLvlPreDun);
		LoadL2Dungeon(lds->dSetLvlDun, lds->dSetLvlDunX, lds->dSetLvlDunY);
		DRLG_ListTrans(sizeof(SkelChamTrans1) / 4, &SkelChamTrans1[0]);
		DRLG_AreaTrans(sizeof(SkelChamTrans2) / 4, &SkelChamTrans2[0]);
		DRLG_ListTrans(sizeof(SkelChamTrans3) / 4, &SkelChamTrans3[0]);
		// gbInitObjFlag = true;
		AddL2Objs(0, 0, MAXDUNX, MAXDUNY);
		AddSChamObjs();
		// gbInitObjFlag = false;
		break;
	case SL_MAZE:
		LoadPreL1Dungeon(lds->dSetLvlPreDun);
		LoadL1Dungeon(lds->dSetLvlDun, lds->dSetLvlDunX, lds->dSetLvlDunY);
		// gbInitObjFlag = true;
		AddL1Objs(0, 0, MAXDUNX, MAXDUNY);
		// gbInitObjFlag = false;
		DRLG_SetMapTrans(lds->dSetLvlPreDun);
		break;
	case SL_POISONWATER:
		LoadPreL3Dungeon(lds->dSetLvlPreDun);
		LoadL3Dungeon(lds->dSetLvlDun, lds->dSetLvlDunX, lds->dSetLvlDunY);
		if (quests[Q_PWATER]._qactive == QUEST_INIT)
			quests[Q_PWATER]._qactive = QUEST_ACTIVE;
		break;
	case SL_VILEBETRAYER:
		LoadPreL1Dungeon(lds->dSetLvlPreDun);
		LoadL1Dungeon(lds->dSetLvlDun, lds->dSetLvlDunX, lds->dSetLvlDunY);
		// gbInitObjFlag = true;
		AddL1Objs(0, 0, MAXDUNX, MAXDUNY);
		AddVileObjs();
		// gbInitObjFlag = false;
		DRLG_SetMapTrans(lds->dSetLvlPreDun);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	LoadPalette(lds->dPalName);
	InitTriggers();
}

DEVILUTION_END_NAMESPACE
