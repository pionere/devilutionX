#include "selok.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

bool _gbSelokEndMenu;

std::vector<UiListItem *> vecSelOkDialogItems;
std::vector<UiItemBase *> vecSelOkDialog;

#define MESSAGE_WIDTH 280

void selok_Free()
{
	ArtBackground.Unload();

	UiClearListItems(vecSelOkDialogItems);

	UiClearItems(vecSelOkDialog);
}

void selok_Select(unsigned index)
{
	_gbSelokEndMenu = true;
}

void selok_Esc()
{
	_gbSelokEndMenu = true;
}

void UiSelOkDialog(const char *title, const char *body, bool background)
{
	char dialogText[256];

	if (!background) {
		LoadBackgroundArt("ui_art\\black.pcx");
	} else {
		LoadBackgroundArt(MENU_ART);
	}

	UiAddBackground(&vecSelOkDialog);
	UiAddLogo(&vecSelOkDialog);

	if (title != NULL) {
		SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
		vecSelOkDialog.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG));

		SDL_Rect rect2 = { PANEL_LEFT + 140, (UI_OFFSET_Y + 210), 560, 168 };
		vecSelOkDialog.push_back(new UiArtText(dialogText, rect2, UIS_MED));
	} else {
		SDL_Rect rect1 = { PANEL_LEFT + 140, (UI_OFFSET_Y + 197), 560, 168 };
		vecSelOkDialog.push_back(new UiArtText(dialogText, rect1, UIS_MED));
	}

	vecSelOkDialogItems.push_back(new UiListItem("OK", 0));
	SDL_Rect rect1 = { PANEL_LEFT + 230, (UI_OFFSET_Y + 390), 180, 35 };
	vecSelOkDialog.push_back(new UiList(&vecSelOkDialogItems, rect1, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(dialogText, body, sizeof(dialogText));
	WordWrapArtStr(dialogText, MESSAGE_WIDTH);

	UiInitList(vecSelOkDialog, 0, NULL, selok_Select, selok_Esc);

	_gbSelokEndMenu = false;
	while (!_gbSelokEndMenu) {
		UiClearScreen();
		UiRenderItems(vecSelOkDialog);
		UiPollAndRender();
	}

	selok_Free();
}
DEVILUTION_END_NAMESPACE
