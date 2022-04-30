#include "selyesno.h"

#include "storm/storm.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbYNEndMenu;
static bool _gbYNValue;

#define MESSAGE_WIDTH (PANEL_WIDTH - 130 * 2)

static void SelyesnoFree()
{
	ArtBackground.Unload();

	UiClearListItems();

	UiClearItems(gUiItems);

	//UiInitList_clear();
}

static void SelyesnoSelect(unsigned index)
{
	_gbYNValue = gUIListItems[index]->m_value == 0;
	_gbYNEndMenu = true;
}

static void SelyesnoEsc()
{
	_gbYNValue = false;
	_gbYNEndMenu = true;
}

bool UiSelHeroYesNoDialog(const char *title, const char *body)
{
	char selyesno_confirmationMessage[256];

	LoadBackgroundArt("ui_art\\black.pcx");
	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + 0, (UI_OFFSET_Y + 161), PANEL_WIDTH, 35 };
	gUiItems.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { PANEL_LEFT + 130, (UI_OFFSET_Y + 236), MESSAGE_WIDTH, 168 };
	gUiItems.push_back(new UiArtText(selyesno_confirmationMessage, rect2, UIS_LEFT | UIS_MED | UIS_SILVER));

	gUIListItems.push_back(new UiListItem("Yes", 0));
	gUIListItems.push_back(new UiListItem("No", 1));
	SDL_Rect rect3 = { PANEL_LEFT + 230, (UI_OFFSET_Y + 390), 180, 35 * 2 };
	gUiItems.push_back(new UiList(&gUIListItems, 2, rect3, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(selyesno_confirmationMessage, body, sizeof(selyesno_confirmationMessage));
	WordWrapArtStr(selyesno_confirmationMessage, MESSAGE_WIDTH, AFT_MED);

	//assert(gUIListItems.size() == 2);
	UiInitList(2, NULL, SelyesnoSelect, SelyesnoEsc);

	// _gbYNValue = true;
	_gbYNEndMenu = false;
	do {
		UiClearScreen();
		//UiRenderItems(gUiItems);
		UiPollAndRender();
	} while (!_gbYNEndMenu);

	SelyesnoFree();

	return _gbYNValue;
}
DEVILUTION_END_NAMESPACE
