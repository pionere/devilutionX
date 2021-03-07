/**
 * @file drlg_l1.cpp
 *
 * Implementation of the cathedral level generation algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Starting position of the megatiles. */
#define BASE_MEGATILE_L1 (22 - 1)

/** Represents a tile ID map of twice the size, repeating each tile of the original map in blocks of 4. */
BYTE L1dflags[DMAXX][DMAXY];
/** Specifies whether a single player quest DUN has been loaded. */
bool L1setloadflag; // TODO: re-use gbSetloadflag ?
/** Specifies whether to generate a horizontal room at position 1 in the Cathedral. */
BOOL HR1;
/** Specifies whether to generate a horizontal room at position 2 in the Cathedral. */
BOOL HR2;
/** Specifies whether to generate a horizontal room at position 3 in the Cathedral. */
BOOL HR3;
#ifdef HELLFIRE
int UberRow;
int UberCol;
bool gbUberRoomOpened;
int UberLeverRow;
int UberLeverCol;
bool gbUberLeverActivated;
int UberDiabloMonsterIndex;
#endif
/** Specifies whether to generate a vertical room at position 1 in the Cathedral. */
BOOL VR1;
/** Specifies whether to generate a vertical room at position 2 in the Cathedral. */
BOOL VR2;
/** Specifies whether to generate a vertical room at position 3 in the Cathedral. */
BOOL VR3;
/** Contains the contents of the single player quest DUN file. */
BYTE *L1pSetPiece;

/** Contains shadows for 2x2 blocks of base tile IDs in the Cathedral. */
const ShadowStruct SPATS[37] = {
	// clang-format off
	// strig, s1, s2, s3, nv1, nv2, nv3
	{      7, 13,  0, 13, 144,   0, 142 },
	{     16, 13,  0, 13, 144,   0, 142 },
	{     15, 13,  0, 13, 145,   0, 142 },
	{      5, 13, 13, 13, 152, 140, 139 },
	{      5, 13,  1, 13, 143, 146, 139 },
	{      5, 13, 13,  2, 143, 140, 148 },
	{      5,  0,  1,  2,   0, 146, 148 },
	{      5, 13, 11, 13, 143, 147, 139 },
	{      5, 13, 13, 12, 143, 140, 149 },
	{      5, 13, 11, 12, 150, 147, 149 },
	{      5, 13,  1, 12, 143, 146, 149 },
	{      5, 13, 11,  2, 143, 147, 148 },
	{      9, 13, 13, 13, 144, 140, 142 },
	{      9, 13,  1, 13, 144, 146, 142 },
	{      9, 13, 11, 13, 151, 147, 142 },
	{      8, 13,  0, 13, 144,   0, 139 },
	{      8, 13,  0, 12, 143,   0, 149 },
	{      8,  0,  0,  2,   0,   0, 148 },
	{     11,  0,  0, 13,   0,   0, 139 },
	{     11, 13,  0, 13, 139,   0, 139 },
	{     11,  2,  0, 13, 148,   0, 139 },
	{     11, 12,  0, 13, 149,   0, 139 },
	{     11, 13, 11, 12, 139,   0, 149 },
	{     14,  0,  0, 13,   0,   0, 139 },
	{     14, 13,  0, 13, 139,   0, 139 },
	{     14,  2,  0, 13, 148,   0, 139 },
	{     14, 12,  0, 13, 149,   0, 139 },
	{     14, 13, 11, 12, 139,   0, 149 },
	{     10,  0, 13,  0,   0, 140,   0 },
	{     10, 13, 13,  0, 140, 140,   0 },
	{     10,  0,  1,  0,   0, 146,   0 },
	{     10, 13, 11,  0, 140, 147,   0 },
	{     12,  0, 13,  0,   0, 140,   0 },
	{     12, 13, 13,  0, 140, 140,   0 },
	{     12,  0,  1,  0,   0, 146,   0 },
	{     12, 13, 11,  0, 140, 147,   0 },
	{      3, 13, 11, 12, 150,   0,   0 }
	// clang-format on
};

// BUGFIX: This array should contain an additional 0 (207 elements).
/** Maps tile IDs to their corresponding base tile ID. */
const BYTE BSTYPES[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 10, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 14, 5, 14,
	10, 4, 14, 4, 5, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	2, 3, 4, 1, 6, 7, 16, 17, 2, 1,
	1, 2, 2, 1, 1, 2, 2, 2, 2, 2,
	1, 1, 11, 1, 13, 13, 13, 1, 2, 1,
	2, 1, 2, 1, 2, 2, 2, 2, 12, 0,
	0, 11, 1, 11, 1, 13, 0, 0, 0, 0,
	0, 0, 0, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 1, 11, 2, 12,
	13, 13, 13, 12, 2, 1, 2, 2, 4, 14,
	4, 10, 13, 13, 4, 4, 1, 1, 4, 2,
	2, 13, 13, 13, 13, 25, 26, 28, 30, 31,
	41, 43, 40, 41, 42, 43, 25, 41, 43, 28,
	28, 1, 2, 25, 26, 22, 22, 25, 26, 0,
	0, 0, 0, 0, 0, 0, 0
};

// BUGFIX: This array should contain an additional 0 (207 elements) (fixed).
/** Maps tile IDs to their corresponding undecorated tile ID. */
const BYTE L1BTYPES[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
	0, 0, 0, 0, 0, 25, 26, 0, 28, 0,
	30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
	40, 41, 42, 43, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 79,
	80, 0, 82, 0, 0, 0, 0, 0, 0, 79,
	0, 80, 0, 0, 79, 80, 0, 2, 2, 2,
	1, 1, 11, 25, 13, 13, 13, 1, 2, 1,
	2, 1, 2, 1, 2, 2, 2, 2, 12, 0,
	0, 11, 1, 11, 1, 13, 0, 0, 0, 0,
	0, 0, 0, 13, 13, 13, 13, 13, 13, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0
};
/** Miniset: stairs up on a corner wall. */
const BYTE STAIRSUP[] = {
	// clang-format off
	4, 4, // width, height

	13, 13, 13, 13, // search
	 2,  2,  2,  2,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0, 66,  6,  0, // replace
	63, 64, 65,  0,
	 0, 67, 68,  0,
	 0,  0,  0,  0,
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
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L5USTAIRS[] = {
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
	58, 59, 60,  0,
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
#else
/** Miniset: stairs up. */
const BYTE L1USTAIRS[] = {
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
#endif
/** Miniset: stairs down. */
const BYTE L1DSTAIRS[] = {
	// clang-format off
	4, 3, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,

	62, 57, 58,  0, // replace
	61, 59, 60,  0,
	 0,  0,  0,  0,
/*  124,  2,   106,107,   110,111,     0,  0,	// MegaTiles
	125,126,   108,109,   112,113,     0,  0,

	122,123,   114,115,   118,119,     0,  0,
	  7,  4,   116,117,   120,121,     0,  0,

	  0,  0,   141,142,   143,  2,     0,  0,
	  0,  0,     7,  4,     7,  4,     0,  0,

	  0,  0,     0,  0,     0,  0,     0,  0,
	  0,  0,     0,  0,     0,  0,     0,  0, */
	// clang-format on
};
#ifdef HELLFIRE
const BYTE L5DSTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	13, 13, 13, 13, // search
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,
	13, 13, 13, 13,

	 0,  0, 52,  0, // replace
	 0, 48, 51,  0,
	 0, 47, 50,  0,
	45, 46, 49,  0,
	 0,  0,  0,  0,
	// clang-format on
};
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
};
#endif
/** Miniset: candlestick. */
const BYTE LAMPS[] = {
	// clang-format off
	2, 2, // width, height

	13,  0, // search
	13, 13,

	129,   0, // replace
	130, 128,
	// clang-format on
};
/** Miniset: Poisoned Water Supply entrance. */
const BYTE PWATERIN[] = {
	// clang-format off
	6, 6, // width, height

	13, 13, 13, 13, 13, 13, // search
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13,

	 0,   0,   0,   0,   0, 0, // replace
	 0, 202, 200, 200,  84, 0,
	 0, 199, 203, 203,  83, 0,
	 0,  85, 206,  80,  81, 0,
	 0,   0, 134, 135,   0, 0,
	 0,   0,   0,   0,   0, 0,
	// clang-format on
};
#ifdef HELLFIRE
const BYTE byte_48A1B4[4] = { 1, 1, 11, 95 };
const BYTE byte_48A1B8[8] = { 1, 1, 12, 96 };
const BYTE byte_48A1C0[8] = {
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
const BYTE byte_48A1C8[8] = {
	// clang-format off
	3, 1, // width, height

	 2,  2,  2, // search

	94, 93, 92, // replace
	// clang-format on
};
const BYTE byte_48A1D0[4] = { 1, 1, 13, 97 };
const BYTE byte_48A1D4[4] = { 1, 1, 13, 98 };
const BYTE byte_48A1D8[4] = { 1, 1, 13, 99 };
const BYTE byte_48A1DC[4] = { 1, 1, 13, 100 };
const BYTE byte_48A1E0[20] = {
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
const BYTE byte_48A1F4[4] = { 1, 1, 11, 185 };
const BYTE byte_48A1F8[4] = { 1, 1, 11, 186 };
const BYTE byte_48A1FC[4] = { 1, 1, 12, 187 };
const BYTE byte_48A200[4] = { 1, 1, 12, 188 };
const BYTE byte_48A204[4] = { 1, 1, 89, 173 };
const BYTE byte_48A208[4] = { 1, 1, 89, 174 };
const BYTE byte_48A20C[4] = { 1, 1, 90, 175 };
const BYTE byte_48A210[4] = { 1, 1, 90, 176 };
const BYTE byte_48A214[4] = { 1, 1, 91, 177 };
const BYTE byte_48A218[4] = { 1, 1, 91, 178 };
const BYTE byte_48A21C[4] = { 1, 1, 92, 179 };
const BYTE byte_48A220[4] = { 1, 1, 92, 180 };
const BYTE byte_48A224[4] = { 1, 1, 92, 181 };
const BYTE byte_48A228[4] = { 1, 1, 92, 182 };
const BYTE byte_48A22C[4] = { 1, 1, 92, 183 };
const BYTE byte_48A230[4] = { 1, 1, 92, 184 };
const BYTE byte_48A234[4] = { 1, 1, 98, 189 };
const BYTE byte_48A238[4] = { 1, 1, 98, 190 };
const BYTE byte_48A23C[4] = { 1, 1, 97, 191 };
const BYTE byte_48A240[4] = { 1, 1, 15, 192 };
const BYTE byte_48A244[4] = { 1, 1, 99, 193 };
const BYTE byte_48A248[4] = { 1, 1, 99, 194 };
const BYTE byte_48A24C[4] = { 1, 1, 100, 195 };
const BYTE byte_48A250[4] = { 1, 1, 101, 196 };
const BYTE byte_48A254[4] = { 1, 1, 101, 197 };
const BYTE byte_48A258[8] = { 1, 1, 101, 198 };
const BYTE byte_48A260[24] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 167, 0,
	0,   0, 0,
	// clang-format on
};
const BYTE byte_48A278[24] = {
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
const BYTE byte_48A290[24] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	0,   0, 0, // replace
	0, 169, 0,
	0,   0, 0,
};
const BYTE byte_48A2A8[24] = {
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
const BYTE byte_48A2C0[24] = {
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
const BYTE byte_48A2D8[20] = {
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
const BYTE byte_48A2EC[4] = { 1, 1, 13, 163 };
const BYTE byte_48A2F0[4] = { 1, 1, 13, 164 };
const BYTE byte_48A2F4[4] = { 1, 1, 13, 165 };
const BYTE byte_48A2F8[4] = { 1, 1, 13, 166 };
const BYTE byte_48A2FC[4] = { 1, 1, 1, 112 };
const BYTE byte_48A300[4] = { 1, 1, 2, 113 };
const BYTE byte_48A304[4] = { 1, 1, 3, 114 };
const BYTE byte_48A308[4] = { 1, 1, 4, 115 };
const BYTE byte_48A30C[4] = { 1, 1, 5, 116 };
const BYTE byte_48A310[4] = { 1, 1, 6, 117 };
const BYTE byte_48A314[4] = { 1, 1, 7, 118 };
const BYTE byte_48A318[4] = { 1, 1, 8, 119 };
const BYTE byte_48A31C[4] = { 1, 1, 9, 120 };
const BYTE byte_48A320[4] = { 1, 1, 10, 121 };
const BYTE byte_48A324[4] = { 1, 1, 11, 122 };
const BYTE byte_48A328[4] = { 1, 1, 12, 123 };
const BYTE byte_48A32C[4] = { 1, 1, 13, 124 };
const BYTE byte_48A330[4] = { 1, 1, 14, 125 };
const BYTE byte_48A334[4] = { 1, 1, 15, 126 };
const BYTE byte_48A338[4] = { 1, 1, 16, 127 };
const BYTE byte_48A33C[4] = { 1, 1, 17, 128 };
const BYTE byte_48A340[4] = { 1, 1, 1, 129 };
const BYTE byte_48A344[4] = { 1, 1, 2, 130 };
const BYTE byte_48A348[4] = { 1, 1, 3, 131 };
const BYTE byte_48A34C[4] = { 1, 1, 4, 132 };
const BYTE byte_48A350[4] = { 1, 1, 5, 133 };
const BYTE byte_48A354[4] = { 1, 1, 6, 134 };
const BYTE byte_48A358[4] = { 1, 1, 7, 135 };
const BYTE byte_48A35C[4] = { 1, 1, 8, 136 };
const BYTE byte_48A360[4] = { 1, 1, 9, 137 };
const BYTE byte_48A364[4] = { 1, 1, 10, 138 };
const BYTE byte_48A368[4] = { 1, 1, 11, 139 };
const BYTE byte_48A36C[4] = { 1, 1, 12, 140 };
const BYTE byte_48A370[4] = { 1, 1, 13, 141 };
const BYTE byte_48A374[4] = { 1, 1, 14, 142 };
const BYTE byte_48A378[4] = { 1, 1, 15, 143 };
const BYTE byte_48A37C[4] = { 1, 1, 16, 144 };
const BYTE byte_48A380[4] = { 1, 1, 17, 145 };
const BYTE byte_48A384[4] = { 1, 1, 1, 146 };
const BYTE byte_48A388[4] = { 1, 1, 2, 147 };
const BYTE byte_48A38C[4] = { 1, 1, 3, 148 };
const BYTE byte_48A390[4] = { 1, 1, 4, 149 };
const BYTE byte_48A394[4] = { 1, 1, 5, 150 };
const BYTE byte_48A398[4] = { 1, 1, 6, 151 };
const BYTE byte_48A39C[4] = { 1, 1, 7, 152 };
const BYTE byte_48A3A0[4] = { 1, 1, 8, 153 };
const BYTE byte_48A3A4[4] = { 1, 1, 9, 154 };
const BYTE byte_48A3A8[4] = { 1, 1, 10, 155 };
const BYTE byte_48A3AC[4] = { 1, 1, 11, 156 };
const BYTE byte_48A3B0[4] = { 1, 1, 12, 157 };
const BYTE byte_48A3B4[4] = { 1, 1, 13, 158 };
const BYTE byte_48A3B8[4] = { 1, 1, 14, 159 };
const BYTE byte_48A3BC[4] = { 1, 1, 15, 160 };
const BYTE byte_48A3C0[4] = { 1, 1, 16, 161 };
const BYTE byte_48A3C4[4] = { 1, 1, 17, 162 };
const BYTE byte_48A3C8[4] = { 1, 1, 1, 199 };
const BYTE byte_48A3CC[4] = { 1, 1, 1, 201 };
const BYTE byte_48A3D0[4] = { 1, 1, 2, 200 };
const BYTE byte_48A3D4[4] = { 1, 1, 2, 202 };

const BYTE UberRoomPattern[] = {
	// clang-format off
	4, 6,					// width, height
	115, 130,   6,  13,
	129, 108,   1,  13,
	  1, 107, 103,  13,
	146, 106, 102,  13,
	129, 168,   1,  13,
	  7,   2,   3,  13
	// clang-format on
};
/*const BYTE CornerstoneRoomPattern[] = {
	// clang-format off
	5, 5,					// width, height
	  4,   2,   2,  2,  6,
	  1, 111, 172,  0,  1,
	  1, 172,   0,  0, 25,
	  1,   0,   0,  0,  1,
	  7,   2,   2,  2,  3
	// clang-format on
};*/
#endif

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L1ConvTbl[16] = { 22, 13, 1, 13, 2, 13, 13, 13, 4, 13, 1, 13, 2, 13, 16, 13 };

#ifdef HELLFIRE
static void DRLG_InitL5Vals()
{
	int i, *dp;
	char pc, *dsp;

	static_assert(sizeof(dPiece) == MAXDUNX * MAXDUNY * sizeof(int), "Linear traverse of dPiece does not work in DRLG_InitL5Vals.");
	static_assert(sizeof(dSpecial) == MAXDUNX * MAXDUNY, "Linear traverse of dSpecial does not work in DRLG_InitL5Vals.");
	dsp = &dSpecial[0][0];
	dp = &dPiece[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, dsp++, dp++) {
		if (*dp == 77)
			pc = 1;
		else if (*dp == 80)
			pc = 2;
		else
			continue;
		*dsp = pc;
	}
}
#endif

static void DRLG_L1PlaceDoor(int x, int y)
{
	if (!(L1dflags[x][y] & DLRG_PROTECTED)) {
		BYTE df = L1dflags[x][y] & 0x7F;
		BYTE c = dungeon[x][y];

		if (df == DLRG_HDOOR) {
			if (y != 1 && c == 2)
				dungeon[x][y] = 26;
			if (y != 1 && c == 7)
				dungeon[x][y] = 31;
			if (y != 1 && c == 14)
				dungeon[x][y] = 42;
			if (y != 1 && c == 4)
				dungeon[x][y] = 43;
			if (x != 1 && c == 1)
				dungeon[x][y] = 25;
			if (x != 1 && c == 10)
				dungeon[x][y] = 40;
			if (x != 1 && c == 6)
				dungeon[x][y] = 30;
		} else if (df == DLRG_VDOOR) {
			if (x != 1 && c == 1)
				dungeon[x][y] = 25;
			if (x != 1 && c == 6)
				dungeon[x][y] = 30;
			if (x != 1 && c == 10)
				dungeon[x][y] = 40;
			if (x != 1 && c == 4)
				dungeon[x][y] = 41;
			if (y != 1 && c == 2)
				dungeon[x][y] = 26;
			if (y != 1 && c == 14)
				dungeon[x][y] = 42;
			if (y != 1 && c == 7)
				dungeon[x][y] = 31;
		} else if (df == (DLRG_HDOOR | DLRG_VDOOR)) {
			if (x != 1 && y != 1 && c == 4)
				dungeon[x][y] = 28;
			if (x != 1 && c == 10)
				dungeon[x][y] = 40;
			if (y != 1 && c == 14)
				dungeon[x][y] = 42;
			if (y != 1 && c == 2)
				dungeon[x][y] = 26;
			if (x != 1 && c == 1)
				dungeon[x][y] = 25;
			if (y != 1 && c == 7)
				dungeon[x][y] = 31;
			if (x != 1 && c == 6)
				dungeon[x][y] = 30;
		}
	}

	L1dflags[x][y] = DLRG_PROTECTED;
}

#ifdef HELLFIRE
static void DRLG_L5Crypt_lavafloor()
{
	int i, j;

	for (j = 1; j < DMAXY; j++) {
		for (i = 1; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 5:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 7:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 8:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 9:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 10:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 11:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 12:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 14:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 15:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 17:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 95:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 96:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 208;
				break;
			case 116:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 118:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 119:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 120:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 121:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 122:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 211;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 212;
				break;
			case 123:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 125:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 126:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 128:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 133:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 135:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 136:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 137:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 213;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 214;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 138:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 139:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 215;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 216;
				break;
			case 140:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 217;
				break;
			case 142:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 143:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 213;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 214;
				break;
			case 145:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 213;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 214;
				break;
			case 150:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 217;
				break;
			case 152:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 153:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 154:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 155:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 156:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 157:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 217;
				break;
			case 159:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 160:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 206;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 207;
				break;
			case 162:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 209;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 210;
				break;
			case 167:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 209;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 210;
				break;
			case 187:
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 208;
				break;
			case 185:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 186:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 203;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 204;
				break;
			case 192:
				if (dungeon[i - 1][j] == 13)
					dungeon[i - 1][j] = 209;
				if (dungeon[i - 1][j - 1] == 13)
					dungeon[i - 1][j - 1] = 210;
				break;
			}
		}
	}
}
#endif

static void DRLG_L1Shadows()
{
	const ShadowStruct *ss;
	int x, y, i;
	BYTE sd00, sd10, sd01, sd11;
	BYTE tnv3;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			sd00 = BSTYPES[dungeon[x][y]];
			sd10 = BSTYPES[dungeon[x - 1][y]];
			sd01 = BSTYPES[dungeon[x][y - 1]];
			sd11 = BSTYPES[dungeon[x - 1][y - 1]];

			ss = SPATS;
			for (i = 0; i < lengthof(SPATS); i++, ss++) {
				if (ss->strig == sd00) {
					if ((ss->s1 != 0 && ss->s1 != sd11)
					 || (ss->s2 != 0 && ss->s2 != sd01)
					 || (ss->s3 != 0 && ss->s3 != sd10))
						continue;
					if (ss->nv1 != 0 && !L1dflags[x - 1][y - 1])
						dungeon[x - 1][y - 1] = ss->nv1;
					if (ss->nv2 != 0 && !L1dflags[x][y - 1])
						dungeon[x][y - 1] = ss->nv2;
					if (ss->nv3 != 0 && !L1dflags[x - 1][y])
						dungeon[x - 1][y] = ss->nv3;
				}
			}
		}
	}

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			if (dungeon[x - 1][y] == 139 && !L1dflags[x - 1][y]) {
				tnv3 = dungeon[x][y];
				if (tnv3 == 29 || tnv3 == 32 || tnv3 == 35 || tnv3 == 37 || tnv3 == 38 || tnv3 == 39)
					tnv3 = 141;
				else
					tnv3 = 139;
				dungeon[x - 1][y] = tnv3;
			}
			if (dungeon[x - 1][y] == 149 && !L1dflags[x - 1][y]) {
				tnv3 = dungeon[x][y];
				if (tnv3 == 29 || tnv3 == 32 || tnv3 == 35 || tnv3 == 37 || tnv3 == 38 || tnv3 == 39)
					tnv3 = 153;
				else
					tnv3 = 149;
				dungeon[x - 1][y] = tnv3;
			}
			if (dungeon[x - 1][y] == 148 && !L1dflags[x - 1][y]) {
				tnv3 = dungeon[x][y];
				if (tnv3 == 29 || tnv3 == 32 || tnv3 == 35 || tnv3 == 37 || tnv3 == 38 || tnv3 == 39)
					tnv3 = 154;
				else
					tnv3 = 148;
				dungeon[x - 1][y] = tnv3;
			}
		}
	}
}

static bool DRLG_PlaceMiniSet(const BYTE *miniset, int numt, BOOL setview)
{
	int sx, sy, sw, sh, xx, yy, i, ii, tries, t;
	bool done;

	sw = miniset[0];
	sh = miniset[1];

	for (i = 0; i < numt; i++) {
		sx = random_(0, DMAXX - sw);
		sy = random_(0, DMAXY - sh);

		tries = 0;
		while (TRUE) {
			done = true;
			if (sx <= 12) {
				sx++;
				done = false;
			}
			if (sy <= 12) {
				sy++;
				done = false;
			}

			ii = 2;
			for (yy = sy; yy < sy + sh && done; yy++) {
				for (xx = sx; xx < sx + sw && done; xx++) {
					if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii])
						done = false;
					if (L1dflags[xx][yy] != 0)
						done = false;
					ii++;
				}
			}

			if (done)
				break;
			if (++tries > 4000)
				return false;

			if (++sx == DMAXX - sw) {
				sx = 0;
				if (++sy == DMAXY - sh)
					sy = 0;
			}
		}

		ii = sw * sh + 2;

		for (yy = sy; yy < sy + sh; yy++) {
			for (xx = sx; xx < sx + sw; xx++) {
				if (miniset[ii] != 0)
					dungeon[xx][yy] = miniset[ii];
				ii++;
			}
		}
	}

	if (miniset == PWATERIN) {
		t = TransVal;
		TransVal = 0;
		DRLG_MRectTrans(sx, sy + 2, sx + 5, sy + 4);
		TransVal = t;

		quests[Q_PWATER]._qtx = 2 * sx + DBORDERX + 5;
		quests[Q_PWATER]._qty = 2 * sy + DBORDERY + 6;
	}

	if (setview) {
		ViewX = 2 * sx + DBORDERX + 3;
		ViewY = 2 * sy + DBORDERY + 4;
	}

	return true;
}

static void DRLG_L1Floor()
{
	int i, j, rv;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (L1dflags[i][j] == 0 && dungeon[i][j] == 13) {
				rv = random_(0, 3);

				if (rv == 1)
					dungeon[i][j] = 162;
				if (rv == 2)
					dungeon[i][j] = 163;
			}
		}
	}
}

static void DRLG_LoadL1SP()
{
	L1setloadflag = false;
	if (QuestStatus(Q_LTBANNER)) {
		L1pSetPiece = LoadFileInMem("Levels\\L1Data\\Banner2.DUN", NULL);
		L1setloadflag = true;
	} else if (QuestStatus(Q_SKELKING) && gbMaxPlayers == 1) {
		L1pSetPiece = LoadFileInMem("Levels\\L1Data\\SKngDO.DUN", NULL);
		L1setloadflag = true;
	} else if (QuestStatus(Q_BUTCHER)) {
		L1pSetPiece = LoadFileInMem("Levels\\L1Data\\rnd6.DUN", NULL);
		L1setloadflag = true;
	}
}

static void DRLG_FreeL1SP()
{
	MemFreeDbg(L1pSetPiece);
}

static void DRLG_InitL1Vals()
{
	int i, pc, *dp;
	char *dsp;
	static_assert(sizeof(dPiece) == MAXDUNX * MAXDUNY * sizeof(int), "Linear traverse of dPiece does not work in DRLG_InitL1Vals.");
	static_assert(sizeof(dSpecial) == MAXDUNX * MAXDUNY, "Linear traverse of dSpecial does not work in DRLG_InitL1Vals.");
	dsp = &dSpecial[0][0];
	dp = &dPiece[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, dsp++, dp++) {
		if (*dp == 12)
			pc = 1;
		else if (*dp == 11)
			pc = 2;
		else if (*dp == 71)
			pc = 1;
		else if (*dp == 253)
			pc = 3;
		else if (*dp == 267)
			pc = 6;
		else if (*dp == 259)
			pc = 5;
		else if (*dp == 249)
			pc = 2;
		else if (*dp == 325)
			pc = 2;
		else if (*dp == 321)
			pc = 1;
		else if (*dp == 255)
			pc = 4;
		else if (*dp == 211)
			pc = 1;
		else if (*dp == 344)
			pc = 2;
		else if (*dp == 341)
			pc = 1;
		else if (*dp == 331)
			pc = 2;
		else if (*dp == 418)
			pc = 1;
		else if (*dp == 421)
			pc = 2;
		else
			continue;
		*dsp = pc;
	}
}

static BYTE *LoadL1DungeonData(const char *sFileName)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm;

	//DRLG_InitTrans();
	pLevelMap = LoadFileInMem(sFileName, NULL);

	memset(L1dflags, 0, sizeof(L1dflags));
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL1DungeonData.");
	memset(dungeon, 22, sizeof(dungeon));

	lm = pLevelMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	lm += 2;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				dungeon[i][j] = *lm;
				L1dflags[i][j] |= DLRG_PROTECTED;
			} else {
				dungeon[i][j] = 13;
			}
			lm += 2;
		}
	}

	DRLG_L1Floor();
	return pLevelMap;
}

void LoadL1Dungeon(const char *sFileName, int vx, int vy)
{
	BYTE *pLevelMap;

	DRLG_InitTrans();

	pLevelMap = LoadL1DungeonData(sFileName);

	ViewX = vx;
	ViewY = vy;
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L1);
	DRLG_Init_Globals();
#ifdef HELLFIRE
	if (currlevel < 17)
#endif
		DRLG_InitL1Vals();
	SetMapMonsters(pLevelMap, 0, 0);
	SetMapObjects(pLevelMap, 0, 0);
	mem_free_dbg(pLevelMap);
}

void LoadPreL1Dungeon(const char *sFileName)
{
	BYTE *pLevelMap = LoadL1DungeonData(sFileName);

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	mem_free_dbg(pLevelMap);
}

static void InitL1Dungeon()
{
	memset(dungeon, 0, sizeof(dungeon));
	memset(L1dflags, 0, sizeof(L1dflags));
}

static void L1ClearFlags()
{
	int i;
	BYTE *pTmp;

	static_assert(sizeof(L1dflags) == DMAXX * DMAXY, "Linear traverse of L1dflags does not work in L1ClearFlags.");
	pTmp = &L1dflags[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++)
		*pTmp &= 0xBF;
}

static void L1drawRoom(int x, int y, int width, int height)
{
	int i, j, x2, y2;

	x2 = x + width;
	y2 = y + height;
	for (j = y; j < y2; j++) {
		for (i = x; i < x2; i++) {
			dungeon[i][j] = 1;
		}
	}
}

static bool L1checkRoom(int x, int y, int width, int height)
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

static void L1roomGen(int x, int y, int w, int h, int dir)
{
	int dirProb, i, width, height, rx, ry, rxy2;
	bool ran2;

	dirProb = random_(0, 4);

	if (dir == 1 ? dirProb == 0 : dirProb != 0) {
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2 + y - height / 2;
			rx = x - width;
			if (L1checkRoom(rx - 1, ry - 1, width + 1, height + 2))  /// BUGFIX: swap args 3 and 4 ("height+2" and "width+1") (fixed)
				break;
		}

		if (i != 0)
			L1drawRoom(rx, ry, width, height);
		rxy2 = x + w;
		ran2 = L1checkRoom(rxy2, ry - 1, width + 1, height + 2);
		if (ran2)
			L1drawRoom(rxy2, ry, width, height);
		if (i != 0)
			L1roomGen(rx, ry, width, height, 1);
		if (ran2)
			L1roomGen(rxy2, ry, width, height, 1);
	} else {
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2 + x - width / 2;
			ry = y - height;
			if (L1checkRoom(rx - 1, ry - 1, width + 2, height + 1))
				break;
		}

		if (i != 0)
			L1drawRoom(rx, ry, width, height);
		rxy2 = y + h;
		ran2 = L1checkRoom(rx - 1, rxy2, width + 2, height + 1);
		if (ran2)
			L1drawRoom(rx, rxy2, width, height);
		if (i != 0)
			L1roomGen(rx, ry, width, height, 0);
		if (ran2)
			L1roomGen(rx, rxy2, width, height, 0);
	}
}

static void L1firstRoom()
{
	int ys, ye, y;
	int xs, xe, x;

	if (random_(0, 2) == 0) {
		ys = 1;
		ye = DMAXY - 1;

		VR1 = random_(0, 2);
		VR2 = random_(0, 2);
		VR3 = random_(0, 2);

		if (!(VR1 & VR3))
			VR2 = TRUE;
		if (VR1)
			L1drawRoom(15, 1, 10, 10);
		else
			ys = 18;

		if (VR2)
			L1drawRoom(15, 15, 10, 10);
		if (VR3)
			L1drawRoom(15, 29, 10, 10);
		else
			ye = 22;

		for (y = ys; y < ye; y++) {
			dungeon[17][y] = 1;
			dungeon[18][y] = 1;
			dungeon[19][y] = 1;
			dungeon[20][y] = 1;
			dungeon[21][y] = 1;
			dungeon[22][y] = 1;
		}

		if (VR1)
			L1roomGen(15, 1, 10, 10, 0);
		if (VR2)
			L1roomGen(15, 15, 10, 10, 0);
		if (VR3)
			L1roomGen(15, 29, 10, 10, 0);

		HR3 = FALSE;
		HR2 = FALSE;
		HR1 = FALSE;
	} else {
		xs = 1;
		xe = DMAXX - 1;

		HR1 = random_(0, 2);
		HR2 = random_(0, 2);
		HR3 = random_(0, 2);

		if (!(HR1 & HR3))
			HR2 = TRUE;
		if (HR1)
			L1drawRoom(1, 15, 10, 10);
		else
			xs = 18;

		if (HR2)
			L1drawRoom(15, 15, 10, 10);
		if (HR3)
			L1drawRoom(29, 15, 10, 10);
		else
			xe = 22;

		for (x = xs; x < xe; x++) {
			dungeon[x][17] = 1;
			dungeon[x][18] = 1;
			dungeon[x][19] = 1;
			dungeon[x][20] = 1;
			dungeon[x][21] = 1;
			dungeon[x][22] = 1;
		}

		if (HR1)
			L1roomGen(1, 15, 10, 10, 1);
		if (HR2)
			L1roomGen(15, 15, 10, 10, 1);
		if (HR3)
			L1roomGen(29, 15, 10, 10, 1);

		VR3 = FALSE;
		VR2 = FALSE;
		VR1 = FALSE;
	}
}

static int L1GetArea()
{
	int i, rv;
	BYTE *pTmp;

	rv = 0;
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in L1GetArea.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		assert(*pTmp <= 1);
		rv += *pTmp;
	}

	return rv;
}

static void DRLG_L1MakeMegas()
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
			dungeon[i][j] = L1ConvTbl[v];
		}
	}
	for (j = 0; j < DMAXY; j++)
		dungeon[DMAXX - 1][j] = 22;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = 22;
}

static int L1HWallOk(int i, int j)
{
	int x;
	BYTE bv;

	for (x = 1; dungeon[i + x][j] == 13; x++) {
		if (dungeon[i + x][j - 1] != 13 || dungeon[i + x][j + 1] != 13 || L1dflags[i + x][j])
			break;
	}

	if (x != 1) {
		bv = dungeon[i + x][j];
		if ((bv >= 3 && bv <= 7)
		 || (bv >= 16 && bv <= 24 && bv != 22))
			return x;
	}
	return -1;
}

static int L1VWallOk(int i, int j)
{
	int y;
	BYTE bv;

	for (y = 1; dungeon[i][j + y] == 13; y++) {
		if (dungeon[i - 1][j + y] != 13 || dungeon[i + 1][j + y] != 13 || L1dflags[i][j + y])
			break;
	}

	if (y != 1) {
		bv = dungeon[i][j + y];
		if ((bv >= 3 && bv <= 7)
		 || (bv >= 16 && bv <= 24 && bv != 22))
			return y;
	}
	return -1;
}

static void L1HorizWall(int i, int j, char pn, int dx)
{
	int xx;
	char wt, dt;

	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 2;
		break;
	case 2:
		dt = 12;
		if (pn == 2)
			pn = 12;
		if (pn == 4)
			pn = 10;
		break;
	case 3:
		dt = 36;
		if (pn == 2)
			pn = 36;
		if (pn == 4)
			pn = 27;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (random_(0, 6) == 5)
		wt = 12;
	else
		wt = 26;
	if (dt == 12)
		wt = 12;

	dungeon[i][j] = pn;

	for (xx = 1; xx < dx; xx++) {
		dungeon[i + xx][j] = dt;
	}

	xx = RandRange(1, dx - 1);

	if (wt == 12) {
		dungeon[i + xx][j] = 12;
	} else {
		dungeon[i + xx][j] = 2;
		L1dflags[i + xx][j] |= DLRG_HDOOR;
	}
}

static void L1VertWall(int i, int j, char pn, int dy)
{
	int yy;
	char wt, dt;

	switch (random_(0, 4)) {
	case 0:
	case 1:
		dt = 1;
		break;
	case 2:
		dt = 11;
		if (pn == 1)
			pn = 11;
		if (pn == 4)
			pn = 14;
		break;
	case 3:
		dt = 35;
		if (pn == 1)
			pn = 35;
		if (pn == 4)
			pn = 37;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (random_(0, 6) == 5)
		wt = 11;
	else
		wt = 25;
	if (dt == 11)
		wt = 11;

	dungeon[i][j] = pn;

	for (yy = 1; yy < dy; yy++) {
		dungeon[i][j + yy] = dt;
	}

	yy = RandRange(1, dy - 1);

	if (wt == 11) {
		dungeon[i][j + yy] = 11;
	} else {
		dungeon[i][j + yy] = 1;
		L1dflags[i][j + yy] |= DLRG_VDOOR;
	}
}

static void L1AddWall()
{
	int i, j, x, y;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (!L1dflags[i][j]) {
				if (dungeon[i][j] == 3 && random_(0, 100) < 100) {
					x = L1HWallOk(i, j);
					if (x != -1)
						L1HorizWall(i, j, 2, x);
				}
				if (dungeon[i][j] == 3 && random_(0, 100) < 100) {
					y = L1VWallOk(i, j);
					if (y != -1)
						L1VertWall(i, j, 1, y);
				}
				if (dungeon[i][j] == 6 && random_(0, 100) < 100) {
					x = L1HWallOk(i, j);
					if (x != -1)
						L1HorizWall(i, j, 4, x);
				}
				if (dungeon[i][j] == 7 && random_(0, 100) < 100) {
					y = L1VWallOk(i, j);
					if (y != -1)
						L1VertWall(i, j, 4, y);
				}
				if (dungeon[i][j] == 2 && random_(0, 100) < 100) {
					x = L1HWallOk(i, j);
					if (x != -1)
						L1HorizWall(i, j, 2, x);
				}
				if (dungeon[i][j] == 1 && random_(0, 100) < 100) {
					y = L1VWallOk(i, j);
					if (y != -1)
						L1VertWall(i, j, 1, y);
				}
			}
		}
	}
}

static void DRLG_L1GChamber(int sx, int sy, BOOL topflag, BOOL bottomflag, BOOL leftflag, BOOL rightflag)
{
	int i, j;

	if (topflag) {
		dungeon[sx + 2][sy] = 12;
		dungeon[sx + 3][sy] = 12;
		dungeon[sx + 4][sy] = 3;
		dungeon[sx + 7][sy] = 9;
		dungeon[sx + 8][sy] = 12;
		dungeon[sx + 9][sy] = 2;
	}
	if (bottomflag) {
		sy += 11;
		dungeon[sx + 2][sy] = 10;
		dungeon[sx + 3][sy] = 12;
		dungeon[sx + 4][sy] = 8;
		dungeon[sx + 7][sy] = 5;
		dungeon[sx + 8][sy] = 12;
		if (dungeon[sx + 9][sy] != 4) {
			dungeon[sx + 9][sy] = 21;
		}
		sy -= 11;
	}
	if (leftflag) {
		dungeon[sx][sy + 2] = 11;
		dungeon[sx][sy + 3] = 11;
		dungeon[sx][sy + 4] = 3;
		dungeon[sx][sy + 7] = 8;
		dungeon[sx][sy + 8] = 11;
		dungeon[sx][sy + 9] = 1;
	}
	if (rightflag) {
		sx += 11;
		dungeon[sx][sy + 2] = 14;
		dungeon[sx][sy + 3] = 11;
		dungeon[sx][sy + 4] = 9;
		dungeon[sx][sy + 7] = 5;
		dungeon[sx][sy + 8] = 11;
		if (dungeon[sx][sy + 9] != 4) {
			dungeon[sx][sy + 9] = 21;
		}
		sx -= 11;
	}

	for (j = 1; j < 11; j++) {
		for (i = 1; i < 11; i++) {
			dungeon[i + sx][j + sy] = 13;
			L1dflags[i + sx][j + sy] |= DLRG_CHAMBER;
		}
	}

	dungeon[sx + 4][sy + 4] = 15;
	dungeon[sx + 7][sy + 4] = 15;
	dungeon[sx + 4][sy + 7] = 15;
	dungeon[sx + 7][sy + 7] = 15;
}

static void DRLG_L1GHall(int x1, int y1, int x2, int y2)
{
	int i;

	if (y1 == y2) {
		for (i = x1; i < x2; i++) {
			dungeon[i][y1] = 12;
			dungeon[i][y1 + 3] = 12;
		}
	} else {
		for (i = y1; i < y2; i++) {
			dungeon[x1][i] = 11;
			dungeon[x1 + 3][i] = 11;
		}
	}
}

static void L1tileFix()
{
	int i, j;
	BYTE v1, v2;

	// BUGFIX: Bounds checks are required in all loop bodies.
	// See https://github.com/diasurgical/devilutionX/pull/401

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			v1 = dungeon[i][j];
			if (i + 1 < DMAXX) {
				v2 = dungeon[i + 1][j];
				if (v1 == 2 && v2 == 22)
					dungeon[i + 1][j] = 23;
				if (v1 == 13 && v2 == 22)
					dungeon[i + 1][j] = 18;
				if (v1 == 13 && v2 == 2)
					dungeon[i + 1][j] = 7;
				if (v1 == 6 && v2 == 22)
					dungeon[i + 1][j] = 24;
			}
			if (j + 1 < DMAXY) {
				v2 = dungeon[i][j + 1];
				if (v1 == 1 && v2 == 22)
					dungeon[i][j + 1] = 24;
				if (v1 == 13 && v2 == 1)
					dungeon[i][j + 1] = 6;
				if (v1 == 13 && v2 == 22)
					dungeon[i][j + 1] = 19;
			}
		}
	}

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			v1 = dungeon[i][j];
			if (i + 1 < DMAXX) {
				v2 = dungeon[i + 1][j];
				if (v1 == 13 && v2 == 19)
					dungeon[i + 1][j] = 21;
				if (v1 == 13 && v2 == 22)
					dungeon[i + 1][j] = 20;
				if (v1 == 13 && v2 == 24)
					dungeon[i + 1][j] = 21;
				if (v1 == 7 && v2 == 22)
					dungeon[i + 1][j] = 23;
				if (v1 == 7 && v2 == 19)
					dungeon[i + 1][j] = 21;
				if (v1 == 7 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 7 && v2 == 24)
					dungeon[i + 1][j] = 21;
				if (v1 == 7 && v2 == 13)
					dungeon[i + 1][j] = 17;
				if (v1 == 19 && v2 == 22)
					dungeon[i + 1][j] = 20;
				if (v1 == 19 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 2 && v2 == 19)
					dungeon[i + 1][j] = 21;
				if (v1 == 2 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 2 && v2 == 24)
					dungeon[i + 1][j] = 21;
				if (v1 == 2 && v2 == 13)
					dungeon[i + 1][j] = 17;
				if (v1 == 3 && v2 == 22)
					dungeon[i + 1][j] = 24;
				if (v1 == 21 && v2 == 1)
					dungeon[i + 1][j] = 6;
				if (v1 == 4 && v2 == 16)
					dungeon[i + 1][j] = 17;
			}
			if (i != 0) {
				v2 = dungeon[i - 1][j];
				if (v1 == 23 && v2 == 22)
					dungeon[i - 1][j] = 19;
				if (v1 == 19 && v2 == 23)
					dungeon[i - 1][j] = 21;
				if (v1 == 6 && v2 == 22)
					dungeon[i - 1][j] = 24;
				if (v1 == 6 && v2 == 23)
					dungeon[i - 1][j] = 21;
			}
			if (j + 1 < DMAXY) {
				v2 = dungeon[i][j + 1];
				if (v1 == 1 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 1 && v2 == 13)
					dungeon[i][j + 1] = 16;
				if (v1 == 6 && v2 == 18)
					dungeon[i][j + 1] = 21;
				if (v1 == 6 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 6 && v2 == 22)
					dungeon[i][j + 1] = 24;
				if (v1 == 6 && v2 == 13)
					dungeon[i][j + 1] = 16;
				if (v1 == 18 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 21 && v2 == 2)
					dungeon[i][j + 1] = 7;
				if (v1 == 13 && v2 == 16)
					dungeon[i][j + 1] = 17;
			}
			if (j != 0) {
				v2 = dungeon[i][j - 1];
				if (v1 == 6 && v2 == 22)
					dungeon[i][j - 1] = 7;
				if (v1 == 7 && v2 == 24)
					dungeon[i][j - 1] = 21;
				if (v1 == 18 && v2 == 24)
					dungeon[i][j - 1] = 21;
			}
		}
	}

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			v1 = dungeon[i][j];
			if (v1 == 4 && j + 1 < DMAXY && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 7;
			if (v1 == 2 && i + 1 < DMAXX && dungeon[i + 1][j] == 19)
				dungeon[i + 1][j] = 21;
			if (v1 == 18 && j + 1 < DMAXY && dungeon[i][j + 1] == 22)
				dungeon[i][j + 1] = 20;
		}
	}
}

#ifdef HELLFIRE
static void DRLG_L5Crypt_rndset(const BYTE *miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii, kk;
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
					if (dflags[xx + sx][yy + sy] != 0) {
						found = false;
					}
					ii++;
				}
			}
			kk = sw * sh + 2;
			if (miniset[kk] >= 84 && miniset[kk] <= 100 && found) {
				// BUGFIX: accesses to dungeon can go out of bounds (fixed)
				// BUGFIX: Comparisons vs 100 should use same tile as comparisons vs 84 (fixed)
				if (sx > 0 && dungeon[sx - 1][sy] >= 84 && dungeon[sx - 1][sy] <= 100) {
					found = false;
				}
				if (dungeon[sx + 1][sy] >= 84 && dungeon[sx + 1][sy] <= 100) {
					found = false;
				}
				if (dungeon[sx][sy + 1] >= 84 && dungeon[sx][sy + 1] <= 100) {
					found = false;
				}
				if (sy > 0 && dungeon[sx][sy - 1] >= 84 && dungeon[sx][sy - 1] <= 100) {
					found = false;
				}
			}
			if (found && random_(0, 100) < rndper) {
				for (yy = 0; yy < sh; yy++) {
					for (xx = 0; xx < sw; xx++) {
						if (miniset[kk] != 0) {
							dungeon[xx + sx][yy + sy] = miniset[kk];
						}
						kk++;
					}
				}
			}
		}
	}
}

static void DRLG_L5CryptSetRoom(int rx1, int ry1)
{
	int rw, rh, i, j, sp;

	rw = UberRoomPattern[0];
	rh = UberRoomPattern[1];

	UberRow = 2 * rx1 + 6;
	UberCol = 2 * ry1 + 8;
	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;
	gbUberRoomOpened = false;
	gbUberLeverActivated = false;

	sp = 2;

	rw += rx1;
	rh += ry1;
	for (j = ry1; j < rh; j++) {
		for (i = rx1; i < rw; i++) {
			assert(UberRoomPattern[sp] != 0);
			dungeon[i][j] = UberRoomPattern[sp];
			L1dflags[i][j] |= DLRG_PROTECTED;
			sp++;
		}
	}
}

/*static void DRLG_L5CryptSetCornerRoom(int rx1, int ry1)
{
	int rw, rh, i, j, sp;

	rw = CornerstoneRoomPattern[0];
	rh = CornerstoneRoomPattern[1];

	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;

	sp = 2;

	rw += rx1;
	rh += ry1;
	for (j = ry1; j < rh; j++) {
		for (i = rx1; i < rw; i++) {
			if (CornerstoneRoomPattern[sp] != 0) {
				dungeon[i][j] = CornerstoneRoomPattern[sp];
				L1dflags[i][j] |= DLRG_PROTECTED;
			} else {
				dungeon[i][j] = 13;
			}
			sp++;
		}
	}
}*/

#endif

static void DRLG_L1Subs()
{
	int x, y, rv, i;

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (random_(0, 4) == 0) {
				BYTE c = L1BTYPES[dungeon[x][y]];

				if (c != 0 && !L1dflags[x][y]) {
					rv = random_(0, 16);
					i = -1;

					while (rv >= 0) {
						if (++i == sizeof(L1BTYPES))
							i = 0;
						if (c == L1BTYPES[i])
							rv--;
					}

					// BUGFIX: Add `&& y > 0` to the if statement. (fixed)
					if (i == 89 && y > 0) {
						if (L1BTYPES[dungeon[x][y - 1]] != 79 || L1dflags[x][y - 1])
							i = 79;
						else
							dungeon[x][y - 1] = 90;
					}
					// BUGFIX: Add `&& x + 1 < DMAXX` to the if statement. (fixed)
					if (i == 91 && x + 1 < DMAXX) {
						if (L1BTYPES[dungeon[x + 1][y]] != 80 || L1dflags[x + 1][y])
							i = 80;
						else
							dungeon[x + 1][y] = 92;
					}
					dungeon[x][y] = i;
				}
			}
		}
	}
}

static void DRLG_L1SetRoom(int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE *sp;

	rw = *L1pSetPiece;
	rh = *(L1pSetPiece + 2);

	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = rw;
	setpc_h = rh;

	sp = L1pSetPiece + 4;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*sp) {
				dungeon[rx1 + i][ry1 + j] = *sp;
				L1dflags[rx1 + i][ry1 + j] |= DLRG_PROTECTED;
			} else {
				dungeon[rx1 + i][ry1 + j] = 13;
			}
			sp += 2;
		}
	}
}

static void L1FillChambers()
{
	int c;

	if (HR1)
		DRLG_L1GChamber(0, 14, FALSE, FALSE, FALSE, TRUE);

	if (HR2) {
		DRLG_L1GChamber(14, 14, FALSE, FALSE, HR1, HR3);
	}

	if (HR3)
		DRLG_L1GChamber(28, 14, FALSE, FALSE, TRUE, FALSE);
	if (HR1 && HR2)
		DRLG_L1GHall(12, 18, 14, 18);
	if (HR2 && HR3)
		DRLG_L1GHall(26, 18, 28, 18);
	if (HR1 && !HR2 && HR3)
		DRLG_L1GHall(12, 18, 28, 18);
	if (VR1)
		DRLG_L1GChamber(14, 0, FALSE, TRUE, FALSE, FALSE);

	if (VR2) {
		DRLG_L1GChamber(14, 14, VR1, VR3, FALSE, FALSE);
	}

	if (VR3)
		DRLG_L1GChamber(14, 28, TRUE, FALSE, FALSE, FALSE);
	if (VR1 && VR2)
		DRLG_L1GHall(18, 12, 18, 14);
	if (VR2 && VR3)
		DRLG_L1GHall(18, 26, 18, 28);
	if (VR1 && !VR2 && VR3)
		DRLG_L1GHall(18, 12, 18, 28);

#ifdef HELLFIRE
	if (currlevel == 24) {
		if (VR1 || VR2 || VR3) {
			c = 1;
			if (!VR1 && VR2 && VR3 && random_(0, 2) != 0)
				c = 2;
			if (VR1 && VR2 && !VR3 && random_(0, 2) != 0)
				c = 0;

			if (VR1 && !VR2 && VR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (VR1 && VR2 && VR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L5CryptSetRoom(16, c);
		} else {
			c = 1;
			if (!HR1 && HR2 && HR3 && random_(0, 2) != 0)
				c = 2;
			if (HR1 && HR2 && !HR3 && random_(0, 2) != 0)
				c = 0;

			if (HR1 && !HR2 && HR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (HR1 && HR2 && HR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L5CryptSetRoom(c, 16);
		}
	}/* else if (currlevel == 21) {
		if (VR1 || VR2 || VR3) {
			c = 1;
			if (!VR1 && VR2 && VR3 && random_(0, 2) != 0)
				c = 2;
			if (VR1 && VR2 && !VR3 && random_(0, 2) != 0)
				c = 0;

			if (VR1 && !VR2 && VR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (VR1 && VR2 && VR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L5CryptSetCornerRoom(16, c);
		} else {
			c = 1;
			if (!HR1 && HR2 && HR3 && random_(0, 2) != 0)
				c = 2;
			if (HR1 && HR2 && !HR3 && random_(0, 2) != 0)
				c = 0;

			if (HR1 && !HR2 && HR3) {
				if (random_(0, 2))
					c = 0;
				else
					c = 2;
			}

			if (HR1 && HR2 && HR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L5CryptSetCornerRoom(c, 16);
		}
	}*/
#endif
	if (L1setloadflag) {
		if (VR1 || VR2 || VR3) {
			c = 1;
			if (!VR1 && VR2 && VR3 && random_(0, 2) != 0)
				c = 2;
			if (VR1 && VR2 && !VR3 && random_(0, 2) != 0)
				c = 0;

			if (VR1 && !VR2 && VR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (VR1 && VR2 && VR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L1SetRoom(16, c);
		} else {
			c = 1;
			if (!HR1 && HR2 && HR3 && random_(0, 2) != 0)
				c = 2;
			if (HR1 && HR2 && !HR3 && random_(0, 2) != 0)
				c = 0;

			if (HR1 && !HR2 && HR3) {
				if (random_(0, 2) != 0)
					c = 0;
				else
					c = 2;
			}

			if (HR1 && HR2 && HR3)
				c = random_(0, 3);

			switch (c) {
			case 0:
				c = 2;
				break;
			case 1:
				c = 16;
				break;
			case 2:
				c = 30;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			DRLG_L1SetRoom(c, 16);
		}
	}
}

#ifdef HELLFIRE
static void DRLG_L5Crypt_pattern1(int rndper)
{
	DRLG_L5Crypt_rndset(byte_48A3C8, rndper);
	DRLG_L5Crypt_rndset(byte_48A3CC, rndper);
	DRLG_L5Crypt_rndset(byte_48A3D0, rndper);
	DRLG_L5Crypt_rndset(byte_48A3D4, rndper);
}

static void DRLG_L5Crypt_pattern2(int rndper)
{
	DRLG_L5Crypt_rndset(byte_48A2FC, rndper);
	DRLG_L5Crypt_rndset(byte_48A300, rndper);
	DRLG_L5Crypt_rndset(byte_48A304, rndper);
	DRLG_L5Crypt_rndset(byte_48A308, rndper);
	DRLG_L5Crypt_rndset(byte_48A30C, rndper);
	DRLG_L5Crypt_rndset(byte_48A310, rndper);
	DRLG_L5Crypt_rndset(byte_48A314, rndper);
	DRLG_L5Crypt_rndset(byte_48A318, rndper);
	DRLG_L5Crypt_rndset(byte_48A31C, rndper);
	DRLG_L5Crypt_rndset(byte_48A320, rndper);
	DRLG_L5Crypt_rndset(byte_48A324, rndper);
	DRLG_L5Crypt_rndset(byte_48A328, rndper);
	DRLG_L5Crypt_rndset(byte_48A32C, rndper);
	DRLG_L5Crypt_rndset(byte_48A330, rndper);
	DRLG_L5Crypt_rndset(byte_48A334, rndper);
	DRLG_L5Crypt_rndset(byte_48A338, rndper);
	DRLG_L5Crypt_rndset(byte_48A33C, rndper);
}

static void DRLG_L5Crypt_pattern3(int rndper)
{
	DRLG_L5Crypt_rndset(byte_48A340, rndper);
	DRLG_L5Crypt_rndset(byte_48A344, rndper);
	DRLG_L5Crypt_rndset(byte_48A348, rndper);
	DRLG_L5Crypt_rndset(byte_48A34C, rndper);
	DRLG_L5Crypt_rndset(byte_48A350, rndper);
	DRLG_L5Crypt_rndset(byte_48A354, rndper);
	DRLG_L5Crypt_rndset(byte_48A358, rndper);
	DRLG_L5Crypt_rndset(byte_48A35C, rndper);
	DRLG_L5Crypt_rndset(byte_48A360, rndper);
	DRLG_L5Crypt_rndset(byte_48A364, rndper);
	DRLG_L5Crypt_rndset(byte_48A368, rndper);
	DRLG_L5Crypt_rndset(byte_48A36C, rndper);
	DRLG_L5Crypt_rndset(byte_48A370, rndper);
	DRLG_L5Crypt_rndset(byte_48A374, rndper);
	DRLG_L5Crypt_rndset(byte_48A378, rndper);
	DRLG_L5Crypt_rndset(byte_48A37C, rndper);
	DRLG_L5Crypt_rndset(byte_48A380, rndper);
}

static void DRLG_L5Crypt_pattern4(int rndper)
{
	DRLG_L5Crypt_rndset(byte_48A384, rndper);
	DRLG_L5Crypt_rndset(byte_48A388, rndper);
	DRLG_L5Crypt_rndset(byte_48A38C, rndper);
	DRLG_L5Crypt_rndset(byte_48A390, rndper);
	DRLG_L5Crypt_rndset(byte_48A394, rndper);
	DRLG_L5Crypt_rndset(byte_48A398, rndper);
	DRLG_L5Crypt_rndset(byte_48A39C, rndper);
	DRLG_L5Crypt_rndset(byte_48A3A0, rndper);
	DRLG_L5Crypt_rndset(byte_48A3A4, rndper);
	DRLG_L5Crypt_rndset(byte_48A3A8, rndper);
	DRLG_L5Crypt_rndset(byte_48A3AC, rndper);
	DRLG_L5Crypt_rndset(byte_48A3B0, rndper);
	DRLG_L5Crypt_rndset(byte_48A3B4, rndper);
	DRLG_L5Crypt_rndset(byte_48A3B8, rndper);
	DRLG_L5Crypt_rndset(byte_48A3BC, rndper);
	DRLG_L5Crypt_rndset(byte_48A3C0, rndper);
	DRLG_L5Crypt_rndset(byte_48A3C4, rndper);
}

static void DRLG_L5Crypt_pattern5(int rndper)
{
	DRLG_L5Crypt_rndset(byte_48A260, rndper);
	DRLG_L5Crypt_rndset(byte_48A278, rndper);
	DRLG_L5Crypt_rndset(byte_48A290, rndper);
	DRLG_L5Crypt_rndset(byte_48A2A8, rndper);
	DRLG_L5Crypt_rndset(byte_48A2C0, rndper);
	DRLG_L5Crypt_rndset(byte_48A2D8, rndper);
	DRLG_L5Crypt_rndset(byte_48A2EC, rndper);
	DRLG_L5Crypt_rndset(byte_48A2F0, rndper);
	DRLG_L5Crypt_rndset(byte_48A2F4, rndper);
	DRLG_L5Crypt_rndset(byte_48A2F8, rndper);
}

static void DRLG_L5Crypt_pattern6(int rndper)
{
	DRLG_L5Crypt_rndset(byte_48A1F4, rndper);
	DRLG_L5Crypt_rndset(byte_48A1FC, rndper);
	DRLG_L5Crypt_rndset(byte_48A1F8, rndper);
	DRLG_L5Crypt_rndset(byte_48A200, rndper);
	DRLG_L5Crypt_rndset(byte_48A204, rndper);
	DRLG_L5Crypt_rndset(byte_48A208, rndper);
	DRLG_L5Crypt_rndset(byte_48A20C, rndper);
	DRLG_L5Crypt_rndset(byte_48A210, rndper);
	DRLG_L5Crypt_rndset(byte_48A214, rndper);
	DRLG_L5Crypt_rndset(byte_48A218, rndper);
	DRLG_L5Crypt_rndset(byte_48A21C, rndper);
	DRLG_L5Crypt_rndset(byte_48A220, rndper);
	DRLG_L5Crypt_rndset(byte_48A224, rndper);
	DRLG_L5Crypt_rndset(byte_48A228, rndper);
	DRLG_L5Crypt_rndset(byte_48A22C, rndper);
	DRLG_L5Crypt_rndset(byte_48A230, rndper);
	DRLG_L5Crypt_rndset(byte_48A234, rndper);
	DRLG_L5Crypt_rndset(byte_48A238, rndper);
	DRLG_L5Crypt_rndset(byte_48A23C, rndper);
	DRLG_L5Crypt_rndset(byte_48A240, rndper);
	DRLG_L5Crypt_rndset(byte_48A244, rndper);
	DRLG_L5Crypt_rndset(byte_48A248, rndper);
	DRLG_L5Crypt_rndset(byte_48A24C, rndper);
	DRLG_L5Crypt_rndset(byte_48A250, rndper);
	DRLG_L5Crypt_rndset(byte_48A254, rndper);
	DRLG_L5Crypt_rndset(byte_48A258, rndper);
}

static void DRLG_L5Crypt_pattern7(int rndper)
{
	DRLG_L5Crypt_rndset(byte_48A1D0, rndper);
	DRLG_L5Crypt_rndset(byte_48A1D4, rndper);
	DRLG_L5Crypt_rndset(byte_48A1D8, rndper);
	DRLG_L5Crypt_rndset(byte_48A1DC, rndper);
}
#endif

static void DRLG_L1FTVR(int i, int j, int x, int y, int dir)
{
	if (dTransVal[x][y] || dungeon[i][j] != 13) {
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
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else {
		dTransVal[x][y] = TransVal;
		dTransVal[x + 1][y] = TransVal;
		dTransVal[x][y + 1] = TransVal;
		dTransVal[x + 1][y + 1] = TransVal;
		DRLG_L1FTVR(i + 1, j, x + 2, y, 1);
		DRLG_L1FTVR(i - 1, j, x - 2, y, 2);
		DRLG_L1FTVR(i, j + 1, x, y + 2, 3);
		DRLG_L1FTVR(i, j - 1, x, y - 2, 4);
		DRLG_L1FTVR(i - 1, j - 1, x - 2, y - 2, 5);
		DRLG_L1FTVR(i + 1, j - 1, x + 2, y - 2, 6);
		DRLG_L1FTVR(i - 1, j + 1, x - 2, y + 2, 7);
		DRLG_L1FTVR(i + 1, j + 1, x + 2, y + 2, 8);
	}
}

static void DRLG_L1FloodTVal()
{
	int xx, yy, i, j;

	yy = DBORDERY;

	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;

		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 13 && !dTransVal[xx][yy]) {
				DRLG_L1FTVR(i, j, xx, yy, 0);
				TransVal++;
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void DRLG_L1TransFix()
{
	int xx, yy, i, j;

	yy = DBORDERY;

	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;

		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
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
				break;
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void DRLG_L1DirtFix()
{
	int i, j;
#ifdef HELLFIRE
	BYTE bv;

	if (currlevel >= 21) {
		for (j = 0; j < DMAXY - 1; j++) {
			for (i = 0; i < DMAXX - 1; i++) {
				switch (dungeon[i][j]) {
				case 18: bv = 82; break;
				case 19: bv = 83; break;
				case 21: bv = 85; break;
				case 23: bv = 87; break;
				case 24: bv = 88; break;
				default: continue;
				}
				dungeon[i][j] = bv;
			}
		}
		return;
	}
#endif
	for (j = 0; j < DMAXY - 1; j++) {
		for (i = 0; i < DMAXX - 1; i++) {
			switch (dungeon[i][j]) {
			case 18:
				if (dungeon[i][j + 1] != 18)
					dungeon[i][j] = 199;
				break;
			case 19:
				if (dungeon[i + 1][j] != 19)
					dungeon[i][j] = 200;
				break;
			case 21:
				if (dungeon[i + 1][j] != 19 || dungeon[i][j + 1] != 18)
					dungeon[i][j] = 202;
				break;
			case 23:
				if (dungeon[i][j + 1] != 18)
					dungeon[i][j] = 204;
				break;
			case 24:
				if (dungeon[i + 1][j] != 19)
					dungeon[i][j] = 205;
				break;
			}
		}
	}
}

static void DRLG_L1CornerFix()
{
	int i, j;

	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			if (!(L1dflags[i][j] & DLRG_PROTECTED) && dungeon[i][j] == 17 && dungeon[i - 1][j] == 13 && dungeon[i][j - 1] == 1) {
				dungeon[i][j] = 16;
				L1dflags[i][j - 1] &= DLRG_PROTECTED;
			}
			if (dungeon[i][j] == 202 && dungeon[i + 1][j] == 13 && dungeon[i][j + 1] == 1) {
				dungeon[i][j] = 8;
			}
		}
	}
}

struct mini_set {
	const BYTE* data;
	BOOL setview;
};
static bool DRLG_PlaceMiniSets(mini_set* minisets, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (minisets[i].data != NULL && !DRLG_PlaceMiniSet(minisets[i].data, 1, minisets[i].setview)) {
			return false;
		}
	}
	return true;
}

static void DRLG_L1(int entry)
{
	int i, j;
	int minarea;
	bool doneflag;

	switch (currlevel) {
	case 1:
		minarea = 533;
		break;
	case 2:
		minarea = 693;
		break;
	default:
		minarea = 761;
		break;
	}

	do {
		DRLG_InitTrans();

		do {
			InitL1Dungeon();
			L1firstRoom();
		} while (L1GetArea() < minarea);

		DRLG_L1MakeMegas();
		L1FillChambers();
		L1tileFix();
		L1AddWall();
		L1ClearFlags();
		DRLG_L1FloodTVal();

		doneflag = true;

		if (QuestStatus(Q_PWATER)) {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_PlaceMiniSet(PWATERIN, 1, TRUE);
			} else {
				doneflag = DRLG_PlaceMiniSet(PWATERIN, 1, FALSE);
				ViewY--;
			}
		}
		if (QuestStatus(Q_LTBANNER)) {
			if (entry == ENTRY_MAIN) {
				doneflag = DRLG_PlaceMiniSet(STAIRSUP, 1, TRUE);
			} else {
				doneflag = DRLG_PlaceMiniSet(STAIRSUP, 1, FALSE);
				if (entry == ENTRY_PREV) {
					ViewX = 2 * setpc_x + DBORDERX + 4;
					ViewY = 2 * setpc_y + DBORDERY + 12;
				} else {
					ViewY--;
				}
			}
#ifdef HELLFIRE
		} else if (currlevel >= 21) {
			mini_set stairs[2] = {
				{ currlevel != 21 ? L5USTAIRS : L5TWARP, entry != ENTRY_PREV },
				{ currlevel != 24 ? L5DSTAIRS : NULL, entry == ENTRY_PREV },
			};
			doneflag = DRLG_PlaceMiniSets(stairs, 2);
			if (entry == ENTRY_MAIN) {
				ViewY++;
			} else if (entry == ENTRY_PREV) {
				ViewY += 3;
			}
		} else {
			// currlevel < 21
			mini_set stairs[2] = {
				{ STAIRSUP, entry != ENTRY_PREV },
				{ L1DSTAIRS, entry == ENTRY_PREV },
			};
			doneflag &= DRLG_PlaceMiniSets(stairs, 2);
			if (entry == ENTRY_PREV) {
				ViewY--;
			}
#else
		} else {
			/*mini_set stairs[2] = {
				{ L1USTAIRS, entry != ENTRY_PREV },
				{ L1DSTAIRS, entry == ENTRY_PREV },
			};
			doneflag = DRLG_PlaceMiniSets(stairs, 2);
			if (entry == ENTRY_PREV) {
				ViewY--;
			}*/

			if (entry == ENTRY_PREV) {
				if (!DRLG_PlaceMiniSet(L1USTAIRS, 1, FALSE)
				|| !DRLG_PlaceMiniSet(L1DSTAIRS, 1, TRUE))
					doneflag = false;
				ViewY--;
			} else {
				if (!DRLG_PlaceMiniSet(L1USTAIRS, 1, TRUE)
				 || !DRLG_PlaceMiniSet(L1DSTAIRS, 1, FALSE))
					doneflag = false;
			}

			/*if (entry == ENTRY_MAIN) {
				if (!DRLG_PlaceMiniSet(L1USTAIRS, 1, TRUE)
				 || !DRLG_PlaceMiniSet(L1DSTAIRS, 1, FALSE))
					doneflag = false;
			} else {
				if (!DRLG_PlaceMiniSet(L1USTAIRS, 1, FALSE)
				|| !DRLG_PlaceMiniSet(L1DSTAIRS, 1, TRUE))
					doneflag = false;
				ViewY--;
			}*/
#endif
		}
	} while (!doneflag);

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 64) {
				int xx = 2 * i + DBORDERX; /* todo: fix loop */
				int yy = 2 * j + DBORDERY;
				DRLG_CopyTrans(xx, yy + 1, xx, yy);
				DRLG_CopyTrans(xx + 1, yy + 1, xx + 1, yy);
			}
		}
	}

	DRLG_L1TransFix();
	DRLG_L1DirtFix();
	DRLG_L1CornerFix();

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (L1dflags[i][j] & 0x7F)
				DRLG_L1PlaceDoor(i, j);
		}
	}

#ifdef HELLFIRE
	if (currlevel >= 21) {
		DRLG_L5Crypt_pattern1(10);
		DRLG_L5Crypt_rndset(byte_48A1B4, 95);
		DRLG_L5Crypt_rndset(byte_48A1B8, 95);
		DRLG_L5Crypt_rndset(byte_48A1C0, 100);
		DRLG_L5Crypt_rndset(byte_48A1C8, 100);
		DRLG_L5Crypt_rndset(byte_48A1E0, 60);
		DRLG_L5Crypt_lavafloor();
		switch (currlevel) {
		case 21:
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(15);
			DRLG_L5Crypt_pattern4(5);
			DRLG_L5Crypt_lavafloor();
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(5);
			DRLG_L5Crypt_pattern5(20);
			break;
		case 22:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(10);
			DRLG_L5Crypt_pattern5(20);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(10);
			DRLG_L5Crypt_lavafloor();
			break;
		case 23:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(15);
			DRLG_L5Crypt_pattern5(30);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(15);
			DRLG_L5Crypt_lavafloor();
			break;
		default:
			DRLG_L5Crypt_pattern7(10);
			DRLG_L5Crypt_pattern6(20);
			DRLG_L5Crypt_pattern5(30);
			DRLG_L5Crypt_pattern2(30);
			DRLG_L5Crypt_pattern3(20);
			DRLG_L5Crypt_pattern4(20);
			DRLG_L5Crypt_lavafloor();
			break;
		}
	} else
#endif
	{
		DRLG_L1Subs();
		DRLG_L1Shadows();
		DRLG_PlaceMiniSet(LAMPS, RandRange(5, 9), FALSE);
		DRLG_L1Floor();
	}

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	DRLG_Init_Globals();
	DRLG_CheckQuests(setpc_x, setpc_y);
}

void CreateL1Dungeon(DWORD rseed, int entry)
{
#ifdef HELLFIRE
	int i, j;
#endif

	SetRndSeed(rseed);

#ifdef HELLFIRE
	UberRow = 0;
	UberCol = 0;
	gbUberRoomOpened = false;
	UberLeverRow = 0;
	UberLeverCol = 0;
	gbUberLeverActivated = false;
	UberDiabloMonsterIndex = 0;
#endif

	DRLG_InitTrans();
	DRLG_InitSetPC();
	DRLG_LoadL1SP();
	DRLG_L1(entry);
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L1);
	DRLG_FreeL1SP();

#ifdef HELLFIRE
	if (currlevel >= 17) {
		DRLG_InitL5Vals();

		for (j = DBORDERY; j < DSIZEY + DBORDERY; j++) {
			for (i = DBORDERX; i < DSIZEX + DBORDERX; i++) {
				if (dPiece[i][j] == 290) {
					UberRow = i;
					UberCol = j;
				}
				/*if (dPiece[i][j] == 317) {
					CornerStone.x = i;
					CornerStone.y = j;
				}*/
			}
		}
	} else
#endif
	{
		DRLG_InitL1Vals();
	}
	DRLG_SetPC();
}

DEVILUTION_END_NAMESPACE
