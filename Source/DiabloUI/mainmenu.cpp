#include "all.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/selok.h"

DEVILUTION_BEGIN_NAMESPACE

static int _gnAttractTimeout; //seconds
static DWORD _gdwAttractTicks;

static std::vector<UiItemBase *> vecMainMenuDialog;
static std::vector<UiListItem *> vecMenuItems;

static int _gnMainMenuResult;

static void UiMainMenuSelect(unsigned index)
{
	_gnMainMenuResult = vecMenuItems[index]->m_value;
}

static void MainmenuEsc()
{
	unsigned last = vecMenuItems.size() - 1;
	if (SelectedItem == last) {
		UiMainMenuSelect(last);
	} else {
		SelectedItem = last;
	}
}

void mainmenu_restart_repintro()
{
	_gdwAttractTicks = SDL_GetTicks() + _gnAttractTimeout * 1000;
}

static void MainmenuLoad(const char *name, void (*fnSound)(const char *file))
{
	gfnSoundFunction = fnSound;

	vecMenuItems.push_back(new UiListItem("Single Player", MAINMENU_SINGLE_PLAYER));
	vecMenuItems.push_back(new UiListItem("Multi Player", MAINMENU_MULTIPLAYER));
	vecMenuItems.push_back(new UiListItem("Replay Intro", MAINMENU_REPLAY_INTRO));
	vecMenuItems.push_back(new UiListItem("Show Credits", MAINMENU_SHOW_CREDITS));
	vecMenuItems.push_back(new UiListItem("Exit Game", MAINMENU_EXIT_DIABLO));

#ifndef NOWIDESCREEN
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

static void MainmenuFree()
{
#ifndef NOWIDESCREEN
	ArtBackgroundWidescreen.Unload();
#endif
	ArtBackground.Unload();

	for (unsigned i = 0; i < vecMainMenuDialog.size(); i++) {
		UiItemBase *pUIItem = vecMainMenuDialog[i];
		delete pUIItem;
	}
	vecMainMenuDialog.clear();

	for (unsigned i = 0; i < vecMenuItems.size(); i++) {
		UiListItem *pUIMenuItem = vecMenuItems[i];
		delete pUIMenuItem;
	}
	vecMenuItems.clear();
}

void UiMainMenuDialog(const char *name, int *pdwResult, void (*fnSound)(const char *file), int attractTimeOut)
{
	_gnMainMenuResult = 0;
	while (_gnMainMenuResult == 0) {
		_gnAttractTimeout = attractTimeOut;
		MainmenuLoad(name, fnSound);

		mainmenu_restart_repintro(); // for automatic starts

		while (_gnMainMenuResult == 0) {
			UiClearScreen();
			UiPollAndRender();
			if (SDL_GetTicks() >= _gdwAttractTicks) {
				_gnMainMenuResult = MAINMENU_ATTRACT_MODE;
			}
		}

		MainmenuFree();
	}

	*pdwResult = _gnMainMenuResult;
}

DEVILUTION_END_NAMESPACE
