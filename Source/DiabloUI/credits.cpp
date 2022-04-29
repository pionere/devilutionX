#include <algorithm>

#include "controls/menu_controls.h"
#include "utils/display.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/credits_lines.h"
#include "DiabloUI/art.h"
#include "DiabloUI/art_draw.h"
#include "DiabloUI/fonts.h"

DEVILUTION_BEGIN_NAMESPACE

namespace {

static const SDL_Rect VIEWPORT = { 0, 114, PANEL_WIDTH, 251 };
#define LINE_H	22

// The maximum number of visible lines is the number of whole lines
// (VIEWPORT.h / LINE_H) rounded up, plus one extra line for when
// a line is leaving the screen while another one is entering.
#define MAX_VISIBLE_LINES ((VIEWPORT.h - 1) / LINE_H + 2)

class CreditsRenderer {

public:
	CreditsRenderer()
	{
		LoadBackgroundArt("ui_art\\credits.pcx");
		UiAddBackground(&gUiItems);
		ticks_begin_ = SDL_GetTicks();
		prev_offset_y_ = 0;
	}

	~CreditsRenderer()
	{
		ArtBackground.Unload();
		UiClearItems(gUiItems);
	}

	bool Render();

private:
	Uint32 ticks_begin_;
	int prev_offset_y_;
};

bool CreditsRenderer::Render()
{
	const int offsetY = -VIEWPORT.h + (SDL_GetTicks() - ticks_begin_) / 40;
	if (offsetY == prev_offset_y_)
		return true;
	prev_offset_y_ = offsetY;

	UiClearScreen();
	UiRenderItems(gUiItems);

	const unsigned linesBegin = std::max(offsetY / LINE_H, 0);
	const unsigned linesEnd = std::min(linesBegin + MAX_VISIBLE_LINES, CREDITS_LINES_SIZE);

	if (linesBegin >= linesEnd) {
		return linesEnd != CREDITS_LINES_SIZE;
	}

	SDL_Rect viewport = VIEWPORT;
	viewport.x += PANEL_LEFT;
	viewport.y += UI_OFFSET_Y;
	//ScaleOutputRect(&viewport); -- unnecessary (and wrong) when drawing to back_surface
	viewport.x += SCREEN_X;
	viewport.y += SCREEN_Y;
	SDL_SetClipRect(DiabloUiSurface(), &viewport);

	// We use unscaled coordinates for calculation throughout.
	int destY = UI_OFFSET_Y + VIEWPORT.y - (offsetY - linesBegin * LINE_H);
	for (unsigned i = linesBegin; i < linesEnd; ++i, destY += LINE_H) {
		const char* text = CREDITS_LINES[i];
		int destX = PANEL_LEFT + VIEWPORT.x + 31;
		for ( ; *text == '\t'; text++) {
			destX += 40;
		}
		SDL_Rect dstRect = { destX, destY, 0, 0 };
		DrawArtStr(text, dstRect, UIS_LEFT | UIS_SMALL | UIS_GOLD);
	}

	SDL_SetClipRect(DiabloUiSurface(), NULL);
	return true;
}

} // namespace

void UiCreditsDialog()
{
	CreditsRenderer creditsRenderer;
	bool endMenu = false;

	SDL_Event event;
	do {
		if (!creditsRenderer.Render())
			break;
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
}

DEVILUTION_END_NAMESPACE
