#include <algorithm>

#include "controls/menu_controls.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/credits_lines.h"
#include "DiabloUI/text_draw.h"
#include "../gameui.h"
#include "../engine.h"
#include "../dx.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbCreditsEnd;

static void CreditsEsc()
{
	_gbCreditsEnd = true;
}

static void CreditsSelect(unsigned index)
{
	_gbCreditsEnd = true;
}

static bool CreditsRender(int offsetY)
{
	BYTE *pStart, *pEnd;

	int linesBegin = std::max(offsetY / CREDITS_LINE_H, 0);
	int linesEnd = std::min((CREDITS_HEIGHT + offsetY + CREDITS_LINE_H - 1) / CREDITS_LINE_H, (int)CREDITS_LINES_SIZE);

	if (linesBegin >= CREDITS_LINES_SIZE)
		return false;

	UiClearScreen();
	UiRenderItems(gUiItems);

	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (PANEL_Y + CREDITS_TOP )];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (PANEL_Y + CREDITS_TOP + CREDITS_HEIGHT)];

	int destY = PANEL_TOP + CREDITS_TOP - (offsetY - linesBegin * CREDITS_LINE_H);
	for (int i = linesBegin; i < linesEnd; ++i, destY += CREDITS_LINE_H) {
		const char* text = CREDITS_LINES[i];
		int destX = PANEL_LEFT + 10;
		for ( ; *text == '\t'; text++) {
			destX += 11;
		}
		SDL_Rect dstRect = { destX, destY, 0, 0 };
		DrawArtStr(text, dstRect, UIS_LEFT | UIS_SMALL | UIS_GOLD);
	}

	gpBufStart = pStart;
	gpBufEnd = pEnd;

	return true;
}

void UiCreditsDialog()
{
	Uint32 ticks_begin_;
	int prev_offset_y_ = 0;

	LoadBackgroundArt("ui_art\\credits.CEL", "ui_art\\credits.pal");
	UiAddBackground(&gUiItems);
	UiInitScreen(0, NULL, CreditsSelect, CreditsEsc);
	gUiDrawCursor = false;
	ticks_begin_ = SDL_GetTicks();

	_gbCreditsEnd = false;

	SDL_Event event;
	do {
		int offsetY = -CREDITS_HEIGHT + (SDL_GetTicks() - ticks_begin_) / 32;
		if (offsetY != prev_offset_y_ && !CreditsRender(offsetY))
			break;
		prev_offset_y_ = offsetY;

		UiFadeIn();
		while (SDL_PollEvent(&event) != 0) {
			UiHandleEvents(&event);
		}
	} while (!_gbCreditsEnd);

	MemFreeDbg(gbBackCel);
	UiClearItems(gUiItems);
}

DEVILUTION_END_NAMESPACE
