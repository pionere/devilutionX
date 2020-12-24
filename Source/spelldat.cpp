/**
 * @file spelldat.cpp
 *
 * Implementation of all spell data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Data related to each spell ID. */
SpellData spelldata[NUM_SPELLS] = {
	// clang-format off
	// sName,    sManaCost, sType,           sNameText,            sBookLvl, sStaffLvl, sScrollLvl, sTargeted, sTownSpell, sMinInt, sSFX,     sMissiles[3],                         sManaAdj, sMinMana, sStaffMin, sStaffMax, sBookCost, sStaffCost
	{ SPL_NULL,          0, 0,               NULL,                 SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,            0, 0,        { 0,                 0,          0 },        0,        0,        40,        80,         0,          0 },
	{ SPL_FIREBOLT,      6, STYPE_FIRE,      "Firebolt",           BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      FALSE,           15, IS_CAST2, { MIS_FIREBOLT,      0,          0 },        1,        3,        40,        80,      1000,         50 },
	{ SPL_HEAL,          5, STYPE_MAGIC,     "Healing",            BOOK_MIN, STAFF_MIN,   SCRL_MIN, FALSE,     TRUE,            17, IS_CAST8, { MIS_HEAL,          0,          0 },        3,        1,        20,        40,      1000,         50 },
	{ SPL_LIGHTNING,    10, STYPE_LIGHTNING, "Lightning",                 4,         6,          4, TRUE,      FALSE,           20, IS_CAST4, { MIS_LIGHTNINGC,    0,          0 },        1,        6,        20,        60,      3000,        150 },
	{ SPL_FLASH,        30, STYPE_LIGHTNING, "Flash",                     5,         8,          6, FALSE,     FALSE,           33, IS_CAST4, { MIS_FLASH,         0,          0 },        2,       16,        20,        40,      7500,        500 },
	{ SPL_IDENTIFY,     13, STYPE_MAGIC,     "Identify",           SPELL_NA,  SPELL_NA,   SCRL_MIN, FALSE,     TRUE,            20, IS_CAST6, { MIS_IDENTIFY,      0,          0 },        2,        1,         8,        12,         0,        100 },
	{ SPL_FIREWALL,     28, STYPE_FIRE,      "Fire Wall",                 3,         4,          4, TRUE,      FALSE,           27, IS_CAST2, { MIS_FIREWALLC,     0,          0 },        2,       16,         8,        16,      6000,        400 },
	{ SPL_TOWN,         35, STYPE_MAGIC,     "Town Portal",               3,         6,          4, TRUE,      FALSE,           20, IS_CAST6, { MIS_TOWN,          0,          0 },        3,       18,         8,        12,      3000,        200 },
#ifdef SPAWN
	{ SPL_STONE,        60, STYPE_MAGIC,     "Stone Curse",        SPELL_NA,  SPELL_NA,          6, TRUE,      FALSE,           51, IS_CAST2, { MIS_STONE,         0,          0 },        3,       40,         8,        16,     12000,        800 },
#else
	{ SPL_STONE,        60, STYPE_MAGIC,     "Stone Curse",               6,        10,          6, TRUE,      FALSE,           51, IS_CAST2, { MIS_STONE,         0,          0 },        3,       40,         8,        16,     12000,        800 },
#endif
	{ SPL_INFRA,        40, STYPE_MAGIC,     "Infravision",        SPELL_NA,  SPELL_NA,          8, FALSE,     FALSE,           43, IS_CAST8, { MIS_INFRA,         0,          0 },        5,       20,         0,         0,         0,        600 },
	{ SPL_RNDTELEPORT,  12, STYPE_MAGIC,     "Phasing",                   7,        12,          6, FALSE,     FALSE,           39, IS_CAST2, { MIS_RNDTELEPORT,   0,          0 },        2,        4,        40,        80,      3500,        200 },
	{ SPL_MANASHIELD,   33, STYPE_MAGIC,     "Mana Shield",               6,        10,          8, FALSE,     FALSE,           25, IS_CAST2, { MIS_MANASHIELD,    0,          0 },        0,       33,         4,        10,     16000,       1200 },
	{ SPL_FIREBALL,     16, STYPE_FIRE,      "Fireball",                  8,        14,          8, TRUE,      FALSE,           48, IS_CAST2, { MIS_FIREBALL,      0,          0 },        1,       10,        40,        80,      8000,        300 },
#ifdef SPAWN
	{ SPL_GUARDIAN,     50, STYPE_FIRE,      "Guardian",           SPELL_NA,  SPELL_NA,         12, TRUE,      FALSE,           61, IS_CAST2, { MIS_GUARDIAN,      0,          0 },        2,       30,        16,        32,     14000,        950 },
#else
	{ SPL_GUARDIAN,     50, STYPE_FIRE,      "Guardian",                  9,        16,         12, TRUE,      FALSE,           61, IS_CAST2, { MIS_GUARDIAN,      0,          0 },        2,       30,        16,        32,     14000,        950 },
#endif
	{ SPL_CHAIN,        30, STYPE_LIGHTNING, "Chain Lightning",           8,        14,         10, FALSE,     FALSE,           54, IS_CAST2, { MIS_CHAIN,         0,          0 },        1,       18,        20,        60,     11000,        750 },
	{ SPL_WAVE,         35, STYPE_FIRE,      "Flame Wave",                9,        16,         10, TRUE,      FALSE,           54, IS_CAST2, { MIS_FIREWAVEC,     0,          0 },        3,       20,        20,        40,     10000,        650 },
	{ SPL_DOOMSERP,      0, STYPE_LIGHTNING, "Doom Serpents",      SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,            0, IS_CAST2, { 0,                 0,          0 },        0,        0,        40,        80,         0,          0 },
	{ SPL_BLODRIT,       0, STYPE_MAGIC,     "Blood Ritual",       SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,            0, IS_CAST2, { 0,                 0,          0 },        0,        0,        40,        80,         0,          0 },
#ifdef HELLFIRE
	{ SPL_NOVA,         60, STYPE_MAGIC,     "Nova",                     14,        20,         14, FALSE,     FALSE,           87, IS_CAST4, { MIS_LIGHTNOVAC,    0,          0 },        3,       35,        16,        32,     21000,       1300 },
#else
	{ SPL_NOVA,         60, STYPE_MAGIC,     "Nova",               SPELL_NA,        20,         14, FALSE,     FALSE,           87, IS_CAST4, { MIS_LIGHTNOVAC,    0,          0 },        3,       35,        16,        32,     21000,       1300 },
#endif
	{ SPL_INVISIBIL,     0, STYPE_MAGIC,     "Invisibility",       SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,            0, IS_CAST2, { 0,                 0,          0 },        0,        0,        40,        80,         0,          0 },
	{ SPL_FLAME,        11, STYPE_FIRE,      "Inferno",                   3,         4,   SCRL_MIN, TRUE,      FALSE,           20, IS_CAST2, { MIS_FLAMEC,        0,          0 },        1,        6,        20,        40,      2000,        100 },
#ifdef SPAWN
	{ SPL_GOLEM,       100, STYPE_FIRE,      "Golem",              SPELL_NA,  SPELL_NA,         10, FALSE,     FALSE,           81, IS_CAST2, { MIS_GOLEM,         0,          0 },        6,       60,        16,        32,     18000,       1100 },
#else
	{ SPL_GOLEM,       100, STYPE_FIRE,      "Golem",                    11,        18,         10, FALSE,     FALSE,           81, IS_CAST2, { MIS_GOLEM,         0,          0 },        6,       60,        16,        32,     18000,       1100 },
#endif
#ifdef HELLFIRE
	{ SPL_BLODBOIL,     15, STYPE_MAGIC,     "Rage",               SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,            0, IS_CAST8, { MIS_BLODBOIL,      0,          0 },        1,        1,         0,         0,         0,          0 },
#else
	{ SPL_BLODBOIL,      0, STYPE_LIGHTNING, "Blood Boil",         SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      FALSE,            0, IS_CAST8, { 0,                 0,          0 },        0,        0,         0,         0,         0,          0 },
#endif
	{ SPL_TELEPORT,     35, STYPE_MAGIC,     "Teleport",                 14,        24,         14, TRUE,      TRUE,           105, IS_CAST6, { MIS_TELEPORT,      0,          0 },        3,       15,        16,        32,     20000,       1250 },
#ifdef SPAWN
	{ SPL_APOCA,       150, STYPE_FIRE,      "Apocalypse",         SPELL_NA,  SPELL_NA,         22, FALSE,     FALSE,          149, IS_CAST2, { MIS_APOCAC,        0,          0 },        6,       90,         8,        12,     30000,       2000 },
#elif defined(HELLFIRE)
	{ SPL_APOCA,       150, STYPE_FIRE,      "Apocalypse",               19,        30,         22, FALSE,     FALSE,          149, IS_CAST2, { MIS_APOCAC,        0,          0 },        6,       90,         8,        12,     30000,       2000 },
#else
	{ SPL_APOCA,       150, STYPE_FIRE,      "Apocalypse",         SPELL_NA,        30,         22, FALSE,     FALSE,          149, IS_CAST2, { MIS_APOCAC,        0,          0 },        6,       90,         8,        12,     30000,       2000 },
#endif
	{ SPL_ETHEREALIZE, 100, STYPE_MAGIC,     "Etherealize",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,           93, IS_CAST2, { MIS_ETHEREALIZE,   0,          0 },        0,      100,         2,         6,     26000,       1600 },
	{ SPL_REPAIR,        0, STYPE_MAGIC,     "Item Repair",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     TRUE,             0, IS_CAST6, { MIS_REPAIR,        0,          0 },        0,        0,        40,        80,         0,          0 },
	{ SPL_RECHARGE,      0, STYPE_MAGIC,     "Staff Recharge",     SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     TRUE,             0, IS_CAST6, { MIS_RECHARGE,      0,          0 },        0,        0,        40,        80,         0,          0 },
	{ SPL_DISARM,        0, STYPE_MAGIC,     "Trap Disarm",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,            0, IS_CAST6, { MIS_DISARM,        0,          0 },        0,        0,        40,        80,         0,          0 },
#ifdef HELLFIRE
	{ SPL_ELEMENT,      35, STYPE_FIRE,      "Elemental",          SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,           68, IS_CAST2, { MIS_ELEMENT,       0,          0 },        2,       20,        20,        60,     10500,        700 },
#else
	{ SPL_ELEMENT,      35, STYPE_FIRE,      "Elemental",                 8,        12,   SPELL_NA, FALSE,     FALSE,           68, IS_CAST2, { MIS_ELEMENT,       0,          0 },        2,       20,        20,        60,     10500,        700 },
#endif
	{ SPL_CBOLT,         6, STYPE_LIGHTNING, "Charged Bolt",       BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      FALSE,           25, IS_CAST2, { MIS_CBOLTC,         0,          0 },        1,        6,        40,        80,      1000,         50 },
	{ SPL_HBOLT,         7, STYPE_MAGIC,     "Holy Bolt",          BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      FALSE,           20, IS_CAST2, { MIS_HBOLT,         0,          0 },        1,        3,        40,        80,      1000,         50 },
	{ SPL_RESURRECT,    20, STYPE_MAGIC,     "Resurrect",          SPELL_NA,        10,   SCRL_MIN, FALSE,     TRUE,            20, IS_CAST8, { MIS_RESURRECT,     0,          0 },        0,       20,         4,        10,      4000,        250 },
	{ SPL_TELEKINESIS,  15, STYPE_MAGIC,     "Telekinesis",               2,         4,   SPELL_NA, FALSE,     FALSE,           33, IS_CAST2, { MIS_TELEKINESIS,   0,          0 },        2,        8,        20,        40,      2500,        200 },
	{ SPL_HEALOTHER,     5, STYPE_MAGIC,     "Heal Other",         BOOK_MIN, STAFF_MIN,   SPELL_NA, FALSE,     TRUE,            17, IS_CAST8, { MIS_HEALOTHER,     0,          0 },        3,        1,        20,        40,      1000,         50 },
#ifdef SPAWN
	{ SPL_FLARE,        25, STYPE_MAGIC,     "Blood Star",         SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,           70, IS_CAST2, { MIS_FLARE,         0,          0 },        2,       14,        20,        60,     27500,       1800 },
	{ SPL_BONESPIRIT,   24, STYPE_MAGIC,     "Bone Spirit",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     FALSE,           34, IS_CAST2, { MIS_BONESPIRIT,    0,          0 },        1,       12,        20,        60,     11500,        800 },
#else
	{ SPL_FLARE,        25, STYPE_MAGIC,     "Blood Star",               14,        26,   SPELL_NA, FALSE,     FALSE,           70, IS_CAST2, { MIS_FLARE,         0,          0 },        2,       14,        20,        60,     27500,       1800 },
	{ SPL_BONESPIRIT,   24, STYPE_MAGIC,     "Bone Spirit",               9,        14,   SPELL_NA, FALSE,     FALSE,           34, IS_CAST2, { MIS_BONESPIRIT,    0,          0 },        1,       12,        20,        60,     11500,        800 },
#endif
#ifdef HELLFIRE
	{ SPL_MANA,        255, STYPE_MAGIC,     "Mana",               SPELL_NA,        10,   SPELL_NA, FALSE,     TRUE,            17, IS_CAST8, { MIS_MANA,          0,          0 },        3,        1,        12,        24,      1000,         50 },
	{ SPL_MAGI,        255, STYPE_MAGIC,     "the Magi",           SPELL_NA,        40,   SPELL_NA, FALSE,     TRUE,            45, IS_CAST8, { MIS_MAGI,          0,          0 },        3,        1,        15,        30,    100000,        200 },
	{ SPL_JESTER,      255, STYPE_MAGIC,     "the Jester",         SPELL_NA,         8,   SPELL_NA, TRUE,      FALSE,           30, IS_CAST8, { MIS_JESTER,        0,          0 },        3,        1,        15,        30,    100000,        200 },
	{ SPL_LIGHTWALL,    28, STYPE_LIGHTNING, "Lightning Wall",            3,         4,   SPELL_NA, TRUE,      FALSE,           27, IS_CAST4, { MIS_LIGHTWALLC,    0,          0 },        2,       16,         8,        16,      6000,        400 },
	{ SPL_IMMOLAT,      60, STYPE_FIRE,      "Immolation",               14,        20,   SPELL_NA, FALSE,     FALSE,           87, IS_CAST2, { MIS_FIRENOVAC,     0,          0 },        3,       35,        16,        32,     21000,       1300 },
	{ SPL_WARP,         35, STYPE_MAGIC,     "Warp",                      3,         6,   SPELL_NA, FALSE,     FALSE,           25, IS_CAST6, { MIS_WARP,          0,          0 },        3,       18,         8,        12,      3000,        200 },
	{ SPL_REFLECT,      35, STYPE_MAGIC,     "Reflect",                   3,         6,   SPELL_NA, FALSE,     FALSE,           25, IS_CAST6, { MIS_REFLECT,       0,          0 },        3,       15,         8,        12,      3000,        200 },
	{ SPL_BERSERK,      35, STYPE_MAGIC,     "Berserk",                   3,         6,   SPELL_NA, TRUE,      FALSE,           35, IS_CAST6, { MIS_BERSERK,       0,          0 },        3,       15,         8,        12,      3000,        200 },
	{ SPL_FIRERING,     28, STYPE_FIRE,      "Ring of Fire",              5,        10,   SPELL_NA, FALSE,     FALSE,           27, IS_CAST2, { MIS_FIRERING,      0,          0 },        2,       16,         8,        16,      6000,        400 },
	{ SPL_SEARCH,       15, STYPE_MAGIC,     "Search",             BOOK_MIN,         6,   SPELL_NA, FALSE,     FALSE,           25, IS_CAST6, { MIS_SEARCH,        0,          0 },        1,        1,         8,        12,      3000,        200 },
	{ SPL_RUNEFIRE,    255, STYPE_MAGIC,     "Rune of Fire",       SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      FALSE,           48, IS_CAST8, { MIS_RUNEFIRE,      0,          0 },        1,       10,        40,        80,      8000,        300 },
	{ SPL_RUNELIGHT,   255, STYPE_MAGIC,     "Rune of Light",      SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      FALSE,           48, IS_CAST8, { MIS_RUNELIGHT,     0,          0 },        1,       10,        40,        80,      8000,        300 },
	{ SPL_RUNENOVA,    255, STYPE_MAGIC,     "Rune of Nova",       SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      FALSE,           48, IS_CAST8, { MIS_RUNENOVA,      0,          0 },        1,       10,        40,        80,      8000,        300 },
	{ SPL_RUNEIMMOLAT, 255, STYPE_MAGIC,     "Rune of Immolation", SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      FALSE,           48, IS_CAST8, { MIS_RUNEIMMOLAT,   0,          0 },        1,       10,        40,        80,      8000,        300 },
	{ SPL_RUNESTONE,   255, STYPE_MAGIC,     "Rune of Stone",      SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      FALSE,           48, IS_CAST8, { MIS_RUNESTONE,     0,          0 },        1,       10,        40,        80,      8000,        300 },
#endif
	// clang-format on
};

DEVILUTION_END_NAMESPACE
