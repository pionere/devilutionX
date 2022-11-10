#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"
#include "../gameui.h"
#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

#define TITLE_TIMEOUT_SEC 7
CelImageBuf* gbLogoBig;

static void TitleLoad()
{
	LoadBackgroundArt("ui_art\\title.CEL", "ui_art\\menu.pal");
	// assert(gbLogoBig == NULL);
	gbLogoBig = CelLoadImage("ui_art\\logo.CEL", BIG_LOGO_WIDTH);

	UiAddBackground(&gUiItems);

	SDL_Rect rect1 = { PANEL_MIDX(BIG_LOGO_WIDTH), BIG_LOGO_TOP, BIG_LOGO_WIDTH, BIG_LOGO_HEIGHT };
	gUiItems.push_back(new UiImage(gbLogoBig, 15, rect1, true));

	UiInitList(0);
}

static void TitleFree()
{
	MemFreeDbg(gbBackCel);
	MemFreeDbg(gbLogoBig);
	UiClearItems(gUiItems);
}

bool UiTitleDialog()
{
	TitleLoad();

	int endMenu = 0;
	Uint32 timeOut = SDL_GetTicks() + TITLE_TIMEOUT_SEC * 1000;

	SDL_Event event;
	do {
		UiRenderItems(gUiItems);
		UiFadeIn(false);

		while (SDL_PollEvent(&event) != 0) {
			if (GetMenuAction(event) != MenuAction_NONE) {
				endMenu = 1;
				break;
			}
			if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
				endMenu = 1;
				break;
			}
			if (event.type == SDL_QUIT) {
				endMenu = 2;
				break;
			}
			UiHandleEvents(&event);
		}
	} while (endMenu == 0 && SDL_GetTicks() < timeOut);

	TitleFree();

	return endMenu != 2;
}

DEVILUTION_END_NAMESPACE
