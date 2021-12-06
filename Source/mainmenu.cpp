/**
 * @file mainmenu.cpp
 *
 * Implementation of functions for interacting with the main menu.
 */
#include "all.h"
#include "diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

/** The active music track id for the main menu. */
int menu_music_track_id = TMUSIC_INTRO;

static void mainmenu_refresh_music()
{
	music_start(menu_music_track_id);

	do {
		menu_music_track_id++;
		if (menu_music_track_id == NUM_MUSIC)
			menu_music_track_id = TMUSIC_TOWN;
	} while (menu_music_track_id == TMUSIC_TOWN || menu_music_track_id == TMUSIC_L1);
}

static bool mainmenu_init_menu(bool bSinglePlayer)
{
	bool success;

	music_stop();

	success = StartGame(bSinglePlayer);
	if (success)
		mainmenu_refresh_music();

	return success;
}

static bool mainmenu_single_player()
{
	if (getIniInt("Diablo", "Game Speed", &gnTicksRate)) {
		if (gnTicksRate < SPEED_NORMAL)
			gnTicksRate = SPEED_NORMAL;
		else if (gnTicksRate > SPEED_FASTEST)
			gnTicksRate = SPEED_FASTEST;
	}

	return mainmenu_init_menu(true);
}

static bool mainmenu_multi_player()
{
	return mainmenu_init_menu(false);
}

static void mainmenu_play_intro()
{
#ifndef HOSTONLY
	music_stop();
	play_movie(INTRO_ARCHIVE, MOV_SKIP);
	mainmenu_refresh_music();
#endif
}

void mainmenu_loop()
{
	mainmenu_refresh_music();

	while (TRUE) {
		switch (UiMainMenuDialog(gszProductName, PlaySFX)) {
		case MAINMENU_SINGLE_PLAYER:
			if (mainmenu_single_player())
				continue;
			break;
		case MAINMENU_MULTIPLAYER:
			if (mainmenu_multi_player())
				continue;
			break;
		case MAINMENU_SETTINGS:
			UiSettingsDialog();
			continue;
		case MAINMENU_ATTRACT_MODE:
		case MAINMENU_REPLAY_INTRO:
			if (gbWndActive)
				mainmenu_play_intro();
			continue;
		case MAINMENU_SHOW_CREDITS:
			UiCreditsDialog();
			continue;
		case MAINMENU_EXIT_DIABLO:
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		break;
	}

	music_stop();
}

DEVILUTION_END_NAMESPACE
