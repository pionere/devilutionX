#include <algorithm>

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text_draw.h"
#include "../gameui.h"
#include "../engine.h"
#include "../dx.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbCreditsEnd;
#ifdef HELLFIRE
#define CREDITS_LINES_SIZE 91
#define CREDITS_TXT        "Meta\\credits_hf.txt"
#else
#define CREDITS_LINES_SIZE 455
#define CREDITS_TXT        "Meta\\credits.txt"
#endif
static char** CREDITS_LINES;

static void CreditsEsc()
{
	_gbCreditsEnd = true;
}

static void CreditsSelect(unsigned index)
{
	_gbCreditsEnd = true;
}

static void CreditsRender(Uint32 ticks_begin_)
{
	BYTE *pStart, *pEnd;

	int offsetY = -CREDITS_HEIGHT + (SDL_GetTicks() - ticks_begin_) / 32;
	int linesBegin = std::max(offsetY / CREDITS_LINE_H, 0);
	int linesEnd = std::min((CREDITS_HEIGHT + offsetY + CREDITS_LINE_H - 1) / CREDITS_LINE_H, (int)CREDITS_LINES_SIZE);

	if (linesBegin >= CREDITS_LINES_SIZE) {
		_gbCreditsEnd = true;
		return;
	}

	UiClearScreen();
	UiRenderItems();

	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + CREDITS_TOP)];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + CREDITS_TOP + CREDITS_HEIGHT)];

	int destY = CREDITS_TOP - (offsetY - linesBegin * CREDITS_LINE_H);
	for (int i = linesBegin; i < linesEnd; ++i, destY += CREDITS_LINE_H) {
		const char* text = CREDITS_LINES[i];
		SDL_Rect dstRect = { CREDITS_LEFT, destY, 0, 0 };
		DrawArtStr(text, dstRect, UIS_LEFT | UIS_SMALL | UIS_GOLD);
	}

	gpBufStart = pStart;
	gpBufEnd = pEnd;
}

void UiCreditsDialog()
{
	Uint32 ticks_begin_;

	CREDITS_LINES = LoadTxtFile(CREDITS_TXT, CREDITS_LINES_SIZE);

	LoadBackgroundArt("ui_art\\credits.CEL", "ui_art\\credits.pal");
	UiAddBackground();
	UiInitScreen(0, NULL, CreditsSelect, CreditsEsc);
	gUiDrawCursor = false;
	ticks_begin_ = SDL_GetTicks();

	_gbCreditsEnd = false;

	Dvl_Event event;
	do {
		CreditsRender(ticks_begin_);

		UiFadeIn();
		while (UiPeekAndHandleEvents(&event)) {
			;
		}
	} while (!_gbCreditsEnd);

	FreeBackgroundArt();
	UiClearItems();
	MemFreeTxtFile(CREDITS_LINES);
}

DEVILUTION_END_NAMESPACE
