#include "dialogs.h"

#include "diabloui.h"
//#include "errorart.h"
#include "text.h"
#include "all.h"
//#include "utils/log.h"
//#include "../palette.h"
//#include "../diablo.h"
//#include "../engine.h"
//#include "storm/storm.h"

DEVILUTION_BEGIN_NAMESPACE

static CelImageBuf* gbDialogBackCel;
static bool _gbDialogEnd;
static bool gbInDialog = false;

static void DialogEsc()
{
	_gbDialogEnd = true;
}

static void DialogSelect(unsigned index)
{
	_gbDialogEnd = true;
}

/*
// clang-format off
#define BLANKCOLOR { 0, 0xFF, 0, 0 }
// clang-format on

static void LoadFallbackPalette()
{
	// clang-format off
	static const SDL_Color FallbackPalette[NUM_COLORS] = {
		{ 0x00, 0x00, 0x00, 0 },
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR,
		{ 0xff, 0xfd, 0x9f, 0 },
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		{ 0xe8, 0xca, 0xca, 0 },
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		{ 0x05, 0x07, 0x0c, 0 },
		{ 0xff, 0xe3, 0xa4, 0 },
		{ 0xee, 0xd1, 0x8c, 0 },
		{ 0xdd, 0xc4, 0x7e, 0 },
		{ 0xcc, 0xb7, 0x75, 0 },
		{ 0xbc, 0xa8, 0x6c, 0 },
		{ 0xab, 0x9a, 0x63, 0 },
		{ 0x98, 0x8b, 0x5d, 0 },
		{ 0x87, 0x7e, 0x54, 0 },
		{ 0x78, 0x6f, 0x49, 0 },
		{ 0x69, 0x60, 0x3f, 0 },
		{ 0x5b, 0x51, 0x34, 0 },
		{ 0x48, 0x40, 0x27, 0 },
		{ 0x39, 0x31, 0x1d, 0 },
		{ 0x31, 0x28, 0x16, 0 },
		{ 0x1a, 0x14, 0x08, 0 },
		{ 0x14, 0x0b, 0x00, 0 },
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR,
		{ 0xff, 0xbd, 0xbd, 0 },
		{ 0xf4, 0x96, 0x96, 0 },
		{ 0xe8, 0x7d, 0x7d, 0 },
		{ 0xe0, 0x6c, 0x6c, 0 },
		{ 0xd8, 0x5b, 0x5b, 0 },
		{ 0xcf, 0x49, 0x49, 0 },
		{ 0xc7, 0x38, 0x38, 0 },
		{ 0xbf, 0x27, 0x27, 0 },
		{ 0xa9, 0x22, 0x22, 0 },
		{ 0x93, 0x1e, 0x1e, 0 },
		{ 0x7c, 0x19, 0x19, 0 },
		{ 0x66, 0x15, 0x15, 0 },
		{ 0x4f, 0x11, 0x11, 0 },
		{ 0x39, 0x0d, 0x0d, 0 },
		{ 0x23, 0x09, 0x09, 0 },
		{ 0x0c, 0x05, 0x05, 0 },
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR,
		{ 0xf3, 0xf3, 0xf3, 0 },
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR,
		{ 0xff, 0xff, 0x00, 0 },
		BLANKCOLOR, BLANKCOLOR, BLANKCOLOR,
		BLANKCOLOR,
	};
	// clang-format on
	ApplyGamma(logical_palette, FallbackPalette);
}*/

static void Init(const char* caption, char* text, bool error/*, const std::vector<UiItemBase*>* renderBehind*/)
{
	//if (renderBehind == NULL) {
		UiClearListItems();
		UiClearItems();
		FreeBackgroundArt();

		LoadBackgroundArt("ui_art\\black.CEL", "ui_art\\menu.pal");
		UiAddBackground();
	//}

	/*if (caption == NULL) {
		gbDialogBackCel = CelLoadImage(error ? "ui_art\\srpopup.CEL" : "ui_art\\spopup.CEL", SMALL_POPUP_WIDTH);
		WordWrapArtStr(text, 240, AFT_SMALL);

		SDL_Rect rect1 = { PANEL_MIDX(SMALL_POPUP_WIDTH), (PANEL_TOP + 168), SMALL_POPUP_WIDTH, SMALL_POPUP_HEIGHT };
		gUiItems.push_back(new UiImage(gbDialogBackCel, 0, rect1, false));

		SDL_Rect rect2 = { PANEL_LEFT + 200, (PANEL_TOP + 211), 240, 80 };
		gUiItems.push_back(new UiText(text, rect2, UIS_LEFT | UIS_SMALL | UIS_GOLD));

		SDL_Rect rect3 = { PANEL_MIDX(SML_BUTTON_WIDTH), (PANEL_TOP + 265), SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
		gUiItems.push_back(new UiButton("OK", &DialogActionOK, rect3));
	} else {*/
		gbDialogBackCel = CelLoadImage(error ? "ui_art\\lrpopup.CEL" : "ui_art\\lpopup.CEL", LARGE_POPUP_WIDTH);
		WordWrapArtStr(text, LARGE_POPUP_TEXT_WIDTH, AFT_SMALL);

		SDL_Rect rect1 = { PANEL_MIDX(LARGE_POPUP_WIDTH), PANEL_MIDY(LARGE_POPUP_HEIGHT), LARGE_POPUP_WIDTH, LARGE_POPUP_HEIGHT };
		gUiItems.push_back(new UiImage(gbDialogBackCel, 0, rect1, false));

		SDL_Rect rect2 = { PANEL_LEFT + 0, PANEL_MIDY(LARGE_POPUP_HEIGHT) + 10, PANEL_WIDTH, 20 };
		gUiItems.push_back(new UiText(caption, rect2, UIS_HCENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect3 = { PANEL_MIDX(LARGE_POPUP_TEXT_WIDTH), PANEL_MIDY(LARGE_POPUP_HEIGHT) + 41, LARGE_POPUP_TEXT_WIDTH, LARGE_POPUP_HEIGHT - SML_BUTTON_HEIGHT - 17 - 17 };
		gUiItems.push_back(new UiText(text, rect3, UIS_LEFT | UIS_SMALL | UIS_GOLD));

		SDL_Rect rect4 = { PANEL_MIDX(SML_BUTTON_WIDTH), (PANEL_MIDY(LARGE_POPUP_HEIGHT) + LARGE_POPUP_HEIGHT - SML_BUTTON_HEIGHT - 17), SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
		gUiItems.push_back(new UiButton("OK", &DialogEsc, rect4));
	//}

	UiInitScreen(0, NULL, DialogSelect, DialogEsc);
}

static void Deinit(/*const std::vector<UiItemBase*>* renderBehind*/)
{
	//if (renderBehind == NULL) {
		FreeBackgroundArt();
	//}
	UiClearItems();
	MemFreeDbg(gbDialogBackCel);
}

static void DialogLoop(/*const std::vector<UiItemBase*>* renderBehind*/)
{
	_gbDialogEnd = false;
	do {
		UiRenderAndPoll();
	} while (!_gbDialogEnd);
}

static void UiOkDialog(const char* caption, char* text, bool error/*, const std::vector<UiItemBase*>* renderBehind*/)
{
	if (gbWndActive && gbWasUiInit && !gbInDialog) {
		gbInDialog = true;
		Init(caption, text, error/*, renderBehind*/);
		DialogLoop(/*renderBehind*/);
		Deinit(/*, renderBehind*/);
		gbInDialog = false;
		return;
	}

	SDL_ShowCursor(SDL_ENABLE);
#ifndef RUN_TESTS
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, caption, text, NULL) < 0) {
		DoLog(SDL_GetError());
#else
	{
#endif
		DoLog(caption);
		DoLog(text);
	}
}

void UiErrorOkDialog(const char* caption, char* text, bool error)
{
	UiOkDialog(caption, text, error/*, NULL*/);
}

/*void UiErrorOkDialog(const char* caption, const char* text, const std::vector<UiItemBase*>* renderBehind)
{
	UiOkDialog(caption, text, true, renderBehind);
}*/

DEVILUTION_END_NAMESPACE
