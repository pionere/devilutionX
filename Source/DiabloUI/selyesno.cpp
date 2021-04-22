#include "selyesno.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "control.h"

DEVILUTION_BEGIN_NAMESPACE

bool selyesno_endMenu;
bool selyesno_value;
char selyesno_confirmationMessage[256];

std::vector<UiListItem *> vecSelYesNoDialogItems;
std::vector<UiItemBase *> vecSelYesNoDialog;

#define MESSAGE_WIDTH 280

void SelyesnoFree()
{
	ArtBackground.Unload();

	for (auto pUIListItem : vecSelYesNoDialogItems) {
		delete pUIListItem;
	}
	vecSelYesNoDialogItems.clear();

	for (auto pUIItem : vecSelYesNoDialog) {
		delete pUIItem;
	}
	vecSelYesNoDialog.clear();
}

void SelyesnoSelect(std::size_t index)
{
	selyesno_value = vecSelYesNoDialogItems[index]->m_value == 0;
	selyesno_endMenu = true;
}

void SelyesnoEsc()
{
	selyesno_value = false;
	selyesno_endMenu = true;
}

bool UiSelHeroYesNoDialog(const char *title, const char *body)
{
	LoadBackgroundArt("ui_art\\black.pcx");
	UiAddBackground(&vecSelYesNoDialog);
	UiAddLogo(&vecSelYesNoDialog);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelYesNoDialog.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 120, (UI_OFFSET_Y + 236), MESSAGE_WIDTH, 168 };
	vecSelYesNoDialog.push_back(new UiArtText(selyesno_confirmationMessage, rect2, UIS_MED));

	vecSelYesNoDialogItems.push_back(new UiListItem("Yes", 0));
	vecSelYesNoDialogItems.push_back(new UiListItem("No", 1));
	vecSelYesNoDialog.push_back(new UiList(vecSelYesNoDialogItems, PANEL_LEFT + 230, (UI_OFFSET_Y + 390), 180, 35, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(selyesno_confirmationMessage, body, sizeof(selyesno_confirmationMessage));
	WordWrapArtStr(selyesno_confirmationMessage, MESSAGE_WIDTH);

	UiInitList(vecSelYesNoDialog, vecSelYesNoDialogItems.size(), NULL, SelyesnoSelect, SelyesnoEsc, NULL, true);

	selyesno_value = true;
	selyesno_endMenu = false;
	while (!selyesno_endMenu) {
		UiClearScreen();
		UiRenderItems(vecSelYesNoDialog);
		UiPollAndRender();
	}

	SelyesnoFree();

	return selyesno_value;
}
DEVILUTION_END_NAMESPACE
