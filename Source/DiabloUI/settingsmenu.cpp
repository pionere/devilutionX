
#include "DiabloUI/diabloui.h"
#include "controls/menu_controls.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

void UiSettingsDialog()
{
	int keypress;
	TMenuItem* settingsMenu;

	// enable speed setting
	gbGameMode = 0;
	assert(!IsMultiGame);
	// load speed setting - TODO: copy-paste from mainmenu_single_player
	if (getIniInt("Diablo", "Game Speed", &gnTicksRate)) {
		if (gnTicksRate < SPEED_NORMAL)
			gnTicksRate = SPEED_NORMAL;
		else if (gnTicksRate > SPEED_FASTEST)
			gnTicksRate = SPEED_FASTEST;
	}
	// initialize the UI
	LoadBackgroundArt("ui_art\\black.CEL", "ui_art\\menu.pal");
	UiAddBackground(&gUiItems);
	UiInitScreen(0);
	// initialize gamemenu
	InitGMenu();
	gamemenu_settings(true);
	settingsMenu = gpCurrentMenu;

	SDL_Event event;
	while (settingsMenu == gpCurrentMenu) {
		UiClearScreen();
		UiRenderItems(gUiItems);
		gmenu_draw();
		UiFadeIn();
		while (SDL_PollEvent(&event) != 0) {
			UiHandleEvents(&event);

			switch (GetMenuAction(event)) {
			case MenuAction_NONE:
				switch (event.type) {
				case SDL_MOUSEMOTION:
					gmenu_on_mouse_move();
					continue;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button != SDL_BUTTON_LEFT)
						continue;
					keypress = DVL_VK_LBUTTON;
					break;
				case SDL_MOUSEBUTTONUP:
					gmenu_left_mouse(false);
					continue;
				default:
					continue;
				}
				break;
			case MenuAction_SELECT:
				keypress = DVL_VK_RETURN;
				break;
			case MenuAction_BACK:
				keypress = DVL_VK_ESCAPE;
				break;
			case MenuAction_DELETE:
				continue;
			case MenuAction_UP:
				keypress = DVL_VK_UP;
				break;
			case MenuAction_DOWN:
				keypress = DVL_VK_DOWN;
				break;
			case MenuAction_LEFT:
				keypress = DVL_VK_LEFT;
				break;
			case MenuAction_RIGHT:
				keypress = DVL_VK_RIGHT;
				break;
			case MenuAction_PAGE_UP:
			case MenuAction_PAGE_DOWN:
				continue;
			default:
				ASSUME_UNREACHABLE
				continue;
			}
			gmenu_presskey(keypress);
		}
	}
	PlaySFX(IS_TITLSLCT); // TODO: UiFocusNavigationSelect/UiPlaySelectSound ? (needs UiInitScreen)
	//PaletteFadeOut();
	// free gamemenu
	gmenu_set_items(NULL, 0, NULL);
	FreeGMenu();
	// free the UI
	MemFreeDbg(gbBackCel);
	UiClearItems(gUiItems);
}

DEVILUTION_END_NAMESPACE
