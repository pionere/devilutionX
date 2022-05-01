#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"
#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE* gbLogoBig;

static void TitleLoad()
{
	LoadBackgroundArt("ui_art\\title.CEL", "ui_art\\menu.pal");
	// assert(gbLogoBig == NULL);
	gbLogoBig = LoadFileInMem("ui_art\\logo.CEL");

	UiAddBackground(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + (PANEL_WIDTH - 550) / 2, PANEL_TOP + 182, 550, 216 };
	gUiItems.push_back(new UiImage(gbLogoBig, 15, rect1, true));

	SDL_Rect rect2 = { PANEL_LEFT, (PANEL_TOP + 420), PANEL_WIDTH, 26 };
	gUiItems.push_back(new UiText("Copyright \xA9 1996-2001 Blizzard Entertainment", rect2, UIS_CENTER | UIS_SMALL | UIS_GOLD));
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
