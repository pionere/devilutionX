#include "DiabloUI/diabloui.h"
#include "all.h"
//#include "../gameui.h"
//#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

#define TITLE_TIMEOUT_SEC 7
static CelImageBuf* gbLogoBig;
static bool _gbTitleEnd;

static void TitleEsc()
{
	_gbTitleEnd = true;
}

static void TitleSelect(unsigned index)
{
	_gbTitleEnd = true;
}

static void TitleLoad()
{
	LoadBackgroundArt("ui_art\\title.CEL", "ui_art\\menu.pal");
	// assert(gbLogoBig == NULL);
	gbLogoBig = CelLoadImage("ui_art\\logo.CEL", BIG_LOGO_WIDTH);

	UiAddBackground();

	SDL_Rect rect1 = { SCREEN_MIDX(BIG_LOGO_WIDTH), BIG_LOGO_TOP, BIG_LOGO_WIDTH, BIG_LOGO_HEIGHT };
	gUiItems.push_back(new UiImage(gbLogoBig, 15, rect1, true));

	UiInitScreen(0, NULL, TitleSelect, TitleEsc);
	gUiDrawCursor = false;
}

static void TitleFree()
{
	FreeBackgroundArt();
	MemFreeDbg(gbLogoBig);
	UiClearItems();
}

bool UiTitleDialog()
{
	TitleLoad();

	Uint32 timeOut = SDL_GetTicks() + TITLE_TIMEOUT_SEC * 1000;

	do {
		UiRenderAndPoll();
	} while (!_gbTitleEnd && SDL_GetTicks() < timeOut);

	TitleFree();

	return true;
}

DEVILUTION_END_NAMESPACE
