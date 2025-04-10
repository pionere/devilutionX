
#include "DiabloUI/diabloui.h"
#include "DiabloUI/text_draw.h"
#include "all.h"
//#include <algorithm>
//#include "../gameui.h"
//#include "../engine.h"
//#include "../dx.h"

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

static void CreditsRender(const UiItemBase* _THIS)
{
	const UiTextScroll* _this = (const UiTextScroll*)_THIS;
	BYTE *pStart, *pEnd;

	int offsetY = -CREDITS_HEIGHT + (SDL_GetTicks() - _this->m_ticks_begin) / 32;
	int linesBegin = std::max(offsetY / CREDITS_LINE_H, 0);
	int linesEnd = std::min((CREDITS_HEIGHT + offsetY + CREDITS_LINE_H - 1) / CREDITS_LINE_H, (int)CREDITS_LINES_SIZE);

	if (linesBegin >= CREDITS_LINES_SIZE) {
		_gbCreditsEnd = true;
		return;
	}

	pStart = gpBufStart;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + CREDITS_TOP)];
	pEnd = gpBufEnd;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + CREDITS_TOP + CREDITS_HEIGHT)];

	int destY = CREDITS_TOP - (offsetY - linesBegin * CREDITS_LINE_H);
	for (int i = linesBegin; i < linesEnd; ++i, destY += CREDITS_LINE_H) {
		const char* text = _this->m_text[i];
		SDL_Rect dstRect = { CREDITS_LEFT, destY, 0, 0 };
		DrawArtStr(text, dstRect, UIS_LEFT | UIS_SMALL | UIS_GOLD);
	}

	gpBufStart = pStart;
	gpBufEnd = pEnd;
}

void UiCreditsDialog()
{
	LoadBackgroundArt("ui_art\\credits.CEL", "ui_art\\credits.pal");
	UiAddBackground();
	SDL_Rect rect1 = { 0, 0, 0, 0 };
	gUiItems.push_back(new UiTextScroll(CREDITS_TXT, CREDITS_LINES_SIZE, SDL_GetTicks(), CreditsRender, rect1));
	UiInitScreen(0, NULL, CreditsSelect, CreditsEsc);
	gUiDrawCursor = false;

	_gbCreditsEnd = false;
	do {
		UiRenderAndPoll();
	} while (!_gbCreditsEnd);

	FreeBackgroundArt();
	UiClearItems();
}

DEVILUTION_END_NAMESPACE
