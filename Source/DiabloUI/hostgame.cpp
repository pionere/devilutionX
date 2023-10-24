#ifndef NOHOSTING
#include "diabloui.h"
//#include "all.h"
#include "DiabloUI/diablo.h"
#include "engine/render/cel_render.h"

DEVILUTION_BEGIN_NAMESPACE

#define PROGRESS_CANCEL 101
// #define PROGRESSBAR_TOP PLRMSG_TEXT_BOTTOM
#define PROGRESSBAR_TOP (SCREEN_Y + PANEL_BOTTOM - SMALL_POPUP_HEIGHT)

static CelImageBuf* _gbProgBackCel;
static CelImageBuf* _gbProgEmptyCel;
static BYTE* _gbProgFillBmp;
static int _gnProgress;

static void HostGameCancel()
{
	_gnProgress = PROGRESS_CANCEL;
}

static void HostGameEsc()
{
	if (gbTalkflag) {
		gbTalkflag = false;
	} else {
		_gnProgress = PROGRESS_CANCEL;
	}
}

static void HostGameRender()
{
	int x, y, i, dx;

	x = PANEL_CENTERX(SMALL_POPUP_WIDTH);
	// y = PANEL_CENTERY(SMALL_POPUP_HEIGHT);
	y = PROGRESSBAR_TOP;
	// draw the popup window
	CelDraw(x, y + SMALL_POPUP_HEIGHT, _gbProgBackCel, 1);
	x += (SMALL_POPUP_WIDTH - PRBAR_WIDTH) / 2;
	y += 46 + PRBAR_HEIGHT;
	// draw the frame of the progress bar
	CelDraw(x, y - 1, _gbProgEmptyCel, 1);
	// draw the progress bar
	dx = _gnProgress;
	if (dx > 100)
		dx = 100;
	dx = PRBAR_WIDTH * dx / 100;
	for (i = 0; i < PRBAR_HEIGHT && dx != 0; i++) {
		memcpy(&gpBuffer[x + (y + i - PRBAR_HEIGHT) * BUFFER_WIDTH], &_gbProgFillBmp[0 + i * PRBAR_WIDTH], dx);
	}

	DrawPlrMsg(gbTalkflag);
}

static void HostGameLoad()
{
	CelImageBuf* progFillCel;
	int i, y;

	LoadBackgroundArt("ui_art\\black.CEL", "ui_art\\menu.pal");

	gbSmlButtonCel = CelLoadImage("ui_art\\smbutton.CEL", SML_BUTTON_WIDTH);
	_gbProgBackCel = CelLoadImage("ui_art\\spopup.CEL", SMALL_POPUP_WIDTH);
	_gbProgEmptyCel = CelLoadImage("ui_art\\prog_bg.CEL", PRBAR_WIDTH);

	_gbProgFillBmp = DiabloAllocPtr(PRBAR_HEIGHT * PRBAR_WIDTH);
	progFillCel = CelLoadImage("ui_art\\prog_fil.CEL", PRBAR_WIDTH);
	CelDraw(PANEL_X, PANEL_Y + PRBAR_HEIGHT - 1, progFillCel, 1);
	for (i = 0; i < PRBAR_HEIGHT; i++) {
		memcpy(&_gbProgFillBmp[0 + i * PRBAR_WIDTH], &gpBuffer[PANEL_X + (PANEL_Y + i) * BUFFER_WIDTH], PRBAR_WIDTH);
	}
	mem_free_dbg(progFillCel);

	UiAddBackground();

	// y = PANEL_MIDY(SMALL_POPUP_HEIGHT);
	y = PROGRESSBAR_TOP - SCREEN_Y;

	SDL_Rect rect0 = { 0, 0, 0, 0 };
	gUiItems.push_back(new UiCustom(HostGameRender, rect0));
	SDL_Rect rect1 = { PANEL_LEFT, y + 20, PANEL_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiText("...Server is running...", rect1, UIS_HCENTER | UIS_SMALL | UIS_GOLD));
	SDL_Rect rect2 = { PANEL_MIDX(SML_BUTTON_WIDTH), y + 97, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiButton("Cancel", &HostGameCancel, rect2));

	UiInitScreen(0, NULL, NULL, HostGameEsc);
}

static void HostGameFree()
{
	FreeBackgroundArt();
	MemFreeDbg(gbSmlButtonCel);
	MemFreeDbg(_gbProgBackCel);
	MemFreeDbg(_gbProgEmptyCel);
	MemFreeDbg(_gbProgFillBmp);

	UiClearItems();
}

void UiHostGameDialog()
{
	HostGameLoad();
	SetFadeLevel(FADE_LEVELS);

	Dvl_Event event;
	do {
		_gnProgress = multi_ui_handle_turn();
		UiRender();
		while (UiPeekAndHandleEvents(&event)) {
			if (gbTalkflag) {
				switch (event.type) {
				case DVL_WM_MOUSEMOVE:
					plrmsg_HandleMouseMoveEvent();
					break;
				case DVL_WM_LBUTTONDOWN:
					plrmsg_presskey(DVL_VK_LBUTTON);
					break;
				case DVL_WM_LBUTTONUP:
					plrmsg_HandleMouseReleaseEvent();
					break;
#ifndef USE_SDL1
				case DVL_WM_TEXT:
					plrmsg_CatToText(event.text.text);
					break;
#endif
				case DVL_WM_KEYDOWN:
					plrmsg_presskey(event.vkcode);
					break;
				}
			} else if (event.type == DVL_WM_KEYDOWN) {
				if (event.vkcode == DVL_VK_RETURN)
					StartPlrMsg();
				else if (event.vkcode == DVL_VK_V) // TODO: WMButtonInputTransTbl ?
					VersionPlrMsg();
			}
		}
	} while (_gnProgress < 100);
	HostGameFree();
}

DEVILUTION_END_NAMESPACE
#endif // !NOHOSTING