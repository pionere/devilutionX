#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

static std::vector<UiItemBase *> vecTitleScreen;

static void TitleLoad()
{
#ifdef HELLFIRE
	LoadBackgroundArt("ui_art\\hf_logo1.pcx", 16);
#ifndef NOWIDESCREEN
	LoadArt("ui_art\\hf_titlew.pcx", &ArtBackgroundWidescreen);
#endif
#else
	LoadBackgroundArt("ui_art\\title.pcx");
	LoadMaskedArt("ui_art\\logo.pcx", &ArtLogos[LOGO_BIG], 15);
#endif
}

static void TitleFree()
{
	ArtBackground.Unload();
#ifndef NOWIDESCREEN
	ArtBackgroundWidescreen.Unload();
#endif
#ifndef HELLFIRE
	ArtLogos[LOGO_BIG].Unload();
#endif
	UiClearItems(vecTitleScreen);
}

void UiTitleDialog()
{
#ifdef HELLFIRE
	SDL_Rect rect = { 0, UI_OFFSET_Y, 0, 0 };
#ifndef NOWIDESCREEN
	vecTitleScreen.push_back(new UiImage(&ArtBackgroundWidescreen, rect));
#endif
	vecTitleScreen.push_back(new UiImage(&ArtBackground, rect));
#else
	UiAddBackground(&vecTitleScreen);
	UiAddLogo(&vecTitleScreen, LOGO_BIG, 182);

	SDL_Rect rect = { PANEL_LEFT + 49, (UI_OFFSET_Y + 410), 550, 26 };
	vecTitleScreen.push_back(new UiArtText("Copyright \xA9 1996-2001 Blizzard Entertainment", rect, UIS_MED | UIS_CENTER));
#endif

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
