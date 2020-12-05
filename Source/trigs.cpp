/**
 * @file trigs.cpp
 *
 * Implementation of functionality for triggering events when the player enters an area.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

BOOL trigflag;
int numtrigs;
TriggerStruct trigs[MAXTRIGGERS];
unsigned char townwarps;
int TWarpFrom;

#define PIECE dPiece[cursmx][cursmy]
/** Specifies the dungeon piece IDs which constitute stairways leading down to the cathedral from town. */
#define TOWN_L1_WARP		(PIECE == 716 || (PIECE >= 723 && PIECE <= 728))
/** Specifies the dungeon piece IDs which constitute stairways leading down to the catacombs from town. */
#define TOWN_L2_WARP		(PIECE >= 1175 && PIECE <= 1178)
/** Specifies the dungeon piece IDs which constitute stairways leading down to the caves from town. */
#define TOWN_L3_WARP		(PIECE >= 1199 && PIECE <= 1220)
/** Specifies the dungeon piece IDs which constitute stairways leading down to hell from town. */
#define TOWN_L4_WARP		(PIECE >= 1240 && PIECE <= 1255)
/** Specifies the dungeon piece IDs which constitute stairways leading down to the hive from town. */
#define TOWN_L5_WARP		(PIECE >= 1307 && PIECE <= 1310)
/** Specifies the dungeon piece IDs which constitute stairways leading down to the crypt from town. */
#define TOWN_L6_WARP		(PIECE >= 1331 && PIECE <= 1338)
/** Specifies the dungeon piece IDs which constitute stairways leading up from the cathedral. */
#define L1_UP_WARP			(PIECE >= 127 && PIECE <= 140 && PIECE != 128 && PIECE != 134 && PIECE != 136)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the cathedral. */
//							{ 106, 107, 108, 109, 110, /*111,*/ 112, /*113,*/ 114, 115, /*116, 117,*/ 118, }
#define L1_DOWN_WARP		((PIECE >= 106 && PIECE <= 115 && PIECE != 111 && PIECE != 113) || PIECE == 118)
/** Specifies the dungeon piece IDs which constitute stairways leading up from the catacombs. */
#define L2_UP_WARP			(PIECE == 266 || PIECE == 267)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the catacombs. */
#define L2_DOWN_WARP		(PIECE >= 269 && PIECE <= 272)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the catacombs. */
#define L2_TOWN_WARP		(PIECE == 558 || PIECE == 559)
/** Specifies the dungeon piece IDs which constitute stairways leading up from the caves. */
#define L3_UP_WARP			(PIECE == 170 || PIECE == 171)
#define L3_UP_WARPx(x)		(x == 170 || x == 171)
/** Specifies the dungeon piece IDs which constitute stairways leading down from the caves. */
#define L3_DOWN_WARP		(PIECE == 168)
#define L3_DOWN_WARPx(x)	(x == 168)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from the caves. */
//#define L3_TOWN_WARP		(PIECE == 548 || PIECE == 549 || PIECE == 559 || PIECE == 560)
#define L3_TOWN_WARP		(PIECE >= 548 && PIECE <= 560 && (PIECE <= 549 || PIECE >= 559))
#define L3_TOWN_WARPx(x)	(x >= 548 && x <= 560 && (x <= 549 || x >= 559))
/** Specifies the dungeon piece IDs which constitute stairways leading up from hell. */
#define L4_UP_WARP			(PIECE == 82 || (PIECE >= 90 && PIECE <= 97 && PIECE != 91 && PIECE != 93))
/** Specifies the dungeon piece IDs which constitute stairways leading down from hell. */
#define L4_DOWN_WARP		(PIECE >= 130 && PIECE <= 133 || PIECE == 120)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from hell. */
#define L4_TOWN_WARP		(PIECE == 421 || (PIECE >= 429 && PIECE <= 436 && PIECE != 430 && PIECE != 432))
/** Specifies the dungeon piece IDs which constitute stairways leading down to Diablo from hell. */
#define L4_PENTA_WARP		(PIECE >= 353 && PIECE <= 384)
#ifdef HELLFIRE
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from crypt. */
#define L5_TOWN_WARP		(PIECE >= 172 && PIECE <= 185 && (PIECE <= 179 || PIECE >= 184))
/** Specifies the dungeon piece IDs which constitute stairways leading up from crypt. */
#define L5_UP_WARP			(PIECE >= 149 && PIECE <= 159 && (PIECE <= 153 || PIECE >= 158))
/** Specifies the dungeon piece IDs which constitute stairways leading down from crypt. */
#define L5_DOWN_WARP		(PIECE >= 125 && PIECE <= 126)
#define L5_DOWN_WARPx(x)	(x >= 125 && x <= 126)
/** Specifies the dungeon piece IDs which constitute stairways leading up to town from nest. */
#define L6_TOWN_WARP		(PIECE >= 79 && PIECE <= 80)
#define L6_TOWN_WARPx(x)	(x >= 79 && x <= 80)
/** Specifies the dungeon piece IDs which constitute stairways leading up from nest. */
#define L6_UP_WARP			(PIECE >= 65 && PIECE <= 66)
#define L6_UP_WARPx(x)		(x >= 65 && x <= 66)
/** Specifies the dungeon piece IDs which constitute stairways leading down from nest. */
#define L6_DOWN_WARPx(x)	(x == 61 || x == 63)
#define TR_CORNERSTONE		(PIECE == 317)
#endif

void InitNoTriggers()
{
	trigflag = FALSE;
	numtrigs = 0;
}

/*
 * Initialize the triggers in town.
 * Only the available triggers are set up correctly (numtrigs won't reflect this!).
 */
void InitTownTriggers()
{
	trigflag = FALSE;

	trigs[TWARP_CATHEDRAL]._tx = 15 + DBORDERX;
	trigs[TWARP_CATHEDRAL]._ty = 19 + DBORDERY;
	trigs[TWARP_CATHEDRAL]._tmsg = WM_DIABNEXTLVL;

	townwarps = GetOpenWarps();

	if (townwarps & (1 << TWARP_CATACOMB)) {
		trigs[TWARP_CATACOMB]._tx = 39 + DBORDERX;
		trigs[TWARP_CATACOMB]._ty = 11 + DBORDERY;
		trigs[TWARP_CATACOMB]._tmsg = WM_DIABTOWNWARP;
		trigs[TWARP_CATACOMB]._tlvl = 5;
	}
	if (townwarps & (1 << TWARP_CAVES)) {
		trigs[TWARP_CAVES]._tx = 7 + DBORDERX;
		trigs[TWARP_CAVES]._ty = 59 + DBORDERY;
		trigs[TWARP_CAVES]._tmsg = WM_DIABTOWNWARP;
		trigs[TWARP_CAVES]._tlvl = 9;
	}
	if (townwarps & (1 << TWARP_HELL)) {
		trigs[TWARP_HELL]._tx = 31 + DBORDERX;
		trigs[TWARP_HELL]._ty = 70 + DBORDERY;
		trigs[TWARP_HELL]._tmsg = WM_DIABTOWNWARP;
		trigs[TWARP_HELL]._tlvl = 13;
	}
#ifdef HELLFIRE
	if (townwarps & (1 << TWARP_HIVE)) {
		trigs[TWARP_HIVE]._tx = 70 + DBORDERX;
		trigs[TWARP_HIVE]._ty = 52 + DBORDERY;
		trigs[TWARP_HIVE]._tmsg = WM_DIABTOWNWARP;
		trigs[TWARP_HIVE]._tlvl = 17;
	}
	if (townwarps & (1 << TWARP_CRYPT)) {
		trigs[TWARP_CRYPT]._tx = 26 + DBORDERX;
		trigs[TWARP_CRYPT]._ty = 14 + DBORDERY;
		trigs[TWARP_CRYPT]._tmsg = WM_DIABTOWNWARP;
		trigs[TWARP_CRYPT]._tlvl = 21;
	}
#endif
	static_assert(NUM_TWARP <= MAXTRIGGERS, "Too many trigger in town.");
	numtrigs = NUM_TWARP;
}

void InitL1Triggers()
{
	int i, j;

	trigflag = FALSE;
	numtrigs = 0;
#ifdef HELLFIRE
	if (currlevel >= 17) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (dPiece[i][j] == 184) {
					trigs[numtrigs]._tx = i;
					trigs[numtrigs]._ty = j;
					trigs[numtrigs]._tmsg = WM_DIABTWARPUP;
					trigs[numtrigs]._tlvl = 0;
					numtrigs++;
				} else if (dPiece[i][j] == 158) {
					trigs[numtrigs]._tx = i;
					trigs[numtrigs]._ty = j;
					trigs[numtrigs]._tmsg = WM_DIABPREVLVL;
					numtrigs++;
				} else if (dPiece[i][j] == 126) {
					trigs[numtrigs]._tx = i;
					trigs[numtrigs]._ty = j;
					trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
					numtrigs++;
				}
			}
		}
		return;
	}
#endif
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 129) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABPREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 115) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
				numtrigs++;
			}
		}
	}
}

void InitL2Triggers()
{
	int i, j;

	trigflag = FALSE;
	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 267 && (i != quests[Q_SCHAMB]._qtx || j != quests[Q_SCHAMB]._qty)) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABPREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 559) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABTWARPUP;
				trigs[numtrigs]._tlvl = 0;
				numtrigs++;
			} else if (dPiece[i][j] == 271) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
				numtrigs++;
			}
		}
	}
}

void InitL3Triggers()
{
	int i, j;

	trigflag = FALSE;
	numtrigs = 0;
#ifdef HELLFIRE
	if (currlevel >= 17) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++) {
				if (dPiece[i][j] == 66) {
					trigs[numtrigs]._tx = i;
					trigs[numtrigs]._ty = j;
					trigs[numtrigs]._tmsg = WM_DIABPREVLVL;
					numtrigs++;
				} else if (dPiece[i][j] == 63) {
					trigs[numtrigs]._tx = i;
					trigs[numtrigs]._ty = j;
					trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
					numtrigs++;
				} else if (dPiece[i][j] == 80) {
					trigs[numtrigs]._tx = i;
					trigs[numtrigs]._ty = j;
					trigs[numtrigs]._tmsg = WM_DIABTWARPUP;
					numtrigs++;
				}
			}
		}
		return;
	}
#endif
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 171) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABPREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 168) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 549) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABTWARPUP;
				numtrigs++;
			}
		}
	}
}

void InitL4Triggers()
{
	int i, j;

	trigflag = FALSE;
	numtrigs = 0;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 82) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABPREVLVL;
				numtrigs++;
			} else if (dPiece[i][j] == 421) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABTWARPUP;
				trigs[numtrigs]._tlvl = 0;
				numtrigs++;
			} else if (dPiece[i][j] == 120) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
				numtrigs++;
			}
		}
	}

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 370 && quests[Q_BETRAYER]._qactive == QUEST_DONE) {
				trigs[numtrigs]._tx = i;
				trigs[numtrigs]._ty = j;
				trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
				numtrigs++;
			}
		}
	}
}

void InitSKingTriggers()
{
	trigflag = FALSE;
	numtrigs = 1;
	trigs[0]._tx = 82;
	trigs[0]._ty = 42;
	trigs[0]._tmsg = WM_DIABRTNLVL;
}

void InitSChambTriggers()
{
	trigflag = FALSE;
	numtrigs = 1;
	trigs[0]._tx = 70;
	trigs[0]._ty = 39;
	trigs[0]._tmsg = WM_DIABRTNLVL;
}

void InitPWaterTriggers()
{
	trigflag = FALSE;
	numtrigs = 1;
	trigs[0]._tx = 30;
	trigs[0]._ty = 83;
	trigs[0]._tmsg = WM_DIABRTNLVL;
}

void InitVPTriggers()
{
	trigflag = FALSE;
	numtrigs = 1;
	trigs[0]._tx = 35;
	trigs[0]._ty = 32;
	trigs[0]._tmsg = WM_DIABRTNLVL;
}

static BOOL ForceTownTrig()
{
	if (TOWN_L1_WARP) {
		copy_cstr(infostr, "Down to dungeon");
		cursmx = trigs[TWARP_CATHEDRAL]._tx;
		cursmy = trigs[TWARP_CATHEDRAL]._ty;
		return TRUE;
	}

	if (townwarps & (1 << TWARP_CATACOMB)) {
		if (TOWN_L2_WARP) {
			copy_cstr(infostr, "Down to catacombs");
			cursmx = trigs[TWARP_CATACOMB]._tx;
			cursmy = trigs[TWARP_CATACOMB]._ty;
			return TRUE;
		}
	}

	if (townwarps & (1 << TWARP_CAVES)) {
		if (TOWN_L3_WARP) {
			copy_cstr(infostr, "Down to caves");
			cursmx = trigs[TWARP_CAVES]._tx;
			cursmy = trigs[TWARP_CAVES]._ty;
			return TRUE;
		}
	}

	if (townwarps & (1 << TWARP_HELL)) {
		if (TOWN_L4_WARP) {
			copy_cstr(infostr, "Down to hell");
			cursmx = trigs[TWARP_HELL]._tx;
			cursmy = trigs[TWARP_HELL]._ty;
			return TRUE;
		}
	}

#ifdef HELLFIRE
	if (townwarps & (1 << TWARP_HIVE)) {
		if (TOWN_L5_WARP) {
			copy_cstr(infostr, "Down to Hive");
			cursmx = trigs[TWARP_HIVE]._tx;
			cursmy = trigs[TWARP_HIVE]._ty;
			return TRUE;
		}
	}
	if (townwarps & (1 << TWARP_CRYPT)) {
		if (TOWN_L6_WARP) {
			copy_cstr(infostr, "Down to Crypt");
			cursmx = trigs[TWARP_CRYPT]._tx;
			cursmy = trigs[TWARP_CRYPT]._ty;
			return TRUE;
		}
	}
#endif

	return FALSE;
}

static BOOL ForceL1Trig()
{
	int i;
#ifdef HELLFIRE

	if (currlevel >= 17) {
		if (L5_UP_WARP) {
			snprintf(infostr, sizeof(infostr), "Up to Crypt level %i", currlevel - 21);
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABPREVLVL) {
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}
		if (TR_CORNERSTONE) {
			copy_cstr(infostr, "Cornerstone of the World");
			return TRUE;
		}
		if (L5_DOWN_WARP
		 || L5_DOWN_WARPx(dPiece[cursmx][cursmy + 1])) {
			snprintf(infostr, sizeof(infostr), "Down to Crypt level %i", currlevel - 19);
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}
		if (currlevel == 21) {
			if (L5_TOWN_WARP) {
				for (i = 0; i < numtrigs; i++) {
					if (trigs[i]._tmsg == WM_DIABTWARPUP) {
						copy_cstr(infostr, "Up to town");
						cursmx = trigs[i]._tx;
						cursmy = trigs[i]._ty;
						return TRUE;
					}
				}
			}
		}
		return FALSE;
	}
#endif
	if (L1_UP_WARP) {
		if (currlevel > 1)
			snprintf(infostr, sizeof(infostr), "Up to level %i", currlevel - 1);
		else
			copy_cstr(infostr, "Up to town");
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABPREVLVL) {
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}
	if (L1_DOWN_WARP) {
		snprintf(infostr, sizeof(infostr), "Down to level %i", currlevel + 1);
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}
	return FALSE;
}

static BOOL ForceL2Trig()
{
	int i;

	if (L2_UP_WARP) {
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABPREVLVL) {
				snprintf(infostr, sizeof(infostr), "Up to level %i", currlevel - 1);
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}

	if (L2_DOWN_WARP) {
		snprintf(infostr, sizeof(infostr), "Down to level %i", currlevel + 1);
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}

	if (currlevel == 5) {
		if (L2_TOWN_WARP) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABTWARPUP) {
					copy_cstr(infostr, "Up to town");
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

static BOOL ForceL3Trig()
{
	int i;

#ifdef HELLFIRE
	if (currlevel >= 17) {
		if (L6_UP_WARP
		 || L6_UP_WARPx(dPiece[cursmx][cursmy + 1])) {
			snprintf(infostr, sizeof(infostr), "Up to Nest level %i", currlevel - 17);
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABPREVLVL) {
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}

		if (L6_DOWN_WARPx(dPiece[cursmx][cursmy])
		 || L6_DOWN_WARPx(dPiece[cursmx + 1][cursmy])) {
			snprintf(infostr, sizeof(infostr), "Down to level %i", currlevel - 15);
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}

		if (currlevel == 17) {
			if (L6_TOWN_WARP
			 || L6_TOWN_WARPx(dPiece[cursmx][cursmy + 1]) ) {
				for (i = 0; i < numtrigs; i++) {
					if (trigs[i]._tmsg == WM_DIABTWARPUP) {
						copy_cstr(infostr, "Up to town");
						cursmx = trigs[i]._tx;
						cursmy = trigs[i]._ty;
						return TRUE;
					}
				}
			}
		}
		return FALSE;
	}
#endif
	if (L3_UP_WARP
	 || L3_UP_WARPx(dPiece[cursmx][cursmy + 1])) {
		snprintf(infostr, sizeof(infostr), "Up to level %i", currlevel - 1);
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABPREVLVL) {
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}

	if (L3_DOWN_WARP
	 || L3_DOWN_WARPx(dPiece[cursmx + 1][cursmy])
	 || L3_DOWN_WARPx(dPiece[cursmx + 1][cursmy + 1])
	 || L3_DOWN_WARPx(dPiece[cursmx + 2][cursmy + 1])) {
		snprintf(infostr, sizeof(infostr), "Down to level %i", currlevel + 1);
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}

	if (currlevel == 9) {
		if (L3_TOWN_WARP
		 || L3_TOWN_WARPx(dPiece[cursmx][cursmy + 1])) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABTWARPUP) {
					copy_cstr(infostr, "Up to town");
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

static BOOL ForceL4Trig()
{
	int i;

	if (L4_UP_WARP) {
		snprintf(infostr, sizeof(infostr), "Up to level %i", currlevel - 1);
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABPREVLVL) {
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}

	if (L4_DOWN_WARP) {
		snprintf(infostr, sizeof(infostr), "Down to level %i", currlevel + 1);
		for (i = 0; i < numtrigs; i++) {
			if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return TRUE;
			}
		}
	}

	if (currlevel == 13) {
		if (L4_TOWN_WARP) {
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABTWARPUP) {
					copy_cstr(infostr, "Up to town");
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}
	}

	if (currlevel == 15) {
		if (L4_PENTA_WARP) {
			copy_cstr(infostr, "Down to Diablo");
			for (i = 0; i < numtrigs; i++) {
				if (trigs[i]._tmsg == WM_DIABNEXTLVL) {
					cursmx = trigs[i]._tx;
					cursmy = trigs[i]._ty;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

void Freeupstairs()
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

static BOOL ForceSKingTrig()
{
	if (L1_UP_WARP) {
		snprintf(infostr, sizeof(infostr), "Back to Level %i", quests[Q_SKELKING]._qlevel);
		cursmx = trigs[0]._tx;
		cursmy = trigs[0]._ty;
		return TRUE;
	}

	return FALSE;
}

static BOOL ForceSChambTrig()
{
	if (L2_DOWN_WARP) {
		snprintf(infostr, sizeof(infostr), "Back to Level %i", quests[Q_SCHAMB]._qlevel);
		cursmx = trigs[0]._tx;
		cursmy = trigs[0]._ty;
		return TRUE;
	}

	return FALSE;
}

static BOOL ForcePWaterTrig()
{
	if (L3_DOWN_WARP) {
		snprintf(infostr, sizeof(infostr), "Back to Level %i", quests[Q_PWATER]._qlevel);
		cursmx = trigs[0]._tx;
		cursmy = trigs[0]._ty;
		return TRUE;
	}

	return FALSE;
}

void CheckTrigForce()
{
	trigflag = FALSE;
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (!sgbControllerActive && MouseY > PANEL_TOP - 1) {
#else
	if (MouseY > PANEL_TOP - 1) {
#endif
		return;
	}

	if (!setlevel) {
		switch (leveltype) {
		case DTYPE_TOWN:
			trigflag = ForceTownTrig();
			break;
		case DTYPE_CATHEDRAL:
			trigflag = ForceL1Trig();
			break;
		case DTYPE_CATACOMBS:
			trigflag = ForceL2Trig();
			break;
		case DTYPE_CAVES:
			trigflag = ForceL3Trig();
			break;
		case DTYPE_HELL:
			trigflag = ForceL4Trig();
			break;
		}
		if (leveltype != DTYPE_TOWN && !trigflag) {
			trigflag = ForceQuests();
		}
	} else {
		switch (setlvlnum) {
		case SL_SKELKING:
			trigflag = ForceSKingTrig();
			break;
		case SL_BONECHAMB:
			trigflag = ForceSChambTrig();
			break;
		case SL_POISONWATER:
			trigflag = ForcePWaterTrig();
			break;
		}
	}

	if (trigflag) {
		ClearPanel();
	}
}

void CheckTriggers()
{
	PlayerStruct *p;
	int x, y, i;
	char abortflag;

	p = &plr[myplr];
	if (p->_pmode != PM_STAND)
		return;

	for (i = 0; i < numtrigs; i++) {
		if (p->_px != trigs[i]._tx || p->_py != trigs[i]._ty) {
			continue;
		}

		switch (trigs[i]._tmsg) {
		case WM_DIABNEXTLVL:
#ifdef SPAWN
			if (currlevel >= 2) {
				NetSendCmdLoc(TRUE, CMD_WALKXY, p->_px, p->_py + 1);
				PlaySFX(PS_WARR18);
				InitDiabloMsg(EMSG_NOT_IN_SHAREWARE);
				break;
			}
#endif
			if (pcurs >= CURSOR_FIRSTITEM && DropItem())
				return;
			StartNewLvl(myplr, WM_DIABNEXTLVL, currlevel + 1);
			break;
		case WM_DIABPREVLVL:
			if (pcurs >= CURSOR_FIRSTITEM && DropItem())
				return;
			StartNewLvl(myplr, WM_DIABPREVLVL, currlevel - 1);
			break;
		case WM_DIABRTNLVL:
			StartNewLvl(myplr, WM_DIABRTNLVL, ReturnLvl);
			break;
		case WM_DIABTOWNWARP:
			if (!(townwarps & (1 << i)))
				continue;
			if (gbMaxPlayers != 1) {
				abortflag = EMSG_NONE;

				if (i == TWARP_CATACOMB && p->_pLevel < 8) {
					x = p->_px;
					y = p->_py + 1;
					abortflag = EMSG_REQUIRES_LVL_8;
				}

				if (i == TWARP_CAVES && p->_pLevel < 13) {
					x = p->_px + 1;
					y = p->_py;
					abortflag = EMSG_REQUIRES_LVL_13;
				}

				if (i == TWARP_HELL && p->_pLevel < 17) {
					x = p->_px;
					y = p->_py + 1;
					abortflag = EMSG_REQUIRES_LVL_17;
				}

				if (abortflag != EMSG_NONE) {
					PlaySFX(sgSFXSets[SFXS_PLR_43][p->_pClass]);
					InitDiabloMsg(abortflag);
					NetSendCmdLoc(TRUE, CMD_WALKXY, x, y);
					return;
				}
			}

			StartNewLvl(myplr, WM_DIABTOWNWARP, trigs[i]._tlvl);
			break;
		case WM_DIABTWARPUP:
			TWarpFrom = currlevel;
			StartNewLvl(myplr, WM_DIABTWARPUP, 0);
			break;
		default:
			app_fatal("Unknown trigger msg");
			break;
		}
	}
}

DEVILUTION_END_NAMESPACE
