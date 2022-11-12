
#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define PROGRESS_CANCEL	101

static CelImageBuf* gbProgBackCel;
static CelImageBuf* gbProgEmptyCel;
static BYTE* gbProgFillBmp;
static int _gnProgress;

static void ProgressEsc()
{
	_gnProgress = PROGRESS_CANCEL;
}

static void ProgressSelect(unsigned index)
{
	_gnProgress = PROGRESS_CANCEL;
}

static void ProgressRender()
{
	int x, y, i, dx;

	x = PANEL_CENTERX(SMALL_POPUP_WIDTH);
	y = PANEL_CENTERY(SMALL_POPUP_HEIGHT);
	// draw the popup window
	CelDraw(x, y + SMALL_POPUP_HEIGHT, gbProgBackCel, 1);
	x += (SMALL_POPUP_WIDTH - PRBAR_WIDTH) / 2;
	y += 46 + PRBAR_HEIGHT;
	// draw the frame of the progress bar
	CelDraw(x, y - 1, gbProgEmptyCel, 1);
	// draw the progress bar
	dx = _gnProgress;
	if (dx > 100)
		dx = 100;
	dx = PRBAR_WIDTH * dx / 100;
	for (i = 0; i < PRBAR_HEIGHT && dx != 0; i++) {
		memcpy(&gpBuffer[x + (y + i - PRBAR_HEIGHT) * BUFFER_WIDTH], &gbProgFillBmp[0 + i * PRBAR_WIDTH], dx);
	}
}

static void ProgressLoad(const char* msg)
{
	CelImageBuf* progFillCel;
	int i, y;

	LoadBackgroundArt("ui_art\\black.CEL", "ui_art\\menu.pal");

	gbSmlButtonCel = CelLoadImage("ui_art\\smbutton.CEL", SML_BUTTON_WIDTH);
	gbProgBackCel = CelLoadImage("ui_art\\spopup.CEL", SMALL_POPUP_WIDTH);
	gbProgEmptyCel = CelLoadImage("ui_art\\prog_bg.CEL", PRBAR_WIDTH);

	gbProgFillBmp = DiabloAllocPtr(PRBAR_HEIGHT * PRBAR_WIDTH);
	progFillCel = CelLoadImage("ui_art\\prog_fil.CEL", PRBAR_WIDTH);
	CelDraw(PANEL_X, PANEL_Y + PRBAR_HEIGHT - 1, progFillCel, 1);
	for (i = 0; i < PRBAR_HEIGHT; i++) {
		memcpy(&gbProgFillBmp[0 + i * PRBAR_WIDTH], &gpBuffer[PANEL_X + (PANEL_Y + i) * BUFFER_WIDTH], PRBAR_WIDTH);
	}
	mem_free_dbg(progFillCel);

	UiAddBackground(&gUiItems);

	y = PANEL_MIDY(SMALL_POPUP_HEIGHT);

	SDL_Rect rect0 = { 0, 0, 0, 0 };
	gUiItems.push_back(new UiCustom(ProgressRender, rect0));
	SDL_Rect rect1 = { PANEL_LEFT, y + 20, PANEL_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiText(msg, rect1, UIS_CENTER | UIS_SMALL | UIS_GOLD));
	SDL_Rect rect2 = { PANEL_MIDX(SML_BUTTON_WIDTH), y + 97, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiButton("Cancel", &ProgressEsc, rect2));

	UiInitScreen(0, NULL, ProgressSelect, ProgressEsc);
}

static void ProgressFree()
{
	MemFreeDbg(gbBackCel);
	MemFreeDbg(gbSmlButtonCel);
	MemFreeDbg(gbProgBackCel);
	MemFreeDbg(gbProgEmptyCel);
	MemFreeDbg(gbProgFillBmp);

	UiClearItems(gUiItems);
}

bool UiProgressDialog(const char* msg, int (*fnfunc)())
{
	ProgressLoad(msg);
	SetFadeLevel(256);

	do {
		_gnProgress = fnfunc();
		UiRenderAndPoll(NULL);
	} while (_gnProgress < 100);
	ProgressFree();

	return _gnProgress == 100;
}

DEVILUTION_END_NAMESPACE
