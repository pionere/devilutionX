
#include "all.h"

#include "controls/menu_controls.h"
#include "DiabloUI/art_draw.h"
#include "DiabloUI/button.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/fonts.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

#define PROGRESS_CANCEL	101

#define PRPANEL_WIDTH	280
#define PRPANEL_HEIGHT	140

static Art ArtPopupSm;
static Art ArtProgBG;
static Art ArtProgFil;
static SDL_Surface *msgSurface;
static SDL_Surface *msgShadow;
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
	LoadSmlButtonArt();
	LoadTtfFont();

	if (font != NULL) {
		SDL_Color color = { 243, 243, 243, 0 };
		SDL_Color black = { 0, 0, 0, 0 };

		msgSurface = TTF_RenderUTF8_Solid(font, msg, color);
		msgShadow = TTF_RenderUTF8_Solid(font, msg, black);
	}
	SDL_Rect rect3 = { PANEL_LEFT + 265, UI_OFFSET_Y + 267, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	vecProgress.push_back(new UiButton(&SmlButton, "Cancel", &DialogActionCancel, rect3));
}

static void ProgressFree()
{
	ArtBackground.Unload();
	ArtPopupSm.Unload();
	ArtProgBG.Unload();
	ArtProgFil.Unload();
	UnloadSmlButtonArt();
	SDL_FreeSurface(msgSurface);
	msgSurface = NULL;
	SDL_FreeSurface(msgShadow);
	msgShadow = NULL;
	UnloadTtfFont();
	UiClearItems(vecProgress);
}

static void ProgressRender()
{
	SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
	DrawArt(0, 0, &ArtBackground);

	int x = PANEL_LEFT + (PANEL_WIDTH - PRPANEL_WIDTH) / 2;
	int y = UI_OFFSET_Y + (480 - PRPANEL_HEIGHT) / 2;

	DrawArt(x, y, &ArtPopupSm, 0, PRPANEL_WIDTH, PRPANEL_HEIGHT);
	DrawArt(x + (PRPANEL_WIDTH - 227) / 2, y + 52, &ArtProgBG, 0, 227);
	if (_gnProgress != 0) {
		DrawArt(x + (PRPANEL_WIDTH - 227) / 2, y + 52, &ArtProgFil, 0, 227 * _gnProgress / 100);
	}
	DrawArt(x + (PRPANEL_WIDTH - SML_BUTTON_WIDTH) / 2, y + 99, &SmlButton, 2, SML_BUTTON_WIDTH);

	if (msgSurface != NULL) {
		SDL_Rect dscRect = {
			x + 50 + 1,
			y + 8 + 1,
			msgSurface->w,
			msgSurface->h
		};
		Blit(msgShadow, NULL, &dscRect);
		dscRect.x -= 1;
		dscRect.y -= 1;
		Blit(msgSurface, NULL, &dscRect);
	}
}

bool UiProgressDialog(const char *msg, int (*fnfunc)())
{
	ProgressLoad(msg);
	SetFadeLevel(256);

	_gnProgress = 0;

	SDL_Event event;
	while (_gnProgress < 100) {
		_gnProgress = fnfunc();
		ProgressRender();
		UiRenderItems(vecProgress);
		DrawMouse();
		RenderPresent();

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
	}
	ProgressFree();

	return _gnProgress == 100;
}

DEVILUTION_END_NAMESPACE
