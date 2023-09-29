/**
 * @file gamemenu.cpp
 *
 * Implementation of the in-game menu functions.
 */
#include "all.h"
#include "storm/storm_cfg.h"

DEVILUTION_BEGIN_NAMESPACE

// Forward-declare menu handlers, used by the global menu structs below.
static void gamemenu_previous(bool bActivate);
static void gamemenu_new_game(bool bActivate);
static void gamemenu_exit_game(bool bActivate);
static void gamemenu_load_game(bool bActivate);
static void gamemenu_save_game(bool bActivate);
static void gamemenu_restart_town(bool bActivate);
//void gamemenu_settings(bool bActivate);
static void gamemenu_music_volume(bool bActivate);
static void gamemenu_sound_volume(bool bActivate);
static void gamemenu_gamma(bool bActivate);
static void gamemenu_speed(bool bActivate);

/** Contains the game menu items of the single player menu. */
static TMenuItem sgSingleMenu[] = {
	// clang-format off
	// pszStr,     fnMenu,              dwFlags, wMenuParam*
	{ "Save Game", &gamemenu_save_game, GMF_ENABLED, 0, 0 },
	{ "Settings",  &gamemenu_settings,  GMF_ENABLED, 0, 0 },
	{ "New Game",  &gamemenu_new_game,  GMF_ENABLED, 0, 0 },
	{ "Load Game", &gamemenu_load_game, GMF_ENABLED, 0, 0 },
	{ "Exit Game", &gamemenu_exit_game, GMF_ENABLED, 0, 0 },
	// clang-format on
};
/** Contains the game menu items of the multi player menu. */
static TMenuItem sgMultiMenu[] = {
	// clang-format off
	// pszStr,           fnMenu,                 dwFlags, wMenuParam*
	{ "Settings",        &gamemenu_settings,     GMF_ENABLED, 0, 0 },
	{ "New Game",        &gamemenu_new_game,     GMF_ENABLED, 0, 0 },
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

static void gamemenu_update_single()
{
	bool enable;

	gmenu_enable(&sgSingleMenu[3], gbValidSaveFile);
	// disable saving in case the player died, the player is changing the level, or diablo is dying
	enable = /*pcursicon == CURSOR_HAND &&*/ gbDeathflag == MDM_ALIVE && !myplr._pLvlChanging;
	// TODO: disable saving if there is a live turn in transit? (SNetGetLiveTurnsInTransit)
	gmenu_enable(&sgSingleMenu[0], enable);
}

static void gamemenu_update_multi()
{
	// disable new game in case the player is dying or dead
	gmenu_enable(&sgMultiMenu[1], gbDeathflag == MDM_ALIVE);
	// disable restart in town in case the player is not dead
	gmenu_enable(&sgMultiMenu[2], gbDeathflag == MDM_DEAD);
}

static void gamemenu_update_settings()
{
}

void gamemenu_on()
{
	if (IsMultiGame) {
		gmenu_set_items(sgMultiMenu, lengthof(sgMultiMenu), gamemenu_update_multi);
	} else {
		gmenu_set_items(sgSingleMenu, lengthof(sgSingleMenu), gamemenu_update_single);
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

static void gamemenu_load_game(bool bActivate)
{
	WNDPROC saveProc = SetWindowProc(DisableInputWndProc);
	gamemenu_off();
	NewCursor(CURSOR_NONE);
	InitDiabloMsg(EMSG_LOADING);
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game();
	gbDeathflag = MDM_ALIVE;
	// gbZoomInFlag = false;
	LoadGame();
	ClrDiabloMsg();
	PaletteFadeOut();
	InitLevelCursor();
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game();
	LoadPWaterPalette();
	PaletteFadeIn(false);
	interface_msg_pump();
	SetWindowProc(saveProc);
}

static void gamemenu_save_game(bool bActivate)
{
	WNDPROC saveProc = SetWindowProc(DisableInputWndProc);
	gamemenu_off();
	// NewCursor(CURSOR_NONE);
	InitDiabloMsg(EMSG_SAVING);
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game();
	SaveGame();
	ClrDiabloMsg();
	// InitLevelCursor();
	gbRedrawFlags = REDRAW_ALL;
	interface_msg_pump();
	SetWindowProc(saveProc);
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
	gmenu_slider_set(&sgSettingsMenu[2], 30, 100, GetGamma());
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

void gamemenu_settings(bool bActivate)
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
			music_start(AllLevels[currLvl._dLevelIdx].dMusic);
	}
	gamemenu_get_music();
	PlaySFX(IS_TITLEMOV);
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
	PlaySFX(IS_TITLEMOV);
#endif
}

static void gamemenu_gamma(bool bActivate)
{
	int gamma;

	if (bActivate) {
		gamma = GetGamma();
		if (gamma == 30)
			gamma = 100;
		else
			gamma = 30;
	} else {
		gamma = gmenu_slider_get(&sgSettingsMenu[2], 30, 100);
	}
	UpdateGamma(gamma);
	gamemenu_get_gamma();
	PlaySFX(IS_TITLEMOV);
}

static void gamemenu_speed(bool bActivate)
{
	if (bActivate) {
		if (gnTicksRate == SPEED_NORMAL)
			gnTicksRate = SPEED_FASTEST;
		else
			gnTicksRate = SPEED_NORMAL;
	} else {
		gnTicksRate = gmenu_slider_get(&sgSettingsMenu[3], SPEED_NORMAL, SPEED_FASTEST);
	}
	setIniInt("Diablo", "Game Speed", gnTicksRate);
	gnTickDelay = 1000 / gnTicksRate;
	gamemenu_get_speed();
	PlaySFX(IS_TITLEMOV);
}

DEVILUTION_END_NAMESPACE
