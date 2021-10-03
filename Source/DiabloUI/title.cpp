#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

static std::vector<UiItemBase *> vecTitleScreen;
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
	UiAddBackground(&vecTitleScreen);
#else
	LoadBackgroundArt("ui_art\\title.pcx");
	LoadMaskedArt("ui_art\\logo.pcx", &ArtLogoBig, 15);

	UiAddBackground(&vecTitleScreen);

	SDL_Rect rect1 = { 0, UI_OFFSET_Y + 182, 0, 0 };
	vecTitleScreen.push_back(new UiImage(&ArtLogoBig, rect1));

	SDL_Rect rect2 = { PANEL_LEFT + 49, (UI_OFFSET_Y + 410), 550, 26 };
	vecTitleScreen.push_back(new UiArtText("Copyright \xA9 1996-2001 Blizzard Entertainment", rect2, UIS_CENTER | UIS_MED));
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
	UiClearItems(vecTitleScreen);
}

void UiTitleDialog()
{
	TitleLoad();

	bool endMenu = false;
	Uint32 timeOut = SDL_GetTicks() + 7000;

	SDL_Event event;
	while (!endMenu && SDL_GetTicks() < timeOut) {
		UiRenderItems(vecTitleScreen);
		UiFadeIn();

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
	}

	TitleFree();
}

DEVILUTION_END_NAMESPACE
