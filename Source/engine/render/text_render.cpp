/**
 * @file text_render.cpp
 *
 * Text rendering.
 */
#include "text_render.h"

#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/raw_render.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * Maps ASCII character code to font frame number as used by the
 * small (SmalText.CEL) and medium (MedTextS.CEL) sized fonts.
 */
const BYTE gbStdFontFrame[256] = {
	// clang-format off
/*  0-*/      '\0',  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,
/* 16-*/  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,
/* 32-*/  0/*' '*/,  1/*'!'*/,  2/*'"'*/,  3/*'#'*/,  4/*'$'*/,  5/*'%'*/,  6/*'&'*/,  7/*'''*/,  8/*'('*/,  9/*')'*/, 10/*'*'*/, 11/*'+'*/, 12/*','*/, 13/*'-'*/, 14/*'.'*/, 15/*'/'*/,
/* 48-*/ 16/*'0'*/, 17/*'1'*/, 18/*'2'*/, 19/*'3'*/, 20/*'4'*/, 21/*'5'*/, 22/*'6'*/, 23/*'7'*/, 24/*'8'*/, 25/*'9'*/, 26/*':'*/, 27/*';'*/, 28/*'<'*/, 29/*'='*/, 30/*'>'*/, 31/*'?'*/,
/* 64-*/ 32/*'@'*/, 33/*'A'*/, 34/*'B'*/, 35/*'C'*/, 36/*'D'*/, 37/*'E'*/, 38/*'F'*/, 39/*'G'*/, 40/*'H'*/, 41/*'I'*/, 42/*'J'*/, 43/*'K'*/, 44/*'L'*/, 45/*'M'*/, 46/*'N'*/, 47/*'O'*/,
/* 80-*/ 48/*'P'*/, 49/*'Q'*/, 50/*'R'*/, 51/*'S'*/, 52/*'T'*/, 53/*'U'*/, 54/*'V'*/, 55/*'W'*/, 56/*'X'*/, 57/*'Y'*/, 58/*'Z'*/, 59/*'['*/, 60/*'\'*/, 61/*']'*/, 62/*'^'*/, 63/*'_'*/,
/* 96-*/ 64/*'`'*/, 33/*'a'*/, 34/*'b'*/, 35/*'c'*/, 36/*'d'*/, 37/*'e'*/, 38/*'f'*/, 39/*'g'*/, 40/*'h'*/, 41/*'i'*/, 42/*'j'*/, 43/*'k'*/, 44/*'l'*/, 45/*'m'*/, 46/*'n'*/, 47/*'o'*/,
/*112-*/ 48/*'p'*/, 49/*'q'*/, 50/*'r'*/, 51/*'s'*/, 52/*'t'*/, 53/*'u'*/, 54/*'v'*/, 55/*'w'*/, 56/*'x'*/, 57/*'y'*/, 58/*'z'*/, 65/*'{'*/, 66/*'|'*/, 67/*'}'*/, 68/*'~'*/,  0/* ? */,
/*128-*/ 35/*'C'*/,  0/* ? */, 12/*','*/, 69/*'f'*/, 70/*','*/, 71/*'.'*/, 72/*'+'*/, 88/*'+'*/, 73/*'^'*/, 74/*'%'*/, 51/*'S'*/, 28/*'<'*/, 75/*'O'*/,  0/* ? */, 58/*'Z'*/, 53/*'U'*/,
/*144-*/  0/* ? */,  7/*'''*/, 64/*'''*/,  2/*'"'*/,  2/*'"'*/, 10/*'*'*/, 13/*'-'*/, 13/*'-'*/, 76/*'"'*/, 77/*'t'*/, 51/*'s'*/, 30/*'>'*/, 75/*'o'*/,  0/* ? */, 58/*'z'*/, 57/*'Y'*/,
/*160-*/  0/*' '*/, 78/*'i'*/, 35/*'c'*/, 44/*'L'*/, 96/*'0'*/, 57/*'Y'*/, 79/*'|'*/, 80/*'s'*/, 81/*'"'*/, 82/*'c'*/, 83/*'a'*/, 84/*'<'*/, 68/*'~'*/, 13/*'-'*/, 85/*'r'*/, 86/*'-'*/,
/*176-*/ 87/*'o'*/, 88/*'+'*/, 89/*'2'*/, 90/*'3'*/,  7/*'''*/, 91/*'u'*/, 92/*'P'*/, 93/*'.'*/, 94/*','*/, 95/*'1'*/, 96/*'0'*/, 97/*'>'*/, 98/*'4'*/, 99/*'2'*/,100/*'3'*/,101/*'?'*/,
/*192-*/ 33/*'A'*/, 33/*'A'*/, 33/*'A'*/, 33/*'A'*/, 33/*'A'*/, 33/*'A'*/,102/*'A'*/, 35/*'C'*/, 37/*'E'*/, 37/*'E'*/, 37/*'E'*/, 37/*'E'*/,103/*'I'*/,103/*'I'*/,103/*'I'*/,104/*'I'*/,
/*208-*/ 36/*'D'*/, 46/*'N'*/, 47/*'O'*/, 47/*'O'*/, 47/*'O'*/, 47/*'O'*/, 47/*'O'*/, 10/*'*'*/, 16/*'0'*/, 53/*'U'*/, 53/*'U'*/, 53/*'U'*/, 53/*'U'*/, 57/*'Y'*/, 36/*'D'*/,105/*'B'*/,
/*224-*/ 33/*'a'*/, 33/*'a'*/, 33/*'a'*/, 33/*'a'*/, 33/*'a'*/, 33/*'a'*/,102/*'a'*/, 35/*'c'*/, 37/*'e'*/, 37/*'e'*/, 37/*'e'*/, 37/*'e'*/,103/*'i'*/,103/*'i'*/,103/*'i'*/,104/*'i'*/,
/*240-*/ 36/*'d'*/, 46/*'n'*/, 47/*'o'*/, 47/*'o'*/, 47/*'o'*/, 47/*'o'*/, 47/*'o'*/,106/*'/'*/, 16/*'0'*/, 53/*'u'*/, 53/*'u'*/, 53/*'u'*/, 53/*'u'*/, 57/*'y'*/, 36/*'d'*/, 57/*'y'*/,
	// clang-format on
};
/**
 * Maps ASCII character code to diacritic frame number as used by the
 * small (SmalText.CEL) and medium (MedTextS.CEL) sized fonts.
 */
static const BYTE gbStdDiacFrame[128] = {
	// clang-format off
/*128-*/ 29/*'='*/,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */, 72/*'+'*/,  0/*   */,  0/*   */, 73/*'^'*/,  0/*   */,  0/*   */,  0/*   */, 73/*'^'*/, 76/*'"'*/,
/*144-*/  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */, 73/*'^'*/,  0/*   */,  0/*   */,  0/*   */, 73/*'^'*/, 81/*'"'*/,
/*160-*/  0/*   */,  0/*   */, 15/*'/'*/, 13/*'-'*/,  0/*   */, 29/*'='*/,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,
/*176-*/  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,  0/*   */,
/*192-*/  7/*'''*/, 64/*'''*/, 73/*'^'*/, 76/*'"'*/, 81/*'"'*/, 87/*'o'*/,  0/*   */, 94/*','*/,  7/*'''*/, 64/*'''*/, 73/*'^'*/, 81/*'"'*/, 64/*'''*/, 64/*'''*/, 64/*'''*/, 81/*'"'*/,
/*208-*/ 13/*'-'*/, 76/*'"'*/,  7/*'''*/, 64/*'''*/, 73/*'^'*/, 76/*'"'*/, 81/*'"'*/,  0/*   */, 15/*'/'*/,  7/*'''*/, 64/*'''*/, 73/*'^'*/, 81/*'"'*/, 64/*'''*/, 66/*'|'*/,  0/*   */,
/*224-*/  7/*'''*/, 64/*'''*/, 73/*'^'*/, 76/*'"'*/, 81/*'"'*/, 87/*'o'*/,  0/*   */, 94/*','*/,  7/*'''*/, 64/*'''*/, 73/*'^'*/, 81/*'"'*/, 64/*'''*/, 64/*'''*/, 64/*'''*/, 81/*'"'*/,
/*240-*/ 13/*'-'*/, 76/*'"'*/,  7/*'''*/, 64/*'''*/, 73/*'^'*/, 76/*'"'*/, 81/*'"'*/,  0/*   */, 15/*'/'*/,  7/*'''*/, 64/*'''*/, 73/*'^'*/, 81/*'"'*/, 64/*'''*/, 66/*'|'*/, 81/*'"'*/,
	// clang-format on
};
/**
 * Maps ASCII character code to font frame number as used by the
 * large (BigTGold.CEL) sized font.
 */
static const BYTE gbHugeFontFrame[128] = {
	// clang-format off
/*  0-*/      '\0',  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,
/* 16-*/  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,  0/* ? */,
/* 32-*/  0/*' '*/,  0/*'!'*/,  0/*'"'*/,  0/*'#'*/,  0/*'$'*/,  0/*'%'*/,  0/*'&'*/,  0/*'''*/,  0/*'('*/,  0/*')'*/,  0/*'*'*/,  0/*'+'*/,  0/*','*/,  0/*'-'*/,  0/*'.'*/,  0/*'/'*/,
/* 48-*/  0/*'0'*/,  0/*'1'*/,  0/*'2'*/,  0/*'3'*/,  0/*'4'*/,  0/*'5'*/,  0/*'6'*/,  0/*'7'*/,  0/*'8'*/,  0/*'9'*/,  0/*':'*/,  0/*';'*/,  0/*'<'*/,  0/*'='*/,  0/*'>'*/,  0/*'?'*/,
/* 64-*/  0/*'@'*/,  1/*'A'*/,  2/*'B'*/,  3/*'C'*/,  4/*'D'*/,  5/*'E'*/,  6/*'F'*/,  7/*'G'*/,  8/*'H'*/,  9/*'I'*/, 10/*'J'*/, 11/*'K'*/, 12/*'L'*/, 13/*'M'*/, 14/*'N'*/, 15/*'O'*/,
/* 80-*/ 16/*'P'*/, 17/*'Q'*/, 18/*'R'*/, 19/*'S'*/, 20/*'T'*/, 21/*'U'*/, 22/*'V'*/, 23/*'W'*/, 24/*'X'*/, 25/*'Y'*/, 26/*'Z'*/,  0/*'['*/,  0/*'\'*/,  0/*']'*/,  0/*'^'*/,  0/*'_'*/,
/* 96-*/  0/*'`'*/,  1/*'a'*/,  2/*'b'*/,  3/*'c'*/,  4/*'d'*/,  5/*'e'*/,  6/*'f'*/,  7/*'g'*/,  8/*'h'*/,  9/*'i'*/, 10/*'j'*/, 11/*'k'*/, 12/*'l'*/, 13/*'m'*/, 14/*'n'*/, 15/*'o'*/,
/*112-*/ 16/*'p'*/, 17/*'q'*/, 18/*'r'*/, 19/*'s'*/, 20/*'t'*/, 21/*'u'*/, 22/*'v'*/, 23/*'w'*/, 24/*'x'*/, 25/*'y'*/, 26/*'z'*/,  0/*'{'*/,  0/*'|'*/,  0/*'}'*/,  0/*'~'*/,  0/* ? */,
	// clang-format on
};

/** Maps from SmalText.CEL frame number to character width. (Frame width: 13) */
const BYTE smallFontWidth[107] = {
	// clang-format off
/* 0-*/  8,  3,  6,  6,  6, 10, 10,  3,  3,  3,
/*10-*/  7,  6,  3,  5,  2,  7, 10,  5,  7,  7,
/*20-*/  8,  7,  7,  7,  7,  7,  2,  2,  4,  7,
/*30-*/  4,  6,  9, 10,  7,  9,  8,  7,  6,  8,
/*40-*/  8,  3,  3,  8,  6, 11,  9, 10,  7,  9,
/*50-*/  8,  6,  9, 11, 10, 13, 10,  9,  7,  4,
/*60-*/  6,  3,  6, 12,  3,  4,  3,  3,  7,  6,
/*70-*/  6,  8,  6,  7, 13,  9,  8, 12,  3,  3,
/*80-*/  6,  7,  6,  8,  7,  6, 10,  5,  6,  6,
/*90-*/  6,  7,  8,  4,  7,  4,  6,  7, 10, 10,
/*100*/ 12,  6,  9,  3,  5,  9,  8
	// clang-format on
};

/** Maps from MedTextS.CEL frame number to character width (Frame width: 22). */
const BYTE bigFontWidth[107] = {
	// clang-format off
/* 0-*/  5,  5, 10, 10,  9, 18, 15,  6,  6,  6,
/*10-*/  8, 10,  5,  7,  5, 12, 15,  5, 11, 11,
/*20-*/ 11, 10, 11, 11, 11, 11,  5,  5,  8, 12,
/*30-*/  8, 11, 16, 15, 10, 13, 14, 10,  9, 13,
/*40-*/ 11,  5,  5, 11, 10, 16, 13, 16, 10, 15,
/*50-*/ 11, 10, 14, 17, 17, 22, 17, 15, 11,  8,
/*60-*/ 12,  8,  8, 15,  7,  9,  3,  9,  9,  8,
/*70-*/  9, 13,  9, 12, 22, 17, 13, 20,  5,  3,
/*80-*/  9, 14,  9, 10, 15,  8, 15,  7,  9,  8,
/*90-*/  9, 14, 10,  5, 10,  6,  9, 15, 16, 17,
/*100*/ 16, 11, 17,  5,  8, 10, 11
	// clang-format on
};

/** Maps from BigTGold.CEL frame number to character width (Frame width: 46). */
static const BYTE hugeFontWidth[27] = {
	// clang-format off
/* 0-*/ 18, 33, 21, 26, 28, 19, 19, 26, 25, 11,
/*10-*/ 12, 25, 19, 33, 28, 32, 21, 32, 25, 20,
/*20-*/ 28, 34, 35, 46, 33, 33, 24, /*11, 23, 22,
	22, 21, 22, 21, 21, 21, 32, 10, 20, 36,
	31, 17, 13, 12, 13, 18, 16, 11, 20, 21,
	11, 10, 12, 11, 21, 23*/
	// clang-format on
};

/** Graphics for the small size font */
static CelImageBuf* pSmallTextCels;
/** Graphics for the medium size font */
CelImageBuf* pBigTextCels;
/** Graphics for the large size font */
static CelImageBuf* pHugeGoldTextCels;

static CelImageBuf* pSmallPentSpinCels;
static CelImageBuf* pHugePentSpinCels;

static BYTE pBoxBorderBmp[2 * BOXBORDER_WIDTH * BORDERBMP_HEIGHT + 2 * BOXBORDER_WIDTH * BORDERBMP_WIDTH];

/**
 * 'One-line' color translations for fonts. The shades of one color are used in one font.CEL
 * SmalText.CEL uses PAL16_GRAY values, while MedTextS and BigTGold.CEL are using PAL16_YELLOWs
 */
static BYTE fontColorTrns[3][16] = {
	// clang-format off
	// TRN for AFT_SILVER (MedTextS)
	{ PAL16_GRAY, PAL16_GRAY + 1, PAL16_GRAY + 2, PAL16_GRAY + 3, PAL16_GRAY + 4, PAL16_GRAY + 5, PAL16_GRAY + 6, PAL16_GRAY + 7, PAL16_GRAY + 8, PAL16_GRAY + 9, PAL16_GRAY + 10, PAL16_GRAY + 11, PAL16_GRAY + 12, PAL16_GRAY + 13, PAL16_GRAY + 14, PAL16_YELLOW + 15 },
	// TRN for COL_BLUE (SmalText)
	{ PAL16_BLUE + 2, PAL16_BLUE + 3, PAL16_BLUE + 4, PAL16_BLUE + 5, PAL16_BLUE + 6, PAL16_BLUE + 7, PAL16_BLUE + 8, PAL16_BLUE + 9, PAL16_BLUE + 10, PAL16_BLUE + 11, PAL16_BLUE + 12, PAL16_BLUE + 13, PAL16_BLUE + 14, PAL16_BLUE + 15, PAL16_ORANGE + 15, 0 },
	// TRN for COL_GOLD (SmalText)
	{ PAL16_YELLOW + 2, PAL16_YELLOW + 3, PAL16_YELLOW + 4, PAL16_YELLOW + 5, PAL16_YELLOW + 6, PAL16_YELLOW + 7, PAL16_YELLOW + 8, PAL16_YELLOW + 9, PAL16_YELLOW + 10, PAL16_YELLOW + 11, PAL16_YELLOW + 12, PAL16_YELLOW + 13, PAL16_YELLOW + 14, PAL16_YELLOW + 15, PAL16_ORANGE + 15, 0 },
	// TRN for COL_RED (SmalText)
	//{ PAL16_RED, PAL16_RED + 1, PAL16_RED + 2, PAL16_RED + 3, PAL16_RED + 4, PAL16_RED + 5, PAL16_RED + 6, PAL16_RED + 7, PAL16_RED + 8, PAL16_RED + 9, PAL16_RED + 10, PAL16_RED + 11, PAL16_RED + 12, PAL16_RED + 13, PAL16_RED + 14, PAL16_RED + 15 },
	// clang-format on
};
#define YLW_FONT_TRN_SILVER (&fontColorTrns[0][0] - PAL16_YELLOW)
#define YLW_FONT_TRN_BLUE   (&fontColorTrns[1][0] - PAL16_YELLOW)
#define GRY_FONT_TRN_BLUE   (&fontColorTrns[1][0] - PAL16_GRAY)
#define GRY_FONT_TRN_GOLD   (&fontColorTrns[2][0] - PAL16_GRAY)
//#define GRY_FONT_TRN_RED    (&fontColorTrns[3][0] - PAL16_GRAY)

void InitText()
{
	assert(pSmallTextCels == NULL);
	pSmallTextCels = CelLoadImage("CtrlPan\\SmalText.CEL", 13);
	assert(pBigTextCels == NULL);
	pBigTextCels = CelLoadImage("Data\\MedTextS.CEL", 22);
	assert(pHugeGoldTextCels == NULL);
	pHugeGoldTextCels = CelLoadImage("Data\\BigTGold.CEL", 46);
	assert(pHugePentSpinCels == NULL);
	pHugePentSpinCels = CelLoadImage("Data\\PentSpin.CEL", FOCUS_HUGE);
	assert(pSmallPentSpinCels == NULL);
	pSmallPentSpinCels = CelLoadImage("Data\\PentSpn2.CEL", FOCUS_MINI);

	CelImageBuf* progFillCel;

	progFillCel = CelLoadImage("Data\\TextBox.CEL", BORDERBMP_WIDTH); //  LTPANEL_WIDTH);
	CelDraw(SCREEN_X, SCREEN_Y + BORDERBMP_HEIGHT /*TPANEL_HEIGHT*/ - 1, progFillCel, 1);
	mem_free_dbg(progFillCel);
	// copy upper lines to the bitmap
	for (int i = 0; i < BOXBORDER_WIDTH; i++) {
		memcpy(&pBoxBorderBmp[0 + i * BORDERBMP_WIDTH], &gpBuffer[SCREENXY(0, i)], BORDERBMP_WIDTH);
	}
	// copy lower lines to the bitmap
	for (int i = BOXBORDER_WIDTH; i < 2 * BOXBORDER_WIDTH; i++) {
		memcpy(&pBoxBorderBmp[0 + i * BORDERBMP_WIDTH], &gpBuffer[SCREENXY(0, BORDERBMP_HEIGHT - BOXBORDER_WIDTH + (i - BOXBORDER_WIDTH))], BORDERBMP_WIDTH);
	}
	// copy left side to the bitmap
	for (int i = 0; i < BOXTEMPLATE_WIDTH; i++) {
		memcpy(&pBoxBorderBmp[i * BOXBORDER_WIDTH + 2 * BOXBORDER_WIDTH * BORDERBMP_WIDTH], &gpBuffer[SCREENXY(0, BOXBORDER_WIDTH + i)], BOXBORDER_WIDTH);
	}
	// copy right side to the bitmap
	for (int i = 0; i < BOXTEMPLATE_WIDTH; i++) {
		memcpy(&pBoxBorderBmp[i * BOXBORDER_WIDTH + BOXBORDER_WIDTH * BOXTEMPLATE_WIDTH + 2 * BOXBORDER_WIDTH * BORDERBMP_WIDTH],
			   &gpBuffer[SCREENXY(BORDERBMP_WIDTH - BOXBORDER_WIDTH, BOXBORDER_WIDTH + i)], BOXBORDER_WIDTH);
	}
}

void FreeText()
{
	MemFreeDbg(pSmallTextCels);
	MemFreeDbg(pBigTextCels);
	MemFreeDbg(pHugeGoldTextCels);
	MemFreeDbg(pHugePentSpinCels);
	MemFreeDbg(pSmallPentSpinCels);
}

/**
 * @brief Print letter to the back buffer
 * @param sx Backbuffer offset
 * @param sy Backbuffer offset
 * @param nCel frame number in pSmallTextCels
 * @param col text_color color value
 */
void PrintSmallColorChar(int sx, int sy, int nCel, BYTE col)
{
	BYTE* tbl;

	switch (col) {
	case COL_WHITE:
		CelDraw(sx, sy, pSmallTextCels, nCel);
		return;
	case COL_BLUE:
		tbl = GRY_FONT_TRN_BLUE;
		break;
	case COL_RED:
		tbl = ColorTrns[COLOR_TRN_CORAL]; // GRY_FONT_TRN_RED;
		break;
	case COL_GOLD:
		tbl = GRY_FONT_TRN_GOLD;
		break;
	default:
		tbl = ColorTrns[col - COL_GOLD];
		break;
	}
	CelDrawTrnTbl(sx, sy, pSmallTextCels, nCel, tbl);
}

static void PrintBigColorChar(int sx, int sy, int nCel, BYTE col)
{
	BYTE* tbl;

	switch (col) {
	case COL_WHITE:
		tbl = YLW_FONT_TRN_SILVER;
		break;
	case COL_BLUE:  // -- unused
		tbl = YLW_FONT_TRN_BLUE;
		break;
	case COL_RED:   // -- unused
		tbl = ColorTrns[COLOR_TRN_CORAL]; // YLW_FONT_TRN_RED;
		break;
	case COL_GOLD:
		CelDraw(sx, sy, pBigTextCels, nCel);
		return;
	default:
		tbl = ColorTrns[col - COL_GOLD];
		break;
	}
	CelDrawTrnTbl(sx, sy, pBigTextCels, nCel, tbl);
}

static void PrintHugeColorChar(int sx, int sy, int nCel, BYTE col)
{
	BYTE* tbl;

	switch (col) {
	case COL_WHITE: // -- unused
		tbl = YLW_FONT_TRN_SILVER;
		break;
	case COL_BLUE:  // -- unused
		tbl = YLW_FONT_TRN_BLUE;
		break;
	case COL_RED:   // -- unused
		tbl = ColorTrns[COLOR_TRN_CORAL]; // YLW_FONT_TRN_RED;
		break;
	case COL_GOLD:
		CelDraw(sx, sy, pHugeGoldTextCels, nCel);
		return;
	default:
		tbl = ColorTrns[col - COL_GOLD];
		break;
	}
	CelDrawTrnTbl(sx, sy, pHugeGoldTextCels, nCel, tbl);
}

int PrintBigChar(int sx, int sy, BYTE chr, BYTE col)
{
	BYTE nCel = gbStdFontFrame[chr];

	if (nCel != 0) {
		PrintBigColorChar(sx, sy, nCel, col);
		// draw optional diacritic
		if (chr >= 128) {
			BYTE dCel = gbStdDiacFrame[chr - 128];
			if (dCel != 0) {
				PrintBigColorChar(sx, sy, dCel, col);
			}
		}
	}

	return bigFontWidth[nCel] + FONT_KERN_BIG;
}

int PrintSmallChar(int sx, int sy, BYTE chr, BYTE col)
{
	BYTE nCel = gbStdFontFrame[chr];

	if (nCel != 0) {
		PrintSmallColorChar(sx, sy, nCel, col);
		// draw optional diacritic
		if (chr >= 128) {
			BYTE dCel = gbStdDiacFrame[chr - 128];
			if (dCel != 0) {
				PrintSmallColorChar(sx, sy, dCel, col);
			}
		}
	}

	return smallFontWidth[nCel] + FONT_KERN_SMALL;
}

int PrintHugeChar(int sx, int sy, BYTE chr, BYTE col)
{
	BYTE nCel = gbHugeFontFrame[chr];

	if (nCel != 0) {
		PrintHugeColorChar(sx, sy, nCel, col);
	}

	return hugeFontWidth[nCel] + FONT_KERN_HUGE;
}

int GetHugeStringWidth(const char* text)
{
	int i;
	BYTE c;

	i = 0;
	while (*text != '\0') {
		c = gbHugeFontFrame[(BYTE)*text++];
		i += hugeFontWidth[c] + FONT_KERN_HUGE;
	}
	return i - FONT_KERN_HUGE;
}

int GetBigStringWidth(const char* text)
{
	int i;
	BYTE c;

	i = 0;
	while (*text != '\0') {
		c = gbStdFontFrame[(BYTE)*text++];
		i += bigFontWidth[c] + FONT_KERN_BIG;
	}
	return i - FONT_KERN_BIG;
}

int GetSmallStringWidth(const char* text)
{
	int i;
	BYTE c;

	i = 0;
	while (*text != '\0') {
		c = gbStdFontFrame[(BYTE)*text++];
		i += smallFontWidth[c] + FONT_KERN_SMALL;
	}

	return i - FONT_KERN_SMALL;
}

void PrintGameStr(int x, int y, const char* text, BYTE col)
{
	// TODO: preselect color-trn if performance is required
	while (*text != '\0') {
		x += PrintSmallChar(x, y, (BYTE)*text++, col);
	}
}

/**
 * @brief Render text string justified to the back buffer
 * @param x Screen coordinate
 * @param y Screen coordinate
 * @param endX End of line in screen coordinate
 * @param text String to print, in Windows-1252 encoding
 * @param col text_color color value
 * @param kern Letter spacing
 */
void PrintJustifiedString(int x, int y, int endX, const char* text, BYTE col, int kern)
{
	BYTE c;
	const char* tmp;
	int strEnd;
restart:
	strEnd = x;
	tmp = text;
	while (*tmp != '\0') {
		c = gbStdFontFrame[(BYTE)*tmp++];
		strEnd += smallFontWidth[c] + kern;
	}
	if (strEnd <= endX) {
		x += (endX - strEnd) >> 1;
	} else if (kern > 0) {
		--kern;
		goto restart;
	}

	PrintLimitedString(x, y, text, endX - x, col, kern);
}

int PrintLimitedString(int x, int y, const char* text, int limit, BYTE col, int kern)
{
	BYTE c;
	// TODO: preselect color-trn if performance is required
	while (*text != '\0') {
		c = gbStdFontFrame[(BYTE)*text++];
		limit -= smallFontWidth[c] + kern;
		if (limit >= 0 && c != 0) {
			PrintSmallColorChar(x, y, c, col);
		}
		x += smallFontWidth[c] + kern;
	}
	return x;
}

void PrintHugeString(int x, int y, const char* text, BYTE col)
{
	// TODO: preselect color-trn if performance is required
	while (*text != '\0') {
		x += PrintHugeChar(x, y, (BYTE)*text++, col);
	}
}

static int AlignXOffset(int flags, int rw, int sw)
{
	if (flags & AFF_HCENTER)
		return (rw - sw) >> 1;
	if (flags & AFF_RIGHT)
		return rw - sw;
	return 0;
}

void PrintString(int flags, const char* text, int x, int y, int w, int h)
{
	unsigned size = (flags >> AFF_SIZE_SHL) & AFF_SIZES;
	unsigned color;
	int sw, lh, dy, sx, sy, cx, cy;
	int (*pChar)(int sx, int sy, BYTE text, BYTE col);

	switch (size) {
	case AFT_SMALL:
		sw = GetSmallStringWidth(text);
		dy = 1;
		lh = SMALL_FONT_HEIGHT - dy;
		//lh = SMALL_FONT_HEIGHT;
		pChar = PrintSmallChar;
		break;
	case AFT_BIG:
		sw = GetBigStringWidth(text);
		dy = 5 - 2;
		lh = BIG_FONT_HEIGHT - dy;
		//lh = BIG_FONT_HEIGHT;
		pChar = PrintBigChar;
		break;
	case AFT_HUGE:
		sw = GetHugeStringWidth(text);
		dy = 10 - 4;
		lh = HUGE_FONT_HEIGHT - dy;
		//lh = HUGE_FONT_HEIGHT;
		pChar = PrintHugeChar;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	sx = x + AlignXOffset(flags, w, sw);
	sy = y + ((flags & AFF_VCENTER) ? ((h - lh) >> 1) : 0) + lh;

	sy += dy;
	lh += dy;
	color = (flags >> AFF_COLOR_SHL) & AFF_COLORS;

	cx = sx;
	cy = sy;
	for ( ; *text != '\0'; text++) {
		if (*text == '\n') {
			cx = sx;
			cy += lh;
			continue;
		}
		cx += pChar(cx, cy, (BYTE)*text, color);
	}
}

static int PentSpn2Spin()
{
	return (SDL_GetTicks() / 64) % 8 + 1;
}

void DrawHugePentSpn(int x1, int x2, int y)
{
	CelDraw(x1, y, pHugePentSpinCels, PentSpn2Spin());
	CelDraw(x2, y, pHugePentSpinCels, PentSpn2Spin());
}

void DrawSmallPentSpn(int x1, int x2, int y)
{
	CelDraw(x1, y, pSmallPentSpinCels, PentSpn2Spin());
	CelDraw(x2, y, pSmallPentSpinCels, PentSpn2Spin());
}

void DrawSingleSmallPentSpn(int x, int y)
{
	CelDraw(x, y, pSmallPentSpinCels, PentSpn2Spin());
}

void DrawColorTextBoxSLine(int x, int y, int w, int dy)
{
	int sxy, dxy, width, length;

	width = BUFFER_WIDTH;
	sxy = x + BOXBORDER_WIDTH + width * (y + 1);
	dxy = x + BOXBORDER_WIDTH + width * (y + dy);
	length = w - 2 * BOXBORDER_WIDTH;

	/// ASSERT: assert(gpBuffer != NULL);

	int i;
	BYTE *src, *dst;

	src = &gpBuffer[sxy];
	dst = &gpBuffer[dxy];

	for (i = 0; i < BOXBORDER_WIDTH; i++, src += width, dst += width)
		memcpy(dst, src, length);
}

void DrawColorTextBox(int x, int y, int w, int h, BYTE col)
{
	int v, sx, sy, n;
	BYTE* tbl;

	// prevent OOB
	static_assert(BORDER_TOP >= BOXBORDER_WIDTH, "DrawColorTextBox needs larger top-border");
	static_assert(BORDER_LEFT >= BOXBORDER_WIDTH, "DrawColorTextBox needs larger side-border.");
	if (h > BORDER_TOP + SCREEN_HEIGHT + BOXBORDER_WIDTH - y)
		h = BORDER_TOP + SCREEN_HEIGHT + BOXBORDER_WIDTH - y;
	if (w > BORDER_LEFT + SCREEN_WIDTH + BOXBORDER_WIDTH - x)
		w = BORDER_LEFT + SCREEN_WIDTH + BOXBORDER_WIDTH - x;

	// draw the background
	DrawRectTrans(x + BOXBORDER_WIDTH, y + BOXBORDER_WIDTH, w - 2 * BOXBORDER_WIDTH, h - 2 * BOXBORDER_WIDTH, PAL_BLACK);

	switch (col) {
	case COL_WHITE:
		tbl = YLW_FONT_TRN_SILVER;
		break;
	case COL_BLUE:  // -- unused
		tbl = YLW_FONT_TRN_BLUE;
		break;
	case COL_RED:   // -- unused
		tbl = ColorTrns[COLOR_TRN_CORAL]; // YLW_FONT_TRN_RED;
		break;
	case COL_GOLD:
#if 0
		// draw the top left corner
		for (int j = 0; j < BOXBORDER_WIDTH; j++) {
			memcpy(&gpBuffer[BUFFERXY(x, y + j)], &pBoxBorderBmp[(0 + j) * BORDERBMP_WIDTH], BOXBORDER_WIDTH);
		}
		// draw the bottom left corner
		for (int j = 0; j < BOXBORDER_WIDTH; j++) {
			memcpy(&gpBuffer[BUFFERXY(x, y + h - BOXBORDER_WIDTH + j)], &pBoxBorderBmp[(BOXBORDER_WIDTH + j) * BORDERBMP_WIDTH], BOXBORDER_WIDTH);
		}
		// draw the top right corner
		for (int j = 0; j < BOXBORDER_WIDTH; j++) {
			memcpy(&gpBuffer[BUFFERXY(x + w - BOXBORDER_WIDTH, y + j)], &pBoxBorderBmp[(0 + j) * BORDERBMP_WIDTH + BORDERBMP_WIDTH - BOXBORDER_WIDTH], BOXBORDER_WIDTH);
		}
		// draw the bottom right corner
		for (int j = 0; j < BOXBORDER_WIDTH; j++) {
			memcpy(&gpBuffer[BUFFERXY(x + w - BOXBORDER_WIDTH, y + h - BOXBORDER_WIDTH)], &pBoxBorderBmp[BOXBORDER_WIDTH * BORDERBMP_WIDTH + BORDERBMP_WIDTH - BOXBORDER_WIDTH], BOXBORDER_WIDTH);
		}
		// draw the top and bottom lines
		v = w - 2 * BOXBORDER_WIDTH;
		sx = x + BOXBORDER_WIDTH;
		n = BOXTEMPLATE_WIDTH;
		while (n > 0) {
			while (v >= n) {
				v -= n;
				// extend the top line
				for (int j = 0; j < BOXBORDER_WIDTH; j++) {
					memcpy(&gpBuffer[BUFFERXY(sx, y + j)], &pBoxBorderBmp[(0 + j) * BORDERBMP_WIDTH + BOXBORDER_WIDTH], n);
				}
				// extend the bottom line
				for (int j = 0; j < BOXBORDER_WIDTH; j++) {
					memcpy(&gpBuffer[BUFFERXY(sx, y + h - BOXBORDER_WIDTH + j)], &pBoxBorderBmp[(BOXBORDER_WIDTH + j) * BORDERBMP_WIDTH + BOXBORDER_WIDTH], n);
				}
				sx += n;
			}
			n >>= 1;
		}
		// draw the left and right lines
		v = h - 2 * BOXBORDER_WIDTH;
		n = BOXTEMPLATE_WIDTH;
		sy = y + BOXBORDER_WIDTH;
		while (n > 0) {
			while (v >= n) {
				v -= n;
				for (int m = 0; m < n; m++) {
					// extend the left line
					memcpy(&gpBuffer[BUFFERXY(x, sy)], &pBoxBorderBmp[m * BOXBORDER_WIDTH + 2 * BOXBORDER_WIDTH * BORDERBMP_WIDTH], BOXBORDER_WIDTH);
					// extend the right line
					memcpy(&gpBuffer[BUFFERXY(x + w - BOXBORDER_WIDTH, sy)], &pBoxBorderBmp[m * BOXBORDER_WIDTH + BOXBORDER_WIDTH * BOXTEMPLATE_WIDTH + 2 * BOXBORDER_WIDTH * BORDERBMP_WIDTH], BOXBORDER_WIDTH);
					sy++;
				}
			}
			n >>= 1;
		}
		return;
#endif
	default:
		tbl = ColorTrns[col - COL_GOLD];
		break;
	}

	// draw the top left corner
	for (int j = 0; j < BOXBORDER_WIDTH; j++) {
		for (int i = 0; i < BOXBORDER_WIDTH; i++) {
			gpBuffer[BUFFERXY(x + i, y + j)] = tbl[pBoxBorderBmp[i + (0 + j) * BORDERBMP_WIDTH]];
		}
	}
	// draw the bottom left corner
	for (int j = 0; j < BOXBORDER_WIDTH; j++) {
		for (int i = 0; i < BOXBORDER_WIDTH; i++) {
			gpBuffer[BUFFERXY(x + i, y + h - BOXBORDER_WIDTH + j)] = tbl[pBoxBorderBmp[i + (BOXBORDER_WIDTH + j) * BORDERBMP_WIDTH]];
		}
	}
	// draw the top right corner
	for (int j = 0; j < BOXBORDER_WIDTH; j++) {
		for (int i = 0; i < BOXBORDER_WIDTH; i++) {
			gpBuffer[BUFFERXY(x + i + w - BOXBORDER_WIDTH, y + j)] = tbl[pBoxBorderBmp[i + (0 + j) * BORDERBMP_WIDTH + BORDERBMP_WIDTH - BOXBORDER_WIDTH]];
		}
	}
	// draw the bottom right corner
	for (int j = 0; j < BOXBORDER_WIDTH; j++) {
		for (int i = 0; i < BOXBORDER_WIDTH; i++) {
			gpBuffer[BUFFERXY(x + i + w - BOXBORDER_WIDTH, y + h - BOXBORDER_WIDTH + j)] = tbl[pBoxBorderBmp[i + (BOXBORDER_WIDTH + j) * BORDERBMP_WIDTH + BORDERBMP_WIDTH - BOXBORDER_WIDTH]];
		}
	}
	// draw the top and bottom lines
	v = w - 2 * BOXBORDER_WIDTH;
	sx = x + BOXBORDER_WIDTH;
	n = BOXTEMPLATE_WIDTH;
	while (n > 0) {
		while (v >= n) {
			v -= n;
			// extend the top line
			for (int j = 0; j < BOXBORDER_WIDTH; j++) {
				for (int i = 0; i < n; i++) {
					gpBuffer[BUFFERXY(sx + i, y + j)] = tbl[pBoxBorderBmp[i + (0 + j) * BORDERBMP_WIDTH + BOXBORDER_WIDTH]];
				}
			}
			// extend the bottom line
			for (int j = 0; j < BOXBORDER_WIDTH; j++) {
				for (int i = 0; i < n; i++) {
					gpBuffer[BUFFERXY(sx + i, y + h - BOXBORDER_WIDTH + j)] = tbl[pBoxBorderBmp[i + (BOXBORDER_WIDTH + j) * BORDERBMP_WIDTH + BOXBORDER_WIDTH]];
				}
			}
			sx += n;
		}
		n >>= 1;
	}
	// draw the left and right lines
	v = h - 2 * BOXBORDER_WIDTH;
	n = BOXTEMPLATE_WIDTH;
	sy = y + BOXBORDER_WIDTH;
	while (n > 0) {
		while (v >= n) {
			v -= n;
			for (int m = 0; m < n; m++) {
				// extend the left line
				for (int i = 0; i < BOXBORDER_WIDTH; i++) {
					gpBuffer[BUFFERXY(x + i, sy)] = tbl[pBoxBorderBmp[i + m * BOXBORDER_WIDTH + 2 * BOXBORDER_WIDTH * BORDERBMP_WIDTH]];
				}
				// extend the right line
				for (int i = 0; i < BOXBORDER_WIDTH; i++) {
					gpBuffer[BUFFERXY(x + i + w - BOXBORDER_WIDTH, sy)] = tbl[pBoxBorderBmp[i + m * BOXBORDER_WIDTH + BOXBORDER_WIDTH * BOXTEMPLATE_WIDTH + 2 * BOXBORDER_WIDTH * BORDERBMP_WIDTH]];
				}
				sy++;
			}
		}
		n >>= 1;
	}
}

DEVILUTION_END_NAMESPACE
