
#include "diabloui.h"
#include "../gameui.h"
#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

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

static void MainmenuLoad()
{
	int numOptions = 3;

	gUIListItems.push_back(new UiListItem("Patch Assets", 0));
	gUIListItems.push_back(new UiListItem("Merge Assets", 1));
	gUIListItems.push_back(new UiListItem("Exit Patcher", 2));

	LoadBackgroundArt("ui_art\\mainmenu.CEL", "ui_art\\menu.pal");

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	//assert(gUIListItems.size() == numOptions);
	SDL_Rect rect1 = { PANEL_MIDX(MAINMENU_WIDTH), MAINMENU_TOP, MAINMENU_WIDTH, MAINMENU_ITEM_HEIGHT * numOptions };
	gUiItems.push_back(new UiList(&gUIListItems, numOptions, rect1, UIS_CENTER | UIS_VCENTER | UIS_HUGE | UIS_GOLD));

	//assert(gUIListItems.size() == numOptions);
	UiInitScreen(numOptions, NULL, UiMainMenuSelect, MainmenuEsc);
}

static void MainmenuFree()
{
	FreeBackgroundArt();

	UiClearItems(gUiItems);

	UiClearListItems();
}

int UiMainMenuDialog(void (*fnSound)(int sfx, int rndCnt))
{
	gfnSoundFunction = fnSound;

	MainmenuLoad();

	_gnMainMenuResult = NUM_MAINMENU;
	do {
		UiRenderAndPoll(NULL);
	} while (_gnMainMenuResult == NUM_MAINMENU);

	MainmenuFree();
	return _gnMainMenuResult;
}

DEVILUTION_END_NAMESPACE
