/**
 * @file spelldat.cpp
 *
 * Implementation of all spell data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef HELLFIRE
#define ICN_BLK 45
#define ICN_ATK 48
#define ICN_SWP 34
#define ICN_RTK 48
#define ICN_RPB 19
#define ICN_RAS 31
#else
#define ICN_BLK 19
#define ICN_ATK 22
#define ICN_SWP 34
#define ICN_RTK 22
#define ICN_RPB 17
#define ICN_RAS 31
#endif

/*
	unused icons :	20 (teleport?)
					22 - SPL_ETHEREALIZE (only in standard)
					25 - SPL_APOCA
					30 - SPL_BONESPIRIT
					32 (red circle with a star)
					33 (red burning stair?)
					// hellfire only
					51 - SPL_MANA/MAGI
					50 - SPL_JESTER
					43 - SPL_WARP
					44 - SPL_SEARCH
*/

/*
 * Data related to each spell ID.
 * sIcon: spelicon.cel frame number.
 */
SpellData spelldata[NUM_SPELLS] = {
	// clang-format off
	// sName,    sManaCost, sType,           sIcon,   sNameText,            sBookLvl, sStaffLvl, sScrollLvl, sTargeted, scCurs,             spCurs,             sFlags,                   sMinInt, sSFX,     sMissile,   sManaAdj, sMinMana, sStaffMin, sStaffMax, sBookCost, sStaffCost
	{ SPL_NULL,          0, 0,               27,      NULL,                 SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_MELEE | SFLAG_RANGED,     0, 0,        0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_WALK,          0, STYPE_NONE,      28,      "Walk",               SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_ANY,                      0, 0,        0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_BLOCK,         0, STYPE_NONE,      ICN_BLK, "Block",              SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_BLOCK,    0, 0,        0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_ATTACK,        0, STYPE_NONE,      ICN_ATK, "Attack",             SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_MELEE,    0, 0,        0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_SWIPE,         2, STYPE_NONE,      ICN_SWP, "Swipe",                     6,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_MELEE,    0, 0,        0,                 0,        2,         0,         0,      8000,          0 },
	{ SPL_RATTACK,       0, STYPE_NONE,      ICN_RTK, "Ranged Attack",      SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,   0, 0,        MIS_ARROWC,        0,        0,         0,         0,         0,          0 },
	{ SPL_POINT_BLANK,   2, STYPE_NONE,      ICN_RPB, "Point Blank",               4,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,   0, 0,        MIS_PBARROWC,      0,        2,         0,         0,      5000,          0 },
	{ SPL_FAR_SHOT,      2, STYPE_NONE,      ICN_RAS, "Far Shot",                  8,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,   0, 0,        MIS_ASARROWC,      0,        2,         0,         0,     10000,          0 },
	{ SPL_FIREBOLT,      6, STYPE_FIRE,      1,       "Firebolt",           BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 15, IS_CAST2, MIS_FIREBOLT,      1,        3,        40,        80,      1000,         50 },
	{ SPL_CBOLT,         6, STYPE_LIGHTNING, 39,      "Charged Bolt",       BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 25, IS_CAST2, MIS_CBOLTC,        1,        4,        40,        80,      1000,         50 },
	{ SPL_HBOLT,         7, STYPE_MAGIC,     42,      "Acidbolt",           BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 20, IS_CAST2, MIS_HBOLT,         1,        4,        40,        80,      1000,         50 },
	{ SPL_LIGHTNING,    15, STYPE_LIGHTNING, 3,       "Lightning",                 4,         6,          4, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 20, IS_CAST4, MIS_LIGHTNINGC,    1,        8,        20,        60,      3000,        150 },
	{ SPL_FLASH,        30, STYPE_LIGHTNING, 4,       "Flash",                     5,         8,          6, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 33, IS_CAST4, MIS_FLASH,         3,       16,        20,        40,      7500,        500 },
	{ SPL_FIREWALL,     28, STYPE_FIRE,      6,       "Fire Wall",                 3,         4,          4, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 27, IS_CAST2, MIS_FIREWALLC,     3,       16,         8,        16,      6000,        400 },
	{ SPL_FIREBALL,     16, STYPE_FIRE,      12,      "Fireball",                  8,        14,          8, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 48, IS_CAST2, MIS_FIREBALL,      2,       10,        40,        80,      8000,        300 },
	{ SPL_CHAIN,        30, STYPE_LIGHTNING, 16,      "Chain Lightning",           8,        14,         10, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 54, IS_CAST2, MIS_CHAIN,         2,       18,        20,        60,     11000,        750 },
	{ SPL_WAVE,         35, STYPE_FIRE,      14,      "Flame Wave",                9,        16,         10, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 54, IS_CAST2, MIS_FIREWAVEC,     4,       20,        20,        40,     10000,        650 },
	{ SPL_NOVA,         60, STYPE_MAGIC,     11,      "Nova",                     14,        20,         14, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 87, IS_CAST4, MIS_LIGHTNOVAC,    5,       35,        16,        32,     21000,       1300 },
	{ SPL_FLAME,        11, STYPE_FIRE,      15,      "Inferno",                   3,         4,   SCRL_MIN, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 20, IS_CAST2, MIS_FLAMEC,        1,        6,        20,        40,      2000,        100 },
	{ SPL_ELEMENT,      35, STYPE_FIRE,      38,      "Elemental",                 8,        12,   SPELL_NA, TRUE,      CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 68, IS_CAST2, MIS_ELEMENT,       4,       20,        20,        60,     10500,        700 },
	{ SPL_FLARE,        25, STYPE_MAGIC,     36,      "Blood Star",               14,        26,   SPELL_NA, TRUE,      CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 70, IS_CAST2, MIS_FLARE,         4,       14,        20,        60,     27500,       1800 },
	{ SPL_GUARDIAN,     50, STYPE_FIRE,      18,      "Guardian",                  9,        16,         12, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 61, IS_CAST2, MIS_GUARDIAN,      4,       30,        16,        32,     14000,        950 },
	{ SPL_GOLEM,       100, STYPE_FIRE,      21,      "Golem",                    11,        18,         10, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 81, IS_CAST2, MIS_GOLEM,        10,       60,        16,        32,     18000,       1100 },
	{ SPL_STONE,        60, STYPE_MAGIC,     8,       "Stone Curse",               6,        10,          6, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 51, IS_CAST2, MIS_STONE,         4,       40,         8,        16,     12000,        800 },
	{ SPL_INFRA,         0, STYPE_MAGIC,     9,       "Infravision",        SPELL_NA,  SPELL_NA,          8, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 43, IS_CAST8, MIS_INFRA,         0,        0,         0,         0,         0,        600 },
	{ SPL_MANASHIELD,   33, STYPE_MAGIC,     13,      "Mana Shield",               6,        10,          8, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_ANY,                     25, IS_CAST2, MIS_MANASHIELD,    0,       33,         4,        10,     16000,       1200 },
	{ SPL_TELEKINESIS,  15, STYPE_MAGIC,     40,      "Telekinesis",               2,         4,   SPELL_NA, TRUE,      CURSOR_TELEKINESIS, CURSOR_NONE,        SFLAG_DUNGEON,                 33, IS_CAST2, MIS_TELEKINESIS,   2,        8,        20,        40,      2500,        200 },
	{ SPL_TELEPORT,     35, STYPE_MAGIC,     24,      "Teleport",                 14,        24,         14, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_ANY,                    105, IS_CAST6, MIS_TELEPORT,      5,       15,        16,        32,     20000,       1250 },
	{ SPL_RNDTELEPORT,  11, STYPE_MAGIC,     28,      "Phasing",                   7,        12,          6, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 39, IS_CAST2, MIS_RNDTELEPORT,   1,        4,        40,        80,      3500,        200 },
	{ SPL_TOWN,         35, STYPE_MAGIC,     7,       "Town Portal",               3,         6,          4, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 20, IS_CAST6, MIS_TOWN,          3,       18,         8,        12,      3000,        200 },
	{ SPL_HEAL,          5, STYPE_MAGIC,     2,       "Healing",            BOOK_MIN, STAFF_MIN,   SCRL_MIN, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_ANY,                     17, IS_CAST8, MIS_HEAL,          6,        1,        20,        40,      1000,         50 },
	{ SPL_HEALOTHER,     5, STYPE_MAGIC,     10,      "Heal Other",         BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_HEALOTHER,   CURSOR_HEALOTHER,   SFLAG_ANY,                     17, IS_CAST8, MIS_HEALOTHER,     6,        1,        20,        40,      1000,         50 },
	{ SPL_RESURRECT,     0, STYPE_MAGIC,     41,      "Resurrect",          SPELL_NA,        10,   SCRL_MIN, TRUE,      CURSOR_RESURRECT,   CURSOR_RESURRECT,   SFLAG_ANY,                     20, IS_CAST8, MIS_RESURRECT,     0,        0,         4,        10,      4000,        250 },
	{ SPL_IDENTIFY,      0, STYPE_MAGIC,     5,       "Identify",           SPELL_NA,  SPELL_NA,   SCRL_MIN, FALSE,     CURSOR_IDENTIFY,    CURSOR_IDENTIFY,    SFLAG_ANY,                     20, IS_CAST6, MIS_IDENTIFY,      0,        0,         0,         0,         0,        100 },
	{ SPL_REPAIR,        0, STYPE_MAGIC,     26,      "Item Repair",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_REPAIR,      CURSOR_REPAIR,      SFLAG_ANY,                      0, IS_CAST6, MIS_REPAIR,        0,        0,         0,         0,         0,          0 },
	{ SPL_RECHARGE,      0, STYPE_MAGIC,     29,      "Staff Recharge",     SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_RECHARGE,    CURSOR_RECHARGE,    SFLAG_ANY,                      0, IS_CAST6, MIS_ABILITY,       0,        0,         0,         0,         0,          0 },
	{ SPL_DISARM,        0, STYPE_MAGIC,     37,      "Trap Disarm",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_DISARM,      CURSOR_DISARM,      SFLAG_DUNGEON,                  0, IS_CAST6, MIS_DISARM,        0,        0,         0,         0,         0,          0 },
#ifdef HELLFIRE
	{ SPL_BLODBOIL,      0, STYPE_MAGIC,     23,      "Rage",               SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                  0, IS_CAST8, MIS_BLODBOIL,      0,        0,         0,         0,         0,          0 },
	{ SPL_WHITTLE,       0, STYPE_MAGIC,     40,      "Staff Whittle",      SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_RECHARGE,    CURSOR_RECHARGE,    SFLAG_ANY,                      0, IS_CAST6, MIS_ABILITY,       0,        0,         0,         0,         0,          0 },
	{ SPL_LIGHTWALL,    28, STYPE_LIGHTNING, 46,      "Lightning Wall",            3,         4,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        SFLAG_DUNGEON,                 27, IS_CAST4, MIS_LIGHTWALLC,    3,       16,         8,        16,      6000,        400 },
	{ SPL_IMMOLAT,      60, STYPE_FIRE,      47,      "Immolation",               14,        20,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 87, IS_CAST2, MIS_FIRENOVAC,     5,       35,        16,        32,     21000,       1300 },
	{ SPL_FIRERING,     28, STYPE_FIRE,      49,      "Ring of Fire",              5,        10,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        SFLAG_DUNGEON,                 27, IS_CAST2, MIS_FIRERING,      3,       16,         8,        16,      6000,        400 },
	{ SPL_RUNEFIRE,      0, STYPE_MAGIC,     35,      "Rune of Fire",       SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    SFLAG_DUNGEON,                 48, IS_CAST8, MIS_RUNEFIRE,      0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNELIGHT,     0, STYPE_MAGIC,     35,      "Rune of Light",      SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    SFLAG_DUNGEON,                 48, IS_CAST8, MIS_RUNELIGHT,     0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNENOVA,      0, STYPE_MAGIC,     35,      "Rune of Nova",       SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    SFLAG_DUNGEON,                 48, IS_CAST8, MIS_RUNENOVA,      0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNEIMMOLAT,   0, STYPE_MAGIC,     35,      "Rune of Immolation", SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    SFLAG_DUNGEON,                 48, IS_CAST8, MIS_RUNEIMMOLAT,   0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNESTONE,     0, STYPE_MAGIC,     35,      "Rune of Stone",      SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    SFLAG_DUNGEON,                 48, IS_CAST8, MIS_RUNESTONE,     0,        0,         0,         0,      8000,        300 },
#endif
	// clang-format on
};

DEVILUTION_END_NAMESPACE
