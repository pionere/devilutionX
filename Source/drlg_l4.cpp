/**
 * @file drlg_l4.cpp
 *
 * Implementation of the hell level generation algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int diabquad1x;
int diabquad1y;
int diabquad2x;
int diabquad2y;
int diabquad3x;
int diabquad3y;
int diabquad4x;
int diabquad4y;
BOOL hallok[20];
int l4holdx;
int l4holdy;
int SP4x1;
int SP4y1;
int SP4x2;
int SP4y2;
BYTE L4dungeon[DSIZEX][DSIZEY];
BYTE dung[20][20];
//int dword_52A4DC;

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L4ConvTbl[16] = { 30, 6, 1, 6, 2, 6, 6, 6, 9, 6, 1, 6, 2, 6, 3, 6 };

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

/** Maps tile IDs to their corresponding undecorated tile ID. */
const BYTE L4BTYPES[140] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
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
};

static void DRLG_L4Shadows()
{
	int x, y;
	BYTE bv;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXY; x++) {
			bv = dungeon[x][y];
			if (bv != 3 && bv != 4 && bv != 8 && bv != 15)
				continue;
			if (dungeon[x - 1][y] == 6) {
				dungeon[x - 1][y] = 47;
			}
			if (dungeon[x - 1][y - 1] == 6) {
				dungeon[x - 1][y - 1] = 48;
			}
		}
	}
}

static void InitL4Dungeon()
{
	int i, j;

	memset(dung, 0, sizeof(dung));
	memset(L4dungeon, 0, sizeof(L4dungeon));
	memset(dflags, 0, sizeof(dflags));

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			dungeon[i][j] = 30;
		}
	}
}

static void DRLG_LoadL4SP()
{
	setloadflag = FALSE;
	if (currlevel == 15 && gbMaxPlayers != 1) {
		pSetPiece = LoadFileInMem("Levels\\L4Data\\Vile1.DUN", NULL);
		setloadflag = TRUE;
	} else if (QuestStatus(Q_WARLORD)) {
		pSetPiece = LoadFileInMem("Levels\\L4Data\\Warlord.DUN", NULL);
		setloadflag = TRUE;
	}
}

static void DRLG_FreeL4SP()
{
	MemFreeDbg(pSetPiece);
}

static void DRLG_L4SetSPRoom(int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE *sp;

	rw = pSetPiece[0];
	rh = pSetPiece[2];

	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;

	sp = &pSetPiece[4];

	rw += rx1;
	rh += ry1;
	for (j = ry1; j < rh; j++) {
		for (i = rx1; i < rw; i++) {
			if (*sp != 0) {
				dungeon[i][j] = *sp;
				dflags[i][j] |= DLRG_PROTECTED;
			} else {
				dungeon[i][j] = 6;
			}
			sp += 2;
		}
	}
}

static void L4makeDmt()
{
	int i, j, idx, val, dmtx, dmty;

	for (j = 0, dmty = 1; dmty <= DSIZEY - 3; j++, dmty += 2) {
		for (i = 0, dmtx = 1; dmtx <= DSIZEX - 3; i++, dmtx += 2) {
			val = 8 * L4dungeon[dmtx + 1][dmty + 1]
				+ 4 * L4dungeon[dmtx][dmty + 1]
				+ 2 * L4dungeon[dmtx + 1][dmty]
				+ L4dungeon[dmtx][dmty];
			idx = L4ConvTbl[val];
			dungeon[i][j] = idx;
		}
	}
}

static int L4HWallOk(int x, int y)
{
	int i;
	BYTE bv;

	i = x;
	while (TRUE) {
		i++;
		bv = dungeon[i][y];
		if (bv != 6)
			break;
		if (dflags[i][y] != 0)
			break;
		if (dungeon[i][y - 1] != 6)
			break;
		if (dungeon[i][y + 1] != 6)
			break;
	}

	i -= x;
	if (i > 3
	 && (bv == 10 || bv == 12 || bv == 13 || bv == 15 || bv == 16 || bv == 21 || bv == 22))
		return i;

	return -1;
}

static int L4VWallOk(int x, int y)
{
	int j;
	BYTE bv;

	j = y;
	while (TRUE) {
		j++;
		bv = dungeon[x][j];
		if (bv != 6)
			break;
		if (dflags[x][j] != 0)
			break;
		if (dungeon[x - 1][j] != 6)
			break;
		if (dungeon[x + 1][j] != 6)
			break;
	}

	j -= y;
	if (j > 3
	 && (bv == 8 || bv == 9 || bv == 11 || bv == 14 || bv == 15 || bv == 16 || bv == 21 || bv == 23))
		return j;

	return -1;
}

static void L4HorizWall(int i, int j, int dx)
{
	int xx;

	if (dungeon[i][j] == 13)
		dungeon[i][j] = 17;
	else if (dungeon[i][j] == 16)
		dungeon[i][j] = 11;
	else if (dungeon[i][j] == 12)
		dungeon[i][j] = 14;

	for (xx = 1; xx < dx; xx++) {
		dungeon[i + xx][j] = 2;
	}

	if (dungeon[i + dx][j] == 15)
		dungeon[i + dx][j] = 14;
	else if (dungeon[i + dx][j] == 10)
		dungeon[i + dx][j] = 17;
	else if (dungeon[i + dx][j] == 21)
		dungeon[i + dx][j] = 23;
	else if (dungeon[i + dx][j] == 22)
		dungeon[i + dx][j] = 29;

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
}

static void L4VertWall(int i, int j, int dy)
{
	int yy;

	if (dungeon[i][j] == 14)
		dungeon[i][j] = 17;
	else if (dungeon[i][j] == 8)
		dungeon[i][j] = 9;
	else if (dungeon[i][j] == 15)
		dungeon[i][j] = 10;

	for (yy = 1; yy < dy; yy++) {
		dungeon[i][j + yy] = 1;
	}

	if (dungeon[i][j + dy] == 11)
		dungeon[i][j + dy] = 17;
	else if (dungeon[i][j + dy] == 9)
		dungeon[i][j + dy] = 10;
	else if (dungeon[i][j + dy] == 16)
		dungeon[i][j + dy] = 13;
	else if (dungeon[i][j + dy] == 21)
		dungeon[i][j + dy] = 22;
	else if (dungeon[i][j + dy] == 23)
		dungeon[i][j + dy] = 29;

	yy = RandRange(1, dy - 3);
	dungeon[i][j + yy] = 53;
	dungeon[i][j + yy + 2] = 52;
	dungeon[i][j + yy + 1] = 6;

	if (dungeon[i - 1][j + yy] == 6) {
		dungeon[i - 1][j + yy] = 54;
	}
	if (dungeon[i - 1][j + yy - 1] == 6) {
		dungeon[i - 1][j + yy - 1] = 55;
	}
}

static void L4AddWall()
{
	int i, j, x, y;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dflags[i][j] != 0) {
				continue;
			}
			if (dungeon[i][j] == 10 && random_(0, 100) < 100) {
				x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (dungeon[i][j] == 12 && random_(0, 100) < 100) {
				x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (dungeon[i][j] == 13 && random_(0, 100) < 100) {
				x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (dungeon[i][j] == 15 && random_(0, 100) < 100) {
				x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (dungeon[i][j] == 16 && random_(0, 100) < 100) {
				x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (dungeon[i][j] == 21 && random_(0, 100) < 100) {
				x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (dungeon[i][j] == 22 && random_(0, 100) < 100) {
				x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (dungeon[i][j] == 8 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
			if (dungeon[i][j] == 9 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
			if (dungeon[i][j] == 11 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
			if (dungeon[i][j] == 14 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
			if (dungeon[i][j] == 15 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
			if (dungeon[i][j] == 16 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
			if (dungeon[i][j] == 21 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
			if (dungeon[i][j] == 23 && random_(0, 100) < 100) {
				y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
		}
	}
}

static void L4tileFix()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 2) {
				if (dungeon[i + 1][j] == 6)
					dungeon[i + 1][j] = 5;
				else if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 13;
			} else if (dungeon[i][j] == 1)
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 14;
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 1:
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 10;
				break;
			case 2:
				if (dungeon[i + 1][j] == 6)
					dungeon[i + 1][j] = 2;
				else if (dungeon[i + 1][j] == 9)
					dungeon[i + 1][j] = 11;
				break;
			case 6:
				if (dungeon[i + 1][j] == 14)
					dungeon[i + 1][j] = 15;
				if (dungeon[i][j + 1] == 13)
					dungeon[i][j + 1] = 16;
				if (dungeon[i][j - 1] == 1)
					dungeon[i][j - 1] = 1;
				break;
			case 9:
				if (dungeon[i + 1][j] == 6)
					dungeon[i + 1][j] = 12;
				break;
			case 14:
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 13;
				break;
			}
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 1:
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 27;
				if (dungeon[i][j - 1] == 15)
					dungeon[i][j - 1] = 10;
				break;
			case 2:
				if (dungeon[i + 1][j] == 15)
					dungeon[i + 1][j] = 14;
				if (dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 9)
					dungeon[i + 1][j] = 29;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 28;
				if (dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 30)
					dungeon[i + 1][j] = 29;
				if (dungeon[i + 1][j] == 18)
					dungeon[i + 1][j] = 25;
				if (dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 2)
					dungeon[i + 1][j] = 29;
				if (dungeon[i + 1][j] == 28 && dungeon[i + 1][j - 1] == 6)
					dungeon[i + 1][j] = 23;
				break;
			case 6:
				if (dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] != 0) /* check */
					dungeon[i + 1][j] = 22;
				if (dungeon[i + 1][j] == 30 && dungeon[i + 1][j - 1] == 6)
					dungeon[i + 1][j] = 21;
				break;
			case 9:
				if (dungeon[i + 1][j] == 15)
					dungeon[i + 1][j] = 14;
				if (dungeon[i + 1][j] == 15)
					dungeon[i + 1][j] = 14;
				break;
			case 11:
				if (dungeon[i + 1][j] == 15)
					dungeon[i + 1][j] = 14;
				if (dungeon[i + 1][j] == 3)
					dungeon[i + 1][j] = 5;
				break;
			case 13:
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 27;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 10;
				break;
			case 14:
				if (dungeon[i + 1][j] == 15)
					dungeon[i + 1][j] = 14;
				if (dungeon[i + 1][j] == 30 && dungeon[i][j + 1] == 6)
					dungeon[i + 1][j] = 28;
				if (dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 1)
					dungeon[i + 1][j] = 23;
				if (dungeon[i + 1][j] == 30 && dungeon[i + 1][j + 1] == 30)
					dungeon[i + 1][j] = 23;
				if (dungeon[i + 1][j] == 23 && dungeon[i + 2][j] == 30)
					dungeon[i + 1][j] = 28;
				if (dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 30 && dungeon[i + 1][j - 1] == 6)
					dungeon[i + 1][j] = 23;
				break;
			case 15:
				if (dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 2)
					dungeon[i + 1][j] = 29;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 28;
				if (dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 30 && dungeon[i + 1][j - 1] == 6)
					dungeon[i + 1][j] = 23;
				if (dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 30)
					dungeon[i + 1][j] = 29;
				if (dungeon[i][j + 1] == 3)
					dungeon[i][j + 1] = 4;
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				break;
			case 16:
				if (dungeon[i + 1][j] == 6 && dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 27;
				if (dungeon[i][j + 1] == 30 && dungeon[i + 1][j + 1] == 30)
					dungeon[i][j + 1] = 27;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				break;
			case 18:
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 10;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				break;
			case 19:
				if (dungeon[i + 1][j] == 27)
					dungeon[i + 1][j] = 26;
				if (dungeon[i + 1][j] == 18)
					dungeon[i + 1][j] = 24;
				if (dungeon[i + 1][j] == 19 && dungeon[i + 1][j - 1] == 30)
					dungeon[i + 1][j] = 24;
				if (dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 18 && dungeon[i + 1][j + 1] == 1)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 22 && dungeon[i + 1][j + 1] == 1)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 18 && dungeon[i + 1][j + 1] == 13)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 1][j] == 10)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				if (dungeon[i + 1][j] == 9)
					dungeon[i + 1][j] = 11;
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 13;
				if (dungeon[i + 1][j] == 13 && dungeon[i + 1][j - 1] == 6)
					dungeon[i + 1][j] = 16;
				break;
			case 21:
				if (dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 1)
					dungeon[i + 1][j] = 13;
				if (dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 18 && dungeon[i + 1][j + 1] == 1)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 1][j + 1] == 1 && dungeon[i + 1][j - 1] == 22 && dungeon[i + 2][j] == 3)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 1][j - 1] == 21)
					dungeon[i + 1][j] = 24;
				if (dungeon[i + 1][j] == 9 && dungeon[i + 2][j] == 2)
					dungeon[i + 1][j] = 11;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 10;
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				if (dungeon[i + 1][j] == 18 && dungeon[i + 2][j] == 30)
					dungeon[i + 1][j] = 24;
				if (dungeon[i + 1][j] == 9 && dungeon[i + 1][j + 1] == 1)
					dungeon[i + 1][j] = 16;
				if (dungeon[i + 1][j] == 27)
					dungeon[i + 1][j] = 26;
				if (dungeon[i + 1][j] == 18)
					dungeon[i + 1][j] = 24;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				break;
			case 22:
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i + 1][j] == 9)
					dungeon[i + 1][j] = 11;
				break;
			case 23:
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 15;
				if (dungeon[i + 1][j] == 9)
					dungeon[i + 1][j] = 11;
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				if (dungeon[i + 1][j] == 18 && dungeon[i][j - 1] == 6)
					dungeon[i + 1][j] = 24;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				break;
			case 24:
				if (dungeon[i][j - 1] == 30 && dungeon[i][j - 2] == 6)
					dungeon[i][j - 1] = 21;
				if (dungeon[i - 1][j] == 30)
					dungeon[i - 1][j] = 19;
				break;
			case 25:
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				break;
			case 26:
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				break;
			case 27:
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				break;
			case 28:
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 15;
				break;
			case 29:
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				if (dungeon[i][j + 1] == 2)
					dungeon[i][j + 1] = 15;
				if (dungeon[i + 1][j] == 30)
					dungeon[i + 1][j] = 19;
				if (dungeon[i][j + 1] == 30)
					dungeon[i][j + 1] = 18;
				break;
			}
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 1:
				if (dungeon[i][j - 1] == 6)
					dungeon[i][j - 1] = 7;
				if (dungeon[i][j + 1] == 3)
					dungeon[i][j + 1] = 4;
				if (dungeon[i][j + 1] == 6)
					dungeon[i][j + 1] = 4;
				if (dungeon[i][j + 1] == 5)
					dungeon[i][j + 1] = 12;
				if (dungeon[i][j + 1] == 16)
					dungeon[i][j + 1] = 13;
				if (dungeon[i][j - 1] == 8)
					dungeon[i][j - 1] = 9;
				break;
			case 2:
				if (dungeon[i - 1][j] == 6)
					dungeon[i - 1][j] = 8;
				if (dungeon[i + 1][j] == 3)
					dungeon[i + 1][j] = 5;
				if (dungeon[i + 1][j] == 5 && dungeon[i + 1][j - 1] == 16)
					dungeon[i + 1][j] = 12;
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				break;
			case 6:
				if (dungeon[i + 1][j] == 15 && dungeon[i + 1][j + 1] == 4)
					dungeon[i + 1][j] = 10;
				if (dungeon[i][j + 1] == 13)
					dungeon[i][j + 1] = 16;
				break;
			case 9:
				if (dungeon[i][j + 1] == 3)
					dungeon[i][j + 1] = 4;
				if (dungeon[i + 1][j] == 3)
					dungeon[i + 1][j] = 5;
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				break;
			case 10:
				if (dungeon[i][j + 1] == 3)
					dungeon[i][j + 1] = 4;
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				break;
			case 11:
				if (dungeon[i + 1][j] == 3)
					dungeon[i + 1][j] = 5;
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				break;
			case 13:
				if (dungeon[i][j + 1] == 3)
					dungeon[i][j + 1] = 4;
				if (dungeon[i][j + 1] == 5)
					dungeon[i][j + 1] = 12;
				break;
			case 14:
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				if (dungeon[i + 1][j] == 3)
					dungeon[i + 1][j] = 5;
				break;
			case 15:
				if (dungeon[i + 1][j + 1] == 9 && dungeon[i + 1][j - 1] == 1 && dungeon[i + 2][j] == 16)
					dungeon[i + 1][j] = 29;
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				if (dungeon[i + 1][j] == 3)
					dungeon[i + 1][j] = 5;
				break;
			case 19:
				if (dungeon[i + 1][j] == 10)
					dungeon[i + 1][j] = 17;
				break;
			case 21:
				if (dungeon[i][j + 1] == 24 && dungeon[i][j + 2] == 1)
					dungeon[i][j + 1] = 17;
				if (dungeon[i + 1][j] == 9)
					dungeon[i + 1][j] = 11;
				if (dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 16;
				break;
			case 25:
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 10;
				break;
			case 27:
				if (dungeon[i + 1][j] == 9)
					dungeon[i + 1][j] = 11;
				break;
			case 28:
				if (dungeon[i][j - 1] == 6 && dungeon[i + 1][j] == 1)
					dungeon[i + 1][j] = 23;
				if (dungeon[i + 1][j] == 23 && dungeon[i + 1][j + 1] == 3)
					dungeon[i + 1][j] = 16;
				break;
			}
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 1:
				if (dungeon[i][j + 1] == 6)
					dungeon[i][j + 1] = 4;
				break;
			case 9:
				if (dungeon[i][j + 1] == 16)
					dungeon[i][j + 1] = 13;
				break;
			case 10:
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				if (dungeon[i][j + 1] == 16)
					dungeon[i][j + 1] = 13;
				if (dungeon[i + 1][j] == 3 && dungeon[i + 1][j - 1] == 16)
					dungeon[i + 1][j] = 12;
				break;
			case 11:
				if (dungeon[i][j + 1] == 5)
					dungeon[i][j + 1] = 12;
				break;
			case 13:
				if (dungeon[i][j + 1] == 5)
					dungeon[i][j + 1] = 12;
				break;
			case 15:
				if (dungeon[i + 1][j] == 10)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 16)
					dungeon[i + 1][j] = 23;
				break;
			case 16:
				if (dungeon[i][j + 1] == 3)
					dungeon[i][j + 1] = 4;
				if (dungeon[i][j + 1] == 5)
					dungeon[i][j + 1] = 12;
				break;
			case 17:
				if (dungeon[i + 1][j] == 4)
					dungeon[i + 1][j] = 12;
				if (dungeon[i][j + 1] == 5)
					dungeon[i][j + 1] = 12;
				break;
			case 21:
				if (dungeon[i + 1][j] == 10)
					dungeon[i + 1][j] = 17;
				if (dungeon[i + 1][j] == 13 && dungeon[i][j + 1] == 10)
					dungeon[i + 1][j + 1] = 12;
				break;
			case 22:
				if (dungeon[i][j + 1] == 11)
					dungeon[i][j + 1] = 17;
				break;
			case 28:
				if (dungeon[i + 1][j] == 23 && dungeon[i + 1][j + 1] == 1 && dungeon[i + 2][j] == 6)
					dungeon[i + 1][j] = 16;
				break;
			case 29:
				if (dungeon[i][j + 1] == 9)
					dungeon[i][j + 1] = 10;
				break;
			}
		}
	}
	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 16)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j - 1] == 21 && dungeon[i + 1][j + 1] == 13 && dungeon[i + 2][j] == 2)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 15 && dungeon[i + 1][j + 1] == 12)
				dungeon[i + 1][j] = 17;
		}
	}
}

static void DRLG_L4Subs()
{
	int x, y, i, rv;
	BYTE c;

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (random_(0, 3) == 0) {
				c = L4BTYPES[dungeon[x][y]];
				if (c != 0 && dflags[x][y] == 0) {
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
	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (random_(0, 10) == 0) {
				if (L4BTYPES[dungeon[x][y]] == 6 && dflags[x][y] == 0) {
					dungeon[x][y] = RandRange(95, 97);
				}
			}
		}
	}
}

static void L4makeDungeon()
{
	int i, j;
	BYTE bv;

	for (j = 0; j < 40; j += 2) {
		for (i = 0; i < 40; i += 2) {
			bv = dung[i >> 1][j >> 1];
			L4dungeon[i][j] = bv;
			L4dungeon[i][j + 1] = bv;
			L4dungeon[i + 1][j] = bv;
			L4dungeon[i + 1][j + 1] = bv;
		}
	}
	for (j = 0; j < 40; j += 2) {
		for (i = 0; i < 40; i += 2) {
			bv = dung[i >> 1][19 - (j >> 1)];
			L4dungeon[i][j + 40] = bv;
			L4dungeon[i][j + 41] = bv;
			L4dungeon[i + 1][j + 40] = bv;
			L4dungeon[i + 1][j + 41] = bv;
		}
	}
	for (j = 0; j < 40; j += 2) {
		for (i = 0; i < 40; i += 2) {
			bv = dung[19 - (i >> 1)][j >> 1];
			L4dungeon[i + 40][j] = bv;
			L4dungeon[i + 40][j + 1] = bv;
			L4dungeon[i + 41][j] = bv;
			L4dungeon[i + 41][j + 1] = bv;
		}
	}
	for (j = 0; j < 40; j += 2) {
		for (i = 0; i < 40; i += 2) {
			bv = dung[19 - (i >> 1)][19 - (j >> 1)];
			L4dungeon[i + 40][j + 40] = bv;
			L4dungeon[i + 40][j + 41] = bv;
			L4dungeon[i + 41][j + 40] = bv;
			L4dungeon[i + 41][j + 41] = bv;
		}
	}
}

static void uShape()
{
	int j, i, rv;

	for (j = 19; j >= 0; j--) {
		for (i = 19; i >= 0; i--) {
			if (dung[i][j] != 1) {
				hallok[j] = FALSE;
			}
			if (dung[i][j] == 1) {
				// BUGFIX: check that i + 1 < 20 and j + 1 < 20 (fixed)
				if (i + 1 < 20 && j + 1 < 20
				    && dung[i][j + 1] == 1 && dung[i + 1][j + 1] == 0) {
					hallok[j] = TRUE;
				} else {
					hallok[j] = FALSE;
				}
				i = 0;
			}
		}
	}

	rv = RandRange(1, 19);
	do {
		if (hallok[rv]) {
			for (i = 19; i >= 0; i--) {
				if (dung[i][rv] == 1) {
					i = -1;
					rv = 0;
				} else {
					dung[i][rv] = 1;
					dung[i][rv + 1] = 1;
				}
			}
		} else {
			rv++;
			if (rv == 20) {
				rv = 1;
			}
		}
	} while (rv != 0);

	for (i = 19; i >= 0; i--) {
		for (j = 19; j >= 0; j--) {
			if (dung[i][j] != 1) {
				hallok[i] = FALSE;
			}
			if (dung[i][j] == 1) {
				// BUGFIX: check that i + 1 < 20 and j + 1 < 20 (fixed)
				if (i + 1 < 20 && j + 1 < 20
				    && dung[i + 1][j] == 1 && dung[i + 1][j + 1] == 0) {
					hallok[i] = TRUE;
				} else {
					hallok[i] = FALSE;
				}
				j = 0;
			}
		}
	}

	rv = RandRange(1, 19);
	do {
		if (hallok[rv]) {
			for (j = 19; j >= 0; j--) {
				if (dung[rv][j] == 1) {
					j = -1;
					rv = 0;
				} else {
					dung[rv][j] = 1;
					dung[rv + 1][j] = 1;
				}
			}
		} else {
			rv++;
			if (rv == 20) {
				rv = 1;
			}
		}
	} while (rv != 0);
}

static int GetArea()
{
	int i, j, rv;

	rv = 0;

	for (j = 0; j < 20; j++) {
		for (i = 0; i < 20; i++) {
			if (dung[i][j] == 1) {
				rv++;
			}
		}
	}

	return rv;
}

static void L4drawRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	x2 = x + width;
	y2 = y + height;
	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			dung[i][j] = 1;
		}
	}
}

static BOOL L4checkRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	if (x <= 0 || y <= 0)
		return FALSE;

	x2 = x + width;
	y2 = y + height;
	if (x2 > 20 || y2 > 20)
		return FALSE;

	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			if (dung[i][j] != 0) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

static void L4roomGen(int x, int y, int w, int h, int dir)
{
	int dirProb, i, width, height, rx, ry, rxy2;
	BOOL ran2;

	dirProb = random_(0, 4);

	if (dir == 1 ? dirProb == 0 : dirProb != 0) {
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2 + y - height / 2;
			rx = x - width;
			if (L4checkRoom(rx - 1, ry - 1, height + 2, width + 1)) /// BUGFIX: swap args 3 and 4 ("ch+2" and "cw+1")
				break;
		}

		if (i != 0)
			L4drawRoom(rx, ry, width, height);
		rxy2 = x + w;
		ran2 = L4checkRoom(rxy2, ry - 1, width + 1, height + 2);
		if (ran2)
			L4drawRoom(rxy2, ry, width, height);
		if (i != 0)
			L4roomGen(rx, ry, width, height, 1);
		if (ran2)
			L4roomGen(rxy2, ry, width, height, 1);
	} else {
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2 + x - width / 2;
			ry = y - height;
			if (L4checkRoom(rx - 1, ry - 1, width + 2, height + 1))
				break;
		}

		if (i != 0)
			L4drawRoom(rx, ry, width, height);
		rxy2 = y + h;
		ran2 = L4checkRoom(rx - 1, rxy2, width + 2, height + 1);
		if (ran2)
			L4drawRoom(rx, rxy2, width, height);
		if (i != 0)
			L4roomGen(rx, ry, width, height, 0);
		if (ran2)
			L4roomGen(rx, rxy2, width, height, 0);
	}
}

static void L4firstRoom()
{
	int x, y, w, h, rndx, rndy, xmin, xmax, ymin, ymax, tx, ty;

	if (currlevel != 16) {
		if (currlevel == quests[Q_WARLORD]._qlevel && quests[Q_WARLORD]._qactive != QUEST_NOTAVAIL) {
			/// ASSERT: assert(gbMaxPlayers == 1);
			w = 11;
			h = 11;
		} else if (currlevel == quests[Q_BETRAYER]._qlevel && gbMaxPlayers != 1) {
			w = 11;
			h = 11;
		} else {
			w = RandRange(2, 6);
			h = RandRange(2, 6);
		}
	} else {
		w = 14;
		h = 14;
	}

	xmax = 19 - w;
	xmin = (xmax + 1) >> 1;
	rndx = RandRange(xmin, xmax);

	ymax = 19 - h;
	ymin = (ymax + 1) >> 1;
	rndy = RandRange(ymin, ymax);

	if (currlevel == 16) {
		l4holdx = x;
		l4holdy = y;
	}
	if (QuestStatus(Q_WARLORD) || currlevel == quests[Q_BETRAYER]._qlevel && gbMaxPlayers != 1) {
		SP4x1 = x + 1;
		SP4y1 = y + 1;
		SP4x2 = SP4x1 + w;
		SP4y2 = SP4y1 + h;
	} else {
		SP4x1 = 0;
		SP4y1 = 0;
		SP4x2 = 0;
		SP4y2 = 0;
	}

	L4drawRoom(x, y, w, h);
	L4roomGen(x, y, w, h, random_(0, 2));
}

static void L4SaveQuads()
{
	int i, j, x, y;

	x = l4holdx;
	y = l4holdy;

	for (j = y; j < y + 14; j++) {
		for (i = x; i < x + 14; i++) {
			dflags[i][j] = 1;
			dflags[DMAXX - 1 - i][j] = 1;
			dflags[i][DMAXY - 1 - j] = 1;
			dflags[DMAXX - 1 - i][DMAXY - 1 - j] = 1;
		}
	}
}

static void DRLG_L4SetRoom(BYTE *pSetPiece, int rx1, int ry1)
{
	int rx2, ry2, i, j;
	BYTE *sp;

	rx2 = pSetPiece[0] + rx1;
	ry2 = pSetPiece[2] + ry1;
	sp = &pSetPiece[4];

	for (j = ry1; j < ry2; j++) {
		for (i = rx1; i < rx2; i++) {
			if (*sp != 0) {
				dungeon[i][j] = *sp;
				dflags[i][j] |= DLRG_PROTECTED;
			} else {
				dungeon[i][j] = 6;
			}
			sp += 2;
		}
	}
}

static void DRLG_LoadDiabQuads(BOOL preflag)
{
	BYTE *lpSetPiece;

	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab1.DUN", NULL);
	diabquad1x = 4 + l4holdx;
	diabquad1y = 4 + l4holdy;
	DRLG_L4SetRoom(lpSetPiece, diabquad1x, diabquad1y);
	mem_free_dbg(lpSetPiece);

	if (preflag) {
		lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab2b.DUN", NULL);
	} else {
		lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab2a.DUN", NULL);
	}
	diabquad2x = 27 - l4holdx;
	diabquad2y = 1 + l4holdy;
	DRLG_L4SetRoom(lpSetPiece, diabquad2x, diabquad2y);
	mem_free_dbg(lpSetPiece);

	if (preflag) {
		lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab3b.DUN", NULL);
	} else {
		lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab3a.DUN", NULL);
	}
	diabquad3x = 1 + l4holdx;
	diabquad3y = 27 - l4holdy;
	DRLG_L4SetRoom(lpSetPiece, diabquad3x, diabquad3y);
	mem_free_dbg(lpSetPiece);

	if (preflag) {
		lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab4b.DUN", NULL);
	} else {
		lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab4a.DUN", NULL);
	}
	diabquad4x = 28 - l4holdx;
	diabquad4y = 28 - l4holdy;
	DRLG_L4SetRoom(lpSetPiece, diabquad4x, diabquad4y);
	mem_free_dbg(lpSetPiece);
}

static BOOL DRLG_L4PlaceMiniSet(const BYTE *miniset, BOOL setview)
{
	int sx, sy, sw, sh, xx, yy, ii, tries;
	BOOL done;

	sw = miniset[0];
	sh = miniset[1];

	sx = random_(0, DMAXX - sw);
	sy = random_(0, DMAXY - sh);

	tries = 0;
	while (TRUE) {
		done = TRUE;
		if (sx >= SP4x1 && sx <= SP4x2 && sy >= SP4y1 && sy <= SP4y2) {
			done = FALSE;
		}
		ii = 2;
		for (yy = sy; yy < sy + sh && done; yy++) {
			for (xx = sx; xx < sx + sw && done; xx++) {
				if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
					done = FALSE;
				}
				if (dflags[xx][yy] != 0) {
					done = FALSE;
				}
				ii++;
			}
		}
		tries++;
		if (done || tries == 200)
			break;
		if (++sx == DMAXX - sw) {
			sx = 0;
			if (++sy == DMAXY - sh) {
				sy = 0;
			}
		}
	}
	if (tries == 200)
		return FALSE;

	ii = sw * sh + 2;
	for (yy = sy; yy < sy + sh; yy++) {
		for (xx = sx; xx < sx + sw; xx++) {
			if (miniset[ii] != 0) {
				dungeon[xx][yy] = miniset[ii];
				dflags[xx][yy] |= 8;
			}
			ii++;
		}
	}

	if (currlevel == 15) {
		quests[Q_BETRAYER]._qtx = sx + 1;
		quests[Q_BETRAYER]._qty = sy + 1;
	}
	if (setview) {
		ViewX = 2 * sx + DBORDERX + 5;
		ViewY = 2 * sy + DBORDERY + 6;
	}

	return TRUE;
}

static void DRLG_L4FTVR(int i, int j, int x, int y, int dir)
{
	if (dTransVal[x][y] != 0 || dungeon[i][j] != 6) {
		switch (dir) {
		case 1:
			dTransVal[x][y] = TransVal;
			dTransVal[x][y + 1] = TransVal;
			break;
		case 2:
			dTransVal[x + 1][y] = TransVal;
			dTransVal[x + 1][y + 1] = TransVal;
			break;
		case 3:
			dTransVal[x][y] = TransVal;
			dTransVal[x + 1][y] = TransVal;
			break;
		case 4:
			dTransVal[x][y + 1] = TransVal;
			dTransVal[x + 1][y + 1] = TransVal;
			break;
		case 5:
			dTransVal[x + 1][y + 1] = TransVal;
			break;
		case 6:
			dTransVal[x][y + 1] = TransVal;
			break;
		case 7:
			dTransVal[x + 1][y] = TransVal;
			break;
		case 8:
			dTransVal[x][y] = TransVal;
			break;
		}
	} else {
		dTransVal[x][y] = TransVal;
		dTransVal[x + 1][y] = TransVal;
		dTransVal[x][y + 1] = TransVal;
		dTransVal[x + 1][y + 1] = TransVal;
		DRLG_L4FTVR(i + 1, j, x + 2, y, 1);
		DRLG_L4FTVR(i - 1, j, x - 2, y, 2);
		DRLG_L4FTVR(i, j + 1, x, y + 2, 3);
		DRLG_L4FTVR(i, j - 1, x, y - 2, 4);
		DRLG_L4FTVR(i - 1, j - 1, x - 2, y - 2, 5);
		DRLG_L4FTVR(i + 1, j - 1, x + 2, y - 2, 6);
		DRLG_L4FTVR(i - 1, j + 1, x - 2, y + 2, 7);
		DRLG_L4FTVR(i + 1, j + 1, x + 2, y + 2, 8);
	}
}

static void DRLG_L4FloodTVal()
{
	int i, j, xx, yy;

	yy = DBORDERY;
	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 6 && dTransVal[xx][yy] == 0) {
				DRLG_L4FTVR(i, j, xx, yy, 0);
				TransVal++;
			}
			xx += 2;
		}
		yy += 2;
	}
}

static BOOL IsDURWall(BYTE dd)
{
	return dd == 25 || dd == 28 || dd == 23;
}

static BOOL IsDLLWall(BYTE dd)
{
	return dd == 27 || dd == 26 || dd == 22;
}

static void DRLG_L4TransFix()
{
	int i, j, xx, yy;

	yy = DBORDERY;
	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;
		for (i = 0; i < DMAXX; i++) {
			if (IsDURWall(dungeon[i][j])) {
				if (dungeon[i][j - 1] == 18) {
					dTransVal[xx + 1][yy] = dTransVal[xx][yy];
					dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
				}
			} else if (IsDLLWall(dungeon[i][j])) {
				if (dungeon[i + 1][j] == 19) {
					dTransVal[xx][yy + 1] = dTransVal[xx][yy];
					dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
				}
			} else if (dungeon[i][j] == 18) {
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			} else if (dungeon[i][j] == 19) {
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			} else if (dungeon[i][j] == 24) {
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			} else if (dungeon[i][j] == 57) {
				dTransVal[xx - 1][yy] = dTransVal[xx][yy + 1];
				dTransVal[xx][yy] = dTransVal[xx][yy + 1];
			} else if (dungeon[i][j] == 53) {
				dTransVal[xx][yy - 1] = dTransVal[xx + 1][yy];
				dTransVal[xx][yy] = dTransVal[xx + 1][yy];
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void DRLG_L4Corners()
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			if (dungeon[i][j] >= 18 && dungeon[i][j] <= 30) {
				if (dungeon[i + 1][j] < 18) {
					dungeon[i][j] += 98;
				} else if (dungeon[i][j + 1] < 18) {
					dungeon[i][j] += 98;
				}
			}
		}
	}
}

static void L4FixRim()
{
	int i, j;

	for (i = 0; i < 20; i++) {
		dung[i][0] = 0;
	}
	for (j = 0; j < 20; j++) {
		dung[0][j] = 0;
	}
}

static void DRLG_L4GeneralFix()
{
	int i, j;

	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			if ((dungeon[i][j] == 24 || dungeon[i][j] == 122) && dungeon[i + 1][j] == 2 && dungeon[i][j + 1] == 5) {
				dungeon[i][j] = 17;
			}
		}
	}
}

static void DRLG_L4(int entry)
{
	int i, j, ar;
	BOOL doneflag;

	do {
		DRLG_InitTrans();
		do {
			InitL4Dungeon();
			L4firstRoom();
			L4FixRim();
			ar = GetArea();
			if (ar >= 173) {
				uShape();
			}
		} while (ar < 173);
		L4makeDungeon();
		L4makeDmt();
		L4tileFix();
		if (currlevel == 16) {
			L4SaveQuads();
		}
		if (QuestStatus(Q_WARLORD) || currlevel == quests[Q_BETRAYER]._qlevel && gbMaxPlayers != 1) {
			for (i = SP4x1; i < SP4x2; i++) {
				for (j = SP4y1; j < SP4y2; j++) {
					dflags[i][j] = 1;
				}
			}
		}
		L4AddWall();
		DRLG_L4FloodTVal();
		DRLG_L4TransFix();
		if (setloadflag) {
			DRLG_L4SetSPRoom(SP4x1, SP4y1);
		}
		if (currlevel == 16) {
			DRLG_LoadDiabQuads(TRUE);
		}
		if (QuestStatus(Q_WARLORD)) {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, TRUE)
					&& (currlevel != 13 || DRLG_L4PlaceMiniSet(L4TWARP, FALSE));
			} else if (entry == ENTRY_PREV) {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, FALSE)
					&& (currlevel != 13 || DRLG_L4PlaceMiniSet(L4TWARP, FALSE));
				ViewX = 2 * setpc_x + DBORDERX + 6;
				ViewY = 2 * setpc_y + DBORDERY + 6;
			} else {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, FALSE)
					&& (currlevel != 13 || DRLG_L4PlaceMiniSet(L4TWARP, TRUE));
			}
		} else if (currlevel != 15) {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, TRUE)
					&& (currlevel == 16 || DRLG_L4PlaceMiniSet(L4DSTAIRS, FALSE))
					&& (currlevel != 13 || DRLG_L4PlaceMiniSet(L4TWARP, FALSE));
			} else if (entry == ENTRY_PREV) {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, FALSE)
					&& (currlevel == 16 || DRLG_L4PlaceMiniSet(L4DSTAIRS, TRUE))
					&& (currlevel != 13 || DRLG_L4PlaceMiniSet(L4TWARP, FALSE));
				ViewX++;
				ViewY -= 2;
			} else {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, FALSE)
					&& (currlevel == 16 || DRLG_L4PlaceMiniSet(L4DSTAIRS, FALSE))
					&& (currlevel != 13 || DRLG_L4PlaceMiniSet(L4TWARP, TRUE));
			}
		} else {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, TRUE);
				if (doneflag) {
					if (gbMaxPlayers == 1 && quests[Q_DIABLO]._qactive != QUEST_ACTIVE) {
						doneflag = DRLG_L4PlaceMiniSet(L4PENTA, FALSE);
					} else {
						doneflag = DRLG_L4PlaceMiniSet(L4PENTA2, FALSE);
					}
				}
			} else {
				doneflag = DRLG_L4PlaceMiniSet(L4USTAIRS, FALSE);
				if (doneflag) {
					if (gbMaxPlayers == 1 && quests[Q_DIABLO]._qactive != QUEST_ACTIVE) {
						doneflag = DRLG_L4PlaceMiniSet(L4PENTA, TRUE);
					} else {
						doneflag = DRLG_L4PlaceMiniSet(L4PENTA2, TRUE);
					}
				}
				ViewY++;
			}
		}
	} while (!doneflag);

	DRLG_L4GeneralFix();

	if (currlevel != 16) {
		DRLG_PlaceThemeRooms(7, 10, 6, 8, TRUE);
	}

	DRLG_L4Shadows();
	DRLG_L4Corners();
	DRLG_L4Subs();
	DRLG_Init_Globals();

	if (QuestStatus(Q_WARLORD)) {
		memcpy(pdungeon, dungeon, sizeof(pdungeon));
	}

	DRLG_CheckQuests(SP4x1, SP4y1);

	if (currlevel == 15) {
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++) {
				if (dungeon[i][j] == 98 || dungeon[i][j] == 107) {
					Make_SetPC(i - 1, j - 1, 5, 5);
				}
			}
		}
	}
	if (currlevel == 16) {
		memcpy(pdungeon, dungeon, sizeof(pdungeon));

		DRLG_LoadDiabQuads(FALSE);
	}
}

static void DRLG_L4Pass3()
{
	int i, j, xx, yy;
	long v1, v2, v3, v4, lv;
	WORD *MegaTiles;

	lv = 30 - 1;

	MegaTiles = (WORD *)&pMegaTiles[lv * 8];
	v1 = SDL_SwapLE16(*(MegaTiles + 0)) + 1;
	v2 = SDL_SwapLE16(*(MegaTiles + 1)) + 1;
	v3 = SDL_SwapLE16(*(MegaTiles + 2)) + 1;
	v4 = SDL_SwapLE16(*(MegaTiles + 3)) + 1;

	for (j = 0; j < MAXDUNY; j += 2) {
		for (i = 0; i < MAXDUNX; i += 2) {
			dPiece[i][j] = v1;
			dPiece[i + 1][j] = v2;
			dPiece[i][j + 1] = v3;
			dPiece[i + 1][j + 1] = v4;
		}
	}

	yy = DBORDERY;
	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;
		for (i = 0; i < DMAXX; i++) {
			lv = dungeon[i][j] - 1;
			if (lv >= 0) {
				MegaTiles = (WORD *)&pMegaTiles[lv * 8];
				v1 = SDL_SwapLE16(*(MegaTiles + 0)) + 1;
				v2 = SDL_SwapLE16(*(MegaTiles + 1)) + 1;
				v3 = SDL_SwapLE16(*(MegaTiles + 2)) + 1;
				v4 = SDL_SwapLE16(*(MegaTiles + 3)) + 1;
			} else {
				v1 = 0;
				v2 = 0;
				v3 = 0;
				v4 = 0;
			}
			dPiece[xx][yy] = v1;
			dPiece[xx + 1][yy] = v2;
			dPiece[xx][yy + 1] = v3;
			dPiece[xx + 1][yy + 1] = v4;
			xx += 2;
		}
		yy += 2;
	}
}

void CreateL4Dungeon(DWORD rseed, int entry)
{
	SetRndSeed(rseed);

	ViewX = DSIZEX / 2;
	ViewY = DSIZEY / 2;

	DRLG_InitSetPC();
	DRLG_LoadL4SP();
	DRLG_L4(entry);
	DRLG_L4Pass3();
	DRLG_FreeL4SP();
	DRLG_SetPC();
}

static void LoadL4Dungeon(char *sFileName, int vx, int vy)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm;

	DRLG_InitTrans();
	InitL4Dungeon();
	pLevelMap = LoadFileInMem(sFileName, NULL);


	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				dungeon[i][j] = *lm;
				dflags[i][j] |= DLRG_PROTECTED;
			} else {
				dungeon[i][j] = 6;
			}
			lm += 2;
		}
	}

	ViewX = vx;
	ViewY = vy;
	DRLG_L4Pass3();
	DRLG_Init_Globals();

	SetMapMonsters(pLevelMap, 0, 0);
	SetMapObjects(pLevelMap, 0, 0);
	mem_free_dbg(pLevelMap);
}

static void LoadPreL4Dungeon(char *sFileName, int vx, int vy)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm;

	InitL4Dungeon();

	pLevelMap = LoadFileInMem(sFileName, NULL);

	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				dungeon[i][j] = *lm;
				dflags[i][j] |= DLRG_PROTECTED;
			} else {
				dungeon[i][j] = 6;
			}
			lm += 2;
		}
	}
	mem_free_dbg(pLevelMap);
}

DEVILUTION_END_NAMESPACE
