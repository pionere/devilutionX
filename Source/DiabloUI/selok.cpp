#include "selok.h"

#include "storm/storm.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

bool _gbSelokEndMenu;

std::vector<UiListItem *> vecSelOkDialogItems;
std::vector<UiItemBase *> vecSelOkDialog;

#define MESSAGE_WIDTH (PANEL_WIDTH - 2 * 60)

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
		SDL_Rect rect1 = { PANEL_LEFT + 0, (UI_OFFSET_Y + 161), PANEL_WIDTH, 35 };
		vecSelOkDialog.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

		SDL_Rect rect2 = { PANEL_LEFT + 60, (UI_OFFSET_Y + 236), MESSAGE_WIDTH, 168 };
		vecSelOkDialog.push_back(new UiArtText(dialogText, rect2, UIS_LEFT | UIS_MED | UIS_SILVER));
	//} else {
	//	SDL_Rect rect1 = { PANEL_LEFT + 140, (UI_OFFSET_Y + 197), MESSAGE_WIDTH, 168 };
	//	vecSelOkDialog.push_back(new UiArtText(dialogText, rect1, UIS_LEFT | UIS_MED | UIS_SILVER));
	//}

	vecSelOkDialogItems.push_back(new UiListItem("OK", 0));
	SDL_Rect rect3 = { PANEL_LEFT + 230, (UI_OFFSET_Y + 390), 180, 35 };
	vecSelOkDialog.push_back(new UiList(&vecSelOkDialogItems, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(dialogText, body, sizeof(dialogText));
	WordWrapArtStr(dialogText, MESSAGE_WIDTH, AFT_MED);

	UiInitList(vecSelOkDialog, 0, NULL, SelokSelect, SelokEsc);

	_gbSelokEndMenu = false;
	do {
		UiClearScreen();
		UiRenderItems(vecSelOkDialog);
		UiPollAndRender();
	} while (!_gbSelokEndMenu);

	SelokFree();
}
DEVILUTION_END_NAMESPACE
