
#include "controls/menu_controls.h"
#include "DiabloUI/art_draw.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/fonts.h"
#include "utils/display.h"
#include "../palette.h"

DEVILUTION_BEGIN_NAMESPACE

#define PROGRESS_CANCEL	101

#define PRPANEL_WIDTH	280
#define PRPANEL_HEIGHT	140

static Art ArtPopupSm;
static Art ArtProgBG;
static Art ArtProgFil;
static std::vector<UiItemBase *> vecProgress;
static int _gnProgress;

static void DialogActionCancel()
{
	_gnProgress = PROGRESS_CANCEL;
}

static void ProgressLoad(const char *msg)
{
	LoadBackgroundArt("ui_art\\black.pcx");
	LoadArt("ui_art\\spopup.pcx", &ArtPopupSm);
	LoadArt("ui_art\\prog_bg.pcx", &ArtProgBG);
	LoadArt("ui_art\\prog_fil.pcx", &ArtProgFil);
	LoadArt("ui_art\\smbutton.pcx", &SmlButton, 2);

	int x = PANEL_LEFT + (PANEL_WIDTH - PRPANEL_WIDTH) / 2;
	int y = UI_OFFSET_Y + (480 - PRPANEL_HEIGHT) / 2;

	SDL_Rect rect1 = { x + 50, y + 20, PRPANEL_WIDTH - 100, SML_BUTTON_HEIGHT };
	vecProgress.push_back(new UiArtText(msg, rect1, UIS_CENTER | UIS_SMALL | UIS_GOLD));
	SDL_Rect rect2 = { x + 85, y + 97, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	vecProgress.push_back(new UiButton("Cancel", &DialogActionCancel, rect2));
}

static void ProgressFree()
{
	ArtBackground.Unload();
	ArtPopupSm.Unload();
	ArtProgBG.Unload();
	ArtProgFil.Unload();
	SmlButton.Unload();
	UiClearItems(vecProgress);
}

static void ProgressRender()
{
	UiClearScreen();
	DrawArt(0, 0, &ArtBackground);

	int x = PANEL_LEFT + (PANEL_WIDTH - PRPANEL_WIDTH) / 2;
	int y = UI_OFFSET_Y + (480 - PRPANEL_HEIGHT) / 2;

	DrawArt(x, y, &ArtPopupSm, 0, PRPANEL_WIDTH, PRPANEL_HEIGHT);
	DrawArt(x + (PRPANEL_WIDTH - 227) / 2, y + 52, &ArtProgBG, 0, 227);
	if (_gnProgress != 0) {
		DrawArt(x + (PRPANEL_WIDTH - 227) / 2, y + 52, &ArtProgFil, 0, 227 * _gnProgress / 100);
	}
	DrawArt(x + (PRPANEL_WIDTH - SML_BUTTON_WIDTH) / 2, y + 99, &SmlButton, 2, SML_BUTTON_WIDTH);
}

bool UiProgressDialog(const char *msg, int (*fnfunc)())
{
	ProgressLoad(msg);
	SetFadeLevel(256);

	SDL_Event event;
	do {
		_gnProgress = fnfunc();
		ProgressRender();
		UiRenderItems(vecProgress);
		DrawMouse();
		UiFadeIn();

		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				UiItemMouseEvents(&event, vecProgress);
				break;
#ifndef USE_SDL1
			case SDLK_KP_ENTER:
#endif
			case SDLK_ESCAPE:
			case SDLK_RETURN:
			case SDLK_SPACE:
				_gnProgress = PROGRESS_CANCEL;
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
