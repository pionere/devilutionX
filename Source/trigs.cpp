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

#define PIECE dPiece[cursmx][cursmy]
/** Specifies the dungeon piece IDs which constitute stairways leading down to the cathedral from town. */
#define TOWN_L1_WARP     (PIECE == 716 || (PIECE >= 723 && PIECE <= 728))
/** Specifies the dungeon piece IDs which constitute stairways leading down to the catacombs from town. */
#define TOWN_L2_WARP     (PIECE >= 1175 && PIECE <= 1178)
/** Specifies the dungeon piece IDs which constitute stairways leading down to the caves from town. */
#define TOWN_L3_WARP     (PIECE >= 1199 && PIECE <= 1220)
/** Specifies the dungeon piece IDs which constitute stairways leading down to hell from town. */
#define TOWN_L4_WARP     (PIECE >= 1240 && PIECE <= 1255)
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
#define L2_UP_WARP       (PIECE == 266 || PIECE == 267)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the catacombs. */
#define L2_DOWN_WARP     (PIECE >= 269 && PIECE <= 272)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the catacombs. */
#define L2_TOWN_WARP     (PIECE == 558 || PIECE == 559)
/** Specifies the dungeon piece IDs which constitute stairways leading up from the caves. */
#define L3_UP_WARP       (PIECE == 170 || PIECE == 171)
#define L3_UP_WARPx(x)   (x == 170 || x == 171)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the caves. */
#define L3_DOWN_WARP     (PIECE == 168)
#define L3_DOWN_WARPx(x) (x == 168)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the caves. */
//#define L3_TOWN_WARP     (PIECE == 548 || PIECE == 549 || PIECE == 559 || PIECE == 560)
#define L3_TOWN_WARP     (PIECE >= 548 && PIECE <= 560 && (PIECE <= 549 || PIECE >= 559))
#define L3_TOWN_WARPx(x) (x >= 548 && x <= 560 && (x <= 549 || x >= 559))
/** Specifies the dungeon piece IDs which constitute stairways leading up from hell. */
#define L4_UP_WARP       (PIECE == 82 || (PIECE >= 90 && PIECE <= 97 && PIECE != 91 && PIECE != 93))
/** Specifies the dungeon piece IDs which constitute stairways leading down from hell. */
#define L4_DOWN_WARP     ((PIECE >= 130 && PIECE <= 133) || PIECE == 120)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from hell. */
#define L4_TOWN_WARP     (PIECE == 421 || (PIECE >= 429 && PIECE <= 436 && PIECE != 430 && PIECE != 432))
/** Specifies the dungeon piece IDs which constitute stairways leading down to Diablo from hell. */
#define L4_PENTA_WARP    (PIECE >= 353 && PIECE <= 384)
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
		trigs[TWARP_HELL]._tx = 31 + DBORDERX;
		trigs[TWARP_HELL]._ty = 70 + DBORDERY;
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
	int i, j;

	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 129) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = currLvl._dLevelIdx == DLV_CATHEDRAL1 ? DVL_DWM_TWARPUP : DVL_DWM_PREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 115) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
				numtrigs++;
			}
		}
	}
}

static void InitL2Triggers()
{
	int i, j;

	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 267 && (quests[Q_SCHAMB]._qactive == QUEST_NOTAVAIL || abs(quests[Q_SCHAMB]._qtx - i) > 1 || abs(quests[Q_SCHAMB]._qty - j) > 1)) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 559) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
				numtrigs++;
			} else if (dPiece[i][j] == 271) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
				numtrigs++;
			}
		}
	}
}

static void InitL3Triggers()
{
	int i, j;

	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 171) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 168) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 549) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
				numtrigs++;
			}
		}
	}
}

static void InitL4Triggers()
{
	int i, j;

	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 82) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 421) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
				numtrigs++;
			} else if (dPiece[i][j] == 120) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
				numtrigs++;
			}
		}
	}

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 370 && quests[Q_BETRAYER]._qactive == QUEST_DONE) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
				numtrigs++;
			}
		}
	}
}

#ifdef HELLFIRE
static void InitL5Triggers()
{
	int i, j;

	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			/*if (dPiece[i][j] == 184) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
				numtrigs++;
			} else*/ if (dPiece[i][j] == 158) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = currLvl._dLevelIdx == DLV_CRYPT1 ? DVL_DWM_TWARPUP : DVL_DWM_PREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 126) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
				numtrigs++;
			}
		}
	}
}

static void InitL6Triggers()
{
	int i, j;

	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 66) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = currLvl._dLevelIdx == DLV_NEST1 ? DVL_DWM_TWARPUP : DVL_DWM_PREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 63) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
				numtrigs++;
			} /*else if (dPiece[i][j] == 80) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
				numtrigs++;
			}*/
		}
	}
}
#endif

static void InitQuestTriggers()
{
	int i;
	QuestStruct* qs;

	for (i = 0; i < NUM_QUESTS; i++) {
		qs = &quests[i];
		if (questlist[i]._qslvl != 0 && i != Q_BETRAYER
		 && currLvl._dLevelIdx == questlist[i]._qdlvl && qs->_qactive != QUEST_NOTAVAIL) {
			trigs[numtrigs]._tx = qs->_qtx;
			trigs[numtrigs]._ty = qs->_qty;
			trigs[numtrigs]._tmsg = DVL_DWM_SETLVL;
			trigs[numtrigs]._tlvl = questlist[i]._qslvl;
			numtrigs++;
		}
	}
}

static void InitSKingTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = DBORDERX + 66;
	trigs[0]._ty = DBORDERY + 26;
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
}

static void InitSChambTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = DBORDERX + 54;
	trigs[0]._ty = DBORDERY + 23;
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
}

static void InitPWaterTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = DBORDERX + 14;
	trigs[0]._ty = DBORDERY + 67;
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
}

void InitVPEntryTrigger(bool recreate)
{
	int i;

	i = quests[Q_BETRAYER]._qactive == QUEST_DONE ? 2 : 1;

	trigs[i]._tx = quests[Q_BETRAYER]._qtx;
	trigs[i]._ty = quests[Q_BETRAYER]._qty;
	trigs[i]._tmsg = DVL_DWM_SETLVL;
	trigs[i]._tlvl = questlist[Q_BETRAYER]._qslvl;
	numtrigs = i + 1;

	AddMissile(0, 0, quests[Q_BETRAYER]._qtx, quests[Q_BETRAYER]._qty, 0, MIS_RPORTAL, MST_NA, -1, recreate ? -1 : 0);
}

void InitVPReturnTrigger(bool recreate)
{
	numtrigs = 1;
	trigs[0]._tx = DBORDERX + 19;
	trigs[0]._ty = DBORDERX + 16;
	trigs[0]._tmsg = DVL_DWM_RTNLVL;

	AddMissile(0, 0, DBORDERX + 19, DBORDERY + 16, 0, MIS_RPORTAL, MST_NA, -1, recreate ? -1 : 0);
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
	int i;
	if (L1_UP_WARP) {
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_PREVLVL // Up to level (currLvl._dLevelIdx - 1)
			 || trigs[i]._tmsg == DVL_DWM_TWARPUP) { // // Up to town
				return i;
			}
		}
	}
	if (L1_DOWN_WARP) {
		// Down to level (currLvl._dLevelIdx + 1)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_NEXTLVL) {
				return i;
			}
		}
	}
	return -1;
}

static int ForceL2Trig()
{
	int i;

	if (L2_UP_WARP && (quests[Q_SCHAMB]._qactive == QUEST_NOTAVAIL || abs(quests[Q_SCHAMB]._qtx - cursmx) > 1 || abs(quests[Q_SCHAMB]._qty - cursmy) > 1)) {
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_PREVLVL) {
				// Up to level (currLvl._dLevelIdx - 1)
				return i;
			}
		}
	}

	if (L2_DOWN_WARP) {
		// Down to level (currLvl._dLevelIdx + 1)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_NEXTLVL) {
				return i;
			}
		}
	}

	if (currLvl._dLevelIdx == DLV_CATACOMBS1) {
		if (L2_TOWN_WARP) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == DVL_DWM_TWARPUP) {
					// Up to town
					return i;
				}
			}
		}
	}

	return -1;
}

static int ForceL3Trig()
{
	int i;

	if (L3_UP_WARP
	 || L3_UP_WARPx(dPiece[cursmx][cursmy + 1])) {
		// Up to level (currLvl._dLevelIdx - 1)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_PREVLVL) {
				return i;
			}
		}
	}

	if (L3_DOWN_WARP
	 || L3_DOWN_WARPx(dPiece[cursmx + 1][cursmy])
	 || L3_DOWN_WARPx(dPiece[cursmx + 1][cursmy + 1])
	 || L3_DOWN_WARPx(dPiece[cursmx + 2][cursmy + 1])) {
		// Down to level (currLvl._dLevelIdx + 1)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_NEXTLVL) {
				return i;
			}
		}
	}

	if (currLvl._dLevelIdx == DLV_CAVES1) {
		if (L3_TOWN_WARP
		 || L3_TOWN_WARPx(dPiece[cursmx][cursmy + 1])) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == DVL_DWM_TWARPUP) {
					// Up to town
					return i;
				}
			}
		}
	}
	return -1;
}

static int ForceL4Trig()
{
	int i;

	if (L4_UP_WARP) {
		// Up to level (currLvl._dLevelIdx - 1)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_PREVLVL) {
				return i;
			}
		}
	}

	if (L4_DOWN_WARP) {
		// Down to level (currLvl._dLevelIdx + 1)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_NEXTLVL) {
				return i;
			}
		}
	}

	if (currLvl._dLevelIdx == DLV_HELL1) {
		if (L4_TOWN_WARP) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == DVL_DWM_TWARPUP) {
					// Up to town
					return i;
				}
			}
		}
	}

	if (currLvl._dLevelIdx == DLV_HELL3) {
		if (L4_PENTA_WARP) {
			// Down to Diablo
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == DVL_DWM_NEXTLVL) {
					return i;
				}
			}
		}
	}

	return -1;
}

#ifdef HELLFIRE
static int ForceL5Trig()
{
	int i;

	if (L5_UP_WARP) {
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_PREVLVL // Up to Crypt level (currLvl._dLevelIdx - 21)
			 || trigs[i]._tmsg == DVL_DWM_TWARPUP) { // // Up to town
				return i;
			}
		}
	}
	if (L5_DOWN_WARP
	 || L5_DOWN_WARPx(dPiece[cursmx][cursmy + 1])) {
		// Down to Crypt level (currLvl._dLevelIdx - 19)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_NEXTLVL) {
				return i;
			}
		}
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
	int i;

	if (L6_UP_WARP
	 || L6_UP_WARPx(dPiece[cursmx][cursmy + 1])) {
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_PREVLVL // Up to Nest level (currLvl._dLevelIdx - 17)
			 || trigs[i]._tmsg == DVL_DWM_TWARPUP) { // Up to town
				return i;
			}
		}
	}

	if (L6_DOWN_WARPx(dPiece[cursmx][cursmy])
	 || L6_DOWN_WARPx(dPiece[cursmx + 1][cursmy])) {
		// Down to level (currLvl._dLevelIdx - 15)
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == DVL_DWM_NEXTLVL) {
				return i;
			}
		}
	}

	/*if (currLvl._dLevelIdx == DLV_NEST1) {
		if (L6_TOWN_WARP
		 || L6_TOWN_WARPx(dPiece[cursmx][cursmy + 1]) ) {
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

static void Freeupstairs()
{
	int i, tx, ty, xx, yy;

	for (i = 0; i < numtrigs; i++) {
		tx = trigs[i]._tx;
		ty = trigs[i]._ty;

		for (yy = -2; yy <= 2; yy++) {
			for (xx = -2; xx <= 2; xx++) {
				dFlags[tx + xx][ty + yy] |= BFLAG_POPULATED;
			}
		}
	}
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
		// Back to Level (questlist[Q_SCHAMB]._qdlvl)
		return 0;
	}

	return -1;
}

static int ForcePWaterTrig()
{
	if (L3_DOWN_WARP) {
		// Back to Level (questlist[Q_PWATER]._qdlvl)
		return 0;
	}

	return -1;
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
		InitQuestTriggers();
		Freeupstairs();
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
		cursmx = trigs[trignum]._tx;
		cursmy = trigs[trignum]._ty;
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
			lvl = gnReturnLvl;
			break;
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
