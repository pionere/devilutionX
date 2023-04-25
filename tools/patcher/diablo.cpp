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
//#include "plrctrls.h"
//#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef GPERF_HEAP_FIRST_GAME_ITERATION
#include <gperftools/heap-profiler.h>
#endif
static const char gszProductName[] = { PROJECT_NAME " v" PROJECT_VERSION };

/** The X/Y-coordinate of the mouse on the screen. */
POS32 MousePos;
/** Specifies whether the intro should be skipped at startup. */
static bool _gbSkipIntro = false;
/** To know if ui has been initialized or not */
bool gbWasUiInit = false;
bool gbSndInited = false;

static void print_help()
{
	printf("Options:\n");
	printf("    %-20s %-30s\n", "-h, --help", "Print this message and exit");
	printf("    %-20s %-30s\n", "--data-dir", "Specify the folder of diabdat.mpq");
	printf("    %-20s %-30s\n", "--save-dir", "Specify the folder of save files");
	printf("    %-20s %-30s\n", "--config-dir", "Specify the location of diablo.ini");
	printf("    %-20s %-30s\n", "-n", "Skip startup videos");
	printf("    %-20s %-30s\n", "-x", "Run in windowed mode");
#if DEBUG_MODE
	printf("\nDebug options:\n");
	printf("    %-20s %-30s\n", "-w", "Enable cheats");
	printf("    %-20s %-30s\n", "-v", "Highlight visibility");
	printf("    %-20s %-30s\n", "-i", "Ignore network timeout");
	printf("    %-20s %-30s\n", "-l <##> <##>", "Start in level as type");
	printf("    %-20s %-30s\n", "-m <##>", "Add debug monster, up to 10 allowed");
	printf("    %-20s %-30s\n", "-q <#>", "Force a certain quest");
	printf("    %-20s %-30s\n", "-r <##########>", "Set map seed");
	printf("    %-20s %-30s\n", "-t <##>", "Set current quest level");
	printf("    %-20s %-30s\n", "--allquests", "Force all quests to generate in a singleplayer game");
#endif
	printf("\nVersion: %s. Report bugs at https://github.com/pionere/devilutionX/\n", gszProductName);
}

static int diablo_parse_flags(int argc, char** argv)
{
	int i;

	static_assert(EX_USAGE + 1 != EX_OK, "diablo_parse_flags shifts the return values.");
	for (i = 1; i < argc; i++) {
		if (SDL_strcasecmp("-h", argv[i]) == 0 || SDL_strcasecmp("--help", argv[i]) == 0) {
			print_help();
			return EX_OK + 1;
		} else if (SDL_strcasecmp("--data-dir", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			SetBasePath(argv[i]);
		} else if (SDL_strcasecmp("--save-dir", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			SetPrefPath(argv[i]);
		} else if (SDL_strcasecmp("--config-dir", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			SetConfigPath(argv[i]);
		} else if (SDL_strcasecmp("-n", argv[i]) == 0) {
			_gbSkipIntro = true;
		} else if (SDL_strcasecmp("-x", argv[i]) == 0) {
			gbFullscreen = false;
#if DEBUG_MODE
		} else if (SDL_strcasecmp("-i", argv[i]) == 0) {
			debug_mode_key_i = TRUE;
			/*
		} else if (SDL_strcasecmp("-j", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			debug_mode_key_J_trigger = argv[i];
		*/
		} else if (SDL_strcasecmp("-l", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			leveldebug = TRUE;
			EnterLevel(SDL_atoi(argv[i]));
			players[0]._pDunLevel = currLvl._dLevelIdx;
		} else if (SDL_strcasecmp("-m", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			monstdebug = TRUE;
			DebugMonsters[debugmonsttypes++] = SDL_atoi(argv[i]);
		} else if (SDL_strcasecmp("-q", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			questdebug = SDL_atoi(argv[i]);
		} else if (SDL_strcasecmp("-r", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			setseed = SDL_atoi(argv[i]);
		} else if (SDL_strcasecmp("-t", argv[i]) == 0) {
			i++;
			if (i == argc)
				return EX_USAGE + 1;
			leveldebug = TRUE;
			EnterLevel(SDL_atoi(argv[i]));
		} else if (SDL_strcasecmp("-v", argv[i]) == 0) {
			visiondebug = TRUE;
		} else if (SDL_strcasecmp("-w", argv[i]) == 0) {
			debug_mode_key_w = TRUE;
		} else if (SDL_strcasecmp("--allquests", argv[i]) == 0) {
			allquests = true;
#endif
		} else {
			// printf("unrecognized option '%s'\n", argv[i]);
			print_help();
			return EX_USAGE + 1;
		}
	}
	return EX_OK;
}

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

static bool diablo_splash()
{
	return UiTitleDialog();
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
	int res = diablo_parse_flags(argc, argv);
	if (res != EX_OK)
		return res - 1;

	diablo_init();
#ifndef HOSTONLY
	if (_gbSkipIntro || diablo_splash())
#endif
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
