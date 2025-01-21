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
static int menu_music_track_id = TMUSIC_INTRO;

static void mainmenu_refresh_music()
{
	music_start(menu_music_track_id);

	do {
		menu_music_track_id++;
		if (menu_music_track_id == NUM_MUSIC)
			menu_music_track_id = TMUSIC_TOWN;
	} while (menu_music_track_id == TMUSIC_TOWN || menu_music_track_id == TMUSIC_L1);
}

void mainmenu_loop()
{
	mainmenu_refresh_music();

	while (true) {
		switch (UiMainMenuDialog()) {
		case 0:
			UiPatcherDialog();
			continue;
		case 1:
			UiMergerDialog();
			continue;
		case 2:
#if 0
			UiCheckerDialog();
			continue;
		case 3:
#endif
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
