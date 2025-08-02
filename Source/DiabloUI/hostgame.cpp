#ifndef NOHOSTING
#include "diabloui.h"
//#include "all.h"
#include "DiabloUI/diablo.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

DISABLE_SPEED_OPTIMIZATION

#define PROGRESS_CANCEL 101

static UiProgressBar* _gbHostPb;

static void HostGameCancel()
{
	_gbHostPb->m_Progress = PROGRESS_CANCEL;
}

static void HostGameEsc()
{
	if (!gbTalkflag) {
		_gbHostPb->m_Progress = PROGRESS_CANCEL;
	}
}

static void HostGameOverlay()
{
	DrawPlrMsg(gbTalkflag);
}

static void HostGameLoad()
{
	int y;

	LoadBackgroundArt(NULL, "ui_art\\menu.pal");

	// UiAddBackground();

	// y = SCREEN_MIDY(SMALL_POPUP_HEIGHT);
	y = SCREEN_HEIGHT - SMALL_POPUP_HEIGHT;

	SDL_Rect rect0 = { SCREEN_MIDX(SMALL_POPUP_WIDTH), y, 0, 0 };
	_gbHostPb = new UiProgressBar(rect0);
	gUiItems.push_back(_gbHostPb);
	SDL_Rect rect01 = { 0, 0, 0, 0 };
	gUiItems.push_back(new UiCustom(HostGameOverlay, rect01));
	SDL_Rect rect1 = { 0, y + 20, SCREEN_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiText("...Server is running...", rect1, UIS_HCENTER | UIS_SMALL | UIS_GOLD));
	SDL_Rect rect2 = { SCREEN_MIDX(SML_BUTTON_WIDTH), y + 97, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiButton("Cancel", &HostGameCancel, rect2));

	UiInitScreen(0, NULL, NULL, HostGameEsc);
}

static void HostGameFree()
{
	// FreeBackgroundArt();
	UiClearItems();
}

void UiHostGameDialog()
{
	HostGameLoad();

	Dvl_Event event;
	do {
		_gbHostPb->m_Progress = multi_ui_handle_turn();
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
	} while (_gbHostPb->m_Progress < 100);
	HostGameFree();
}

ENABLE_SPEED_OPTIMIZATION

DEVILUTION_END_NAMESPACE
#endif // !NOHOSTING