/**
 * @file drlg_l2.cpp
 *
 * Implementation of the catacombs level generation algorithms.
 *
 * dflags matrix is used as a BOOLEAN matrix to protect the quest room.
 */

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Starting position of the megatiles. */
#define BASE_MEGATILE_L2 (12 - 1)

/** The number of generated rooms. */
int nRoomCnt;
ROOMNODE RoomList[81];
HALLNODE *pHallList = NULL;

#define AREA_MIN 2
#define ROOM_MAX 10
#define ROOM_MIN 4
const int Dir_Xadd[5] = { 0, 0, 1, 0, -1 };
const int Dir_Yadd[5] = { 0, -1, 0, 1, 0 };
const ShadowStruct SPATSL2[2] = { { 6, 3, 0, 3, 48, 0, 50 }, { 9, 3, 0, 3, 48, 0, 50 } };
//short word_48489A = 0;

const BYTE BTYPESL2[161] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 17, 18, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const BYTE BSTYPESL2[161] = { 0, 1, 2, 3, 0, 0, 6, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 6, 6, 6, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 2, 2, 2, 0, 0, 0, 1, 1, 1, 1, 6, 2, 2, 2, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 2, 2, 3, 3, 3, 3, 1, 1, 2, 2, 3, 3, 3, 3, 1, 1, 3, 3, 2, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/** Miniset: Arch vertical. */
const BYTE VARCH1[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 7,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH2[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 8,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH3[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 6,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH4[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 9,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH5[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 14,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH6[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 13,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH7[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 16,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH8[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 1,
	3, 4,
	0, 15,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH9[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 8,
	3, 4,
	0, 7,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH10[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 8,
	3, 4,
	0, 8,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH11[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 8,
	3, 4,
	0, 6,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH12[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 8,
	3, 4,
	0, 9,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH13[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  8,
	3,  4,
	0, 14,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH14[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  8,
	3,  4,
	0, 13,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH15[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  8,
	3,  4,
	0, 16,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - corner. */
const BYTE VARCH16[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  8,
	3,  4,
	0, 15,

	48,  0, // replace
	51, 42,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH17[] = {
	// clang-format off
	2, 3, // width, height

	2, 7, // search
	3, 4,
	0, 7,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH18[] = {
	// clang-format off
	2, 3, // width, height

	2, 7, // search
	3, 4,
	0, 8,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH19[] = {
	// clang-format off
	2, 3, // width, height

	2, 7, // search
	3, 4,
	0, 6,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH20[] = {
	// clang-format off
	2, 3, // width, height

	2, 7, // search
	3, 4,
	0, 9,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH21[] = {
	// clang-format off
	2, 3, // width, height

	2,  7, // search
	3,  4,
	0, 14,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH22[] = {
	// clang-format off
	2, 3, // width, height

	2,  7, // search
	3,  4,
	0, 13,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH23[] = {
	// clang-format off
	2, 3, // width, height

	2,  7, // search
	3,  4,
	0, 16,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - open wall. */
const BYTE VARCH24[] = {
	// clang-format off
	2, 3, // width, height

	2,  7, // search
	3,  4,
	0, 15,

	141, 39, // replace
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH25[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 4,
	3, 1,
	0, 7,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH26[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 4,
	3, 1,
	0, 8,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH27[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 4,
	3, 1,
	0, 6,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH28[] = {
	// clang-format off
	2, 4, // width, height

	3, 0, // search
	3, 4,
	3, 1,
	0, 9,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH29[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  4,
	3,  1,
	0, 14,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH30[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  4,
	3,  1,
	0, 13,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH31[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  4,
	3,  1,
	0, 16,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical. */
const BYTE VARCH32[] = {
	// clang-format off
	2, 4, // width, height

	3,  0, // search
	3,  4,
	3,  1,
	0, 15,

	48,  0, // replace
	51, 39,
	47, 44,
	 0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH33[] = {
	// clang-format off
	2, 4, // width, height

	2, 0, // search
	3, 8,
	3, 4,
	0, 7,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH34[] = {
	// clang-format off
	2, 4, // width, height

	2, 0, // search
	3, 8,
	3, 4,
	0, 8,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH35[] = {
	// clang-format off
	2, 4, // width, height

	2, 0, // search
	3, 8,
	3, 4,
	0, 6,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH36[] = {
	// clang-format off
	2, 4, // width, height

	2, 0, // search
	3, 8,
	3, 4,
	0, 9,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH37[] = {
	// clang-format off
	2, 4, // width, height

	2,  0, // search
	3,  8,
	3,  4,
	0, 14,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH38[] = {
	// clang-format off
	2, 4, // width, height

	2,  0, // search
	3,  8,
	3,  4,
	0, 13,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH39[] = {
	// clang-format off
	2, 4, // width, height

	2,  0, // search
	3,  8,
	3,  4,
	0, 16,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch vertical - room west entrance. */
const BYTE VARCH40[] = {
	// clang-format off
	2, 4, // width, height

	2,  0, // search
	3,  8,
	3,  4,
	0, 15,

	142,  0, // replace
	 51, 42,
	 47, 44,
	  0,  0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH1[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	2, 5, 9,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH2[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	2, 5, 6,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH3[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	2, 5, 8,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH4[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	2, 5, 7,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH5[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	2, 5, 15,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH6[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	2, 5, 16,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH7[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	2, 5, 13,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH8[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	2, 5, 14,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH9[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	8, 5, 9,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH10[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	8, 5, 6,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH11[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	8, 5, 8,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH12[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	8, 5, 7,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH13[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	8, 5, 15,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH14[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	8, 5, 16,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH15[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	8, 5, 13,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - north corner. */
const BYTE HARCH16[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	8, 5, 14,

	49, 46, 0, // replace
	43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH17[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 9,

	140, 46, 0, // replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH18[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 6,

	140, 46, 0, // Replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH19[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 8,

	140, 46, 0, // replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH20[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 7,

	140, 46, 0, // replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH21[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 15,

	140, 46, 0, // replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH22[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 16,

	140, 46, 0, // replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH23[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 13,

	140, 46, 0, // replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - wall. */
const BYTE HARCH24[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	8, 5, 14,

	140, 46, 0, // replace
	 43, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH25[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	5, 2, 9,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH26[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	5, 2, 6,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH27[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	5, 2, 8,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH28[] = {
	// clang-format off
	3, 2, // width, height

	3, 3, 0, // search
	5, 2, 7,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH29[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	5, 2, 15,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH30[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	5, 2, 16,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH31[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	5, 2, 13,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal. */
const BYTE HARCH32[] = {
	// clang-format off
	3, 2, // width, height

	3, 3,  0, // search
	5, 2, 14,

	49, 46, 0, // replace
	40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH33[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	9, 5, 9,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH34[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	9, 5, 6,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH35[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	9, 5, 8,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH36[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	9, 5, 7,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH37[] = {
	// clang-format off
	3, 2, // width, height

	1, 3, 0, // search
	9, 5, 15,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH38[] = {
	// clang-format off
	3, 2, // width, height

	1, 3,  0, // search
	9, 5, 16,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH39[] = {
	// clang-format off
	3, 2, // width, height

	1, 3,  0, // search
	9, 5, 13,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Arch horizontal - west corner. */
const BYTE HARCH40[] = {
	// clang-format off
	3, 2, // width, height

	1, 3,  0, // search
	9, 5, 14,

	140, 46, 0, // replace
	 40, 45, 0,
	// clang-format on
};
/** Miniset: Stairs up. */
const BYTE L2USTAIRS[] = {
	// clang-format off
	4, 4, // width, height

	3, 3, 3, 3, // search
	3, 3, 3, 3,
	3, 3, 3, 3,
	3, 3, 3, 3,

	0,  0,  0, 0, // replace
	0, 72, 77, 0,
	0, 76,  0, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stairs down. */
const BYTE L2DSTAIRS[] = {
	// clang-format off
	4, 4, // width, height

	3, 3, 3, 3, // search
	3, 3, 3, 3,
	3, 3, 3, 3,
	3, 3, 3, 3,

	0,  0,  0, 0, // replace
	0, 48, 71, 0,
	0, 50, 78, 0,
	0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Stairs to town. */
const BYTE L2TWARP[] = {
	// clang-format off
	4, 4, // width, height

	3, 3, 3, 3, // search
	3, 3, 3, 3,
	3, 3, 3, 3,
	3, 3, 3, 3,

	0,   0,   0, 0, // replace
	0, 158, 160, 0,
	0, 159,   0, 0,
	0,   0,   0, 0,
	// clang-format on
};
/** Miniset: Crumbled south pillar. */
const BYTE CRUSHCOL[] = {
	// clang-format off
	3, 3, // width, height

	3, 1, 3, // search
	2, 6, 3,
	3, 3, 3,

	0,  0, 0, // replace
	0, 83, 0,
	0,  0, 0,
	// clang-format on
};
/** Miniset: Vertical oil spill. */
const BYTE BIG1[] = {
	// clang-format off
	2, 2, // width, height

	3, 3, // search
	3, 3,

	113, 0, // replace
	112, 0,
	// clang-format on
};
/** Miniset: Horizontal oil spill. */
const BYTE BIG2[] = {
	// clang-format off
	2, 2, // width, height

	3, 3, // search
	3, 3,

	114, 115, // replace
	  0,   0,
	// clang-format on
};
/** Miniset: Horizontal platform. */
const BYTE BIG3[] = {
	// clang-format off
	1, 2, // width, height

	1, // search
	1,

	117, // replace
	116,
	// clang-format on
};
/** Miniset: Vertical platform. */
const BYTE BIG4[] = {
	// clang-format off
	2, 1, // width, height

	2, 2, // search

	118, 119, // replace
	// clang-format on
};
/** Miniset: Large oil spill. */
const BYTE BIG5[] = {
	// clang-format off
	2, 2, // width, height

	3, 3, // search
	3, 3,

	120, 122, // replace
	121, 123,
	// clang-format on
};
/** Miniset: Vertical wall with debris. */
const BYTE BIG6[] = {
	// clang-format off
	1, 2, // width, height

	1, // search
	1,

	125, // replace
	124,
	// clang-format on
};
/** Miniset: Horizontal wall with debris. */
const BYTE BIG7[] = {
	// clang-format off
	2, 1, // width, height

	2, 2, // search

	126, 127, // replace
	// clang-format on
};
/** Miniset: Rock pile. */
const BYTE BIG8[] = {
	// clang-format off
	2, 2, // width, height

	3, 3, // search
	3, 3,

	128, 130, // replace
	129, 131,
	// clang-format on
};
/** Miniset: Vertical wall collapsed. */
const BYTE BIG9[] = {
	// clang-format off
	2, 2, // width, height

	1, 3, // search
	1, 3,

	133, 135, // replace
	132, 134,
	// clang-format on
};
/** Miniset: Horizontal wall collapsed. */
const BYTE BIG10[] = {
	// clang-format off
	2, 2, // width, height

	2, 2, // search
	3, 3,

	136, 137, // replace
	  3,   3,
	// clang-format on
};
/** Miniset: Crumbled vertical wall 1. */
const BYTE RUINS1[] = {
	// clang-format off
	1, 1, // width, height

	1, // search

	80, // replace
	// clang-format on
};
/** Miniset: Crumbled vertical wall 2. */
const BYTE RUINS2[] = {
	// clang-format off
	1, 1, // width, height

	1, // search

	81, // replace
	// clang-format on
};
/** Miniset: Crumbled vertical wall 3. */
const BYTE RUINS3[] = {
	// clang-format off
	1, 1, // width, height

	1, // search

	82, // replace
	// clang-format on
};
/** Miniset: Crumbled horizontal wall 1. */
const BYTE RUINS4[] = {
	// clang-format off
	1, 1, // width, height

	2, // search

	84, // replace
	// clang-format on
};
/** Miniset: Crumbled horizontal wall 2. */
const BYTE RUINS5[] = {
	// clang-format off
	1, 1, // width, height

	2, // search

	85, // replace
	// clang-format on
};
/** Miniset: Crumbled horizontal wall 3. */
const BYTE RUINS6[] = {
	// clang-format off
	1, 1, // width, height

	2, // search

	86, // replace
	// clang-format on
};
/** Miniset: Crumbled north pillar. */
const BYTE RUINS7[] = {
	// clang-format off
	1, 1, // width, height

	8, // search

	87, // replace
	// clang-format on
};
/** Miniset: Bloody gib 1. */
const BYTE PANCREAS1[] = {
	// clang-format off
	5, 3, // width, height

	3, 3, 3, 3, 3, // search
	3, 3, 3, 3, 3,
	3, 3, 3, 3, 3,

	0, 0,   0, 0, 0, // replace
	0, 0, 108, 0, 0,
	0, 0,   0, 0, 0,
	// clang-format on
};
/** Miniset: Bloody gib 2. */
const BYTE PANCREAS2[] = {
	// clang-format off
	5, 3, // width, height

	3, 3, 3, 3, 3, // search
	3, 3, 3, 3, 3,
	3, 3, 3, 3, 3,

	0, 0,   0, 0, 0, // replace
	0, 0, 110, 0, 0,
	0, 0,   0, 0, 0,
	// clang-format on
};

/*
 * Patterns with length of 9 (3x3) and the replacement.
 * The remaining(6) values are for alignment.
 */
const int Patterns[][16] = {
	{ 0, 0, 0, 0, 2, 0, 0, 0, 0, 3 },
	{ 0, 7, 0, 0, 1, 0, 0, 5, 0, 2 },
	{ 0, 5, 0, 0, 1, 0, 0, 7, 0, 2 },
	{ 0, 0, 0, 7, 1, 5, 0, 0, 0, 1 },
	{ 0, 0, 0, 5, 1, 7, 0, 0, 0, 1 },
	{ 0, 1, 0, 0, 3, 0, 0, 1, 0, 4 },
	{ 0, 0, 0, 1, 3, 1, 0, 0, 0, 5 },
	{ 0, 6, 0, 6, 1, 0, 0, 0, 0, 6 },
	{ 0, 6, 0, 0, 1, 6, 0, 0, 0, 9 },
	{ 0, 0, 0, 6, 1, 0, 0, 6, 0, 7 },
	{ 0, 0, 0, 0, 1, 6, 0, 6, 0, 8 },
	{ 0, 6, 0, 6, 6, 0, 8, 6, 0, 7 },
	{ 0, 6, 8, 6, 6, 6, 0, 0, 0, 9 },
	{ 0, 6, 0, 0, 6, 6, 0, 6, 8, 8 },
	{ 6, 6, 6, 6, 6, 6, 0, 6, 0, 8 },
	{ 2, 6, 6, 6, 6, 6, 0, 6, 0, 8 },
	{ 7, 7, 7, 6, 6, 6, 0, 6, 0, 8 },
	{ 6, 6, 2, 6, 6, 6, 0, 6, 0, 8 },
	{ 6, 2, 6, 6, 6, 6, 0, 6, 0, 8 },
	{ 2, 6, 6, 6, 6, 6, 0, 6, 0, 8 },
	{ 6, 7, 7, 6, 6, 6, 0, 6, 0, 8 },
	{ 4, 4, 6, 6, 6, 6, 2, 6, 2, 8 },
	{ 2, 2, 2, 2, 6, 2, 2, 6, 2, 7 },
	{ 2, 2, 2, 2, 6, 2, 6, 6, 6, 7 },
	{ 2, 2, 6, 2, 6, 6, 2, 2, 6, 9 },
	{ 2, 6, 2, 2, 6, 2, 2, 2, 2, 6 },
	{ 2, 2, 2, 2, 6, 6, 2, 2, 2, 9 },
	{ 2, 2, 2, 6, 6, 2, 2, 2, 2, 6 },
	{ 2, 2, 0, 2, 6, 6, 2, 2, 0, 9 },
	{ 0, 0, 0, 0, 4, 0, 0, 0, 0, 12 },
	{ 0, 1, 0, 0, 1, 4, 0, 1, 0, 10 },
	{ 0, 0, 0, 1, 1, 1, 0, 4, 0, 11 },
	{ 0, 0, 0, 6, 1, 4, 0, 1, 0, 14 },
	{ 0, 6, 0, 1, 1, 0, 0, 4, 0, 16 },
	{ 0, 6, 0, 0, 1, 1, 0, 4, 0, 15 },
	{ 0, 0, 0, 0, 1, 1, 0, 1, 4, 13 },
	{ 8, 8, 8, 8, 1, 1, 0, 1, 1, 13 },
	{ 8, 8, 4, 8, 1, 1, 0, 1, 1, 10 },
	{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 11 },
	{ 1, 1, 1, 1, 1, 1, 2, 2, 8, 2 },
	{ 0, 1, 0, 1, 1, 4, 1, 1, 0, 16 },
	{ 0, 0, 0, 1, 1, 1, 1, 1, 4, 11 },
	{ 1, 1, 4, 1, 1, 1, 0, 2, 2, 2 },
	{ 1, 1, 1, 1, 1, 1, 6, 2, 6, 2 },
	{ 4, 1, 1, 1, 1, 1, 6, 2, 6, 2 },
	{ 2, 2, 2, 1, 1, 1, 4, 1, 1, 11 },
	{ 4, 1, 1, 1, 1, 1, 2, 2, 2, 2 },
	{ 1, 1, 4, 1, 1, 1, 2, 2, 1, 2 },
	{ 4, 1, 1, 1, 1, 1, 1, 2, 2, 2 },
	{ 2, 2, 6, 1, 1, 1, 4, 1, 1, 11 },
	{ 4, 1, 1, 1, 1, 1, 2, 2, 6, 2 },
	{ 1, 2, 2, 1, 1, 1, 4, 1, 1, 11 },
	{ 0, 1, 1, 0, 1, 1, 0, 1, 1, 10 },
	{ 2, 1, 1, 3, 1, 1, 2, 1, 1, 14 },
	{ 1, 1, 0, 1, 1, 2, 1, 1, 0, 1 },
	{ 0, 4, 0, 1, 1, 1, 0, 1, 1, 14 },
	{ 4, 1, 0, 1, 1, 0, 1, 1, 0, 1 },
	{ 0, 1, 0, 4, 1, 1, 0, 1, 1, 15 },
	{ 1, 1, 1, 1, 1, 1, 0, 2, 2, 2 },
	{ 0, 1, 1, 2, 1, 1, 2, 1, 4, 10 },
	{ 2, 1, 1, 1, 1, 1, 0, 4, 0, 16 },
	{ 1, 1, 4, 1, 1, 2, 0, 1, 2, 1 },
	{ 2, 1, 1, 2, 1, 1, 1, 1, 4, 10 },
	{ 1, 1, 2, 1, 1, 2, 4, 1, 8, 1 },
	{ 2, 1, 4, 1, 1, 1, 4, 4, 1, 16 },
	{ 2, 1, 1, 1, 1, 1, 1, 1, 1, 16 },
	{ 1, 1, 2, 1, 1, 1, 1, 1, 1, 15 },
	{ 1, 1, 1, 1, 1, 1, 2, 1, 1, 14 },
	{ 4, 1, 1, 1, 1, 1, 2, 1, 1, 14 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 2, 8 },
};

static bool DRLG_L2PlaceMiniSet(const BYTE *miniset, BOOL setview)
{
	int sx, sy, sw, sh, xx, yy, ii, tries;
	bool done;

	sw = miniset[0];
	sh = miniset[1];

	sx = random_(0, DMAXX - sw);
	sy = random_(0, DMAXY - sh);

	tries = 0;
	while (TRUE) {
		done = true;
		ii = 2;
		for (yy = sy; yy < sy + sh && done; yy++) {
			for (xx = sx; xx < sx + sw && done; xx++) {
				if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
					done = false;
				}
				if (dflags[xx][yy]) {
					done = false;
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
		return false;

	ii = sw * sh + 2;
	for (yy = sy; yy < sy + sh; yy++) {
		for (xx = sx; xx < sx + sw; xx++) {
			if (miniset[ii] != 0) {
				dungeon[xx][yy] = miniset[ii];
			}
			ii++;
		}
	}

	if (setview) {
		ViewX = 2 * sx + DBORDERX + 5;
		ViewY = 2 * sy + DBORDERY + 6;
	}

	return true;
}

static void DRLG_L2PlaceRndSet(const BYTE *miniset, int rndper)
{
	int sx, sy, sw, sh, xx, yy, ii, kk;
	bool found;

	sw = miniset[0];
	sh = miniset[1];

	for (sy = 0; sy < DMAXY - sh; sy++) {
		for (sx = 0; sx < DMAXX - sw; sx++) {
			found = true;
			ii = 2;
			for (yy = sy; yy < sy + sh && found; yy++) {
				for (xx = sx; xx < sx + sw && found; xx++) {
					if (miniset[ii] != 0 && dungeon[xx][yy] != miniset[ii]) {
						found = false;
					}
					if (dflags[xx][yy]) {
						found = false;
					}
					ii++;
				}
			}
			kk = sw * sh + 2;
			if (found) {
				for (yy = std::max(sy - sh, 0); yy < std::min(sy + 2 * sh, DMAXY) && found; yy++) {
					for (xx = std::max(sx - sw, 0); xx < std::min(sx + 2 * sw, DMAXX); xx++) {
						// BUGFIX: yy and xx can go out of bounds (fixed)
						if (dungeon[xx][yy] == miniset[kk]) {
							found = false;
						}
					}
				}
			}
			if (found && random_(0, 100) < rndper) {
				for (yy = sy; yy < sy + sh; yy++) {
					for (xx = sx; xx < sx + sw; xx++) {
						if (miniset[kk] != 0) {
							dungeon[xx][yy] = miniset[kk];
						}
						kk++;
					}
				}
			}
		}
	}
}

static void DRLG_L2Subs()
{
	int x, y, i, j, k, rv;
	BYTE c;

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			if (dflags[x][y])
				continue;
			if (random_(0, 4) == 0) {
				c = BTYPESL2[dungeon[x][y]];
				if (c != 0) {
					rv = random_(0, 16);
					k = -1;
					while (rv >= 0) {
						k++;
						if (k == sizeof(BTYPESL2)) {
							k = 0;
						}
						if (c == BTYPESL2[k]) {
							rv--;
						}
					}
					for (j = y - 2; j < y + 2; j++) {
						for (i = x - 2; i < x + 2; i++) {
							if (dungeon[i][j] == k) {
								j = y + 3;
								i = x + 2;
							}
						}
					}
					if (j < y + 3) {
						dungeon[x][y] = k;
					}
				}
			}
		}
	}
}

static void DRLG_L2Shadows()
{
	const ShadowStruct *ss;
	int x, y, i;
	BYTE sd00, sd10, sd01, sd11;

	for (y = 1; y < DMAXY; y++) {
		for (x = 1; x < DMAXX; x++) {
			sd00 = BSTYPESL2[dungeon[x][y]];
			sd10 = BSTYPESL2[dungeon[x - 1][y]];
			sd01 = BSTYPESL2[dungeon[x][y - 1]];
			sd11 = BSTYPESL2[dungeon[x - 1][y - 1]];
			ss = SPATSL2;
			for (i = 0; i < 2; i++) {
				if (ss->strig == sd00) {
					if ((ss->s1 != 0 && ss->s1 != sd11)
					 || (ss->s2 != 0 && ss->s2 != sd01)
					 || (ss->s3 != 0 && ss->s3 != sd10)) {
						continue;
					}
					if (ss->nv1 != 0) {
						dungeon[x - 1][y - 1] = ss->nv1;
					}
					if (ss->nv2 != 0) {
						dungeon[x][y - 1] = ss->nv2;
					}
					if (ss->nv3 != 0) {
						dungeon[x - 1][y] = ss->nv3;
					}
				}
			}
		}
	}
}

static void DRLG_L2InitDungeon()
{
	memset(dflags, 0, sizeof(dflags));

	static_assert(sizeof(pdungeon) == DMAXX * DMAXY, "Linear traverse of pdungeon does not work in DRLG_L2InitDungeon.");
	memset(pdungeon, 32, sizeof(pdungeon));
}

static void DRLG_LoadL2SP()
{
	pSetPiece = NULL;
	if (QuestStatus(Q_BLIND)) {
		pSetPiece = LoadFileInMem("Levels\\L2Data\\Blind1.DUN", NULL);
	} else if (QuestStatus(Q_BLOOD)) {
		pSetPiece = LoadFileInMem("Levels\\L2Data\\Blood1.DUN", NULL);
	} else if (QuestStatus(Q_SCHAMB)) {
		pSetPiece = LoadFileInMem("Levels\\L2Data\\Bonestr2.DUN", NULL);
	}
}

static void DRLG_FreeL2SP()
{
	MemFreeDbg(pSetPiece);
}

static void DRLG_L2SetRoom(int rx1, int ry1)
{
	int rw, rh, i, j;
	BYTE *sp;

	rw = pSetPiece[0];
	rh = pSetPiece[2];

	// assert(setpc_x == rx1);
	// assert(setpc_y == ry1);
	setpc_w = rw;
	setpc_h = rh;

	sp = &pSetPiece[4];

	rw += rx1;
	rh += ry1;
	for (j = ry1; j < rh; j++) {
		for (i = rx1; i < rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : 3;
			dflags[i][j] = TRUE; // |= DLRG_PROTECTED;
			sp += 2;
		}
	}
}

static void DefineRoom(int nX1, int nY1, int nX2, int nY2)
{
	int i, j;

	nRoomCnt++;
	RoomList[nRoomCnt].nRoomx1 = nX1;
	RoomList[nRoomCnt].nRoomx2 = nX2;
	RoomList[nRoomCnt].nRoomy1 = nY1;
	RoomList[nRoomCnt].nRoomy2 = nY2;

	pdungeon[nX1][nY1] = 67;
	pdungeon[nX1][nY2] = 69;
	pdungeon[nX2][nY1] = 66;
	pdungeon[nX2][nY2] = 65;

	for (i = nX1 + 1; i < nX2; i++) {
		pdungeon[i][nY1] = 35;
		pdungeon[i][nY2] = 35;
	}
	for (j = nY1 + 1; j < nY2; j++) {
		pdungeon[nX1][j] = 35;
		pdungeon[nX2][j] = 35;
	}
	for (j = nY1 + 1; j < nY2; j++) {
		for (i = nX1 + 1; i < nX2; i++) {
			pdungeon[i][j] = 46;
		}
	}
}

static void CreateDoorType(int nX, int nY)
{
	if (pdungeon[nX - 1][nY] != 68
	 && pdungeon[nX + 1][nY] != 68
	 && pdungeon[nX][nY - 1] != 68
	 && pdungeon[nX][nY + 1] != 68
	 && (pdungeon[nX][nY] < 65 || pdungeon[nX][nY] > 69)) {
		pdungeon[nX][nY] = 68;
	}
}

static void PlaceHallExt(int nX, int nY)
{
	if (pdungeon[nX][nY] == 32) {
		pdungeon[nX][nY] = 44;
	}
}

static void AddHall(int nX1, int nY1, int nX2, int nY2, int nHd)
{
	HALLNODE *p1, *p2;

	p1 = (HALLNODE *)DiabloAllocPtr(sizeof(*p1));
	p2 = pHallList;
	if (p2 == NULL) {
		pHallList = p1;
	} else {
		while (p2->pNext != NULL) {
			p2 = p2->pNext;
		}
		p2->pNext = p1;
	}
	p1->nHallx1 = nX1;
	p1->nHally1 = nY1;
	p1->nHallx2 = nX2;
	p1->nHally2 = nY2;
	p1->nHalldir = nHd;
	p1->pNext = NULL;
}

/**
 * Draws a random room rectangle, and then subdivides the rest of the passed in rectangle into 4 and recurses.
 * @param nX1 Lower X boundary of the area to draw into.
 * @param nY1 Lower Y boundary of the area to draw into.
 * @param nX2 Upper X boundary of the area to draw into.
 * @param nY2 Upper Y boundary of the area to draw into.
 * @param nRDest The room number of the parent room this call was invoked for. Zero for empty
 * @param nHDir The direction of the hall from nRDest to this room.
 * @param nH Height of the room, if not zero.
 * @param nW Width of the room, if set
 */
static void CreateRoom(int nX1, int nY1, int nX2, int nY2, int nRDest, int nHDir, int nH, int nW)
{
	int nAw, nAh, nRw, nRh, nRx1, nRy1, nRx2, nRy2, nHw, nHh, nHx1, nHy1, nHx2, nHy2, nRid;

	if (nRoomCnt >= lengthof(RoomList) - 1) {
		return;
	}

	if (nX1 < 1)
		nX1 = 1;
	if (nX2 > DMAXX - 2)
		nX2 = DMAXX - 2;
	if (nY1 < 1)
		nY1 = 1;
	if (nY2 > DMAXY - 2)
		nY2 = DMAXY - 2;

	nAw = nX2 - nX1;
	nAh = nY2 - nY1;
	if (nAw < AREA_MIN || nAh < AREA_MIN) {
		return;
	}

	if (nAw > ROOM_MAX) {
		nRw = RandRange(ROOM_MIN, ROOM_MAX);
	} else if (nAw > ROOM_MIN) {
		nRw = RandRange(ROOM_MIN, nAw);
	} else {
		nRw = nAw;
	}
	if (nAh > ROOM_MAX) {
		nRh = RandRange(ROOM_MIN, ROOM_MAX);
	} else if (nAh > ROOM_MIN) {
		nRh = RandRange(ROOM_MIN, nAh);
	} else {
		nRh = nAh;
	}

	if (nW != 0) {
		nRw = nW;
		nRh = nH;
	}

	nRx1 = RandRange(nX1, nX2 - nRw);
	nRy1 = RandRange(nY1, nY2 - nRh);
	nRx2 = nRw + nRx1;
	nRy2 = nRh + nRy1;

	if (nW != 0) {
		setpc_x = nRx1 + 2;
		setpc_y = nRy1 + 2;
	}
	DefineRoom(nRx1, nRy1, nRx2, nRy2);

	nRid = nRoomCnt;
	RoomList[nRid].nRoomDest = nRDest;

	if (nRDest != 0) {
		switch (nHDir) {
		case 1:
			nHx1 = RandRange(nRx1 + 1, nRx2 - 2);
			nHy1 = nRy1;
			nHw = RoomList[nRDest].nRoomx2 - RoomList[nRDest].nRoomx1 - 2;
			nHx2 = random_(0, nHw) + RoomList[nRDest].nRoomx1 + 1;
			nHy2 = RoomList[nRDest].nRoomy2;
			break;
		case 3:
			nHx1 = RandRange(nRx1 + 1, nRx2 - 2);
			nHy1 = nRy2;
			nHw = RoomList[nRDest].nRoomx2 - RoomList[nRDest].nRoomx1 - 2;
			nHx2 = random_(0, nHw) + RoomList[nRDest].nRoomx1 + 1;
			nHy2 = RoomList[nRDest].nRoomy1;
			break;
		case 2:
			nHx1 = nRx2;
			nHy1 = RandRange(nRy1 + 1, nRy2 - 2);
			nHx2 = RoomList[nRDest].nRoomx1;
			nHh = RoomList[nRDest].nRoomy2 - RoomList[nRDest].nRoomy1 - 2;
			nHy2 = random_(0, nHh) + RoomList[nRDest].nRoomy1 + 1;
			break;
		case 4:
			nHx1 = nRx1;
			nHy1 = RandRange(nRy1 + 1, nRy2 - 2);
			nHx2 = RoomList[nRDest].nRoomx2;
			nHh = RoomList[nRDest].nRoomy2 - RoomList[nRDest].nRoomy1 - 2;
			nHy2 = random_(0, nHh) + RoomList[nRDest].nRoomy1 + 1;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		AddHall(nHx1, nHy1, nHx2, nHy2, nHDir);
	}

	if (nRh > nRw) {
		CreateRoom(nX1 + 2, nY1 + 2, nRx1 - 2, nRy2 - 2, nRid, 2, 0, 0);
		CreateRoom(nRx2 + 2, nRy1 + 2, nX2 - 2, nY2 - 2, nRid, 4, 0, 0);
		CreateRoom(nX1 + 2, nRy2 + 2, nRx2 - 2, nY2 - 2, nRid, 1, 0, 0);
		CreateRoom(nRx1 + 2, nY1 + 2, nX2 - 2, nRy1 - 2, nRid, 3, 0, 0);
	} else {
		CreateRoom(nX1 + 2, nY1 + 2, nRx2 - 2, nRy1 - 2, nRid, 3, 0, 0);
		CreateRoom(nRx1 + 2, nRy2 + 2, nX2 - 2, nY2 - 2, nRid, 1, 0, 0);
		CreateRoom(nX1 + 2, nRy1 + 2, nRx1 - 2, nY2 - 2, nRid, 2, 0, 0);
		CreateRoom(nRx2 + 2, nY1 + 2, nX2 - 2, nRy2 - 2, nRid, 4, 0, 0);
	}
}

static void GetHall(int *nX1, int *nY1, int *nX2, int *nY2, int *nHd)
{
	HALLNODE *p1;

	p1 = pHallList->pNext;
	*nX1 = pHallList->nHallx1;
	*nY1 = pHallList->nHally1;
	*nX2 = pHallList->nHallx2;
	*nY2 = pHallList->nHally2;
	*nHd = pHallList->nHalldir;
	MemFreeDbg(pHallList);
	pHallList = p1;
}

static void ConnectHall(int nX1, int nY1, int nX2, int nY2, int nHd)
{
	int nCurrd, nDx, nDy, nRp, nOrigX1, nOrigY1, fMinusFlag, fPlusFlag;
	bool fInroom;

	fMinusFlag = random_(0, 100);
	fPlusFlag = random_(0, 100);
	nOrigX1 = nX1;
	nOrigY1 = nY1;
	CreateDoorType(nX1, nY1);
	CreateDoorType(nX2, nY2);
	nCurrd = nHd;
	nX2 -= Dir_Xadd[nCurrd];
	nY2 -= Dir_Yadd[nCurrd];
	pdungeon[nX2][nY2] = 44;
	fInroom = false;

	while (TRUE) {
		if (nX1 >= DMAXX - 2 && nCurrd == 2) {
			nCurrd = 4;
		}
		if (nY1 >= DMAXY - 2 && nCurrd == 3) {
			nCurrd = 1;
		}
		if (nX1 <= 1 && nCurrd == 4) {
			nCurrd = 2;
		}
		if (nY1 <= 1 && nCurrd == 1) {
			nCurrd = 3;
		}
		if (pdungeon[nX1][nY1] == 67 && (nCurrd == 1 || nCurrd == 4)) {
			nCurrd = 2;
		}
		if (pdungeon[nX1][nY1] == 66 && (nCurrd == 1 || nCurrd == 2)) {
			nCurrd = 3;
		}
		if (pdungeon[nX1][nY1] == 69 && (nCurrd == 4 || nCurrd == 3)) {
			nCurrd = 1;
		}
		if (pdungeon[nX1][nY1] == 65 && (nCurrd == 2 || nCurrd == 3)) {
			nCurrd = 4;
		}
		nX1 += Dir_Xadd[nCurrd];
		nY1 += Dir_Yadd[nCurrd];
		if (pdungeon[nX1][nY1] == 32) {
			if (fInroom) {
				CreateDoorType(nX1 - Dir_Xadd[nCurrd], nY1 - Dir_Yadd[nCurrd]);
			} else {
				if (fMinusFlag < 50) {
					if (nCurrd != 1 && nCurrd != 3) {
						PlaceHallExt(nX1, nY1 - 1);
					} else {
						PlaceHallExt(nX1 - 1, nY1);
					}
				}
				if (fPlusFlag < 50) {
					if (nCurrd != 1 && nCurrd != 3) {
						PlaceHallExt(nX1, nY1 + 1);
					} else {
						PlaceHallExt(nX1 + 1, nY1);
					}
				}
			}
			pdungeon[nX1][nY1] = 44;
			fInroom = false;
		} else {
			if (!fInroom && pdungeon[nX1][nY1] == 35) {
				CreateDoorType(nX1, nY1);
			}
			if (pdungeon[nX1][nY1] != 44) {
				fInroom = true;
			}
		}
		nDx = abs(nX2 - nX1);
		nDy = abs(nY2 - nY1);
		if (nDx > nDy) {
			nRp = 2 * nDx;
			if (nRp > 30) {
				nRp = 30;
			}
			if (random_(0, 100) < nRp) {
				if (nX2 <= nX1 || nX1 >= DMAXX) {
					nCurrd = 4;
				} else {
					nCurrd = 2;
				}
			}
		} else {
			nRp = 5 * nDy;
			if (nRp > 80) {
				nRp = 80;
			}
			if (random_(0, 100) < nRp) {
				if (nY2 <= nY1 || nY1 >= DMAXY) {
					nCurrd = 1;
				} else {
					nCurrd = 3;
				}
			}
		}
		if (nDy < 10 && nX1 == nX2 && (nCurrd == 2 || nCurrd == 4)) {
			if (nY2 <= nY1 || nY1 >= DMAXY) {
				nCurrd = 1;
			} else {
				nCurrd = 3;
			}
		}
		if (nDx < 10 && nY1 == nY2 && (nCurrd == 1 || nCurrd == 3)) {
			if (nX2 <= nX1 || nX1 >= DMAXX) {
				nCurrd = 4;
			} else {
				nCurrd = 2;
			}
		}
		if (nDy == 1 && nDx > 1 && (nCurrd == 1 || nCurrd == 3)) {
			if (nX2 <= nX1 || nX1 >= DMAXX) {
				nCurrd = 4;
			} else {
				nCurrd = 2;
			}
		}
		if (nDx == 1 && nDy > 1 && (nCurrd == 2 || nCurrd == 4)) {
			if (nY2 <= nY1 || nX1 >= DMAXX) {
				nCurrd = 1;
			} else {
				nCurrd = 3;
			}
		}
		if (nDx == 0 && pdungeon[nX1][nY1] != 32 && (nCurrd == 2 || nCurrd == 4)) {
			if (nX2 <= nOrigX1 || nX1 >= DMAXX) {
				nCurrd = 1;
			} else {
				nCurrd = 3;
			}
		}
		if (nDy == 0 && pdungeon[nX1][nY1] != 32 && (nCurrd == 1 || nCurrd == 3)) {
			if (nY2 <= nOrigY1 || nY1 >= DMAXY) {
				nCurrd = 4;
			} else {
				nCurrd = 2;
			}
		}
		if (nX1 == nX2 && nY1 == nY2) {
			break;
		}
	}
}

static void DRLG_L2MakeMegas()
{
	int x, y, i, j, xx, yy;

	memset(dungeon, 3, sizeof(dungeon));

	for (y = 0; y < DMAXY; y++) {
		for (x = 0; x < DMAXX; x++) {
			for (i = lengthof(Patterns) - 1; i >= 0; i--) {
				xx = x - 1;
				yy = y - 1;
				for (j = 0; j < 9; j++, xx++) {
					if (j == 3 || j == 6) {
						yy++;
						xx = x - 1;
					}
					if (xx < 0 || xx >= DMAXX || yy < 0 || yy >= DMAXY)
						continue;
					switch (Patterns[i][j]) {
					case 0:
						continue;
					case 1:
						if (pdungeon[xx][yy] == 35) {
							continue;
						}
						break;
					case 2:
						if (pdungeon[xx][yy] == 46) {
							continue;
						}
						break;
					case 3:
						if (pdungeon[xx][yy] == 68) {
							continue;
						}
						break;
					case 4:
						if (pdungeon[xx][yy] == 32) {
							continue;
						}
						break;
					case 5:
						if (pdungeon[xx][yy] == 68 || pdungeon[xx][yy] == 46) {
							continue;
						}
						break;
					case 6:
						if (pdungeon[xx][yy] == 68 || pdungeon[xx][yy] == 35) {
							continue;
						}
						break;
					case 7:
						if (pdungeon[xx][yy] == 32 || pdungeon[xx][yy] == 46) {
							continue;
						}
						break;
					case 8:
						if (pdungeon[xx][yy] == 68 || pdungeon[xx][yy] == 35 || pdungeon[xx][yy] == 46) {
							continue;
						}
						break;
					default:
						ASSUME_UNREACHABLE
						break;
					}
					break;
				}
				if (j == 9) {
					dungeon[x][y] = Patterns[i][9];
					break;
				}
			}
		}
	}
}

static void L2TileFix()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 3) {
				dungeon[i][j + 1] = 1;
			}
			if (dungeon[i][j] == 3 && dungeon[i][j + 1] == 1) {
				dungeon[i][j + 1] = 3;
			}
			if (dungeon[i][j] == 3 && dungeon[i + 1][j] == 7) {
				dungeon[i + 1][j] = 3;
			}
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 3) {
				dungeon[i + 1][j] = 2;
			}
			if (dungeon[i][j] == 11 && dungeon[i + 1][j] == 14) {
				dungeon[i + 1][j] = 16;
			}
		}
	}
}

static int DL2_NumNoChar()
{
	int i, rv;
	BYTE *pTmp;

	rv = 0;
	static_assert(sizeof(pdungeon) == DMAXX * DMAXY, "Linear traverse of pdungeon does not work in DL2_NumNoChar.");
	pTmp = &pdungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++)
		if (*pTmp == 32)
			rv++;

	return rv;
}

static void DL2_DrawRoom(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			pdungeon[i][j] = 46;
		}
	}
	for (j = y1; j <= y2; j++) {
		pdungeon[x1][j] = 35;
		pdungeon[x2][j] = 35;
	}
	for (i = x1; i <= x2; i++) {
		pdungeon[i][y1] = 35;
		pdungeon[i][y2] = 35;
	}
}

static void DL2_KnockWalls(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (i = x1 + 1; i < x2; i++) {
		if (pdungeon[i][y1 - 1] == 46 && pdungeon[i][y1 + 1] == 46) {
			pdungeon[i][y1] = 46;
		}
		if (pdungeon[i][y2 - 1] == 46 && pdungeon[i][y2 + 1] == 46) {
			pdungeon[i][y2] = 46;
		}
		if (pdungeon[i][y1 - 1] == 68) {
			pdungeon[i][y1 - 1] = 46;
		}
		if (pdungeon[i][y2 + 1] == 68) {
			pdungeon[i][y2 + 1] = 46;
		}
	}
	for (j = y1 + 1; j < y2; j++) {
		if (pdungeon[x1 - 1][j] == 46 && pdungeon[x1 + 1][j] == 46) {
			pdungeon[x1][j] = 46;
		}
		if (pdungeon[x2 - 1][j] == 46 && pdungeon[x2 + 1][j] == 46) {
			pdungeon[x2][j] = 46;
		}
		if (pdungeon[x1 - 1][j] == 68) {
			pdungeon[x1 - 1][j] = 46;
		}
		if (pdungeon[x2 + 1][j] == 68) {
			pdungeon[x2 + 1][j] = 46;
		}
	}
}

static bool DL2_FillVoids()
{
	int i, j, xx, yy, x1, x2, y1, y2;
	bool xf1, xf2, yf1, yf2;
	int tries;

	for (tries = 0; DL2_NumNoChar() > 700 && tries < 100; tries++) {
		do {
			xx = RandRange(1, 38);
			yy = RandRange(1, 38);
		} while (pdungeon[xx][yy] != 35);
		if (pdungeon[xx - 1][yy] == 32 && pdungeon[xx + 1][yy] == 46) {
			if (pdungeon[xx + 1][yy - 1] != 46
			 || pdungeon[xx + 1][yy + 1] != 46
			 || pdungeon[xx - 1][yy - 1] != 32
			 || pdungeon[xx - 1][yy + 1] != 32)
				continue;
			xf2 = false;
			xf1 = yf1 = yf2 = true;
		} else if (pdungeon[xx + 1][yy] == 32 && pdungeon[xx - 1][yy] == 46) {
			if (pdungeon[xx - 1][yy - 1] != 46
			 || pdungeon[xx - 1][yy + 1] != 46
			 || pdungeon[xx + 1][yy - 1] != 32
			 || pdungeon[xx + 1][yy + 1] != 32)
				continue;
			xf1 = false;
			xf2 = yf1 = yf2 = true;
		} else if (pdungeon[xx][yy - 1] == 32 && pdungeon[xx][yy + 1] == 46) {
			if (pdungeon[xx - 1][yy + 1] != 46
			 || pdungeon[xx + 1][yy + 1] != 46
			 || pdungeon[xx - 1][yy - 1] != 32
			 || pdungeon[xx + 1][yy - 1] != 32)
				continue;
			yf2 = false;
			yf1 = xf1 = xf2 = true;
		} else if (pdungeon[xx][yy + 1] == 32 && pdungeon[xx][yy - 1] == 46) {
			if (pdungeon[xx - 1][yy - 1] != 46
			 || pdungeon[xx + 1][yy - 1] != 46
			 || pdungeon[xx - 1][yy + 1] != 32
			 || pdungeon[xx + 1][yy + 1] != 32)
				continue;
			yf1 = false;
			yf2 = xf1 = xf2 = true;
		} else
			continue;
		if (xf1) {
			x1 = xx - 1;
		} else {
			x1 = xx;
		}
		if (xf2) {
			x2 = xx + 1;
		} else {
			x2 = xx;
		}
		if (yf1) {
			y1 = yy - 1;
		} else {
			y1 = yy;
		}
		if (yf2) {
			y2 = yy + 1;
		} else {
			y2 = yy;
		}
		if (!xf1) {
			while (yf1 || yf2) {
				if (y1 == 0) {
					yf1 = false;
				}
				if (y2 == DMAXY - 1) {
					yf2 = false;
				}
				if (y2 - y1 >= 14) {
					yf1 = false;
					yf2 = false;
				}
				if (yf1) {
					y1--;
				}
				if (yf2) {
					y2++;
				}
				if (pdungeon[x2][y1] != 32) {
					yf1 = false;
				}
				if (pdungeon[x2][y2] != 32) {
					yf2 = false;
				}
			}
			y1 += 2;
			y2 -= 2;
			if (y2 - y1 > 5) {
				while (xf2) {
					if (x2 == DMAXX - 1) {
						xf2 = false;
					}
					if (x2 - x1 >= 12) {
						xf2 = false;
					}
					for (j = y1; j <= y2; j++) {
						if (pdungeon[x2][j] != 32) {
							xf2 = false;
						}
					}
					if (xf2) {
						x2++;
					}
				}
				x2 -= 2;
				if (x2 - x1 > 5) {
					DL2_DrawRoom(x1, y1, x2, y2);
					DL2_KnockWalls(x1, y1, x2, y2);
				}
			}
		} else if (!xf2) {
			while (yf1 || yf2) {
				if (y1 == 0) {
					yf1 = false;
				}
				if (y2 == DMAXY - 1) {
					yf2 = false;
				}
				if (y2 - y1 >= 14) {
					yf1 = false;
					yf2 = false;
				}
				if (yf1) {
					y1--;
				}
				if (yf2) {
					y2++;
				}
				if (pdungeon[x1][y1] != 32) {
					yf1 = false;
				}
				if (pdungeon[x1][y2] != 32) {
					yf2 = false;
				}
			}
			y1 += 2;
			y2 -= 2;
			if (y2 - y1 > 5) {
				while (xf1) {
					if (x1 == 0) {
						xf1 = false;
					}
					if (x2 - x1 >= 12) {
						xf1 = false;
					}
					for (j = y1; j <= y2; j++) {
						if (pdungeon[x1][j] != 32) {
							xf1 = false;
						}
					}
					if (xf1) {
						x1--;
					}
				}
				x1 += 2;
				if (x2 - x1 > 5) {
					DL2_DrawRoom(x1, y1, x2, y2);
					DL2_KnockWalls(x1, y1, x2, y2);
				}
			}
		} else if (!yf1) {
			while (xf1 || xf2) {
				if (x1 == 0) {
					xf1 = false;
				}
				if (x2 == DMAXX - 1) {
					xf2 = false;
				}
				if (x2 - x1 >= 14) {
					xf1 = false;
					xf2 = false;
				}
				if (xf1) {
					x1--;
				}
				if (xf2) {
					x2++;
				}
				if (pdungeon[x1][y2] != 32) {
					xf1 = false;
				}
				if (pdungeon[x2][y2] != 32) {
					xf2 = false;
				}
			}
			x1 += 2;
			x2 -= 2;
			if (x2 - x1 > 5) {
				while (yf2) {
					if (y2 == DMAXY - 1) {
						yf2 = false;
					}
					if (y2 - y1 >= 12) {
						yf2 = false;
					}
					for (i = x1; i <= x2; i++) {
						if (pdungeon[i][y2] != 32) {
							yf2 = false;
						}
					}
					if (yf2) {
						y2++;
					}
				}
				y2 -= 2;
				if (y2 - y1 > 5) {
					DL2_DrawRoom(x1, y1, x2, y2);
					DL2_KnockWalls(x1, y1, x2, y2);
				}
			}
		} else if (!yf2) {
			while (xf1 || xf2) {
				if (x1 == 0) {
					xf1 = false;
				}
				if (x2 == DMAXX - 1) {
					xf2 = false;
				}
				if (x2 - x1 >= 14) {
					xf1 = false;
					xf2 = false;
				}
				if (xf1) {
					x1--;
				}
				if (xf2) {
					x2++;
				}
				if (pdungeon[x1][y1] != 32) {
					xf1 = false;
				}
				if (pdungeon[x2][y1] != 32) {
					xf2 = false;
				}
			}
			x1 += 2;
			x2 -= 2;
			if (x2 - x1 > 5) {
				while (yf1) {
					if (y1 == 0) {
						yf1 = false;
					}
					if (y2 - y1 >= 12) {
						yf1 = false;
					}
					for (i = x1; i <= x2; i++) {
						if (pdungeon[i][y1] != 32) {
							yf1 = false;
						}
					}
					if (yf1) {
						y1--;
					}
				}
				y1 += 2;
				if (y2 - y1 > 5) {
					DL2_DrawRoom(x1, y1, x2, y2);
					DL2_KnockWalls(x1, y1, x2, y2);
				}
			}
		}
	}

	return DL2_NumNoChar() <= 700;
}

static bool DRLG_L2CreateDungeon()
{
	int i, j, nHx1, nHy1, nHx2, nHy2, nHd, ForceH, ForceW;

	ForceW = 0;
	ForceH = 0;

	if (pSetPiece != NULL) {
		ForceW = pSetPiece[0] + 4; // TODO: add border to the setmaps?
		ForceH = pSetPiece[2] + 4;
	}

	CreateRoom(1, 1, DMAXX - 2, DMAXY - 2, 0, 0, ForceH, ForceW);

	while (pHallList != NULL) {
		GetHall(&nHx1, &nHy1, &nHx2, &nHy2, &nHd);
		ConnectHall(nHx1, nHy1, nHx2, nHy2, nHd);
	}

	for (j = 0; j < DMAXY; j++) {     /// BUGFIX: change '<=' to '<' (fixed)
		for (i = 0; i < DMAXX; i++) { /// BUGFIX: change '<=' to '<' (fixed)
			if (pdungeon[i][j] == 67
			 || pdungeon[i][j] == 66
			 || pdungeon[i][j] == 69
			 || pdungeon[i][j] == 65) {
				pdungeon[i][j] = 35;
			} else if (pdungeon[i][j] == 44) {
				pdungeon[i][j] = 46;
				if (pdungeon[i - 1][j - 1] == 32) {
					pdungeon[i - 1][j - 1] = 35;
				}
				if (pdungeon[i - 1][j] == 32) {
					pdungeon[i - 1][j] = 35;
				}
				if (pdungeon[i - 1][1 + j] == 32) {
					pdungeon[i - 1][1 + j] = 35;
				}
				if (pdungeon[i + 1][j - 1] == 32) {
					pdungeon[i + 1][j - 1] = 35;
				}
				if (pdungeon[i + 1][j] == 32) {
					pdungeon[i + 1][j] = 35;
				}
				if (pdungeon[i + 1][1 + j] == 32) {
					pdungeon[i + 1][1 + j] = 35;
				}
				if (pdungeon[i][j - 1] == 32) {
					pdungeon[i][j - 1] = 35;
				}
				if (pdungeon[i][j + 1] == 32) {
					pdungeon[i][j + 1] = 35;
				}
			}
		}
	}

	return DL2_FillVoids();
}

static void DRLG_L2TransFix()
{
	int i, j, xx, yy;

	yy = DBORDERY;
	for (j = 0; j < DMAXY; j++) {
		xx = DBORDERX;
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 10:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 11:
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			case 14:
				if (dungeon[i][j - 1] == 10) {
					DRLG_CopyTrans(xx, yy, xx + 1, yy);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 15:
				if (dungeon[i + 1][j] == 11) {
					DRLG_CopyTrans(xx, yy, xx, yy + 1);
					DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				}
				break;
			case 16:
				DRLG_CopyTrans(xx, yy, xx + 1, yy);
				DRLG_CopyTrans(xx, yy, xx, yy + 1);
				DRLG_CopyTrans(xx, yy, xx + 1, yy + 1);
				break;
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void L2DirtFix()
{
	int i, j;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			switch (dungeon[i][j]) {
			case 10:
				if (dungeon[i][j + 1] != 10)
					dungeon[i][j] = 143;
				break;
			case 11:
				if (dungeon[i + 1][j] != 11)
					dungeon[i][j] = 144;
				break;
			case 13:
				if (dungeon[i + 1][j] != 11 || dungeon[i][j + 1] != 10)
					dungeon[i][j] = 146;
				break;
			case 14:
				if (dungeon[i][j + 1] != 15)
					dungeon[i][j] = 147;
				break;
			case 15:
				if (dungeon[i + 1][j] != 11)
					dungeon[i][j] = 148;
				break;
			}
		}
	}
}

static void L2LockoutFix()
{
	int i, j;
	bool doorok;

	for (j = 0; j < DMAXY; j++) {
		for (i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 4 && dungeon[i - 1][j] != 3) {
				dungeon[i][j] = 1;
			}
			if (dungeon[i][j] == 5 && dungeon[i][j - 1] != 3) {
				dungeon[i][j] = 2;
			}
		}
	}
	for (j = 1; j < DMAXY - 1; j++) {
		for (i = 1; i < DMAXX - 1; i++) {
			if (dflags[i][j] != 0) {
				continue;
			}
			if ((dungeon[i][j] == 2 || dungeon[i][j] == 5) && dungeon[i][j - 1] == 3 && dungeon[i][j + 1] == 3) {
				doorok = false;
				do {
					if (dungeon[i][j - 1] != 3 || dungeon[i][j + 1] != 3) {
						break;
					}
					if (dungeon[i][j] == 5) {
						doorok = true;
						break;
					}
					i++;
				} while (dungeon[i][j] == 2 || dungeon[i][j] == 5);
				if (!doorok && !dflags[i - 1][j]) {
					dungeon[i - 1][j] = 5;
				}
			}
		}
	}
	for (i = 1; i < DMAXX - 1; i++) {
		for (j = 1; j < DMAXY - 1; j++) {
			if (dflags[i][j]) {
				continue;
			}
			if ((dungeon[i][j] == 1 || dungeon[i][j] == 4) && dungeon[i - 1][j] == 3 && dungeon[i + 1][j] == 3) {
				doorok = false;
				do {
					if (dungeon[i - 1][j] != 3 || dungeon[i + 1][j] != 3) {
						break;
					}
					if (dungeon[i][j] == 4) {
						doorok = true;
						break;
					}
					j++;
				} while (dungeon[i][j] == 1 || dungeon[i][j] == 4);
				if (!doorok && !dflags[i][j - 1]) {
					dungeon[i][j - 1] = 4;
				}
			}
		}
	}
}

static void L2DoorFix()
{
	int i, j;

	for (j = 1; j < DMAXY; j++) {
		for (i = 1; i < DMAXX; i++) {
			if (dungeon[i][j] == 4 && dungeon[i][j - 1] == 3) {
				dungeon[i][j] = 7;
			}
			if (dungeon[i][j] == 5 && dungeon[i - 1][j] == 3) {
				dungeon[i][j] = 9;
			}
		}
	}
}

static bool IsPillar(BYTE bv)
{
	return (bv >= 6 && bv <= 9) || (bv >= 13 && bv <= 16);
}

/*
 * Move doors away from pillars.
 */
static void L2DoorFix2()
{
	int i, j;

	for (i = 1; i < DMAXX - 1; i++) {
		for (j = 1; j < DMAXY - 1; j++) {
			if (dungeon[i][j] != 4 || dflags[i][j])
				continue;
			if (IsPillar(dungeon[i][j + 1])) {
				//3, 1, 3,  search
				//0, 4, 0,
				//0, P, 0,

				//0, 4, 0, replace
				//0, 1, 0,
				//0, 0, 0,
				if (dungeon[i][j - 1] == 1
				 && dungeon[i - 1][j - 1] == 3
				 && dungeon[i + 1][j - 1] == 3) {
					dungeon[i][j] = 1;
					dungeon[i][j - 1] = 4;
				}
			} else if (IsPillar(dungeon[i][j - 1])) {
				//0, P, 0,  search
				//0, 4, 0,
				//3, 1, 3,

				//0, 0, 0, replace
				//0, 1, 0,
				//0, 4, 0,
				if (dungeon[i][j + 1] == 1
				 && dungeon[i - 1][j + 1] == 3
				 && dungeon[i + 1][j + 1] == 3) {
					dungeon[i][j] = 1;
					dungeon[i][j + 1] = 4;
				}
			} else if (IsPillar(dungeon[i + 1][j])) {
				//3, 0, 0,  search
				//1, 4, P,
				//3, 0, 0,

				//0, 0, 0, replace
				//4, 1, 0,
				//0, 0, 0,
				if (dungeon[i - 1][j] == 1
				 && dungeon[i - 1][j + 1] == 3
				 && dungeon[i - 1][j - 1] == 3) {
					dungeon[i][j] = 1;
					dungeon[i - 1][j] = 4;
				}
			} else if (IsPillar(dungeon[i - 1][j])) {
				//0, 0, 3,  search
				//P, 4, 1,
				//0, 0, 3,

				//0, 0, 0, replace
				//0, 1, 4,
				//0, 0, 0,
				if (dungeon[i + 1][j] == 1
				 && dungeon[i + 1][j - 1] == 3
				 && dungeon[i + 1][j + 1] == 3) {
					dungeon[i][j] = 1;
					dungeon[i + 1][j] = 4;
				}
			}
		}
	}
}

struct mini_set {
	const BYTE* data;
	BOOL setview;
};
static bool DRLG_L2PlaceMiniSets(mini_set* minisets, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (minisets[i].data != NULL && !DRLG_L2PlaceMiniSet(minisets[i].data, minisets[i].setview)) {
			return false;
		}
	}
	return true;
}

static void DRLG_L2(int entry)
{
	bool doneflag;

	do {
		do {
			nRoomCnt = 0;
			DRLG_L2InitDungeon();
			DRLG_InitTrans();
		} while (!DRLG_L2CreateDungeon());

		DRLG_L2MakeMegas();

		L2TileFix();
		if (pSetPiece != NULL) {
			DRLG_L2SetRoom(setpc_x, setpc_y);
		}
		DRLG_FloodTVal(3);
		DRLG_L2TransFix();

		mini_set stairs[3] = {
				{ L2USTAIRS, entry == ENTRY_MAIN },
				{ L2DSTAIRS, entry == ENTRY_PREV },
				{ currLvl._dLevelIdx != DLV_CATACOMBS1 ? NULL : L2TWARP, entry != ENTRY_MAIN  && entry != ENTRY_PREV }
		};
		doneflag = DRLG_L2PlaceMiniSets(stairs, 3);
		if (entry == ENTRY_MAIN) {
			ViewY -= 2;
		} else if (entry == ENTRY_PREV) {
			ViewX--;
		} else {
			ViewY -= 2;
		}
	} while (!doneflag);

	L2LockoutFix();
	L2DoorFix();
	L2DirtFix();

	DRLG_PlaceThemeRooms(6, 10, 3, 0, false);

	L2DoorFix2();

	DRLG_L2PlaceRndSet(VARCH33, 100);
	DRLG_L2PlaceRndSet(VARCH34, 100);
	DRLG_L2PlaceRndSet(VARCH35, 100);
	DRLG_L2PlaceRndSet(VARCH36, 100);
	DRLG_L2PlaceRndSet(VARCH37, 100);
	DRLG_L2PlaceRndSet(VARCH38, 100);
	DRLG_L2PlaceRndSet(VARCH39, 100);
	DRLG_L2PlaceRndSet(VARCH40, 100);
	DRLG_L2PlaceRndSet(VARCH1, 100);
	DRLG_L2PlaceRndSet(VARCH2, 100);
	DRLG_L2PlaceRndSet(VARCH3, 100);
	DRLG_L2PlaceRndSet(VARCH4, 100);
	DRLG_L2PlaceRndSet(VARCH5, 100);
	DRLG_L2PlaceRndSet(VARCH6, 100);
	DRLG_L2PlaceRndSet(VARCH7, 100);
	DRLG_L2PlaceRndSet(VARCH8, 100);
	DRLG_L2PlaceRndSet(VARCH9, 100);
	DRLG_L2PlaceRndSet(VARCH10, 100);
	DRLG_L2PlaceRndSet(VARCH11, 100);
	DRLG_L2PlaceRndSet(VARCH12, 100);
	DRLG_L2PlaceRndSet(VARCH13, 100);
	DRLG_L2PlaceRndSet(VARCH14, 100);
	DRLG_L2PlaceRndSet(VARCH15, 100);
	DRLG_L2PlaceRndSet(VARCH16, 100);
	DRLG_L2PlaceRndSet(VARCH17, 100);
	DRLG_L2PlaceRndSet(VARCH18, 100);
	DRLG_L2PlaceRndSet(VARCH19, 100);
	DRLG_L2PlaceRndSet(VARCH20, 100);
	DRLG_L2PlaceRndSet(VARCH21, 100);
	DRLG_L2PlaceRndSet(VARCH22, 100);
	DRLG_L2PlaceRndSet(VARCH23, 100);
	DRLG_L2PlaceRndSet(VARCH24, 100);
	DRLG_L2PlaceRndSet(VARCH25, 100);
	DRLG_L2PlaceRndSet(VARCH26, 100);
	DRLG_L2PlaceRndSet(VARCH27, 100);
	DRLG_L2PlaceRndSet(VARCH28, 100);
	DRLG_L2PlaceRndSet(VARCH29, 100);
	DRLG_L2PlaceRndSet(VARCH30, 100);
	DRLG_L2PlaceRndSet(VARCH31, 100);
	DRLG_L2PlaceRndSet(VARCH32, 100);
	DRLG_L2PlaceRndSet(HARCH1, 100);
	DRLG_L2PlaceRndSet(HARCH2, 100);
	DRLG_L2PlaceRndSet(HARCH3, 100);
	DRLG_L2PlaceRndSet(HARCH4, 100);
	DRLG_L2PlaceRndSet(HARCH5, 100);
	DRLG_L2PlaceRndSet(HARCH6, 100);
	DRLG_L2PlaceRndSet(HARCH7, 100);
	DRLG_L2PlaceRndSet(HARCH8, 100);
	DRLG_L2PlaceRndSet(HARCH9, 100);
	DRLG_L2PlaceRndSet(HARCH10, 100);
	DRLG_L2PlaceRndSet(HARCH11, 100);
	DRLG_L2PlaceRndSet(HARCH12, 100);
	DRLG_L2PlaceRndSet(HARCH13, 100);
	DRLG_L2PlaceRndSet(HARCH14, 100);
	DRLG_L2PlaceRndSet(HARCH15, 100);
	DRLG_L2PlaceRndSet(HARCH16, 100);
	DRLG_L2PlaceRndSet(HARCH17, 100);
	DRLG_L2PlaceRndSet(HARCH18, 100);
	DRLG_L2PlaceRndSet(HARCH19, 100);
	DRLG_L2PlaceRndSet(HARCH20, 100);
	DRLG_L2PlaceRndSet(HARCH21, 100);
	DRLG_L2PlaceRndSet(HARCH22, 100);
	DRLG_L2PlaceRndSet(HARCH23, 100);
	DRLG_L2PlaceRndSet(HARCH24, 100);
	DRLG_L2PlaceRndSet(HARCH25, 100);
	DRLG_L2PlaceRndSet(HARCH26, 100);
	DRLG_L2PlaceRndSet(HARCH27, 100);
	DRLG_L2PlaceRndSet(HARCH28, 100);
	DRLG_L2PlaceRndSet(HARCH29, 100);
	DRLG_L2PlaceRndSet(HARCH30, 100);
	DRLG_L2PlaceRndSet(HARCH31, 100);
	DRLG_L2PlaceRndSet(HARCH32, 100);
	DRLG_L2PlaceRndSet(HARCH33, 100);
	DRLG_L2PlaceRndSet(HARCH34, 100);
	DRLG_L2PlaceRndSet(HARCH35, 100);
	DRLG_L2PlaceRndSet(HARCH36, 100);
	DRLG_L2PlaceRndSet(HARCH37, 100);
	DRLG_L2PlaceRndSet(HARCH38, 100);
	DRLG_L2PlaceRndSet(HARCH39, 100);
	DRLG_L2PlaceRndSet(HARCH40, 100);
	DRLG_L2PlaceRndSet(CRUSHCOL, 99);
	DRLG_L2PlaceRndSet(RUINS1, 10);
	DRLG_L2PlaceRndSet(RUINS2, 10);
	DRLG_L2PlaceRndSet(RUINS3, 10);
	DRLG_L2PlaceRndSet(RUINS4, 10);
	DRLG_L2PlaceRndSet(RUINS5, 10);
	DRLG_L2PlaceRndSet(RUINS6, 10);
	DRLG_L2PlaceRndSet(RUINS7, 50);
	DRLG_L2PlaceRndSet(PANCREAS1, 1);
	DRLG_L2PlaceRndSet(PANCREAS2, 1);
	DRLG_L2PlaceRndSet(BIG1, 3);
	DRLG_L2PlaceRndSet(BIG2, 3);
	DRLG_L2PlaceRndSet(BIG3, 3);
	DRLG_L2PlaceRndSet(BIG4, 3);
	DRLG_L2PlaceRndSet(BIG5, 3);
	DRLG_L2PlaceRndSet(BIG6, 20);
	DRLG_L2PlaceRndSet(BIG7, 20);
	DRLG_L2PlaceRndSet(BIG8, 3);
	DRLG_L2PlaceRndSet(BIG9, 20);
	DRLG_L2PlaceRndSet(BIG10, 20);
	DRLG_L2Subs();
	DRLG_L2Shadows();

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	DRLG_Init_Globals();
	DRLG_CheckQuests(setpc_x, setpc_y);
}

static void DRLG_InitL2Vals()
{
	int i, j, *dp;
	BYTE pc, *dsp;

	static_assert(sizeof(dPiece) == MAXDUNX * MAXDUNY * sizeof(int), "Linear traverse of dPiece does not work in DRLG_InitL2Vals.");
	static_assert(sizeof(dSpecial) == MAXDUNX * MAXDUNY, "Linear traverse of dSpecial does not work in DRLG_InitL2Vals.");
	dsp = &dSpecial[0][0];
	dp = &dPiece[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, dsp++, dp++) {
		if (*dp == 541 || *dp == 178 || *dp == 551)
			pc = 5;
		else if (*dp == 542 || *dp == 553)
			pc = 6;
		else
			continue;
		*dsp = pc;
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dPiece[i][j] == 132) {
				dSpecial[i][j + 1] = 2;
				dSpecial[i][j + 2] = 1;
			} else if (dPiece[i][j] == 135 || dPiece[i][j] == 139) {
				dSpecial[i + 1][j] = 3;
				dSpecial[i + 2][j] = 4;
			}
		}
	}
}

static BYTE *LoadL2DungeonData(const char *sFileName)
{
	int i, j, rw, rh;
	BYTE *pLevelMap, *lm, *pTmp;

	//DRLG_L2InitDungeon();
	//DRLG_InitTrans();
	pLevelMap = LoadFileInMem(sFileName, NULL);

	memset(dflags, 0, sizeof(dflags));
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL2DungeonData.");
	memset(dungeon, 12, sizeof(dungeon));

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
				dungeon[i][j] = 3;
			}
			lm += 2;
		}
	}
	static_assert(sizeof(dungeon) == DMAXX * DMAXY, "Linear traverse of dungeon does not work in LoadL2DungeonData II.");
	pTmp = &dungeon[0][0];
	for (i = 0; i < DMAXX * DMAXY; i++, pTmp++)
		if (*pTmp == 0)
			*pTmp = 12;

	return pLevelMap;
}

void LoadL2Dungeon(const char *sFileName, int vx, int vy)
{
	BYTE *pLevelMap;

	ViewX = vx;
	ViewY = vy;

	pLevelMap = LoadL2DungeonData(sFileName);

	DRLG_PlaceMegaTiles(BASE_MEGATILE_L2);

	DRLG_InitTrans();
	DRLG_Init_Globals();
	DRLG_InitL2Vals();

	SetMapMonsters(pLevelMap, 0, 0);
	SetMapObjects(pLevelMap);
	mem_free_dbg(pLevelMap);
}

void LoadPreL2Dungeon(const char *sFileName)
{
	BYTE *pLevelMap = LoadL2DungeonData(sFileName);

	memcpy(pdungeon, dungeon, sizeof(pdungeon));

	mem_free_dbg(pLevelMap);
}

void CreateL2Dungeon(int entry)
{
	// in the original version the function was executed twice in case the quest of the
	// current level was not available (only in single player mode). The point of this
	// could have been to share the same layout between levels, but that does not make too
	// much sense due to the stairs placement are 'wrong' anyway. Just to have a reasonable
	// sized main room, changing DRLG_L2CreateDungeon would have been much cheaper solution.
	DRLG_InitSetPC();
	DRLG_LoadL2SP();
	DRLG_L2(entry);
	DRLG_PlaceMegaTiles(BASE_MEGATILE_L2);
	DRLG_FreeL2SP();
	DRLG_InitL2Vals();
	DRLG_SetPC();
}

DEVILUTION_END_NAMESPACE
