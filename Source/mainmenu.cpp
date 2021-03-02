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

static BOOL mainmenu_init_menu(int type)
{
	BOOL success;

	if (type == SELHERO_PREVIOUS)
		return TRUE;

	music_stop();

	success = StartGame(type != SELHERO_CONNECT);
	if (success)
		mainmenu_refresh_music();

	return success;
}

static BOOL mainmenu_single_player()
{
	gbMaxPlayers = 1;

	if (!getIniInt("devilutionx", "game speed", &gnTicksPerSec)) {
		setIniInt("devilutionx", "game speed", gnTicksPerSec);
	}

	return mainmenu_init_menu(SELHERO_NEW_DUNGEON);
}

static BOOL mainmenu_multi_player()
{
	gbMaxPlayers = MAX_PLRS;
	return mainmenu_init_menu(SELHERO_CONNECT);
}

static void mainmenu_play_intro()
{
	music_stop();
	play_movie(INTRO_ARCHIVE, MOV_SKIP);
	mainmenu_refresh_music();
}

void mainmenu_change_name(int arg1, int arg2, int arg3, int arg4, char *name_1, char *name_2)
{
	if (UiValidPlayerName(name_2))
		pfile_rename_hero(name_1, name_2);
}

bool mainmenu_select_hero_dialog(const _SNETPROGRAMDATA *client_info)
{
	int dlgresult = 0;
	if (gbMaxPlayers == 1) {
		UiSelHeroSingDialog(
		    pfile_ui_set_hero_infos,
		    pfile_ui_save_create,
		    pfile_delete_save,
		    pfile_ui_set_class_stats,
		    &dlgresult,
		    gszHero,
		    &gnDifficulty);
		client_info->initdata->bDifficulty = gnDifficulty;

		gbLoadGame = dlgresult == SELHERO_CONTINUE;
	} else {
		UiSelHeroMultDialog( 
		    pfile_ui_set_hero_infos,
		    pfile_ui_save_create,
		    pfile_delete_save,
		    pfile_ui_set_class_stats,
		    &dlgresult,
		    gszHero);
	}
	if (dlgresult == SELHERO_PREVIOUS) {
		SErrSetLastError(1223);
		return FALSE;
	}

	pfile_create_player_description(NULL, NULL);
	return TRUE;
}

void mainmenu_loop()
{
	BOOL done;
	int menu;

	mainmenu_refresh_music();
	done = FALSE;

	do {
		menu = 0;
		UiMainMenuDialog(gszProductName, &menu, effects_play_sound, 30);

		switch (menu) {
		case MAINMENU_SINGLE_PLAYER:
			if (!mainmenu_single_player())
				done = TRUE;
			break;
		case MAINMENU_MULTIPLAYER:
			if (!mainmenu_multi_player())
				done = TRUE;
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
			done = TRUE;
			break;
		}
	} while (!done);

	music_stop();
}

DEVILUTION_END_NAMESPACE
