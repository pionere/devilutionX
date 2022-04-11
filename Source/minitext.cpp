/**
 * @file minitext.cpp
 *
 * Implementation of scrolling dialog text.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Current y position of text in px */
static int qtexty;
/** Pointer to the current text being displayed */
static const char* qtextptr;
/** Time of next rendering of the text */
static Uint32 qtextTime;
/** Specify if the quest dialog window is being shown */
bool gbQtextflag;
/** Vertical speed of the scrolling text in ms/px */
static int scrolltexty;

void InitQTextMsg(int m, bool showText)
{
	const TextData* tds;
	int sfxnr;

	tds = &alltext[m];
	if (tds->scrlltxt && showText) {
		ClearPanels();
		gbQtextflag = true;
		qtextptr = tds->txtstr;
		qtexty = LTPANEL_Y + TPANEL_HEIGHT + 13;
		scrolltexty = tds->txtspd;
		qtextTime = SDL_GetTicks() + scrolltexty;
	}
	sfxnr = tds->sfxnr;
	if (tds->txtsfxset) {
		sfxnr = sgSFXSets[sfxnr][myplr._pClass];
	}
	PlaySFX(sfxnr);
}

void DrawQText()
{
	int len, tx, ty;
	BYTE c;
	const char *pnl, *str, *sstr, *endstr;
	Uint32 currTime;
	BYTE *pStart, *pEnd;

	DrawTextBox();

	/// ASSERT: assert(gpBuffer != NULL);
	// TODO: create a function in engine to draw with a given height? (similar to DrawFlask2 in control.cpp)
	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (LTPANEL_Y + 25)];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (LTPANEL_Y + TPANEL_HEIGHT - 18)];

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
			c = bhFontFrame[gbFontTransTbl[(BYTE)*sstr++]];
			len += bigFontWidth[c] + 2;
			if (c == 0) // allow wordwrap on blank glyph
				endstr = sstr;
			else if (len >= LTPANEL_WIDTH - 2 * 24)
				break;
		}

		tx = LTPANEL_X + 24;
		while (str < endstr) {
			c = bhFontFrame[gbFontTransTbl[(BYTE)*str++]];
			if (c != 0) {
				CelDraw(tx, ty, pBigTextCels, c, 22);
			}
			tx += bigFontWidth[c] + 2;
		}
		if (pnl == NULL) {
			pnl = endstr;
		}
		ty += 38;
		if (ty > LTPANEL_Y + TPANEL_HEIGHT + 14) {
			break;
		}
	}

	gpBufStart = pStart;
	gpBufEnd = pEnd;

	for (currTime = SDL_GetTicks(); qtextTime < currTime; qtextTime += scrolltexty) {
		if (gbGamePaused)
			continue;
		qtexty--;
		if (qtexty <= LTPANEL_Y + 25) {
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
