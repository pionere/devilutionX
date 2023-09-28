#include "selok.h"

#include "storm/storm.h"

#include "diabloui.h"
#include "text.h"
#include "../gameui.h"
#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbSelokEndMenu;

#define MESSAGE_WIDTH (PANEL_WIDTH - 2 * 60)

static void SelokFree()
{
	FreeBackgroundArt();

	UiClearListItems();

	UiClearItems(gUiItems);
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
		LoadBackgroundArt("ui_art\\black.CEL", "ui_art\\menu.pal");
	//} else {
	//	LoadBackgroundArt(background);
	//}

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	//if (title != NULL) {
		SDL_Rect rect1 = { PANEL_LEFT + 0, SELYNOK_TITLE_TOP, PANEL_WIDTH, 35 };
		gUiItems.push_back(new UiText(title, rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

		SDL_Rect rect2 = { PANEL_LEFT + 60, SELYNOK_CONTENT_TOP, MESSAGE_WIDTH, SELYNOK_CONTENT_HEIGHT };
		gUiItems.push_back(new UiText(dialogText, rect2, UIS_LEFT | UIS_MED | UIS_SILVER));
	//} else {
	//	SDL_Rect rect1 = { PANEL_LEFT + 140, (PANEL_TOP + 197), MESSAGE_WIDTH, 168 };
	//	gUiItems.push_back(new UiText(dialogText, rect1, UIS_LEFT | UIS_MED | UIS_SILVER));
	//}

	gUIListItems.push_back(new UiListItem("OK", 0));
	SDL_Rect rect3 = { PANEL_MIDX(180), SELYNOK_BUTTON_TOP, 180, 35 * 1 };
	gUiItems.push_back(new UiList(&gUIListItems, 1, rect3, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(dialogText, body, sizeof(dialogText));
	WordWrapArtStr(dialogText, MESSAGE_WIDTH, AFT_MED);

	UiInitScreen(1, NULL, SelokSelect, SelokEsc);

	_gbSelokEndMenu = false;
	do {
		UiRenderAndPoll();
	} while (!_gbSelokEndMenu);

	SelokFree();
}
DEVILUTION_END_NAMESPACE
