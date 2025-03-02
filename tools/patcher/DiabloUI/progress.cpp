
#include "diabloui.h"
//#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define PROGRESS_CANCEL 101

static UiProgressBar* _gbPb;

static void ProgressEsc()
{
	_gbPb->m_Progress = PROGRESS_CANCEL;
}

static void ProgressSelect(unsigned index)
{
	_gbPb->m_Progress = PROGRESS_CANCEL;
}

static void ProgressLoad(const char* msg)
{
	int y;

	LoadBackgroundArt("ui_art\\black.CEL", "ui_art\\menu.pal");

	UiAddBackground();

	y = PANEL_MIDY(SMALL_POPUP_HEIGHT);

	SDL_Rect rect0 = { PANEL_MIDX(SMALL_POPUP_WIDTH), y, 0, 0 };
	_gbPb = new UiProgressBar(rect0);
	gUiItems.push_back(_gbPb);
	SDL_Rect rect1 = { PANEL_LEFT, y + 20, PANEL_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiText(msg, rect1, UIS_HCENTER | UIS_SMALL | UIS_GOLD));
	SDL_Rect rect2 = { PANEL_MIDX(SML_BUTTON_WIDTH), y + 97, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	gUiItems.push_back(new UiButton("Cancel", &ProgressEsc, rect2));

	UiInitScreen(0, NULL, ProgressSelect, ProgressEsc);
}

static void ProgressFree()
{
	FreeBackgroundArt();
	UiClearItems();
}

bool UiProgressDialog(const char* msg, int (*fnfunc)())
{
	bool result;

	ProgressLoad(msg);

	do {
		_gbPb->m_Progress = fnfunc();
		UiRenderAndPoll();
	} while (_gbPb->m_Progress < 100);
	result = _gbPb->m_Progress == 100;
	ProgressFree();

	return result;
}

DEVILUTION_END_NAMESPACE
