/**
 * @file minitext.cpp
 *
 * Implementation of scrolling dialog text.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Current y position of text in px */
int qtexty;
/** Pointer to the current text being displayed */
const char *qtextptr;
/** Time of last rendering of the text */
DWORD qtextTime;
/** Specify if the quest dialog window is being shown */
bool gbQtextflag;
/** Vertical speed of the scrolling text in ms/px */
DWORD scrolltexty;
/** Graphics for the medium size font */
BYTE *pMedTextCels;
/** Graphics for the window border */
BYTE *pTextBoxCels;

/** Maps from font index to medtexts.cel frame number. */
const BYTE mfontframe[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 37, 49, 38, 0, 39, 40, 47,
	42, 43, 41, 45, 52, 44, 53, 55, 36, 27,
	28, 29, 30, 31, 32, 33, 34, 35, 51, 50,
	48, 46, 49, 54, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 42, 0, 43, 0, 0, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 48, 0, 49, 0, 0
};
/**
 * Maps from medtexts.cel frame number to character width. Note, the
 * character width may be distinct from the frame width, which is 22 for every
 * medtexts.cel frame.
 */
const BYTE mfontkern[56] = {
	5, 15, 10, 13, 14, 10, 9, 13, 11, 5,
	5, 11, 10, 16, 13, 16, 10, 15, 12, 10,
	14, 17, 17, 22, 17, 16, 11, 5, 11, 11,
	11, 10, 11, 11, 11, 11, 15, 5, 10, 18,
	15, 8, 6, 6, 7, 10, 9, 6, 10, 10,
	5, 5, 5, 5, 11, 12
};

/* data */

void FreeQuestText()
{
	MemFreeDbg(pMedTextCels);
	MemFreeDbg(pTextBoxCels);
}

void InitQuestText()
{
	pMedTextCels = LoadFileInMem("Data\\MedTextS.CEL", NULL);
	pTextBoxCels = LoadFileInMem("Data\\TextBox.CEL", NULL);
	gbQtextflag = false;
}

void InitQTextMsg(int m, bool showText)
{
	const TextDataStruct *tds;
	int speed, sfxnr;

	tds = &alltext[m];
	if (tds->scrlltxt && showText) {
		ClearPanels();
		gbQtextflag = true;
		qtextptr = tds->txtstr;
		qtexty = 340 + SCREEN_Y + UI_OFFSET_Y;
		speed = tds->txtspd;
		if (speed <= 0)
			scrolltexty = 50 / -(speed - 1);
		else
			scrolltexty = ((speed + 1) * 50) / speed;
		qtextTime = SDL_GetTicks();
	}
	sfxnr = tds->sfxnr;
	if (tds->txtsfxset) {
		sfxnr = sgSFXSets[sfxnr][plr[myplr]._pClass];
	}
	PlaySFX(sfxnr);
}

/**
 * @brief Draw the quest dialog window decoration and background
 */
void DrawQTextBack()
{
	CelDraw(PANEL_X + 24, SCREEN_Y + 327 + UI_OFFSET_Y, pTextBoxCels, 1, 591);
	trans_rect(PANEL_X + 27, SCREEN_Y + UI_OFFSET_Y + 28, 585, 297);
}

/**
 * @brief Print a character
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 */
static void PrintQTextChr(int sx, int sy, BYTE *pCelBuff, int nCel)
{
	BYTE *pStart, *pEnd;

	/// ASSERT: assert(gpBuffer != NULL);
	// TODO: create a function in engine to draw with a given height? (similar to DrawFlask2 in control.cpp)
	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (49 + SCREEN_Y + UI_OFFSET_Y)];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (309 + SCREEN_Y + UI_OFFSET_Y)];
	CelDraw(sx, sy, pCelBuff, nCel, 22);

	gpBufStart = pStart;
	gpBufEnd = pEnd;
}

void DrawQText()
{
	int i, l, w, tx, ty;
	BYTE c;
	const char *p, *pnl, *s;
	char tempstr[128];
	bool doneflag;
	DWORD currTime;

	DrawQTextBack();

	p = qtextptr;
	pnl = NULL;
	tx = 48 + PANEL_X;
	ty = qtexty;

	doneflag = false;
	while (!doneflag) {
		w = 0;
		s = p;
		l = 0;
		while (*s != '\n' && *s != '|' && w < 543) {
			c = gbFontTransTbl[(BYTE)*s];
			s++;
			if (c != '\0') {
				tempstr[l] = c;
				w += mfontkern[mfontframe[c]] + 2;
				l++;
			}
		}
		tempstr[l] = '\0';
		if (*s == '|') {
			doneflag = true;
		} else if (*s != '\n') {
			while (tempstr[l] != ' ' && l > 0) {
				tempstr[l] = '\0';
				l--;
			}
		}
		for (i = 0; tempstr[i] != '\0'; i++) {
			p++;
			c = mfontframe[gbFontTransTbl[(BYTE)tempstr[i]]];
			if (*p == '\n') {
				p++;
			}
			if (c != 0) {
				PrintQTextChr(tx, ty, pMedTextCels, c);
			}
			tx += mfontkern[c] + 2;
		}
		if (pnl == NULL) {
			pnl = p;
		}
		tx = 48 + PANEL_X;
		ty += 38;
		if (ty > 341 + SCREEN_Y + UI_OFFSET_Y) {
			doneflag = true;
		}
	}

	for (currTime = SDL_GetTicks(); qtextTime + scrolltexty < currTime; qtextTime += scrolltexty) {
		qtexty--;
		if (qtexty <= 49 + SCREEN_Y + UI_OFFSET_Y) {
			qtexty += 38;
			qtextptr = pnl;
			if (*pnl == '|') {
				gbQtextflag = false;
			}
			break;
		}
	}
}

DEVILUTION_END_NAMESPACE
