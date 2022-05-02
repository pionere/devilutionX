/**
 * @file text_render.cpp
 *
 * Text rendering.
 */
#include "text_render.hpp"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * Maps ASCII character code to font index, as used by the
 * small, medium and large sized fonts; which corresponds to smaltext.cel,
 * medtexts.cel and bigtgold.cel respectively.
 */
const BYTE gbFontTransTbl[256] = {
	// clang-format off
	'\0', 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	' ',  '!',  '\"', '#',  '$',  '%',  '&',  '\'', '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	'@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  '\\', ']',  '^',  '_',
	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x01,
	'C',  'u',  'e',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'i',  'i',  'i',  'A',  'A',
	'E',  'a',  'A',  'o',  'o',  'o',  'u',  'u',  'y',  'O',  'U',  'c',  'L',  'Y',  'P',  'f',
	'a',  'i',  'o',  'u',  'n',  'N',  'a',  'o',  '?',  0x01, 0x01, 0x01, 0x01, '!',  '<',  '>',
	'o',  '+',  '2',  '3',  '\'', 'u',  'P',  '.',  ',',  '1',  '0',  '>',  0x01, 0x01, 0x01, '?',
	'A',  'A',  'A',  'A',  'A',  'A',  'A',  'C',  'E',  'E',  'E',  'E',  'I',  'I',  'I',  'I',
	'D',  'N',  'O',  'O',  'O',  'O',  'O',  'X',  '0',  'U',  'U',  'U',  'U',  'Y',  'b',  'B',
	'a',  'a',  'a',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'e',  'i',  'i',  'i',  'i',
	'o',  'n',  'o',  'o',  'o',  'o',  'o',  '/',  '0',  'u',  'u',  'u',  'u',  'y',  'b',  'y',
	// clang-format on
};

/** Maps from font index to smaltext.cel frame number. */
const BYTE smallFontFrame[128] = {
	// clang-format off
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0, 54, 44, 57, 58, 56, 55, 47, 40, 41, 59, 39, 50, 37, 51, 52,
	36, 27, 28, 29, 30, 31, 32, 33, 34, 35, 48, 49, 60, 38, 61, 53,
	62,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 42, 63, 43, 64, 65,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 40, 66, 41, 67,  0
	// clang-format on
};

/** Maps from font index to medtexts.cel and bigtgold.cel frame number. */
const BYTE bhFontFrame[128] = {
	// clang-format off
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0, 37, 49, 38,  0, 39, 40, 47, 42, 43, 41, 45, 52, 44, 53, 55,
	36, 27, 28, 29, 30, 31, 32, 33, 34, 35, 51, 50,  0, 46,  0, 54,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 42,  0, 43,  0,  0,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,  0,  0,  0,  0,  0
	// clang-format on
};

/** Maps from smaltext.cel frame number to character width. (Frame width: 13) */
const BYTE smallFontWidth[68] = {
	// clang-format off
	 8, 10,  7,  9,  8,  7,  6,  8,  8,  3,
	 3,  8,  6, 11,  9, 10,  6,  9,  9,  6,
	 9, 11, 10, 13, 10, 11,  7,  5,  7,  7,
	 8,  7,  7,  7,  7,  7, 10,  5,  5,  6,
	 3,  3,  4,  3,  6,  6,  3,  3,  3,  3,
	 3,  2,  7,  6,  3, 10, 10,  6,  6,  7,
	 4,  4,  9,  6,  6, 12,  3,  7
	// clang-format on
};

/** Maps from medtexts.cel frame number to character width (Frame width: 22). */
const BYTE bigFontWidth[56] = {
	// clang-format off
	 5, 15, 10, 13, 14, 10,  9, 13, 11,  5,
	 5, 11, 10, 16, 13, 16, 10, 15, 12, 10,
	14, 17, 17, 22, 17, 16, 11,  5, 11, 11,
	11, 10, 11, 11, 11, 11, 15,  5, 10, 18,
	15,  8,  6,  6,  7, 10,  9,  6, 10, 10,
	 5,  5,  5,  5, 11, 12
	// clang-format on
};

/** Maps from bigtgold.cel frame number to character width (Frame width: 46). */
static const BYTE hugeFontWidth[56] = {
	// clang-format off
	18, 33, 21, 26, 28, 19, 19, 26, 25, 11,
	12, 25, 19, 34, 28, 32, 20, 32, 28, 20,
	28, 36, 35, 46, 33, 33, 24, 11, 23, 22,
	22, 21, 22, 21, 21, 21, 32, 10, 20, 36,
	31, 17, 13, 12, 13, 18, 16, 11, 20, 21,
	11, 10, 12, 11, 21, 23
	// clang-format on
};

//int LineHeights[3] = { 17, 43, 50 };

/** Small-Text images CEL */
static CelImageBuf* pSmallTextCels;
/** Graphics for the medium size font */
CelImageBuf* pBigTextCels;

static CelImageBuf* pHugeGoldTextCels;

static CelImageBuf* pSmallPentSpinCels;
static CelImageBuf* pHugePentSpinCels;

static BYTE fontColorTableGold[256];
static BYTE fontColorTableBlue[256];
static BYTE fontColorTableRed[256];

void InitText()
{
	int i;
	BYTE pix;

	assert(pSmallTextCels == NULL);
	pSmallTextCels = CelLoadImage("CtrlPan\\SmalText.CEL", 13);
	assert(pBigTextCels == NULL);
	pBigTextCels = CelLoadImage("Data\\MedTextS.CEL", 22);
	assert(pHugeGoldTextCels == NULL);
	pHugeGoldTextCels = CelLoadImage("Data\\BigTGold.CEL", 46);
	assert(pHugePentSpinCels == NULL);
	pHugePentSpinCels = CelLoadImage("Data\\PentSpin.CEL", 48);
	assert(pSmallPentSpinCels == NULL);
	pSmallPentSpinCels = CelLoadImage("Data\\PentSpn2.CEL", 12);

	for (i = 0; i < lengthof(fontColorTableBlue); i++) {
		pix = i;
		if (pix >= PAL16_GRAY) {
			if (pix >= PAL16_GRAY + 14)
				pix = PAL16_BLUE + 15;
			else 
				pix -= PAL16_GRAY - (PAL16_BLUE + 2);
		}
		fontColorTableBlue[i] = pix;
	}

	for (i = 0; i < lengthof(fontColorTableRed); i++) {
		pix = i;
		if (pix >= PAL16_GRAY)
			pix -= PAL16_GRAY - PAL16_RED;
		fontColorTableRed[i] = pix;
	}

	for (i = 0; i < lengthof(fontColorTableGold); i++) {
		pix = i;
		if (pix >= PAL16_GRAY) {
			if (pix >= PAL16_GRAY + 14)
				pix = PAL16_YELLOW + 15;
			else
				pix -= PAL16_GRAY - (PAL16_YELLOW + 2);
		}
		fontColorTableGold[i] = pix;
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
 * @param nCel Number of letter in Windows-1252
 * @param col text_color color value
 */
void PrintChar(int sx, int sy, int nCel, BYTE col)
{
	BYTE *tbl;

	switch (col) {
	case COL_WHITE:
		CelDraw(sx, sy, pSmallTextCels, nCel);
		return;
	case COL_BLUE:
		tbl = fontColorTableBlue;
		break;
	case COL_RED:
		tbl = fontColorTableRed;
		break;
	case COL_GOLD:
		tbl = fontColorTableGold;
		break;
	/*case COL_BLACK:
		tbl = ColorTrns[MAXDARKNESS];
		break;*/
	default:
		ASSUME_UNREACHABLE
		break;
	}
	CelDrawLight(sx, sy, pSmallTextCels, nCel, tbl);
}

int PrintBigChar(int sx, int sy, BYTE text, BYTE col)
{
	BYTE nCel = bhFontFrame[gbFontTransTbl[text]];
	BYTE* tbl;

	if (nCel != 0) {
		switch (col) {
		case COL_WHITE:
			CelDraw(sx, sy, pBigTextCels, nCel);
			return bigFontWidth[nCel] + FONT_KERN_BIG;
		case COL_BLUE:
			tbl = fontColorTableBlue;
			break;
		case COL_RED:
			tbl = fontColorTableRed;
			break;
		case COL_GOLD:
			tbl = fontColorTableGold;
			break;
		//case COL_BLACK:
		//	tbl = ColorTrns[MAXDARKNESS];
		//	break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		CelDrawLight(sx, sy, pBigTextCels, nCel, tbl);
	}

	return bigFontWidth[nCel] + FONT_KERN_BIG;
}

int PrintSmallChar(int sx, int sy, BYTE text, BYTE col)
{
	BYTE nCel = smallFontFrame[gbFontTransTbl[text]];
	BYTE* tbl;

	if (nCel != 0) {
		switch (col) {
		case COL_WHITE:
			CelDraw(sx, sy, pSmallTextCels, nCel);
			return smallFontWidth[nCel] + FONT_KERN_SMALL;
		case COL_BLUE:
			tbl = fontColorTableBlue;
			break;
		case COL_RED:
			tbl = fontColorTableRed;
			break;
		case COL_GOLD:
			tbl = fontColorTableGold;
			break;
		//case COL_BLACK:
		//	tbl = ColorTrns[MAXDARKNESS];
		//	break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		CelDrawLight(sx, sy, pSmallTextCels, nCel, tbl);
	}

	return smallFontWidth[nCel] + FONT_KERN_SMALL;
}

int PrintHugeChar(int sx, int sy, BYTE text, BYTE col)
{
	BYTE nCel = bhFontFrame[gbFontTransTbl[text]];
	//BYTE* tbl;

	if (nCel != 0) {
		/*switch (col) {
		case COL_WHITE:
			CelDraw(sx, sy, pHugeGoldTextCels, nCel);
			return hugeFontWidth[nCel] + FONT_KERN_HUGE;
		case COL_BLUE:
			tbl = fontColorTableBlue;
			break;
		case COL_RED:
			tbl = fontColorTableRed;
			break;
		case COL_GOLD:
			tbl = fontColorTableGold;
			break;
		//case COL_BLACK:
		//	tbl = ColorTrns[MAXDARKNESS];
		//	break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		CelDrawLight(sx, sy, pHugeGoldTextCels, nCel, tbl);*/
		CelDraw(sx, sy, pHugeGoldTextCels, nCel);
	}

	return hugeFontWidth[nCel] + FONT_KERN_HUGE;
}

int GetHugeStringWidth(const char* text)
{
	int i;
	BYTE c;

	i = 0;
	while (*text != '\0') {
		c = bhFontFrame[gbFontTransTbl[(BYTE)*text++]];
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
		c = bhFontFrame[gbFontTransTbl[(BYTE)*text++]];
		i += bigFontWidth[c] + FONT_KERN_BIG;
	}
	return i - FONT_KERN_BIG;
}

int GetMediumStringWidth(const char* text)
{
	int i;
	BYTE c;

	i = 0;
	while (*text != '\0') {
		c = bhFontFrame[gbFontTransTbl[(BYTE)*text++]];
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
		c = smallFontFrame[gbFontTransTbl[(BYTE)*text++]];
		i += smallFontWidth[c] + FONT_KERN_SMALL;
	}

	return i - FONT_KERN_SMALL;
}

void PrintGameStr(int x, int y, const char *text, BYTE color)
{
	while (*text != '\0') {
		x += PrintSmallChar(x, y, (BYTE)*text++, color);
	}
}

/**
 * @brief Render text string to back buffer
 * @param x Screen coordinate
 * @param y Screen coordinate
 * @param endX End of line in screen coordinate
 * @param pszStr String to print, in Windows-1252 encoding
 * @param center 
 * @param col text_color color value
 * @param kern Letter spacing
 */
void PrintString(int x, int y, int endX, const char *text, bool cjustflag, BYTE col, int kern)
{
	BYTE c;
	const char *tmp;
	int strEnd;
	int k;

	if (cjustflag) {
		strEnd = x;
		tmp = text;
		while (*tmp != '\0') {
			c = smallFontFrame[gbFontTransTbl[(BYTE)*tmp++]];
			strEnd += smallFontWidth[c] + kern;
		}
		if (strEnd < endX) {
			x += (endX - strEnd) >> 1;
		}
	}
	while (*text != '\0') {
		c = smallFontFrame[gbFontTransTbl[(BYTE)*text++]];
		k = smallFontWidth[c] + kern;
		if (x + k < endX && c != 0) {
			PrintChar(x, y, c, col);
		}
		x += k;
	}
}

int PrintLimitedString(int x, int y, const char *text, int limit, BYTE col)
{
	BYTE c;

	while (*text != '\0') {
		c = smallFontFrame[gbFontTransTbl[(BYTE)*text++]];
		limit -= smallFontWidth[c] + FONT_KERN_SMALL;
		if (limit >= 0 && c != 0) {
			PrintChar(x, y, c, col);
		}
		x += smallFontWidth[c] + FONT_KERN_SMALL;
	}
	return x;
}

void PrintHugeString(int x, int y, const char* text, int light)
{
	BYTE c, *tbl;

	// TODO: uncomment if performance is required
	//tbl = light == 0 ? NULL : ColorTrns[light];
	tbl = ColorTrns[light];
	while (*text != '\0') {
		c = bhFontFrame[gbFontTransTbl[(BYTE)*text++]];
		if (c != 0) {
			/*if (tbl == NULL)
				CelDraw(x, y, pHugeGoldTextCels, c);
			else*/
				CelDrawLight(x, y, pHugeGoldTextCels, c, tbl);
		}
		x += hugeFontWidth[c] + FONT_KERN_HUGE;
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

DEVILUTION_END_NAMESPACE
