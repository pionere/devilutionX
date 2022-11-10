
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
	UiInitList(0);
	// initialize gamemenu
	InitGMenu();
	gamemenu_settings(true);
	settingsMenu = gpCurrentMenu;

	SDL_Event event;
	while (settingsMenu == gpCurrentMenu) {
		ClearScreenBuffer();
		gmenu_draw();
		scrollrt_draw_screen(true);
		while (SDL_PollEvent(&event) != 0) {
			UiHandleEvents(&event);

			keypress = DVL_VK_NONAME;
			switch (GetMenuAction(event)) {
			case MenuAction_NONE:
				switch (event.type) {
				case SDL_MOUSEMOTION:
					gmenu_on_mouse_move();
					break;
				case SDL_MOUSEBUTTONDOWN:
					keypress = DVL_VK_LBUTTON;
					break;
				case SDL_MOUSEBUTTONUP:
					gmenu_left_mouse(false);
					break;
				default:
					break;
				}
				break;
			case MenuAction_SELECT:
				keypress = DVL_VK_RETURN;
				break;
			case MenuAction_BACK:
				keypress = DVL_VK_ESCAPE;
				break;
			case MenuAction_DELETE:
				break;
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
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			if (keypress != DVL_VK_NONAME)
				gmenu_presskey(keypress);
		}
	}
	PlaySFX(IS_TITLSLCT); // UiFocusNavigationSelect(); -- needs UiInitList...
	//PaletteFadeOut();

	gmenu_set_items(NULL, 0, NULL);
	FreeGMenu();
}

DEVILUTION_END_NAMESPACE
