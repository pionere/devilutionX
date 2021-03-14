/**
 * @file defs.h
 *
 * Global definitions and Macros.
 */

#ifdef USE_SDL1
// SDL1 controllers
#ifndef HAS_JOYSTICK
#define HAS_JOYSTICK	1
#endif
#ifndef HAS_DPAD
#define HAS_DPAD		1
#endif
#define HAS_GAMECTRL	0
#define HAS_TOUCHPAD	0
#else
// SDL2 controllers
#ifndef HAS_JOYSTICK
#define HAS_JOYSTICK	1
#endif
#ifndef HAS_DPAD
#define HAS_DPAD		1
#endif
#ifndef HAS_GAMECTRL
#define HAS_GAMECTRL	1
#endif
#ifndef HAS_TOUCHPAD
#define HAS_TOUCHPAD	1
#endif
//#ifndef HAS_KBCTRL
//#define HAS_KBCTRL		1
//#endif
#endif

#ifdef SPAWN
#define DATA_ARCHIVE_MAIN		"spawn.mpq"
#define DATA_ARCHIVE_MAIN_ALT	"SPAWN.MPQ"
#define DATA_ARCHIVE_PATCH		"patch_sh.mpq"
#define MENU_ART				"ui_art\\swmmenu.pcx"
#else
#define DATA_ARCHIVE_MAIN		"diabdat.mpq"
#define DATA_ARCHIVE_MAIN_ALT	"DIABDAT.MPQ"
#define DATA_ARCHIVE_PATCH		"patch_rt.mpq"
#define MENU_ART				"ui_art\\mainmenu.pcx"
#endif

#ifdef HELLFIRE
#define LOGO_DATA				"Data\\hf_logo3.CEL"
#define LOGO_WIDTH				430
#define INTRO_ARCHIVE			"gendata\\Hellfire.smk"
#define GAME_ID					((int)'HRTL')
#define GAME_VERSION			34
#define HELP_TITLE				"Hellfire Help"
#ifdef SPAWN
#define PROGRAM_NAME			"Diablo Shareware"
#else
#define PROGRAM_NAME			"Hellfire Retail"
#endif
#else
#define LOGO_DATA				"Data\\Diabsmal.CEL"
#define LOGO_WIDTH				296
#define INTRO_ARCHIVE			"gendata\\diablo1.smk"
#define GAME_ID					((int)'DRTL')
#define GAME_VERSION			42
#define HELP_TITLE				"Diablo Help"
#ifdef SPAWN
#define PROGRAM_NAME			"Diablo Shareware"
#else
#define PROGRAM_NAME			"Diablo Retail"
#endif
#endif

// MAXDUN = DSIZE + 2 * DBORDER
// DSIZE = 2 * DMAX
#define DMAXX					40
#define DMAXY					40
#define DBORDERX				16
#define DBORDERY				16
#define DSIZEX					80
#define DSIZEY					80
#define MAXDUNX					112
#define MAXDUNY					112

#define LIGHTSIZE				6912 // 27 * 256

#define GMENU_SLIDER			0x40000000
#define GMENU_ENABLED			0x80000000

// must be unsigned to generate unsigned comparisons with pnum
#define MAX_PLRS				4
#define MAX_MINIONS				MAX_PLRS

#define MAX_CHARACTERS			99
#ifdef HELLFIRE
#define NUMLEVELS				25
#define MAX_LVLS				24
#define MAX_LVLMTYPES			24
#define MAXQUESTS				24
#define MAXMULTIQUESTS			10
#define MAXTRIGGERS				7
#else
#define NUMLEVELS				17
#define MAX_LVLS				16
#define MAX_LVLMTYPES			16
#define MAXQUESTS				16
#define MAXMULTIQUESTS			4
#define MAXTRIGGERS				5
#endif

#define MAX_CHUNKS				(MAX_LVLS + 5)

// #define MAX_PATH				260
#define MAX_SEND_STR_LEN		80

#define MAXDEAD					31
#define MAXITEMS				127
#define MAXBELTITEMS			8
#define MAXLIGHTS				32
#define MAXMISSILES				125
#define MAXMONSTERS				200
#define MAXOBJECTS				127
#define MAXPORTAL				MAX_PLRS
#define MAXTHEMES				50
#define MAXTILES				2048
#define MAXVISION				32
#define MDMAXX					40
#define MDMAXY					40
#define MAXCHARLEVEL			50
#define MAXSPLLEVEL				15
#ifdef HELLFIRE
#define ITEMTYPES				43
#define BASESTAFFCHARGES		18
#else
#define ITEMTYPES				35
#define BASESTAFFCHARGES		40
#endif

// number of inventory grid cells
#define NUM_INV_GRID_ELEM		40
#define INV_SLOT_SIZE_PX		28

// Item indestructible durability
#define DUR_INDESTRUCTIBLE		255

#define VOLUME_MIN				-1600
#define VOLUME_MAX				0

#define NUM_TOWNERS				16

// todo: enums
#ifdef HELLFIRE
#define HEALER_ITEMS			20
#define BOY_MAX_VALUE			200000
#define WITCH_ITEMS				25
#define WITCH_MAX_VALUE			200000
#define SMITH_ITEMS				25
#define SMITH_PREMIUM_ITEMS		15
#define SMITH_MAX_VALUE			200000
#define SMITH_MAX_PREMIUM_VALUE 200000
#define STORE_LINES				104
#else
#define HEALER_ITEMS			20
#define BOY_MAX_VALUE			90000
#define WITCH_ITEMS				20
#define WITCH_MAX_VALUE			140000
#define SMITH_ITEMS				20
#define SMITH_PREMIUM_ITEMS		6
#define SMITH_MAX_VALUE			140000
#define SMITH_MAX_PREMIUM_VALUE 140000
#define STORE_LINES				24
#endif

// from diablo 2 beta
#define MAXRESIST				75

#define GOLD_SMALL_LIMIT		1000
#define GOLD_MEDIUM_LIMIT		2500
#define GOLD_MAX_LIMIT			5000

#define PLR_NAME_LEN			32

#define MAXPATHNODES			300

#define MAX_PATH_LENGTH			25

// 256 kilobytes + 3 bytes (demo leftover) for file magic (262147)
// final game uses 4-byte magic instead of 3
#define FILEBUFF				((256 * 1024) + 3)

#define PMSG_COUNT				8

// Diablo uses a 256 color palette
// Entry 0-127 (0x00-0x7F) are level specific
// Entry 128-255 (0x80-0xFF) are global

// standard palette for all levels
// 8 or 16 shades per color
// example (dark blue): PAL16_BLUE+14, PAL8_BLUE+7
// example (light red): PAL16_RED+2, PAL8_RED
// example (orange): PAL16_ORANGE+8, PAL8_ORANGE+4
#define PAL8_BLUE		128
#define PAL8_RED		136
#define PAL8_YELLOW		144
#define PAL8_ORANGE		152
#define PAL16_BEIGE		160
#define PAL16_BLUE		176
#define PAL16_YELLOW	192
#define PAL16_ORANGE	208
#define PAL16_RED		224
#define PAL16_GRAY		240

#ifndef DEFAULT_WIDTH
#define DEFAULT_WIDTH	640
#endif
#ifndef DEFAULT_HEIGHT
#define DEFAULT_HEIGHT	480
#endif

#define SCREEN_WIDTH	dvl::screenWidth
#define SCREEN_HEIGHT	dvl::screenHeight

// If defined, use 32-bit colors instead of 8-bit [Default -> Undefined]
//#define RGBMODE

#ifndef RGBMODE
#define SCREEN_BPP		8
#else
#define SCREEN_BPP		32
#endif

#define BORDER_LEFT		64
#define BORDER_TOP		160
#define BORDER_RIGHT	dvl::borderRight
#define BORDER_BOTTOM	16

#define SCREEN_X		BORDER_LEFT
#define SCREEN_Y		BORDER_TOP

#define BUFFER_WIDTH	(BORDER_LEFT + SCREEN_WIDTH + BORDER_RIGHT)
#define BUFFER_HEIGHT	(BORDER_TOP + SCREEN_HEIGHT + BORDER_BOTTOM)

#define UI_OFFSET_Y		((SCREEN_HEIGHT - 480) / 2)

#define TILE_WIDTH		64
#define TILE_HEIGHT		32

#define PANEL_WIDTH     640
#define PANEL_HEIGHT    128
#define PANEL_TOP		(SCREEN_HEIGHT - PANEL_HEIGHT)
#define PANEL_LEFT		(SCREEN_WIDTH - PANEL_WIDTH) / 2
#define PANEL_X			(SCREEN_X + PANEL_LEFT)
#define PANEL_Y			(SCREEN_Y + PANEL_TOP)

#define SPANEL_WIDTH	 294
#define SPANEL_HEIGHT	 298
//#define PANELS_COVER (SCREEN_WIDTH <= PANEL_WIDTH && SCREEN_HEIGHT <= SPANEL_HEIGHT + PANEL_HEIGHT)
#define PANELS_COVER	FALSE

#define RIGHT_PANEL		(SCREEN_WIDTH - SPANEL_WIDTH)
#define RIGHT_PANEL_X	(SCREEN_X + RIGHT_PANEL)

#define VIEWPORT_HEIGHT dvl::viewportHeight

#define DIALOG_TOP		((SCREEN_HEIGHT - PANEL_HEIGHT) / 2 - 18)
#define DIALOG_Y		(SCREEN_Y + DIALOG_TOP)

#define SCREENXY(x, y) ((x) + SCREEN_X + ((y) + SCREEN_Y) * BUFFER_WIDTH)

#define MENUBTN_WIDTH	71
#define MENUBTN_HEIGHT	19
#define CHRBTN_WIDTH	41
#define CHRBTN_HEIGHT	22
#define SPLICONLENGTH	56
#define SPLROWICONLS	10
#ifdef HELLFIRE
#define SPLICONLAST		52
#define SPLBOOKTABS		5
#else
#define SPLICONLAST		43
#define SPLBOOKTABS		4
#endif

#ifdef SPAWN
#define MONST_AVAILABILITY_MASK	1
#else
#define MONST_AVAILABILITY_MASK	3
#endif

#define NIGHTMARE_TO_HIT_BONUS  85
#define HELL_TO_HIT_BONUS      120

#define NIGHTMARE_AC_BONUS 50
#define HELL_AC_BONUS      80

#define IN_DUNGEON_AREA(x, y) \
	(x >= 0                   \
	&& x < MAXDUNX            \
	&& y >= 0                 \
	&& y < MAXDUNY)

#define MemFreeDbg(p)       \
	{                       \
		void *p__p;         \
		p__p = p;           \
		p = NULL;           \
		mem_free_dbg(p__p); \
	}

#undef assert

#ifdef _DEBUG
#define assert(exp) (void)((exp) || (assert_fail(__LINE__, __FILE__, #exp), 0))
#elif defined(_DEVMODE)
#define assert(exp) (void)((exp) || (app_fatal("Assert fail at %d, %s, %s", __LINE__, __FILE__, #exp), 0))
#else
#define assert(exp) ((void)0)
#endif

#ifdef _MSC_VER
#ifdef _DEVMODE
#define ASSUME_UNREACHABLE assert(0);
#else
#define ASSUME_UNREACHABLE __assume(0);
#endif
#elif defined(__clang__)
#define ASSUME_UNREACHABLE __builtin_unreachable();
#elif defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 405
#define ASSUME_UNREACHABLE __builtin_unreachable();
#else
#define ASSUME_UNREACHABLE assert(0);
#endif
#endif

#ifdef _DEBUG
#undef ASSUME_UNREACHABLE
#define ASSUME_UNREACHABLE assert(0);
#endif

#define ERR_DLG(title, text) ErrDlg(title, text, __FILE__, __LINE__)

// To apply to certain functions which have local variables aligned by 1 for unknown yet reason
#if (_MSC_VER == 1200)
#define ALIGN_BY_1 __declspec(align(1))
#else
#define ALIGN_BY_1
#endif

#define SwapLE32 SDL_SwapLE32
#define SwapLE16 SDL_SwapLE16

#define ErrSdl() ErrDlg("SDL Error", SDL_GetError(), __FILE__, __LINE__)

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif
