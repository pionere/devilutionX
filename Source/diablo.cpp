/**
 * @file diablo.cpp
 *
 * Implementation of the main game initialization functions.
 */
#include "all.h"
#include "paths.h"
#include "diabloui.h"
#include <config.h>
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

SDL_Window *ghMainWnd;
DWORD glSeedTbl[NUMLEVELS];
int gnLevelTypeTbl[NUMLEVELS];
int MouseX;
int MouseY;
bool _gbGameLoopStartup;
bool gbRunGame;
bool gbRunGameResult;
bool gbZoomflag;
/** Enable updating of player character, set to false once Diablo dies */
bool gbProcessPlayers;
bool gbLoadGame;
bool gbCineflag;
int gbRedrawFlags;
int PauseMode;
#ifdef HELLFIRE
bool gbUseTheoQuest;
bool gbUseCowFarmer;
bool gbUseNestArt;
#endif
int sgnTimeoutCurs;
bool gbActionBtnDown;
bool gbAltActionBtnDown;
DWORD sgdwLastABD, sgdwLastAABD; // tick counter when the last time one of the mouse-buttons were pressed down
int actionBtnKey, altActionBtnKey;
int gnTicksRate = SPEED_NORMAL;
unsigned gnTickDelay = 1000 / SPEED_NORMAL;

/* rdata */

/**
 * Specifies whether to give the game exclusive access to the
 * screen, as needed for efficient rendering in fullscreen mode.
 */
bool gbFullscreen = true;
bool _gbShowintro = true;
bool gbShowTooltip = false;
#ifdef _DEBUG
int DebugMonsters[10];
BOOL visiondebug;
/** unused */
BOOL scrollflag;
BOOL lightflag;
BOOL light4flag;
BOOL leveldebug;
BOOL monstdebug;
/** unused */
BOOL trigdebug;
int setseed;
int debugmonsttypes;
bool allquests;
int questdebug = -1;
int debug_mode_key_s;
int debug_mode_key_w;
int debug_mode_key_inverted_v;
BOOL debug_mode_god_mode;
int debug_mode_key_d;
int debug_mode_key_i;
int dbgplr;
int dbgqst;
int dbgmon;
int arrowdebug;
#endif
/** Specifies whether players are in non-PvP mode. */
bool gbFriendlyMode = true;
/** Default controls. */
// clang-format off
BYTE WMButtonInputTransTbl[] = { ACT_NONE,
// LBUTTON, RBUTTON,    CANCEL,   MBUTTON,  X1BUTTON, X2BUTTON, UNDEF,    BACK,     TAB,         UNDEF,
  ACT_ACT,  ACT_ALTACT, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_AUTOMAP, ACT_NONE,
// UNDEF,   CLEAR,    RETURN,     UNDEF,    UNDEF,    SHIFT,    CONTROL,  ALT,      PAUSE,     CAPS,
  ACT_NONE, ACT_NONE, ACT_RETURN, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_PAUSE, ACT_NONE,
// KANAH,   IMEON,    JUNJA,    FINAL,    HANJA,    IMEOFF,   ESCAPE,     CONVERT,  NONCONV,  ACCEPT,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_ESCAPE, ACT_NONE, ACT_NONE, ACT_NONE,
// CHANGE,  SPACE,       PGUP,     PGDOWN,     END,      HOME,     LEFT,     UP,     RIGHT,     DOWN,
  ACT_NONE, ACT_CLEARUI, ACT_PGUP, ACT_PGDOWN, ACT_NONE, ACT_NONE, ACT_LEFT, ACT_UP, ACT_RIGHT, ACT_DOWN,
// SELECT,  PRINT,    EXEC,     PRINTSCRN, INSERT,  DELETE,   HELP,     0,        1,         2,       
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_ITEM0, ACT_ITEM1,
// 3,        4,         5,         6,         7,         8,         9,        UNDEF,    UNDEF,    UNDEF,
  ACT_ITEM2, ACT_ITEM3, ACT_ITEM4, ACT_ITEM5, ACT_ITEM6, ACT_ITEM7, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    A,        B,           C,        D,        E,        F,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_SPLBOOK, ACT_CHAR, ACT_NONE, ACT_NONE, ACT_GAMMA_INC,
// G,            H,        I,       J,        K,        L,        M,        N,        O,        P,
  ACT_GAMMA_DEC, ACT_NONE, ACT_INV, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_PAUSE,
// Q,         R,        S,           T,           U,        V,       W,        X,        Y,        Z,
  ACT_QUESTS, ACT_NONE, ACT_SPLLIST, ACT_TOOLTIP, ACT_NONE, ACT_VER, ACT_NONE, ACT_NONE, ACT_NONE, ACT_ZOOM,
// LWIN,    RWIN,     APPS,     UNDEF,    SLEEP,    NUM0,     NUM1,     NUM2,     NUM3,     NUM4,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// NUM5,    NUM6,     NUM7,     NUM8,     NUM9,     MULT,     ADD,         SEP,      SUB,          DEC,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_MAPZ_IN, ACT_NONE, ACT_MAPZ_OUT, ACT_NONE,
// DIV,     F1,       F2,       F3,       F4,       F5,       F6,       F7,       F8,       F9,
  ACT_NONE, ACT_HELP, ACT_NONE, ACT_NONE, ACT_NONE, ACT_SPL0, ACT_SPL1, ACT_SPL2, ACT_SPL3, ACT_MSG0,
// F10,     F11,      F12,      F13,      F14,      F15,      F16,      F17,      F18,      F19,
  ACT_MSG1, ACT_MSG2, ACT_MSG3, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// F20,     F21,      F22,      F23,      F24,      UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    NUMLOCK,  SCRLLOCK, UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    LSHIFT,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// RSHIFT,  LCTRL,    RCTRL,    LMENU,    RMENU,    BBACK,    BFWD,     BREFRESH, BSTOP,    BSEARCH,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// BFAV,    BHOME,    MUTE,     VOL_UP,   VOL_DOWN, NTRACK,   PTRACK,   STOP,     PLAYP,    MAIL,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// MSEL,    APP1,     APP2,     UNDEF,    UNDEF,    OEM_1,    OPLUS,       OCOMMA,   OMINUS,       OPERIOD,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_MAPZ_IN, ACT_NONE, ACT_MAPZ_OUT, ACT_NONE,
// OEM_2,   OEM_3,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    OEM_4,    OEM_5,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// OEM_6,   OEM_7,    OEM_8,    UNDEF,    UNDEF,    OEM_102,  UNDEF,    UNDEF,    PROC,     UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// PACKET,  UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    ATTN,     CRSEL,    EREOF,    PLAY,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// ZOOM,    UNDEF,    PA1,      OCLEAR,   UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
};
// clang-format on

/** To know if these things have been done when we get to the diablo_deinit() function */
bool _gbWasArchivesInit = false;
/** To know if surfaces have been initialized or not */
bool _gbWasWindowInit = false;
bool _gbWasUiInit = false;
bool gbSndInited = false;

static void print_help_and_exit()
{
	printf("Options:\n");
	printf("    %-20s %-30s\n", "-h, --help", "Print this message and exit");
	printf("    %-20s %-30s\n", "--version", "Print the version and exit");
	printf("    %-20s %-30s\n", "--data-dir", "Specify the folder of diabdat.mpq");
	printf("    %-20s %-30s\n", "--save-dir", "Specify the folder of save files");
	printf("    %-20s %-30s\n", "--config-dir", "Specify the location of diablo.ini");
	printf("    %-20s %-30s\n", "-n", "Skip startup videos");
	printf("    %-20s %-30s\n", "-f", "Display frames per second");
	printf("    %-20s %-30s\n", "-x", "Run in windowed mode");
#ifdef HELLFIRE
	printf("    %-20s %-30s\n", "--theoquest", "Enable the Theo quest");
	printf("    %-20s %-30s\n", "--cowquest", "Enable the Cow quest");
	printf("    %-20s %-30s\n", "--nestart", "Use alternate nest palette");
#endif
#ifdef _DEBUG
	printf("\nDebug options:\n");
	printf("    %-20s %-30s\n", "-d", "Increased item drops");
	printf("    %-20s %-30s\n", "-w", "Enable cheats");
	printf("    %-20s %-30s\n", "-$", "Enable god mode");
	printf("    %-20s %-30s\n", "-^", "Enable god mode and debug tools");
	//printf("    %-20s %-30s\n", "-b", "Enable item drop log");
	printf("    %-20s %-30s\n", "-v", "Highlight visibility");
	printf("    %-20s %-30s\n", "-i", "Ignore network timeout");
	//printf("    %-20s %-30s\n", "-j <##>", "Init trigger at level");
	printf("    %-20s %-30s\n", "-l <##> <##>", "Start in level as type");
	printf("    %-20s %-30s\n", "-m <##>", "Add debug monster, up to 10 allowed");
	printf("    %-20s %-30s\n", "-q <#>", "Force a certain quest");
	printf("    %-20s %-30s\n", "-r <##########>", "Set map seed");
	printf("    %-20s %-30s\n", "-t <##>", "Set current quest level");
	printf("    %-20s %-30s\n", "--allquests", "Force all quests to generate in a singleplayer game");
#endif
	printf("\nReport bugs at https://github.com/diasurgical/devilutionX/\n");
	diablo_quit(0);
}

static void diablo_parse_flags(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		if (strcasecmp("-h", argv[i]) == 0 || strcasecmp("--help", argv[i]) == 0) {
			print_help_and_exit();
		} else if (strcasecmp("--version", argv[i]) == 0) {
			printf("%s v%s\n", PROJECT_NAME, PROJECT_VERSION);
			diablo_quit(0);
		} else if (strcasecmp("--data-dir", argv[i]) == 0) {
			SetBasePath(argv[++i]);
		} else if (strcasecmp("--save-dir", argv[i]) == 0) {
			SetPrefPath(argv[++i]);
		} else if (strcasecmp("--config-dir", argv[i]) == 0) {
			SetConfigPath(argv[++i]);
		} else if (strcasecmp("-n", argv[i]) == 0) {
			_gbShowintro = false;
		} else if (strcasecmp("-f", argv[i]) == 0) {
			EnableFrameCount();
		} else if (strcasecmp("-x", argv[i]) == 0) {
			gbFullscreen = false;
#ifdef HELLFIRE
		} else if (strcasecmp("--theoquest", argv[i]) == 0) {
			gbUseTheoQuest = true;
		} else if (strcasecmp("--cowquest", argv[i]) == 0) {
			gbUseCowFarmer = true;
		} else if (strcasecmp("--nestart", argv[i]) == 0) {
			gbUseNestArt = true;
#endif
#ifdef _DEBUG
		} else if (strcasecmp("-^", argv[i]) == 0) {
			debug_mode_key_inverted_v = TRUE;
			debug_mode_god_mode = TRUE;
		} else if (strcasecmp("-$", argv[i]) == 0) {
			debug_mode_god_mode = TRUE;
			/*
		} else if (strcasecmp("-b", argv[i]) == 0) {
			debug_mode_key_b = 1;
		*/
		} else if (strcasecmp("-d", argv[i]) == 0) {
			debug_mode_key_d = TRUE;
		} else if (strcasecmp("-i", argv[i]) == 0) {
			debug_mode_key_i = TRUE;
			/*
		} else if (strcasecmp("-j", argv[i]) == 0) {
			debug_mode_key_J_trigger = argv[++i];
		*/
		} else if (strcasecmp("-l", argv[i]) == 0) {
			gbSetlevel = false;
			leveldebug = TRUE;
			leveltype = SDL_atoi(argv[++i]);
			currlevel = SDL_atoi(argv[++i]);
			plr[0].plrlevel = currlevel;
		} else if (strcasecmp("-m", argv[i]) == 0) {
			monstdebug = TRUE;
			DebugMonsters[debugmonsttypes++] = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-q", argv[i]) == 0) {
			questdebug = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-r", argv[i]) == 0) {
			setseed = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-s", argv[i]) == 0) {
			debug_mode_key_s = TRUE;
		} else if (strcasecmp("-t", argv[i]) == 0) {
			leveldebug = TRUE;
			gbSetlevel = true;
			setlvlnum = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-v", argv[i]) == 0) {
			visiondebug = TRUE;
		} else if (strcasecmp("-w", argv[i]) == 0) {
			debug_mode_key_w = TRUE;
		} else if (strcasecmp("--allquests", argv[i]) == 0) {
			allquests = true;
#endif
		} else {
			printf("unrecognized option '%s'\n", argv[i]);
			print_help_and_exit();
		}
	}
}

void FreeGameMem()
{
	music_stop();

	MemFreeDbg(pDungeonCels);
	MemFreeDbg(pMegaTiles);
	MemFreeDbg(pLevelPieces);
	MemFreeDbg(pSpecialCels);

	FreeMissiles();
	FreeMonsters();
	FreeObjectGFX();
	FreeMonsterSnd();
	FreeTownerGFX();
}

static void start_game(unsigned int uMsg)
{
	gbZoomflag = true;
	CalcViewportGeometry();
	gbCineflag = false;
	InitCursor();
	InitLightTable();
#ifdef _DEBUG
	LoadDebugGFX();
#endif
	assert(ghMainWnd != NULL);
	music_stop();
	ShowProgress(uMsg);
	gmenu_init_menu();
	InitLevelCursor();
	sgnTimeoutCurs = CURSOR_NONE;
	gbActionBtnDown = false;
	gbAltActionBtnDown = false;
}

static void free_game()
{
	int i;

	FreeControlPan();
	FreeInvGFX();
	FreeGMenu();
	FreeQuestText();
	FreeStoreMem();

	for (i = 0; i < MAX_PLRS; i++)
		FreePlayerGFX(i);

	FreeItemGFX();
	FreeCursor();
	FreeLightTable();
#ifdef _DEBUG
	FreeDebugGFX();
#endif
	FreeGameMem();
}

static void PressKey(int vkey);

static bool ProcessInput()
{
	if (PauseMode == 2) {
		return false;
	}

	if (gmenu_is_active()) {
		return gbMaxPlayers != 1;
	}

	if (sgnTimeoutCurs == CURSOR_NONE) {
#if HAS_TOUCHPAD == 1
		finish_simulated_mouse_clicks(MouseX, MouseY);
#endif
		CheckCursMove();
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
		plrctrls_after_check_curs_move();
#endif
		DWORD tick = SDL_GetTicks();
		if (gbActionBtnDown && (tick - sgdwLastABD) >= 200) {
			gbActionBtnDown = false;
			PressKey(actionBtnKey);
		}
		if (gbAltActionBtnDown && (tick - sgdwLastAABD) >= 200) {
			gbAltActionBtnDown = false;
			PressKey(altActionBtnKey);
		}
	}

	return true;
}

static void game_logic()
{
	if (!ProcessInput()) {
		return;
	}
	if (gbProcessPlayers) {
		ProcessPlayers();
	}
	if (leveltype != DTYPE_TOWN) {
		ProcessMonsters();
		ProcessObjects();
		ProcessMissiles();
		ProcessItems();
		ProcessLightList();
		ProcessVisionList();
	} else {
		ProcessTowners();
		ProcessItems();
		ProcessMissiles();
	}

#ifdef _DEBUG
	if (debug_mode_key_inverted_v && GetAsyncKeyState(DVL_VK_SHIFT)) {
		ScrollView();
	}
#endif

	sound_update();
	ClearPlrMsg();
	CheckTriggers();
	CheckQuests();
	pfile_update(false);

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	plrctrls_after_game_logic();
#endif
}

/**
 * @param bStartup Process additional ticks before returning
 */
static void game_loop(bool bStartup)
{
	int i;

	i = gbMaxPlayers == 1 ? 1 : (bStartup ? gnTicksRate * 3 : 3);

	do {
		if (!multi_handle_delta()) {
			static_assert(CURSOR_NONE == 0, "BitOr optimization of timeout_cursor depends on CURSOR_NONE being 0.");
			if ((sgnTimeoutCurs | gbActionBtnDown | gbAltActionBtnDown) == 0) {
				sgnTimeoutCurs = pcurs;
				multi_net_ping();
				InitDiabloMsg(EMSG_DESYNC);
				NewCursor(CURSOR_HOURGLASS);
				gbRedrawFlags = REDRAW_ALL;
			}
			scrollrt_draw_game_screen(true);
			break;
		}
		if (sgnTimeoutCurs != CURSOR_NONE) {
			NewCursor(sgnTimeoutCurs);
			sgnTimeoutCurs = CURSOR_NONE;
			gbRedrawFlags = REDRAW_ALL;
		}
		game_logic();
	} while (--i != 0 && gbRunGame && nthread_has_500ms_passed());
}

static void run_game_loop(unsigned int uMsg)
{
	WNDPROC saveProc;
	MSG msg;

	nthread_ignore_mutex(true);
	start_game(uMsg);
	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(GameWndProc);
	run_delta_info();
	gbRunGame = true;
	gbProcessPlayers = true;
	gbRunGameResult = true;
	gbRedrawFlags = REDRAW_ALL;
	DrawAndBlit();
	LoadPWaterPalette();
	PaletteFadeIn();
	gbRedrawFlags = REDRAW_ALL;
	_gbGameLoopStartup = true;
	nthread_ignore_mutex(false);

	while (gbRunGame) {
		while (PeekMessage(&msg)) {
			if (msg.message == DVL_WM_QUIT) {
				gbRunGameResult = false;
				gbRunGame = false;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!gbRunGame)
			break;
		if (!nthread_has_500ms_passed()) {
			ProcessInput();
			DrawAndBlit();
			continue;
		}
		diablo_color_cyc_logic();
		multi_process_network_packets();
		game_loop(_gbGameLoopStartup);
		_gbGameLoopStartup = false;
		DrawAndBlit();
	}

	if (gbMaxPlayers != 1) {
		pfile_write_hero();
	}

	pfile_flush_W();
	PaletteFadeOut();
	NewCursor(CURSOR_NONE);
	ClearScreenBuffer();
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game_screen(true);
	saveProc = SetWindowProc(saveProc);
	assert(saveProc == GameWndProc);
	free_game();

	if (gbCineflag) {
		gbCineflag = false;
		DoEnding();
	}
}

bool StartGame(bool bSinglePlayer)
{
	gbSelectProvider = true;
	gbSelectHero = true;

	while (TRUE) {
		gbLoadGame = false;

		if (!NetInit(bSinglePlayer)) {
			gbRunGameResult = true;
			break;
		}

		run_game_loop((gbLoadGame && gbValidSaveFile) ? WM_DIABLOADGAME : WM_DIABNEWGAME);
		NetClose();
		if (!gbRunGameResult)
			break;
		pfile_create_player_description(NULL, 0);
	}

	SNetDestroy();
	return gbRunGameResult;
}

static void diablo_init_screen()
{
	MouseX = SCREEN_WIDTH / 2;
	MouseY = SCREEN_HEIGHT / 2;
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (!sgbControllerActive)
#endif
		SetCursorPos(MouseX, MouseY);
	ScrollInfo._sdx = 0;
	ScrollInfo._sdy = 0;
	ScrollInfo._sxoff = 0;
	ScrollInfo._syoff = 0;
	ScrollInfo._sdir = SDIR_NONE;

	ClrDiabloMsg();
}

static void InitControls()
{
	int i;

	// load key-configuration from diablo.ini
	SLoadKeyMap(WMButtonInputTransTbl);

	// find the action-keys to trigger when the button is held down
	actionBtnKey = ACT_NONE;
	altActionBtnKey = ACT_NONE;
	for (i = 0; i < lengthof(WMButtonInputTransTbl); i++) {
		if (WMButtonInputTransTbl[i] == ACT_ACT)
			actionBtnKey = i;
		else if (WMButtonInputTransTbl[i] == ACT_ALTACT)
			altActionBtnKey = i;
	}
}

static void diablo_init()
{
	init_create_window();
	_gbWasWindowInit = true;

	init_archives();
	_gbWasArchivesInit = true;

	UiInitialize();
	_gbWasUiInit = true;

	ReadOnlyTest();

	diablo_init_screen();

	snd_init();
	gbSndInited = true;

	ui_sound_init();

	InitControls();
}

static void diablo_splash()
{
	if (!_gbShowintro)
		return;

	play_movie("gendata\\logo.smk", MOV_SKIP);

	if (getIniBool("Diablo", "Intro", true)) {
		play_movie(INTRO_ARCHIVE, MOV_SKIP);
		setIniValue("Diablo", "Intro", "0");
	}

	UiTitleDialog();
}

static void diablo_deinit()
{
	NetClose();
	if (gbSndInited)
		effects_cleanup_sfx();
	if (_gbWasUiInit)
		UiDestroy();
	if (_gbWasArchivesInit)
		init_cleanup();
	if (_gbWasWindowInit)
		dx_cleanup(); // Cleanup SDL surfaces stuff, so we have to do it before SDL_Quit().
	if (gbWasFontsInit)
		FontsCleanup();
	if (SDL_WasInit(SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC))
		SDL_Quit();
}

void diablo_quit(int exitStatus)
{
	diablo_deinit();
	exit(exitStatus);
}

int DiabloMain(int argc, char **argv)
{
	diablo_parse_flags(argc, argv);
	diablo_init();
	diablo_splash();
	mainmenu_loop();
	diablo_deinit();

	return 0;
}

static bool AttackCmd(bool bShift, int attack)
{
	int sl = GetSpellLevel(myplr, attack);

	if (plr[myplr]._pSkillFlags & SFLAG_RANGED) {
		if (bShift) {
			NetSendCmdLocBParam2(true, CMD_RATTACKXY, cursmx, cursmy, attack, sl);
			return true;
		}
		if (pcursmonst != -1) {
			if (CanTalkToMonst(pcursmonst)) {
				NetSendCmdParam3(true, CMD_ATTACKID, pcursmonst, attack, sl);
			} else {
				NetSendCmdParam3(true, CMD_RATTACKID, pcursmonst, attack, sl);
			}
			return true;
		}
		if (pcursplr != -1) {
			if (!gbFriendlyMode)
				NetSendCmdBParam3(true, CMD_RATTACKPID, pcursplr, attack, sl);
			return true;
		}
	} else {
		if (bShift) {
			if (pcursmonst != -1) {
				if (CanTalkToMonst(pcursmonst)) {
					NetSendCmdParam3(true, CMD_ATTACKID, pcursmonst, attack, sl);
				} else {
					NetSendCmdLocBParam2(true, CMD_SATTACKXY, cursmx, cursmy, attack, sl);
				}
			} else {
				NetSendCmdLocBParam2(true, CMD_SATTACKXY, cursmx, cursmy, attack, sl);
			}
			return true;
		}
		if (pcursmonst != -1) {
			NetSendCmdParam3(true, CMD_ATTACKID, pcursmonst, attack, sl);
			return true;
		}
		if (pcursplr != -1) {
			if (!gbFriendlyMode)
				NetSendCmdBParam3(true, CMD_ATTACKPID, pcursplr, attack, sl);
			return true;
		}
	}
	return false;
}

static void ActionBtnCmd(bool bShift)
{
	bool bNear;

	assert(pcurs == CURSOR_HAND);

	if (pcursitem != -1 && !bShift) {
		NetSendCmdLocParam1(true, gbInvflag ? CMD_GOTOGETITEM : CMD_GOTOAGETITEM, cursmx, cursmy, pcursitem);
		return;
	}
	if (leveltype == DTYPE_TOWN) {
		if (pcursmonst != -1) {
			NetSendCmdLocParam1(true, CMD_TALKXY, cursmx, cursmy, pcursmonst);
			return;
		}
		if (pcursitem == -1 && pcursplr == -1)
			NetSendCmdLoc(true, CMD_WALKXY, cursmx, cursmy);
	} else {
		bNear = abs(plr[myplr]._px - cursmx) < 2 && abs(plr[myplr]._py - cursmy) < 2;
		if (pcursobj != -1 && (!bShift || bNear && object[pcursobj]._oBreak == 1)) {
			NetSendCmdLocParam1(true, CMD_OPOBJXY, cursmx, cursmy, pcursobj);
			return;
		}
		if (AttackCmd(bShift, plr[myplr]._pLSpell))
			return;
		assert(pcursplr == -1);
		assert(pcursmonst == -1);
		if (pcursitem == -1 && pcursobj == -1)
			NetSendCmdLoc(true, CMD_WALKXY, cursmx, cursmy);
	}
}

bool TryIconCurs(bool bShift)
{
	switch (pcurs) {
	case CURSOR_IDENTIFY:
	case CURSOR_REPAIR:
	case CURSOR_RECHARGE:
		if (pcursinvitem != -1) {
			PlayerStruct *p = &plr[myplr];
			NetSendCmdLocBParam3(true, CMD_SPELLXY, p->_px, p->_py, p->_pTSpell, p->_pTSplFrom, pcursinvitem);
		}
		break;
	case CURSOR_DISARM:
		if (pcursobj != -1) {
			if (!bShift ||
			 (abs(plr[myplr]._px - cursmx) < 2 && abs(plr[myplr]._py - cursmy) < 2)) {
				NetSendCmdLocParam1(true, CMD_DISARMXY, cursmx, cursmy, pcursobj);
				return true;
			}
		}
		break;
	case CURSOR_OIL:
		if (pcursinvitem != -1)
			NetSendCmdBParam2(true, CMD_DOOIL, plr[myplr]._pOilFrom, pcursinvitem);
		break;
	case CURSOR_TELEKINESIS:
		if (pcursobj != -1)
			NetSendCmdParam1(true, CMD_OPOBJT, pcursobj);
		if (pcursitem != -1)
			NetSendCmdGItem(true, CMD_REQUESTAGITEM, myplr, myplr, pcursitem);
		if (pcursmonst != -1 && !MonTalker(pcursmonst))
			NetSendCmdParam1(true, CMD_KNOCKBACK, pcursmonst);
		break;
	case CURSOR_RESURRECT:
		if (pcursplr != -1) {
			int sn = plr[myplr]._pTSpell;
			int sf = plr[myplr]._pTSplFrom;
			NetSendCmdLocBParam3(true, CMD_SPELLXY, plr[pcursplr]._px, plr[pcursplr]._py, sn, sf, pcursplr);
		}
		break;
	case CURSOR_TELEPORT: {
		int sn = plr[myplr]._pTSpell;
		int sf = plr[myplr]._pTSplFrom;
		int sl = GetSpellLevel(myplr, sn);
		if (pcursmonst != -1)
			NetSendCmdWBParam4(true, CMD_SPELLID, pcursmonst, sn, sf, sl);
		else if (pcursplr != -1)
			NetSendCmdWBParam4(true, CMD_SPELLPID, pcursplr, sn, sf, sl);
		else
			NetSendCmdLocBParam3(true, CMD_SPELLXY, cursmx, cursmy, sn, sf, sl);
	} break;
	case CURSOR_HEALOTHER:
		if (pcursplr != -1) {
			int sn = plr[myplr]._pTSpell;
			int sf = plr[myplr]._pTSplFrom;
			int sl = GetSpellLevel(myplr, sn);
			NetSendCmdWBParam4(true, CMD_SPELLPID, pcursplr, sn, sf, sl);
		}
		break;
	default:
		return false;
	}
	NewCursor(CURSOR_HAND);
	return true;
}

static void ActionBtnDown(bool bShift)
{
	assert(!gbDropGoldFlag);
	assert(!gmenu_left_mouse(TRUE));
	assert(sgnTimeoutCurs == CURSOR_NONE);
	assert(!gbTalkflag || !control_check_talk_btn());
	assert(!gbDeathflag);
	assert(PauseMode != 2);
	assert(!gbDoomflag);

	if (gbSpselflag) {
		SetRSpell();
		return;
	}

	if (stextflag != STORE_NONE) {
		CheckStoreBtn();
		return;
	}

	if (gmenu_is_active()) {
		DoLimitedPanBtn();
		return;
	}

	if (DoPanBtn()) {
		return;
	}

	if (TryIconCurs(bShift))
		return;

	if (gbQtextflag) {
		gbQtextflag = false;
		stream_stop();
		return;
	}

	if (gbQuestlog && MouseX < SPANEL_WIDTH && MouseY < SPANEL_HEIGHT) {
		CheckQuestlog();
		return;
	}

	if (MouseX <= InvRect[SLOTXY_BELT_LAST].X + INV_SLOT_SIZE_PX && MouseY >= SCREEN_HEIGHT - InvRect[SLOTXY_BELT_FIRST].Y - INV_SLOT_SIZE_PX) {
		// in belt
		// assert(!DoPanBtn());
		CheckBeltClick();
		return;
	}

	if (gbChrflag && MouseX < SPANEL_WIDTH && MouseY < SPANEL_HEIGHT) {
		CheckChrBtns();
		return;
	}

	if (gbInvflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		// in inventory
		CheckInvClick();
		return;
	}

	if (gbSbookflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		CheckSBook();
		return;
	}

	if (pcurs >= CURSOR_FIRSTITEM) {
		DropItem();
		return;
	}

	ActionBtnCmd(bShift);
}

void AltActionBtnCmd(bool bShift)
{
	PlayerStruct *p;
	int rspell, sf, sl;

	if ((DWORD)myplr >= MAX_PLRS) {
		dev_fatal("AltActionBtnCmd: illegal player %d", myplr);
	}

	p = &plr[myplr];
	assert(pcurs == CURSOR_HAND);
	rspell = p->_pRSpell;
	if (rspell == SPL_INVALID
	 || (spelldata[rspell].sFlags & p->_pSkillFlags) != spelldata[rspell].sFlags) {
		PlaySFX(sgSFXSets[SFXS_PLR_34][p->_pClass]);
		return;
	}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (!sgbControllerActive)
#endif
		if (PANELS_COVER && spelldata[rspell].sTargeted
		 && (/*(MouseY >= PANEL_TOP && MouseX >= PANEL_LEFT && MouseX <= RIGHT_PANEL)       // inside main panel
		   ||*/ ((gbChrflag || gbQuestlog) && MouseX < SPANEL_WIDTH && MouseY < SPANEL_HEIGHT)  // inside left panel
		   || ((gbInvflag || gbSbookflag) && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT)) // inside right panel
		       ) {
			return;
		}

	switch (p->_pRSplType) {
	case RSPLTYPE_ABILITY:
		// assert(spelldata[rspell].sManaCost == 0);
		if (rspell == SPL_WATTACK) {
			if (leveltype != DTYPE_TOWN && AttackCmd(bShift, SPL_ATTACK))
				return;
			rspell = SPL_WALK;
		} else if (rspell == SPL_WRATTACK) {
			if (leveltype != DTYPE_TOWN && AttackCmd(bShift, SPL_RATTACK))
				return;
			rspell = SPL_WALK;
		}
		if (rspell == SPL_WALK) {
			NetSendCmdLoc(true, CMD_WALKXY, cursmx, cursmy);
			return;
		}
		if (rspell == SPL_ATTACK) {
			AttackCmd(bShift, SPL_ATTACK);
			return;
		}
		if (rspell == SPL_RATTACK) {
			AttackCmd(bShift, SPL_RATTACK);
			return;
		}
		if (rspell == SPL_BLOCK) {
			int dir = GetDirection(p->_px, p->_py, cursmx, cursmy);
			NetSendCmdParam1(true, CMD_BLOCK, dir);
			return;
		}
		sf = SPLFROM_ABILITY;
		break;
	case RSPLTYPE_SPELL:
		if (CheckSpell(myplr, rspell)) {
			if (spelldata[rspell].sType == STYPE_NONE) {
				AttackCmd(bShift, rspell);
				return;
			}
			sf = SPLFROM_MANA;
		} else {
			sf = SPLFROM_INVALID;
			PlaySFX(sgSFXSets[SFXS_PLR_35][p->_pClass]);
		}
		break;
	case RSPLTYPE_SCROLL:
		sf = SpellSourceInv(rspell);
		break;
	case RSPLTYPE_CHARGES:
		sf = SpellSourceEquipment(rspell);
		break;
	case RSPLTYPE_INVALID:
		sf = SPLFROM_INVALID;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (sf == SPLFROM_INVALID)
		return;

	if (spelldata[rspell].spCurs != CURSOR_NONE) {
		NewCursor(spelldata[rspell].spCurs);
		p->_pTSpell = rspell;
		p->_pTSplFrom = sf;
		return;
	}

	sl = GetSpellLevel(myplr, rspell);
	if (pcursmonst != -1) {
		NetSendCmdWBParam4(true, CMD_SPELLID, pcursmonst, rspell, sf, sl);
	} else if (pcursplr != -1) {
		NetSendCmdWBParam4(true, CMD_SPELLPID, pcursplr, rspell, sf, sl);
	} else { //145
		NetSendCmdLocBParam3(true, CMD_SPELLXY, cursmx, cursmy, rspell, sf, sl);
	}
}

static void AltActionBtnDown(bool bShift)
{
	assert(!gmenu_is_active());
	assert(sgnTimeoutCurs == CURSOR_NONE);
	assert(PauseMode != 2);
	assert(!gbDoomflag);

	if (plr[myplr]._pInvincible)
		return;

	if (gbSpselflag) {
		SetRSpell();
		return;
	}

	if (stextflag != STORE_NONE)
		return;

	if (TryIconCurs(bShift))
		return;
	if (pcurs >= CURSOR_FIRSTITEM) {
		DropItem();
		return;
	}

	if (gbQuestlog) {
		gbQuestlog = false;
		return;
	}

	if (gbQtextflag) {
		gbQtextflag = false;
		stream_stop();
		return;
	}

	if (pcursinvitem != -1 && UseInvItem(pcursinvitem))
		return;
	AltActionBtnCmd(bShift);
}

static void diablo_pause_game()
{
	if (gbMaxPlayers == 1) {
		if (PauseMode != 0) {
			PauseMode = 0;
		} else {
			PauseMode = 2;
			sound_stop();
		}
		gbRedrawFlags = REDRAW_ALL;
	}
}

static void diablo_hotkey_msg(DWORD dwMsg)
{
	if (gbMaxPlayers == 1) {
		return;
	}

	char entryKey[16];
	snprintf(entryKey, sizeof(entryKey), "QuickMsg%02d", dwMsg);
	if (!getIniValue("NetMsg", entryKey, gbNetMsg, sizeof(gbNetMsg)))
		return;

	NetSendCmdString(-1);
}

static bool PressSysKey(int wParam)
{
	if (gmenu_is_active() || wParam != DVL_VK_F10)
		return false;
	diablo_hotkey_msg(1);
	return true;
}

static void ReleaseKey(int vkey)
{
	if (vkey == DVL_VK_LBUTTON) {
		gmenu_left_mouse(FALSE);
		if (gbTalkflag)
			control_release_talk_btn();
		if (gabPanbtn[PANBTN_MAINMENU])
			CheckBtnUp();
		if (gbChrbtnactive)
			ReleaseChrBtns();
		if (gbLvlbtndown)
			ReleaseLvlBtn();
		if (stextflag != STORE_NONE)
			ReleaseStoreBtn();
	} else if (vkey == DVL_VK_SNAPSHOT) {
		CaptureScreen();
	}

	if (WMButtonInputTransTbl[vkey] == ACT_ACT) {
		gbActionBtnDown = false;
	} else if (WMButtonInputTransTbl[vkey] == ACT_ALTACT) {
		gbAltActionBtnDown = false;
	}
}

bool PressEscKey()
{
	bool rv = false;

	if (gbDoomflag) {
		doom_close();
		rv = true;
	}
	if (gbHelpflag) {
		gbHelpflag = false;
		rv = true;
	}

	if (gbQtextflag) {
		gbQtextflag = false;
		stream_stop();
		rv = true;
	} else if (stextflag) {
		STextESC();
		rv = true;
	}

	if (currmsg != EMSG_NONE) {
		msgdelay = 0;
		rv = true;
	}
	if (gbTalkflag) {
		control_reset_talk();
		rv = true;
	}
	if (gbDropGoldFlag) {
		control_drop_gold(DVL_VK_ESCAPE);
		rv = true;
	}
	if (gbSpselflag) {
		gbSpselflag = false;
		rv = true;
	}
	if (pcurs != CURSOR_HAND && pcurs < CURSOR_FIRSTITEM) {
		NewCursor(CURSOR_HAND);
		rv = true;
	}

	return rv;
}

static void ClearUI()
{
	gbHelpflag = false;
	gbInvflag = false;
	gbChrflag = false;
	gbSbookflag = false;
	gbSpselflag = false;
	if (gbQtextflag && leveltype == DTYPE_TOWN) {
		gbQtextflag = false;
		stream_stop();
	}
	gbQuestlog = false;
	gbAutomapflag = false;
	msgdelay = 0;
	gabPanbtn[PANBTN_MAINMENU] = false;
	gamemenu_off();
	//doom_close();
}

static void PressKey(int vkey)
{
	if (gmenu_is_active()) {
		if (gmenu_presskeys(vkey))
			return;
	}
	if (gbTalkflag) {
		if (control_presskeys(vkey))
			return;
	}

	if (vkey == DVL_VK_ESCAPE) {
		if (!PressEscKey()) {
			gamemenu_on();
		}
		return;
	}

	if (sgnTimeoutCurs != CURSOR_NONE) {
		return;
	}

	if (vkey == DVL_VK_RETURN && GetAsyncKeyState(DVL_VK_MENU)) {
		dx_reinit();
		return;
	}

	if (gbDeathflag) {
		if (vkey == DVL_VK_RETURN) {
			control_type_message();
		} else if (vkey == DVL_VK_LBUTTON) {
			DoLimitedPanBtn();
		} else {
			int transKey = WMButtonInputTransTbl[vkey];
			if (transKey == ACT_MSG0) {
				diablo_hotkey_msg(0);
			} else if (transKey == ACT_MSG1) {
				diablo_hotkey_msg(1);
			} else if (transKey == ACT_MSG2) {
				diablo_hotkey_msg(2);
			} else if (transKey == ACT_MSG3) {
				diablo_hotkey_msg(3);
			}
		}
		return;
	}

	int transKey = WMButtonInputTransTbl[vkey];
	if (transKey == ACT_PAUSE) {
		diablo_pause_game();
		return;
	}
	if (PauseMode == 2) {
		return;
	}

	if (gbDropGoldFlag) {
		control_drop_gold(vkey);
		return;
	}

	if (gbDoomflag) {
		doom_close();
		return;
	}

	switch (transKey) {
	case ACT_NONE:
		break;
	case ACT_ACT:
		if (!gbActionBtnDown) {
			gbActionBtnDown = true;
			sgdwLastABD = SDL_GetTicks();
			ActionBtnDown(GetAsyncKeyState(DVL_VK_SHIFT) != 0);
		}
		break;
	case ACT_ALTACT:
		if (!gbAltActionBtnDown) {
			gbAltActionBtnDown = true;
			sgdwLastAABD = SDL_GetTicks();
			AltActionBtnDown(GetAsyncKeyState(DVL_VK_SHIFT) != 0);
		}
		break;
	case ACT_SPL0:
	case ACT_SPL1:
	case ACT_SPL2:
	case ACT_SPL3:
		static_assert(ACT_SPL0 + 1 == ACT_SPL1, "PressKey expects a continuous assignment of ACT_SPLx 1.");
		static_assert(ACT_SPL1 + 1 == ACT_SPL2, "PressKey expects a continuous assignment of ACT_SPLx 2.");
		static_assert(ACT_SPL2 + 1 == ACT_SPL3, "PressKey expects a continuous assignment of ACT_SPLx 3.");
		if (gbSpselflag)
			SetSpeedSpell(transKey - ACT_SPL0);
		else
			ToggleSpell(transKey - ACT_SPL0);
		break;
	case ACT_INV:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_INVENTORY);
			if (!gbInvflag || gbChrflag) {
				if (MouseX < 480 && MouseY < PANEL_TOP && PANELS_COVER) {
					SetCursorPos(MouseX + 160, MouseY);
				}
			} else {
				if (MouseX > 160 && MouseY < PANEL_TOP && PANELS_COVER) {
					SetCursorPos(MouseX - 160, MouseY);
				}
			}
		}
		break;
	case ACT_CHAR:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_CHARINFO);
			if (!gbChrflag || gbInvflag) {
				if (MouseX > 160 && MouseY < PANEL_TOP && PANELS_COVER) {
					SetCursorPos(MouseX - 160, MouseY);
				}
			} else {
				if (MouseX < 480 && MouseY < PANEL_TOP && PANELS_COVER) {
					SetCursorPos(MouseX + 160, MouseY);
				}
			}
		}
		break;
	case ACT_SPLBOOK:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_SPELLBOOK);
		}
		break;
	case ACT_SPLLIST:
		if (stextflag == STORE_NONE) {
			HandleSpellBtn();
		}
		break;
	case ACT_ITEM0:
	case ACT_ITEM1:
	case ACT_ITEM2:
	case ACT_ITEM3:
	case ACT_ITEM4:
	case ACT_ITEM5:
	case ACT_ITEM6:
	case ACT_ITEM7:
		static_assert(ACT_ITEM0 + 1 == ACT_ITEM1, "PressKey expects a continuous assignment of ACT_ITEMx 1.");
		static_assert(ACT_ITEM1 + 1 == ACT_ITEM2, "PressKey expects a continuous assignment of ACT_ITEMx 2.");
		static_assert(ACT_ITEM2 + 1 == ACT_ITEM3, "PressKey expects a continuous assignment of ACT_ITEMx 3.");
		static_assert(ACT_ITEM3 + 1 == ACT_ITEM4, "PressKey expects a continuous assignment of ACT_ITEMx 4.");
		static_assert(ACT_ITEM4 + 1 == ACT_ITEM5, "PressKey expects a continuous assignment of ACT_ITEMx 5.");
		static_assert(ACT_ITEM5 + 1 == ACT_ITEM6, "PressKey expects a continuous assignment of ACT_ITEMx 6.");
		static_assert(ACT_ITEM6 + 1 == ACT_ITEM7, "PressKey expects a continuous assignment of ACT_ITEMx 7.");
		UseInvItem(INVITEM_BELT_FIRST + transKey - ACT_ITEM0);
		break;
	case ACT_AUTOMAP:
		HandlePanBtn(PANBTN_AUTOMAP);
		break;
	case ACT_MAPZ_IN:
		if (gbAutomapflag) {
			AutomapZoomIn();
		}
		break;
	case ACT_MAPZ_OUT:
		if (gbAutomapflag) {
			AutomapZoomOut();
		}
		break;
	case ACT_CLEARUI:
		if (!gbChrflag && gbInvflag && MouseX < 480 && MouseY < PANEL_TOP && PANELS_COVER) {
			SetCursorPos(MouseX + 160, MouseY);
		}
		if (!gbInvflag && gbChrflag && MouseX > 160 && MouseY < PANEL_TOP && PANELS_COVER) {
			SetCursorPos(MouseX - 160, MouseY);
		}
		ClearUI();
		break;
	case ACT_UP:
		if (stextflag) {
			STextUp();
		} else if (gbQuestlog) {
			QuestlogUp();
		} else if (gbHelpflag) {
			HelpScrollUp();
		} else if (gbAutomapflag) {
			AutomapUp();
		}
		break;
	case ACT_DOWN:
		if (stextflag) {
			STextDown();
		} else if (gbQuestlog) {
			QuestlogDown();
		} else if (gbHelpflag) {
			HelpScrollDown();
		} else if (gbAutomapflag) {
			AutomapDown();
		}
		break;
	case ACT_LEFT:
		if (gbAutomapflag) {
			AutomapLeft();
		}
		break;
	case ACT_RIGHT:
		if (gbAutomapflag) {
			AutomapRight();
		}
		break;
	case ACT_PGUP:
		if (stextflag) {
			STextPrior();
		}
		break;
	case ACT_PGDOWN:
		if (stextflag) {
			STextNext();
		}
		break;
	case ACT_RETURN:
		if (stextflag) {
			STextEnter();
		} else if (gbQuestlog) {
			QuestlogEnter();
		} else {
			control_type_message();
		}
		break;
	case ACT_QUESTS:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_QLOG);
		}
		break;
	case ACT_MSG0:
	case ACT_MSG1:
	case ACT_MSG2:
	case ACT_MSG3:
		static_assert(ACT_MSG0 + 1 == ACT_MSG1, "PressKey expects a continuous assignment of ACT_MSGx 1.");
		static_assert(ACT_MSG1 + 1 == ACT_MSG2, "PressKey expects a continuous assignment of ACT_MSGx 2.");
		static_assert(ACT_MSG2 + 1 == ACT_MSG3, "PressKey expects a continuous assignment of ACT_MSGx 3.");
		diablo_hotkey_msg(transKey - ACT_MSG0);
		break;
	case ACT_GAMMA_DEC:
		DecreaseGamma();
		break;
	case ACT_GAMMA_INC:
		IncreaseGamma();
		break;
	case ACT_ZOOM:
		gbZoomflag = !gbZoomflag;
		CalcViewportGeometry();
		break;
	case ACT_VER:
		if (GetAsyncKeyState(DVL_VK_SHIFT)) {
			copy_str(gbNetMsg, gszProductName);
			NetSendCmdString(1 << myplr);
		} else {
			const char *difficulties[3] = { "Normal", "Nightmare", "Hell" };
			snprintf(gbNetMsg, sizeof(gbNetMsg), "%s, mode = %s", gszProductName, difficulties[gnDifficulty]);
			NetSendCmdString(1 << myplr);
		}
		break;
	case ACT_HELP:
		if (gbHelpflag) {
			gbHelpflag = false;
		} else if (stextflag == STORE_NONE) {
			ClearUI();
			DisplayHelp();
		}
		break;
	case ACT_PAUSE:
		break;
	case ACT_ESCAPE:
		break;
	case ACT_TOOLTIP:
		gbShowTooltip = !gbShowTooltip;
		break;
	default:
		ASSUME_UNREACHABLE
	}

#ifdef _DEBUG
	if (vkey == DVL_VK_F2) {
	}
	else if (vkey == DVL_VK_F3) {
		if (pcursitem != -1) {
			snprintf(
			    gbNetMsg,
				sizeof(gbNetMsg),
			    "IDX = %i  :  Seed = %i  :  CF = %i",
			    item[pcursitem]._iIdx,
			    item[pcursitem]._iSeed,
			    item[pcursitem]._iCreateInfo);
			NetSendCmdString(1 << myplr);
		}
		snprintf(gbNetMsg, sizeof(gbNetMsg), "Numitems : %i", numitems);
		NetSendCmdString(1 << myplr);
	}
	else if (vkey == DVL_VK_F4) {
		PrintDebugQuest();
	}
#endif
}

/**
 * @internal `return` must be used instead of `break` to be bin exact as C++
 */
static void PressChar(WPARAM vkey)
{
	if (gmenu_is_active()) {
		return;
	}
	if (gbTalkflag) {
		if (control_talk_last_key(vkey))
			return;
	}
#ifdef _DEBUG
	if (sgnTimeoutCurs != CURSOR_NONE || gbDeathflag)
		return;

	if (PauseMode == 2) {
		return;
	}
	switch (vkey) {
	case ')':
	case '0':
		if (debug_mode_key_inverted_v) {
			if (arrowdebug > 2) {
				arrowdebug = 0;
			}
			if (arrowdebug == 0) {
				plr[myplr]._pIFlags &= ~ISPL_FIRE_ARROWS;
				plr[myplr]._pIFlags &= ~ISPL_LIGHT_ARROWS;
			}
			if (arrowdebug == 1) {
				plr[myplr]._pIFlags |= ISPL_FIRE_ARROWS;
			}
			if (arrowdebug == 2) {
				plr[myplr]._pIFlags |= ISPL_LIGHT_ARROWS;
			}
			arrowdebug++;
		}
		break;
	case '9':
		if (debug_mode_key_inverted_v || debug_mode_key_w) {
			NetSendCmd(true, CMD_CHEAT_EXPERIENCE);
		}
		break;
	case ':':
		if (currlevel == 0 && debug_mode_key_w) {
			SetAllSpellsCheat();
		}
		break;
	case '[':
		if (currlevel == 0 && debug_mode_key_w) {
			TakeGoldCheat();
		}
		break;
	case ']':
		if (currlevel == 0 && debug_mode_key_w) {
			MaxSpellsCheat();
		}
		break;
	case 'a':
		if (debug_mode_key_inverted_v) {
			plr[myplr]._pSplLvl[plr[myplr]._pSpell]++;
		}
		break;
	case 'D':
		PrintDebugPlayer(true);
		break;
	case 'd':
		PrintDebugPlayer(false);
		break;
	case 'L':
	case 'l':
		if (debug_mode_key_inverted_v) {
			ToggleLighting();
		}
		break;
	case 'M':
		NextDebugMonster();
		break;
	case 'm':
		GetDebugMonster();
		break;
	case 'R':
	case 'r':
		snprintf(gbNetMsg, sizeof(gbNetMsg), "seed = %i", glSeedTbl[currlevel]);
		NetSendCmdString(1 << myplr);
		break;
	case 'T':
	case 't':
		if (debug_mode_key_inverted_v) {
			snprintf(gbNetMsg, sizeof(gbNetMsg), "PX = %i  PY = %i", plr[myplr]._px, plr[myplr]._py);
			NetSendCmdString(1 << myplr);
			snprintf(gbNetMsg, sizeof(gbNetMsg), "CX = %i  CY = %i  DP = %i", cursmx, cursmy, dungeon[cursmx][cursmy]);
			NetSendCmdString(1 << myplr);
		}
		break;
	case '|':
		if (currlevel == 0 && debug_mode_key_w) {
			GiveGoldCheat();
		}
		break;
	case '~':
		if (currlevel == 0 && debug_mode_key_w) {
			StoresCheat();
		}
		break;
	}
#endif
}

static void GetMousePos(LPARAM lParam)
{
	MouseX = (short)(lParam & 0xffff);
	MouseY = (short)((lParam >> 16) & 0xffff);
}

static void UpdateActionBtnState(int vKey, bool dir)
{
	if (vKey == actionBtnKey)
		gbActionBtnDown = dir;
	if (vKey == altActionBtnKey)
		gbAltActionBtnDown = dir;
}

void DisableInputWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case DVL_WM_KEYDOWN:
		UpdateActionBtnState(wParam, true);
		return;
	case DVL_WM_KEYUP:
		UpdateActionBtnState(wParam, false);
		return;
	case DVL_WM_CHAR:
	case DVL_WM_SYSKEYDOWN:
	case DVL_WM_SYSCOMMAND:
		return;
	case DVL_WM_MOUSEMOVE:
		GetMousePos(lParam);
		return;
	case DVL_WM_LBUTTONDOWN:
		UpdateActionBtnState(DVL_VK_LBUTTON, true);
		return;
	case DVL_WM_LBUTTONUP:
		UpdateActionBtnState(DVL_VK_LBUTTON, false);
		return;
	case DVL_WM_RBUTTONDOWN:
		UpdateActionBtnState(DVL_VK_RBUTTON, true);
		return;
	case DVL_WM_RBUTTONUP:
		UpdateActionBtnState(DVL_VK_RBUTTON, false);
		return;
	case DVL_WM_CAPTURECHANGED:
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		return;
	}

	MainWndProc(uMsg, wParam, lParam);
}

void GameWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case DVL_WM_KEYDOWN:
		PressKey(wParam);
		return;
	case DVL_WM_KEYUP:
		ReleaseKey(wParam);
		return;
	case DVL_WM_CHAR:
		PressChar(wParam);
		return;
	case DVL_WM_SYSKEYDOWN:
		if (PressSysKey(wParam))
			return;
		break;
	case DVL_WM_SYSCOMMAND:
		if (wParam == DVL_SC_CLOSE) {
			gbRunGame = false;
			gbRunGameResult = false;
			return;
		}
		break;
	case DVL_WM_MOUSEMOVE:
		GetMousePos(lParam);
		gmenu_on_mouse_move();
		return;
	case DVL_WM_LBUTTONDOWN:
		GetMousePos(lParam);
		PressKey(DVL_VK_LBUTTON);
		return;
	case DVL_WM_LBUTTONUP:
		GetMousePos(lParam);
		ReleaseKey(DVL_VK_LBUTTON);
		return;
	case DVL_WM_RBUTTONDOWN:
		GetMousePos(lParam);
		PressKey(DVL_VK_RBUTTON);
		return;
	case DVL_WM_RBUTTONUP:
		GetMousePos(lParam);
		ReleaseKey(DVL_VK_RBUTTON);
		return;
	case DVL_WM_CAPTURECHANGED:
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		break;
	case WM_DIABNEXTLVL:
	case WM_DIABPREVLVL:
	case WM_DIABRTNLVL:
	case WM_DIABSETLVL:
	case WM_DIABWARPLVL:
	case WM_DIABTOWNWARP:
	case WM_DIABTWARPUP:
	case WM_DIABRETOWN:
		if (gbMaxPlayers != 1)
			pfile_write_hero();
		nthread_ignore_mutex(true);
		PaletteFadeOut();
		sound_stop();
		music_stop();
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		ShowProgress(uMsg);
		gbRedrawFlags = REDRAW_ALL;
		DrawAndBlit();
		LoadPWaterPalette();
		if (gbRunGame)
			PaletteFadeIn();
		nthread_ignore_mutex(false);
		_gbGameLoopStartup = true;
		return;
	}

	MainWndProc(uMsg, wParam, lParam);
}

static void LoadLvlGFX()
{
	assert(pDungeonCels == NULL);

	switch (leveltype) {
	case DTYPE_TOWN:
#ifdef HELLFIRE
		pDungeonCels = LoadFileInMem("NLevels\\TownData\\Town.CEL", NULL);
		pMegaTiles = LoadFileInMem("NLevels\\TownData\\Town.TIL", NULL);
		pLevelPieces = LoadFileInMem("NLevels\\TownData\\Town.MIN", NULL);
#else
		pDungeonCels = LoadFileInMem("Levels\\TownData\\Town.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\TownData\\Town.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\TownData\\Town.MIN", NULL);
#endif
		pSpecialCels = LoadFileInMem("Levels\\TownData\\TownS.CEL", NULL);
		break;
	case DTYPE_CATHEDRAL:
#ifdef HELLFIRE
		if (currlevel >= 21) {
			pDungeonCels = LoadFileInMem("NLevels\\L5Data\\L5.CEL", NULL);
			pMegaTiles = LoadFileInMem("NLevels\\L5Data\\L5.TIL", NULL);
			pLevelPieces = LoadFileInMem("NLevels\\L5Data\\L5.MIN", NULL);
			pSpecialCels = LoadFileInMem("NLevels\\L5Data\\L5S.CEL", NULL);
			break;
		}
#endif
		pDungeonCels = LoadFileInMem("Levels\\L1Data\\L1.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L1Data\\L1.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L1Data\\L1.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L1Data\\L1S.CEL", NULL);
		break;
	case DTYPE_CATACOMBS:
		pDungeonCels = LoadFileInMem("Levels\\L2Data\\L2.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L2Data\\L2.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L2Data\\L2.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L2Data\\L2S.CEL", NULL);
		break;
	case DTYPE_CAVES:
#ifdef HELLFIRE
		if (currlevel >= 17) {
			pDungeonCels = LoadFileInMem("NLevels\\L6Data\\L6.CEL", NULL);
			pMegaTiles = LoadFileInMem("NLevels\\L6Data\\L6.TIL", NULL);
			pLevelPieces = LoadFileInMem("NLevels\\L6Data\\L6.MIN", NULL);
		} else
#endif
		{
			pDungeonCels = LoadFileInMem("Levels\\L3Data\\L3.CEL", NULL);
			pMegaTiles = LoadFileInMem("Levels\\L3Data\\L3.TIL", NULL);
			pLevelPieces = LoadFileInMem("Levels\\L3Data\\L3.MIN", NULL);
		}
		pSpecialCels = LoadFileInMem("Levels\\L1Data\\L1S.CEL", NULL);
		break;
	case DTYPE_HELL:
		pDungeonCels = LoadFileInMem("Levels\\L4Data\\L4.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L4Data\\L4.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L4Data\\L4.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L2Data\\L2S.CEL", NULL);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void LoadAllGFX()
{
	IncProgress();
	IncProgress();
	InitObjectGFX();
	IncProgress();
	InitMissileGFX();
	IncProgress();
}

/**
 * @param lvldir method of entry
 */
static void CreateLevel(int lvldir)
{
	switch (leveltype) {
	case DTYPE_TOWN:
		CreateTown(lvldir);
		InitTownTriggers();
		LoadRndLvlPal(DTYPE_TOWN);
		break;
	case DTYPE_CATHEDRAL:
		CreateL1Dungeon(glSeedTbl[currlevel], lvldir);
		InitL1Triggers();
		Freeupstairs();
#ifdef HELLFIRE
		if (currlevel >= 21) {
			LoadRndLvlPal(DTYPE_NEST);
			break;
		}
#endif
		LoadRndLvlPal(DTYPE_CATHEDRAL);
		break;
	case DTYPE_CATACOMBS:
		CreateL2Dungeon(glSeedTbl[currlevel], lvldir);
		InitL2Triggers();
		Freeupstairs();
		LoadRndLvlPal(DTYPE_CATACOMBS);
		break;
	case DTYPE_CAVES:
		CreateL3Dungeon(glSeedTbl[currlevel], lvldir);
		InitL3Triggers();
		Freeupstairs();
#ifdef HELLFIRE
		if (currlevel >= 17) {
			LoadRndLvlPal(DTYPE_CRYPT);
			break;
		}
#endif
		LoadRndLvlPal(DTYPE_CAVES);
		break;
	case DTYPE_HELL:
		CreateL4Dungeon(glSeedTbl[currlevel], lvldir);
		InitL4Triggers();
		Freeupstairs();
		LoadRndLvlPal(DTYPE_HELL);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void LoadGameLevel(bool firstflag, int lvldir)
{
	int i;

	if (firstflag && lvldir == ENTRY_MAIN) {
		InitLevels();
		InitQuests();
		InitPortals();
		InitDungMsgs(myplr);
	}

#ifdef _DEBUG
	if (setseed)
		glSeedTbl[currlevel] = setseed;
#endif

	music_stop();
	NewCursor(CURSOR_HAND);
	//SetRndSeed(glSeedTbl[currlevel]);
	IncProgress();
	MakeLightTable();
	LoadLvlGFX();
	IncProgress();

	if (firstflag) {
		InitInv();
		InitItemGFX();
		InitQuestText();

		for (i = 0; i < gbMaxPlayers; i++)
			InitPlrGFXMem(i);

		InitStores();
		InitAutomapOnce();
		InitHelp();
		InitControlPan();
	}

	//SetRndSeed(glSeedTbl[currlevel]);

	IncProgress();
	InitAutomap();

	if (lvldir != ENTRY_LOAD) {
		InitLighting();
		InitVision();
	}

	InitLevelMonsters();
	IncProgress();

	if (!gbSetlevel) {
		CreateLevel(lvldir);
		IncProgress();
		FillSolidBlockTbls();
		if (leveltype != DTYPE_TOWN) {
			SetRndSeed(glSeedTbl[currlevel]);
			GetLevelMTypes();
			InitThemes();
			LoadAllGFX();
		} else {
			SetupTownStores();
			IncProgress();
			IncProgress();
			InitMissileGFX();
			IncProgress();
			IncProgress();
		}

		IncProgress();

		if (lvldir == ENTRY_RTNLVL)
			GetReturnLvlPos();
		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();

		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currlevel == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag, true);
			}
		}

		PlayDungMsgs();
		IncProgress();

		SetRndSeed(glSeedTbl[currlevel]);

		if (leveltype != DTYPE_TOWN) {
			if (firstflag || lvldir == ENTRY_LOAD || !plr[myplr]._pLvlVisited[currlevel] || gbMaxPlayers != 1) {
				HoldThemeRooms();
				InitMonsters();
				IncProgress();
				InitObjects();
				InitItems();
#ifdef HELLFIRE
				if (currlevel < 17)
#endif
					CreateThemeRooms();
				IncProgress();
				InitMissiles();
				InitDead();

				if (gbMaxPlayers != 1)
					DeltaLoadLevel();

				IncProgress();
				SavePreLighting();
			} else {
				HoldThemeRooms();
				InitMonsters();
				InitMissiles();
				InitDead();
				IncProgress();
				LoadLevel();
				IncProgress();
			}
		} else {
			InitTowners();
			InitItems();
			InitMissiles();
			IncProgress();

			if (gbMaxPlayers != 1)
				DeltaLoadLevel();
			else if (!firstflag && lvldir != ENTRY_LOAD && plr[myplr]._pLvlVisited[currlevel])
				LoadLevel();

			IncProgress();
		}
		if (gbMaxPlayers == 1)
			ResyncQuests();
		else
			ResyncMPQuests();
	} else {
		LoadSetMap();
		IncProgress();
		GetLevelMTypes();
		IncProgress();
		InitMonsters();
		IncProgress();
		InitMissileGFX();
		IncProgress();
		InitDead();
		IncProgress();
		FillSolidBlockTbls();
		IncProgress();

		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();
		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currlevel == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag, true);
			}
		}

		//PlayDungMsgs();
		if (setlvlnum == SL_SKELKING && quests[Q_SKELKING]._qactive == QUEST_ACTIVE) {
			sfxdelay = 30;
			sfxdnum = USFX_SKING1;
		}

		IncProgress();
		IncProgress();

		if (firstflag || lvldir == ENTRY_LOAD || !plr[myplr]._pSLvlVisited[setlvlnum]) {
			InitItems();
			SavePreLighting();
		} else {
			LoadLevel();
		}

		InitMissiles();
		IncProgress();
	}

	SyncPortals();

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && plr[i].plrlevel == currlevel && (!plr[i]._pLvlChanging || i == myplr)) {
			if (plr[i]._pHitPoints >= (1 << 6)) {
				/*if (gbMaxPlayers == 1)
					dPlayer[plr[i]._px][plr[i]._py] = i + 1;
				else*/
					SyncInitPlrPos(i);
			} else {
				dFlags[plr[i]._px][plr[i]._py] |= BFLAG_DEAD_PLAYER;
			}
		}
	}

	SetDungeonMicros(0, 0, MAXDUNX, MAXDUNY);

	InitLightMax();
	IncProgress();
	IncProgress();

	IncProgress();
	if (leveltype != DTYPE_TOWN) {
		ProcessLightList();
		ProcessVisionList();
	}

#ifdef HELLFIRE
	// BUGFIX: TODO: does not belong here, DeltaLoadLevel should take care about this
	if (currlevel == 24 && quests[Q_NAKRUL]._qactive == QUEST_DONE) {
		OpenUberRoom();
	}
#endif

#ifdef HELLFIRE
	if (currlevel >= 17)
		music_start(currlevel > 20 ? TMUSIC_L5 : TMUSIC_L6);
	else
#endif
		music_start(leveltype);

	while (!IncProgress())
		;
}

void diablo_color_cyc_logic()
{
	if (!gbColorCyclingEnabled)
		return;

	if (leveltype == DTYPE_HELL)
		lighting_color_cycling();
#ifdef HELLFIRE
	else if (currlevel >= 21)
		palette_update_crypt();
	else if (currlevel >= 17)
		palette_update_hive();
#endif
	else if (leveltype == DTYPE_CAVES)
		palette_update_caves();
}

DEVILUTION_END_NAMESPACE
