/**
 * @file mainmenu.cpp
 *
 * Implementation of functions for interacting with the main menu.
 */
#include "all.h"
#include "diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

char gszHero[16];

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
	gbMaxPlayers = 1;

	if (getIniInt("devilutionx", "game speed", &gnTicksRate)) {
		if (gnTicksRate < SPEED_NORMAL)
			gnTicksRate = SPEED_NORMAL;
		else if (gnTicksRate > SPEED_FASTEST)
			gnTicksRate = SPEED_FASTEST;
	}

	return mainmenu_init_menu(true);
}

static bool mainmenu_multi_player()
{
	gbMaxPlayers = MAX_PLRS;
	return mainmenu_init_menu(false);
}

static void mainmenu_play_intro()
{
	music_stop();
	play_movie(INTRO_ARCHIVE, MOV_SKIP);
	mainmenu_refresh_music();
}

void mainmenu_loop()
{
	bool done;
	int menu;

	mainmenu_refresh_music();
	done = false;

	do {
		menu = 0;
		UiMainMenuDialog(gszProductName, &menu, effects_play_sound, 30);

		switch (menu) {
		case MAINMENU_SINGLE_PLAYER:
			if (!mainmenu_single_player())
				done = true;
			break;
		case MAINMENU_MULTIPLAYER:
			if (!mainmenu_multi_player())
				done = true;
			break;
		case MAINMENU_ATTRACT_MODE:
		case MAINMENU_REPLAY_INTRO:
			if (gbActive)
				mainmenu_play_intro();
			break;
		case MAINMENU_SHOW_CREDITS:
			UiCreditsDialog();
			break;
		case MAINMENU_EXIT_DIABLO:
			done = true;
			break;
		}
	} while (!done);

	music_stop();
}

DEVILUTION_END_NAMESPACE
