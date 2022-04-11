#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

#ifndef HELLFIRE
Art ArtLogoBig;
#endif

static void TitleLoad()
{
#ifdef HELLFIRE
	LoadBackgroundArt("ui_art\\hf_logo1.pcx", 16);
#ifndef NOWIDESCREEN
	LoadArt("ui_art\\hf_titlew.pcx", &ArtBackgroundWidescreen);
#endif
	UiAddBackground(&gUiItems);
#else
	LoadBackgroundArt("ui_art\\title.pcx");
	LoadMaskedArt("ui_art\\logo.pcx", &ArtLogoBig, 15, 250);

	UiAddBackground(&gUiItems);

	SDL_Rect rect1 = { 0, UI_OFFSET_Y + 182, 0, 0 };
	gUiItems.push_back(new UiImage(&ArtLogoBig, 0, rect1, UIS_CENTER, true));

	SDL_Rect rect2 = { PANEL_LEFT + 49, (UI_OFFSET_Y + 410), 550, 26 };
	gUiItems.push_back(new UiArtText("Copyright \xA9 1996-2001 Blizzard Entertainment", rect2, UIS_CENTER | UIS_MED | UIS_SILVER));
#endif
}

static void TitleFree()
{
	ArtBackground.Unload();
#ifdef HELLFIRE
#ifndef NOWIDESCREEN
	ArtBackgroundWidescreen.Unload();
#endif
#else // HELLFIRE
	ArtLogoBig.Unload();
#endif
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
