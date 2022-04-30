#include <algorithm>

#include "controls/menu_controls.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/credits_lines.h"
#include "DiabloUI/text_draw.h"
#include "../engine.h"
#include "../dx.h"

DEVILUTION_BEGIN_NAMESPACE

#define VIEWPORT_Y		100
#define VIEWPORT_H		280
#define LINE_H	22

static bool CreditsRender(int offsetY)
{
	BYTE *pStart, *pEnd;

	UiClearScreen();
	UiRenderItems(gUiItems);

	int linesBegin = std::max(offsetY / LINE_H, 0);
	int linesEnd = std::min((VIEWPORT_H + offsetY + LINE_H - 1) / LINE_H, (int)CREDITS_LINES_SIZE);

	if (linesBegin >= linesEnd) {
		return linesEnd != CREDITS_LINES_SIZE;
	}

	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (PANEL_Y + VIEWPORT_Y )];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (PANEL_Y + VIEWPORT_Y + VIEWPORT_H)];

	int destY = PANEL_TOP + VIEWPORT_Y - (offsetY - linesBegin * LINE_H);
	for (int i = linesBegin; i < linesEnd; ++i, destY += LINE_H) {
		const char* text = CREDITS_LINES[i];
		int destX = PANEL_LEFT + 31;
		for ( ; *text == '\t'; text++) {
			destX += 40;
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
	bool endMenu = false;
	Uint32 ticks_begin_;
	int prev_offset_y_ = 0;

	LoadBackgroundArt("ui_art\\credits.CEL", "ui_art\\credits.pal");
	UiAddBackground(&gUiItems);
	ticks_begin_ = SDL_GetTicks();

	SDL_Event event;
	do {
		int offsetY = -VIEWPORT_H + (SDL_GetTicks() - ticks_begin_) / 32;
		if (offsetY != prev_offset_y_ && !CreditsRender(offsetY))
			break;
		prev_offset_y_ = offsetY;

		UiFadeIn(false);
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_MOUSEBUTTONDOWN:
				endMenu = true;
				break;
			default:
				switch (GetMenuAction(event)) {
				case MenuAction_BACK:
				case MenuAction_SELECT:
					endMenu = true;
					break;
				default:
					break;
				}
			}
			UiHandleEvents(&event);
		}
	} while (!endMenu);

	MemFreeDbg(gbBackCel);
	UiClearItems(gUiItems);
}

DEVILUTION_END_NAMESPACE
