/**
 * @file gamemenu.cpp
 *
 * Implementation of the in-game menu functions.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

// Forward-declare menu handlers, used by the global menu structs below.
static void gamemenu_previous(bool bActivate);
static void gamemenu_new_game(bool bActivate);
static void gamemenu_quit_game(bool bActivate);
static void gamemenu_load_game(bool bActivate);
static void gamemenu_save_game(bool bActivate);
static void gamemenu_restart_town(bool bActivate);
static void gamemenu_settings(bool bActivate);
static void gamemenu_music_volume(bool bActivate);
static void gamemenu_sound_volume(bool bActivate);
static void gamemenu_gamma(bool bActivate);
static void gamemenu_speed(bool bActivate);

/** Contains the game menu items of the single player menu. */
static TMenuItem sgSingleMenu[] = {
	// clang-format off
	// dwFlags,      pszStr,          fnMenu
	{ GMENU_ENABLED, "Save Game",     &gamemenu_save_game  },
	{ GMENU_ENABLED, "Settings",      &gamemenu_settings   },
	{ GMENU_ENABLED, "New Game",      &gamemenu_new_game   },
	{ GMENU_ENABLED, "Load Game",     &gamemenu_load_game  },
	{ GMENU_ENABLED, "Quit Game",     &gamemenu_quit_game  },
	{ GMENU_ENABLED, NULL,            NULL }
	// clang-format on
};
/** Contains the game menu items of the multi player menu. */
static TMenuItem sgMultiMenu[] = {
	// clang-format off
	// dwFlags,      pszStr,            fnMenu
	{ GMENU_ENABLED, "Settings",        &gamemenu_settings     },
	{ GMENU_ENABLED, "New Game",        &gamemenu_new_game     },
	{ GMENU_ENABLED, "Restart In Town", &gamemenu_restart_town },
	{ GMENU_ENABLED, "Quit Game",       &gamemenu_quit_game    },
	{ GMENU_ENABLED, NULL,              NULL                   },
	// clang-format on
};
static TMenuItem sgOptionsMenu[] = {
	// clang-format off
	// dwFlags,                     pszStr,          fnMenu
	{ GMENU_ENABLED | GMENU_SLIDER, NULL,            &gamemenu_music_volume  },
	{ GMENU_ENABLED | GMENU_SLIDER, NULL,            &gamemenu_sound_volume  },
	{ GMENU_ENABLED | GMENU_SLIDER, "Gamma",         &gamemenu_gamma         },
	{ GMENU_ENABLED | GMENU_SLIDER, "Speed",         &gamemenu_speed         },
	{ GMENU_ENABLED               , "Previous Menu", &gamemenu_previous      },
	{ GMENU_ENABLED               , NULL,            NULL                    },
	// clang-format on
};
/** Specifies the menu names for music enabled and disabled. */
static const char *const music_toggle_names[] = {
	"Music",
	"Music Disabled",
};
/** Specifies the menu names for sound enabled and disabled. */
static const char *const sound_toggle_names[] = {
	"Sound",
	"Sound Disabled",
};

static void gamemenu_update_single(TMenuItem *pMenuItems)
{
	bool enable;

	gmenu_enable(&sgSingleMenu[3], gbValidSaveFile);

	enable = false;
	if (plr[myplr]._pmode != PM_DEATH && !gbDeathflag)
		enable = true;

	gmenu_enable(&sgSingleMenu[0], enable);
}

static void gamemenu_update_multi(TMenuItem *pMenuItems)
{
	gmenu_enable(&sgMultiMenu[2], gbDeathflag);
}

void gamemenu_on()
{
	if (gbMaxPlayers == 1) {
		gmenu_set_items(sgSingleMenu, gamemenu_update_single);
	} else {
		gmenu_set_items(sgMultiMenu, gamemenu_update_multi);
	}
	PressEscKey();
}

void gamemenu_off()
{
	gmenu_set_items(NULL, NULL);
}

static void gamemenu_previous(bool bActivate)
{
	gamemenu_on();
}

static void gamemenu_new_game(bool bActivate)
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		plr[i]._pmode = PM_QUIT;
		plr[i]._pInvincible = TRUE;
	}

	gbDeathflag = false;
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game_screen(true);
	gbRunGame = false;
	gamemenu_off();
}

static void gamemenu_quit_game(bool bActivate)
{
	gamemenu_new_game(bActivate);
	gbRunGameResult = false;
}

static void gamemenu_load_game(bool bActivate)
{
	WNDPROC saveProc = SetWindowProc(DisableInputWndProc);
	gamemenu_off();
	NewCursor(CURSOR_NONE);
	InitDiabloMsg(EMSG_LOADING);
	gbRedrawFlags = REDRAW_ALL;
	DrawAndBlit();
	LoadGame(false);
	ClrDiabloMsg();
	PaletteFadeOut();
	gbDeathflag = false;
	gbRedrawFlags = REDRAW_ALL;
	DrawAndBlit();
	LoadPWaterPalette();
	PaletteFadeIn();
	NewCursor(CURSOR_HAND);
	interface_msg_pump();
	SetWindowProc(saveProc);
}

static void gamemenu_save_game(bool bActivate)
{
	if (pcurs != CURSOR_HAND) {
		return;
	}

	if (plr[myplr]._pmode == PM_DEATH || gbDeathflag) {
		gamemenu_off();
		return;
	}

	WNDPROC saveProc = SetWindowProc(DisableInputWndProc);
	NewCursor(CURSOR_NONE);
	gamemenu_off();
	InitDiabloMsg(EMSG_SAVING);
	gbRedrawFlags = REDRAW_ALL;
	DrawAndBlit();
	SaveGame();
	ClrDiabloMsg();
	gbRedrawFlags = REDRAW_ALL;
	NewCursor(CURSOR_HAND);
	interface_msg_pump();
	SetWindowProc(saveProc);
}

static void gamemenu_restart_town(bool bActivate)
{
	NetSendCmd(true, CMD_RETOWN);
}

static void gamemenu_sound_music_toggle(const char *const *names, TMenuItem *menu_item, int volume)
{
	assert(gbSndInited);
	//if (gbSndInited) {
		menu_item->dwFlags |= GMENU_ENABLED | GMENU_SLIDER;
		menu_item->pszStr = *names;
		gmenu_slider_steps(menu_item, 17);
		gmenu_slider_set(menu_item, VOLUME_MIN, VOLUME_MAX, volume);
	/*	return;
	}

	menu_item->dwFlags &= ~(GMENU_ENABLED | GMENU_SLIDER);
	menu_item->pszStr = names[1];*/
}

static int gamemenu_slider_music_sound(TMenuItem *menu_item)
{
	return gmenu_slider_get(menu_item, VOLUME_MIN, VOLUME_MAX);
}

static void gamemenu_get_music()
{
	gamemenu_sound_music_toggle(music_toggle_names, sgOptionsMenu, sound_get_music_volume());
}

static void gamemenu_get_sound()
{
	gamemenu_sound_music_toggle(sound_toggle_names, &sgOptionsMenu[1], sound_get_sound_volume());
}

static void gamemenu_get_gamma()
{
	gmenu_slider_steps(&sgOptionsMenu[2], 15);
	gmenu_slider_set(&sgOptionsMenu[2], 30, 100, UpdateGamma(0));
}

static void gamemenu_get_speed()
{
	if (gbMaxPlayers != 1) {
		sgOptionsMenu[3].dwFlags &= ~(GMENU_ENABLED | GMENU_SLIDER);
		const char *speed;
		if (gnTicksRate >= SPEED_FASTEST)
			speed = "Speed: Fastest";
		else if (gnTicksRate >= SPEED_FASTER)
			speed = "Speed: Faster";
		else if (gnTicksRate >= SPEED_FAST)
			speed = "Speed: Fast";
		else // if (gnTicksRate == SPEED_NORMAL)
			speed = "Speed: Normal";
		sgOptionsMenu[3].pszStr = speed;
		return;
	}

	sgOptionsMenu[3].dwFlags |= GMENU_ENABLED | GMENU_SLIDER;

	sgOptionsMenu[3].pszStr = "Speed";
	gmenu_slider_steps(&sgOptionsMenu[3], 46);
	gmenu_slider_set(&sgOptionsMenu[3], SPEED_NORMAL, SPEED_FASTEST, gnTicksRate);
}

static int gamemenu_slider_gamma()
{
	return gmenu_slider_get(&sgOptionsMenu[2], 30, 100);
}

static void gamemenu_settings(bool bActivate)
{
	gamemenu_get_music();
	gamemenu_get_sound();
	gamemenu_get_gamma();
	gamemenu_get_speed();
	gmenu_set_items(sgOptionsMenu, NULL);
}

static void gamemenu_music_volume(bool bActivate)
{
	int volume;

	if (bActivate) {
		if (gbMusicOn)
			volume = VOLUME_MIN;
		else
			volume = VOLUME_MAX;
	} else {
		volume = gamemenu_slider_music_sound(&sgOptionsMenu[0]);
	}
	sound_set_music_volume(volume);
	if (volume == VOLUME_MIN) {
		if (gbMusicOn) {
			gbMusicOn = false;
			music_stop();
		}
	} else if (!gbMusicOn) {
		gbMusicOn = true;
		music_start(AllLevels[currLvl._dLevelIdx].dMusic);
	}
	gamemenu_get_music();
}

static void gamemenu_sound_volume(bool bActivate)
{
	int volume;
	if (bActivate) {
		if (gbSoundOn) {
			gbSoundOn = false;
			sound_stop();
			sound_set_sound_volume(VOLUME_MIN);
		} else {
			gbSoundOn = true;
			sound_set_sound_volume(VOLUME_MAX);
		}
	} else {
		volume = gamemenu_slider_music_sound(&sgOptionsMenu[1]);
		sound_set_sound_volume(volume);
		if (volume == VOLUME_MIN) {
			if (gbSoundOn) {
				gbSoundOn = false;
				sound_stop();
			}
		} else if (!gbSoundOn) {
			gbSoundOn = true;
		}
	}
	PlaySFX(IS_TITLEMOV);
	gamemenu_get_sound();
}

static void gamemenu_gamma(bool bActivate)
{
	int gamma;
	if (bActivate) {
		gamma = UpdateGamma(0);
		if (gamma == 30)
			gamma = 100;
		else
			gamma = 30;
	} else {
		gamma = gamemenu_slider_gamma();
	}

	UpdateGamma(gamma);
	gamemenu_get_gamma();
}

static void gamemenu_speed(bool bActivate)
{
	if (bActivate) {
		gmenu_slider_set(&sgOptionsMenu[3], SPEED_NORMAL, SPEED_FASTEST, gnTicksRate);
	} else {
		gnTicksRate = gmenu_slider_get(&sgOptionsMenu[3], SPEED_NORMAL, SPEED_FASTEST);
		gnTickDelay = 1000 / gnTicksRate;

		setIniInt("devilutionx", "game speed", gnTicksRate);
	}
}

DEVILUTION_END_NAMESPACE
