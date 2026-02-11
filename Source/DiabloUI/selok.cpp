#include "selok.h"

#include "diabloui.h"
#include "text.h"
#include "all.h"
//#include "../gameui.h"
//#include "../engine.h"
//#include "storm/storm.h"

DEVILUTION_BEGIN_NAMESPACE

DISABLE_SPEED_OPTIMIZATION

static bool _gbSelokEndMenu;

#define MESSAGE_WIDTH (PANEL_WIDTH - 2 * 60)

static void SelokFree()
{
	// FreeBackgroundArt();

	UiClearListItems();

	UiClearItems();
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
		LoadBackgroundArt(NULL, "ui_art\\menu.pal");
	//} else {
	//	LoadBackgroundArt(background);
	//}

	// UiAddBackground();
	UiAddLogo();

	//if (title != NULL) {
		SDL_Rect rect1 = { 0, SELYNOK_TITLE_TOP, SCREEN_WIDTH, 35 };
		gUiItems.push_back(new UiText(title, rect1, UIS_HCENTER | UIS_BIG | UIS_SILVER));

		SDL_Rect rect2 = { SCREEN_MIDX(MESSAGE_WIDTH), SELYNOK_CONTENT_TOP, MESSAGE_WIDTH, SELYNOK_CONTENT_HEIGHT };
		gUiItems.push_back(new UiText(dialogText, rect2, UIS_LEFT | UIS_MED | UIS_SILVER));
	//} else {
	//	SDL_Rect rect1 = { SCREEN_MIDX(MESSAGE_WIDTH), SCREEN_MIDY(86), MESSAGE_WIDTH, SELYNOK_CONTENT_HEIGHT };
	//	gUiItems.push_back(new UiText(dialogText, rect1, UIS_LEFT | UIS_MED | UIS_SILVER));
	//}

	gUIListItems.push_back(new UiListItem("OK", 0));
	SDL_Rect rect3 = { SCREEN_MIDX(180), SELYNOK_BUTTON_TOP, 180, 35 * 1 };
	gUiItems.push_back(new UiList(&gUIListItems, 1, rect3, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SStrCopy(dialogText, body, sizeof(dialogText));
	WordWrapArtStr(dialogText, MESSAGE_WIDTH, (unsigned)UIS_MED >> 0);

	UiInitScreen(0, NULL, SelokSelect, SelokEsc);

	_gbSelokEndMenu = false;
	do {
		UiRenderAndPoll();
	} while (!_gbSelokEndMenu);

	SelokFree();
}

ENABLE_SPEED_OPTIMIZATION

DEVILUTION_END_NAMESPACE
