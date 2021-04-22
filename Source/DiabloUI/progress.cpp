#include "DiabloUI/art_draw.h"
#include "DiabloUI/button.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/fonts.h"
#include "control.h"
#include "controls/menu_controls.h"
#include "dx.h"
#include "palette.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

Art dialogArt;
char dialogText[256];
Art progressArt;
Art ArtPopupSm;
Art ArtProgBG;
Art ProgFil;
SDL_Surface *msgSurface;
SDL_Surface *msgShadow;
std::vector<UiItemBase *> vecProgress;
bool endMenu;

void DialogActionCancel()
{
	endMenu = true;
}

void ProgressLoad(const char *msg)
{
	LoadBackgroundArt("ui_art\\black.pcx");
	LoadArt("ui_art\\spopup.pcx", &ArtPopupSm);
	LoadArt("ui_art\\prog_bg.pcx", &ArtProgBG);
	LoadArt("ui_art\\prog_fil.pcx", &ProgFil);
	LoadSmlButtonArt();
	LoadTtfFont();

	if (font != NULL) {
		SDL_Color color = { 243, 243, 243, 0 };
		SDL_Color black = { 0, 0, 0, 0 };

		msgSurface = TTF_RenderUTF8_Solid(font, msg, color);
		msgShadow = TTF_RenderUTF8_Solid(font, msg, black);
	}
	SDL_Rect rect3 = { (Sint16)(PANEL_LEFT + 265), (Sint16)(UI_OFFSET_Y + 267), SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT };
	vecProgress.push_back(new UiButton(&SmlButton, "Cancel", &DialogActionCancel, rect3, 0));
}

void ProgressFree()
{
	ArtBackground.Unload();
	ArtPopupSm.Unload();
	ArtProgBG.Unload();
	ProgFil.Unload();
	UnloadSmlButtonArt();
	SDL_FreeSurface(msgSurface);
	msgSurface = NULL;
	SDL_FreeSurface(msgShadow);
	msgShadow = NULL;
	UnloadTtfFont();
}

void ProgressRender(BYTE progress)
{
	SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
	DrawArt(0, 0, &ArtBackground);

	int x = GetCenterOffset(280);
	int y = GetCenterOffset(144, SCREEN_HEIGHT);

	DrawArt(x, y, &ArtPopupSm, 0, 280, 140);
	DrawArt(GetCenterOffset(227), y + 52, &ArtProgBG, 0, 227);
	if (progress) {
		DrawArt(GetCenterOffset(227), y + 52, &ProgFil, 0, 227 * progress / 100);
	}
	DrawArt(GetCenterOffset(110), y + 99, &SmlButton, 2, 110);

	if (msgSurface) {
		SDL_Rect dscRect = {
			static_cast<Sint16>(x + 50 + 1),
			static_cast<Sint16>(y + 8 + 1),
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

	endMenu = false;
	int progress = 0;

	SDL_Event event;
	while (!endMenu && progress < 100) {
		progress = fnfunc();
		ProgressRender(progress);
		UiRenderItems(vecProgress);
		DrawMouse();
		RenderPresent();

		while (SDL_PollEvent(&event)) {
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
				endMenu = true;
				break;
			default:
				switch (GetMenuAction(event)) {
				case MenuAction_BACK:
				case MenuAction_SELECT:
					endMenu = true;
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

	return progress == 100;
}

DEVILUTION_END_NAMESPACE
