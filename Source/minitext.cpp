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
/** Graphics for the window border */
BYTE *pTextBoxCels;

void FreeQuestText()
{
	MemFreeDbg(pTextBoxCels);
}

void InitQuestText()
{
	pTextBoxCels = LoadFileInMem("Data\\TextBox.CEL");
	gbQtextflag = false;
}

void InitQTextMsg(int m, bool showText)
{
	const TextDataStruct *tds;
	int sfxnr;

	tds = &alltext[m];
	if (tds->scrlltxt && showText) {
		ClearPanels();
		gbQtextflag = true;
		qtextptr = tds->txtstr;
		qtexty = 340 + SCREEN_Y + UI_OFFSET_Y;
		scrolltexty = tds->txtspd;
		qtextTime = SDL_GetTicks();
	}
	sfxnr = tds->sfxnr;
	if (tds->txtsfxset) {
		sfxnr = sgSFXSets[sfxnr][players[myplr]._pClass];
	}
	PlaySFX(sfxnr);
}

/**
 * @brief Draw the quest dialog window decoration and background
 */
void DrawQTextBack()
{
	CelDraw(QPANEL_X, SCREEN_Y + 327 + UI_OFFSET_Y, pTextBoxCels, 1, QPANEL_WIDTH);
	trans_rect(QPANEL_X + 3, SCREEN_Y + UI_OFFSET_Y + 28, QPANEL_WIDTH - 2 * 3, 297);
}

void DrawQText()
{
	int len, tx, ty;
	BYTE c;
	const char *pnl;
	const char *str, *sstr, *endstr;
	DWORD currTime;
	BYTE *pStart, *pEnd;

	DrawQTextBack();

	/// ASSERT: assert(gpBuffer != NULL);
	// TODO: create a function in engine to draw with a given height? (similar to DrawFlask2 in control.cpp)
	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (49 + SCREEN_Y + UI_OFFSET_Y)];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (309 + SCREEN_Y + UI_OFFSET_Y)];

	str = qtextptr;
	pnl = NULL;
	ty = qtexty;

	while (*str != '\0') {
		len = 0;
		sstr = endstr = str;
		while (TRUE) {
			if (*sstr == '\0') {
				endstr = sstr;
				break;
			}
			if (*sstr == '\n') {
				endstr = sstr + 1;
				break;
			}
			c = mfontframe[gbFontTransTbl[(BYTE)*sstr++]];
			len += mfontkern[c] + 2;
			if (c == 0) // allow wordwrap on blank glyph
				endstr = sstr;
			else if (len >= QPANEL_WIDTH - 2 * 24)
				break;
		}

		tx = QPANEL_X + 24;
		while (str < endstr) {
			c = mfontframe[gbFontTransTbl[(BYTE)*str++]];
			if (c != 0) {
				CelDraw(tx, ty, pMedTextCels, c, 22);
			}
			tx += mfontkern[c] + 2;
		}
		if (pnl == NULL) {
			pnl = endstr;
		}
		ty += 38;
		if (ty > 341 + SCREEN_Y + UI_OFFSET_Y) {
			break;
		}
	}

	gpBufStart = pStart;
	gpBufEnd = pEnd;

	for (currTime = SDL_GetTicks(); qtextTime + scrolltexty < currTime; qtextTime += scrolltexty) {
		qtexty--;
		if (qtexty <= 49 + SCREEN_Y + UI_OFFSET_Y) {
			qtexty += 38;
			qtextptr = pnl;
			if (*pnl == '\0') {
				gbQtextflag = false;
			}
			break;
		}
	}
}

DEVILUTION_END_NAMESPACE
