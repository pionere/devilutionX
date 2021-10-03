#include "selok.h"

#include "all.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

bool _gbSelokEndMenu;

std::vector<UiListItem *> vecSelOkDialogItems;
std::vector<UiItemBase *> vecSelOkDialog;

#define MESSAGE_WIDTH 280

static void SelokFree()
{
	ArtBackground.Unload();

	UiClearListItems(vecSelOkDialogItems);

	UiClearItems(vecSelOkDialog);
}

static void SelokSelect(unsigned index)
{
	_gbSelokEndMenu = true;
}

void SelokEsc()
{
	_gbSelokEndMenu = true;
}

void UiSelOkDialog(const char* title, const char* body)
{
	char dialogText[256];

	//if (!background) {
		LoadBackgroundArt("ui_art\\black.pcx");
	//} else {
	//	LoadBackgroundArt(MENU_ART);
	//}

	UiAddBackground(&vecSelOkDialog);
	UiAddLogo(&vecSelOkDialog);

	//if (title != NULL) {
		SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
		vecSelOkDialog.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG));

		SDL_Rect rect2 = { PANEL_LEFT + 140, (UI_OFFSET_Y + 210), 560, 168 };
		vecSelOkDialog.push_back(new UiArtText(dialogText, rect2, UIS_MED));
	//} else {
	//	SDL_Rect rect1 = { PANEL_LEFT + 140, (UI_OFFSET_Y + 197), 560, 168 };
	//	vecSelOkDialog.push_back(new UiArtText(dialogText, rect1, UIS_MED));
	//}

	vecSelOkDialogItems.push_back(new UiListItem("OK", 0));
	SDL_Rect rect3 = { PANEL_LEFT + 230, (UI_OFFSET_Y + 390), 180, 35 };
	vecSelOkDialog.push_back(new UiList(&vecSelOkDialogItems, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(dialogText, body, sizeof(dialogText));
	WordWrapArtStr(dialogText, MESSAGE_WIDTH);

	UiInitList(vecSelOkDialog, 0, NULL, SelokSelect, SelokEsc);

	_gbSelokEndMenu = false;
	while (!_gbSelokEndMenu) {
		UiClearScreen();
		UiRenderItems(vecSelOkDialog);
		UiPollAndRender();
	}

	SelokFree();
}
DEVILUTION_END_NAMESPACE
