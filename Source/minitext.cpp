/**
 * @file minitext.cpp
 *
 * Implementation of scrolling dialog text.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

/** Current y offset of the text in px */
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
#define MINITEXT_PNL_X_OFFSET 24
#define MINITEXT_PNL_Y_OFFSET 21
#define MINITEXT_PANEL_WIDTH  591
#define MINITEXT_PANEL_HEIGHT 303
#define MINITEXT_PANEL_X      PANEL_CENTERX(MINITEXT_PANEL_WIDTH)
#define MINITEXT_PANEL_Y      (PANEL_CENTERY(MINITEXT_PANEL_HEIGHT) - 64)

static_assert(1900 / MINITEXT_LINE_HEIGHT == 50, "minitxtdata must be timed.");

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
		qtexty = (MINITEXT_PANEL_HEIGHT - MINITEXT_PNL_Y_OFFSET) + MINITEXT_LINE_HEIGHT - 7;
		scrolltexty = tds->txtdelay;
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

	sx = MINITEXT_PANEL_X;
	sy = MINITEXT_PANEL_Y;

	DrawColorTextBox(sx, sy, MINITEXT_PANEL_WIDTH, MINITEXT_PANEL_HEIGHT, COL_GOLD);

	/// ASSERT: assert(gpBuffer != NULL);
	// TODO: create a function in engine to draw with a given height? (similar to DrawFlask2 in control.cpp)
	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (sy + MINITEXT_PNL_Y_OFFSET)];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (sy + MINITEXT_PANEL_HEIGHT - MINITEXT_PNL_Y_OFFSET)];

	str = qtextptr;
	pnl = NULL;
	ty = sy + qtexty;
	sx += MINITEXT_PNL_X_OFFSET;

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
			else if (len >= MINITEXT_PANEL_WIDTH - 2 * MINITEXT_PNL_X_OFFSET)
				break;
		}

		tx = sx;
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
		if (ty >= sy + MINITEXT_PANEL_HEIGHT - MINITEXT_PNL_Y_OFFSET + BIG_FONT_HEIGHT) {
			break;
		}
	}

	gpBufStart = pStart;
	gpBufEnd = pEnd;

	for (currTime = SDL_GetTicks(); SDL_TICKS_PASSED(currTime, qtextTime); qtextTime += scrolltexty) {
		if (gnGamePaused != 0)
			continue;
		qtexty--;
		if (qtexty <= MINITEXT_PNL_Y_OFFSET) {
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
