#include "selyesno.h"

#include "storm/storm.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbYNEndMenu;
static bool _gbYNValue;

static std::vector<UiListItem *> vecSelYesNoDialogItems;
static std::vector<UiItemBase *> vecSelYesNoDialog;

#define MESSAGE_WIDTH (PANEL_WIDTH - 130 * 2)

static void SelyesnoFree()
{
	ArtBackground.Unload();

	UiClearListItems(vecSelYesNoDialogItems);

	UiClearItems(vecSelYesNoDialog);
}

static void SelyesnoSelect(unsigned index)
{
	_gbYNValue = vecSelYesNoDialogItems[index]->m_value == 0;
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
	UiAddBackground(&vecSelYesNoDialog);
	UiAddLogo(&vecSelYesNoDialog);

	SDL_Rect rect1 = { PANEL_LEFT + 0, (UI_OFFSET_Y + 161), PANEL_WIDTH, 35 };
	vecSelYesNoDialog.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { PANEL_LEFT + 130, (UI_OFFSET_Y + 236), MESSAGE_WIDTH, 168 };
	vecSelYesNoDialog.push_back(new UiArtText(selyesno_confirmationMessage, rect2, UIS_LEFT | UIS_MED | UIS_SILVER));

	vecSelYesNoDialogItems.push_back(new UiListItem("Yes", 0));
	vecSelYesNoDialogItems.push_back(new UiListItem("No", 1));
	SDL_Rect rect3 = { PANEL_LEFT + 230, (UI_OFFSET_Y + 390), 180, 35 };
	vecSelYesNoDialog.push_back(new UiList(&vecSelYesNoDialogItems, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(selyesno_confirmationMessage, body, sizeof(selyesno_confirmationMessage));
	WordWrapArtStr(selyesno_confirmationMessage, MESSAGE_WIDTH, AFT_MED);

	UiInitList(vecSelYesNoDialog, vecSelYesNoDialogItems.size(), NULL, SelyesnoSelect, SelyesnoEsc, NULL);

	// _gbYNValue = true;
	_gbYNEndMenu = false;
	do {
		UiClearScreen();
		UiRenderItems(vecSelYesNoDialog);
		UiPollAndRender();
	} while (!_gbYNEndMenu);

	SelyesnoFree();

	return _gbYNValue;
}
DEVILUTION_END_NAMESPACE
