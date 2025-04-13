/**
 * @file minitext.cpp
 *
 * Implementation of scrolling dialog text.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"

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

#define MINITEXT_LINE_HEIGHT  38

void StartQTextMsg(int m, bool showText)
{
	const TextData* tds;
	int speed, sfxnr;

	tds = &minitxtdata[m];
	if (tds->txtstr && showText) {
		// ClearPanels();
		// gamemenu_off();
		// StopQTextMsg();
		gbQtextflag = true;
		gbActionBtnDown = 0;
		qtextptr = tds->txtstr;
		qtexty = LTPANEL_Y + TPANEL_HEIGHT + 13;
		speed = tds->txtspd;
		if (speed <= 0)
			speed = 50 / -(speed - 1);
		else
			speed = ((speed + 1) * 50) / speed;
		scrolltexty = speed;
		qtextTime = SDL_GetTicks() + scrolltexty;
	}
	sfxnr = tds->sfxnr;
	if (tds->txtsfxset) {
		sfxnr = sgSFXSets[sfxnr][myplr._pClass];
	}
	PlaySfx(sfxnr);
}

void StopQTextMsg()
{
	gbQtextflag = false;
	StopStreamSFX();
}

void DrawQText()
{
	int len, sx, sy, tx, ty;
	BYTE c;
	const char *pnl, *str, *sstr, *endstr;
	Uint32 currTime;
	BYTE *pStart, *pEnd;

	sx = LTPANEL_X;
	sy = LTPANEL_Y;

	DrawColorTextBox(sx, sy, LTPANEL_WIDTH, TPANEL_HEIGHT, COL_GOLD);

	/// ASSERT: assert(gpBuffer != NULL);
	// TODO: create a function in engine to draw with a given height? (similar to DrawFlask2 in control.cpp)
	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (sy + 25)];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (sy + TPANEL_HEIGHT - 18)];

	str = qtextptr;
	pnl = NULL;
	ty = qtexty;

	while (*str != '\0') {
		len = 0;
		sstr = endstr = str;
		while (true) {
			if (*sstr == '\0') {
				endstr = sstr;
				break;
			}
			if (*sstr == '\n') {
				endstr = sstr + 1;
				break;
			}
			c = gbStdFontFrame[(BYTE)*sstr++];
			len += bigFontWidth[c] + FONT_KERN_BIG;
			if (c == 0) // allow wordwrap on blank glyph
				endstr = sstr;
			else if (len >= LTPANEL_WIDTH - 2 * 24)
				break;
		}

		tx = sx + 24;
		while (str < endstr) {
			// tx += PrintBigChar(tx, ty, (BYTE)*str++, COL_GOLD);
			c = gbStdFontFrame[(BYTE)*str++];
			if (c != 0) {
				CelDraw(tx, ty, pBigTextCels, c);
			}
			tx += bigFontWidth[c] + FONT_KERN_BIG;
		}
		if (pnl == NULL) {
			pnl = endstr;
		}
		ty += MINITEXT_LINE_HEIGHT;
		if (ty >= sy + TPANEL_HEIGHT - 18 + BIG_FONT_HEIGHT) {
			break;
		}
	}

	gpBufStart = pStart;
	gpBufEnd = pEnd;

	for (currTime = SDL_GetTicks(); SDL_TICKS_PASSED(currTime, qtextTime); qtextTime += scrolltexty) {
		if (gnGamePaused != 0)
			continue;
		qtexty--;
		if (qtexty <= sy + 25) {
			qtexty += MINITEXT_LINE_HEIGHT;
			qtextptr = pnl;
			if (*pnl == '\0') {
				// StopQTextMsg(); ?
				gbQtextflag = false;
			}
			break;
		}
	}
}

DEVILUTION_END_NAMESPACE
