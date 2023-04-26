/**
 * @file diablo.cpp
 *
 * Implementation of the main game initialization functions.
 */
#include "all.h"
#include <config.h>
#include "engine/render/text_render.h"
#include "utils/display.h"
#include "utils/paths.h"
#include "diabloui.h"
#include "plrctrls.h"
//#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

static const char gszProductName[] = { PROJECT_NAME " v" PROJECT_VERSION };

/** The X/Y-coordinate of the mouse on the screen. */
POS32 MousePos;
/** Specifies whether the intro should be skipped at startup. */
static bool _gbSkipIntro = false;
/** To know if ui has been initialized or not */
bool gbWasUiInit = false;
bool gbSndInited = false;

static void diablo_init_screen()
{
	MousePos.x = SCREEN_WIDTH / 2;
	MousePos.y = SCREEN_HEIGHT / 2;
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

	dx_init(); // inititalize SDL + create the window

	InitArchives();
#if GAME
#if DEBUG_MODE || DEV_MODE
	ValidateData();
#endif
#endif // GAME
	MakeLightTable();
	InitText();
	InitCursorGFX();
	UiInitialize();
	gbWasUiInit = true;

	diablo_init_screen();

	InitSound();
	gbSndInited = true;

	InitUiSFX(); // sfx
}

static void diablo_deinit()
{
#if GAME
	NetClose();
	pfile_flush(true);
#endif
	// FreeGameFX(); StopHelp/ClearPanels(); -- TODO: enable if the OS cares about non-freed memory
	if (gbSndInited) {
		StopSFX(); // stop click-effect
		FreeUiSFX();
		FreeSound();
	}
	//if (gbWasUiInit)
		UiDestroy();
		FreeText();
		FreeCursorGFX();
	//if (_gbWasArchivesInit)
		FreeArchives();
	//if (_gbWasWindowInit) {
		dx_cleanup(); // close the window + SDL
}

int DiabloMain(int argc, char** argv)
{
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
