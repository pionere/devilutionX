
#include "diabloui.h"
#include "all.h"
//#include "../gameui.h"
//#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

static int _gnMainMenuResult;

static void UiMainMenuSelect(unsigned index)
{
	_gnMainMenuResult = gUIListItems[index]->m_value;
}

static void MainmenuEsc()
{
	unsigned last = (unsigned)gUIListItems.size() - 1;
	if (SelectedItem == last) {
		UiMainMenuSelect(last);
	} else {
		SelectedItem = last;
	}
}

static void MainmenuLoad()
{
	gUIListItems.push_back(new UiListItem("Patch Assets", PATCHERMENU_PATCH));
	gUIListItems.push_back(new UiListItem("Merge Assets", PATCHERMENU_MERGE));
	// gUIListItems.push_back(new UiListItem("Check Assets", PATCHERMENU_CHECK));
	gUIListItems.push_back(new UiListItem("Exit Patcher", PATCHERMENU_EXIT));

	LoadBackgroundArt(NULL, "ui_art\\menu.pal");

	// UiAddBackground();
	UiAddLogo();

	//assert(gUIListItems.size() == numOptions);
	SDL_Rect rect1 = { SCREEN_MIDX(MAINMENU_WIDTH), MAINMENU_TOP, MAINMENU_WIDTH, MAINMENU_ITEM_HEIGHT * NUM_PATCHERMENU };
	gUiItems.push_back(new UiList(&gUIListItems, NUM_PATCHERMENU, rect1, UIS_HCENTER | UIS_VCENTER | UIS_HUGE | UIS_GOLD));

	//assert(gUIListItems.size() == numOptions);
	UiInitScreen(NUM_PATCHERMENU, NULL, UiMainMenuSelect, MainmenuEsc);
}

static void MainmenuFree()
{
	// FreeBackgroundArt();

	UiClearItems();

	UiClearListItems();
}

int UiMainMenuDialog()
{
	MainmenuLoad();

	_gnMainMenuResult = NUM_MAINMENU;
	do {
		UiRenderAndPoll();
	} while (_gnMainMenuResult == NUM_MAINMENU);

	MainmenuFree();
	return _gnMainMenuResult;
}

DEVILUTION_END_NAMESPACE
