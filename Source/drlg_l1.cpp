/**
 * @file drlg_l1.cpp
 *
 * Implementation of the cathedral level generation algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** The default external tile. */
#define BASE_MEGATILE_L1 22
/** The default floor tile. */
#define DEFAULT_MEGATILE_L1 13
/** Size of the main chambers in the dungeon. */
#define CHAMBER_SIZE 10
/** Shadow type of the base floor(13). */
#define SF 4

/**  The number of generated rooms */
static int nRoomCnt;
/** Specifies whether to generate a vertical or horizontal rooms in the Cathedral. */
static BOOLEAN ChambersVertical;
/** Specifies whether to generate a room at position 1 in the Cathedral. */
static BOOLEAN ChambersFirst;
/** Specifies whether to generate a room at position 2 in the Cathedral. */
static BOOLEAN ChambersMiddle;
/** Specifies whether to generate a room at position 3 in the Cathedral. */
static BOOLEAN ChambersLast;

/*
 * Maps tile IDs to their corresponding undecorated tile type.
 */
static const BYTE L1BTYPES[207] = {
	// clang-format off
	0, 1, 2, 0, 0, 0, 0, 0, 0, 0,
	0, 3, 4, 5, 0, 0, 0, 0, 0, 0, // 10..
	0, 0, 0, 0, 0, 6, 0, 0, 0, 0, // 20..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 30..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 50..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 70..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80..
	0, 0, 0, 0, 0, 0, 0, 2, 2, 2, // 90..
	1, 1, 3, 6, 5, 5, 5, 1, 2, 1, //100..
	2, 1, 2, 1, 2, 2, 2, 2, 4, 0, //110..
	0, 3, 1, 3, 1, 5, 0, 0, 0, 0, //120..
	0, 0, 0, 5, 5, 5, 5, 5, 5, 0, //130..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180..
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190..
	0, 0, 0, 0, 0, 0, 0
	// clang-format on
};
/** Miniset: Entry point of the dynamic maps. */
const BYTE L1DYNENTRY[] = {
	// clang-format off
	3, 3, // width, height -- larger miniset to prevent theme-room placement

	 2,  2,  2, // search
	13, 13, 13,
	13, 13, 13,

	 0,   0, 0, // replace
	 0, 134, 0,
	 0,   0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L5DYNENTRY[] = {
	// clang-format off
	2, 2, // width, height

	 2,  2, // search
	13, 13,

	0, 0, // replace
	195, 0,
	// clang-format on
};
#endif
/** Miniset: stairs up on a corner wall. */
//const BYTE STAIRSUP[] = {
//	// clang-format off
//	4, 4, // width, height
//
//	13, 13, 13, 13, // search
//	 2,  2,  2,  2,
//	13, 13, 13, 13,
//	13, 13, 13, 13,
//
//	 0, 66,  6,  0, // replace
//	63, 64, 65,  0,
//	 0, 67, 68,  0,
//	 0,  0,  0,  0,
/*    0,  0,   137,138,    13,  2,     0,  0,	// MegaTiles
	  0,  0,   139,140,     3,  4,     0,  0,

	  5,127,   129,130,   133,134,     0,  0,
	  7,128,   131,132,   135,136,     0,  0,

	  0,  0,   141,142,   143,  2,     0,  0,
	  0,  0,     7,  4,     7,  4,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
/*    0,  0,   192,193,    21, 22,     0,  0,	// MegaTiles - HELLFIRE
	  0,  0,   194,195,    23, 24,     0,  0,

	180,181,   184,185,   188,189,     0,  0,
	182,183,   186,187,   190,191,     0,  0,

	  0,  0,   196,197,   200,201,     0,  0,
	  0,  0,   198,199,   202,203,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
//	// clang-format on
//};
#ifdef HELLFIRE
static const BYTE L5USTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	22, 22, 22, 22, // search
	22, 22, 22, 22,
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 54, 23,  0, // replace
	 0, 53, 18,  0,
	55, 56, 57,  0,
	99, 59, 60,  0,
	 0,  0,  0,  0
/*    0,  0   151,152,    73, 64,     0,  0,	// MegaTiles
	  0,  0   153,130,    65, 66,     0,  0,

	  0,  0   149,130,    63, 64,     0,  0,
	  0,  0   150,130,    65, 66,     0,  0,

	154,155,  158,159,   162,163,     0,  0,
	156,157,  160,161,   164,165,     0,  0,

	166,167,  168,169,   170,171,     0,  0,
	 47, 48,   47, 48,    47, 48,     0,  0,

	  0,  0,    0,  0,     0,  0,     0,  0,
	  0,  0,    0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#endif
/** Miniset: stairs up. */
static const BYTE L1USTAIRS[] = {
	// clang-format off
	4, 4, // width, height

	22, 22, 22, 22, // search
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 66, 23,  0, // replace
	63, 64, 65,  0,
	 0, 67, 68,  0,
	 0,  0,  0,  0,
/*    0,  0,   137,138,    41, 31,     0,  0,	// MegaTiles
	  0,  0,   139,140,    39, 33,     0,  0,

	  5,127,   129,130,   133,134,     0,  0,
	  7,128,   131,132,   135,136,     0,  0,

	  0,  0,   141,142,   143,  2,     0,  0,
	  0,  0,     7,  4,     7,  4,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
/*
 * Miniset: stairs down.
 * Added an extra line to the top to prevent placing it too close to other entities (eg. upstair)
 */
static const BYTE L1DSTAIRS[] = {
	// clang-format off
	4, 4, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0,  0,  0,  0,
	62, 57, 58,  0, // replace
	61, 59, 60,  0,
	 0,  0,  0,  0,
/*	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, 

	124,  2,   106,107,   110,111,     0,  0,	// MegaTiles
	125,126,   108,109,   112,113,     0,  0,

	122,123,   114,115,   118,119,     0,  0,
	  7,  4,   116,117,   120,121,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#ifdef HELLFIRE
static const BYTE L5DSTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0,  0,  0,  0, // replace
	 0, 48, 51,  0,
	 0, 47, 50,  0,
	45, 46, 49,  0,
	 0,  0,  0,  0,
	// clang-format on
};
/* same look as L5USTAIRS
const BYTE L5TWARP[] = {
	// clang-format off
	4, 5, // width, height

	22, 22, 22, 22, // search
	22, 22, 22, 22,
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 62, 23,  0, // replace
	 0, 61, 18,  0,
	63, 64, 65,  0,
	66, 67, 68,  0,
	 0,  0,  0,  0,
	// clang-format on
};*/
#endif
/** Miniset: candlestick. */
static const BYTE LAMPS[] = {
	// clang-format off
	2, 2, // width, height

	13, 13, // search
	13, 13,

	 0,   0, // replace
	 0, 128,
	// clang-format on
};
/** Miniset: Poisoned Water Supply entrance. */
static const BYTE PWATERIN[] = {
	// clang-format off
	6, 6, // width, height

	13, 13, 13, 13, 13, 13, // search
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,

	 0,     0,   0,   0,   0, 0, // replace
	 0,    21,  19,  19,  84, 0,
	 159,  18,  22,  22,  83, 0,
	 142,  85, 206,  80,  81, 0,
	 0,     0, 134, 135,   0, 0,
	 0,     0,   0,   0,   0, 0,
	// clang-format on
};
#ifdef HELLFIRE
/** Miniset: Column on the northern side of a vertical wall 1. */
static const BYTE L5VERTCOL1[] = { 1, 1, 11, 95 };
/** Miniset: Column on the northern side of a horizontal wall 1. */
// static const BYTE L5HORIZCOL1[] = { 1, 1, 12, 96 };
static const BYTE L5HORIZCOL1a[] = {
	// clang-format off
	1, 2, // width, height

	205, // search
	 12,

	208, // replace
	 96,
	// clang-format on
};
static const BYTE L5HORIZCOL1b[] = {
	// clang-format off
	1, 2, // width, height

	204, // search
	 12,

	217, // replace
	 96,
	// clang-format on
};
/*static const BYTE L5HORIZCOL1c[] = {
	// clang-format off
	1, 2, // width, height

	111, // search
	 12,

	216, // replace
	 96,
	// clang-format on
};*/
/** Miniset: Prepare random large vertical wall tile */
static const BYTE L5PREVERTWALL[] = {
	// clang-format off
	1, 3, // width, height

	1, // search
	1,
	1,

	91, // replace
	90,
	89,
	// clang-format on
};
/** Miniset: Prepare random large horizontal wall tile */
static const BYTE L5PREHORIZWALL[] = {
	// clang-format off
	3, 1, // width, height

	 2,  2,  2, // search

	94, 93, 92, // replace
	// clang-format on
};
/** Miniset: Use random floor tile 1. */
static const BYTE L5RNDFLOOR1[] = { 1, 1, 13, 97 };
/** Miniset: Use random floor tile 2. */
static const BYTE L5RNDFLOOR2[] = { 1, 1, 13, 98 };
/** Miniset: Use random floor tile 3. */
static const BYTE L5RNDFLOOR3[] = { 1, 1, 13, 99 };
/** Miniset: Use random floor tile 4. */
static const BYTE L5RNDFLOOR4[] = { 1, 1, 13, 100 };
/** Miniset: Use random large floor tile. */
static const BYTE L5RNDLFLOOR1[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 101, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: Lava fountain bowl. */
static const BYTE L5RNDLFLOOR2[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	210,   0, 0, // replace
	 45, 167, 0,
	  0,   0, 0,
	// clang-format on
};
/** Miniset: Mooring on a stone. */
static const BYTE L5RNDLFLOOR3[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 168, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: A stone coffin(?). */
static const BYTE L5RNDLFLOOR4[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 169, 0,
	0,   0, 0,
};
/** Miniset: A stone. */
static const BYTE L5RNDLFLOOR5[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 170, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: A broken head of a stone-column. */
static const BYTE L5RNDLFLOOR6[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 171, 0,
	0,   0, 0,
	// clang-format on
};
/** Miniset: A leaf on the floor. */
static const BYTE L5RNDLFLOOR7[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	 0,   0, 0, // replace
	 0, 172, 0,
	 0,   0, 0,
	// clang-format on
};
static const BYTE L5RNDCOL7[] = {
	// clang-format off
	2, 2, // width, height

	207,  0, // search
	206, 15,

	210,   0, // replace
	 45, 160,
	// clang-format on
};
static const BYTE L5RNDCOL9[] = {
	// clang-format off
	2, 2, // width, height

	207,  0, // search
	206, 15,

	210,   0, // replace
	 45, 192,
	// clang-format on
};
#endif

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
static const BYTE L1ConvTbl[16] = { BASE_MEGATILE_L1, 13, 1, 13, 2, 13, 13, 13, 4, 13, 1, 13, 2, 13, 16, 13 };

/*
 * Place doors on the marked places.
 * New dungeon values: 25, 26
 */
/*static void DRLG_L1PlaceDoors()
{
	int i, j;
	BYTE df, c;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			df = drlgFlags[i][j];
			if ((df & ~DRLG_PROTECTED) == 0)
				continue;
			assert(!(df & DRLG_PROTECTED));
			//if (!(df & DRLG_PROTECTED)) {
				c = dungeon[i][j];

				if (df == DRLG_L1_HDOOR) {
					assert(c == 2);
					if (j == 1)
						df = 0;
					else //if (j != 1 && c == 2)
						c = 26;
					// commented out because this is not possible with the current implementation
					//if (j != 1 && c == 7)
					//	c = 31; -- slightly different 26
					//if (j != 1 && c == 14)
					//	c = 42; -- edge with arch on the other side
					//if (j != 1 && c == 4)
					//	c = 43; -- edge with wall on the other side
					//if (i != 1 && c == 1)
					//	c = 25; ?
					//if (i != 1 && c == 10)
					//	c = 40; ?
					//if (i != 1 && c == 6)
					//	c = 30; ?
				} else {
					assert(df == DRLG_L1_VDOOR);
					assert(c == 1);
					if (i == 1)
						df = 0;
					else // if (i != 1 && c == 1)
						c = 25;
					// commented out because this is not possible with the current implementation
					//if (i != 1 && c == 6)
					//	c = 30; -- slightly different 25
					//if (i != 1 && c == 10)
					//	c = 40; -- edge with arch on the other side
					//if (i != 1 && c == 4)
					//	c = 41; -- edge with wall on the other side
					//if (j != 1 && c == 2)
					//	c = 26; ?
					//if (j != 1 && c == 14)
					//	c = 42; ?
					//if (j != 1 && c == 7)
					//	c = 31; ?
				}
				// commented out because this is not possible with the current implementation
				//else if (df == (DRLG_L1_HDOOR | DRLG_L1_VDOOR)) {
				//	if (i != 1 && j != 1 && c == 4)
				//		c = 28; -- edge with double door
				//	if (i != 1 && c == 10)
				//		c = 40;
				//	if (j != 1 && c == 14)
				//		c = 42;
				//	if (j != 1 && c == 2)
				//		c = 26;
				//	if (i != 1 && c == 1)
				//		c = 25;
				//	if (j != 1 && c == 7)
				//		c = 31;
				//	if (i != 1 && c == 6)
				//		c = 30;
				//}
				dungeon[i][j] = c;
			//}
			// TODO: might want to convert DRLG_L1_VDOOR and DRLG_L1_HDOOR to DRLG_PROTECTED
			drlgFlags[i][j] = df;
		}
	}
}*/

#ifdef HELLFIRE
static void DRLG_L5Shadows()
{
	int i, j;

	for (j = DMAXY - 1; j > 0; j--) {
		for (i = DMAXX - 1; i > 0; i--) {
			BYTE bv = dungeon[i][j];
			bool horizArch = false;
			bool vertArch = false;
			bool pillar = false;
			/*switch (bv) {
			case 5:
			// case 116:
			// case 133:
			// case 150:
				pillar = true;
				break;
			case 28:
			case 7:
			// case 118:
			// case 135:
			// case 152:
			case 15:
			// case 126:
			// case 143:
			case 16:
			// case 127:
			// case 144:
			// case 161:
			// case 3:
			// case 114:
			// case 131:
			// case 148:
			// case 17:
			// case 128:
			// case 145:
			// case 162:
			// case 31:
			// case 33:
			case 85:
			case 86:
				pillar = true;
				break;
			case 8:
			// case 119:
			// case 136:
			// case 153:
			case 14:
			// case 125:
			// case 142:
			// case 159:
			case 37:
			// case 32:
			// case 39:
			// case 42:
			// case 29:
			// case 38:
			// case 44:
				pillar = true;
				break;
			case 9:
			// case 120:
			// case 137:
			// case 154:
				pillar = true;
				break;
			}*/
			pillar = (nTrnShadowTable[bv] & TIF_L5_PILLAR) != 0;
			horizArch = (nTrnShadowTable[bv] & TIF_L5_EAST_ARCH_GRATE) != 0;
			vertArch = (nTrnShadowTable[bv] & TIF_L5_WEST_ARCH_GRATE) != 0;
			if (horizArch) {
				if (dungeon[i][j - 1] == 13) {
					dungeon[i][j - 1] = 205;
				} else if (dungeon[i][j - 1] == 1) {
					dungeon[i][j - 1] = 109;
				} else if (dungeon[i][j - 1] == 6) {
					dungeon[i][j - 1] = 110;
				} else if (dungeon[i][j - 1] == 11) {
					dungeon[i][j - 1] = 111;
				} else if (dungeon[i][j - 1] == 35) {
					dungeon[i][j - 1] = 215;
				//} else if (dungeon[i][j - 1] == 203) {
				//	dungeon[i][j - 1] = 204;
				} else {
					// 25 -> not perfect, but ok and it would require a new door object as well
					// 195 -> not perfect, but ok and only on the dyn map entrance
					// TODO: what else?
				}
			}
			if (vertArch) {
				BYTE replaceA; bool okB;
				BYTE replaceB = dungeon[i - 1][j];
				switch (replaceB) {
				case 13:
				case 203: replaceA = 203; okB = false; break;
				case 207: replaceA = 203; okB = false; break;
				case 205: replaceA = 204; okB = false; break;
				// case 71:  replaceA = 80;  okB = true;  break;
				case 80:  replaceA = 80;  okB = true;  break;
				// case 85:  replaceA = 86;  okB = true;  break;
				case 86:  replaceA = 86;  okB = true;  break;
				// case 81:  replaceA = 82;  okB = true;  break;
				case 82:  replaceA = 82;  okB = true;  break;
				// case 87:  replaceA = 88;  okB = true;  break;
				case 88:  replaceA = 88;  okB = true;  break;
				// case 83:  replaceA = 84;  okB = true;  break;
				case 84:  replaceA = 84;  okB = true;  break;
				default:
					// 2, 4, 7, 12, 14, 26, 36, 37 -> ok
					// TODO: what else?
					continue;
				}

				dungeon[i - 1][j] = replaceA;
				if (okB) {
					continue;
				}
				// pillar = pillar && (dungeon[i][j - 1] == 13 /* || 203 207 204 81 ... 2 3 7 9 12 15 16 17 26 36 */);
				pillar = pillar && (nTrnShadowTable[dungeon[i][j - 1]] & TIF_L5_WEST_WALL) == 0;
				switch (dungeon[i - 1][j - 1]) {
				case 13: replaceB = pillar ? 207 : 203; break;
				case 2:  replaceB = pillar ? 71 : 80;   break;
				case 7:  replaceB = pillar ? 85 : 86;   break;
				case 12: replaceB = pillar ? 81 : 82;   break;
				case 26: replaceB = pillar ? 87 : 88;   break;
				case 36: replaceB = pillar ? 83 : 84;   break;
				default:
					dungeon[i - 1][j] = replaceB; // restore original value
					continue; // TODO: what else?
				}
				dungeon[i - 1][j - 1] = replaceB;
				continue;
			}
			if (pillar) {
				if (dungeon[i - 1][j] == 13) {
					BYTE replace = dungeon[i - 1][j - 1];
					// pillar = (dungeon[i][j - 1] == 13 /* || 203 207 204 81 ... 2 3 7 9 12 15 16 17 26 36 */);
					pillar = (nTrnShadowTable[dungeon[i][j - 1]] & TIF_L5_WEST_WALL) == 0;
					if (replace == 13) {
						replace = pillar ? 207 : 203;
					} else if (replace == 2) {
						replace = pillar ? 71 : 80;
					} else if (replace == 7) {
						replace = pillar ? 85 : 86;
					} else if (replace == 12) {
						replace = pillar ? 81 : 82;
					} else if (replace == 26) {
						replace = pillar ? 87 : 88;
					} else if (replace == 36) {
						replace = pillar ? 83 : 84;
					} else {
						continue; // TODO: what else?
					}
					dungeon[i - 1][j] = 206;
					dungeon[i - 1][j - 1] = replace;
				} else {
					// 2, 4, 7, 12, 14, 36, 37, 209, 212, 213, 214 -> ok
					// TODO: what else?
				}
			}
		}
	}
}
#endif

static void DRLG_L1Shadows()
{
	int i, j;

	for (j = DMAXY - 1; j > 0; j--) {
		for (i = DMAXX - 1; i > 0; i--) {
			BYTE bv = dungeon[i][j];
			bool horizArch = false;
			bool vertArch = false;
			bool pillar = false;
			bool largePillar = false;
			/*switch (bv) {
			case 5:
				pillar = true;
				break;
			case 15:
				largePillar = true;
				break;
			case 28:
			case 7:
			// case 15:
			case 16:
			// case 17:
			// case 31:
			case 154:
			case 155:
				pillar = true;
				break;
			case 8:
			case 14:
			case 37:
			// case 29:
			// case 32:
			// case 33:
			// case 38:
			// case 42:
			// case 44:
			// case 46:
			// case 85:
			case 160:
			case 161:
				pillar = true;
				break;
			case 9:
			// case 33:
				pillar = true;
				break;
			}*/
			largePillar = bv == 15;
			pillar = (nTrnShadowTable[bv] & TIF_L1_PILLAR) != 0;
			horizArch = (nTrnShadowTable[bv] & TIF_L1_EAST_ARCH_GRATE) != 0;
			vertArch = (nTrnShadowTable[bv] & TIF_L1_WEST_ARCH_GRATE) != 0;
			if (horizArch) {
				BYTE replaceA; bool okB;
				replaceA = dungeon[i][j - 1];
				bool pillarC = i == DMAXX - 1 || (nTrnShadowTable[dungeon[i + 1][j - 1]] & TIF_L1_WEST_WALL) == 0;
				// TODO: handle !pillarC
				switch (replaceA) {
				case 13:  replaceA = pillarC ? 140 : 141; okB = false; break;
				case 1:   replaceA = 146; okB = true;  break;
				case 6:   replaceA = 147; okB = true;  break;
				case 11:  replaceA = 145; okB = false; break;
				case 35:  replaceA = 157; okB = false; break;
				case 145: replaceA = 145; okB = false; break;
				case 146: replaceA = 146; okB = true;  break;
				case 147: replaceA = 147; okB = true;  break;
				case 157: replaceA = 157; okB = false; break;
				// case 131: replaceA = pillarC ? 131 : 132; okB = false; break;
				case 164: replaceA = pillarC ? 131 : 132; okB = false; break;
				// case 139: replaceA = 165; okB = false; break;
				default:
					okB = true;
					// 25 -> not perfect, but ok and it would require a new door piece as well
					// TODO: what else?
				}
				dungeon[i][j - 1] = replaceA;
				if (!okB) {
					if (dungeon[i - 1][j - 1] == 13) {
						dungeon[i - 1][j - 1] = 164;
					} else if (dungeon[i - 1][j - 1] == 1) {
						dungeon[i - 1][j - 1] = 146;
					} else if (dungeon[i - 1][j - 1] == 6) {
						dungeon[i - 1][j - 1] = 147;
					} else if (dungeon[i - 1][j - 1] == 11) {
						dungeon[i - 1][j - 1] = 145;
					} else if (dungeon[i - 1][j - 1] == 35) {
						dungeon[i - 1][j - 1] = 157;
					} else {
						// 25 -> not perfect, but ok and it would require a new door object as well
						// TODO: what else?
					}
				}
			}
			if (vertArch) {
				BYTE replaceA; bool okB;
				BYTE replaceB = dungeon[i - 1][j];
				switch (replaceB) {
				case 13:
				case 139: replaceA = 139; okB = false; break;
				case 143: replaceA = 139; okB = false; break;
				// case 140: replaceA = 127; okB = false; break; - should not happen TIF_L1_WEST_ARCH_GRATE implies TIF_L1_WEST_WALL
				case 148: replaceA = 148; okB = true;  break;
				case 149: replaceA = 149; okB = true;  break;
				// case 150: replaceA = 148; okB = true;  break;
				// case 151: replaceA = 149; okB = true;  break;
				// case 152: replaceA = 153; okB = true;  break;
				case 153: replaceA = 153; okB = true;  break;
				// case 154: replaceA = 155; okB = true;  break;
				case 155: replaceA = 155; okB = true;  break;
				case 156: replaceA = 156; okB = true;  break;
				case 159: replaceA = 139; okB = false; break;
				case 2:   replaceA = 148; okB = true;  break;
				case 4:   replaceA = 158; okB = true;  break;
				case 7:   replaceA = 155; okB = true;  break;
				case 12:  replaceA = 149; okB = true;  break;
				case 14:  replaceA = 160; okB = true;  break;
				case 26:  replaceA = 156; okB = true;  break;
				case 36:  replaceA = 152; okB = true;  break;
				case 37:  replaceA = 161; okB = true;  break;
				case 164: replaceA = 165; okB = false; break;
				case 132: replaceA = 126; okB = false; break;
				case 141: replaceA = 127; okB = false; break;
				default:
					// 25 -> not perfect, but ok and it would require a new door object as well
					continue;
				}

				dungeon[i - 1][j] = replaceA;
				if (okB) {
					continue;
				}
				// pillar = pillar && (dungeon[i][j - 1] == 13 /* || 203 207 204 81 ... 2 3 7 9 12 15 16 17 26 36 */);
				pillar = pillar && (nTrnShadowTable[dungeon[i][j - 1]] & TIF_L1_WEST_WALL) == 0;
				switch (dungeon[i - 1][j - 1]) {
				case 13: replaceB = pillar ? 143 : 159; break;
				case 2:  replaceB = pillar ? 150 : 148; break;
				case 7:  replaceB = pillar ? 154 : 155; break;
				case 12: replaceB = pillar ? 151 : 149; break;
				case 26: replaceB = pillar ? 156 : 156; break;
				case 36: replaceB = pillar ? 152 : 153; break;
				case 164: continue;
				default:
					dungeon[i - 1][j] = replaceB; // restore original value
					continue;
				}
				dungeon[i - 1][j - 1] = replaceB;
				continue;
			}
			if (pillar) {
				if (dungeon[i - 1][j] == 13) {
					BYTE replace = dungeon[i - 1][j - 1];
					// pillar = (dungeon[i][j - 1] == 13 /* || 203 207 204 81 ... 2 3 7 9 12 15 16 17 26 36 */);
					pillar = (nTrnShadowTable[dungeon[i][j - 1]] & TIF_L1_WEST_WALL) == 0;
					if (replace == 13) {
						replace = pillar ? 143 : 159;
					} else if (replace == 2) {
						replace = pillar ? 150 : 148;
					} else if (replace == 7) {
						replace = pillar ? 154 : 155;
					} else if (replace == 12) {
						replace = pillar ? 151 : 149;
					} else if (replace == 26) {
						replace = pillar ? 156 : 156;
					} else if (replace == 36) {
						replace = pillar ? 152 : 153;
					} else if (replace != 164) {
						continue;
					}
					dungeon[i - 1][j] = 142;
					dungeon[i - 1][j - 1] = replace;
				} else {
					// 2, 4, 7, 12, 14, 36, 37 -> ok
					// TODO: what else?
				}
				continue;
			}
			if (largePillar) {
				if (dungeon[i - 1][j] == 13 && dungeon[i - 1][j - 1] == 13) {
					dungeon[i - 1][j] = 142;
					dungeon[i - 1][j - 1] = 144;
				} else {
					// TODO: what else?
				}
			}
		}
	}
}

static void DRLG_L1Floor()
{
	DRLG_PlaceRndTile(13, 162, 33);
	DRLG_PlaceRndTile(13, 163, 50);
}

static void DRLG_LoadL1SP()
{
	// assert(pSetPieces[0]._spData == NULL);
	if (QuestStatus(Q_BANNER)) {
		pSetPieces[0]._sptype = SPT_BANNER;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
#if 0
		// patch set-piece - Banner1.DUN
		uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
		// fix the shadows
		lm[2 + 0 + 1 * 8] = SwapLE16(11);
		lm[2 + 3 + 1 * 8] = SwapLE16(11);
		// - protect the main structure
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (x >= 1 && y >= 3 && x <= 6 && y <= 5) {
					bool skip = true;
					if (x == 1 && y == 3) {
						skip = false;
					}
					if (x == 2 && y == 3) {
						skip = false;
					}
					if (x >= 4 && y == 4) {
						skip = false;
					}
					if (skip) {
						continue;
					}
				}
				if (x == 3 && y == 6) {
					continue;
				}
				lm[2 + 8 * 8 + x + y * 8] = SwapLE16(1);
			}
		}
		// ensure the changing tiles are reserved
		lm[2 + 8 * 8 + 3 + 6 * 8] = SwapLE16(3);
		for (int y = 3; y <= 5; y++) {
			for (int x = 1; x <= 6; x++) {
				if (x == 1 && y == 3) {
					continue;
				}
				if (x == 2 && y == 3) {
					continue;
				}
				if (x >= 4 && y == 4) {
					continue;
				}
				lm[2 + 8 * 8 + x + y * 8] = SwapLE16(3);
			}
		}
#endif
	} else if (QuestStatus(Q_SKELKING)) {
		pSetPieces[0]._sptype = SPT_SKELKING;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
#if 0
		// patch set-piece - SKngDO.DUN
		uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
		// use common tiles
		lm[2 + 5 + 4 * 7] =  SwapLE16(203 - 181);
		// use common tile and make the inner tile at the entrance non-walkable
		lm[2 + 5 + 2 * 7] =  SwapLE16(203 - 181);
		// let the game generate the shadow
		lm[2 + 0 + 5 * 7] = 0;
		lm[2 + 0 + 6 * 7] = 0;
		// protect the main structure
		for (int y = 1; y < 7; y++) {
			for (int x = 1; x < 7; x++) {
				lm[2 + 7 * 7 + x + y * 7] = SwapLE16(3);
			}
		}
#endif
	} else if (QuestStatus(Q_BUTCHER)) {
		pSetPieces[0]._sptype = SPT_BUTCHER;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
#ifdef HELLFIRE
	} else if (QuestStatus(Q_NAKRUL)) {
		pSetPieces[0]._sptype = SPT_NAKRUL;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
#endif
	}
}

static void L1ClearChamberFlags()
{
	int i;
	BYTE* pTmp;

	static_assert(sizeof(drlgFlags) == DMAXX * DMAXY, "Linear traverse of drlgFlags does not work in L1ClearFlags.");
	pTmp = &drlgFlags[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++)
		*pTmp &= ~DRLG_L1_CHAMBER;
}

static void L1DrawRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	drlg.L1RoomList[nRoomCnt].lrx = x;
	drlg.L1RoomList[nRoomCnt].lry = y;
	drlg.L1RoomList[nRoomCnt].lrw = width;
	drlg.L1RoomList[nRoomCnt].lrh = height;
	nRoomCnt++;

	x2 = x + width;
	y2 = y + height;
	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			dungeon[i][j] = 1;
		}
	}
}

static bool L1CheckRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	if (x < 0 || y < 0)
		return false;

	x2 = x + width;
	y2 = y + height;
	if (x2 > DMAXX || y2 > DMAXY)
		return false;

	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			if (dungeon[i][j] != 0)
				return false;
		}
	}

	return true;
}

/* L1CheckVHall and L1CheckHHall
 *  make sure there is at least one empty block between the rooms
 */
static bool L1CheckVHall(int x, int top, int h)
{
	int j = top, bottom;

	//assert((unsigned)x < DMAXX);
	if (j < 0)
		return false;
	bottom = j + h;
	if (bottom >= DMAXY)
		return false;
	while (j < bottom && dungeon[x][j] == 0)
		j++;
	while (j < bottom && dungeon[x][j] == 1)
		j++;
	while (j < bottom && dungeon[x][j] == 0)
		j++;
	return j == bottom;
}

static bool L1CheckHHall(int y, int left, int w)
{
	int i = left, right;

	//assert((unsigned)y < DMAXY);
	if (i < 0)
		return false;
	right = i + w;
	if (right >= DMAXX)
		return false;
	while (i < right && dungeon[i][y] == 0)
		i++;
	while (i < right && dungeon[i][y] == 1)
		i++;
	while (i < right && dungeon[i][y] == 0)
		i++;
	return i == right;
}

static void L1RoomGen(int x, int y, int w, int h, bool dir)
{
	int dirProb, i, width, height, rx, ry, rxy2;
	static_assert((DMAXX * DMAXY - (CHAMBER_SIZE + 2) * (CHAMBER_SIZE + 2) + 4) / (2 * 2) <= lengthof(drlg.L1RoomList), "L1RoomGen skips limit checks assuming enough L1RoomList entries.");

	dirProb = random_(0, 4);

	if (dir == (dirProb == 0)) {
		// try to place a room to the left
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2u + y - height / 2u;
			rx = x - width;
			if (L1CheckVHall(x, ry - 1, height + 2)
			 && L1CheckRoom(rx - 1, ry - 1, width + 1, height + 2)) { // BUGFIX: swap args 3 and 4 ("height+2" and "width+1") (fixed)
				// - add room to the left
				L1DrawRoom(rx, ry, width, height);
				break;
			}
		}
		if (i != 0) {
			// room added to the left -> force similar room on the right side
			i = 1;
		} else {
			// room was not added to the left -> try to more options on the right
			rx = -1;
			i = 20;
		}
		// try to place a room to the right
		rxy2 = x + w;
		while (true) {
			if (L1CheckVHall(rxy2 - 1, ry - 1, height + 2)
			 && L1CheckRoom(rxy2, ry - 1, width + 1, height + 2)) {
				// - add room to the right
				L1DrawRoom(rxy2, ry, width, height);
				break;
			}
			if (--i == 0)
				break;
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2u + y - height / 2u;
		}
		// proceed with the placed a room on the left
		if (rx >= 0)
			L1RoomGen(rx, ry, width, height, true);
		// proceed with the placed a room on the right
		if (i != 0)
			L1RoomGen(rxy2, ry, width, height, true);
	} else {
		// try to place a room to the top
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2u + x - width / 2u;
			ry = y - height;
			if (L1CheckHHall(y, rx - 1, width + 2)
			 && L1CheckRoom(rx - 1, ry - 1, width + 2, height + 1)) {
				// - add room to the top
				L1DrawRoom(rx, ry, width, height);
				break;
			}
		}
		if (i != 0) {
			// room added to the top -> force similar room on the bottom side
			i = 1;
		} else {
			// room was not added to the top -> try to more options on the bottom
			ry = -1;
			i = 20;
		}
		// try to place a room to the bottom
		rxy2 = y + h;
		while (true) {
			if (L1CheckHHall(rxy2 - 1, rx - 1, width + 2)
			 && L1CheckRoom(rx - 1, rxy2, width + 2, height + 1)) {
				// - add room to the bottom
				L1DrawRoom(rx, rxy2, width, height);
				break;
			}
			if (--i == 0)
				break;
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2u + x - width / 2u;
		}
		// proceed with the placed a room on the top
		if (ry >= 0)
			L1RoomGen(rx, ry, width, height, false);
		// proceed with the placed a room on the bottom
		if (i != 0)
			L1RoomGen(rx, rxy2, width, height, false);
	}
}

/*
 * Create dungeon blueprint.
 * New dungeon values: 1
 */
static void DRLG_L1CreateDungeon()
{
	int is, ie, i;

	nRoomCnt = 0;
	ChambersVertical = random_(0, 2);
	// select the base chambers
	i = random_(0, 8);
	// make sure at least one chamber is selected
	//  (prefer complete selection over a single chamber to increase the chance of success)
	if (i == 0)
		i = 7;
	ChambersFirst = (i & 1) ? TRUE : FALSE;
	ChambersMiddle = (i & 2) ? TRUE : FALSE;
	ChambersLast = (i & 4) ? TRUE : FALSE;
	// prevent standalone first/last chambers
	if (!(ChambersFirst & ChambersLast))
		ChambersMiddle = TRUE;
	if (ChambersVertical) {
		// place the main rooms vertically
		is = CHAMBER_SIZE + 1;
		ie = 29;

		// draw the selected rooms
		if (ChambersFirst)
			L1DrawRoom(15, 1, CHAMBER_SIZE, CHAMBER_SIZE);
		else
			is = 1 + CHAMBER_SIZE + 4 + CHAMBER_SIZE;

		if (ChambersMiddle)
			L1DrawRoom(15, 15, CHAMBER_SIZE, CHAMBER_SIZE);
		if (ChambersLast)
			L1DrawRoom(15, 29, CHAMBER_SIZE, CHAMBER_SIZE);
		else
			ie = 15;
		// draw a hallway between the rooms
		for (i = is; i < ie; i++) {
			dungeon[17][i] = 1;
			dungeon[18][i] = 1;
			dungeon[19][i] = 1;
			dungeon[20][i] = 1;
			dungeon[21][i] = 1;
			dungeon[22][i] = 1;
		}
		// spread additional rooms starting from the main rooms
		if (ChambersFirst)
			L1RoomGen(15, 1, CHAMBER_SIZE, CHAMBER_SIZE, false);
		if (ChambersMiddle)
			L1RoomGen(15, 15, CHAMBER_SIZE, CHAMBER_SIZE, false);
		if (ChambersLast)
			L1RoomGen(15, 29, CHAMBER_SIZE, CHAMBER_SIZE, false);

	} else {
		// place the main rooms horizontally
		is = CHAMBER_SIZE + 1;
		ie = 29;

		// draw the selected rooms
		if (ChambersFirst)
			L1DrawRoom(1, 15, CHAMBER_SIZE, CHAMBER_SIZE);
		else
			is = 1 + CHAMBER_SIZE + 4 + CHAMBER_SIZE;

		if (ChambersMiddle)
			L1DrawRoom(1 + CHAMBER_SIZE + 4, 15, CHAMBER_SIZE, CHAMBER_SIZE);
		if (ChambersLast)
			L1DrawRoom(1 + CHAMBER_SIZE + 4 + CHAMBER_SIZE + 4, 15, CHAMBER_SIZE, CHAMBER_SIZE);
		else
			ie = 15;
		// draw a hallway between the rooms
		for (i = is; i < ie; i++) {
			dungeon[i][17] = 1;
			dungeon[i][18] = 1;
			dungeon[i][19] = 1;
			dungeon[i][20] = 1;
			dungeon[i][21] = 1;
			dungeon[i][22] = 1;
		}
		// spread additional rooms starting from the main rooms
		if (ChambersFirst)
			L1RoomGen(1, 15, CHAMBER_SIZE, CHAMBER_SIZE, true);
		if (ChambersMiddle)
			L1RoomGen(15, 15, CHAMBER_SIZE, CHAMBER_SIZE, true);
		if (ChambersLast)
			L1RoomGen(29, 15, CHAMBER_SIZE, CHAMBER_SIZE, true);
	}
}

static int DRLG_L1GetArea()
{
	int i, rv;
#if 1
	BYTE* pTmp;

	rv = 0;
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L1GetArea.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		// assert(*pTmp <= 1);
		rv += *pTmp;
	}
#else
	rv = 0;
	for (i = 0; i < nRoomCnt; i++) {
		rv += drlg.L1RoomList[i].lrw * drlg.L1RoomList[i].lrh;
	}
	if (ChambersFirst + ChambersMiddle + ChambersLast == 3) {
		rv += 6 * 4 * 2;
	} else if (ChambersFirst + ChambersMiddle + ChambersLast == 2) {
		rv += 6 * 4;
		if (!ChambersMiddle)
			rv += 6 * 4 + CHAMBER_SIZE * 6;
	}
#endif
	return rv;
}

/*
 * Transform dungeon by replacing values using 2x2 block patterns defined in L1ConvTbl
 * New dungeon values: 1 2 4 13 16 22
 */
static void DRLG_L1MakeMegas()
{
	int i, j;
	BYTE v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			// assert(dungeon[i][j] <= 1 && dungeon[i + 1][j] <= 1 && dungeon[i][j + 1] <= 1 && dungeon[i + 1][j + 1] <= 1);
			v = dungeon[i][j]
			 | (dungeon[i + 1][j] << 1)
			 | (dungeon[i][j + 1] << 2)
			 | (dungeon[i + 1][j + 1] << 3);
			assert(v != 6 && v != 9);
			dungeon[i][j] = L1ConvTbl[v];
		}
	}
	for (j = 0; j < DMAXY; j++)
		dungeon[DMAXX - 1][j] = BASE_MEGATILE_L1;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = BASE_MEGATILE_L1;
}

static void L1HorizWall(int i, int j, int dx)
{
	int xx;
	BYTE pn, dt;
	// convert the first tile / select the internal replacement
	pn = dungeon[i][j] == 6 ? 4 : 2;

	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 2;
		break;
	case 2:
		dt = 12;
		pn = pn == 4 ? 10 : 12;
		break;
	case 3:
		dt = 36;
		pn = pn == 4 ? 27 : 36;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	dungeon[i][j] = pn;
	// convert the internal tiles
	for (xx = 1; xx < dx; xx++) {
		dungeon[i + xx][j] = dt;
	}
	// add 'door'
	if (dt != 12) {
		xx = RandRange(1, dx - 1);
		dt = random_(0, 6) == 0 ? 12 : 26;
		dungeon[i + xx][j] = dt;
		// assert(drlgFlags[i + xx][j] == 0);
		// drlgFlags[i + xx][j] = dt == 26 ? DRLG_L1_HDOOR : 0;
	}
}

static constexpr uint32_t GOOD_HWALL_ENDS =
	  (1 << 3)
	| (1 << 4)
	| (1 << 5)
	| (1 << 6)
	//| (1 << 7) -- does not fit the internal pn (2/12/36)
	//| (1 << 16) -- does not fit the internal pn (2/12/36)
	| (1 << 17)
	| (1 << 18)
	//| (1 << 19) -- should not happen
	//| (1 << 20) -- should not happen
	| (1 << 21)
	| (1 << 23);
	//| (1 << 24); -- should not happen
static bool L1AddHWall(int x, int y)
{
	int i;
	BYTE bv;

	i = x;
	while (true) {
		i++;
		bv = dungeon[i][y];
		if (bv != 13)
			break;
		if (dungeon[i][y - 1] != 13)
			break;
		if (dungeon[i][y + 1] != 13)
			break;
		if (drlgFlags[i][y] != 0)
			break;
	}

	i -= x;
	if (i != 1
	// && ((bv >= 3 && bv <= 7) || (bv >= 16 && bv <= 24 && bv != 22)) {
	 && (bv < 25 && (GOOD_HWALL_ENDS & (1 << bv)))) {
		L1HorizWall(x, y, i);
		return true;
	}
	return false;
}

static void L1VertWall(int i, int j, int dy)
{
	int yy;
	BYTE pn, dt;
	// convert the first tile / select the internal replacement
	pn = dungeon[i][j] == 7 ? 4 : 1;
	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 1;
		break;
	case 2:
		dt = 11;
		pn = pn == 4 ? 14 : 11;
		break;
	case 3:
		dt = 35;
		pn = pn == 4 ? 37 : 35;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	dungeon[i][j] = pn;
	// convert the internal tiles
	for (yy = 1; yy < dy; yy++) {
		dungeon[i][j + yy] = dt;
	}
	// add 'door'
	if (dt != 11) {
		yy = RandRange(1, dy - 1);
		dt = random_(0, 6) == 0 ? 11 : 25;
		dungeon[i][j + yy] = dt;
		// assert(drlgFlags[i][j + yy] == 0);
		// drlgFlags[i][j + yy] = dt == 25 ? DRLG_L1_VDOOR : 0;
	}
}

static constexpr uint32_t GOOD_VWALL_ENDS =
	  (1 << 3)
	| (1 << 4)
	| (1 << 5)
	| (1 << 6)
	| (1 << 7)
	| (1 << 16)
	//| (1 << 17) -- does not fit the internal pn (1/11/35)
	//| (1 << 18) -- should not happen
	| (1 << 19)
	//| (1 << 20) -- should not happen
	| (1 << 21)
	//| (1 << 23) -- should not happen
	| (1 << 24);
static void L1AddVWall(int x, int y)
{
	int j;
	BYTE bv;

	j = y;
	while (true) {
		j++;
		bv = dungeon[x][j];
		if (bv != 13)
			break;
		if (dungeon[x - 1][j] != 13)
			break;
		if (dungeon[x + 1][j] != 13)
			break;
		if (drlgFlags[x][j] != 0)
			break;
	}

	j -= y;
	if (j != 1
	// && ((bv >= 3 && bv <= 7) || (bv >= 16 && bv <= 24 && bv != 22))) {
	 && (bv < 25 && (GOOD_VWALL_ENDS & (1 << bv)))) {
		L1VertWall(x, y, j);
		return; // true;
	}
	//return false;
}

/*
 * Draw walls between pillars (out of the chambers)
 * New dungeon values: 1 2 4 10 11 12 14 25 26 27 35 36 37
 */
static void L1AddWall()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (drlgFlags[i][j] != 0)
				continue;
			switch (dungeon[i][j]) {
			case 3:
				if (L1AddHWall(i, j))
					break;
				/* fall-through */
			case 7:
			case 1:
				L1AddVWall(i, j);
				break;
			case 2:
			case 6:
				L1AddHWall(i, j);
				break;
			}
		}
	}
}

static void DRLG_L1GChamber(int sx, int sy)
{
	int i, j;
	// set flags for the chamber
	for (i = 0; i < CHAMBER_SIZE; i++) {
		for (j = 0; j < CHAMBER_SIZE; j++) {
			assert(dungeon[i + sx][j + sy] == 13);
			drlgFlags[i + sx][j + sy] |= DRLG_L1_CHAMBER;
		}
	}
	// add the four main pillars of the chamber
	dungeon[sx + 3][sy + 3] = 15;
	dungeon[sx + 6][sy + 3] = 15;
	dungeon[sx + 3][sy + 6] = 15;
	dungeon[sx + 6][sy + 6] = 15;
}

static void DRLG_L1GHallHoriz(int x1, int y1, int x2)
{
	int i;

	i = x1;
	assert(dungeon[i][y1 - 2] == 7);
	dungeon[i][y1 - 2] = 14;
	dungeon[i][y1 - 1] = 11;
	dungeon[i][y1 + 0] = 9;
	dungeon[i][y1 + 3] = 5;
	dungeon[i][y1 + 4] = 11;
	//assert(dungeon[i][y1 + 5] == 4 || dungeon[i][y1 + 5] == 21 || dungeon[i][y1 + 5] == 7);
	assert(dungeon[i][y1 + 5] == 4 || dungeon[i][y1 + 5] == 21); // 7 is not generated with an empty line between walls

	for (i++; i < x2; i++) {
		dungeon[i][y1] = 12;
		dungeon[i][y1 + 3] = 12;
	}

	//assert(dungeon[i][y1 - 2] == 3 || dungeon[i][y1 - 2] == 17);
	assert(dungeon[i][y1 - 2] == 3); // 17 is not generated with an empty line between walls

	//if (dungeon[i][y1 - 2] == 17)
	//	dungeon[i][y1 - 2] = 8;
	//else //if (dungeon[i][y1 - 2] == 3)
		dungeon[i][y1 - 2] = 11;
	dungeon[i][y1 - 1] = 11;
	dungeon[i][y1 + 0] = 28;
	dungeon[i][y1 + 3] = 8;
	dungeon[i][y1 + 4] = 11;
	//assert(dungeon[i][y1 + 5] == 6 || dungeon[i][y1 + 5] == 17);
	assert(dungeon[i][y1 + 5] == 6); // 17 is not generated with an empty line between walls
	//if (dungeon[i][y1 + 5] == 17)
	//	dungeon[i][y1 + 5] = 3;
	//else if (dungeon[i][y1 + 5] == 6)
		dungeon[i][y1 + 5] = 1; // use tile 1 instead of 6 if possible for better shadows/decorations
}

static void DRLG_L1GHallVert(int x1, int y1, int y2)
{
	int i;

	i = y1;
	assert(dungeon[x1 - 2][i] == 6);
	dungeon[x1 - 2][i] = 10;
	dungeon[x1 - 1][i] = 12;
	dungeon[x1 + 0][i] = 8;
	dungeon[x1 + 3][i] = 5;
	dungeon[x1 + 4][i] = 12;
	//assert(dungeon[x1 + 5][i] == 4 || dungeon[x1 + 5][i] == 6 || dungeon[x1 + 5][i] == 21);
	assert(dungeon[x1 + 5][i] == 4 || dungeon[x1 + 5][i] == 21); // 6 is not generated with an empty line between walls

	for (i++; i < y2; i++) {
		dungeon[x1][i] = 11;
		dungeon[x1 + 3][i] = 11;
	}

	//assert(dungeon[x1 - 2][i] == 3 || dungeon[x1 - 2][i] == 16);
	assert(dungeon[x1 - 2][i] == 3); // 16 is not generated with an empty line between walls
	//if (dungeon[x1 - 2][i] == 16)
	//	dungeon[x1 - 2][i] = 9;
	//else //if (dungeon[x1 - 2][i] == 3)
		dungeon[x1 - 2][i] = 12;
	dungeon[x1 - 1][i] = 12;
	dungeon[x1 + 0][i] = 28;
	dungeon[x1 + 3][i] = 9;
	dungeon[x1 + 4][i] = 12;
	//assert(dungeon[x1 + 5][i] == 7 || dungeon[x1 + 5][i] == 16);
	assert(dungeon[x1 + 5][i] == 7); // 16 is not generated with an empty line between walls
	//if (dungeon[x1 + 5][i] == 16)
	//	dungeon[x1 + 5][i] = 3;
	//else //if (dungeon[x1 + 5][i] == 7)
		dungeon[x1 + 5][i] = 2;
}

static void DRLG_L1SetRoom(int rx1, int ry1)
{
	pSetPieces[0]._spx = rx1;
	pSetPieces[0]._spy = ry1;
	DRLG_LoadSP(0, DEFAULT_MEGATILE_L1);
}

/*
 * Add pillars to the chambers.
 * New dungeon values: [2] (1) 5 8 9 10 11 12 14 15 28
 */
static void L1FillChambers()
{
	int c;

	if (ChambersVertical) {
		// draw the vertical chambers
		if (ChambersFirst)
			DRLG_L1GChamber(15, 1);
		if (ChambersMiddle)
			DRLG_L1GChamber(15, 15);
		if (ChambersLast)
			DRLG_L1GChamber(15, 29);
		// add pillars to the hall between the vertical chambers
		if (ChambersMiddle) {
			if (ChambersFirst)
				DRLG_L1GHallVert(18, 11, 14);
			if (ChambersLast)
				DRLG_L1GHallVert(18, 25, 28);
		} else {
			//assert(ChambersFirst && ChambersLast);
			DRLG_L1GHallVert(18, 11, 28);
		}
	} else {
		// draw the horizontal chambers
		if (ChambersFirst)
			DRLG_L1GChamber(1, 15);
		if (ChambersMiddle)
			DRLG_L1GChamber(15, 15);
		if (ChambersLast)
			DRLG_L1GChamber(29, 15);
		// add pillars to the hall between the horizontal chambers
		if (ChambersMiddle) {
			if (ChambersFirst)
				DRLG_L1GHallHoriz(11, 18, 14);
			if (ChambersLast)
				DRLG_L1GHallHoriz(25, 18, 28);
		} else {
			//assert(ChambersFirst && ChambersLast);
			DRLG_L1GHallHoriz(11, 18, 28);
		}
	}

	if (pSetPieces[0]._spData != NULL) { // pSetPieces[0]._sptype != SPT_NONE
		c = ChambersFirst + ChambersMiddle + ChambersLast;
		c = random_low(0, c);
		if (ChambersFirst) {
			if (c == 0)
				c = 2; // select the first chamber for the set piece
			else
				c--;   // 'deselect' the first chamber
		}
		if (ChambersMiddle) {
			if (c == 0)
				c = 16; // select the middle chamber for the set piece
			else if (c != 2)
				c--;    // 'deselect' the middle chamber (only if the first was not selected)
		}
		if (ChambersLast) {
			if (c == 0)
				c = 30; // select the last chamber for the set piece
		}
		if (ChambersVertical) {
			DRLG_L1SetRoom(16, c);
		} else {
			DRLG_L1SetRoom(c, 16);
		}
	}
}

/* possible configurations:
  (cases in brackets are not possible with an empty line between walls)
		[ 1, 13, 4 ]
	[ 2, 13, 22]  	1		[ 13 ]
		[ 1, 2, 16, 13, 4, 22 ]

		[ 1, 13, 22 ]
	[ 2, 13, 4  ]	2		[ 1, 2, 16, (13), 4, 22 ]
		[ 13 ]

		[ 1, 13, 4 ]
	[ 2, 13, 4 ]	16		[ 13 ]
		[ 13 ]

		[ 1, 13, 22 ]
	[ 2, 13, 22 ]	4		[ 2 16 (13) ]
		[ 1, 16, 13 ]

		[ 1, 13, 22 ]
	[ 2, 13, 22 ]	22		 [ 1, 4, 22 ]
		[ 2, 4, 22 ]

		[ 1, 2, 16, 13, 4 ]
	[ 1, (2), 16, 13, (4) ] 13 	[ 1, 2, 16, 13, 4, 22 ]
		[ 1, 2, 16, 13, 4, 22 ]
*/

/*
 * Draw wall around the tiles selected by DRLG_L1CreateDungeon.
 * Assumes the border of dungeon was empty.
 * New dungeon values: 3 6 7 16 17 18 19 21 23 24
 */
static void L1TileFix()
{
	int i, j;

	// convert tiles based on the tile above and left from them.
	// Connect, add or cut walls.
	for (i = DMAXX - 1; i > 0; i--) {
		for (j = DMAXY - 1; j > 0; j--) {
			switch (dungeon[i][j]) {
			case 1:
				//          [ 13(6) ]
				// [ 2(6) ]     1
				// 2(6) -- the possible values on the top are 1, 4 and 13.
				//         in case of 1 and 4 the conversion is not necessary.
				//         in case of 13, the same conversion is triggered.
				if (/*dungeon[i - 1][j] == 2 ||*/ dungeon[i][j - 1] == 13)
					dungeon[i][j] = 6; // cut/connect
				break;
			case 2:
				//          [ 1(7) ]
				// [ 13(7) ]    2
				// 1(7) -- the possible values on the left are 2, 4 and 13.
				//         in case of 2 and 4 the conversion is not necessary.
				//         in case of 13, the same conversion is triggered.
				if (dungeon[i - 1][j] == 13 /*|| dungeon[i][j - 1] == 1*/) {
					dungeon[i][j] = 7; // cut/connect
				}
				break;
			case 4:
				break;
			case 13:
				//          [ 1(16) ]
				// [ 2(17) ]    13
				// Impossible case ([2 13]) if there is an empty tile between walls.
				assert(dungeon[i - 1][j] != 2);
				if (dungeon[i][j - 1] == 1) {
					//if (dungeon[i - 1][j] == 2) {
					//	dungeon[i][j] = 3; // connect
					//} else {
						dungeon[i][j] = 16; // connect
					//}
				} /*else {
					if (dungeon[i - 1][j] == 2) {
						dungeon[i][j] = 17; // connect
					}
				}*/
				break;
			case 16:
				//              [ 13(17) ]
				// [ 2(3), 4(3) ]   16
				if (dungeon[i - 1][j] == 2 || dungeon[i - 1][j] == 4) {
					if (dungeon[i][j - 1] == 13) {
						dungeon[i][j] = 17; // cut + connect
					} else {
						dungeon[i][j] = 3; // connect
					}
				} else {
					assert(dungeon[i][j - 1] != 13);
					//if (dungeon[i][j - 1] == 13) {
					//	dungeon[i][j] = 13; // cut + cut
					//}
				}
				break;
			case 22:
				//          [ 1(24!), 13(19!) ]
				// [ 2(23!), 13(18!) ]  22
				if (dungeon[i - 1][j] == 2) {
					if (dungeon[i][j - 1] == 1 || dungeon[i][j - 1] == 13) {
						dungeon[i][j] = 21; // new wall (SW, SE)
					} else {
						assert(dungeon[i][j - 1] == 22);
						dungeon[i][j] = 23; // new wall (SW)
					}
				} else if (dungeon[i - 1][j] == 13) {
					if (dungeon[i][j - 1] == 1 || dungeon[i][j - 1] == 13) {
						dungeon[i][j] = 21; // new wall (SW, SE)
					} else {
						assert(dungeon[i][j - 1] == 22);
						dungeon[i][j] = 18; // new wall (SW, NE)
					}
				} else {
					assert(dungeon[i - 1][j] == 22);
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 24; // new wall (SE)
					} else if (dungeon[i][j - 1] == 13) {
						dungeon[i][j] = 19; // new wall (NW, SE)
					} else {
						assert(dungeon[i][j - 1] == 22);
					}
				}
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
		}
	}

	// apply the same logic to the first row/column
	for (i = DMAXX - 1; i > 0; i--) {
		/* Impossible case ([2 13]) if there is an empty tile between walls.
		if (dungeon[i][0] == 13) {
			// [ 2(17) ]   13
			if (dungeon[i - 1][0] == 2) {
				dungeon[i][0] = 17; // connect
			}
		} else*/ if (dungeon[i][0] == 22) {
			// [ 2(23) ]   22
			if (dungeon[i - 1][0] == 2) {
				dungeon[i][0] = 23; // new wall (SW)
			}
		}
	}
	for (j = DMAXY - 1; j > 0; j--) {
		if (dungeon[0][j] == 22) {
			// [ 1(24) ]
			//    22
			if (dungeon[0][j - 1] == 1) {
				dungeon[0][j] = 24; // new wall (SE)
			} else {
				assert(dungeon[0][j - 1] == 22);
			}
		}
	}

	// connect the new walls
	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			switch (dungeon[i][j]) {
			case 19: // new wall (NW, SE)
			case 21: // new wall (SW, SE)
			case 24: // new wall (SE)
				assert(i < DMAXX - 1);
				if (dungeon[i + 1][j] == 1
				// use tile 1 instead of 6 if possible for better shadows/decorations
				 && (dungeon[i + 1][j - 1] != 1 && dungeon[i + 1][j - 1] != 4
					 && dungeon[i + 1][j - 1] != 6)) {
					dungeon[i + 1][j] = 6; // connect
				}
				break;
			}
		}
	}
#if DEBUG_MODE
	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			switch (dungeon[i][j]) {
			case 18: // new wall (SW, NE)
				assert(i > 0);
				// check SW (SW-18)
				assert(j < DMAXY - 1);
				if (dungeon[i][j + 1] == 22) {
					assert(dungeon[i - 1][j + 1] == 19 || dungeon[i - 1][j + 1] == 21);
				} else if (dungeon[i][j + 1] == 2) {
					assert(dungeon[i - 1][j + 1] == 2
					 || dungeon[i - 1][j + 1] == 4
					 || dungeon[i - 1][j + 1] == 7);
				} else {
					assert(dungeon[i][j + 1] == 4
					 || dungeon[i][j + 1] == 7 // should not happen with an empty line between walls
					 || dungeon[i][j + 1] == 18
					 || dungeon[i][j + 1] == 23);
				}
				// check NE
				assert(j > 0);
				assert(dungeon[i][j - 1] == 18
					 || dungeon[i][j - 1] == 21
					 || dungeon[i][j - 1] == 23);
				break;
			case 19: // new wall (NW, SE)
				// check SE (SE-19)
				assert(i < DMAXX - 1);
				assert(j > 0);
				if (dungeon[i + 1][j] == 22) {
					assert(dungeon[i + 1][j - 1] == 18 || dungeon[i + 1][j - 1] == 21);
				} else {
					assert(dungeon[i + 1][j] == 4 || dungeon[i + 1][j] == 6
					 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 1)
					 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 4)
					 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 6)
					 || dungeon[i + 1][j] == 24
					 || dungeon[i + 1][j] == 19);
				}
				// check NW
				assert(i > 0);
				assert(dungeon[i - 1][j] == 19 || dungeon[i - 1][j] == 21 || dungeon[i - 1][j] == 24);
				break;
			case 21: // new wall (SW, SE)
				// check SW (SW-18)'
				assert(j < DMAXY - 1);
				assert(i > 0);
				if (dungeon[i][j + 1] == 22) {
					assert(dungeon[i - 1][j + 1] == 19 || dungeon[i - 1][j + 1] == 21);
				} else if (dungeon[i][j + 1] == 2) {
					assert(dungeon[i - 1][j + 1] == 2
					 || dungeon[i - 1][j + 1] == 4
					 || dungeon[i - 1][j + 1] == 7);
				} else {
					assert(dungeon[i][j + 1] == 4
					 || dungeon[i][j + 1] == 7
					 || dungeon[i][j + 1] == 18
					 || dungeon[i][j + 1] == 21
					 || dungeon[i][j + 1] == 23);
				}
				// check SE (SE-19)
				if (dungeon[i + 1][j] == 22) {
					assert(dungeon[i + 1][j - 1] == 18 || dungeon[i + 1][j - 1] == 21);
				} else {
					assert(dungeon[i + 1][j] == 4 || dungeon[i + 1][j] == 6
					 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 1)
					 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 4)
					 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 6)
					 || dungeon[i + 1][j] == 19
					 || dungeon[i + 1][j] == 24);
				}
				break;
			case 23: // new wall (SW)
				assert(j < DMAXY - 1);
				assert(dungeon[i][j + 1] == 4
				 || dungeon[i][j + 1] == 7 // should not happen with an empty line between walls
				 || dungeon[i][j + 1] == 18);
				break;
			case 24: // new wall (SE)
				assert(i < DMAXX - 1);
				assert(dungeon[i + 1][j] == 4 || dungeon[i + 1][j] == 6
				 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 1)
				 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 4)
				 || (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 6)
				 || dungeon[i + 1][j] == 19);
				break;
			}
		}
	}
#endif
}

static void DRLG_L1PlaceThemeRooms()
{
	RECT_AREA32 thops[32];
	int i, numops = 0;
	for (i = ChambersFirst + ChambersMiddle + ChambersLast; i < nRoomCnt; i++) {
		int roomLeft = drlg.L1RoomList[i].lrx;
		int roomRight = roomLeft + drlg.L1RoomList[i].lrw - 1;
		int roomTop = drlg.L1RoomList[i].lry;
		int roomBottom = roomTop + drlg.L1RoomList[i].lrh - 1;
		// select floor on the top-left corner
		if (dungeon[roomLeft][roomTop] != DEFAULT_MEGATILE_L1) {
			if (dungeon[roomLeft + 1][roomTop] == DEFAULT_MEGATILE_L1) {
				roomLeft++;
			} else if (dungeon[roomLeft][roomTop + 1] == DEFAULT_MEGATILE_L1) {
				roomTop++;
			} else if (dungeon[roomLeft + 1][roomTop + 1] == DEFAULT_MEGATILE_L1) {
				roomLeft++;
				roomTop++;
			} else {
				continue;
			}
		}
		// select floor on the bottom-right corner
		if (dungeon[roomRight][roomBottom] != DEFAULT_MEGATILE_L1) {
			if (dungeon[roomRight - 1][roomBottom] == DEFAULT_MEGATILE_L1) {
				roomRight--;
			} else if (dungeon[roomLeft][roomBottom - 1] == DEFAULT_MEGATILE_L1) {
				roomBottom--;
			} else if (dungeon[roomRight - 1][roomBottom - 1] == DEFAULT_MEGATILE_L1) {
				roomRight--;
				roomBottom--;
			} else {
				continue;
			}
		}
		// check inner tiles
		bool fit = roomLeft <= roomRight && roomTop <= roomBottom;
		for (int x = roomLeft; x <= roomRight; x++) {
			for (int y = roomTop; y <= roomBottom; y++) {
				if (dungeon[x][y] != DEFAULT_MEGATILE_L1 || (drlgFlags[x][y] & DRLG_PROTECTED)) {
					fit = false;
				}
			}
		}
		// check border tiles
		for (int x = roomLeft - 1; x <= roomRight + 1; x++) {
			if (dungeon[x][roomTop - 1] == DEFAULT_MEGATILE_L1 || dungeon[x][roomBottom + 1] == DEFAULT_MEGATILE_L1) {
				fit = false;
			}
		}
		for (int y = roomTop - 1; y <= roomBottom + 1; y++) {
			if (dungeon[roomLeft - 1][y] == DEFAULT_MEGATILE_L1 || dungeon[roomRight + 1][y] == DEFAULT_MEGATILE_L1) {
				fit = false;
			}
		}
		if (!fit)
			continue; // room is too small or incomplete
		// create the room
		int w = (roomRight + 1) - (roomLeft - 1) + 1;
		int h = (roomBottom + 1) - (roomTop - 1) + 1;
		if (w > 10 - 2 || h > 10 - 2)
			continue; // room is too large
		// register the room
		thops[numops].x1 = roomLeft - 1;
		thops[numops].y1 = roomTop - 1;
		thops[numops].x2 = roomLeft - 1 + w - 1;
		thops[numops].y2 = roomTop - 1 + h - 1;
		numops++;
		if (numops == lengthof(thops))
			break; // should not happen (too often), otherwise the theme-placement is biased
	}
	// filter the rooms
	while (numops > lengthof(themes)) {
		i = random_low(0, numops);
		--numops;
		thops[i] = thops[numops];
	}
	// add the rooms
	for (i = 0; i < numops; i++) {
		themes[i]._tsx1 = thops[i].x1;
		themes[i]._tsy1 = thops[i].y1;
		themes[i]._tsx2 = thops[i].x2;
		themes[i]._tsy2 = thops[i].y2;
	}
	numthemes = numops;
}

#ifdef HELLFIRE
static void DRLG_L5PlaceRndSet(const BYTE* miniset, BYTE rndper)
{
	int sx, sy, sw, sh, xx, yy, ii;
	bool found;

	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXY - sh; sy++) {
		for (sx = 0; sx < DMAXX - sw; sx++) {
			found = true;
			ii = 2;
			for (yy = 0; yy < sh && found; yy++) {
				for (xx = 0; xx < sw && found; xx++) {
					if (miniset[ii] != 0 && dungeon[xx + sx][yy + sy] != miniset[ii]) {
						found = false;
					}
					if (drlgFlags[xx + sx][yy + sy] != 0) {
						found = false;
					}
					ii++;
				}
			}
			if (!found)
				continue;
			// assert(ii == sw * sh + 2);
			if (miniset[ii] >= 89 && miniset[ii] <= 101) {
				// BUGFIX: accesses to dungeon can go out of bounds (fixed)
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84 (fixed)
				if ((sx > 0 && dungeon[sx - 1][sy] >= 89 && dungeon[sx - 1][sy] <= 101)
				 || (dungeon[sx + 1][sy] >= 89 && dungeon[sx + 1][sy] <= 101)
				 || (dungeon[sx][sy + 1] >= 89 && dungeon[sx][sy + 1] <= 101)
				 || (sy > 0 && dungeon[sx][sy - 1] >= 89 && dungeon[sx][sy - 1] <= 101)) {
					continue;
				}
			}
			if (random_(0, 100) < rndper) {
				for (yy = 0; yy < sh; yy++) {
					for (xx = 0; xx < sw; xx++) {
						if (miniset[ii] != 0) {
							dungeon[xx + sx][yy + sy] = miniset[ii];
						}
						ii++;
					}
				}
			}
		}
	}
}
#endif

/*
 * Replace undecorated tiles with matching decorated tiles.
 * New dungeon values: 97 98 99 100 101 102 103 104 105 106 107 108 (97..108)  121 122 123 124 125 (121..125)  133 134 135 136 137 138 (133..138)
 */
static void DRLG_L1Subs()
{
	int x, y;
	BYTE c, k;
	int8_t rv;
	const unsigned MAX_MATCH = 11;
	const unsigned NUM_L1TYPES = 139;
	static_assert(MAX_MATCH <= INT8_MAX, "MAX_MATCH does not fit to rv(int8_t) in DRLG_L1Subs.");
	static_assert(NUM_L1TYPES <= UCHAR_MAX, "NUM_L1TYPES does not fit to i(BYTE) in DRLG_L1Subs.");
#if DEBUG_MODE
	for (int i = sizeof(L1BTYPES) - 1; i >= 0; i--) {
		if (L1BTYPES[i] != 0) {
			if (i >= NUM_L1TYPES)
				app_fatal("Value %d is ignored in L1BTYPES at %d", L1BTYPES[i], i);
			break;
		}
	}

	for (int i = 0; i < sizeof(L1BTYPES); i++) {
		c = L1BTYPES[i];
		if (c == 0)
			continue;
		x = 0;
		for (int j = 0; j < sizeof(L1BTYPES); j++) {
			if (c == L1BTYPES[j])
				x++;
		}
		if (x > MAX_MATCH)
			app_fatal("Too many(%d) matching('%d') values in L1BTYPES", x, c);
	}
#endif
	for (x = 0; x < DMAXX; x++) {
		for (y = 0; y < DMAXY; y++) {
			if (random_(0, 4) == 0) {
				c = L1BTYPES[dungeon[x][y]];
				if (c != 0 && (drlgFlags[x][y] & DRLG_FROZEN) == 0) {
					rv = random_(0, MAX_MATCH);
					k = 0;
					while (true) {
						if (c == L1BTYPES[k] && --rv < 0) {
							break;
						}
						if (++k == NUM_L1TYPES)
							k = 0;
					}
					dungeon[x][y] = k;
				}
			}
		}
	}
}

#ifdef HELLFIRE
static void DRLG_L5Crypt_pattern2(BYTE rndper)
{
	/** Miniset: Use random vertical wall tile A1. */
	DRLG_PlaceRndTile(1, 112, rndper);
	/** Miniset: Use random horizontal wall tile 1. */
	DRLG_PlaceRndTile(2, 113, rndper);
	/** Miniset: South-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(3, 114, rndper);
	DRLG_PlaceRndTile(28, 126, rndper);
	/** Miniset: Use random corner north 1. */
	DRLG_PlaceRndTile(4, 115, rndper);
	/** Miniset: North-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(5, 116, rndper);
	/** Miniset: Use random vertical wall tile B1. */
	DRLG_PlaceRndTile(6, 117, rndper);
	/** Miniset: Use random concave corner east A1. */
	DRLG_PlaceRndTile(7, 118, rndper);
	/** Miniset: East-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(8, 119, rndper);
	/** Miniset: West-Edge column on the northern side 1. */
	DRLG_PlaceRndTile(9, 120, rndper);
	/** Miniset: Use random vertical wall tile C1. */
	DRLG_PlaceRndTile(10, 121, rndper);
	/** Miniset: Column on the northern side of a vertical wall 4. */
	DRLG_PlaceRndTile(11, 122, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 4. */
	DRLG_PlaceRndTile(12, 123, rndper);
	/** Miniset: Use random floor tile 9. */
	DRLG_PlaceRndTile(13, 124, rndper);
	/** Miniset: Use random concave corner east B1. */
	DRLG_PlaceRndTile(14, 125, rndper);
	/** Miniset: Use random column 2. */
	DRLG_PlaceRndTile(15, 126, rndper);
	/** Miniset: Use random concave corner north A1. */
	DRLG_PlaceRndTile(16, 127, rndper);
	/** Miniset: Use random concave corner north B1. */
	DRLG_PlaceRndTile(17, 128, rndper);
}

static void DRLG_L5Crypt_pattern3(BYTE rndper)
{
	/** Miniset: Use random vertical wall tile A2. */
	DRLG_PlaceRndTile(1, 129, rndper);
	/** Miniset: Use random horizontal wall tile 2. */
	DRLG_PlaceRndTile(2, 130, rndper);
	/** Miniset: South-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(3, 131, rndper);
	DRLG_PlaceRndTile(28, 143, rndper);
	/** Miniset: Use random corner north 2. */
	DRLG_PlaceRndTile(4, 132, rndper);
	/** Miniset: North-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(5, 133, rndper);
	/** Miniset: Use random vertical wall tile B2. */
	DRLG_PlaceRndTile(6, 134, rndper);
	/** Miniset: Use random concave corner east A2. */
	DRLG_PlaceRndTile(7, 135, rndper);
	/** Miniset: East-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(8, 136, rndper);
	/** Miniset: West-Edge column on the northern side 2. */
	DRLG_PlaceRndTile(9, 137, rndper);
	/** Miniset: Use random vertical wall tile C2. */
	DRLG_PlaceRndTile(10, 138, rndper);
	/** Miniset: Column on the northern side of a vertical wall 5. */
	DRLG_PlaceRndTile(11, 139, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 5. */
	DRLG_PlaceRndTile(12, 140, rndper);
	/** Miniset: Use random floor tile 10. */
	DRLG_PlaceRndTile(13, 141, rndper);
	/** Miniset: Use random concave corner east B2. */
	DRLG_PlaceRndTile(14, 142, rndper);
	/** Miniset: Use random column 3. (cracked column) */
	DRLG_PlaceRndTile(15, 143, rndper);
	/** Miniset: Use random concave corner north A2. */
	DRLG_PlaceRndTile(16, 144, rndper);
	/** Miniset: Use random concave corner north B2. */
	DRLG_PlaceRndTile(17, 145, rndper);
}

static void DRLG_L5Crypt_pattern4(BYTE rndper)
{
	/** Miniset: Use random vertical wall tile A3. */
	DRLG_PlaceRndTile(1, 146, rndper);
	/** Miniset: Use random horizontal wall tile 3. */
	DRLG_PlaceRndTile(2, 147, rndper);
	/** Miniset: South-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(3, 148, rndper);
	// DRLG_PlaceRndTile(28, 160, rndper);
	/** Miniset: Use random corner north 3. */
	DRLG_PlaceRndTile(4, 149, rndper);
	/** Miniset: North-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(5, 150, rndper);
	/** Miniset: Use random vertical wall tile B3. */
	DRLG_PlaceRndTile(6, 151, rndper);
	/** Miniset: Use random concave corner east A3. */
	DRLG_PlaceRndTile(7, 152, rndper);
	/** Miniset: East-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(8, 153, rndper);
	/** Miniset: West-Edge column on the northern side 3. */
	DRLG_PlaceRndTile(9, 154, rndper);
	/** Miniset: Use random vertical wall tile C3. */
	DRLG_PlaceRndTile(10, 155, rndper);
	/** Miniset: Column on the northern side of a vertical wall 6. */
	DRLG_PlaceRndTile(11, 156, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 6. */
	DRLG_PlaceRndTile(12, 157, rndper);
	/** Miniset: Use random floor tile 11. */
	DRLG_PlaceRndTile(13, 158, rndper);
	/** Miniset: Use random concave corner east B3. */
	DRLG_PlaceRndTile(14, 159, rndper);
	/** Miniset: Use random column 4. (Lava fountain) */
	DRLG_L5PlaceRndSet(L5RNDCOL7, rndper); // 15 -> 160
	/** Miniset: Use random concave corner north A3. */
	DRLG_PlaceRndTile(16, 161, rndper);
	/** Miniset: Use random concave corner north B3. */
	DRLG_PlaceRndTile(17, 162, rndper);
}

static void DRLG_L5Crypt_pattern5(BYTE rndper)
{
	DRLG_L5PlaceRndSet(L5RNDLFLOOR2, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR3, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR4, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR5, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR6, rndper);
	DRLG_L5PlaceRndSet(L5RNDLFLOOR7, rndper);
	/** Miniset: Use random floor tile 5. */
	DRLG_PlaceRndTile(13, 163, rndper);
	/** Miniset: Use random floor tile 6. */
	DRLG_PlaceRndTile(13, 164, rndper);
	/** Miniset: Use random floor tile 7. */
	DRLG_PlaceRndTile(13, 165, rndper);
	/** Miniset: Use random floor tile 8. */
	DRLG_PlaceRndTile(13, 166, rndper);
}

static void DRLG_L5Crypt_pattern6(BYTE rndper)
{
	/** Miniset: Column on the northern side of a vertical wall 2. */
	DRLG_PlaceRndTile(11, 185, rndper);
	/** Miniset: Column on the northern side of a vertical wall 3. */
	DRLG_PlaceRndTile(11, 186, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 2. */
	DRLG_PlaceRndTile(96, 187, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 3. */
	DRLG_PlaceRndTile(96, 188, rndper);
	/** Miniset: Use random column 1. (Cracked) */
	DRLG_L5PlaceRndSet(L5RNDCOL9, rndper); // 15 -> 192
	/** Miniset: Replace vertical wall tile on the bottom 1. */
	DRLG_PlaceRndTile(89, 173, rndper);
	/** Miniset: Replace vertical wall tile on the bottom 2. */
	DRLG_PlaceRndTile(89, 174, rndper);
	/** Miniset: Replace vertical wall tile in the middle 1. */
	DRLG_PlaceRndTile(90, 175, rndper);
	/** Miniset: Replace vertical wall tile in the middle 2. */
	DRLG_PlaceRndTile(90, 176, rndper);
	/** Miniset: Replace vertical wall tile on the top 1. */
	DRLG_PlaceRndTile(91, 177, rndper);
	/** Miniset: Replace vertical wall tile on the top 2. */
	DRLG_PlaceRndTile(91, 178, rndper);
	/** Miniset: Replace random horizontal wall tile on east 1. but pointless? */
	DRLG_PlaceRndTile(92, 179, rndper);
	/** Miniset: Replace random horizontal wall tile on east 2. */
	DRLG_PlaceRndTile(92, 180, rndper);
	/** Miniset: Replace random horizontal wall tile in the middle 1. */
	DRLG_PlaceRndTile(93, 181, rndper);
	/** Miniset: Replace random horizontal wall tile in the middle 2. */
	DRLG_PlaceRndTile(93, 182, rndper);
	/** Miniset: Replace random horizontal wall tile on west 1. */
	DRLG_PlaceRndTile(94, 183, rndper);
	/** Miniset: Replace random horizontal wall tile on west 2. */
	DRLG_PlaceRndTile(94, 184, rndper);
	/** Miniset: Replace random floor tile 1 with another one. */
	DRLG_PlaceRndTile(97, 191, rndper);
	/** Miniset: Replace random floor tile 2 with another one 1. */
	DRLG_PlaceRndTile(98, 189, rndper);
	/** Miniset: Replace random floor tile 2 with another one 2. */
	DRLG_PlaceRndTile(98, 190, rndper);
	/** Miniset: Replace random floor tile 3 with another one 1. */
	DRLG_PlaceRndTile(99, 193, rndper);
	/** Miniset: Replace random floor tile 3 with another one 2. */
	DRLG_PlaceRndTile(99, 194, rndper);
	/** Miniset: Replace random floor tile 4 with another one 1. */
	DRLG_PlaceRndTile(100, 195, rndper);
	/** Miniset: Replace random floor tile 4 with another one 2. */
	DRLG_PlaceRndTile(100, 196, rndper);
	/** Miniset: Replace random large floor tile 1. */
	DRLG_PlaceRndTile(101, 197, rndper);
	/** Miniset: Replace random large floor tile 2. */
	DRLG_PlaceRndTile(101, 198, rndper);
}

static void DRLG_L5Crypt_pattern7(BYTE rndper)
{
	DRLG_L5PlaceRndSet(L5RNDFLOOR1, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR2, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR3, rndper);
	DRLG_L5PlaceRndSet(L5RNDFLOOR4, rndper);
}
#endif

/*
 * Spread transVals further.
 * - spread transVals on corner tiles to make transparency smoother.
 */
static void DRLG_L1TransFix()
{
	int xx, yy, i, j;
	//BYTE tv;

	yy = DBORDERY;

	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;

		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			/* commented out because a simplified version is added below
			case 18:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 19:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 20:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 23:
				// BUGFIX: Should check for `j > 0` first. (fixed)
				if (j > 0 && dungeon[i][j - 1] == 18) {
					DRLG_CopyTrans(xx, yy, xx + 1, yy);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 24:
				// BUGFIX: Should check for `i + 1 < DMAXX` first. (fixed)
				if (i < DMAXX - 1 && dungeon[i + 1][j] == 19) {
					DRLG_CopyTrans(xx, yy, xx, yy + 1);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;*/
			// fix transVals of corners
			case 22:
			//case 203:
			case 20:
			//case 201:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 23:
			//case 204:
				//if (dungeon[i][j - 1] != 199) {
				//	break;
				//}
				/* fall-through */
			case 18:
			//case 199:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 24:
			//case 205:
				//if (dungeon[i + 1][j] != 200) {
				//	break;
				//}
				/* fall-through */
			case 19:
			//case 200:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			/* fix transVals around the stairs - necessary only if DRLG_FloodTVal is run after the placement
			// - due to id conflict between hellfire and diablo, this does not work on tile 63
			case 55:
			case 58:
			case 61:
			case 62:
			case 63:
			case 66:
				tv = dTransVal[xx + 6][yy];
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix walls on the west side
				//if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				//if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				break;
			case 67:
				DRLG_CopyTrans(xx, yy + 1, xx + 1, yy);
				break;
			case 68:
				DRLG_CopyTrans(xx, yy + 1, xx, yy);
				break;*/
			}
			xx += 2;
		}
		yy += 2;
	}
}

#ifdef HELLFIRE
/*
 * Spread transVals further.
 * - spread transVals on corner tiles to make transparency smoother.
 */
static void DRLG_L5TransFix()
{
	int xx, yy, i, j;
	//BYTE tv;

	yy = DBORDERY;

	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;

		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			/* commented out because a simplified version is added below
			case 18:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 19:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 20:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 23:
				// BUGFIX: Should check for `j > 0` first. (fixed)
				if (j > 0 && dungeon[i][j - 1] == 18) {
					DRLG_CopyTrans(xx, yy, xx + 1, yy);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 24:
				// BUGFIX: Should check for `i + 1 < DMAXX` first. (fixed)
				if (i < DMAXX - 1 && dungeon[i + 1][j] == 19) {
					DRLG_CopyTrans(xx, yy, xx, yy + 1);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;*/
			// fix transVals of corners
			case 22:
			//case 86:
			case 20:
			//case 84:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 23:
			//case 87:
				//if (dungeon[i][j - 1] != 82) {
				//	break;
				//}
				/* fall-through */
			case 18:
			//case 82:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 24:
			//case 88:
				//if (dungeon[i + 1][j] != 83) {
				//	break;
				//}
				/* fall-through */
			case 19:
			//case 83:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			/* fix transVals around the stairs - necessary only if DRLG_FloodTVal is run after the placement
			// - due to id conflict between hellfire and diablo, this does not work on tile 63
			case 55:
			case 58:
			case 61:
			case 62:
			case 63:
			case 66:
				tv = dTransVal[xx + 6][yy];
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix walls on the west side
				//if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				//if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				break;
			case 67:
				DRLG_CopyTrans(xx, yy + 1, xx + 1, yy);
				break;
			case 68:
				DRLG_CopyTrans(xx, yy + 1, xx, yy);
				break;*/
			}
			xx += 2;
		}
		yy += 2;
	}
}
#endif

void DRLG_L1InitTransVals()
{
	static_assert(sizeof(drlg.transvalMap) == sizeof(dungeon), "transvalMap vs dungeon mismatch.");
	memcpy(drlg.transvalMap, dungeon, sizeof(dungeon));

	DRLG_FloodTVal();
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		DRLG_L5TransFix();
	} else
#endif
	{
		DRLG_L1TransFix();
	}
}

/*
 * Replace tiles with complete ones to hide rendering glitch of transparent corners.
 * New dungeon values: 199..205 / 82..88
 * Obsolete dungeon values: 18..24
 */
/*static void DRLG_L1Corners()
{
	int i;
	BYTE dv = 181, *pTmp;

	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L1Corners.");
	pTmp = &dungeon[0][0];
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		dv = 64;
	}
	// assert(currLvl._dType == DTYPE_CATHEDRAL);
#endif
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		if (*pTmp >= 18 && *pTmp <= 24) // && *pTmp != 22 && *pTmp != 20
			*pTmp += dv;
	}
	//for (j = 0; j < DMAXY - 1; j++) {
	//	for (i = 0; i < DMAXX - 1; i++) {
	//		switch (dungeon[i][j]) {
	//		case 18:
	//			if (dungeon[i][j + 1] != 18)
	//				dungeon[i][j] = 199;
	//			break;
	//		case 19:
	//			if (dungeon[i + 1][j] != 19)
	//				dungeon[i][j] = 200;
	//			break;
	//		case 21:
	//			if (dungeon[i + 1][j] != 19 || dungeon[i][j + 1] != 18)
	//				dungeon[i][j] = 202;
	//			break;
	//		case 23:
	//			if (dungeon[i][j + 1] != 18)
	//				dungeon[i][j] = 204;
	//			break;
	//		case 24:
	//			if (dungeon[i + 1][j] != 19)
	//				dungeon[i][j] = 205;
	//			break;
	//		}
	//	}
	//}
}*/

/*
 * Miniset replacement of corner tiles.
 * New dungeon values: (8 16)
 * TODO: use DRLG_PlaceMiniSet instead?
 */
/*static void DRLG_L1CornerFix()
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			// 0,  1, 0,  search
			//13, 17, 0,

			// 0,  0, 0, replace
			// 0, 16, 0,
			if (!(drlgFlags[i][j] & DRLG_PROTECTED) && dungeon[i][j] == 17 && dungeon[i - 1][j] == 13 && dungeon[i][j - 1] == 1) {
				dungeon[i][j] = 16;
				drlgFlags[i][j - 1] &= DRLG_PROTECTED;
			}
			// 0, 202, 13,  search
			// 0,   1,  0,

			// 0,   8,  0, replace
			// 0,   0,  0,
			if (dungeon[i][j] == 202 && dungeon[i + 1][j] == 13 && dungeon[i][j + 1] == 1) {
				dungeon[i][j] = 8;
			}
		}
	}
}*/

static void DRLG_L1()
{
	int i, areaidx;
	// bool placeWater = QuestStatus(Q_PWATER);
	const int arealimits[] = { DMAXX * DMAXY, 761, 693, 533 };
	areaidx = 0;
	if (currLvl._dLevelIdx == DLV_CATHEDRAL1) {
		areaidx = 2;
	} else if (currLvl._dLevelIdx == DLV_CATHEDRAL2) {
		areaidx = 1;
	}

	while (true) {
		do {
			memset(dungeon, 0, sizeof(dungeon));
			DRLG_L1CreateDungeon();
			i = DRLG_L1GetArea();
		} while (i > arealimits[areaidx]  || i < arealimits[areaidx + 1]);

		DRLG_L1MakeMegas();
		L1TileFix();
		memset(drlgFlags, 0, sizeof(drlgFlags));
		L1FillChambers();
		L1AddWall();
		L1ClearChamberFlags();
		if (currLvl._dDynLvl) {
#ifdef HELLFIRE
			POS32 warpPos = DRLG_PlaceMiniSet(currLvl._dType == DTYPE_CRYPT ? L5DYNENTRY : L1DYNENTRY);
#else
			POS32 warpPos = DRLG_PlaceMiniSet(L1DYNENTRY);
#endif
			if (warpPos.x < 0) {
				continue;
			}
			pWarps[DWARP_ENTRY]._wx = warpPos.x;
			pWarps[DWARP_ENTRY]._wy = warpPos.y;
			pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX + 2;
			pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY + 1;
			pWarps[DWARP_ENTRY]._wtype = WRPT_CIRCLE;
			break;
		}
		if (QuestStatus(Q_PWATER)) {
			POS32 warpPos = DRLG_PlaceMiniSet(PWATERIN);
			if (warpPos.x < 0) {
				continue;
			}
			pWarps[DWARP_SIDE]._wx = warpPos.x + 2;
			pWarps[DWARP_SIDE]._wy = warpPos.y + 3;
			pWarps[DWARP_SIDE]._wx = 2 * pWarps[DWARP_SIDE]._wx + DBORDERX + 1;
			pWarps[DWARP_SIDE]._wy = 2 * pWarps[DWARP_SIDE]._wy + DBORDERY;
			pWarps[DWARP_SIDE]._wtype = WRPT_L1_PWATER;
			pWarps[DWARP_SIDE]._wlvl = questlist[Q_PWATER]._qslvl;
		}
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_CRYPT) {
			POS32 warpPos = DRLG_PlaceMiniSet(L5USTAIRS); // L5USTAIRS (3, 6), was STAIRSUP, entry == ENTRY_MAIN
			if (warpPos.x < 0) {
				continue;
			}
			if (currLvl._dLevelIdx == DLV_CRYPT1) {
				pWarps[DWARP_TOWN]._wx = warpPos.x + 1;
				pWarps[DWARP_TOWN]._wy = warpPos.y + 2;
				pWarps[DWARP_TOWN]._wx = 2 * pWarps[DWARP_TOWN]._wx + DBORDERX;
				pWarps[DWARP_TOWN]._wy = 2 * pWarps[DWARP_TOWN]._wy + DBORDERY;
				pWarps[DWARP_TOWN]._wtype = WRPT_L1_UP;
			} else {
				pWarps[DWARP_ENTRY]._wx = warpPos.x + 1;
				pWarps[DWARP_ENTRY]._wy = warpPos.y + 2;
				pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX;
				pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY;
				pWarps[DWARP_ENTRY]._wtype = WRPT_L1_UP;
			}
			if (currLvl._dLevelIdx != DLV_CRYPT4) {
				warpPos = DRLG_PlaceMiniSet(L5DSTAIRS); // L5DSTAIRS (3, 7)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_EXIT]._wx = warpPos.x + 1;
				pWarps[DWARP_EXIT]._wy = warpPos.y + 3;
				pWarps[DWARP_EXIT]._wx = 2 * pWarps[DWARP_EXIT]._wx + DBORDERX + 1;
				pWarps[DWARP_EXIT]._wy = 2 * pWarps[DWARP_EXIT]._wy + DBORDERY;
				pWarps[DWARP_EXIT]._wtype = WRPT_L1_DOWN;
			}
		} else
#endif
		{
			// assert(currLvl._dType == DTYPE_CATHEDRAL);
			POS32 warpPos = DRLG_PlaceMiniSet(L1USTAIRS); // L1USTAIRS (3, 4)
			if (warpPos.x < 0) {
				continue;
			}
			if (currLvl._dLevelIdx == DLV_CATHEDRAL1) {
				pWarps[DWARP_TOWN]._wx = warpPos.x + 1;
				pWarps[DWARP_TOWN]._wy = warpPos.y + 1;
				pWarps[DWARP_TOWN]._wx = 2 * pWarps[DWARP_TOWN]._wx + DBORDERX;
				pWarps[DWARP_TOWN]._wy = 2 * pWarps[DWARP_TOWN]._wy + DBORDERY;
				pWarps[DWARP_TOWN]._wtype = WRPT_L1_UP;
			} else {
				pWarps[DWARP_ENTRY]._wx = warpPos.x + 1;
				pWarps[DWARP_ENTRY]._wy = warpPos.y + 1;
				pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX;
				pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY;
				pWarps[DWARP_ENTRY]._wtype = WRPT_L1_UP;
			}
			if (pSetPieces[0]._sptype == SPT_SKELKING) {
				pWarps[DWARP_SIDE]._wx = pSetPieces[0]._spx + 6; // L1DSTAIRS (3, 5)
				pWarps[DWARP_SIDE]._wy = pSetPieces[0]._spy + 3;
				pWarps[DWARP_SIDE]._wx = 2 * pWarps[DWARP_SIDE]._wx + DBORDERX;
				pWarps[DWARP_SIDE]._wy = 2 * pWarps[DWARP_SIDE]._wy + DBORDERY + 1;
				pWarps[DWARP_SIDE]._wtype = WRPT_L1_SKING;
				pWarps[DWARP_SIDE]._wlvl = questlist[Q_SKELKING]._qslvl;
			}
			if (pSetPieces[0]._sptype == SPT_BANNER) {
				pWarps[DWARP_EXIT]._wx = pSetPieces[0]._spx + 1; // L1DSTAIRS (3, 5)
				pWarps[DWARP_EXIT]._wy = pSetPieces[0]._spy + 5;
			} else {
				warpPos = DRLG_PlaceMiniSet(L1DSTAIRS); // L1DSTAIRS (3, 5)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_EXIT]._wx = warpPos.x + 1;
				pWarps[DWARP_EXIT]._wy = warpPos.y + 2;
			}
			pWarps[DWARP_EXIT]._wx = 2 * pWarps[DWARP_EXIT]._wx + DBORDERX + 1;
			pWarps[DWARP_EXIT]._wy = 2 * pWarps[DWARP_EXIT]._wy + DBORDERY;
			pWarps[DWARP_EXIT]._wtype = WRPT_L1_DOWN;
		}
		break;
	}

	/*if (placeWater) {
		int x, y;

		x = pWarps[DWARP_SIDE]._wx + 1;
		y = pWarps[DWARP_SIDE]._wy + 1;
		// fix transVal of the set-map (entrance)
		DRLG_CopyTrans(x + 0, y + 2, x + 0, y + 0);
		DRLG_CopyTrans(x + 1, y + 2, x + 1, y + 0);
		DRLG_CopyTrans(x + 0, y + 2, x + 0, y + 1);
		DRLG_CopyTrans(x + 1, y + 2, x + 1, y + 1);
	}*/
	// DRLG_L1Corners(); - commented out, because this is no longer necessary
	// DRLG_L1CornerFix(); - commented out, because this is no longer necessary

	// DRLG_L1PlaceDoors(); - commented out, because this is no longer necessary

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		DRLG_L5Shadows();
		/** Miniset: Use random vertical wall 4. (Statue). */
		DRLG_PlaceRndTile(1, 199, 10);
		/** Miniset: Use random vertical wall 5. (Succubus statue) */
		DRLG_PlaceRndTile(1, 201, 10);
		/** Miniset: Use random horizontal wall 4. (Statue). */
		DRLG_PlaceRndTile(2, 200, 10);
		/** Miniset: Use random horizontal wall 5. (Succubus statue) */
		DRLG_PlaceRndTile(2, 202, 10);
		DRLG_L5PlaceRndSet(L5VERTCOL1, 95); // 11 -> 95
		DRLG_L5PlaceRndSet(L5HORIZCOL1a, 95); // 12 -> 96
		DRLG_L5PlaceRndSet(L5HORIZCOL1b, 95);
		// DRLG_L5PlaceRndSet(L5HORIZCOL1c, 95);
		DRLG_L5PlaceRndSet(L5PREVERTWALL, 100);
		DRLG_L5PlaceRndSet(L5PREHORIZWALL, 100);
		DRLG_L5PlaceRndSet(L5RNDLFLOOR1, 60);
		switch (currLvl._dLevelNum) {
		case DLV_CRYPT1:
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(15);
			DRLG_L5Crypt_pattern4(5);
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(5);
			DRLG_L5Crypt_pattern5(20);
			break;
		case DLV_CRYPT2:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(10);
			DRLG_L5Crypt_pattern5(20);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(10);
			break;
		case DLV_CRYPT3:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(15);
			DRLG_L5Crypt_pattern5(30);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(15);
			break;
		case DLV_CRYPT4:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(20);
			DRLG_L5Crypt_pattern5(30);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(20);
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else
#endif
	{
		// assert(currLvl._dType == DTYPE_CATHEDRAL);
		DRLG_L1PlaceThemeRooms();

		DRLG_L1Shadows();
		for (i = RandRange(5, 9); i > 0; i--)
			DRLG_PlaceMiniSet(LAMPS);
	}
}
#if 0
static void DRLG_L1FixMap()
{
	uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;

	if (pSetPieces[0]._sptype == SPT_LVL_BETRAYER) {
		// patch the map - Vile1.DUN
		// external tiles
		for (int y = 0; y < 23; y++) {
			for (int x = 0; x < 21; x++) {
				uint16_t currTileRef = SwapLE16(lm[2 + x + y * 21]);
				if (currTileRef >= 181 + 18 && currTileRef <= 181 + 24) {
					lm[2 + x + y * 21] = SwapLE16(currTileRef - 181);
				}
			}
		}
		// replace default tiles with external piece
		// - SW in the middle
		lm[2 + 12 + 22 * 21] = SwapLE16(203 - 181);
		lm[2 + 13 + 22 * 21] = SwapLE16(203 - 181);
		lm[2 + 14 + 22 * 21] = SwapLE16(203 - 181);
		// - SE
		for (int i = 1; i < 23; i++) {
			lm[2 + 20 + i * 21] = SwapLE16(203 - 181);
		}
		// use common tiles
		lm[2 + 11 +  3 * 21] = SwapLE16(18);
		// ensure the changing tiles are reserved
		lm[2 + 21 * 23 +  4 +  5 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 +  4 +  6 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 +  4 +  7 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 +  5 +  5 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 +  6 +  5 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 12 +  5 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 13 +  5 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 14 +  5 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 15 +  5 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 15 +  6 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 15 +  7 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 +  7 + 18 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 +  8 + 18 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 +  9 + 18 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 10 + 18 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 11 + 18 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 12 + 18 * 21] = SwapLE16(3);
		lm[2 + 21 * 23 + 14 + 15 * 21] = SwapLE16(3);
	} else if (pSetPieces[0]._sptype == SPT_LVL_SKELKING) {
		// patch the map - SklKng1.DUN
		// external tiles
		for (int y = 0; y < 25; y++) {
			for (int x = 0; x < 37; x++) {
				uint16_t currTileRef = SwapLE16(lm[2 + x + y * 37]);
				if (currTileRef >= 181 + 18 && currTileRef <= 181 + 24) {
					lm[2 + x + y * 37] = SwapLE16(currTileRef - 181);
				}
			}
		}
		// useless tiles
		lm[2 + 15 + 12 * 37] = 0;
		lm[2 + 15 + 16 * 37] = 0;
		lm[2 + 25 + 11 * 37] = 0;
		lm[2 + 24 + 23 * 37] = 0;
		// fix the shadows
		lm[2 +  9 + 2 * 37] = SwapLE16(143);
		lm[2 + 12 + 2 * 37] = SwapLE16(143);
		lm[2 + 10 + 5 * 37] = SwapLE16(157);
		lm[2 + 24, 18 * 37] = SwapLE16(140);
		// use common tiles
		lm[2 +  7 + 14 * 37] = SwapLE16(84);
		// use the new shadows
		lm[2 +  9 +  3 * 37] = SwapLE16(139);
		lm[2 +  9 +  4 * 37] = SwapLE16(139);
		lm[2 +  9 +  5 * 37] = SwapLE16(126);
		lm[2 + 12 +  3 * 37] = SwapLE16(139);
		lm[2 + 12 +  4 * 37] = SwapLE16(139);
		lm[2 + 12 +  5 * 37] = SwapLE16(127);
		lm[2 +  4 + 15 * 37] = SwapLE16(150);
		lm[2 +  6 + 16 * 37] = SwapLE16(150);
		lm[2 + 15 + 17 * 37] = SwapLE16(159);
		lm[2 + 15 + 13 * 37] = SwapLE16(159);
		lm[2 + 27 + 13 * 37] = SwapLE16(159);
		lm[2 +  8 + 10 * 37] = SwapLE16(159);
		lm[2 +  8 + 12 * 37] = SwapLE16(144);
		lm[2 + 13 + 12 * 37] = SwapLE16(144);
		lm[2 +  8 + 17 * 37] = SwapLE16(144);
		lm[2 + 13 + 17 * 37] = SwapLE16(144);
		// remove fix decorations
		lm[2 +  3 + 15 * 37] = SwapLE16(2);
		lm[2 +  5 + 20 * 37] = SwapLE16(1);
		lm[2 +  6 +  9 * 37] = SwapLE16(2);
		lm[2 + 10 +  1 * 37] = SwapLE16(2);
		lm[2 + 13 +  1 * 37] = SwapLE16(2);
		lm[2 + 15 +  9 * 37] = SwapLE16(2);
		lm[2 + 17 + 14 * 37] = SwapLE16(2);
		lm[2 + 22 + 12 * 37] = SwapLE16(2);
		lm[2 + 20 +  8 * 37] = SwapLE16(1);
		lm[2 + 21 +  7 * 37] = SwapLE16(2);
		lm[2 + 24 +  7 * 37] = SwapLE16(2);
		lm[2 + 25 + 12 * 37] = SwapLE16(2);
		lm[2 + 26 + 12 * 37] = SwapLE16(2);
		lm[2 + 29 + 14 * 37] = SwapLE16(2);
		lm[2 + 16 + 15 * 37] = SwapLE16(11);
		lm[2 +  8 +  3 * 37] = SwapLE16(1);
		lm[2 +  8 +  4 * 37] = SwapLE16(1);
		lm[2 + 10 +  7 * 37] = SwapLE16(1);
		lm[2 + 10 +  9 * 37] = SwapLE16(1);
		lm[2 + 21 + 13 * 37] = SwapLE16(1);
		lm[2 + 21 + 17 * 37] = SwapLE16(1);
		lm[2 + 23 +  9 * 37] = SwapLE16(1);
		lm[2 + 23 + 10 * 37] = SwapLE16(1);
		lm[2 + 23 + 20 * 37] = SwapLE16(1);
		lm[2 + 23 + 21 * 37] = SwapLE16(1);
		lm[2 + 31 + 15 * 37] = SwapLE16(11);
		lm[2 + 31 + 16 * 37] = SwapLE16(11);
		lm[2 + 32 + 16 * 37] = 0;
		lm[2 + 33 + 16 * 37] = 0;
		// - remove sarcophagi tiles
		lm[2 +  6 + 11 * 37] = 0;
		lm[2 +  7 + 11 * 37] = 0;
		lm[2 + 15 + 11 * 37] = 0;
		lm[2 + 16 + 11 * 37] = 0;
		lm[2 +  6 + 20 * 37] = 0;
		lm[2 +  7 + 20 * 37] = 0;
		lm[2 + 15 + 20 * 37] = 0;
		lm[2 + 16 + 20 * 37] = 0;
		lm[2 + 24 +  8 * 37] = 0;
		lm[2 + 24 + 22 * 37] = 0;
		// ensure the changing tiles are reserved
		lm[2 + 25 * 37 + 10 + 11 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 11 + 11 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 12 + 11 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 20 + 14 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 20 + 15 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 20 + 16 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 21 + 14 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 21 + 15 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 21 + 16 * 37] = SwapLE16(3);
		lm[2 + 25 * 37 + 23 +  8 * 37] = SwapLE16(3);
	}
}

static void DRLG_L1FixPreMap(int idx)
{
	uint16_t* lm = (uint16_t*)pSetPieces[idx]._spData;

	if (pSetPieces[idx]._sptype == SPT_BANNER) {
		// patch set-piece - Banner2.DUN
		// useless tiles
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (x >= 3 && y >= 3 && x <= 3 && y <= 6) {
					continue;
				}
				if (x >= 3 && y >= 5 && x <= 6 && y <= 5) {
					continue;
				}
				lm[2 + x + y * 8] = 0;
			}
		}
		// protect subtiles from spawning monsters/objects
		// - protect objects from monsters/objects
		lm[2 + 8 * 8 + (10 / 2) + ( 2 / 2) * 8] = SwapLE16(3 << 12);
		// - protect monsters from monsters/objects
		lm[2 + 8 * 8 + ( 0 / 2) + ( 0 / 2) * 8] = SwapLE16(3 << 14);
		lm[2 + 8 * 8 + ( 2 / 2) + ( 2 / 2) * 8] = SwapLE16(3 << 10);
		lm[2 + 8 * 8 + ( 8 / 2) + ( 0 / 2) * 8] = SwapLE16(3 << 12);
		lm[2 + 8 * 8 + ( 8 / 2) + ( 4 / 2) * 8] = SwapLE16(3 << 10);
		lm[2 + 8 * 8 + (10 / 2) + ( 2 / 2) * 8] = SwapLE16(3 << 10);
		// - protect area from monsters/objects
		lm[2 + 8 * 8 + ( 0 / 2) + ( 6 / 2) * 8] = SwapLE16(3 << 14);
		lm[2 + 8 * 8 + ( 0 / 2) + ( 8 / 2) * 8] = SwapLE16((3 << 10) | (3 << 14));
		lm[2 + 8 * 8 + ( 0 / 2) + (10 / 2) * 8] = SwapLE16((3 << 10) | (3 << 14));
		lm[2 + 8 * 8 + ( 0 / 2) + (12 / 2) * 8] = SwapLE16((3 << 10) | (3 << 14));
		for (int x = 2 / 2; x <= 12 / 2; x++) {
			lm[2 + 8 * 8 + x + (6 / 2) * 8] = SwapLE16((3 << 12) | (3 << 14));
		}
		for (int y = 8 / 2; y <= 12 / 2; y++) {
			for (int x = 2 / 2; x <= 12 / 2; x++) {
				lm[2 + 8 * 8 + x + y * 8] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// - replace monsters
		for (int y = 7; y <= 9; y++) {
			for (int x = 7; x <= 13; x++) {
				lm[2 + 8 * 8 + 8 * 8 * 2 * 2 + x + y * 8 * 2] = SwapLE16(16);
			}
		}
		// - remove monsters
		lm[2 + 8 * 8 + 8 * 8 * 2 * 2 + 1 + 4 * 8 * 2] = 0;
		lm[2 + 8 * 8 + 8 * 8 * 2 * 2 + 13 + 5 * 8 * 2] = 0;
		lm[2 + 8 * 8 + 8 * 8 * 2 * 2 + 7 + 12 * 8 * 2] = 0;
		// - add unique
		lm[2 + 8 * 8 + 8 * 8 * 2 * 2 + 8 + 12 * 8 * 2] = SwapLE16((UMT_SNOTSPIL + 1) | (1 << 15));
		// - add sign-chest
		lm[2 + 8 * 8 + 8 * 8 * 2 * 2 + 8 * 8 * 2 * 2 + 10 + 3 * 8 * 2] = SwapLE16(90);
	} else if (pSetPieces[idx]._sptype == SPT_LVL_BETRAYER) {
		// patch the map - Vile2.DUN
		// useless tiles
		for (int y = 0; y < 23; y++) {
			for (int x = 0; x < 21; x++) {
				// room on the left side
				if (x >= 4 && y >= 5 && x <= 6 && y <= 7) {
					continue;
				}
				if (x >= 4 && y >= 8 && x <= 7 && y <= 10) {
					continue;
				}
				// room on the right side
				if (x == 12 && y == 5) {
					continue;
				}
				if (x >= 13 && y >= 5 && x <= 15 && y <= 7) {
					continue;
				}
				if (x >= 12 && y >= 8 && x <= 14 && y <= 10) {
					continue;
				}
				// main room
				if (x >= 7 && y >= 13 && x <= 13 && y <= 17) {
					continue;
				}
				if (x >= 7 && y >= 18 && x <= 12 && y <= 18) {
					continue;
				}
				if (x >= 8 && y >= 20 && x <= 11 && y <= 22) {
					continue;
				}
				lm[2 + x + y * 21] = 0;
			}
		}
		// - fix empty tiles
		lm[2 + 8 + 16 * 21] = SwapLE16(203 - 181);
		/*lm[2 + 12 + 22 * 21] = SwapLE16(203 - 181);
		lm[2 + 13 + 22 * 21] = SwapLE16(203 - 181);
		lm[2 + 14 + 22 * 21] = SwapLE16(203 - 181);
		for (int i = 1; i < 23; i++) {
			lm[2 + 20 + i * 21] = SwapLE16(203 - 181);
		}*/
		// use the new shadows
		lm[2 + 14 + 5 * 21] = SwapLE16(152);
		// - add monsters
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 16 + 30 * 21 * 2] = SwapLE16((UMT_LAZARUS + 1) | (1 << 15));
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 24 + 29 * 21 * 2] = SwapLE16((UMT_RED_VEX + 1) | (1 << 15));
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 22 + 33 * 21 * 2] = SwapLE16((UMT_BLACKJADE + 1) | (1 << 15));
		// - replace the books
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 21 * 23 * 2 * 2 + 10 + 29 * 21 * 2] = SwapLE16(47);
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 21 * 23 * 2 * 2 + 29 + 30 * 21 * 2] = SwapLE16(47);
	} else if (pSetPieces[idx]._sptype == SPT_LVL_SKELKING) {
		// patch the map - SklKng2.DUN
		// external tiles
		for (int y = 0; y < 25; y++) {
			for (int x = 0; x < 37; x++) {
				uint16_t currTileRef = SwapLE16(lm[2 + x + y * 37]);
				if (currTileRef >= 181 + 18 && currTileRef <= 181 + 24) {
					lm[2 + x + y * 37] = SwapLE16(currTileRef - 181);
				}
			}
		}
		// useless tiles
		for (int y = 0; y < 25; y++) {
			for (int x = 0; x < 37; x++) {
				// large hidden room
				if (x >= 8 && y >= 1 && x <= 15 && y <= 6) {
					continue;
				}
				if (x >= 10 && y >= 7 && x <= 13 && y <= 10) {
					continue;
				}
				if (x == 11 && y == 11) {
					continue;
				}
				// small hidden room
				if (x >= 20 && y >= 7 && x <= 22 && y <= 10) {
					continue;
				}
				if (x == 23 && y == 8) {
					continue;
				}
				// grate
				if (x >= 20 && y >= 14 && x <= 21 && y <= 16) {
					continue;
				}
				lm[2 + x + y * 37] = 0;
			}
		}
		// add sarcophagi
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 37 * 25 * 2 * 2 + 14 + 23 * 37 * 2] = SwapLE16(5);
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 37 * 25 * 2 * 2 + 14 + 41 * 37 * 2] = SwapLE16(5);
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 37 * 25 * 2 * 2 + 32 + 23 * 37 * 2] = SwapLE16(5);
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 37 * 25 * 2 * 2 + 32 + 41 * 37 * 2] = SwapLE16(5);
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 37 * 25 * 2 * 2 + 48 + 45 * 37 * 2] = SwapLE16(5);
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 37 * 25 * 2 * 2 + 48 + 17 * 37 * 2] = SwapLE16(5);
		// add the skeleton king
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 19 + 31 * 37 * 2] = SwapLE16((UMT_SKELKING + 1) | (1 << 15));
		// remove monsters
		for (int y = 21; y <= 41; y++) {
			for (int x = 13; x <= 39; x++) {
				if (x >= 18 && y >= 30 && x <= 20 && y <= 32) {
					continue;
				}
				lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + x + y * 37 * 2] = 0;
			}
		}
		for (int y = 21; y <= 36; y++) {
			for (int x = 43; x <= 59; x++) {
				lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + x + y * 37 * 2] = 0;
			}
		}
		// protect subtiles from spawning additional monsters/objects
		// - protect objects from monsters
		lm[2 + 37 * 25 + (10 / 2) + (20 / 2) * 37] = SwapLE16(3 << 14);
		lm[2 + 37 * 25 + (12 / 2) + (20 / 2) * 37] = SwapLE16(3 << 8);
		lm[2 + 37 * 25 + (12 / 2) + (40 / 2) * 37] = SwapLE16(3 << 12);
		lm[2 + 37 * 25 + (10 / 2) + (36 / 2) * 37] = SwapLE16(3 << 14);
		lm[2 + 37 * 25 + (30 / 2) + (18 / 2) * 37] = SwapLE16(3 << 12);
		lm[2 + 37 * 25 + (32 / 2) + (36 / 2) * 37] = SwapLE16(3 << 14);
		lm[2 + 37 * 25 + (14 / 2) + (22 / 2) * 37] = SwapLE16((3 << 8) | (3 << 12));
		lm[2 + 37 * 25 + (14 / 2) + (40 / 2) * 37] = SwapLE16((3 << 8) | (3 << 12));
		lm[2 + 37 * 25 + (18 / 2) + (30 / 2) * 37] = SwapLE16((3 << 8) | (3 << 14));
		lm[2 + 37 * 25 + (18 / 2) + (32 / 2) * 37] = SwapLE16(3 << 8);
		lm[2 + 37 * 25 + (20 / 2) + (30 / 2) * 37] = SwapLE16(3 << 8);
		lm[2 + 37 * 25 + (20 / 2) + (32 / 2) * 37] = SwapLE16(3 << 8);
		lm[2 + 37 * 25 + (32 / 2) + (22 / 2) * 37] = SwapLE16((3 << 8) | (3 << 12));
		lm[2 + 37 * 25 + (32 / 2) + (40 / 2) * 37] = SwapLE16((3 << 8) | (3 << 12));
		lm[2 + 37 * 25 + (34 / 2) + (18 / 2) * 37] = SwapLE16(3 << 12);
		lm[2 + 37 * 25 + (46 / 2) + (38 / 2) * 37] = SwapLE16(3 << 14);
		lm[2 + 37 * 25 + (48 / 2) + (38 / 2) * 37] = SwapLE16((3 << 12) | (3 << 14));
		lm[2 + 37 * 25 + (48 / 2) + (40 / 2) * 37] = SwapLE16((3 << 8) | (3 << 10));
		lm[2 + 37 * 25 + (50 / 2) + (38 / 2) * 37] = SwapLE16(3 << 12);
		lm[2 + 37 * 25 + (48 / 2) + (14 / 2) * 37] = SwapLE16(3 << 12);
		lm[2 + 37 * 25 + (48 / 2) + (16 / 2) * 37] = SwapLE16((3 << 8) | (3 << 12));
		lm[2 + 37 * 25 + (48 / 2) + (18 / 2) * 37] = SwapLE16(3 << 8);
		lm[2 + 37 * 25 + (48 / 2) + (42 / 2) * 37] = SwapLE16(3 << 12);
		lm[2 + 37 * 25 + (48 / 2) + (44 / 2) * 37] = SwapLE16((3 << 8) | (3 << 12));
		// - protect the back-room from additional monsters
		lm[2 + 37 * 25 + ( 2 / 2) + (30 / 2) * 37] = SwapLE16(3 << 14);
		lm[2 + 37 * 25 + ( 2 / 2) + (32 / 2) * 37] = SwapLE16((3 << 10) | (3 << 14));
		lm[2 + 37 * 25 + ( 2 / 2) + (34 / 2) * 37] = SwapLE16((3 << 10) | (3 << 14));
		lm[2 + 37 * 25 + ( 2 / 2) + (36 / 2) * 37] = SwapLE16((3 << 10) | (3 << 14));
		lm[2 + 37 * 25 + ( 4 / 2) + (30 / 2) * 37] = SwapLE16((3 << 12) | (3 << 14));
		lm[2 + 37 * 25 + ( 6 / 2) + (30 / 2) * 37] = SwapLE16((3 << 12) | (3 << 14));
		lm[2 + 37 * 25 + ( 8 / 2) + (30 / 2) * 37] = SwapLE16((3 << 12) | (3 << 14));
		lm[2 + 37 * 25 + (10 / 2) + (32 / 2) * 37] = SwapLE16((3 << 12) | (3 << 14));
		lm[2 + 37 * 25 + (12 / 2) + (32 / 2) * 37] = SwapLE16((3 << 12) | (3 << 14));
		lm[2 + 37 * 25 + (10 / 2) + (34 / 2) * 37] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
		lm[2 + 37 * 25 + (12 / 2) + (34 / 2) * 37] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
		for (int y = 32 / 2; y <= 36 / 2; y++) {
			for (int x = 4 / 2; x <= 8 / 2; x++) {
				lm[2 + 37 * 25 + x + y * 37] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// - add empty space after the grate
		for (int y = 28 / 2; y <= 32 / 2; y++) {
			for (int x = 32 / 2; x <= 32 / 2; x++) {
				lm[2 + 37 * 25 + x + y * 37] = SwapLE16((3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		for (int y = 30 / 2; y <= 32 / 2; y++) {
			for (int x = 34 / 2; x <= 40 / 2; x++) {
				lm[2 + 37 * 25 + x + y * 37] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		for (int y = 28 / 2; y <= 28 / 2; y++) {
			for (int x = 32 / 2; x <= 40 / 2; x++) {
				lm[2 + 37 * 25 + x + y * 37] = SwapLE16((3 << 12) | (3 << 14));
			}
		}
		// protect the changing tiles from trap placement
		lm[2 + 37 * 25 + (22 / 2) + (22 / 2) * 37] = SwapLE16((3 << 8) | (3 << 10));
		lm[2 + 37 * 25 + (46 / 2) + (16 / 2) * 37] = SwapLE16((3 << 8) | (3 << 12));
	}
}
#endif // 0

static void DRLG_L1DrawPreMaps()
{
	for (int i = lengthof(pSetPieces) - 1; i >= 0; i--) {
		if (pSetPieces[i]._sptype == SPT_NONE)
			continue;
		if (setpiecedata[pSetPieces[i]._sptype]._spdPreDunFile != NULL) {
			MemFreeDbg(pSetPieces[i]._spData);
			pSetPieces[i]._spData = LoadFileInMem(setpiecedata[pSetPieces[i]._sptype]._spdPreDunFile);
			// DRLG_L1FixPreMap(i);
			DRLG_DrawMap(i);
		}
	}
}

static void LoadL1Dungeon(const LevelData* lds)
{
	pWarps[DWARP_ENTRY]._wx = lds->dSetLvlDunX;
	pWarps[DWARP_ENTRY]._wy = lds->dSetLvlDunY;
	pWarps[DWARP_ENTRY]._wtype = lds->dSetLvlWarp;

	// load dungeon
	pSetPieces[0]._spx = 0;
	pSetPieces[0]._spy = 0;
	pSetPieces[0]._sptype = lds->dSetLvlPiece;
	pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
	// DRLG_L1FixMap();

	memset(drlgFlags, 0, sizeof(drlgFlags));
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL1DungeonData.");
	memset(dungeon, BASE_MEGATILE_L1, sizeof(dungeon));

	DRLG_LoadSP(0, DEFAULT_MEGATILE_L1);
}

void CreateL1Dungeon()
{
	const LevelData* lds = &AllLevels[currLvl._dLevelNum];

	if (lds->dSetLvl) {
		LoadL1Dungeon(lds);
	} else {
		DRLG_LoadL1SP();
		DRLG_L1();
	}

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		;
	} else
#endif
	{
		DRLG_L1Subs();
		DRLG_L1Floor();
	}

	memcpy(pdungeon, dungeon, sizeof(pdungeon));
	DRLG_L1DrawPreMaps();

	DRLG_L1InitTransVals();
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L1);
}

DEVILUTION_END_NAMESPACE
