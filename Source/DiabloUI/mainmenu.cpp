
#include "utils/display.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/selok.h"

DEVILUTION_BEGIN_NAMESPACE

static const int ATTRACT_TIMEOUT = 30; //seconds
static Uint32 guAttractTc;

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
	guAttractTc = SDL_GetTicks() + ATTRACT_TIMEOUT * 1000;
}

static void MainmenuLoad(const char* name)
{
	int numOptions = 4;

#ifndef HOSTONLY
	numOptions = 5;
	vecMenuItems.push_back(new UiListItem("Single Player", MAINMENU_SINGLE_PLAYER));
#endif
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

	//assert(vecMenuItems.size() == numOptions);
	SDL_Rect rect1 = { PANEL_LEFT + 64, (UI_OFFSET_Y + 192), 510, 43 * numOptions };
	vecMainMenuDialog.push_back(new UiList(&vecMenuItems, numOptions, rect1, UIS_CENTER | UIS_HUGE | UIS_GOLD));

	SDL_Rect rect2 = { 17, (SCREEN_HEIGHT - 36), 605, 21 };
	vecMainMenuDialog.push_back(new UiArtText(name, rect2, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	//assert(vecMenuItems.size() == numOptions);
	UiInitList(vecMainMenuDialog, numOptions, NULL, UiMainMenuSelect, MainmenuEsc, NULL, true);
}

static void MainmenuFree()
{
#ifndef NOWIDESCREEN
	ArtBackgroundWidescreen.Unload();
#endif
	ArtBackground.Unload();

	UiClearItems(vecMainMenuDialog);

	UiClearListItems(vecMenuItems);
}

int UiMainMenuDialog(const char* name, void (*fnSound)(int sfx, int rndCnt))
{
	gfnSoundFunction = fnSound;

	MainmenuLoad(name);

	mainmenu_restart_repintro(); // for automatic starts

	_gnMainMenuResult = 0;
	do {
		UiClearScreen();
		UiPollAndRender();
		if (SDL_GetTicks() >= guAttractTc) {
			_gnMainMenuResult = MAINMENU_ATTRACT_MODE;
		}
	} while (_gnMainMenuResult == 0);

	MainmenuFree();
	return _gnMainMenuResult;
}

DEVILUTION_END_NAMESPACE
