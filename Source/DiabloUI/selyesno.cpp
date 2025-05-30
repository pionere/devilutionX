#include "selyesno.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "all.h"
//#include "../gameui.h"
//#include "../engine.h"
//#include "storm/storm.h"

DEVILUTION_BEGIN_NAMESPACE

DISABLE_SPEED_OPTIMIZATION

static bool _gbYNEndMenu;
static bool _gbYNValue;

#define MESSAGE_WIDTH (PANEL_WIDTH - 130 * 2)

static void SelyesnoFree()
{
	// FreeBackgroundArt();
	UiClearListItems();

	UiClearItems();
}

static void SelyesnoSelect(unsigned index)
{
	_gbYNValue = index == 0;
	_gbYNEndMenu = true;
}

static void SelyesnoEsc()
{
	_gbYNValue = false;
	_gbYNEndMenu = true;
}

bool UiSelYesNoDialog(const char* title, const char* body)
{
	char selyesno_confirmationMessage[256];

	LoadBackgroundArt(NULL, "ui_art\\menu.pal");
	// UiAddBackground();
	UiAddLogo();

	SDL_Rect rect1 = { 0, SELYNOK_TITLE_TOP, SCREEN_WIDTH, 35 };
	gUiItems.push_back(new UiText(title, rect1, UIS_HCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { PANEL_LEFT + 130, SELYNOK_CONTENT_TOP, MESSAGE_WIDTH, SELYNOK_CONTENT_HEIGHT };
	gUiItems.push_back(new UiText(selyesno_confirmationMessage, rect2, UIS_LEFT | UIS_MED | UIS_SILVER));

	gUIListItems.push_back(new UiListItem("Yes", 0));
	gUIListItems.push_back(new UiListItem("No", 1));
	SDL_Rect rect3 = { SCREEN_MIDX(180), SELYNOK_BUTTON_TOP, 180, 35 * 2 };
	gUiItems.push_back(new UiList(&gUIListItems, 2, rect3, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(selyesno_confirmationMessage, body, sizeof(selyesno_confirmationMessage));
	WordWrapArtStr(selyesno_confirmationMessage, MESSAGE_WIDTH, (unsigned)UIS_MED >> 0);

	//assert(gUIListItems.size() == 2);
	UiInitScreen(2, NULL, SelyesnoSelect, SelyesnoEsc);

	// _gbYNValue = true;
	_gbYNEndMenu = false;
	do {
		UiRenderAndPoll();
	} while (!_gbYNEndMenu);

	SelyesnoFree();

	return _gbYNValue;
}

ENABLE_SPEED_OPTIMIZATION

DEVILUTION_END_NAMESPACE
