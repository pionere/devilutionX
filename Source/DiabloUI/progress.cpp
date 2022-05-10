
#include "controls/menu_controls.h"
#include "DiabloUI/diabloui.h"
#include "all.h"
//#include "../dx.h"
//#include "../engine.h"
//#include "../engine/render/cel_render.hpp"
//#include "../palette.h"

DEVILUTION_BEGIN_NAMESPACE

#define PROGRESS_CANCEL	101

#define PRBAR_WIDTH		228
#define PRBAR_HEIGHT	38

static CelImageBuf* gbProgBackCel;
static CelImageBuf* gbProgEmptyCel;
static BYTE* gbProgFillBmp;
static int _gnProgress;

static void DialogActionCancel()
{
	_gnProgress = PROGRESS_CANCEL;
}

static void ProgressLoad(const char *msg)
{
	CelImageBuf* gbProgFillCel;
	int i, y;

	LoadBackgroundArt("ui_art\\black.CEL", "ui_art\\menu.pal");
	gbSmlButtonCel = CelLoadImage("ui_art\\smbutton.CEL", SML_BUTTON_WIDTH);
	gbProgBackCel = CelLoadImage("ui_art\\spopup.CEL", SMALL_POPUP_WIDTH);
	gbProgEmptyCel = CelLoadImage("ui_art\\prog_bg.CEL", PRBAR_WIDTH);

	gbProgFillBmp = DiabloAllocPtr(PRBAR_HEIGHT * PRBAR_WIDTH);
	gbProgFillCel = CelLoadImage("ui_art\\prog_fil.CEL", PRBAR_WIDTH);
	CelDraw(SCREEN_X, SCREEN_Y + PRBAR_HEIGHT - 1, gbProgFillCel, 1);
	for (i = 0; i < PRBAR_HEIGHT; i++) {
		memcpy(&gbProgFillBmp[0 + i * PRBAR_WIDTH], &gpBuffer[SCREEN_X + (SCREEN_Y + i) * BUFFER_WIDTH], PRBAR_WIDTH);
	}
	MemFreeDbg(gbProgFillCel);

	y = PANEL_TOP + (PANEL_HEIGHT - SMALL_POPUP_HEIGHT) / 2;

	SDL_Rect rect1 = { PANEL_LEFT, y + 20, PANEL_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiText(msg, rect1, UIS_CENTER | UIS_SMALL | UIS_GOLD));
	SDL_Rect rect2 = { PANEL_LEFT + (PANEL_WIDTH - SML_BUTTON_WIDTH) / 2, y + 97, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiButton("Cancel", &DialogActionCancel, rect2));
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

static void ProgressRender()
{
	int x, y, i, dx;

	CelDraw(PANEL_X, PANEL_Y + PANEL_HEIGHT - 1, gbBackCel, 1);

	x = PANEL_X + (PANEL_WIDTH - SMALL_POPUP_WIDTH) / 2;
	y = PANEL_Y + (PANEL_HEIGHT - SMALL_POPUP_HEIGHT) / 2;

	CelDraw(x, y + SMALL_POPUP_HEIGHT, gbProgBackCel, 1);
	x += (SMALL_POPUP_WIDTH - PRBAR_WIDTH) / 2;
	y += 46 + PRBAR_HEIGHT;
	CelDraw(x, y - 1, gbProgEmptyCel, 1);
	dx = _gnProgress;
	if (dx > 100)
		dx = 100;
	dx = PRBAR_WIDTH * dx / 100;
	for (i = 0; i < PRBAR_HEIGHT && dx != 0; i++) {
		memcpy(&gpBuffer[x + (y + i - PRBAR_HEIGHT) * BUFFER_WIDTH], &gbProgFillBmp[0 + i * PRBAR_WIDTH], dx);
	}
}

bool UiProgressDialog(const char *msg, int (*fnfunc)())
{
	ProgressLoad(msg);
	SetFadeLevel(256);

	SDL_Event event;
	do {
		_gnProgress = fnfunc();
		UiClearScreen();
		ProgressRender();
		UiRenderItems(gUiItems);
		UiFadeIn(true);

		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				UiItemMouseEvents(&event);
				break;
			default:
				switch (GetMenuAction(event)) {
				case MenuAction_BACK:
				case MenuAction_SELECT:
					_gnProgress = PROGRESS_CANCEL;
					break;
				default:
					break;
				}
				break;
			}
			UiHandleEvents(&event);
		}
	} while (_gnProgress < 100);
	ProgressFree();

	return _gnProgress == 100;
}

DEVILUTION_END_NAMESPACE
