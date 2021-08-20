#include "selyesno.h"

#include "all.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbYNEndMenu;
static bool _gbYNValue;

static std::vector<UiListItem *> vecSelYesNoDialogItems;
static std::vector<UiItemBase *> vecSelYesNoDialog;

#define MESSAGE_WIDTH 280

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

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelYesNoDialog.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 120, (UI_OFFSET_Y + 236), MESSAGE_WIDTH, 168 };
	vecSelYesNoDialog.push_back(new UiArtText(selyesno_confirmationMessage, rect2, UIS_MED));

	vecSelYesNoDialogItems.push_back(new UiListItem("Yes", 0));
	vecSelYesNoDialogItems.push_back(new UiListItem("No", 1));
	SDL_Rect rect3 = { PANEL_LEFT + 230, (UI_OFFSET_Y + 390), 180, 35 };
	vecSelYesNoDialog.push_back(new UiList(&vecSelYesNoDialogItems, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(selyesno_confirmationMessage, body, sizeof(selyesno_confirmationMessage));
	WordWrapArtStr(selyesno_confirmationMessage, MESSAGE_WIDTH);

	UiInitList(vecSelYesNoDialog, vecSelYesNoDialogItems.size(), NULL, SelyesnoSelect, SelyesnoEsc, NULL, true);

	// _gbYNValue = true;
	_gbYNEndMenu = false;
	while (!_gbYNEndMenu) {
		UiClearScreen();
		UiRenderItems(vecSelYesNoDialog);
		UiPollAndRender();
	}

	SelyesnoFree();

	return _gbYNValue;
}
DEVILUTION_END_NAMESPACE
