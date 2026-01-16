/**
 * @file defs.h
 *
 * Global definitions and Macros.
 */
#ifndef _DEFS_H
#define _DEFS_H

#ifndef ASSET_MPL
#define ASSET_MPL                1
#endif

#define DATA_ARCHIVE_MAIN        "diabdat.mpq"
#define DATA_ARCHIVE_MAIN_ALT    "DIABDAT.MPQ"
#define DATA_ARCHIVE_PATCH       "patch_rt.mpq"
#define DATA_ARCHIVE_MAX_PATH    128

#ifdef HELLFIRE
#define INTRO_ARCHIVE            "gendata\\Hellfire.smk"
#define GAME_ID                  ((int)'HRTL')
#define GAME_VERSION             34
#define CREDITS_LINE_COUNT       91
#define CREDITS_TXT              "Meta\\credits_hf.txt"
#define HELP_TITLE               "Hellfire Help"
#else
#define INTRO_ARCHIVE            "gendata\\diablo1.smk"
#define GAME_ID                  ((int)'DRTL')
#define GAME_VERSION             42
#define CREDITS_LINE_COUNT       455
#define CREDITS_TXT              "Meta\\credits.txt"
#define HELP_TITLE               "Diablo Help"
#endif
#define HELP_LINE_COUNT          65
#define HELP_TXT                 "Meta\\help.txt"

// MAXDUN = DSIZE + 2 * DBORDER
// DSIZE = 2 * DMAX
#define DMAXX                    40
#define DMAXY                    40
#define DBORDERX                 16
#define DBORDERY                 16
#define DSIZEX                   80
#define DSIZEY                   80
#define MAXDUNX                  112
#define MAXDUNY                  112
/** The size of the quads in hell. */
static_assert(DMAXX % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even width.");
#define L4BLOCKX (DMAXX / 2)
static_assert(DMAXY % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even height.");
#define L4BLOCKY (DMAXY / 2)

#ifndef NONET
// must be unsigned to generate unsigned comparisons with pnum
#define MAX_PLRS                 4
#define NONETCONST
#else
#define MAX_PLRS                 1
#define NONETCONST               const
#endif
#define MAX_MINIONS              MAX_PLRS
#define PLR_NONE                 0xFF
#define PLR_VALID(x) ((int8_t)x >= 0)

#define MAX_CHARACTERS           99
#define MAX_TOWNERS              16
#define STORE_TOWNERS            8
#define MAX_LVLMTYPES            12
#define MAX_LVLMIMAGE            4000

#ifdef HELLFIRE
#define MAXTRIGGERS              7
#else
#define MAXTRIGGERS              5
#endif
#define TRIG_NONE                -1
#define TRIG_VALID(x) (x >= 0)

// the maximum size of the turn packets
#define NET_TURN_MSG_SIZE       512
// the maximum size of a normal (nmsg) message
#define NET_NORMAL_MSG_SIZE     512
// the maximum size of a large (nmsg) message used during delta-load
#define NET_LARGE_MSG_SIZE      0x8000
// the minimum size of a large message which needs to be compressed
#define NET_COMP_MSG_SIZE       256
// the default port if there is no corresponding entry in the .ini (tcp)
#define NET_DEFAULT_PORT        6112
// the maximum length of the name of an 'instance'
#define NET_MAX_GAMENAME_LEN    31
// the length of the name of an 'instance' in case of zerotier
#define NET_ZT_GAMENAME_LEN     6
// the maximum length of the password of an 'instance'
#define NET_MAX_PASSWD_LEN      15
// the maximum length of a text-message to other players
#define MAX_SEND_STR_LEN        80
// the length of the port-string (tcp)
#define NET_TCP_PORT_LENGTH     5
// the number of turns to let the clients sync a joining player
#define NET_JOIN_WINDOW         30
// the number of turns to wait for level-deltas
#define NET_JOIN_TIMEOUT        30
// seconds between the timeout-checks (tcp)
#define NET_TIMEOUT_BASE        1
// number of iterations before a pending connection timeouts (tcp)
#define NET_TIMEOUT_CONNECT     30
// number of iterations before an active connection timeouts (tcp)
#define NET_TIMEOUT_ACTIVE      60
// number of iterations before a ghost connection timeouts (tcp + zt)
#define NET_TIMEOUT_GHOST       5
// seconds to wait if there is no free connection (tcp)
#define NET_WAIT_PENDING        10
// number of milliseconds before a pending connection timeouts (zt)
#define NET_TIMEOUT_SOCKET      (5 * 1000)
// seconds after a game-info becomes obsolete (zt)
#define NET_TIMEOUT_GAME        5

#define DEAD_MULTI              0xFF
#define MAXITEMS                127
#define ITEM_NONE               0xFF
#define ITEM_VALID(x) ((int8_t)x >= 0)
#define MAXBELTITEMS            8
#define MAXLIGHTS               32
#define MAXMISSILES             125
#define MIS_MULTI               0xFF
#define MAXMONSTERS             200
#define MON_NONE                -1
#define MON_VALID(x) (x >= 0)
#define MAXOBJECTS              127
#define OBJ_NONE                0xFF
#define OBJ_VALID(x) ((int8_t)x >= 0)
#define MAXPORTAL               MAX_PLRS
#define MAXTHEMES               8
#define MAXTILES                255
#define MAXSUBTILES             1023
#define MAXVISION               (MAX_PLRS + MAX_MINIONS)
#define MAXCHARLEVEL            50
#define MAXSPLLEVEL             15
#ifdef HELLFIRE
#define BASESTAFFCHARGES        18
#else
#define BASESTAFFCHARGES        40
#endif

#define MAXCAMPAIGNLVL          60
#define MAXCAMPAIGNSIZE         16

// number of inventory grid cells
#define NUM_INV_GRID_ELEM       40

// Item indestructible durability
#define DUR_INDESTRUCTIBLE      255

// sfx constants
#define VOLUME_MIN              0
#define VOLUME_MAX              1024
#define SFX_DIST_MAX            16
#define SFX_STREAM_CHANNEL      0

// todo: enums
#define HEALER_ITEMS            16
#define BOY_MAX_VALUE           200000
#define WITCH_ITEMS             24
#define WITCH_MAX_VALUE         140000
#define SMITH_ITEMS             16
#define SMITH_PREMIUM_ITEMS     8
#define SMITH_MAX_VALUE         140000
#define SMITH_MAX_PREMIUM_VALUE 140000

// from diablo 2 beta
#define MAXRESIST               75
#define PLR_MIN_VISRAD          10

#define GOLD_SMALL_LIMIT        1000
#define GOLD_MEDIUM_LIMIT       2500
#define GOLD_MAX_LIMIT          5000

#define PLR_NAME_LEN            16

#define MAXPATHNODES            256

#define MAX_PATH_LENGTH         23

// 272 kilobytes .. (was 256 kb in vanilla which is not safe)
#define FILEBUFF                (272 * 1024)

// Diablo uses a 256 color palette
// Entry 0-127 (0x00-0x7F) are level specific
// Entry 128-255 (0x80-0xFF) are global
#define NUM_COLORS        256

// standard palette for all levels
// 8 or 16 shades per color
// example (dark blue): PAL16_BLUE+14, PAL8_BLUE+7
// example (light red): PAL16_RED+2, PAL8_RED
// example (orange): PAL16_ORANGE+8, PAL8_ORANGE+4
#define PAL_BLACK         0
#define PAL8_BLUE         128
#define PAL8_RED          136
#define PAL8_YELLOW       144
#define PAL8_ORANGE       152
#define PAL16_BEIGE       160
#define PAL16_BLUE        176
#define PAL16_YELLOW      192
#define PAL16_ORANGE      208
#define PAL16_RED         224
#define PAL16_GRAY        240

#define NIGHTMARE_LEVEL_BONUS   16
#define HELL_LEVEL_BONUS        32

/*#define NIGHTMARE_TO_HIT_BONUS  20
#define HELL_TO_HIT_BONUS       40

#define NIGHTMARE_AC_BONUS 20
#define HELL_AC_BONUS      40

#define NIGHTMARE_EVASION_BONUS 20
#define HELL_EVASION_BONUS      40

#define NIGHTMARE_MAGIC_BONUS 20
#define HELL_MAGIC_BONUS      40*/

#define POS_IN_RECT(x, y, rx, ry, rw, rh) \
    ((x) >= (rx)                          \
    && (x) < (rx + rw)                    \
    && (y) >= (ry)                        \
    && (y) < (ry + rh))

#define IN_DUNGEON_AREA(x, y) \
    ((unsigned)(x) < MAXDUNX  \
    && (unsigned)(y) < MAXDUNY)

#define IN_ACTIVE_AREA(x, y)   \
    ((x) >= DBORDERX           \
    && (x) < DBORDERX + DSIZEX \
    && (y) >= DBORDERY         \
    && (y) < DBORDERY + DSIZEY)

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

#undef assert

#if DEBUG_MODE || DEV_MODE
#define assert(exp) (void)((exp) || (app_fatal("Assert fail at %d, %s, %s", __LINE__, __FILE__, #exp), 0))
#else
#define assert(exp) do { (void) sizeof(exp); } while(0)
#endif

#ifdef _MSC_VER
#define ASSUME_UNREACHABLE __assume(0);
#elif defined(__clang__)
#define ASSUME_UNREACHABLE __builtin_unreachable();
#elif defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 405
#define ASSUME_UNREACHABLE __builtin_unreachable();
#else
#define ASSUME_UNREACHABLE assert(0);
#endif
#endif

#if DEBUG_MODE || DEV_MODE
#undef ASSUME_UNREACHABLE
#define ASSUME_UNREACHABLE assert(0);
#endif

#if INET_MODE
#define net_assert(x) assert(x)
#else
#define net_assert(x) do { (void) sizeof(x); } while(0)
#endif

#define SwapLE64 SDL_SwapLE64
#define SwapLE32 SDL_SwapLE32
#define SwapLE16 SDL_SwapLE16

#ifdef __has_attribute
#define DVL_HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define DVL_HAVE_ATTRIBUTE(x) 0
#endif

#if DVL_HAVE_ATTRIBUTE(always_inline) ||  (defined(__GNUC__) && !defined(__clang__))
#define DVL_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define DVL_ATTRIBUTE_ALWAYS_INLINE
#endif

#if DVL_HAVE_ATTRIBUTE(format) || (defined(__GNUC__) && !defined(__clang__))
#define DVL_PRINTF_ATTRIBUTE(fmtargnum, firstarg) \
  __attribute__((__format__(__printf__, fmtargnum, firstarg)))
#else
#define DVL_PRINTF_ATTRIBUTE(fmtargnum, firstarg)
#endif

#if DVL_HAVE_ATTRIBUTE(hot)
#define DVL_ATTRIBUTE_HOT __attribute__((hot))
#else
#define DVL_ATTRIBUTE_HOT
#endif

#ifdef _MSC_VER
#define DVL_RESTRICT __restrict
#else
#define DVL_RESTRICT __restrict__
#endif

#if defined(_MSC_VER) && !defined(NXDK)
#define DIAG_PRAGMA(x)                                            __pragma(warning(x))
#define DISABLE_WARNING(gcc_unused, clang_unused, msvc_errorcode) DIAG_PRAGMA(push) DIAG_PRAGMA(disable:##msvc_errorcode)
#define ENABLE_WARNING(gcc_unused, clang_unused, msvc_errorcode)  DIAG_PRAGMA(pop)
//#define DISABLE_WARNING(gcc_unused,clang_unused,msvc_errorcode) __pragma(warning(suppress: msvc_errorcode))
//#define ENABLE_WARNING(gcc_unused,clang_unused,msvc_unused) ((void)0)
#else
#define DIAG_STR(s)              #s
#define DIAG_JOINSTR(x, y)       DIAG_STR(x ## y)
#define DO_DIAG_PRAGMA(x)        _Pragma(#x)
#define DIAG_PRAGMA(compiler, x) DO_DIAG_PRAGMA(compiler diagnostic x)
#if defined(__clang__)
# define DISABLE_WARNING(gcc_unused, clang_option, msvc_unused) DIAG_PRAGMA(clang, push) DIAG_PRAGMA(clang, ignored DIAG_JOINSTR(-W, clang_option))
# define ENABLE_WARNING(gcc_unused, clang_option, msvc_unused)  DIAG_PRAGMA(clang, pop)
#elif defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
# define DISABLE_WARNING(gcc_option, clang_unused, msvc_unused) DIAG_PRAGMA(GCC, push) DIAG_PRAGMA(GCC, ignored DIAG_JOINSTR(-W, gcc_option))
# define ENABLE_WARNING(gcc_option, clang_unused, msvc_unused)  DIAG_PRAGMA(GCC, pop)
#else
# define DISABLE_WARNING(gcc_option, clang_unused, msvc_unused) DIAG_PRAGMA(GCC, ignored DIAG_JOINSTR(-W, gcc_option))
# define ENABLE_WARNING(gcc_option, clang_option, msvc_unused)  DIAG_PRAGMA(GCC, warning DIAG_JOINSTR(-W, gcc_option))
#endif
#else
#define DISABLE_WARNING(gcc_unused, clang_unused, msvc_unused) ;
#define ENABLE_WARNING(gcc_unused, clang_unused, msvc_unused)  ;
#endif
#endif

#if DEBUG_MODE
#define DISABLE_SPEED_OPTIMIZATION
#define ENABLE_SPEED_OPTIMIZATION
#else
#if defined(_MSC_VER) && !defined(NXDK)
#define DISABLE_SPEED_OPTIMIZATION \
__pragma (optimize( "", off )) \
__pragma (optimize( "gsy", on ))
#define ENABLE_SPEED_OPTIMIZATION \
__pragma (optimize( "", on ))
#elif defined(__GNUC__) && !defined(__clang__)
#define DISABLE_SPEED_OPTIMIZATION \
_Pragma ("GCC push_options") \
_Pragma ("GCC optimize(\"Os\")")
#define ENABLE_SPEED_OPTIMIZATION \
_Pragma ("GCC pop_options")
#else
#define DISABLE_SPEED_OPTIMIZATION
#define ENABLE_SPEED_OPTIMIZATION
#endif
#endif // DEBUG_MODE

#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#endif

#ifndef EX_OK
#define EX_OK 0
#endif
#ifndef EX_USAGE
#if __linux__
#define EX_USAGE 64
#else
#define EX_USAGE 2
#endif
#endif
#ifndef EX_SOFTWARE
#if __linux__
#define EX_SOFTWARE 71
#else
#define EX_SOFTWARE 1
#endif
#endif

#define DEVILUTION_BEGIN_NAMESPACE namespace dvl {
#define DEVILUTION_END_NAMESPACE }

#endif /* _DEFS_H */