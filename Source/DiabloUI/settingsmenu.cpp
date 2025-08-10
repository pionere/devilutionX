
#include "DiabloUI/diabloui.h"
#include "all.h"
#include "plrctrls.h"
#include "storm/storm_cfg.h"

DEVILUTION_BEGIN_NAMESPACE

DISABLE_SPEED_OPTIMIZATION

void UiSettingsDialog()
{
	TMenuItem* settingsMenu;

	// enable speed setting
	// assert(!gbRunGame);
	// load speed setting - TODO: copy-paste from mainmenu_single_player
	if (getIniInt("Diablo", "Game Speed", &gnTicksRate)) {
		if (gnTicksRate < SPEED_NORMAL)
			gnTicksRate = SPEED_NORMAL;
		else if (gnTicksRate > SPEED_FASTEST)
			gnTicksRate = SPEED_FASTEST;
	}
	// initialize the UI
	LoadBackgroundArt(NULL, "ui_art\\menu.pal");
	// UiAddBackground();
	SDL_Rect rect0 = { 0, 0, 0, 0 };
	gUiItems.push_back(new UiCustom(gmenu_draw, rect0));
	UiInitScreen(0, NULL, NULL, NULL);
	// initialize gamemenu
	InitGMenu();
	gamemenu_settings(true);
	settingsMenu = gpCurrentMenu;

	Dvl_Event event;
	while (settingsMenu == gpCurrentMenu) {
		UiRender();
		while (UiPeekAndHandleEvents(&event)) {
			switch (event.type) {
			case DVL_WM_MOUSEMOVE:
				gmenu_on_mouse_move();
				break;
			case DVL_WM_LBUTTONDOWN:
				gmenu_presskey(DVL_VK_LBUTTON);
				break;
			case DVL_WM_LBUTTONUP:
				gmenu_left_mouse(false);
				break;
			case DVL_WM_KEYDOWN:
				gmenu_presskey(event.vkcode);
				break;
			}
		}
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		gamemenu_checkmove();
#endif
	}
	PlaySfx(IS_TITLSLCT); // TODO: UiFocusNavigationSelect/UiPlaySelectSound ? (needs UiInitScreen)
	//PaletteFadeOut();
	// free gamemenu
	gmenu_set_items(NULL, 0, NULL);
	FreeGMenu();
	// free the UI
	// FreeBackgroundArt();
	UiClearItems();
}

ENABLE_SPEED_OPTIMIZATION

DEVILUTION_END_NAMESPACE
