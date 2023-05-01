/**
 * @file trigs.cpp
 *
 * Implementation of functionality for triggering events when the player enters an area.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

int numtrigs;
TriggerStruct trigs[MAXTRIGGERS];
BYTE gbOpenWarps;
BYTE gbTWarpFrom;

#define PIECE dPiece[pcurspos.x][pcurspos.y]
/** Specifies the dungeon piece IDs which constitute stairways leading down to the cathedral from town. */
#define TOWN_L1_WARP     (PIECE == 716 || (PIECE >= 723 && PIECE <= 728))
/** Specifies the dungeon piece IDs which constitute stairways leading down to the catacombs from town. */
#define TOWN_L2_WARP     (PIECE >= 1175 && PIECE <= 1178)
/** Specifies the dungeon piece IDs which constitute stairways leading down to the caves from town. */
#define TOWN_L3_WARP     (PIECE >= 1210 && PIECE <= 1213)
/** Specifies the dungeon piece IDs which constitute stairways leading down to hell from town. */
#define TOWN_L4_WARP     (PIECE >= 1244 && PIECE <= 1252)
/** Specifies the dungeon piece IDs which constitute stairways leading down to the hive from town. */
#define TOWN_L5_WARP     (PIECE >= 1307 && PIECE <= 1310)
/** Specifies the dungeon piece IDs which constitute stairways leading down to the crypt from town. */
#define TOWN_L6_WARP     (PIECE >= 1331 && PIECE <= 1338)
/** Specifies the dungeon piece IDs which constitute stairways leading up from the cathedral. */
#define L1_UP_WARP       (PIECE >= 129 && PIECE <= 140 && PIECE != 134 && PIECE != 136)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the cathedral. */
//							{ 106, 107, 108, 109, 110, /*111,*/ 112, /*113,*/ 114, 115, /*116, 117,*/ 118, }
#define L1_DOWN_WARP     ((PIECE >= 106 && PIECE <= 115 && PIECE != 111 && PIECE != 113) || PIECE == 118)
/** Specifies the dungeon piece IDs which constitute stairways leading up from the catacombs. */
#define L2_UP_WARP       (PIECE >= 266 && PIECE <= 267)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the catacombs. */
#define L2_DOWN_WARP     (PIECE >= 271 && PIECE <= 272)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the catacombs. */
#define L2_TOWN_WARP     (PIECE >= 558 && PIECE <= 559)
/** Specifies the dungeon piece IDs which constitute stairways leading up from the caves. */
#define L3_UP_WARP       (PIECE == 170 || PIECE == 171)
#define L3_UP_WARPx(x)   (x == 170 || x == 171)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the caves. */
#define L3_DOWN_WARP     (PIECE == 168)
#define L3_DOWN_WARPx(x) (x == 168)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the caves. */
//#define L3_TOWN_WARP     (PIECE == 548 || PIECE == 549 || PIECE == 559 || PIECE == 560)
#define L3_TOWN_WARP     (PIECE == 548 || PIECE == 549)
#define L3_TOWN_WARPx(x) (x == 548 || x == 549)
/** Specifies the dungeon piece IDs which constitute stairways leading up from hell. */
#define L4_UP_WARP       (PIECE >= 94 && PIECE <= 97)
/** Specifies the dungeon piece IDs which constitute stairways leading down from hell. */
#define L4_DOWN_WARP     ((PIECE >= 130 && PIECE <= 133) || PIECE == 120)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from hell. */
#define L4_TOWN_WARP     (PIECE >= 433 && PIECE <= 436)
/** Specifies the dungeon piece IDs which constitute stairways leading down to Diablo from hell. */
#define L4_PENTA_WARP    (PIECE >= 367 && PIECE <= 370)
#ifdef HELLFIRE
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from crypt. */
//#define L5_TOWN_WARP		(PIECE >= 172 && PIECE <= 185 && (PIECE <= 179 || PIECE >= 184))
/** Specifies the dungeon piece IDs which constitute stairways leading up from crypt. */
#define L5_UP_WARP       (PIECE >= 149 && PIECE <= 159 && (PIECE <= 153 || PIECE >= 158))
/** Specifies the dungeon piece IDs which constitute stairways leading down from crypt. */
#define L5_DOWN_WARP     (PIECE >= 125 && PIECE <= 126)
#define L5_DOWN_WARPx(x) (x >= 125 && x <= 126)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from nest. */
//#define L6_TOWN_WARP     (PIECE >= 79 && PIECE <= 80)
//#define L6_TOWN_WARPx(x) (x >= 79 && x <= 80)
/** Specifies the dungeon piece IDs which constitute stairways leading up from nest. */
#define L6_UP_WARP       (PIECE >= 65 && PIECE <= 66)
#define L6_UP_WARPx(x)   (x >= 65 && x <= 66)
/** Specifies the dungeon piece IDs which constitute stairways leading down from nest. */
#define L6_DOWN_WARPx(x) (x == 61 || x == 63)
#endif

static void InitNoTriggers()
{
	numtrigs = 0;
}

/*
 * Initialize the triggers in town.
 * Only the available triggers are set up correctly (numtrigs won't reflect this!).
 */
static void InitTownTriggers()
{
	assert(gbOpenWarps & (1 << TWARP_CATHEDRAL));
	trigs[TWARP_CATHEDRAL]._tx = 15 + DBORDERX;
	trigs[TWARP_CATHEDRAL]._ty = 19 + DBORDERY;
	trigs[TWARP_CATHEDRAL]._tmsg = DVL_DWM_TWARPDN;
	trigs[TWARP_CATHEDRAL]._tlvl = DLV_CATHEDRAL1;

	if (gbOpenWarps & (1 << TWARP_CATACOMB)) {
		trigs[TWARP_CATACOMB]._tx = 39 + DBORDERX;
		trigs[TWARP_CATACOMB]._ty = 11 + DBORDERY;
		trigs[TWARP_CATACOMB]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_CATACOMB]._tlvl = DLV_CATACOMBS1;
	}
	if (gbOpenWarps & (1 << TWARP_CAVES)) {
		trigs[TWARP_CAVES]._tx = 7 + DBORDERX;
		trigs[TWARP_CAVES]._ty = 59 + DBORDERY;
		trigs[TWARP_CAVES]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_CAVES]._tlvl = DLV_CAVES1;
	}
	if (gbOpenWarps & (1 << TWARP_HELL)) {
		trigs[TWARP_HELL]._tx = 30 + DBORDERX;
		trigs[TWARP_HELL]._ty = 69 + DBORDERY;
		trigs[TWARP_HELL]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_HELL]._tlvl = DLV_HELL1;
	}
#ifdef HELLFIRE
	if (gbOpenWarps & (1 << TWARP_NEST)) {
		trigs[TWARP_NEST]._tx = 70 + DBORDERX;
		trigs[TWARP_NEST]._ty = 52 + DBORDERY;
		trigs[TWARP_NEST]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_NEST]._tlvl = DLV_NEST1;
	}
	if (gbOpenWarps & (1 << TWARP_CRYPT)) {
		trigs[TWARP_CRYPT]._tx = 26 + DBORDERX;
		trigs[TWARP_CRYPT]._ty = 14 + DBORDERY;
		trigs[TWARP_CRYPT]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_CRYPT]._tlvl = DLV_CRYPT1;
	}
#endif
	static_assert(NUM_TWARP <= MAXTRIGGERS, "Too many trigger in town.");
	numtrigs = NUM_TWARP;
}

static void InitL1Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = currLvl._dLevelIdx == DLV_CATHEDRAL1 ? DVL_DWM_TWARPUP : DVL_DWM_PREVLVL;
		numtrigs++;
	// }
	// if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		numtrigs++;
	// }
	if (pWarps[DWARP_SIDE]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_SIDE]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_SIDE]._wy;
		if (currLvl._dLevelIdx == questlist[Q_SKELKING]._qdlvl) { // TODO: add qn to pWarps?
			trigs[numtrigs]._tlvl = questlist[Q_SKELKING]._qslvl;
			trigs[numtrigs]._ty += 1;
		} else {
			trigs[numtrigs]._tlvl = questlist[Q_PWATER]._qslvl;
			trigs[numtrigs]._tx += 1;
		}
		trigs[numtrigs]._tmsg = DVL_DWM_SETLVL;
		numtrigs++;
	}
}

static void InitL2Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		numtrigs++;
	// }
	// if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		numtrigs++;
	// }
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		numtrigs++;
	}
	if (pWarps[DWARP_SIDE]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_SIDE]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_SIDE]._wy + 1;
		trigs[numtrigs]._tlvl = questlist[Q_BCHAMB]._qslvl;
		trigs[numtrigs]._tmsg = DVL_DWM_SETLVL;
		numtrigs++;
	}
}

static void InitL3Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		numtrigs++;
	// }
	// if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		numtrigs++;
	// }
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		numtrigs++;
	}
}

static void InitL4Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		numtrigs++;
	// }
	if (pWarps[DWARP_EXIT]._wx != 0) {
		if (currLvl._dLevelIdx != DLV_HELL3) {
			trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx;
			trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy;
			trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
			numtrigs++;
		} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE) {
			trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx + 1;
			trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
			trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
			numtrigs++;
		}
	}
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		numtrigs++;
	}
}

#ifdef HELLFIRE
static void InitL5Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = currLvl._dLevelIdx == DLV_CRYPT1 ? DVL_DWM_TWARPUP : DVL_DWM_PREVLVL;
		numtrigs++;
	// }
	if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		numtrigs++;
	}
}

static void InitL6Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = currLvl._dLevelIdx == DLV_NEST1 ? DVL_DWM_TWARPUP : DVL_DWM_PREVLVL;
		numtrigs++;
	// }
	if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		numtrigs++;
	}
}
#endif

static void InitSKingTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx; // DBORDERX + 66
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy; // DBORDERY + 26
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_SKELKING]._qdlvl;
}

static void InitSChambTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx;     // DBORDERX + 54
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy + 1; // DBORDERY + 23
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_BCHAMB]._qdlvl;
}

static void InitPWaterTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx;     // DBORDERX + 14
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy + 1; // DBORDERY + 67
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_PWATER]._qdlvl;
}

void InitVPEntryTrigger(bool recreate)
{
	int i;

	i = numtrigs; // quests[Q_BETRAYER]._qactive == QUEST_DONE ? 2 : 1;

	trigs[i]._tx = pWarps[DWARP_EXIT]._wx - 2;
	trigs[i]._ty = pWarps[DWARP_EXIT]._wy - 2;
	trigs[i]._tmsg = DVL_DWM_SETLVL;
	trigs[i]._tlvl = questlist[Q_BETRAYER]._qslvl;
	numtrigs = i + 1;

	AddMissile(0, 0, trigs[i]._tx, trigs[i]._ty, 0, MIS_RPORTAL, MST_NA, -1, recreate ? -1 : 0);
}

void InitVPReturnTrigger(bool recreate)
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx;     // DBORDERX + 19
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy - 4; // DBORDERX + 16
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_BETRAYER]._qdlvl;
	// TODO: set BFLAG_MON_PROTECT | BFLAG_OBJ_PROTECT?
	AddMissile(0, 0, trigs[0]._tx, trigs[0]._ty, 0, MIS_RPORTAL, MST_NA, -1, recreate ? -1 : 0);
}

static int ForceTownTrig()
{
	if (TOWN_L1_WARP) {
		// Down to dungeon
		return TWARP_CATHEDRAL;
	}

	if (gbOpenWarps & (1 << TWARP_CATACOMB)) {
		if (TOWN_L2_WARP) {
			// Down to catacombs
			return TWARP_CATACOMB;
		}
	}

	if (gbOpenWarps & (1 << TWARP_CAVES)) {
		if (TOWN_L3_WARP) {
			//Down to caves
			return TWARP_CAVES;
		}
	}

	if (gbOpenWarps & (1 << TWARP_HELL)) {
		if (TOWN_L4_WARP) {
			// Down to hell
			return TWARP_HELL;
		}
	}

#ifdef HELLFIRE
	if (gbOpenWarps & (1 << TWARP_NEST)) {
		if (TOWN_L5_WARP) {
			// Down to Hive
			return TWARP_NEST;
		}
	}
	if (gbOpenWarps & (1 << TWARP_CRYPT)) {
		if (TOWN_L6_WARP) {
			// Down to Crypt
			return TWARP_CRYPT;
		}
	}
#endif

	return -1;
}

static int ForceL1Trig()
{
	if (L1_UP_WARP) {
		// Up to town or Up to Cathedral level (currLvl._dLevelIdx - 1)
		// assert(trigs[0]._tmsg == DVL_DWM_TWARPUP || trigs[0]._tmsg == DVL_DWM_PREVLVL);
		return 0;
	}
	if (L1_DOWN_WARP) {
		// Down to level (currLvl._dLevelIdx + 1)
		// assert(trigs[1]._tmsg == DVL_DWM_NEXTLVL);
		return 1;
	}

	return -1;
}

static int ForceL2Trig()
{
	if (L2_UP_WARP) {
		// Up to level (currLvl._dLevelIdx - 1)
		if (trigs[0]._tx == pcurspos.x && (trigs[0]._ty >> 1) == (pcurspos.y >> 1)) {
			// assert(trigs[0]._tmsg == DVL_DWM_PREVLVL);
			return 0;
		} else {
			// assert(trigs[2]._tmsg == DVL_DWM_SETLVL);
			return 2;
		}
	}

	if (L2_DOWN_WARP) {
		// Down to level (currLvl._dLevelIdx + 1)
		// assert(trigs[1]._tmsg == DVL_DWM_NEXTLVL);
		return 1;
	}

	// if (currLvl._dLevelIdx == DLV_CATACOMBS1) {
		if (L2_TOWN_WARP) {
			// Up to town
			// assert(trigs[2]._tmsg == DVL_DWM_TWARPUP);
			return 2;
		}
	// }

	return -1;
}

static int ForceL3Trig()
{
	if (L3_UP_WARP
	 || L3_UP_WARPx(dPiece[pcurspos.x][pcurspos.y + 1])) {
		// Up to level (currLvl._dLevelIdx - 1)
		// assert(trigs[0]._tmsg == DVL_DWM_PREVLVL);
		return 0;
	}

	if (L3_DOWN_WARP
	 || L3_DOWN_WARPx(dPiece[pcurspos.x + 1][pcurspos.y])
	 || L3_DOWN_WARPx(dPiece[pcurspos.x + 1][pcurspos.y + 1])
	 /*|| L3_DOWN_WARPx(dPiece[pcurspos.x + 2][pcurspos.y + 1])*/) {
		// Down to level (currLvl._dLevelIdx + 1)
		// assert(trigs[1]._tmsg == DVL_DWM_NEXTLVL);
		return 1;
	}

	if (currLvl._dLevelIdx == DLV_CAVES1) {
		if (L3_TOWN_WARP
		 || L3_TOWN_WARPx(dPiece[pcurspos.x][pcurspos.y + 1])) {
			// assert(trigs[2]._tmsg == DVL_DWM_TWARPUP);
			return 2;
		}
	}
	return -1;
}

static int ForceL4Trig()
{
	if (L4_UP_WARP) {
		// Up to level (currLvl._dLevelIdx - 1)
		// assert(trigs[0]._tmsg == DVL_DWM_PREVLVL);
		return 0;
	}

	if (L4_DOWN_WARP) {
		// Down to level (currLvl._dLevelIdx + 1)
		// assert(trigs[1]._tmsg == DVL_DWM_NEXTLVL);
		return 1;
	}

	if (currLvl._dLevelIdx == DLV_HELL1) {
		if (L4_TOWN_WARP) {
			// assert(trigs[1]._tmsg == DVL_DWM_TWARPUP || trigs[2]._tmsg == DVL_DWM_TWARPUP);
			return trigs[1]._tmsg == DVL_DWM_TWARPUP ? 1 : 2;
		}
	}

	if (currLvl._dLevelIdx == DLV_HELL3) {
		if (L4_PENTA_WARP && numtrigs != 1) {
			// Down to Diablo
			// assert(trigs[1]._tmsg == DVL_DWM_NEXTLVL);
			return 1;
		}
	}

	return -1;
}

#ifdef HELLFIRE
static int ForceL5Trig()
{
	if (L5_UP_WARP) {
		// Up to town or Up to Crypt level (currLvl._dLevelIdx - 21)
		// assert(trigs[0]._tmsg == DVL_DWM_TWARPUP || trigs[0]._tmsg == DVL_DWM_PREVLVL);
		return 0;
	}
	if (L5_DOWN_WARP
	 || L5_DOWN_WARPx(dPiece[pcurspos.x][pcurspos.y + 1])) {
		// Down to Crypt level (currLvl._dLevelIdx - 19)
		// assert(trigs[1]._tmsg == DVL_DWM_NEXTLVL);
		return 1;
	}
	/*if (currLvl._dLevelIdx == DLV_CRYPT1) {
		if (L5_TOWN_WARP) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == DVL_DWM_TWARPUP) {
					// Up to town
					return i;
				}
			}
		}
	}*/
	return -1;
}

static int ForceL6Trig()
{
	if (L6_UP_WARP
	 || L6_UP_WARPx(dPiece[pcurspos.x][pcurspos.y + 1])) {
		// Up to town or Up to Nest level (currLvl._dLevelIdx - 17)
		// assert(trigs[0]._tmsg == DVL_DWM_TWARPUP || trigs[0]._tmsg == DVL_DWM_PREVLVL);
		return 0;
	}

	if (L6_DOWN_WARPx(dPiece[pcurspos.x][pcurspos.y])
	 || L6_DOWN_WARPx(dPiece[pcurspos.x + 1][pcurspos.y])) {
		// Down to level (currLvl._dLevelIdx - 15)
		// assert(trigs[1]._tmsg == DVL_DWM_NEXTLVL);
		return 1;
	}

	/*if (currLvl._dLevelIdx == DLV_NEST1) {
		if (L6_TOWN_WARP
		 || L6_TOWN_WARPx(dPiece[pcurspos.x][pcurspos.y + 1]) ) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == DVL_DWM_TWARPUP) {
					// Up to town
					return i;
				}
			}
		}
	}*/
	return -1;
}
#endif

static int ForceQuests()
{
	int i;

	for (i = numtrigs - 1; i >= 0; i--) {
		if (trigs[i]._tmsg == DVL_DWM_SETLVL) {
			/*      ^
			 *      |
			 * ----++------>
			 *    +++
			 *    ++|
			 *      |
			 */
			int dx = pcurspos.x - (trigs[i]._tx - 1);
			int dy = pcurspos.y - (trigs[i]._ty - 1);
			if (abs(dx) <= 1 && abs(dy) <= 1 // select the 3x3 square around (-1;-1)
			 && abs(dx - dy) < 2) {          // exclude the top left and bottom right positions
				return i;
			}
		}
	}

	return -1;
}

static int ForceSKingTrig()
{
	if (L1_UP_WARP) {
		// Back to Level (questlist[Q_SKELKING]._qdlvl)
		return 0;
	}

	return -1;
}

static int ForceSChambTrig()
{
	if (L2_DOWN_WARP) {
		// Back to Level (questlist[Q_BCHAMB]._qdlvl)
		return 0;
	}

	return -1;
}

static int ForcePWaterTrig()
{
	if (L3_DOWN_WARP
	 || L3_DOWN_WARPx(dPiece[pcurspos.x + 1][pcurspos.y])
	 || L3_DOWN_WARPx(dPiece[pcurspos.x + 1][pcurspos.y + 1])
	 /*|| L3_DOWN_WARPx(dPiece[pcurspos.x + 2][pcurspos.y + 1])*/) {
		// Back to Level (questlist[Q_PWATER]._qdlvl)
		return 0;
	}

	return -1;
}

void InitView(int entry)
{
	int type;

	if (entry == ENTRY_WARPLVL) {
		GetPortalLvlPos();
		return;
	}

	if (currLvl._dLevelIdx == DLV_TOWN) {
		if (entry == ENTRY_MAIN) {
			// New game
			ViewX = 65 + DBORDERX;
			ViewY = 58 + DBORDERY;
		/*} else if (entry == ENTRY_PREV) { // Cathedral
			ViewX = 15 + DBORDERX;
			ViewY = 21 + DBORDERY;*/
		} else if (entry == ENTRY_TWARPUP) {
			switch (gbTWarpFrom) {
			case TWARP_CATHEDRAL:
				ViewX = 15 + DBORDERX;
				ViewY = 21 + DBORDERY;
				break;
			case TWARP_CATACOMB:
				ViewX = 39 + DBORDERX;
				ViewY = 12 + DBORDERY;
				break;
			case TWARP_CAVES:
				ViewX = 8 + DBORDERX;
				ViewY = 59 + DBORDERY;
				break;
			case TWARP_HELL:
				ViewX = 30 + DBORDERX;
				ViewY = 70 + DBORDERY;
				break;
#ifdef HELLFIRE
			case TWARP_CRYPT:
				ViewX = 26 + DBORDERX;
				ViewY = 15 + DBORDERY;
				break;
			case TWARP_NEST:
				ViewX = 69 + DBORDERX;
				ViewY = 52 + DBORDERY;
				break;
#endif
			default:
				ASSUME_UNREACHABLE
				break;
			}
		} else if (entry == ENTRY_RETOWN) {
			// Restart in Town
			ViewX = 63 + DBORDERX;
			ViewY = 70 + DBORDERY;
		}
		return;
	}

	switch (entry) {
	case ENTRY_MAIN:
		type = DWARP_ENTRY;
		break;
	case ENTRY_PREV:
		type = DWARP_EXIT;
		break;
	case ENTRY_SETLVL:
		type = DWARP_ENTRY;
		break;
	case ENTRY_RTNLVL:
		type = DWARP_SIDE;
		if (pWarps[type]._wtype == WRPT_NONE) {
			// return from the betrayer side-map - TODO: better solution?
			assert(currLvl._dLevelIdx == DLV_HELL3);
			type = DWARP_EXIT;
			ViewX = pWarps[type]._wx;
			ViewY = pWarps[type]._wy;
			assert(pWarps[type]._wtype == WRPT_L4_PENTA);
			ViewX += -1;
			ViewY += -1;
			return;
		}
		break;
	case ENTRY_LOAD:    // set from the save file
	case ENTRY_WARPLVL: // should not happen
		return;
	case ENTRY_TWARPDN:
		type = DWARP_TOWN;
		if (pWarps[type]._wtype == WRPT_NONE)
			type = DWARP_ENTRY; // MAIN vs TWARPDN from town
		break;
	case ENTRY_TWARPUP: // should not happen
	case ENTRY_RETOWN:  // should not happen
		return;
	}

	ViewX = pWarps[type]._wx;
	ViewY = pWarps[type]._wy;
	type = pWarps[type]._wtype;
	switch (type) {
	case WRPT_NONE:
		break; // should not happen
	case WRPT_L1_UP:
		ViewX += 1;
		ViewY += 2;
		break;
	case WRPT_L1_DOWN:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L1_SKING:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L1_PWATER:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L2_UP:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L2_DOWN:
		ViewX += -1;
		ViewY += 1;
		break;
	case WRPT_L3_UP:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L3_DOWN:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L4_UP:
		ViewX += 1;
		ViewY += 2;
		break;
	case WRPT_L4_DOWN:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L4_PENTA:
		ViewX += 1;
		ViewY += 2;
		break;
	case WRPT_CIRCLE:
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void InitTriggers()
{
	if (!currLvl._dSetLvl) {
		switch (currLvl._dType) {
		case DTYPE_TOWN:
			InitTownTriggers();
			return;
		case DTYPE_CATHEDRAL:
			InitL1Triggers();
			break;
		case DTYPE_CATACOMBS:
			InitL2Triggers();
			break;
		case DTYPE_CAVES:
			InitL3Triggers();
			break;
		case DTYPE_HELL:
			InitL4Triggers();
			break;
#ifdef HELLFIRE
		case DTYPE_CRYPT:
			InitL5Triggers();
			break;
		case DTYPE_NEST:
			InitL6Triggers();
			break;
#endif
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else {
		switch (currLvl._dLevelIdx) {
		case SL_SKELKING:
			InitSKingTriggers();
			break;
		case SL_BONECHAMB:
			InitSChambTriggers();
			break;
		//case SL_MAZE:
		//	break;
		case SL_POISONWATER:
			InitPWaterTriggers();
			break;
		case SL_VILEBETRAYER:
			InitNoTriggers();
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}
}

void CheckTrigForce()
{
	int trignum;

	if (!currLvl._dSetLvl) {
		switch (currLvl._dType) {
		case DTYPE_TOWN:
			trignum = ForceTownTrig();
			break;
		case DTYPE_CATHEDRAL:
			trignum = ForceL1Trig();
			break;
		case DTYPE_CATACOMBS:
			trignum = ForceL2Trig();
			break;
		case DTYPE_CAVES:
			trignum = ForceL3Trig();
			break;
		case DTYPE_HELL:
			trignum = ForceL4Trig();
			break;
#ifdef HELLFIRE
		case DTYPE_CRYPT:
			trignum = ForceL5Trig();
			break;
		case DTYPE_NEST:
			trignum = ForceL6Trig();
			break;
#endif
		default:
			ASSUME_UNREACHABLE
			trignum = -1;
		}
		if (trignum == -1) {
			trignum = ForceQuests();
		}
	} else {
		switch (currLvl._dLevelIdx) {
		case SL_SKELKING:
			trignum = ForceSKingTrig();
			break;
		case SL_BONECHAMB:
			trignum = ForceSChambTrig();
			break;
		case SL_POISONWATER:
			trignum = ForcePWaterTrig();
			break;
		case SL_VILEBETRAYER:
			trignum = -1;
			break;
		default:
			ASSUME_UNREACHABLE
			trignum = -1;
			break;
		}
	}
	pcurstrig = trignum;
	if (trignum != -1) {
		pcurspos.x = trigs[trignum]._tx;
		pcurspos.y = trigs[trignum]._ty;
	}
}

void CheckTriggers()
{
	PlayerStruct* p;
	int i, fom, lvl;

	p = &myplr;
	if (p->_pmode != PM_STAND)
		return;

	for (i = 0; i < numtrigs; i++) {
		if (p->_px != trigs[i]._tx || p->_py != trigs[i]._ty) {
			continue;
		}

		fom = trigs[i]._tmsg;
		switch (fom) {
		case DVL_DWM_NEXTLVL:
			lvl = currLvl._dLevelIdx + 1;
			break;
		case DVL_DWM_PREVLVL:
			lvl = currLvl._dLevelIdx - 1;
			break;
		case DVL_DWM_RTNLVL:
		case DVL_DWM_SETLVL:
			lvl = trigs[i]._tlvl;
			break;
		case DVL_DWM_TWARPDN:
			if (!(gbOpenWarps & (1 << i)))
				continue;
			lvl = trigs[i]._tlvl;
			break;
		case DVL_DWM_TWARPUP:
			lvl = DLV_TOWN;
			break;
		default:
			ASSUME_UNREACHABLE;
			break;
		}
		static_assert((int)DVL_DWM_TWARPUP < UCHAR_MAX, "CheckTriggers sends DVL_DWM_* value in BYTE field I.");
		static_assert((int)DVL_DWM_NEXTLVL < UCHAR_MAX, "CheckTriggers sends DVL_DWM_* value in BYTE field II.");
		static_assert((int)NUM_LEVELS < UCHAR_MAX, "CheckTriggers sends level index in BYTE field.");
		NetSendCmdBParam2(CMD_NEWLVL, fom, lvl);
		numtrigs = 0; // prevent triggering again
	}
}

DEVILUTION_END_NAMESPACE
