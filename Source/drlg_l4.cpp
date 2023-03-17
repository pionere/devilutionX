/**
 * @file drlg_l4.cpp
 *
 * Implementation of the hell level generation algorithms.
 *
 * drlgFlags matrix is used as a BOOLEAN matrix to protect the quest room and the quads in HELL4.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Starting position of the megatiles. */
#define BASE_MEGATILE_L4 (30 - 1)
/** Default megatile if the tile is zero. */
#define DEFAULT_MEGATILE_L4 6

static_assert(DMAXX % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even width.");
#define L4BLOCKX (DMAXX / 2)
static_assert(DMAXY % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even height.");
#define L4BLOCKY (DMAXY / 2)
static_assert(DQUAD_ROOM_SIZE <= L4BLOCKX, "Rooms of diablo-quads must fit to the dungeon blocks of DRLG_L4 I.");
static_assert(DQUAD_ROOM_SIZE <= L4BLOCKY, "Rooms of diablo-quads must fit to the dungeon blocks of DRLG_L4 II.");
static BYTE dungBlock[L4BLOCKX][L4BLOCKY];

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L4ConvTbl[16] = { BASE_MEGATILE_L4 + 1, 6, 1, 6, 2, 6, 6, 6, 9, 6, 1, 6, 2, 6, 3, 6 };

/** Miniset: Stairs up. */
const BYTE L4USTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	 6, 6, 6, 6, // search
	 6, 6, 6, 6,
	 6, 6, 6, 6,
	 6, 6, 6, 6,
	 6, 6, 6, 6,

	 0,  0,  0,  0, // replace
	36, 38, 35,  0,
	37, 34, 33, 32,
	 0,  0, 31,  0,
	 0,  0,  0,  0,
/*    0,  0,     0,  0,     0,  0,     0,  0,	// MegaTiles
	  0,  0,     0,  0,     0,  0,     0,  0,

	102,103,   110,111,    98, 99,     0,  0,
	104,105,   112,113,   100,101,     0,  0,

	106,107,    94, 95,    90, 91,    86, 87,
	108,109,    96, 97,    92, 93,    88, 89 

	  0,  0,     0,  0,    82, 83,     0,  0,
	  0,  0,     0,  0,    84, 85,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
/** Miniset: Stairs up to town. */
const BYTE L4TWARP[] = {
	// clang-format off
	4, 5, // width, height

	6, 6, 6, 6, // search
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,

	  0,   0,   0,   0, // replace
	134, 136, 133,   0,
	135, 132, 131, 130,
	  0,   0, 129,   0,
	  0,   0,   0,   0,
/*    0,  0,     0,  0,     0,  0,     0,  0,	// MegaTiles
	  0,  0,     0,  0,     0,  0,     0,  0,

	441,442,   449,450,   437,438,     0,  0,
	443,444,   451,452,   439,440,     0,  0,

	445,446,   433,434,   429,430,   425,426,
	447,448,   435,436,   431,432,   427,428,

	  0,  0,     0,  0,   421,422,     0,  0,
	  0,  0,     0,  0,   423,424,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
/** Miniset: Stairs down. */
const BYTE L4DSTAIRS[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,  0,  0,  0, 0, // replace
	0,  0, 45, 41, 0,
	0, 44, 43, 40, 0,
	0, 46, 42, 39, 0,
	0,  0,  0,  0, 0,
/*    0,  0,     0,  0,     0,  0,     0,  0,    0,  0,	// MegaTiles
	  0,  0,     0,  0,     0,  0,     0,  0,    0,  0,

	  0,  0,     0,  0,   137,138,   122,123,    0,  0,
	  0,  0,     0,  0,   139,140,   124,125,    0,  0,

	  0,  0,   134,135,   130,131,   118,119,    0,  0,
	  0,  0,    49,136,	  132,133,   120,121,    0,  0,

	  0,  0,   141,142,   126,127,   114,115     0,  0,
	  0,  0,   143,144,   128,129,   116,117     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
/** Miniset: Pentagram. */
const BYTE L4PENTA[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0,  98, 100, 103, 0,
	0,  99, 102, 105, 0,
	0, 101, 104, 106, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};
/** Miniset: Pentagram portal. */
const BYTE L4PENTA2[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0, 107, 109, 112, 0,
	0, 108, 111, 114, 0,
	0, 110, 113, 115, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};

/*
 * Maps tile IDs to their corresponding undecorated tile ID.
 * Values with a single entry are commented out, because pointless to randomize a single option.
 */
const BYTE L4BTYPES[140] = {
	// clang-format off
	0, 1, 2, 0 /*3*/, 4, 5, 6, 7, 0/*8*/, 9,
	0/*10*/, 0/*11*/, 12, 0/*13*/, 0/*14*/, 15, 16, 0/*17*/, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 6,
	6, 6, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 1, 2, 1, 2, 1, 1, 2,
	2, 0, 0, 0, 0, 0, 0, 15, 16, 9,
	12, 4, 5, 7, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	// clang-format on
};

/*
 * Miniset replacement to add shadows.
 * New dungeon values: 47 48
 * TODO: use DRLG_PlaceMiniSet instead?
 */
static void DRLG_L4Shadows()
{
	int x, y;
	BYTE bv;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXY; x++) {
			bv = dungeon[x][y];
			if (bv != 3 && bv != 4 && bv != 8 && bv != 15)
				continue;
			//6, 3/4/8/15,  search

			//47, 0, replace
			if (dungeon[x - 1][y] == 6) {
				dungeon[x - 1][y] = 47;
			}
			//6,  0, search
			//0, 3/4/8/15,

			//48, 0, replace
			// 0, 0,
			if (dungeon[x - 1][y - 1] == 6) {
				dungeon[x - 1][y - 1] = 48;
			}
		}
	}
}

static void DRLG_LoadL4SP()
{
	DRLG_InitSetPC();
	assert(pSetPiece == NULL);
	if (IsMultiGame && QuestStatus(Q_BETRAYER)) {
		pSetPiece = LoadFileInMem("Levels\\L4Data\\Vile1.DUN");
		setpc_type = SPT_BETRAYER;
	} else if (QuestStatus(Q_WARLORD)) {
		pSetPiece = LoadFileInMem("Levels\\L4Data\\Warlord.DUN");
		// ensure the changing tiles are reserved
		pSetPiece[(2 + 7 + 2 * 8) * 2] = DEFAULT_MEGATILE_L4;
		pSetPiece[(2 + 7 + 3 * 8) * 2] = DEFAULT_MEGATILE_L4;
		pSetPiece[(2 + 7 + 4 * 8) * 2] = DEFAULT_MEGATILE_L4;
		setpc_type = SPT_WARLORD;
	}
}

static void DRLG_FreeL4SP()
{
	MemFreeDbg(pSetPiece);
}

static void DRLG_L4SetSPRoom(int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE* sp;

	rw = SwapLE16(*(uint16_t*)&pSetPiece[0]);
	rh = SwapLE16(*(uint16_t*)&pSetPiece[2]);

	// assert(setpc_x == rx1);
	// assert(setpc_y == ry1);
	setpc_w = rw;
	setpc_h = rh;

	sp = &pSetPiece[4];

	rw += rx1;
	rh += ry1;
	for (j = ry1; j < rh; j++) {
		for (i = rx1; i < rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : DEFAULT_MEGATILE_L4;
			drlgFlags[i][j] = *sp != 0 ? TRUE : FALSE; // |= DLRG_PROTECTED;
			sp += 2;
		}
	}
}

/*
 * Transform dungeon by replacing values using 2x2 block patterns defined in L1ConvTbl
 * New dungeon values: 1 2 3 6 9 30
 */
static void DRLG_L4MakeMegas()
{
	int i, j;
	BYTE v;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			assert(dungeon[i][j] <= 1);
			v = dungeon[i][j]
			 | (dungeon[i + 1][j] << 1)
			 | (dungeon[i][j + 1] << 2)
			 | (dungeon[i + 1][j + 1] << 3);
			assert(v != 6 && v != 9);
			dungeon[i][j] = L4ConvTbl[v];
		}
	}
	for (j = 0; j < DMAXY; j++)
		dungeon[DMAXX - 1][j] = BASE_MEGATILE_L4 + 1;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = BASE_MEGATILE_L4 + 1;
}

/*
 * Add horizontal wall.
 *
 * @param i: the x coordinate of the walls first tile
 * @param j: the y coordinate of the walls first tile
 * @param dx: the length of the wall
 * @return true if after the change a check for vertical wall is necessary.
 */
static bool L4HorizWall(int i, int j, int dx)
{
	int xx;
	BYTE bv;
	// convert the internal tiles
	for (xx = 1; xx < dx; xx++) {
		dungeon[i + xx][j] = 2;
	}
	// convert the last tile
	bv = dungeon[i + dx][j];
	switch (bv) {
	case 10:
		bv = 17;
		break;
	case 12:
	case 13:
		break;
	case 15:
		bv = 14;
		break;
	case 16:
		break;
	case 21:
		bv = 23;
		break;
	case 22:
		bv = 29;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	dungeon[i + dx][j] = bv;
	// add 'door'
	xx = RandRange(1, dx - 3);
	dungeon[i + xx][j] = 57;
	dungeon[i + xx + 2][j] = 56;
	dungeon[i + xx + 1][j] = 60;

	if (dungeon[i + xx][j - 1] == 6) {
		dungeon[i + xx][j - 1] = 58;
	}
	if (dungeon[i + xx + 1][j - 1] == 6) {
		dungeon[i + xx + 1][j - 1] = 59;
	}
	// convert the first tile
	if (dungeon[i][j] == 13) {
		dungeon[i][j] = 17;
	} else if (dungeon[i][j] == 16) {
		dungeon[i][j] = 11;
		return true;
	} else if (dungeon[i][j] == 12) {
		dungeon[i][j] = 14;
		return true;
	}
	return false;
}

static constexpr uint32_t HORIZ_WALL_ENDS =
	  (1 << 10)
	| (1 << 12)
	| (1 << 13)
	| (1 << 15)
	| (1 << 16)
	| (1 << 21)
	| (1 << 22);
static bool L4AddHWall(int x, int y)
{
	int i;
	BYTE bv;

	i = x;
	while (TRUE) {
		i++;
		bv = dungeon[i][y];
		if (bv != 6)
			break;
		if (drlgFlags[i][y])
			break;
		if (dungeon[i][y - 1] != 6)
			break;
		if (dungeon[i][y + 1] != 6)
			break;
	}

	i -= x;
	if (i > 3
	// && (bv == 10 || bv == 12 || bv == 13 || bv == 15 || bv == 16 || bv == 21 || bv == 22)) {
	 && (bv < 23 && (HORIZ_WALL_ENDS & (1 << bv)))) {
		return L4HorizWall(x, y, i);
	}

	return false;
}

/*
 * Add vertical wall.
 *
 * @param i: the x coordinate of the walls first tile
 * @param j: the y coordinate of the walls first tile
 * @param dy: the length of the wall
 */
static void L4VertWall(int i, int j, int dy)
{
	int yy;
	BYTE bv;
	// convert the first tile
	if (dungeon[i][j] == 14)
		dungeon[i][j] = 17;
	else if (dungeon[i][j] == 8)
		dungeon[i][j] = 9;
	else if (dungeon[i][j] == 15)
		dungeon[i][j] = 10;
	// convert the internal tiles
	for (yy = 1; yy < dy; yy++) {
		dungeon[i][j + yy] = 1;
	}
	// convert the last tile
	bv = dungeon[i][j + dy];
	switch (bv) {
	case 8:
		break;
	case 9:
		bv = 10;
		break;
	case 11:
		bv = 17;
		break;
	case 14:
	case 15:
		break;
	case 16:
		bv = 13;
		break;
	case 21:
		bv = 22;
		break;
	case 23:
		bv = 29;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	dungeon[i][j + dy] = bv;
	// add 'door'
	yy = RandRange(1, dy - 3);
	dungeon[i][j + yy] = 53;
	dungeon[i][j + yy + 2] = 52;
	dungeon[i][j + yy + 1] = 60;

	if (dungeon[i - 1][j + yy] == 6) {
		dungeon[i - 1][j + yy] = 54;
	}
	if (dungeon[i - 1][j + yy - 1] == 6) {
		dungeon[i - 1][j + yy - 1] = 55;
	}
}

static constexpr uint32_t VERT_WALL_ENDS =
	  (1 << 8)
	| (1 << 9)
	| (1 << 11)
	| (1 << 14)
	| (1 << 15)
	| (1 << 16)
	| (1 << 21)
	| (1 << 23);
static void L4AddVWall(int x, int y)
{
	int j;
	BYTE bv;

	j = y;
	while (TRUE) {
		j++;
		bv = dungeon[x][j];
		if (bv != 6)
			break;
		if (drlgFlags[x][j])
			break;
		if (dungeon[x - 1][j] != 6)
			break;
		if (dungeon[x + 1][j] != 6)
			break;
	}

	j -= y;
	if (j > 3
	 //&& (bv == 8 || bv == 9 || bv == 11 || bv == 14 || bv == 15 || bv == 16 || bv == 21 || bv == 23)) {
	 && (bv < 24 && (VERT_WALL_ENDS & (1 << bv)))) {
		/*return*/ L4VertWall(x, y, j);
	}

	//return false;
}

/*
 * Draw walls between pillars.
 * New dungeon values: 2 11 14 17 23 29 56 57 58 59 60
 *                     1 9 10 13 17 22 29 52 53 54 55 60
 */
static void L4AddWall()
{
	int i, j;
	bool checkVert;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (drlgFlags[i][j]) {
				continue;
			}
			checkVert = false;
			switch (dungeon[i][j]) {
			case 15:
			case 16:
			case 21:
				checkVert = true;
				/* fall-through */
			case 10:
			case 12:
			case 13:
			case 22:
				checkVert |= L4AddHWall(i, j);
				if (!checkVert)
					break;
				/* fall-through */
			case 8:
			case 9:
			case 11:
			case 14:
			case 23:
				L4AddVWall(i, j);
				break;
			}
		}
	}
}

void EnsureSWConnection(int x, int y)
{
	assert(y < DMAXY - 1);
	y++;
	if (dungeon[x][y] == 2) {
		dungeon[x][y] = 14;
	} else if (dungeon[x][y] == 9) {
		dungeon[x][y] = 10;
	} else if (dungeon[x][y] == 11) {
		dungeon[x][y] = 17;
	} else if (dungeon[x][y] == 28) {
		dungeon[x][y] = 25; // new wall type
	} else if (dungeon[x][y] == 30) {
		dungeon[x][y] = 24;
	} /*else {
		assert(dungeon[x][y] == 15 || dungeon[x][y] == 17
			|| dungeon[x][y] == 18 || dungeon[x][y] == 24);
	}*/
}

void EnsureSEConnection(int x, int y)
{
	assert(x < DMAXX - 1);
	x++;
	if (dungeon[x][y] == 1) {
		dungeon[x][y] = 13;
	} else if (dungeon[x][y] == 10) {
		dungeon[x][y] = 17;
	} else if (dungeon[x][y] == 9) {
		dungeon[x][y] = 11;
	} else if (dungeon[x][y] == 27) {
		dungeon[x][y] = 26; // new wall type
	} else if (dungeon[x][y] == 30) {
		dungeon[x][y] = 24;
	} /*else {
		assert(dungeon[x][y] == 16
		 || dungeon[x][y] == 19 || dungeon[x][y] == 24);
	}*/
}

#if DEBUG_MODE
static void ValidateNWConnection(int x, int y, bool full)
{
	assert(x > 0);
	x--;
	if (dungeon[x][y] == 19 || dungeon[x][y] == 21
	 || dungeon[x][y] == 22 || dungeon[x][y] == 23
	 || dungeon[x][y] == 26 || dungeon[x][y] == 27
	 || dungeon[x][y] == 29)
		return;
	assert(full);
	assert(dungeon[x][y] == 2 || dungeon[x][y] == 8
	 || dungeon[x][y] == 9 || dungeon[x][y] == 10
	 || dungeon[x][y] == 11 || dungeon[x][y] == 14
	 || dungeon[x][y] == 15 || dungeon[x][y] == 17);
}

static void ValidateNEConnection(int x, int y, bool full)
{
	assert(y > 0);
	y--;
	if (dungeon[x][y] == 18 || dungeon[x][y] == 21
	 || dungeon[x][y] == 22 || dungeon[x][y] == 23
	 || dungeon[x][y] == 25 || dungeon[x][y] == 28
	 || dungeon[x][y] == 29)
		return;
	assert(full);
	assert(dungeon[x][y] == 1 || dungeon[x][y] == 7
	 || dungeon[x][y] == 9 || dungeon[x][y] == 10
	 || dungeon[x][y] == 11 || dungeon[x][y] == 13
	 || dungeon[x][y] == 16 || dungeon[x][y] == 17);
}

static void ValidateSWConnection(int x, int y)
{
	assert(y < DMAXY - 1);
	y++;
	assert(dungeon[x][y] == 14 || dungeon[x][y] == 10
		 || dungeon[x][y] == 15 || dungeon[x][y] == 17
		 || dungeon[x][y] == 18 || dungeon[x][y] == 24
		 || dungeon[x][y] == 25);
}

static void ValidateSEConnection(int x, int y)
{
	assert(x < DMAXX - 1);
	x++;
	assert(dungeon[x][y] == 11 || dungeon[x][y] == 13
	 || dungeon[x][y] == 16 || dungeon[x][y] == 17
	 || dungeon[x][y] == 19 || dungeon[x][y] == 24
	 || dungeon[x][y] == 26);
}
/* possible configurations:
		[ 1, 6, 9 ]
	[ 2, 6, 30]  	1		[ 6 ]
		[ 1, 2, 3, 6, 9, 30 ]

		[ 1, 6, 30 ]
	[ 2, 6, 9 ]	2		[ 1, 2, 3, 6, 9, 30 ]
		[ 6 ]

		[ 1, 6, 9 ]
	[ 2, 6, 9 ]	3		[ 6 ]
		[ 6 ]

		[ 1, 6, 30 ]
	[ 2, 6, 30 ]	9		[ 2 3 6 ]
		[ 1, 3, 6 ]

		[ 1, 6, 30 ]
	[ 2, 6, 30 ]	30		 [ 1, 9, 30 ]
		[ 2, 9, 30 ]

		[ 1, 2, 3, 6, 9 ]
	[ 1, 2, 3, 6, 9 ] 6 	[ 1, 2, 3, 6, 9, 30 ]
		[ 1, 2, 3, 6, 9, 30 ]
*/
#endif

/*
 * Draw wall around the tiles selected by L4FirstRoom (and L4ConnectBlock).
 * Assumes the border of dungeon was empty.
 * New dungeon values: 4 .. 29 except 20
 */
static void L4TileFix()
{
	int i, j;

	// convert tiles based on the tile above and left from them.
	// Connect, add or cut walls.
	for (i = DMAXX - 1; i > 0; i--) {
		for (j = DMAXY - 1; j > 0; j--) {
			switch (dungeon[i][j]) {
			case 1:
				//		[ 6(L2:16, L6:7, L30: 16) ]
				// [ 2(13) ]  	1
				if (dungeon[i][j - 1] == 6) {
					if (dungeon[i - 1][j] == 2 || dungeon[i - 1][j] == 30) {
						dungeon[i][j] = 16; // connect + cut || new wall (NW) + cut
					} else {
						assert(dungeon[i - 1][j] == 6);
						dungeon[i][j] = 7; // cut
					}
				} else {
					if (dungeon[i - 1][j] == 2) {
						dungeon[i][j] = 13; // connect
					}
				}
				break;
			case 2:
			//													[ 1(14) ]
			// [ 6(T1:15, T6:8, T30:15) ]	2
				if (dungeon[i - 1][j] == 6) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 15; // connect + cut
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 8; // cut
					} else {
						assert(dungeon[i][j - 1] == 30);
						dungeon[i][j] = 15; // new wall (NE) + cut
					}
				} else {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 14; // connect
					}
				}
				break;
			case 3:
			case 6:
				//				[ 1(4), 9(4) ]
				// [ 2(5), 9(5) ]	[3, 6 ]
				if (dungeon[i - 1][j] == 2 || dungeon[i - 1][j] == 9) {
					if (dungeon[i][j - 1] == 1 || dungeon[i][j - 1] == 9) {
						dungeon[i][j] = 12; // connect
					} else {
						dungeon[i][j] = 5; // connect
					}
				} else {
					if (dungeon[i][j - 1] == 1 || dungeon[i][j - 1] == 9) {
						dungeon[i][j] = 4; // connect
					}
				}
				break;
			case 9:
				//			[ 1(10) ]
				// [ 2(11) ]	9
				if (dungeon[i - 1][j] == 2) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 17; // connect
					} else {
						dungeon[i][j] = 11; // connect
					}
				} else {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 10; // connect
					}
				}
				break;
			case 30:
				//			[ 1(27), 6(19) ]
				// [ 2(28), 6(18) ]	30
				if (dungeon[i - 1][j] == 2) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 29; // connect + new wall (SW, SE)
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 23; // connect + new wall (SW, SE)
					} else {
						assert(dungeon[i][j - 1] == 30);
						dungeon[i][j] = 28; // connect + new wall (SW)
					}
				} else if (dungeon[i - 1][j] == 6) {
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 22; // connect + new wall (SW, SE)
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 21; // new wall (SW, SE)
					} else {
						assert(dungeon[i][j - 1] == 30);
						dungeon[i][j] = 18; // new wall (SW, NE)
					}
				} else {
					assert(dungeon[i - 1][j] == 30);
					if (dungeon[i][j - 1] == 1) {
						dungeon[i][j] = 27; // connect + new wall (SE)
					} else if (dungeon[i][j - 1] == 6) {
						dungeon[i][j] = 19; // new wall (NW, SE)
					} else {
						assert(dungeon[i][j - 1] == 30);
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
		if (dungeon[i][0] == 30) {
			// [ 2(28) ]	30
			if (dungeon[i - 1][0] == 2) {
				dungeon[i][0] = 28; // connect + new wall (SW)
			}
		} else if (dungeon[i][0] == 9) {
			// [ 2(11) ]	9
			if (dungeon[i - 1][0] == 2) {
				dungeon[i][0] = 11; // connect
			}
		}
	}
	for (j = DMAXY - 1; j > 0; j--) {
		if (dungeon[0][j] == 30) {
			// [ 1(27) ]
			//	30
			if (dungeon[0][j - 1] == 1) {
				dungeon[0][j] = 27; // connect + new wall (SE)
			} else {
				assert(dungeon[0][j - 1] == 30);
			}
		} else if (dungeon[0][j] == 9) {
			// [ 1(10) ]
			//	9
			if (dungeon[0][j - 1] == 1) {
				dungeon[0][j] = 10; // connect
			}
		}
	}

	// connect the new walls
	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			switch (dungeon[i][j]) {
			case 19: // new wall (NW, SE) 30
				//assert(j > 0);
				//assert(dungeon[i][j - 1] == 6);
				// check SE (29-SE)
				EnsureSEConnection(i, j);
				break;
			case 18: // new wall (SW, NE) 30
			case 28: // new wall (SW) 30
			case 25: // new wall type (SW) 30
				// check SW (21-SW)
				EnsureSWConnection(i, j);
				break;
			case 21: // new wall (SW, SE) 30
			case 22: // new wall (SW, SE) 30
			case 23: // new wall (SW, SE) 30
			case 29: // new wall (SW, SE) 30
				// check SW (21-SW)
				EnsureSWConnection(i, j);
				// check SE (29-SE)
				EnsureSEConnection(i, j);
				break;
				break;
			case 26: // new wall type (SE) 30
			case 27: // new wall (SE) 30
				assert(i < DMAXX - 1);
				if (dungeon[i + 1][j] == 9) {
					dungeon[i + 1][j] = 11;
				}
				break;
			}
		}
	}

#if DEBUG_MODE
	for (i = 0; i < DMAXX; i++) {
		for (j = 0; j < DMAXY; j++) {
			switch (dungeon[i][j]) {
			case 28: // new wall (SW) 30
			case 25: // new wall type (SW) 30
				// check SW (21-SW)
				ValidateSWConnection(i, j);
				break;
			case 29: // new wall (SW, SE) 30
			case 23: // new wall (SW, SE) 30
			case 22: // new wall (SW, SE) 30
			case 21: // new wall (SW, SE) 30
				// check SW (21-SW)
				ValidateSWConnection(i, j);
				// check SE (29-SE)
				ValidateSEConnection(i, j);
				break;
			case 18: // new wall (SW, NE) 30
				// check SW (21-SW)
				ValidateSWConnection(i, j);
				// check NE
				ValidateNEConnection(i, j, false);
				break;
			case 26: // new wall type (SE) 30
			case 27: // new wall (SE) 30
				assert(i < DMAXX - 1);
				assert(dungeon[i + 1][j] == 11
				 || dungeon[i + 1][j] == 16 || dungeon[i + 1][j] == 19);
				break;
			case 19: // new wall (NW, SE) 30
				assert(j > 0);
				assert(dungeon[i][j - 1] == 6);
				// check SE (29-SE)
				ValidateSEConnection(i, j);
				// check NW
				ValidateNWConnection(i, j, false);
				break;
			case 15: // new wall (NE) 2
				ValidateNEConnection(i, j, true);
				break;
			case 16: // new wall (NW) 1
				ValidateNWConnection(i, j, true);
				break;
			}
		}
	}
#endif
}

/*
 * Replace undecorated tiles with matching decorated tiles.
 * New dungeon values: 51 52 61..70 77..83 95 96 97
 */
static void DRLG_L4Subs()
{
	int x, y, i, rv;
	BYTE c;

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (random_(0, 3) == 0) {
				c = L4BTYPES[dungeon[x][y]];
				if (c != 0 && !drlgFlags[x][y]) {
					rv = random_(0, 16);
					i = -1;
					while (rv >= 0) {
						i++;
						if (i == sizeof(L4BTYPES)) {
							i = 0;
						}
						if (c == L4BTYPES[i]) {
							rv--;
						}
					}
					dungeon[x][y] = i;
				}
			}
		}
	}
	// TODO: second round of replacement? why not merge with the first one?
	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (random_(0, 10) == 0) {
				if (L4BTYPES[dungeon[x][y]] == 6 && !drlgFlags[x][y]) {
					dungeon[x][y] = RandRange(95, 97);
				}
			}
		}
	}
}

/** Fill dungeon based on the dungBlock matrix. */
static void L4Block2Dungeon()
{
	int i, j;
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i][j] = dungBlock[i][j];
		}
	}
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i][j + L4BLOCKY] = dungBlock[i][L4BLOCKY - 1 - j];
		}
	}
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i + L4BLOCKX][j] = dungBlock[L4BLOCKX - 1 - i][j];
		}
	}
	for (j = 0; j < L4BLOCKY; j++) {
		for (i = 0; i < L4BLOCKX; i++) {
			dungeon[i + L4BLOCKX][j + L4BLOCKY] = dungBlock[L4BLOCKX - 1 - i][L4BLOCKY - 1 - j];
		}
	}
}

/*
 * Create link between the quarters (blocks) of the dungeon.
 * Assumes the border of dungBlock to be empty.
 */
static void L4ConnectBlock()
{
	int j, i, rv;
	BYTE hallok[std::max(L4BLOCKX, L4BLOCKY)];

	memset(hallok, 0, sizeof(hallok));
	for (j = L4BLOCKY - 2; j >= 0; j--) {
		for (i = L4BLOCKX - 2; i >= 0; i--) {
			if (dungBlock[i][j] == 1) {
				assert(i + 1 < L4BLOCKX && j + 1 < L4BLOCKY);
				if (dungBlock[i][j + 1] == 1 && dungBlock[i + 1][j + 1] == 0) {
					hallok[j] = i;
				}
				i = 0;
			}
		}
	}

	rv = RandRange(1, L4BLOCKY - 1);
	while (TRUE) {
		if (hallok[rv] != 0) {
			for (i = L4BLOCKX - 1; i > hallok[rv]; i--) {
				dungBlock[i][rv] = 1;
				dungBlock[i][rv + 1] = 1;
			}
			break;
		} else {
			rv++;
			if (rv == L4BLOCKY) {
				rv = 1;
			}
		}
	}

	memset(hallok, 0, sizeof(hallok));
	for (i = L4BLOCKX - 2; i >= 0; i--) {
		for (j = L4BLOCKY - 2; j >= 0; j--) {
			if (dungBlock[i][j] == 1) {
				assert(i + 1 < L4BLOCKX && j + 1 < L4BLOCKY);
				if (dungBlock[i + 1][j] == 1 && dungBlock[i + 1][j + 1] == 0) {
					hallok[i] = j;
				}
				j = 0;
			}
		}
	}

	rv = RandRange(1, L4BLOCKX - 1);
	while (TRUE) {
		if (hallok[rv] != 0) {
			for (j = L4BLOCKY - 1; j > hallok[rv]; j--) {
				dungBlock[rv][j] = 1;
				dungBlock[rv + 1][j] = 1;
			}
			break;
		} else {
			rv++;
			if (rv == L4BLOCKX) {
				rv = 1;
			}
		}
	}
}

static int GetArea()
{
	int i, rv;
	BYTE* pTmp;

	rv = 0;
	static_assert(sizeof(dungBlock) == L4BLOCKX * L4BLOCKY, "Linear traverse of dungBlock does not work in GetArea.");
	pTmp = &dungBlock[0][0];
	for (i = 0; i < L4BLOCKX * L4BLOCKY; i++, pTmp++) {
		assert(*pTmp <= 1);
		rv += *pTmp;
	}

	return rv;
}

static void L4DrawRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	x2 = x + width;
	y2 = y + height;
	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			dungBlock[i][j] = 1;
		}
	}
}

static bool L4CheckRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	if (x < 0 || y < 0)
		return false;

	x2 = x + width;
	y2 = y + height;
	if (x2 > L4BLOCKX || y2 > L4BLOCKY)
		return false;

	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			if (dungBlock[i][j] != 0) {
				return false;
			}
		}
	}

	return true;
}

/* L4CheckVHall and L4CheckHHall
 *  make sure there is at least one empty block between the rooms
 */
static bool L4CheckVHall(int x, int top, int h)
{
	int j = top, bottom;

	//assert((unsigned)x < L4BLOCKX);
	if (j < 0)
		return false;
	bottom = j + h;
	if (bottom >= L4BLOCKY)
		return false;
	while (j < bottom && dungBlock[x][j] == 0)
		j++;
	while (j < bottom && dungBlock[x][j] == 1)
		j++;
	while (j < bottom && dungBlock[x][j] == 0)
		j++;
	return j == bottom;
}

static bool L4CheckHHall(int y, int left, int w)
{
	int i = left, right;

	//assert((unsigned)y < L4BLOCKY);
	if (i < 0)
		return false;
	right = i + w;
	if (right >= L4BLOCKX)
		return false;
	while (i < right && dungBlock[i][y] == 0)
		i++;
	while (i < right && dungBlock[i][y] == 1)
		i++;
	while (i < right && dungBlock[i][y] == 0)
		i++;
	return i == right;
}

static void L4RoomGen(int x, int y, int w, int h, bool dir)
{
	int dirProb, i, width, height, rx, ry, rxy2;
	bool ran2;

	dirProb = random_(0, 4);

	if (dir == (dirProb == 0)) {
		// try to place a room to the left
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2 + y - height / 2;
			rx = x - width;
			if (L4CheckVHall(x, ry - 1, height + 2)
			 && L4CheckRoom(rx - 1, ry - 1, width + 1, height + 2))  /// BUGFIX: swap args 3 and 4 ("ch+2" and "cw+1") (fixed)
				break;
		}

		if (i != 0)
			L4DrawRoom(rx, ry, width, height);
		// try to place a room to the right
		rxy2 = x + w;
		ran2 = L4CheckVHall(rxy2 - 1, ry - 1, height + 2)
			&& L4CheckRoom(rxy2, ry - 1, width + 1, height + 2);
		if (ran2)
			L4DrawRoom(rxy2, ry, width, height);
		// proceed with the placed a room on the left
		if (i != 0)
			L4RoomGen(rx, ry, width, height, true);
		// proceed with the placed a room on the right
		if (ran2)
			L4RoomGen(rxy2, ry, width, height, true);
	} else {
		// try to place a room to the top
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2 + x - width / 2;
			ry = y - height;
			if (L4CheckHHall(y, rx - 1, width + 2)
			 && L4CheckRoom(rx - 1, ry - 1, width + 2, height + 1))
				break;
		}

		if (i != 0)
			L4DrawRoom(rx, ry, width, height);
		// try to place a room to the bottom
		rxy2 = y + h;
		ran2 = L4CheckHHall(rxy2 - 1, rx - 1, width + 2)
			&& L4CheckRoom(rx - 1, rxy2, width + 2, height + 1);
		if (ran2)
			L4DrawRoom(rx, rxy2, width, height);
		// proceed with the placed a room on the top
		if (i != 0)
			L4RoomGen(rx, ry, width, height, false);
		// proceed with the placed a room on the bottom
		if (ran2)
			L4RoomGen(rx, rxy2, width, height, false);
	}
}

/*
 * Create dungeon blueprint, but leave the border of dungBlock empty.
 * New dungeon (dungBlock) values: 1
 */
static void L4FirstRoom()
{
	int x, y, w, h, xmin, xmax, ymin, ymax;

	if (currLvl._dLevelIdx != DLV_HELL4) {
		if (pSetPiece != NULL) {
			w = SwapLE16(*(uint16_t*)&pSetPiece[0]) + 4; // TODO: add border to the setmaps
			h = SwapLE16(*(uint16_t*)&pSetPiece[0]) + 2;
			if (setpc_type == SPT_WARLORD)
				w--;
		} else {
			w = RandRange(2, 6) & ~1;
			h = RandRange(2, 6) & ~1;
		}
	} else {
		w = DQUAD_ROOM_SIZE;
		h = DQUAD_ROOM_SIZE;
	}

	xmax = L4BLOCKX - 1 - w;
	xmin = (xmax + 1) >> 1;
	x = RandRange(xmin, xmax);

	ymax = L4BLOCKY - 1 - h;
	ymin = (ymax + 1) >> 1;
	y = RandRange(ymin, ymax);

	if (currLvl._dLevelIdx == DLV_HELL4) {
		setpc_x = x + 1;
		setpc_y = y + 1;
	}
	if (pSetPiece != NULL) {
		setpc_x = x + 1;
		setpc_y = y + 1;
	}

	L4DrawRoom(x, y, w, h);
	static_assert((int)true == 1, "Bool to int conversion in L4FirstRoom.");
	L4RoomGen(x, y, w, h, random_(0, 2));
}

/*static void L4SaveQuads()
{
	int i, j, x, y;

	x = setpc_x - 1;
	y = setpc_y - 1;

	for (j = y; j < y + DQUAD_ROOM_SIZE; j++) {
		for (i = x; i < x + DQUAD_ROOM_SIZE; i++) {
			drlgFlags[i][j] = TRUE;
			drlgFlags[DMAXX - 1 - i][j] = TRUE;
			drlgFlags[i][DMAXY - 1 - j] = TRUE;
			drlgFlags[DMAXX - 1 - i][DMAXY - 1 - j] = TRUE;
		}
	}
}*/

static void DRLG_L4SetRoom(int rx1, int ry1)
{
	int rx2, ry2, i, j;
	BYTE* sp;

	rx2 = rx1 + SwapLE16(*(uint16_t*)&pSetPiece[0]);
	ry2 = ry1 + SwapLE16(*(uint16_t*)&pSetPiece[2]);
	sp = &pSetPiece[4];

	for (j = ry1; j < ry2; j++) {
		for (i = rx1; i < rx2; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : DEFAULT_MEGATILE_L4;
			drlgFlags[i][j] = TRUE; // |= DLRG_PROTECTED;
			sp += 2;
		}
	}
}

static void DRLG_LoadDiabQuads(bool preflag)
{
	assert(pSetPiece == NULL);

	pSetPiece = LoadFileInMem("Levels\\L4Data\\diab1.DUN");
	DRLG_L4SetRoom(DIAB_QUAD_1X, DIAB_QUAD_1Y);
	MemFreeDbg(pSetPiece);

	pSetPiece = LoadFileInMem(preflag ? "Levels\\L4Data\\diab2b.DUN" : "Levels\\L4Data\\diab2a.DUN");
	DRLG_L4SetRoom(DIAB_QUAD_2X, DIAB_QUAD_2Y);
	MemFreeDbg(pSetPiece);

	pSetPiece = LoadFileInMem(preflag ? "Levels\\L4Data\\diab3b.DUN" : "Levels\\L4Data\\diab3a.DUN");
	DRLG_L4SetRoom(DIAB_QUAD_3X, DIAB_QUAD_3Y);
	MemFreeDbg(pSetPiece);

	pSetPiece = LoadFileInMem(preflag ? "Levels\\L4Data\\diab4b.DUN" : "Levels\\L4Data\\diab4a.DUN");
	DRLG_L4SetRoom(DIAB_QUAD_4X, DIAB_QUAD_4Y);
	MemFreeDbg(pSetPiece);
}

/*
 * Spread transVals further.
 * - spread transVals on corner tiles to make transparency smoother.
 * - spread transVals between 'rooms' where the special 'door'-tiles stop the standard spread.
 */
static void DRLG_L4TransFix()
{
	int i, j, xx, yy;
	BYTE tv;

	yy = DBORDERY;
	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			//* commented out because a simplified version is added below
			/*case 23:
			case 25:
			case 28:
				if (dungeon[i][j - 1] == 18) {
					DRLG_CopyTrans(xx, yy, xx + 1, yy);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 22:
			case 26:
			case 27:
				if (dungeon[i + 1][j] == 19) {
					DRLG_CopyTrans(xx, yy, xx, yy + 1);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 18:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 19:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;*/
			// fix transVals of corners
			case 24:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 18:
			case 25:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 19:
			case 26:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				//DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			// fix transVals of 'doors'
			case 52:
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy);
				break;
			case 53:
				DRLG_CopyTrans(xx + 1, yy, xx, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy + 1);
				break;
			case 56:
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy);
				break;
			case 57:
				DRLG_CopyTrans(xx, yy + 1, xx + 1, yy);
				break;
			case 60:
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx + 1, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx, yy + 1);
				break;
			case 54:
				tv = dTransVal[xx][yy];
				if (tv != 0 && dTransVal[xx][yy + 1] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx + 1][yy] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the south side - in case the tile is in the corner
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					// fix the wall on the north side - in case 55 is not placed
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					break;
				}
				tv = dTransVal[xx][yy + 1];
				if (tv != 0) {
					// there is a valid tv on the south side - use it
					dTransVal[xx][yy] = tv;
					dTransVal[xx + 1][yy] = tv;
					// fix the wall on the west side
					if (dTransVal[xx - 1][yy] == 0)
						dTransVal[xx - 1][yy] = tv;
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					// fix the wall on the east side
					if (dTransVal[xx + 2][yy] == 0)
						dTransVal[xx + 2][yy] = tv;
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					// fix the wall on the north side - in case 55 is not placed
					if (dTransVal[xx][yy - 1] == 0)
						dTransVal[xx][yy - 1] = tv;
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					break;
				}
				// no valid tv on the south side -> there must be a one on the north side
				tv = dTransVal[xx][yy - 3];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the south side
				assert(dTransVal[xx][yy + 2] == 0);
				dTransVal[xx][yy + 2] = tv;
				assert(dTransVal[xx + 1][yy + 2] == 0);
				dTransVal[xx + 1][yy + 2] = tv;
				// fix the wall on the west side
				if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				// fix the corner on south-west
				if (dTransVal[xx - 1][yy + 2] == 0)
					dTransVal[xx - 1][yy + 2] = tv;
				// fix the wall on the east side
				if (dTransVal[xx + 2][yy] == 0)
					dTransVal[xx + 2][yy] = tv;
				if (dTransVal[xx + 2][yy + 1] == 0)
					dTransVal[xx + 2][yy + 1] = tv;
				// fix the corner on south-east
				if (dTransVal[xx + 2][yy + 2] == 0)
					dTransVal[xx + 2][yy + 2] = tv;
				break;
			case 55:
				tv = dTransVal[xx][yy + 1];
				if (tv != 0 && dTransVal[xx][yy] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx + 1][yy] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the north side - in case the tile is in the corner
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					// fix the wall on the south side - in case 54 is not placed
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					break;
				}
				tv = dTransVal[xx][yy];
				if (tv != 0) {
					// there is a valid tv on the north side - use it
					dTransVal[xx][yy + 1] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the west side
					if (dTransVal[xx - 1][yy] == 0)
						dTransVal[xx - 1][yy] = tv;
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					// fix the wall on the east side
					if (dTransVal[xx + 2][yy] == 0)
						dTransVal[xx + 2][yy] = tv;
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					// fix the wall on the south side - in case 54 is not placed
					if (dTransVal[xx][yy + 2] == 0)
						dTransVal[xx][yy + 2] = tv;
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					break;
				}
				// no valid tv on the north side -> there must be a one on the south side
				tv = dTransVal[xx][yy + 4];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the north side
				assert(dTransVal[xx][yy - 1] == 0);
				dTransVal[xx][yy - 1] = tv;
				assert(dTransVal[xx + 1][yy - 1] == 0);
				dTransVal[xx + 1][yy - 1] = tv;
				// fix the wall on the west side
				if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				// fix the corner on north-west
				if (dTransVal[xx - 1][yy - 1] == 0)
					dTransVal[xx - 1][yy - 1] = tv;
				// fix the wall on the east side
				if (dTransVal[xx + 2][yy] == 0)
					dTransVal[xx + 2][yy] = tv;
				if (dTransVal[xx + 2][yy + 1] == 0)
					dTransVal[xx + 2][yy + 1] = tv;
				// fix the corner on north-east
				if (dTransVal[xx + 2][yy - 1] == 0)
					dTransVal[xx + 2][yy - 1] = tv;
				break;
			case 58:
				tv = dTransVal[xx + 1][yy];
				if (tv != 0 && dTransVal[xx][yy] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx][yy + 1] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the west side - in case the tile is in the corner
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					// fix the wall on the east side - in case 59 is not placed
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					break;
				}
				tv = dTransVal[xx][yy];
				if (tv != 0) {
					// there is a valid tv on the west side - use it
					dTransVal[xx + 1][yy] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the north side
					if (dTransVal[xx][yy - 1] == 0)
						dTransVal[xx][yy - 1] = tv;
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					// fix the wall on the south side
					if (dTransVal[xx][yy + 2] == 0)
						dTransVal[xx][yy + 2] = tv;
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					// fix the wall on the east side - in case 59 is not placed
					if (dTransVal[xx + 2][yy] == 0)
						dTransVal[xx + 2][yy] = tv;
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					break;
				}
				// no valid tv on the west side -> there must be a one on the east side
				tv = dTransVal[xx + 4][yy];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the west side
				assert(dTransVal[xx - 1][yy] == 0);
				dTransVal[xx - 1][yy] = tv;
				assert(dTransVal[xx - 1][yy + 1] == 0);
				dTransVal[xx - 1][yy + 1] = tv;
				// fix the wall on the north side
				if (dTransVal[xx][yy - 1] == 0)
					dTransVal[xx][yy - 1] = tv;
				if (dTransVal[xx + 1][yy - 1] == 0)
					dTransVal[xx + 1][yy - 1] = tv;
				// fix the corner on north-west
				if (dTransVal[xx - 1][yy - 1] == 0)
					dTransVal[xx - 1][yy - 1] = tv;
				// fix the wall on the south side
				if (dTransVal[xx][yy + 2] == 0)
					dTransVal[xx][yy + 2] = tv;
				if (dTransVal[xx + 1][yy + 2] == 0)
					dTransVal[xx + 1][yy + 2] = tv;
				// fix the corner on south-west
				if (dTransVal[xx - 1][yy + 2] == 0)
					dTransVal[xx - 1][yy + 2] = tv;
				break;
			case 59:
				tv = dTransVal[xx][yy];
				if (tv != 0 && dTransVal[xx + 1][yy] == tv) {
					// the tile has a valid tv - use it
					dTransVal[xx][yy + 1] = tv;
					dTransVal[xx + 1][yy + 1] = tv;
					// fix the wall on the east side - in case the tile is in the corner
					if (dTransVal[xx + 2][yy + 1] == 0)
						dTransVal[xx + 2][yy + 1] = tv;
					// fix the wall on the west side - in case 58 is not placed
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					break;
				}
				tv = dTransVal[xx + 1][yy];
				if (tv != 0) {
					// there is a valid tv on the east side - use it
					dTransVal[xx][yy] = tv;
					dTransVal[xx][yy + 1] = tv;
					// fix the wall on the north side
					if (dTransVal[xx][yy - 1] == 0)
						dTransVal[xx][yy - 1] = tv;
					if (dTransVal[xx + 1][yy - 1] == 0)
						dTransVal[xx + 1][yy - 1] = tv;
					// fix the wall on the south side
					if (dTransVal[xx][yy + 2] == 0)
						dTransVal[xx][yy + 2] = tv;
					if (dTransVal[xx + 1][yy + 2] == 0)
						dTransVal[xx + 1][yy + 2] = tv;
					// fix the wall on the west side - in case 58 is not placed
					if (dTransVal[xx - 1][yy] == 0)
						dTransVal[xx - 1][yy] = tv;
					if (dTransVal[xx - 1][yy + 1] == 0)
						dTransVal[xx - 1][yy + 1] = tv;
					break;
				}
				// no valid tv on the east side -> there must be a one on the west side
				tv = dTransVal[xx - 3][yy];
				assert(tv != 0);
				// use it on the current tile
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the east side
				assert(dTransVal[xx + 2][yy] == 0);
				dTransVal[xx + 2][yy] = tv;
				assert(dTransVal[xx + 2][yy + 1] == 0);
				dTransVal[xx + 2][yy + 1] = tv;
				// fix the wall on the north side
				if (dTransVal[xx][yy - 1] == 0)
					dTransVal[xx][yy - 1] = tv;
				if (dTransVal[xx + 1][yy - 1] == 0)
					dTransVal[xx + 1][yy - 1] = tv;
				// fix the corner on north-east
				if (dTransVal[xx + 2][yy - 1] == 0)
					dTransVal[xx + 2][yy - 1] = tv;
				// fix the wall on the south side
				if (dTransVal[xx][yy + 2] == 0)
					dTransVal[xx][yy + 2] = tv;
				if (dTransVal[xx + 1][yy + 2] == 0)
					dTransVal[xx + 1][yy + 2] = tv;
				// fix the corner on south-east
				if (dTransVal[xx + 1][yy + 2] == 0)
					dTransVal[xx + 1][yy + 2] = tv;
				break;
			// fix transVals around the stairs - necessary only if DRLG_FloodTVal is run after the placement
			// - due to complex cases with the horizontal/vertical 'doors', this is not really feasible
			/*case 36:
			case 134:
				tv = dTransVal[xx][yy];
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				// fix the wall on the west side
				//if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				//if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				break;
			case 37:
			case 135:
				tv = dTransVal[xx][yy + 1];
				dTransVal[xx][yy] = tv;
				dTransVal[xx + 1][yy] = tv;
				// fix the wall on the west side
				//if (dTransVal[xx - 1][yy] == 0)
					dTransVal[xx - 1][yy] = tv;
				//if (dTransVal[xx - 1][yy + 1] == 0)
					dTransVal[xx - 1][yy + 1] = tv;
				break;
			case 38:
			case 136:
				tv = dTransVal[xx][yy];
				dTransVal[xx][yy + 1] = tv;
				dTransVal[xx + 1][yy + 1] = tv;
				break;
			case 32:
			case 130:
				tv = dTransVal[xx][yy];
				// fix the wall on the east side
				//if (dTransVal[xx + 3][yy] == 0)
					dTransVal[xx + 3][yy] = tv;
				//if (dTransVal[xx + 3][yy + 1] == 0)
					dTransVal[xx + 3][yy + 1] = tv;
				break;*/
			}
			xx += 2;
		}
		yy += 2;
	}

	/* fix transVals of the diablo-level
	   - commented out because it does not really matter. The tiles with zero value are contained
	     in the quads. the few random monsters are not bothered by them.
	if (currLvl._dLevelIdx == DLV_HELL4) {
		xx = DBORDERX + 2 * DIAB_QUAD_1X;
		yy = DBORDERY + 2 * DIAB_QUAD_1Y;
		tv = dTransVal[xx][yy];
		assert(tv != 0);
		// middle tiles of the 1. quad
		const POS32 mm_offs[] = {
			{  1,  9 }, {  1, 10 },
			{  3,  9 }, {  4,  9 }, {  9,  3 },
		};
		for (i = 0; i < lengthof(mm_offs); i++)
			dTransVal[xx + mm_offs[i].x][yy + mm_offs[i].y] = tv;

		xx = DBORDERX + 2 * DIAB_QUAD_2X;
		yy = DBORDERY + 2 * DIAB_QUAD_2Y;
		tv = dTransVal[xx][yy];
		assert(tv != 0);
		// many tiles of the 2. quad
		for (i = xx; i <= xx + 22; i++) {
			for (j = yy; j <= yy + 22; j++) {
				if (dTransVal[i][j] == 0)
					dTransVal[i][j] = tv;
			}
		}

		// outer tiles of the 3. quad
		xx = DBORDERX + 2 * DIAB_QUAD_3X;
		yy = DBORDERY + 2 * DIAB_QUAD_3Y;
		tv = dTransVal[xx][yy];
		assert(tv != 0);
		const POS32 oo_offs[] = {
			{  1,  1 }, {  1,  2 }, {  1,  3 },
			{  2,  1 }, {  2,  2 }, {  2,  3 },
			{  3,  1 }, {  3,  2 }, {  3,  3 },

			{  1, 17 }, {  1, 18 }, {  1, 19 }, {  1, 20 },
			{  2, 17 }, {  2, 18 }, {  2, 19 }, {  2, 20 },
			{  3, 17 }, {  3, 18 }, {  3, 19 }, {  3, 20 },

			{ 17,  1 }, { 17,  2 }, { 17,  3 },
			{ 18,  1 }, { 18,  2 }, { 18,  3 },
			{ 19,  1 }, { 19,  2 }, { 19,  3 },
			{ 20,  1 }, { 20,  2 }, { 20,  3 },
		
			{ 17, 17 }, { 17, 18 }, { 17, 19 }, { 17, 20 },
			{ 18, 17 }, { 18, 18 }, { 18, 19 }, { 18, 20 },
			{ 19, 17 }, { 19, 18 }, { 19, 19 }, { 19, 20 },
			{ 20, 17 }, { 20, 18 }, { 20, 19 }, { 20, 20 },
		};

		for (i = 0; i < lengthof(oo_offs); i++)
			dTransVal[xx + oo_offs[i].x][yy + oo_offs[i].y] = tv;

		// inner tiles of the 3. quad
		tv = dTransVal[xx + 6][yy + 6];
		assert(tv != 0);
		const POS32 ii_offs[] = {
			{  1,  5 }, {  1,  6 }, {  1,  7 }, {  1,  8 },
			{  2,  5 }, {  2,  6 }, {  2,  7 }, {  2,  8 },
			{  5,  1 }, {  5,  2 }, {  5,  3 }, {  5,  4 },
			{  6,  1 }, {  6,  2 }, {  6,  3 }, {  6,  4 },
			{  7, 11 }, {  7, 12 }, 

			{ 13,  9 }, { 13, 10 }, { 13, 11 }, { 13, 12 },
			{ 14,  9 }, { 14, 10 }, { 14, 11 }, { 14, 12 },

			{ 15,  1 }, { 15,  2 }, { 15,  3 }, { 15,  4 },
			{ 19,  5 }, { 19,  6 },
		};
		for (i = 0; i < lengthof(ii_offs); i++)
			dTransVal[xx + ii_offs[i].x][yy + ii_offs[i].y] = tv;

		xx = DBORDERX + 2 * DIAB_QUAD_4X;
		yy = DBORDERY + 2 * DIAB_QUAD_4Y;
		tv = dTransVal[xx][yy];
		assert(tv != 0);
		// inner tiles of the 4. quad
		for (i = xx; i <= xx + 16; i++) {
			for (j = yy; j <= yy + 16; j++) {
				if (dTransVal[i][j] == 0)
					dTransVal[i][j] = tv;
			}
		}
	}*/
}

/*
 * Replace tiles with complete ones to hide rendering glitch of transparent corners.
 * New dungeon values: 116..128
 */
static void DRLG_L4Corners()
{
	int i;
	BYTE* pTmp;

	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L4Corners.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		if (*pTmp >= 18 && *pTmp < 30 /*&& *pTmp != 20 && *pTmp != 24*/)
			*pTmp += 98;
	}
}

/*
 * Miniset replacement of corner tiles.
 * New dungeon values: (17)
 * TODO: use DRLG_PlaceMiniSet instead?
 */
static void DRLG_L4GeneralFix()
{
	/* commented out because this is no longer necessary
	int i, j;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			//24/122, 2,  search
			// 5,     0,

			//17, 0, replace
			// 0, 0,
			if ((dungeon[i][j] == 24 || dungeon[i][j] == 122) && dungeon[i + 1][j] == 2 && dungeon[i][j + 1] == 5) {
				dungeon[i][j] = 17;
			}
		}
	}*/
}

struct mini_set {
	const BYTE* data;
	bool setview;
};
static bool DRLG_L4PlaceMiniSets(mini_set* minisets, int n)
{
	int i;
	POS32 mpos;

	for (i = 0; i < n; i++) {
		if (minisets[i].data == NULL)
			continue;
		mpos = DRLG_PlaceMiniSet(minisets[i].data);
		if (mpos.x == DMAXX)
			return false;
		if (minisets[i].setview) {
			ViewX = 2 * mpos.x + DBORDERX + 5;
			ViewY = 2 * mpos.y + DBORDERY + 6;
		}
	}
	return true;
}

static void DRLG_L4(int entry)
{
	int i, j;
	bool doneflag;

	do {
		do {
			memset(dungBlock, 0, sizeof(dungBlock));

			//static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in DRLG_L4.");
			//memset(dungeon, 30, sizeof(dungeon));
			L4FirstRoom();
		} while (GetArea() < 173);
		L4ConnectBlock();

		L4Block2Dungeon();
		DRLG_L4MakeMegas();
		L4TileFix();
		memset(drlgFlags, 0, sizeof(drlgFlags));
		if (pSetPiece != NULL) {
			DRLG_L4SetSPRoom(setpc_x, setpc_y);
		}
		if (currLvl._dLevelIdx == DLV_HELL4) {
			// L4SaveQuads();
			DRLG_LoadDiabQuads(true);
		}
		L4AddWall();
		DRLG_InitTrans();
		DRLG_FloodTVal(6);
		if (setpc_type == SPT_WARLORD) {
			mini_set stairs[2] = {
				{ L4USTAIRS, entry == ENTRY_MAIN },
				{ currLvl._dLevelIdx != DLV_HELL1 ? NULL : L4TWARP, entry != ENTRY_MAIN  && entry != ENTRY_PREV }
			};
			doneflag = DRLG_L4PlaceMiniSets(stairs, 2);
			if (entry == ENTRY_PREV) {
				ViewX = 2 * setpc_x + DBORDERX + 6;
				ViewY = 2 * setpc_y + DBORDERY + 6;
			}
		} else if (currLvl._dLevelIdx != DLV_HELL3) {
			mini_set stairs[3] = {
				{ L4USTAIRS, entry == ENTRY_MAIN },
				{ currLvl._dLevelIdx != DLV_HELL4 ? L4DSTAIRS : NULL, entry == ENTRY_PREV },
				{ currLvl._dLevelIdx != DLV_HELL1 ? NULL : L4TWARP, entry != ENTRY_MAIN  && entry != ENTRY_PREV }
			};
			doneflag = DRLG_L4PlaceMiniSets(stairs, 3);
			if (entry == ENTRY_PREV) {
				ViewX++;
				ViewY -= 2;
			}
		} else {
			mini_set stairs[2] = {
				{ L4USTAIRS, entry == ENTRY_MAIN },
				{ (!IsMultiGame && quests[Q_DIABLO]._qactive != QUEST_ACTIVE) ?
					L4PENTA : L4PENTA2, entry != ENTRY_MAIN }
			};
			doneflag = DRLG_L4PlaceMiniSets(stairs, 2);
			if (entry == ENTRY_MAIN)
				ViewY++;
		}
	} while (!doneflag);

	DRLG_L4GeneralFix();
	DRLG_L4TransFix();

	if (currLvl._dLevelIdx != DLV_HELL4) {
		DRLG_PlaceThemeRooms(7, 10, DEFAULT_MEGATILE_L4, 8, true);
	}

	DRLG_L4Shadows();
	DRLG_L4Corners();
	DRLG_L4Subs();

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	DRLG_Init_Globals();

	if (setpc_type == SPT_WARLORD) {
		DRLG_DrawMap("Levels\\L4Data\\Warlord2.DUN", DEFAULT_MEGATILE_L4);
	}
	if (currLvl._dLevelIdx == DLV_HELL3) {
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++) {
				if (dungeon[i][j] == 98 || dungeon[i][j] == 107) {
					// set the rportal position of Q_BETRAYER and help InitMonsters to find the exit
					quests[Q_BETRAYER]._qtx = 2 * i + DBORDERX;
					quests[Q_BETRAYER]._qty = 2 * j + DBORDERY;
				}
			}
		}
	}
	if (currLvl._dLevelIdx == DLV_HELL4) {
		DRLG_LoadDiabQuads(false);
	}
}

void CreateL4Dungeon(int entry)
{
	DRLG_LoadL4SP();
	DRLG_L4(entry);
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L4);
	DRLG_FreeL4SP();
	DRLG_SetPC();
}

/*static BYTE* LoadL4DungeonData(const char* sFileName)
{
	int rw, rh, i, j;
	BYTE* pMap;
	BYTE* sp;

	pMap = LoadFileInMem(sFileName);

	static_assert(sizeof(dungeon[0][0]) == 1, "memset on dungeon does not work in LoadL4DungeonData.");
	memset(dungeon, BASE_MEGATILE_L4 + 1, sizeof(dungeon));

	rw = SwapLE16(*(uint16_t*)&pMap[0]);
	rh = SwapLE16(*(uint16_t*)&pMap[2]);

	sp = &pMap[4];

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : DEFAULT_MEGATILE_L4;
			// no need to protect the fields, unused on setmaps
			// drlgFlags[i][j] = *sp != 0 ? TRUE : FALSE; // |= DLRG_PROTECTED;
			sp += 2;
		}
	}

	return pMap;
}

void LoadL4Dungeon(const LevelData* lds)
{
	BYTE* pMap;

	ViewX = lds->dSetLvlDunX;
	ViewY = lds->dSetLvlDunY;

	// load pre-dungeon
	pMap = LoadL4DungeonData(lds->dSetLvlPreDun);

	DRLG_InitTrans();
	//DRLG_FloodTVal(6);
	DRLG_SetMapTrans(pMap);

	mem_free_dbg(pMap);

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	// load dungeon
	pMap = LoadL4DungeonData(lds->dSetLvlDun);

	DRLG_Init_Globals();
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L4);

	SetMapMonsters(pMap, 0, 0);
	SetMapObjects(pMap);

	mem_free_dbg(pMap);
}*/

DEVILUTION_END_NAMESPACE
