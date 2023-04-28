/**
 * @file drlg_l1.cpp
 *
 * Implementation of the cathedral level generation algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Starting position of the base megatiles. */
#define BASE_MEGATILE_L1 (22 - 1)
/** Default megatile if the tile is zero. */
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

/** Contains shadows for 2x2 blocks of tiles in the Cathedral. */
const ShadowStruct L1SPATS[] = {
	// clang-format off
	//sh11, 01, 10,  00,   mask11    01    10    00    nv1, nv2, nv3
	{ { SF, 11, 12,   3 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 150,   0,   0 },
#if DEBUG_MODE
	{ {  0,  1,  0,   3 }, { 0x00, 0xFF, 0xFF, 0xFF },   0,   0,   0 }, // shadow is not necessary
	{ {  0,  0,  2,   3 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // shadow is not necessary
	{ {  0,  0,  7,   3 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // shadow is not necessary
	{ {  0,  0, 14,   3 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // shadow is not necessary
	{ {  0,  0, 35,   3 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // shadow is not necessary
	{ {  0,  0,  0,   3 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // shadow is not necessary
#endif
	{ { SF, SF, SF,   5 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 152, 140, 139 },
	{ { SF,  0, SF,   7 }, { 0xFF, 0x00, 0xFF, 0xFF }, 144,   0, 142 },
#if DEBUG_MODE
	{ { 36,  0, SF,   7 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 36
	{ {  2,  0, SF,   7 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  2
	{ { 12,  0, SF,   7 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 12
	{ { 26,  0, SF,   7 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 26
	{ {  7,  0, SF,   7 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  7
#endif
	{ { SF, SF, 12,   8 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 143,   0, 149 },
	{ { SF, 11, SF,   9 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 151, 147, 142 },
	{ {  0, SF,  0,  10 }, { 0x00, 0xFF, 0x00, 0xFF },   0, 140,   0 },
	{ {  0,  1,  0,  10 }, { 0x00, 0xFF, 0x00, 0xFF },   0, 146,   0 },
#if DEBUG_MODE
	{ {  0,  6,  0,  10 }, { 0xFF, 0xFF, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  6
	{ { SF, 25,  0,  10 }, { 0xFF, 0xFF, 0x00, 0xFF },   0,   0,   0 }, // missing shadow for 25
#endif
	{ { SF, 11,  0,  10 }, { 0xFF, 0xFF, 0x00, 0xFF }, 139, 147,   0 }, // shadow could be better
	//{ { SF, 11,  2,  10 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	//{ { SF, 11,  7,  10 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	//{ { SF, 11, SF,  10 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	//{ { SF, 11, 26,  10 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	{ { SF, 11, 12,  10 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 143, 147,   0 },
	{ { SF, 11, 36,  10 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 143, 147,   0 },
#if DEBUG_MODE
	{ { SF, 35,  0,  10 }, { 0xFF, 0xFF, 0x00, 0xFF },   0,   0,   0 }, // missing shadow for 35
#endif
	{ {  0,  0,  2,  11 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0, 148 },
	{ { SF,  0, SF,  11 }, { 0xFF, 0x00, 0xFF, 0xFF }, 144,   0, 139 },
	{ { SF,  5, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139,   0, 139 },
	{ { SF, 11, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139,   0, 139 },
	{ { SF, 14, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139,   0, 139 },
	{ { SF, 35, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 141,   0, 139 },
#if DEBUG_MODE
	{ {  7, 11, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 7
	{ {  7, 35, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 7
	{ {  7,  1, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 7
	{ {  0,  0,  7,  11 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 7
#endif
	{ {  2, 35, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 154,   0, 139 },
	{ {  2, 11, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 148,   0, 139 },
	{ {  2,  1, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 148,   0, 139 },
	{ { 12,  8, SF,  11 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 149,   0, 139 },
	{ {  0,  1,  0,  12 }, { 0x00, 0xFF, 0x00, 0xFF },   0, 146,   0 },
	{ {  0, SF,  0,  12 }, { 0x00, 0xFF, 0x00, 0xFF },   0, 140,   0 },
#if DEBUG_MODE
	{ {  0,  6,  0,  12 }, { 0x00, 0xFF, 0x00, 0xFF },   0,   0,   0 }, // missing shadow for  6
	{ {  7,  0, SF,  14 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  7
	{ { 26,  0, SF,  14 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 26
	{ { 36,  0, SF,  14 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 36
#endif
	{ {  2,  0, SF,  14 }, { 0xFF, 0x00, 0xFF, 0xFF }, 148,   0, 139 },
	//{ {  2,  1, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 148,   0, 139 }, // covered above
	//{ {  2,  6, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 148,   0, 139 }, // covered above
	{ { 12,  0, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 149,   0, 139 },
	//{ { 12,  6, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 149,   0, 139 }, // covered above
	{ { SF,  0, SF,  14 }, { 0xFF, 0x00, 0xFF, 0xFF }, 144,   0, 139 },
	//{ { SF,  1, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 144,   0, 139 }, covered above
	//{ { SF, 25, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 144,   0, 139 }, covered above
	{ { SF, 11, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139,   0, 139 },
	{ { SF, 35, SF,  14 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 141,   0, 139 },
	{ { SF,  0, SF,  15 }, { 0xFF, 0x00, 0xFF, 0xFF }, 145,   0, 142 },
	{ { SF,  0, SF,  16 }, { 0xFF, 0x00, 0xFF, 0xFF }, 144,   0, 142 },
#if DEBUG_MODE
	{ {  2,  0, SF,  16 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  2
	{ {  7,  0, SF,  16 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  7
	{ { 12,  0, SF,  16 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 12
	{ { 26,  0, SF,  16 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 26
	{ { 36,  0, SF,  16 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 36
#endif
	{ {  0,  0,  2,  35 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0, 154 },
	{ {  2,  0, SF,  35 }, { 0xFF, 0x00, 0xFF, 0xFF }, 148,   0, 141 },
	//{ {  2,  1, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 148,   0, 141 }, // covered above
	//{ {  2,  6, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 148,   0, 141 }, // covered above
	{ {  2, 35, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 154,   0, 141 },
#if DEBUG_MODE
	{ { 36,  0, SF,  35 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 36
	{ { 26,  0, SF,  35 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for 26
	{ {  7,  0, SF,  35 }, { 0xFF, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  7
	{ {  0,  0,  7,  35 }, { 0x00, 0x00, 0xFF, 0xFF },   0,   0,   0 }, // missing shadow for  7
#endif
	{ { 12,  0, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 149,   0, 141 },
	//{ { 12,  6, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 149,   0, 141 }, // covered above
	{ { 12, 35, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 153,   0, 141 },
	{ { SF,  0, SF,  35 }, { 0xFF, 0x00, 0xFF, 0xFF }, 152,   0, 141 },
	//{ { SF,  1, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 152,   0, 141 }, // covered above
	//{ { SF,  6, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 152,   0, 141 }, // covered above
	//{ { SF, 25, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 152,   0, 141 }, // covered above
	{ { SF, 11, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139,   0, 141 },
	{ { SF, 35, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 141,   0, 141 },
	{ { SF, SF, SF,  35 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 152, 140, 141 }, // only for tile 37
	{ {  0,  1,  0,  36 }, { 0x00, 0xFF, 0x00, 0xFF },   0, 146,   0 },
#if DEBUG_MODE
	{ {  0,  6,  0,  36 }, { 0x00, 0xFF, 0x00, 0xFF },   0,   0,   0 }, // missing shadow for 6
	{ {  0, 25,  0,  36 }, { 0x00, 0xFF, 0x00, 0xFF },   0,   0,   0 }, // missing shadow for 25
#endif
	{ {  0, SF,  0,  36 }, { 0x00, 0xFF, 0x00, 0xFF },   0, 140,   0 },
	{ { SF, 11,  0,  36 }, { 0xFF, 0xFF, 0x00, 0xFF }, 139, 147,   0 },
	//{ { SF, 11, SF,  36 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	//{ { SF, 11,  2,  36 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	//{ { SF, 11, 26,  36 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	//{ { SF, 11,  7,  36 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 139, 147,   0 }, // covered above
	{ { SF, 11, 12,  36 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 150, 147,   0 },
	{ { SF, 11, 36,  36 }, { 0xFF, 0xFF, 0xFF, 0xFF }, 150, 147,   0 },
#if DEBUG_MODE
	{ { SF, 35,  0,  36 }, { 0xFF, 0xFF, 0x00, 0xFF },   0,   0,   0 }, // missing shadow for 35
#endif
	{ {  0,  0,  0, 255 }, {    0,    0,    0,    0 },   0,   0,   0 }
	// clang-format on
};

/*
 * Maps tile IDs to their corresponding shadow types.
 * Values with higher than 16 and the values 4 and 6 are commented out, because there is
 *  no matching possibility in L1SPATS.
 * Value 4 is reused for the most common floor type (13) to increase the speed.
 * BUGFIX: This array should contain an additional 0 (207 elements).
 */
const BYTE BSTYPES[] = {
	// clang-format off
	0, 1, 2, 3, 0/*4*/, 5, 0/*6*/, 7, 8, 9,
	10, 11, 12, SF, 14, 15, 16, 0/*17*/, 0, 0,
	0, 0, 0, 0, 0, 25/*1*/, 26/*2*/, 36/*10*/, 0/*4*/, 5,
	0/*6*/, 7, 8, 9, 10, 35/*11*/, 36/*12*/, 35/*14*/, 5, 14,
	10, 0/*4*/, 14, 0/*4*/, 5, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	2, 3, 0/*4*/, 1, 0/*6*/, 7, 16, 0/*17*/, 2, 1,
	1, 2, 2, 1, 1, 2, 2, 2, 2, 2,
	1, 1, 11, 1, SF, SF, SF, 1, 2, 1, // 100 ...
	2, 1, 2, 1, 2, 2, 2, 2, 12, 0,
	0, 11, 1, 11, 1, SF, 0, 0, 0, 0,
	0, 0, 0, SF, SF, SF, SF, SF, SF, SF,
	SF, SF, SF, SF, SF, SF, 1, 11, 2, 12,
	SF, SF, SF, 12, 2, 1, 2, 2, 0/*4*/, 14,
	0/*4*/, 10, SF, SF, 0/*4*/, 0/*4*/, 1, 1, 0/*4*/, 2,
	2, SF, SF, SF, SF, 0/*25*/, 0/*26*/, 0/*28*/, 0/*30*/, 0/*31*/,
	0/*41*/, 0/*43*/, 0/*40*/, 0/*41*/, 0/*42*/, 0/*43*/, 0/*25*/, 0/*41*/, 0/*43*/, 0/*28*/,
	0/*28*/, 1, 2, 0/*25*/, 0/*26*/, 0/*22*/, 0/*22*/, 0/*25*/, 0/*26*/, 0,
	0, 0, 0, 0, 0, 0, 0
	// clang-format on
};
/*
 * Maps tile IDs to their corresponding undecorated tile type.
 */
const BYTE L1BTYPES[207] = {
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
/*
 * Specifies where the given tile ID should spread the room ID (transval).
 */
const BYTE L1FTYPES[207] = {
	// clang-format off
	 0, 10, 12, 14,  8,  8, 10, 12, 10, 12,
	 8, 10, 12, 15,  8, 14, 14, 14,  0,  0, // 10..
	 0,  0,  0,  0,  0, 10, 12,  8,  8,  8, // 20..
	10, 12, 10, 12,  8, 10, 12,  8,  8,  8, // 30..
	 8,  8,  8,  8,  8, 15, 15, 15, 15, 15, // 40..
	15, 15, 15, 15, 15, 15, 15,  0, 10, 12, // 50..
	14, 15, 15,  4,  0,  8,  0, 15, 15, 15, // 60..
	15,  0,  0, 15, 15, 15, 15, 15, 15, 10, // 70..
	12, 14,  8, 10, 10, 12, 14, 14, 12, 10, // 80..
	10, 12, 12, 10, 10, 12, 12, 12, 12, 12, // 90..
	10, 10, 10, 10, 15, 15, 15, 10, 12, 10, //100..
	12, 10, 12, 10, 12, 12, 12, 12, 12, 12, //110..
	12, 10, 10, 10, 10, 15, 10, 10, 15, 15, //120..
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, //130..
	15, 15, 15, 15, 15, 15, 10, 10, 12, 12, //140..
	15, 15, 15, 12, 12, 10, 12, 12,  8,  8, //150..
	 8,  8, 15, 15,  8,  8, 10, 10,  8, 12, //160..
	12, 15, 15, 15, 15, 10, 12,  8, 10, 12, //170..
	 8,  8,  8,  8,  8,  8, 10,  8,  8,  8, //180..
	 8, 10, 12, 10, 12,  0,  0, 10, 12,  0, //190..
	 0,  0,  0,  0,  0,  0, 12
	// clang-format on
};
#ifdef HELLFIRE
/*
 * Specifies where the given tile ID should spread the room ID (transval).
 */
const BYTE L5FTYPES[218] = {
	// clang-format off
	 0, 10, 12,  8,  8,  8, 10, 12, 10, 12,
	 8, 10, 12, 15,  8,  8,  8,  8,  0,  0, // 10..
	 0,  0,  0,  0,  0, 10, 12,  8,  8,  8, // 20..
	10, 12, 10, 12,  8, 10, 12,  8,  8,  8, // 30..
	 8,  8,  8,  8,  8, 15, 15, 15, 15, 15, // 40..
	15, 15, 15,  0,  0,  0,  0,  0, 15, 15, // 50..
	15,  0,  0,  0,  0,  0, 15, 15, 15, 10, // 60..
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8, // 70..
	 8,  8,  0,  0,  0,  0,  0,  0,  0, 10, // 80..
	10, 10, 12, 12, 12, 10, 12, 15, 15, 15, // 90..
	15, 15, 10, 10, 10, 10, 15, 15, 15, 15, //100..
	15, 15, 10, 12,  8,  8,  8,  8,  8,  8, //110..
	 8,  8, 10, 12, 15,  8,  8,  8,  8, 10, //120..
	12,  8,  8,  8,  8,  8,  8,  8,  8, 10, //130..
	12, 15,  8,  8,  8,  8, 10, 12,  8,  8, //140..
	 8,  8,  8,  8,  8,  8, 10, 12, 15,  8, //150..
	15,  8,  8, 15, 15, 15, 15, 15, 15, 15, //160..
	15, 15, 15, 10, 10, 10, 10, 10, 10, 12, //170..
	12, 12, 12, 12, 12, 10, 10, 12, 12, 15, //180..
	15, 15, 15, 15, 15, 15, 15, 15, 15,  0, //190..
	 0,  0,  0, 15, 15, 15, 15, 15, 15, 15, //200..
	15, 15, 15, 15, 15, 15, 15, 15,         //210..
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
#endif
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
/*
 * Miniset: stairs down.
 * Added an extra line to the top to prevent placing it too close to other entities (eg. upstair)
 */
const BYTE L1DSTAIRS[] = {
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
	 0,  21,  19,  19,  84, 0,
	 0,  18,  22,  22,  83, 0,
	 0,  85, 206,  88,  81, 0, // in vanilla tile 80 was used instead of 88
	 0,   0, 134, 135,   0, 0,
	 0,   0,   0,   0,   0, 0,
	// clang-format on
};
#ifdef HELLFIRE
/** Miniset: Column on the northern side of a vertical wall 1. */
const BYTE L5VERTCOL1[] = { 1, 1, 11, 95 };
/** Miniset: Column on the northern side of a horizontal wall 1. */
const BYTE L5HORIZCOL1[] = { 1, 1, 12, 96 };
/** Miniset: Prepare random large vertical wall tile */
const BYTE L5PREVERTWALL[] = {
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
const BYTE L5PREHORIZWALL[] = {
	// clang-format off
	3, 1, // width, height

	 2,  2,  2, // search

	94, 93, 92, // replace
	// clang-format on
};
/** Miniset: Use random floor tile 1. */
const BYTE L5RNDFLOOR1[] = { 1, 1, 13, 97 };
/** Miniset: Use random floor tile 2. */
const BYTE L5RNDFLOOR2[] = { 1, 1, 13, 98 };
/** Miniset: Use random floor tile 3. */
const BYTE L5RNDFLOOR3[] = { 1, 1, 13, 99 };
/** Miniset: Use random floor tile 4. */
const BYTE L5RNDFLOOR4[] = { 1, 1, 13, 100 };
/** Miniset: Use random large floor tile. */
const BYTE L5RNDLFLOOR1[] = {
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
const BYTE L5RNDLFLOOR2[] = {
	// clang-format off
	3, 3, // width, height

	13, 13, 13, // search
	13, 13, 13,
	13, 13, 13,

	210,   0, 0, // replace
	209, 167, 0,
	  0,   0, 0,
	// clang-format on
};
/** Miniset: Mooring on a stone. */
const BYTE L5RNDLFLOOR3[] = {
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
const BYTE L5RNDLFLOOR4[] = {
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
const BYTE L5RNDLFLOOR5[] = {
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
const BYTE L5RNDLFLOOR6[] = {
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
const BYTE L5RNDLFLOOR7[] = {
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
const BYTE L5RNDCOL1[] = {
	// clang-format off
	2, 2, // width, height

	204,  0, // search
	203, 11,

	212,   0, // replace
	211, 122,
	// clang-format on
};
const BYTE L5RNDCOL2[] = {
	// clang-format off
	2, 2, // width, height

	207, 0, // search
	206, 9,

	214,   0, // replace
	213, 137,
	// clang-format on
};
const BYTE L5RNDCOL3[] = {
	// clang-format off
	2, 2, // width, height

	204,  0, // search
	203, 11,

	216,   0, // replace
	215, 139,
	// clang-format on
};
const BYTE L5RNDCOL4[] = {
	// clang-format off
	1, 2, // width, height

	205, // search
	 12,

	217, // replace
	140,
	// clang-format on
};
const BYTE L5RNDCOL5[] = {
	// clang-format off
	2, 2, // width, height

	207,  0, // search
	206, 15,

	214,   0, // replace
	213, 143,
	// clang-format on
};
const BYTE L5RNDCOL6[] = {
	// clang-format off
	1, 2, // width, height

	205, // search
	  5,

	217, // replace
	150,
	// clang-format on
};
const BYTE L5RNDCOL7[] = {
	// clang-format off
	2, 2, // width, height

	207,  0, // search
	206, 15,

	210,   0, // replace
	209, 160,
	// clang-format on
};
const BYTE L5RNDCOL8[] = {
	// clang-format off
	1, 2, // width, height

	205, // search
	 12,

	208, // replace
	188,
	// clang-format on
};
const BYTE L5RNDCOL9[] = {
	// clang-format off
	2, 2, // width, height

	207,  0, // search
	206, 15,

	210,   0, // replace
	209, 192,
	// clang-format on
};
#endif

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L1ConvTbl[16] = { BASE_MEGATILE_L1 + 1, 13, 1, 13, 2, 13, 13, 13, 4, 13, 1, 13, 2, 13, 16, 13 };

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
			switch (dungeon[i][j]) {
			// -- pointless. placed in the chamber hallway, where one of the
			//    shadows is always messed up...
			//case 3:
			//	if (dungeon[i - 1][j] == 13 && dungeon[i - 1][j - 1] == 13) {
			//		dungeon[i - 1][j] = 206;
			//		dungeon[i - 1][j - 1] = 207;
			//	}
			//	break;
			case 5:
			//case 116: 5
			//case 133: 5
				if ((dungeon[i - 1][j] == 13 || dungeon[i - 1][j] == 204)
				 && (dungeon[i - 1][j - 1] == 13/* || dungeon[i - 1][j - 1] == 203*/)) {
					dungeon[i - 1][j] = 203;
					dungeon[i - 1][j - 1] = 204;
				}
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			case 7:
			case 15:
			case 16:
			case 17:
			//case 118: 7
			//case 126: 15
			//case 127: 16
			//case 128: 17
			//case 135: 7
			//case 144: 16
			//case 152: 7
			//case 160: 15 ?
			//case 161: 16
			//case 192: 15 ?
				if (dungeon[i - 1][j] == 13 && dungeon[i - 1][j - 1] == 13) {
					dungeon[i - 1][j] = 206;
					dungeon[i - 1][j - 1] = 207;
				}
				break;
			case 8:
				if (dungeon[i - 1][j - 1] == 13) {
					if (dungeon[i - 1][j] == 12) {
						dungeon[i - 1][j - 1] = 204;
					//} else if (dungeon[i - 1][j] == 13) {
					//	dungeon[i - 1][j] = 203;
					//	dungeon[i - 1][j - 1] = 204;
					}
				}
				break;
			case 11:
			case 35:
			case 14:
			case 37:
			//case 95: // necessary because of DRLG_L5PlaceRndSet(L5VERTCOL1)
			//case 119: 8
			//case 125: 14
			//case 136: 8
			//case 142: 14
			//case 153: 8
			//case 156: 11
			//case 159: 14
			//case 185: 11
			//case 186: 11
				if (dungeon[i - 1][j] == 13 || dungeon[i - 1][j] == 204 || dungeon[i - 1][j] == 205) {
					if (dungeon[i - 1][j - 1] == 13/* || dungeon[i - 1][j - 1] == 203*/) {
						dungeon[i - 1][j] = 203;
						dungeon[i - 1][j - 1] = 204;
					} else if (dungeon[i - 1][j - 1] == 2 || dungeon[i - 1][j - 1] == 12) {
						dungeon[i - 1][j] = 203;
					}
				}
				break;
			case 9:
			//case 120: 9
			//case 154: 9
				if (dungeon[i - 1][j] == 13
				 && (dungeon[i - 1][j - 1] == 13/* || dungeon[i - 1][j - 1] == 203 || dungeon[i - 1][j - 1] == 206*/)) {
					dungeon[i - 1][j] = 206;
					dungeon[i - 1][j - 1] = 207;
				}
				//assert(dungeon[i][j - 1] == 11); -- or its decorated one
				//if (dungeon[i][j - 1] == 13)
				//	dungeon[i][j - 1] = 205;
				break;
			case 10:
			case 12:
			case 36:
			//case 121: 10
			//case 123: 12
			//case 138: 10
			//case 155: 10
			//case 187: 12
			//case 188: 12
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 205;
				break;
			/*case 96: // 12
			case 188: // 12
				if (dungeon[i][j - 1] == 13)
					dungeon[i][j - 1] = 208;
				break;
			case 122: // 11
				if (dungeon[i - 1][j] == 203 && dungeon[i - 1][j - 1] == 204) {
					dungeon[i - 1][j] = 211;
					dungeon[i - 1][j - 1] = 212;
				}
				break;
			case 137: // 9
				//if (dungeon[i - 1][j] == 13)
				//if (dungeon[i - 1][j] == 206)
				assert(dungeon[i - 1][j] == 206);
					dungeon[i - 1][j] = 213;
				//if (dungeon[i - 1][j - 1] == 13)
				//if (dungeon[i - 1][j - 1] == 207)
				assert(dungeon[i - 1][j - 1] == 207);
					dungeon[i - 1][j - 1] = 214;
				//assert(dungeon[i][j - 1] == 205);
				break;
			case 139: // 11
				if (dungeon[i - 1][j] == 203 && dungeon[i - 1][j - 1] == 204) {
					dungeon[i - 1][j] = 215;
					dungeon[i - 1][j - 1] = 216;
				}
				break;
			case 140: // 12
			case 157: // 12
				//if (dungeon[i][j - 1] == 13)
				if (dungeon[i][j - 1] == 205)
					dungeon[i][j - 1] = 217;
				break;
			case 143: // 15
			//case 145: // 17
				//if (dungeon[i - 1][j] == 13 && dungeon[i - 1][j - 1] == 13)
				if (dungeon[i - 1][j] == 206 / * && dungeon[i - 1][j - 1] == 207 * /) {
					assert(dungeon[i - 1][j - 1] == 207);
					dungeon[i - 1][j] = 213;
					dungeon[i - 1][j - 1] = 214;
				}
				break;
			case 150: // 5
				//if (dungeon[i - 1][j] == 13)
				//	dungeon[i - 1][j] = 203;
				//if (dungeon[i - 1][j - 1] == 13)
				//	dungeon[i - 1][j - 1] = 204;
				//if (dungeon[i][j - 1] == 13)
				//if (dungeon[i][j - 1] == 205)
				assert(dungeon[i][j - 1] == 205);
					dungeon[i][j - 1] = 217;
				break;
			//case 162: // 17 -- why?
			//case 167: // done in DRLG_L5PlaceRndSet using L5RNDLFLOOR2
			case 160: // 15
			case 192: // 15
				//if (dungeon[i - 1][j] == 13 && dungeon[i - 1][j - 1] == 13)
				if (dungeon[i - 1][j] == 206 / * && dungeon[i - 1][j - 1] == 207 * /) {
					assert(dungeon[i - 1][j - 1] == 207);
					dungeon[i - 1][j] = 209;
					dungeon[i - 1][j - 1] = 210;
				}
				break;*/
			}
		}
	}
}
#endif

static void DRLG_L1Shadows()
{
	const ShadowStruct* ss;
	int x, y;
	ShadowPattern sdp;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			sdp.sh11 = BSTYPES[dungeon[x - 1][y - 1]];
			sdp.sh01 = BSTYPES[dungeon[x][y - 1]];
			sdp.sh10 = BSTYPES[dungeon[x - 1][y]];
			sdp.sh00 = BSTYPES[dungeon[x][y]];
			for (ss = L1SPATS; ss->shPattern.sh00 <= sdp.sh00; ss++) {
				if (ss->shPattern.sh00 == sdp.sh00) {
					if ((sdp.asUInt32 & ss->shMask.asUInt32) != ss->shPattern.asUInt32)
						continue;
					if (ss->nv1 != 0 && drlgFlags[x - 1][y - 1] == 0)
						dungeon[x - 1][y - 1] = ss->nv1;
					if (ss->nv2 != 0 && drlgFlags[x][y - 1] == 0)
						dungeon[x][y - 1] = ss->nv2;
					if (ss->nv3 != 0 && drlgFlags[x - 1][y] == 0)
						dungeon[x - 1][y] = ss->nv3;
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
	} else if (QuestStatus(Q_SKELKING)) {
		pSetPieces[0]._sptype = SPT_SKELKING;
		pSetPieces[0]._spData = LoadFileInMem(setpiecedata[pSetPieces[0]._sptype]._spdDunFile);
		/*// patch set-piece to use common tiles - SKngDO.DUN
		pSetPieces[0]._spData[(2 + 5 + 3 * 7) * 2] = 203;
		pSetPieces[0]._spData[(2 + 5 + 4 * 7) * 2] = 203;
		// patch set-piece to use common tiles and make the inner tile at the entrance non-walkable - SKngDO.DUN
		pSetPieces[0]._spData[(2 + 5 + 2 * 7) * 2] = 203;*/
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

void DRLG_InitL1Specials(int x1, int y1, int x2, int y2)
{
	int i, j, pn;
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		// add rims above doors
		for (i = x1; i <= x2; ++i) {
			for (j = y1; j <= y2; ++j) {
				pn = dPiece[i][j];
				if (pn == 77)
					pn = 1;
				else if (pn == 80)
					pn = 2;
				else
					pn = 0;
				dSpecial[i][j] = pn;
			}
		}
	} else
#endif
	{
		// add special arches
		for (i = x1; i <= x2; ++i) {
			for (j = y1; j <= y2; ++j) {
				pn = dPiece[i][j];
				if (pn == 12 || pn == 71 || pn == 211 || pn == 321 || pn == 341)
					pn = 1;
				else if (pn == 11 || pn == 249 || pn == 325 || pn == 331 || pn == 344)
					pn = 2;
				else if (pn == 253)
					pn = 3;
				else if (pn == 255)
					pn = 4;
				else if (pn == 259)
					pn = 5;
				else if (pn == 267)
					pn = 6;
				else if (pn == 418)
					pn = 7;
				else if (pn == 421)
					pn = 8;
				else
					pn = 0;
				dSpecial[i][j] = pn;
			}
		}
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
	bool ran2;

	dirProb = random_(0, 4);

	if (dir == (dirProb == 0)) {
		// try to place a room to the left
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			ry = h / 2 + y - height / 2;
			rx = x - width;
			if (L1CheckVHall(x, ry - 1, height + 2)
			 && L1CheckRoom(rx - 1, ry - 1, width + 1, height + 2)) // BUGFIX: swap args 3 and 4 ("height+2" and "width+1") (fixed)
				break;
		}

		if (i != 0)
			L1DrawRoom(rx, ry, width, height);
		// try to place a room to the right
		rxy2 = x + w;
		ran2 = L1CheckVHall(rxy2 - 1, ry - 1, height + 2)
			&& L1CheckRoom(rxy2, ry - 1, width + 1, height + 2);
		if (ran2)
			L1DrawRoom(rxy2, ry, width, height);
		// proceed with the placed a room on the left
		if (i != 0)
			L1RoomGen(rx, ry, width, height, true);
		// proceed with the placed a room on the right
		if (ran2)
			L1RoomGen(rxy2, ry, width, height, true);
	} else {
		// try to place a room to the top
		for (i = 20; i != 0; i--) {
			width = RandRange(2, 6) & ~1;
			height = RandRange(2, 6) & ~1;
			rx = w / 2 + x - width / 2;
			ry = y - height;
			if (L1CheckHHall(y, rx - 1, width + 2)
			 && L1CheckRoom(rx - 1, ry - 1, width + 2, height + 1))
				break;
		}

		if (i != 0)
			L1DrawRoom(rx, ry, width, height);
		// try to place a room to the bottom
		rxy2 = y + h;
		ran2 = L1CheckHHall(rxy2 - 1, rx - 1, width + 2)
			&& L1CheckRoom(rx - 1, rxy2, width + 2, height + 1);
		if (ran2)
			L1DrawRoom(rx, rxy2, width, height);
		// proceed with the placed a room on the top
		if (i != 0)
			L1RoomGen(rx, ry, width, height, false);
		// proceed with the placed a room on the bottom
		if (ran2)
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
	ChambersFirst = random_(0, 2);
	ChambersMiddle = random_(0, 2);
	ChambersLast = random_(0, 2);
	// make sure there is at least one chamber + prevent standalone first/last chambers
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

static int L1GetArea()
{
	int i, rv;
	BYTE* pTmp;

	rv = 0;
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in L1GetArea.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++) {
		assert(*pTmp <= 1);
		rv += *pTmp;
	}

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
		dungeon[DMAXX - 1][j] = BASE_MEGATILE_L1 + 1;
	for (i = 0; i < DMAXX - 1; i++)
		dungeon[i][DMAXY - 1] = BASE_MEGATILE_L1 + 1;
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
	while (TRUE) {
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
	while (TRUE) {
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
 * New dungeon values: 27 35 36 37
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
	dungeon[i][y1 + 0] = 3;
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
	dungeon[x1 + 0][i] = 3;
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
 * New dungeon values: [2] (1) 3 5 8 9 10 11 12 14 15
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
 * New dungeon values: 6 7 16 17 18 19 23 24
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
	for (int i = ChambersFirst + ChambersMiddle + ChambersLast; i < nRoomCnt; i++) {
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
			continue;
		// create the room
		int w = roomRight - roomLeft + 1;
		int h = roomBottom - roomTop + 1;
		w += 2;
		h += 2;
		themes[numthemes]._tsx1 = roomLeft - 1;
		themes[numthemes]._tsy1 = roomTop - 1;
		themes[numthemes]._tsx2 = roomLeft + w - 2;
		themes[numthemes]._tsy2 = roomTop + h - 2;
		numthemes++;
		if (numthemes == lengthof(themes))
			break;
	}
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
					while (TRUE) {
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
	DRLG_L5PlaceRndSet(L5RNDCOL1, rndper); // 11 -> 122
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
	DRLG_L5PlaceRndSet(L5RNDCOL2, rndper); // 9 -> 137
	/** Miniset: Use random vertical wall tile C2. */
	DRLG_PlaceRndTile(10, 138, rndper);
	/** Miniset: Column on the northern side of a vertical wall 5. */
	DRLG_L5PlaceRndSet(L5RNDCOL3, rndper); // 11 -> 139
	/** Miniset: Column on the northern side of a horizontal wall 5. */
	DRLG_L5PlaceRndSet(L5RNDCOL4, rndper); // 12 -> 140
	/** Miniset: Use random floor tile 10. */
	DRLG_PlaceRndTile(13, 141, rndper);
	/** Miniset: Use random concave corner east B2. */
	DRLG_PlaceRndTile(14, 142, rndper);
	/** Miniset: Use random column 3. (cracked column) */
	DRLG_L5PlaceRndSet(L5RNDCOL5, rndper); // 15 -> 143
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
	/** Miniset: Use random corner north 3. */
	DRLG_PlaceRndTile(4, 149, rndper);
	/** Miniset: North-Edge column on the northern side 3. */
	DRLG_L5PlaceRndSet(L5RNDCOL6, rndper); // 5 -> 150
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
	/** Miniset: Column on the northern side of a horizontal wall 2. */
	DRLG_PlaceRndTile(12, 187, rndper);
	/** Miniset: Column on the northern side of a vertical wall 3. */
	DRLG_PlaceRndTile(11, 186, rndper);
	/** Miniset: Column on the northern side of a horizontal wall 3. */
	DRLG_L5PlaceRndSet(L5RNDCOL8, rndper); // 12 -> 188
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

	const BYTE *floorTypes = L1FTYPES;
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		floorTypes = L5FTYPES;
	}
#endif
	DRLG_FloodTVal(floorTypes);
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
	int i;
	int minarea;
	bool placeWater = QuestStatus(Q_PWATER);

	switch (currLvl._dLevelIdx) {
	case DLV_CATHEDRAL1:
		minarea = 533;
		break;
	case DLV_CATHEDRAL2:
		minarea = 693;
		break;
	default:
		minarea = 761;
		break;
	}

	while (true) {
		do {
			memset(dungeon, 0, sizeof(dungeon));
			DRLG_L1CreateDungeon();
		} while (L1GetArea() < minarea);

		DRLG_L1MakeMegas();
		L1TileFix();
		memset(drlgFlags, 0, sizeof(drlgFlags));
		L1FillChambers();
		L1AddWall();
		L1ClearChamberFlags();
		if (placeWater) {
			POS32 warpPos = DRLG_PlaceMiniSet(PWATERIN);
			if (warpPos.x < 0) {
				continue;
			}
			pWarps[DWARP_SIDE]._wx = warpPos.x + 2;
			pWarps[DWARP_SIDE]._wy = warpPos.y + 3;
			pWarps[DWARP_SIDE]._wx = 2 * pWarps[DWARP_SIDE]._wx + DBORDERX;
			pWarps[DWARP_SIDE]._wy = 2 * pWarps[DWARP_SIDE]._wy + DBORDERY;
			pWarps[DWARP_SIDE]._wtype = WRPT_L1_PWATER;
		}
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_CRYPT) {
			POS32 warpPos = DRLG_PlaceMiniSet(L5USTAIRS); // L5USTAIRS (3, 6), was STAIRSUP, entry == ENTRY_MAIN
			if (warpPos.x < 0) {
				continue;
			}
			pWarps[DWARP_ENTRY]._wx = warpPos.x + 1;
			pWarps[DWARP_ENTRY]._wy = warpPos.y + 2;
			pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX;
			pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY;
			pWarps[DWARP_ENTRY]._wtype = WRPT_L1_UP;
			if (currLvl._dLevelIdx != DLV_CRYPT4) {
				warpPos = DRLG_PlaceMiniSet(L5DSTAIRS); // L5DSTAIRS (3, 7)
				if (warpPos.x < 0) {
					continue;
				}
				pWarps[DWARP_EXIT]._wx = warpPos.x + 1;
				pWarps[DWARP_EXIT]._wy = warpPos.y + 3;
				pWarps[DWARP_EXIT]._wx = 2 * pWarps[DWARP_EXIT]._wx + DBORDERX;
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
			pWarps[DWARP_ENTRY]._wx = warpPos.x + 1;
			pWarps[DWARP_ENTRY]._wy = warpPos.y + 1;
			pWarps[DWARP_ENTRY]._wx = 2 * pWarps[DWARP_ENTRY]._wx + DBORDERX;
			pWarps[DWARP_ENTRY]._wy = 2 * pWarps[DWARP_ENTRY]._wy + DBORDERY;
			pWarps[DWARP_ENTRY]._wtype = WRPT_L1_UP;
			if (pSetPieces[0]._sptype == SPT_SKELKING) {
				pWarps[DWARP_SIDE]._wx = pSetPieces[0]._spx + 6; // L1DSTAIRS (3, 5)
				pWarps[DWARP_SIDE]._wy = pSetPieces[0]._spy + 3;
				pWarps[DWARP_SIDE]._wx = 2 * pWarps[DWARP_SIDE]._wx + DBORDERX;
				pWarps[DWARP_SIDE]._wy = 2 * pWarps[DWARP_SIDE]._wy + DBORDERY;
				pWarps[DWARP_SIDE]._wtype = WRPT_L1_SKING;
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
			pWarps[DWARP_EXIT]._wx = 2 * pWarps[DWARP_EXIT]._wx + DBORDERX;
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
		DRLG_L5PlaceRndSet(L5VERTCOL1, 95);
		DRLG_L5PlaceRndSet(L5HORIZCOL1, 95);
		DRLG_L5PlaceRndSet(L5PREVERTWALL, 100);
		DRLG_L5PlaceRndSet(L5PREHORIZWALL, 100);
		DRLG_L5PlaceRndSet(L5RNDLFLOOR1, 60);
		switch (currLvl._dLevelIdx) {
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

/*static void DRLG_L1FixPreMap(int idx)
{
	uint16_t* lm = (uint16_t*)pSetPieces[idx]._spData;

	if (pSetPieces[idx]._sptype == SPT_BANNER) {
		// patch the map - Banner2.DUN
		// - replace the wall with door
		lm[2 + 7 + 6 * 8] = SwapLE16(193);
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
		// patch set-piece - Vile2.DUN
		// - fix empty tiles
		lm[2 + 8 + 16 * 21] = SwapLE16(203);
		lm[2 + 12 + 22 * 21] = SwapLE16(203);
		lm[2 + 13 + 22 * 21] = SwapLE16(203);
		lm[2 + 14 + 22 * 21] = SwapLE16(203);
		for (int i = 1; i < 23; i++) {
			lm[2 + 20 + i * 21] = SwapLE16(203);
		}
		// - add monsters
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 16 + 30 * 21 * 2] = SwapLE16((UMT_LAZARUS + 1) | (1 << 15));
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 24 + 29 * 21 * 2] = SwapLE16((UMT_RED_VEX + 1) | (1 << 15));
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 22 + 33 * 21 * 2] = SwapLE16((UMT_BLACKJADE + 1) | (1 << 15));
		// - replace the books
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 21 * 23 * 2 * 2 + 10 + 29 * 21 * 2] = SwapLE16(47);
		lm[2 + 21 * 23 + 21 * 23 * 2 * 2 + 21 * 23 * 2 * 2 + 29 + 30 * 21 * 2] = SwapLE16(47);
	} else if (pSetPieces[idx]._sptype == SPT_LVL_SKELKING) {
		// patch set-piece to add monsters - SklKng2.DUN
		lm[2 + 37 * 25 + 37 * 25 * 2 * 2 + 19 + 31 * 37 * 2] = SwapLE16((UMT_SKELKING + 1) | (1 << 15));
	}
}*/

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

	memset(drlgFlags, 0, sizeof(drlgFlags));
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL1DungeonData.");
	memset(dungeon, BASE_MEGATILE_L1 + 1, sizeof(dungeon));

	DRLG_LoadSP(0, DEFAULT_MEGATILE_L1);
}

void CreateL1Dungeon()
{
	const LevelData* lds = &AllLevels[currLvl._dLevelIdx];

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
	DRLG_Init_Globals();
	DRLG_InitL1Specials(DBORDERX, DBORDERY, MAXDUNX - DBORDERX - 1, MAXDUNY - DBORDERY - 1);

	DRLG_SetPC();
}

DEVILUTION_END_NAMESPACE
