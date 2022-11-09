
#include "utils/display.h"

#include "DiabloUI/diabloui.h"
#include "../gameui.h"
#include "../engine.h"

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

static void MainmenuLoad()
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

	LoadBackgroundArt("ui_art\\mainmenu.CEL", "ui_art\\menu.pal");

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	//assert(gUIListItems.size() == numOptions);
	SDL_Rect rect1 = { PANEL_MIDX(MAINMENU_WIDTH), MAINMENU_TOP, MAINMENU_WIDTH, MAINMENU_ITEM_HEIGHT * numOptions };
	gUiItems.push_back(new UiList(&gUIListItems, numOptions, rect1, UIS_CENTER | UIS_VCENTER | UIS_HUGE | UIS_GOLD));

	//assert(gUIListItems.size() == numOptions);
	UiInitList(numOptions, NULL, UiMainMenuSelect, MainmenuEsc);
}

static void MainmenuFree()
{
	MemFreeDbg(gbBackCel);

	UiClearItems(gUiItems);

	UiClearListItems();

	//UiInitList_clear();
}

int UiMainMenuDialog(void (*fnSound)(int sfx, int rndCnt))
{
	gfnSoundFunction = fnSound;

	MainmenuLoad();

	mainmenu_restart_repintro(); // for automatic starts

	_gnMainMenuResult = NUM_MAINMENU;
	do {
		UiRenderAndPoll(NULL);
		if (SDL_GetTicks() >= guAttractTc) {
			_gnMainMenuResult = MAINMENU_ATTRACT_MODE;
		}
	} while (_gnMainMenuResult == NUM_MAINMENU);

	MainmenuFree();
	return _gnMainMenuResult;
}

DEVILUTION_END_NAMESPACE
