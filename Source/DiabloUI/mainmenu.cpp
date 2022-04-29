
#include "utils/display.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/selok.h"

DEVILUTION_BEGIN_NAMESPACE

static const int ATTRACT_TIMEOUT = 30; //seconds
static Uint32 guAttractTc;

static int _gnMainMenuResult;

static void UiMainMenuSelect(unsigned index)
{
	_gnMainMenuResult = gUIListItems[index]->m_value;
}

static void MainmenuEsc()
{
	unsigned last = gUIListItems.size() - 1;
	if (SelectedItem == last) {
		UiMainMenuSelect(last);
	} else {
		SelectedItem = last;
	}
}

void mainmenu_restart_repintro()
{
	guAttractTc = SDL_GetTicks() + ATTRACT_TIMEOUT * 1000;
}

static void MainmenuLoad(const char* name)
{
	int numOptions = 5;

#ifndef HOSTONLY
	numOptions = 6;
	gUIListItems.push_back(new UiListItem("Single Player", MAINMENU_SINGLE_PLAYER));
#endif
	gUIListItems.push_back(new UiListItem("Multi Player", MAINMENU_MULTIPLAYER));
	gUIListItems.push_back(new UiListItem("Settings", MAINMENU_SETTINGS));
	gUIListItems.push_back(new UiListItem("Replay Intro", MAINMENU_REPLAY_INTRO));
	gUIListItems.push_back(new UiListItem("Show Credits", MAINMENU_SHOW_CREDITS));
	gUIListItems.push_back(new UiListItem("Exit Game", MAINMENU_EXIT_DIABLO));

	LoadBackgroundArt("ui_art\\mainmenu.pcx");

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	//assert(gUIListItems.size() == numOptions);
	SDL_Rect rect1 = { PANEL_LEFT + 64, (UI_OFFSET_Y + 190), PANEL_WIDTH - 64 * 2, 43 * numOptions };
	gUiItems.push_back(new UiList(&gUIListItems, numOptions, rect1, UIS_CENTER | UIS_HUGE | UIS_GOLD));

	SDL_Rect rect2 = { 17, (SCREEN_HEIGHT - 36), 605, 21 };
	gUiItems.push_back(new UiArtText(name, rect2, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	//assert(gUIListItems.size() == numOptions);
	UiInitList(numOptions, NULL, UiMainMenuSelect, MainmenuEsc);
}

static void MainmenuFree()
{
	ArtBackground.Unload();

	UiClearItems(gUiItems);

	UiClearListItems();

	//UiInitList_clear();
}

int UiMainMenuDialog(const char* name, void (*fnSound)(int sfx, int rndCnt))
{
	gfnSoundFunction = fnSound;

	MainmenuLoad(name);

	mainmenu_restart_repintro(); // for automatic starts

	_gnMainMenuResult = NUM_MAINMENU;
	do {
		UiClearScreen();
		UiPollAndRender();
		if (SDL_GetTicks() >= guAttractTc) {
			_gnMainMenuResult = MAINMENU_ATTRACT_MODE;
		}
	} while (_gnMainMenuResult == NUM_MAINMENU);

	MainmenuFree();
	return _gnMainMenuResult;
}

DEVILUTION_END_NAMESPACE
