/**
 * @file defs.h
 *
 * Global definitions and Macros.
 */
#ifndef _DEFS_H
#define _DEFS_H

#ifndef ASSET_MPL
#define ASSET_MPL				1
#endif

#define DATA_ARCHIVE_MAIN		"diabdat.mpq"
#define DATA_ARCHIVE_MAIN_ALT	"DIABDAT.MPQ"
#define DATA_ARCHIVE_PATCH		"patch_rt.mpq"
#define DATA_ARCHIVE_MAX_PATH	128

#ifdef HELLFIRE
#define MPQONE					"hellone.mpq"
#define INTRO_ARCHIVE			"gendata\\Hellfire.smk"
#define GAME_ID					((int)'HRTL')
#define GAME_VERSION			34
#define HELP_TITLE				"Hellfire Help"
#else
#define MPQONE					"diablone.mpq"
#define INTRO_ARCHIVE			"gendata\\diablo1.smk"
#define GAME_ID					((int)'DRTL')
#define GAME_VERSION			42
#define HELP_TITLE				"Diablo Help"
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
/** The size of the quads in hell. */
static_assert(DMAXX % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even width.");
#define L4BLOCKX (DMAXX / 2)
static_assert(DMAXY % 2 == 0, "DRLG_L4 constructs the dungeon by mirroring a quarter block -> requires to have a dungeon with even height.");
#define L4BLOCKY (DMAXY / 2)

// must be unsigned to generate unsigned comparisons with pnum
#define MAX_PLRS				4
#define MAX_MINIONS				MAX_PLRS
#define PLR_NONE				0xFF

#define MAX_CHARACTERS			99
#define MAX_TOWNERS				16
#define STORE_TOWNERS			8
#define MAX_LVLMTYPES			12
#define MAX_LVLMIMAGE			4000

#ifdef HELLFIRE
#define MAXTRIGGERS				7
#else
#define MAXTRIGGERS				5
#endif

// the maximum size of the turn packets
#define NET_TURN_MSG_SIZE		512
// the maximum size of a normal (nmsg) message
#define NET_NORMAL_MSG_SIZE		512
// the maximum size of a large (nmsg) message used during delta-load
#define NET_LARGE_MSG_SIZE		0x8000
// the minimum size of a large message which needs to be compressed
#define NET_COMP_MSG_SIZE		256
#define NET_DEFAULT_PORT		6112
#define MAX_SEND_STR_LEN		80

#define DEAD_MULTI				0xFF
#define MAXITEMS				127
#define ITEM_NONE				0xFF
#define MAXBELTITEMS			8
#define MAXLIGHTS				31
#define MAXMISSILES				125
#define MIS_MULTI				0xFF
#define MAXMONSTERS				200
#define MON_NONE				0xFF
#define MAXOBJECTS				127
#define OBJ_NONE				0xFF
#define MAXPORTAL				MAX_PLRS
#define MAXTHEMES				32
#define MAXTILES				2047
#define MAXVISION				(MAX_PLRS + MAX_MINIONS)
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

// Item indestructible durability
#define DUR_INDESTRUCTIBLE		255

// sfx constants
#define VOLUME_MIN				0
#define VOLUME_MAX				1024
#define SFX_DIST_MAX			16
#define SFX_STREAM_CHANNEL		0

// todo: enums
#define HEALER_ITEMS			16
#define BOY_MAX_VALUE			200000
#define WITCH_ITEMS				24
#define WITCH_MAX_VALUE			140000
#define SMITH_ITEMS				16
#define SMITH_PREMIUM_ITEMS		8
#define SMITH_MAX_VALUE			140000
#define SMITH_MAX_PREMIUM_VALUE 140000

// from diablo 2 beta
#define MAXRESIST				75
#define PLR_MIN_VISRAD			10

#define GOLD_SMALL_LIMIT		1000
#define GOLD_MEDIUM_LIMIT		2500
#define GOLD_MAX_LIMIT			5000

#define PLR_NAME_LEN			32

#define MAXPATHNODES			256

#define MAX_PATH_LENGTH			23

// 272 kilobytes .. (was 256 kb in vanilla which is not safe)
#define FILEBUFF				(272 * 1024)

// Diablo uses a 256 color palette
// Entry 0-127 (0x00-0x7F) are level specific
// Entry 128-255 (0x80-0xFF) are global
#define NUM_COLORS		256

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

#define DIFFICULTY_EXP_BONUS   800

#define NIGHTMARE_LEVEL_BONUS   16
#define HELL_LEVEL_BONUS        32

#define NIGHTMARE_TO_HIT_BONUS  85
#define HELL_TO_HIT_BONUS      120

#define NIGHTMARE_AC_BONUS 50
#define HELL_AC_BONUS      80

#define NIGHTMARE_EVASION_BONUS 35
#define HELL_EVASION_BONUS      50

#define NIGHTMARE_MAGIC_BONUS 35
#define HELL_MAGIC_BONUS      50

#define POS_IN_RECT(x, y, rx, ry, rw, rh) \
	((x) >= (rx)                          \
	&& (x) < (rx + rw)                    \
	&& (y) >= (ry)                        \
	&& (y) < (ry + rh))

#define IN_DUNGEON_AREA(x, y) \
	(x >= 0                   \
	&& x < MAXDUNX            \
	&& y >= 0                 \
	&& y < MAXDUNY)

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
#define assert(exp) ((void)0)
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
#define net_assert(x) do { } while(0)
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