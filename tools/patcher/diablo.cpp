/**
 * @file diablo.cpp
 *
 * Implementation of the main game initialization functions.
 */
#include "all.h"
#include "engine/render/text_render.h"
#include "utils/display.h"
#include "utils/paths.h"
#include "utils/screen_reader.hpp"
#include "diabloui.h"
#include "plrctrls.h"
#include "storm/storm_cfg.h"

DEVILUTION_BEGIN_NAMESPACE

/** The X/Y-coordinate of the mouse on the screen. */
POS32 MousePos;
/** To know if ui has been initialized or not */
bool gbWasUiInit = false;
bool gbSndInited = false;

BYTE* pMicrosCel;

static int diablo_parse_flags(int argc, char** argv)
{
	for (int i = 1; i < argc; i++) {
		if (SDL_strcasecmp("--data-dir", argv[i]) == 0) {
			i++;
			if (i < argc)
				SetBasePath(argv[i]);
		} else if (SDL_strcasecmp("--save-dir", argv[i]) == 0) {
			i++;
			if (i < argc)
				SetPrefPath(argv[i]);
#if !FULLSCREEN_ONLY
		} else if (SDL_strcasecmp("-x", argv[i]) == 0) {
			gbFullscreen = false;
#endif
		}
	}
	return EX_OK;
}

static void diablo_init_screen()
{
	MousePos.x = SCREEN_WIDTH / 2u;
	MousePos.y = MAINMENU_TOP + MAINMENU_ITEM_HEIGHT / 2;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	if (!sgbControllerActive)
#endif
		SetCursorPos(MousePos.x, MousePos.y);
	static_assert(EMSG_NONE == 0, "ClrDiabloMsg is not called, because zero initialization cares about it.");
	// not the best place to call this, since it is an in-game 'system'
	// InitGameFX would be more appropriate place, but calling it once would
	// suffice for the moment, because the player can not leave the game
	// until the messages are cleared
	//ClrDiabloMsg();
}

static void diablo_init()
{
	InitPaths();
	InitConfig();

	dx_init(); // inititalize SDL + create the window

	InitArchives();

	InitLighting();
	InitText();
	InitCursorGFX();
	UiInitialize();
	gbWasUiInit = true;

	diablo_init_screen();
#ifdef SCREEN_READER_INTEGRATION
	InitScreenReader();
#endif
	InitSound();
	gbSndInited = true;

	InitUiSFX(); // sfx
}

static void diablo_deinit()
{
	// FreeGameFX(); StopHelp/ClearPanels(); -- TODO: enable if the OS cares about non-freed memory
	if (gbSndInited) {
		StopSFX(); // stop click-effect
		FreeUiSFX();
		FreeSound();
	}
#ifdef SCREEN_READER_INTEGRATION
	FreeScreenReader();
#endif
	//if (gbWasUiInit)
		UiDestroy();
		FreeText();
		FreeCursorGFX();
	//if (_gbWasArchivesInit)
		FreeArchives();
	//if (_gbWasWindowInit) {
		dx_cleanup(); // close the window + SDL
	FreeConfig();
}

int DiabloMain(int argc, char** argv)
{
	int res = diablo_parse_flags(argc, argv);
	if (res != EX_OK)
		return res - 1;

	diablo_init();
	mainmenu_loop();
	diablo_deinit();
	return 0;
}

void diablo_quit(int exitStatus)
{
	diablo_deinit();
	exit(exitStatus);
}

DEVILUTION_END_NAMESPACE
