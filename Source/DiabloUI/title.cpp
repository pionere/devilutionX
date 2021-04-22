#include "DiabloUI/diabloui.h"
#include "control.h"
#include "controls/menu_controls.h"

DEVILUTION_BEGIN_NAMESPACE

std::vector<UiItemBase *> vecTitleScreen;

void TitleLoad()
{
#ifdef HELLFIRE
	LoadBackgroundArt("ui_art\\hf_logo1.pcx", 16);
	LoadArt("ui_art\\hf_titlew.pcx", &ArtBackgroundWidescreen);
#else
	LoadBackgroundArt("ui_art\\title.pcx");
	LoadMaskedArt("ui_art\\logo.pcx", &ArtLogos[LOGO_BIG], 15);
#endif
}

void TitleFree()
{
	ArtBackground.Unload();
#ifdef HELLFIRE
	ArtBackgroundWidescreen.Unload();
#endif
	ArtLogos[LOGO_BIG].Unload();

	for (auto pUIItem : vecTitleScreen) {
		delete pUIItem;
	}
	vecTitleScreen.clear();
}

void UiTitleDialog()
{
#ifdef HELLFIRE
	SDL_Rect rect = { 0, UI_OFFSET_Y, 0, 0 };
	vecTitleScreen.push_back(new UiImage(&ArtBackgroundWidescreen, /*bAnimated=*/true, /*iFrame=*/0, rect, UIS_CENTER));
	vecTitleScreen.push_back(new UiImage(&ArtBackground, /*bAnimated=*/true, /*iFrame=*/0, rect, UIS_CENTER));
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

		while (SDL_PollEvent(&event)) {
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
