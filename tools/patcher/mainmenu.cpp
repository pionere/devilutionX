/**
 * @file mainmenu.cpp
 *
 * Implementation of functions for interacting with the main menu.
 */
#include <string>
#include <fstream>

#include "all.h"
#include "diabloui.h"
#include "utils/paths.h"

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
#if GAME
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

static int mainmenu_play_intro()
{
	int result = MPR_DONE;
#ifndef HOSTONLY
	// music_stop(); -- no need to stop/start music, play_movie takes care about it
	PaletteFadeOut();
	result = play_movie(INTRO_ARCHIVE, MOV_SKIP);
	// PaletteFadeIn(false); -- no need, the mainmenu is going to fade in
	// mainmenu_refresh_music();
#endif
	return result;
}
#endif // GAME

void mainmenu_loop()
{
	mainmenu_refresh_music();

	while (TRUE) {
		switch (UiMainMenuDialog(PlaySFX)) {
		case 0:
			UiPatcherDialog();
			continue;
		case 1:
			UiMergerDialog();
			continue;
		case 2:
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
