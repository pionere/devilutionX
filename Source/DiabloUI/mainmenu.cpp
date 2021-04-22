
#include "control.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/selok.h"

DEVILUTION_BEGIN_NAMESPACE

int mainmenu_attract_time_out; //seconds
DWORD dwAttractTicks;

std::vector<UiItemBase *> vecMainMenuDialog;
std::vector<UiListItem *> vecMenuItems;

int MainMenuResult;

void UiMainMenuSelect(std::size_t index)
{
	MainMenuResult = vecMenuItems[index]->m_value;
}

void MainmenuEsc()
{
	std::size_t last = vecMenuItems.size() - 1;
	if (SelectedItem == last) {
		UiMainMenuSelect(last);
	} else {
		SelectedItem = last;
	}
}

void mainmenu_restart_repintro()
{
	dwAttractTicks = SDL_GetTicks() + mainmenu_attract_time_out * 1000;
}

void MainmenuLoad(const char *name, void (*fnSound)(const char *file))
{
	gfnSoundFunction = fnSound;

	vecMenuItems.push_back(new UiListItem("Single Player", MAINMENU_SINGLE_PLAYER));
	vecMenuItems.push_back(new UiListItem("Multi Player", MAINMENU_MULTIPLAYER));
	vecMenuItems.push_back(new UiListItem("Replay Intro", MAINMENU_REPLAY_INTRO));
	vecMenuItems.push_back(new UiListItem("Show Credits", MAINMENU_SHOW_CREDITS));
	vecMenuItems.push_back(new UiListItem("Exit Game", MAINMENU_EXIT_DIABLO));

#ifdef HELLFIRE
	LoadArt("ui_art\\mainmenuw.pcx", &ArtBackgroundWidescreen);
#endif
	LoadBackgroundArt(MENU_ART);

	UiAddBackground(&vecMainMenuDialog);
	UiAddLogo(&vecMainMenuDialog);

	vecMainMenuDialog.push_back(new UiList(vecMenuItems, PANEL_LEFT + 64, (UI_OFFSET_Y + 192), 510, 43, UIS_HUGE | UIS_GOLD | UIS_CENTER));

	SDL_Rect rect = { 17, (SCREEN_HEIGHT - 36), 605, 21 };
	vecMainMenuDialog.push_back(new UiArtText(name, rect, UIS_SMALL));

	UiInitList(vecMainMenuDialog, vecMenuItems.size(), NULL, UiMainMenuSelect, MainmenuEsc, NULL, true);
}

void MainmenuFree()
{
#ifdef HELLFIRE
	ArtBackgroundWidescreen.Unload();
#endif
	ArtBackground.Unload();

	for (auto pUIItem : vecMainMenuDialog) {
		delete pUIItem;
	}
	vecMainMenuDialog.clear();

	for (auto pUIMenuItem : vecMenuItems) {
		delete pUIMenuItem;
	}
	vecMenuItems.clear();
}

bool UiMainMenuDialog(const char *name, int *pdwResult, void (*fnSound)(const char *file), int attractTimeOut)
{
	MainMenuResult = 0;
	while (MainMenuResult == 0) {
		mainmenu_attract_time_out = attractTimeOut;
		MainmenuLoad(name, fnSound);

		mainmenu_restart_repintro(); // for automatic starts

		while (MainMenuResult == 0) {
			UiClearScreen();
			UiPollAndRender();
#ifndef SPAWN
			if (SDL_GetTicks() >= dwAttractTicks) {
				MainMenuResult = MAINMENU_ATTRACT_MODE;
			}
#endif
		}

		MainmenuFree();

#ifdef SPAWN
		if (MainMenuResult == MAINMENU_REPLAY_INTRO) {
			UiSelOkDialog(NULL, "The Diablo introduction cinematic is only available in the full retail version of Diablo. Visit https://www.gog.com/game/diablo to purchase.", true);
			MainMenuResult = 0;
		}
#endif
	}

	*pdwResult = MainMenuResult;
	return true;
}

DEVILUTION_END_NAMESPACE
