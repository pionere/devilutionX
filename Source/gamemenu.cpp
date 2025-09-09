/**
 * @file gamemenu.cpp
 *
 * Implementation of the in-game menu functions.
 */
#include "all.h"
#include "engine/render/text_render.h"
#include "plrctrls.h"
#include "storm/storm_cfg.h"
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include "controls/axis_direction.h"
#include "controls/controller_motion.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

// Forward-declare menu handlers, used by the global menu structs below.
static void gamemenu_previous(bool bActivate);
static void gamemenu_new_game(bool bActivate);
static void gamemenu_exit_game(bool bActivate);
//static void gamemenu_load_game(bool bActivate);
static void gamemenu_save_game(bool bActivate);
static void gamemenu_open_chat(bool bActivate);
static void gamemenu_restart_town(bool bActivate);
static void gamemenu_settings(bool bActivate);
static void gamemenu_music_volume(bool bActivate);
static void gamemenu_sound_volume(bool bActivate);
static void gamemenu_gamma(bool bActivate);
static void gamemenu_speed(bool bActivate);

/** Contains the game menu items of the single player menu. */
static TMenuItem sgSingleMenu[] = {
	// clang-format off
	// pszStr,     fnMenu,              dwFlags, wMenuParam*
	{ "Settings",  &gamemenu_settings,  GMF_ENABLED, 0, 0 },
	{ "New Game",  &gamemenu_new_game,  GMF_ENABLED, 0, 0 },
	// { "Load Game", &gamemenu_load_game, GMF_ENABLED, 0, 0 },
	{ "Save Game", &gamemenu_save_game, GMF_ENABLED, 0, 0 },
	{ "Exit Game", &gamemenu_exit_game, GMF_ENABLED, 0, 0 },
	// clang-format on
};
/** Contains the game menu items of the multi player menu. */
static TMenuItem sgMultiMenu[] = {
	// clang-format off
	// pszStr,           fnMenu,                 dwFlags, wMenuParam*
	{ "Settings",        &gamemenu_settings,     GMF_ENABLED, 0, 0 },
	{ "New Game",        &gamemenu_new_game,     GMF_ENABLED, 0, 0 },
	{ "Open Chat",       &gamemenu_open_chat,    GMF_ENABLED, 0, 0 },
	{ "Restart In Town", &gamemenu_restart_town, GMF_ENABLED, 0, 0 },
	{ "Exit Game",       &gamemenu_exit_game,    GMF_ENABLED, 0, 0 },
	// clang-format on
};
/** Contains the menu items of the settings menu. */
static TMenuItem sgSettingsMenu[] = {
	// clang-format off
	// pszStr,          fnMenu,                 dwFlags, wMenuParam*
#ifdef NOSOUND
	{ "Music Disabled", &gamemenu_music_volume, 0, 0, 0 },
	{ "Sound Disabled", &gamemenu_sound_volume, 0, 0, 0 },
#else
	{ "Music",          &gamemenu_music_volume, GMF_ENABLED | GMF_SLIDER, 0, 0 },
	{ "Sound",          &gamemenu_sound_volume, GMF_ENABLED | GMF_SLIDER, 0, 0 },
#endif
	{ "Gamma",          &gamemenu_gamma,        GMF_ENABLED | GMF_SLIDER, 0, 0 },
	{ "Speed",          &gamemenu_speed,        GMF_ENABLED | GMF_SLIDER, 0, 0 },
	{ "Previous Menu",  &gamemenu_previous,     GMF_ENABLED, 0, 0 },
	// clang-format on
};

/** The number of options in the small-menu. */
static unsigned gnNumSubmenus;
/** Currently selected entry in the small-menu */
static unsigned gnCurrSubmenu;
/** Specifies whether the cursor should be moved to the current small-menu */
static bool gbMoveCursor;

static void gamemenu_update_single()
{
	bool enable;

	// disable saving in case the player died, the player is changing the level, or diablo is dying
	enable = /*pcursicon == CURSOR_HAND &&*/ gbDeathflag == MDM_ALIVE && !myplr._pLvlChanging;
	// TODO: disable saving if there is a live turn in transit? (SNetGetLiveTurnsInTransit)
	gmenu_enable(&sgSingleMenu[2], enable);
}

static void gamemenu_update_multi()
{
	// disable new game in case the player is dying or dead
	gmenu_enable(&sgMultiMenu[1], gbDeathflag == MDM_ALIVE);
	// disable start chat in case of local games
	gmenu_enable(&sgMultiMenu[2], !IsLocalGame);
	// disable restart in town in case the player is not dead
	gmenu_enable(&sgMultiMenu[3], gbDeathflag == MDM_DEAD);
}

static void gamemenu_update_settings()
{
}

/* Initalize the settings-menu in the main menu */
void gamemenu_main()
{
	InitGMenu();
	gnNumSubmenus = 0;
	gamemenu_settings(true);
}

static void gamemenu_large()
{
	gnNumSubmenus = 0;
	if (IsMultiGame) {
		gmenu_set_items(sgMultiMenu, lengthof(sgMultiMenu), gamemenu_update_multi);
	} else {
		gmenu_set_items(sgSingleMenu, lengthof(sgSingleMenu), gamemenu_update_single);
	}
}

/* Open the menu in the game */
void gamemenu_on()
{
	if (gbDeathflag == MDM_ALIVE) {
		gnNumSubmenus = IsLocalGame ? NUM_GMMS - 2 : NUM_GMMS;
		gpCurrentMenu = (TMenuItem*)-1;
		gnCurrSubmenu = GMM_INVENTORY;
	} else {
		gamemenu_large();
	}
	PressEscKey();
}

void gamemenu_off()
{
	gmenu_set_items(NULL, 0, NULL);
}

static void gamemenu_previous(bool bActivate)
{
	gamemenu_on();
}

static void gamemenu_new_game(bool bActivate)
{
	gamemenu_off();
	NetSendCmd(CMD_DISCONNECT);
}

static void gamemenu_exit_game(bool bActivate)
{
	gamemenu_new_game(bActivate);
	gbRunGameResult = false;
}

/*static void gamemenu_load_game(bool bActivate)
{
	WNDPROC saveProc = SetWindowProc(DisableInputWndProc);
	gamemenu_off();
	NewCursor(CURSOR_NONE);
	InitDiabloMsg(EMSG_LOADING);
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
	scrollrt_render_game();
	gbDeathflag = MDM_ALIVE;
	// gbZoomInFlag = false;
	FreeLevelMem();
	LoadGame();
	ClrDiabloMsg();
	PaletteFadeOut();
	InitLevelCursor();
	gbRedrawFlags = REDRAW_RECALC_FLASKS; // | REDRAW_DRAW_ALL;
	scrollrt_render_game();
	LoadPWaterPalette();
	PaletteFadeIn(false);
	interface_msg_pump();
	SetWindowProc(saveProc);
}*/

static void gamemenu_save_game(bool bActivate)
{
	WNDPROC saveProc = SetWindowProc(DisableInputWndProc);
	assert(saveProc == GameWndProc);
	gamemenu_off();
	// NewCursor(CURSOR_NONE);
	// InitDiabloMsg(EMSG_SAVING);
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
	scrollrt_render_game();
	SaveGame();
	// ClrDiabloMsg();
	EventPlrMsg("Game saved.");
	// InitLevelCursor();
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
	interface_msg_pump();
	SetWindowProc(GameWndProc); // saveProc);
}

static void gamemenu_open_chat(bool bActivate)
{
	gamemenu_off();
	StartPlrMsg();
}

static void gamemenu_restart_town(bool bActivate)
{
	NetSendCmd(CMD_RETOWN);
}

static void gamemenu_sound_music_toggle(/*const char *const *names,*/ TMenuItem* menu_item, int volume)
{
#ifndef NOSOUND
	assert(gbSndInited);
	//if (!gbSndInited)
	//	return;
	//menu_item->dwFlags |= GMF_ENABLED | GMF_SLIDER;
	//menu_item->pszStr = *names;
	static_assert(((VOLUME_MAX - VOLUME_MIN) % 16) == 0, "sfx slider expects a volume range divisible by 16.");
	gmenu_slider_steps(menu_item, 16 /*(VOLUME_MAX - VOLUME_MIN) / 100*/);
	gmenu_slider_set(menu_item, VOLUME_MIN, VOLUME_MAX, volume);
#else
	//menu_item->dwFlags &= ~(GMF_ENABLED | GMF_SLIDER);
	//menu_item->pszStr = names[1];
#endif
}

static void gamemenu_get_music()
{
	gamemenu_sound_music_toggle(/*music_toggle_names,*/ &sgSettingsMenu[0], gnMusicVolume);
}

static void gamemenu_get_sound()
{
	gamemenu_sound_music_toggle(/*sound_toggle_names,*/ &sgSettingsMenu[1], gnSoundVolume);
}

static void gamemenu_get_gamma()
{
	gmenu_slider_steps(&sgSettingsMenu[2], 14 /*(100 - 30) / 5*/);
	gmenu_slider_set(&sgSettingsMenu[2], 30, 100, 130 - GetGamma());
}

static void gamemenu_get_speed()
{
	TMenuItem* pItem = &sgSettingsMenu[3];

	// speed can not be changed in multi-player mode if not in the main menu
	if (IsMultiGame && gbRunGame) {
		pItem->dwFlags = 0; // &= ~(GMF_ENABLED | GMF_SLIDER);
		const char* speed;
		if (gnTicksRate >= SPEED_FASTEST)
			speed = "Speed: Fastest";
		else if (gnTicksRate >= SPEED_FASTER)
			speed = "Speed: Faster";
		else if (gnTicksRate >= SPEED_FAST)
			speed = "Speed: Fast";
		else // if (gnTicksRate == SPEED_NORMAL)
			speed = "Speed: Normal";
		pItem->pszStr = speed;
		return;
	}

	pItem->dwFlags = GMF_ENABLED | GMF_SLIDER; // |= GMF_ENABLED | GMF_SLIDER;

	pItem->pszStr = "Speed";
	gmenu_slider_steps(pItem, SPEED_FASTEST - SPEED_NORMAL);
	gmenu_slider_set(pItem, SPEED_NORMAL, SPEED_FASTEST, gnTicksRate);
}

static void gamemenu_settings(bool bActivate)
{
	gamemenu_get_music();
	gamemenu_get_sound();
	gamemenu_get_gamma();
	gamemenu_get_speed();
	gmenu_set_items(sgSettingsMenu, lengthof(sgSettingsMenu), gamemenu_update_settings);
}

static void gamemenu_music_volume(bool bActivate)
{
#ifndef NOSOUND
	int volume;
	bool musicOn = gbMusicOn;

	if (bActivate) {
		if (musicOn)
			volume = VOLUME_MIN;
		else
			volume = VOLUME_MAX;
	} else {
		volume = gmenu_slider_get(&sgSettingsMenu[0], VOLUME_MIN, VOLUME_MAX);
	}
	sound_set_music_volume(volume);
	if (volume == VOLUME_MIN) {
		// assert(!gbMusicOn);
		if (musicOn)
			music_stop();
	} else {
		// assert(gbMusicOn);
		if (!musicOn)
			music_start(AllLevels[currLvl._dLevelNum].dMusic);
	}
	gamemenu_get_music();
	PlaySfx(IS_TITLEMOV);
#endif
}

static void gamemenu_sound_volume(bool bActivate)
{
#ifndef NOSOUND
	int volume;
	bool soundOn = gbSoundOn;

	if (bActivate) {
		if (soundOn)
			volume = VOLUME_MIN;
		else
			volume = VOLUME_MAX;
	} else {
		volume = gmenu_slider_get(&sgSettingsMenu[1], VOLUME_MIN, VOLUME_MAX);
	}
	sound_set_sound_volume(volume);
	if (volume == VOLUME_MIN) {
		// assert(!gbSoundOn);
		if (soundOn)
			StopSFX();
	} else {
		; // assert(gbSoundOn);
	}
	gamemenu_get_sound();
	PlaySfx(IS_TITLEMOV);
#endif
}

static void gamemenu_gamma(bool bActivate)
{
	int gamma;

	if (bActivate) {
		gamma = GetGamma();
		if (gamma == 100)
			gamma = 30;
		else
			gamma = 100;
	} else {
		gamma = gmenu_slider_get(&sgSettingsMenu[2], 100, 30);
	}
	SetGamma(gamma);
	gamemenu_get_gamma();
	PlaySfx(IS_TITLEMOV);
}

static void gamemenu_speed(bool bActivate)
{
	int speed;

	if (bActivate) {
		if (gnTicksRate == SPEED_NORMAL)
			speed = SPEED_FASTEST;
		else
			speed = SPEED_NORMAL;
	} else {
		speed = gmenu_slider_get(&sgSettingsMenu[3], SPEED_NORMAL, SPEED_FASTEST);
	}
	gnTicksRate = speed;
	gnTickDelay = 1000 / speed;
	setIniInt("Diablo", "Game Speed", speed);
	gamemenu_get_speed();
	PlaySfx(IS_TITLEMOV);
}

#define SMALLMENU_OFFSETX (FOCUS_MINI + 10)
#define SMALLMENU_OFFSETY 10
#define SMALLMENU_LINE_HEIGHT 26
#define SMALLMENU_WIDTH (130 + 2 * SMALLMENU_OFFSETX + 10 * 2)
#define SMALLMENU_HEIGHT (SMALLMENU_LINE_HEIGHT * gnNumSubmenus + 2 * SMALLMENU_OFFSETY)
#define SMALLMENU_X SCREEN_CENTERX(SMALLMENU_WIDTH)
#define SMALLMENU_Y SCREEN_CENTERY(SMALLMENU_HEIGHT)
void gamemenu_draw()
{
	int x, y, flags;
	int i;
	BYTE col;
	const char* label;
	if (gnNumSubmenus == 0) {
		gmenu_draw();
		return;
	}

	y = SMALLMENU_Y;
	x = SMALLMENU_X;
	DrawColorTextBox(x, y, SMALLMENU_WIDTH, SMALLMENU_HEIGHT, COL_WHITE);

	x += SMALLMENU_OFFSETX;
	y += SMALLMENU_OFFSETY;
	for (i = gnNumSubmenus - 1; i >= 0; i--) {
		switch (i) {
		case GMM_EXITGAME:  label = "exit game";  break;
		case GMM_MAINMENU:  label = "options";    break;
		case GMM_QLOG:      label = "quests";     break;
		case GMM_CHARINFO:  label = "profile";    break;
		case GMM_INVENTORY: label = "inventory";  break;
		case GMM_SKILLLIST: label = "skill list"; break;
		case GMM_SPELLBOOK: label = "skill book"; break;
		case GMM_AUTOMAP:   label = "automap";    break;
		case GMM_SENDMSG:   label = "open chat";  break;
		case GMM_TEAMBOOK:  label = "teams";      break;
		default: ASSUME_UNREACHABLE;              break;
		}
		col = COL_GOLD;
		if (POS_IN_RECT(MousePos.x, MousePos.y, x - SCREEN_X, y - SCREEN_Y, SMALLMENU_WIDTH, SMALLMENU_LINE_HEIGHT)) {
			col = COL_GOLD + 1 + 2;
		}
		y += SMALLMENU_LINE_HEIGHT;
		flags = AFF_HCENTER | AFF_BIG | (col << AFF_COLOR_SHL);
		PrintString(flags, label, x, y - (BIG_FONT_HEIGHT - 2 + SMALLMENU_LINE_HEIGHT) / 2, SMALLMENU_WIDTH - 2 * SMALLMENU_OFFSETX, 0);
		if ((unsigned)i == gnCurrSubmenu) {
			DrawSmallPentSpn(x - FOCUS_MINI, x + SMALLMENU_WIDTH - 2 * SMALLMENU_OFFSETX, y - (SMALLMENU_LINE_HEIGHT - FOCUS_MINI) / 2);
			if (gbMoveCursor) {
				gbMoveCursor = false;
				SetCursorPos(x - SCREEN_X + (SMALLMENU_WIDTH - 2 * SMALLMENU_OFFSETX) / 2, y - SCREEN_Y - SMALLMENU_LINE_HEIGHT / 2);
			}
		}
	}
}

static void gamemenu_up_down(bool isDown)
{
	// assert(gmenu_is_active());
	// assert(gnNumSubmenus != 0);
	unsigned n;
	n = gnCurrSubmenu + (isDown ? gnNumSubmenus - 1 : 1);
	n %= gnNumSubmenus;
	// if (n != gnCurrSubmenu) {
		gnCurrSubmenu = n;
		// PlaySfx(IS_TITLEMOV);
		gbMoveCursor = true;
	// }
}

void gamemenu_enter(int submenu)
{
	if (stextflag != STORE_NONE)
		return;

	switch (submenu) {
	case GMM_EXITGAME:
		gamemenu_exit_game(false);
		return;
	case GMM_MAINMENU:
		gamemenu_large();
		return;
	case GMM_QLOG:
		gbSkillListFlag = false;
		if (ToggleWindow(WND_QUEST))
			StartQuestlog();
		break;
	case GMM_CHARINFO:
		gbSkillListFlag = false;
		gbLvlUp = false;
		if (ToggleWindow(WND_CHAR)) {
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
			if (sgbControllerActive)
				FocusOnCharInfo();
#endif
		}
		break;
	case GMM_INVENTORY:
		gbSkillListFlag = false;
		gbInvflag = ToggleWindow(WND_INV);
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		if (gbInvflag && sgbControllerActive)
			FocusOnInventory();
#endif
		break;
	case GMM_SKILLLIST:
		HandleSkillBtn(false);
		break;
	case GMM_SPELLBOOK:
		gbSkillListFlag = false;
		ToggleWindow(WND_BOOK);
		break;
	case GMM_AUTOMAP:
		ToggleAutomap();
		return;
	case GMM_SENDMSG:
		if (gbTalkflag)
			StopPlrMsg();
		else
			StartPlrMsg();
		break;
	case GMM_TEAMBOOK:
		gbSkillListFlag = false;
		ToggleWindow(WND_TEAM);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	gamemenu_off();
}

static void gamemenu_left_right(bool isRight)
{
	if (gnCurrSubmenu == GMM_AUTOMAP) {
		if (isRight) {
			AutomapZoomIn();
		} else {
			AutomapZoomOut();
		}
		return;
	}
	if (isRight) {
		gamemenu_enter(gnCurrSubmenu);
	} else {
		gamemenu_off();
	}
}

static void gamemenu_left_mouse_down()
{
	int px = SMALLMENU_X - SCREEN_X + SMALLMENU_OFFSETX;
	int py = SMALLMENU_Y - SCREEN_Y + SMALLMENU_OFFSETY;
	int sx = MousePos.x - px;
	int sy = MousePos.y - py;
	if (sx < 0 || sx >= SMALLMENU_WIDTH - SMALLMENU_OFFSETX) {
		return;
	}
	if (sy < 0) {
		return;
	}
	unsigned y = ((unsigned)sy) / SMALLMENU_LINE_HEIGHT;
	if (y < gnNumSubmenus) {
		y = gnNumSubmenus - y;
		y--;
		gnCurrSubmenu = y;
		gamemenu_enter(y);
	}
}

void gamemenu_on_mouse_move()
{
	if (gnNumSubmenus == 0) {
		gmenu_on_mouse_move();
	}
}

void gamemenu_presskey(int vkey)
{
	if (gnNumSubmenus == 0) {
		gmenu_presskey(vkey);
		return;
	}
	switch (vkey) {
	case DVL_VK_LBUTTON:
		gamemenu_left_mouse_down();
		break;
	case DVL_VK_RETURN:
		gamemenu_enter(gnCurrSubmenu);
		break;
	case DVL_VK_XBUTTON1:
	case DVL_VK_ESCAPE:
	case DVL_VK_SPACE:
		gamemenu_off();
		break;
	case DVL_VK_LEFT:
		gamemenu_left_right(false);
		break;
	case DVL_VK_RIGHT:
		gamemenu_left_right(true);
		break;
	case DVL_VK_UP:
		gamemenu_up_down(false);
		break;
	case DVL_VK_DOWN:
		gamemenu_up_down(true);
		break;
	}
}
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
void gamemenu_checkmove()
{
	// assert(gmenu_is_active());
	const AxisDirection move_dir = axisDirRepeater.Get(GetLeftStickOrDpadDirection(true));
	if (move_dir.x != AxisDirectionX_NONE) {
		gamemenu_presskey(move_dir.x == AxisDirectionX_RIGHT ? DVL_VK_RIGHT : DVL_VK_LEFT);
	}
	if (move_dir.y != AxisDirectionY_NONE) {
		gamemenu_presskey(move_dir.y == AxisDirectionY_DOWN ? DVL_VK_DOWN : DVL_VK_UP);
	}
}
#endif

void gamemenu_left_mouse(bool isDown)
{
	if (gnNumSubmenus == 0) {
		gmenu_left_mouse(isDown);
	}
}

void gamemenu_update()
{
	if (gnNumSubmenus == 0) {
		gmenu_update();
	}
}

DEVILUTION_END_NAMESPACE
