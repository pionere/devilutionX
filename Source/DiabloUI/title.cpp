#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"
#include "../gameui.h"
#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

CelImageBuf* gbLogoBig;

static void TitleLoad()
{
	LoadBackgroundArt("ui_art\\title.CEL", "ui_art\\menu.pal");
	// assert(gbLogoBig == NULL);
	gbLogoBig = CelLoadImage("ui_art\\logo.CEL", BIG_LOGO_WIDTH);

	UiAddBackground(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + (PANEL_WIDTH - BIG_LOGO_WIDTH) / 2, PANEL_TOP + 182, BIG_LOGO_WIDTH, BIG_LOGO_HEIGHT };
	gUiItems.push_back(new UiImage(gbLogoBig, 15, rect1, true));
}

static void TitleFree()
{
	MemFreeDbg(gbBackCel);
	MemFreeDbg(gbLogoBig);
	UiClearItems(gUiItems);
}

void UiTitleDialog()
{
	TitleLoad();

	bool endMenu = false;
	Uint32 timeOut = SDL_GetTicks() + 7000;

	SDL_Event event;
	do {
		UiRenderItems(gUiItems);
		UiFadeIn(false);

		while (SDL_PollEvent(&event) != 0) {
			if (GetMenuAction(event) != MenuAction_NONE) {
				endMenu = true;
				break;
			}
			switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_MOUSEBUTTONDOWN:
				endMenu = true;
				break;
			}
			UiHandleEvents(&event);
		}
	} while (!endMenu && SDL_GetTicks() < timeOut);

	TitleFree();
}

DEVILUTION_END_NAMESPACE
