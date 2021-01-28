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
	// sName,    sManaCost, sType,           sNameText,            sBookLvl, sStaffLvl, sScrollLvl, sTargeted, scCurs,             spCurs,             sTownSpell, sMinInt, sSFX,     sMissile,   sManaAdj, sMinMana, sStaffMin, sStaffMax, sBookCost, sStaffCost
	{ SPL_NULL,          0, 0,               NULL,                 SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_TELEKINESIS, FALSE,            0, 0,        0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_FIREBOLT,      6, STYPE_FIRE,      "Firebolt",           BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           15, IS_CAST2, MIS_FIREBOLT,      1,        3,        40,        80,      1000,         50 },
	{ SPL_HEAL,          5, STYPE_MAGIC,     "Healing",            BOOK_MIN, STAFF_MIN,   SCRL_MIN, FALSE,     CURSOR_NONE,        CURSOR_NONE,        TRUE,            17, IS_CAST8, MIS_HEAL,          3,        1,        20,        40,      1000,         50 },
	{ SPL_LIGHTNING,    10, STYPE_LIGHTNING, "Lightning",                 4,         6,          4, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           20, IS_CAST4, MIS_LIGHTNINGC,    1,        6,        20,        60,      3000,        150 },
	{ SPL_FLASH,        30, STYPE_LIGHTNING, "Flash",                     5,         8,          6, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           33, IS_CAST4, MIS_FLASH,         2,       16,        20,        40,      7500,        500 },
	{ SPL_IDENTIFY,     13, STYPE_MAGIC,     "Identify",           SPELL_NA,  SPELL_NA,   SCRL_MIN, FALSE,     CURSOR_IDENTIFY,    CURSOR_IDENTIFY,    TRUE,            20, IS_CAST6, MIS_IDENTIFY,      0,        0,         0,         0,         0,        100 },
	{ SPL_FIREWALL,     28, STYPE_FIRE,      "Fire Wall",                 3,         4,          4, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           27, IS_CAST2, MIS_FIREWALLC,     2,       16,         8,        16,      6000,        400 },
	{ SPL_TOWN,         35, STYPE_MAGIC,     "Town Portal",               3,         6,          4, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           20, IS_CAST6, MIS_TOWN,          3,       18,         8,        12,      3000,        200 },
	{ SPL_STONE,        60, STYPE_MAGIC,     "Stone Curse",               6,        10,          6, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           51, IS_CAST2, MIS_STONE,         3,       40,         8,        16,     12000,        800 },
	{ SPL_INFRA,        40, STYPE_MAGIC,     "Infravision",        SPELL_NA,  SPELL_NA,          8, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           43, IS_CAST8, MIS_INFRA,         0,        0,         0,         0,         0,        600 },
	{ SPL_RNDTELEPORT,  12, STYPE_MAGIC,     "Phasing",                   7,        12,          6, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           39, IS_CAST2, MIS_RNDTELEPORT,   2,        4,        40,        80,      3500,        200 },
	{ SPL_MANASHIELD,   33, STYPE_MAGIC,     "Mana Shield",               6,        10,          8, FALSE,     CURSOR_NONE,        CURSOR_NONE,        TRUE,            25, IS_CAST2, MIS_MANASHIELD,    0,       33,         4,        10,     16000,       1200 },
	{ SPL_FIREBALL,     16, STYPE_FIRE,      "Fireball",                  8,        14,          8, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           48, IS_CAST2, MIS_FIREBALL,      1,       10,        40,        80,      8000,        300 },
	{ SPL_GUARDIAN,     50, STYPE_FIRE,      "Guardian",                  9,        16,         12, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           61, IS_CAST2, MIS_GUARDIAN,      2,       30,        16,        32,     14000,        950 },
	{ SPL_CHAIN,        30, STYPE_LIGHTNING, "Chain Lightning",           8,        14,         10, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           54, IS_CAST2, MIS_CHAIN,         1,       18,        20,        60,     11000,        750 },
	{ SPL_WAVE,         35, STYPE_FIRE,      "Flame Wave",                9,        16,         10, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           54, IS_CAST2, MIS_FIREWAVEC,     3,       20,        20,        40,     10000,        650 },
#ifdef HELLFIRE
	{ SPL_NOVA,         60, STYPE_MAGIC,     "Nova",                     14,        20,         14, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           87, IS_CAST4, MIS_LIGHTNOVAC,    3,       35,        16,        32,     21000,       1300 },
#else
	{ SPL_NOVA,         60, STYPE_MAGIC,     "Nova",               SPELL_NA,        20,         14, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           87, IS_CAST4, MIS_LIGHTNOVAC,    3,       35,        16,        32,     21000,       1300 },
#endif
	{ SPL_INVISIBIL,     0, STYPE_MAGIC,     "Invisibility",       SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,            0, IS_CAST2, 0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_FLAME,        11, STYPE_FIRE,      "Inferno",                   3,         4,   SCRL_MIN, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           20, IS_CAST2, MIS_FLAMEC,        1,        6,        20,        40,      2000,        100 },
	{ SPL_GOLEM,       100, STYPE_FIRE,      "Golem",                    11,        18,         10, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           81, IS_CAST2, MIS_GOLEM,         6,       60,        16,        32,     18000,       1100 },
#ifdef HELLFIRE
	{ SPL_BLODBOIL,     15, STYPE_MAGIC,     "Rage",               SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,            0, IS_CAST8, MIS_BLODBOIL,      0,        0,         0,         0,         0,          0 },
#else
	{ SPL_BLODBOIL,      0, STYPE_LIGHTNING, "Blood Boil",         SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,            0, IS_CAST8, 0,                 0,        0,         0,         0,         0,          0 },
#endif
	{ SPL_TELEPORT,     35, STYPE_MAGIC,     "Teleport",                 14,        24,         14, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        TRUE,           105, IS_CAST6, MIS_TELEPORT,      3,       15,        16,        32,     20000,       1250 },
	{ SPL_ETHEREALIZE, 100, STYPE_MAGIC,     "Etherealize",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           93, IS_CAST2, MIS_ETHEREALIZE,   0,        0,         0,         0,     26000,       1600 },
	{ SPL_REPAIR,        0, STYPE_MAGIC,     "Item Repair",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_REPAIR,      CURSOR_REPAIR,      TRUE,             0, IS_CAST6, MIS_REPAIR,        0,        0,         0,         0,         0,          0 },
	{ SPL_RECHARGE,      0, STYPE_MAGIC,     "Staff Recharge",     SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_RECHARGE,    CURSOR_RECHARGE,    TRUE,             0, IS_CAST6, MIS_RECHARGE,      0,        0,         0,         0,         0,          0 },
	{ SPL_DISARM,        0, STYPE_MAGIC,     "Trap Disarm",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_DISARM,      CURSOR_DISARM,      FALSE,            0, IS_CAST6, MIS_DISARM,        0,        0,         0,         0,         0,          0 },
#ifdef HELLFIRE
	{ SPL_ELEMENT,      35, STYPE_FIRE,      "Elemental",          SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_NONE,        CURSOR_NONE,        FALSE,           68, IS_CAST2, MIS_ELEMENT,       2,       20,        20,        60,     10500,        700 },
#else
	{ SPL_ELEMENT,      35, STYPE_FIRE,      "Elemental",                 8,        12,   SPELL_NA, TRUE,      CURSOR_NONE,        CURSOR_NONE,        FALSE,           68, IS_CAST2, MIS_ELEMENT,       2,       20,        20,        60,     10500,        700 },
#endif
	{ SPL_CBOLT,         6, STYPE_LIGHTNING, "Charged Bolt",       BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           25, IS_CAST2, MIS_CBOLTC,        1,        6,        40,        80,      1000,         50 },
	{ SPL_HBOLT,         7, STYPE_MAGIC,     "Holy Bolt",          BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           20, IS_CAST2, MIS_HBOLT,         1,        3,        40,        80,      1000,         50 },
	{ SPL_RESURRECT,    20, STYPE_MAGIC,     "Resurrect",          SPELL_NA,        10,   SCRL_MIN, TRUE,      CURSOR_RESURRECT,   CURSOR_RESURRECT,   TRUE,            20, IS_CAST8, MIS_RESURRECT,     0,        0,         4,        10,      4000,        250 },
	{ SPL_TELEKINESIS,  15, STYPE_MAGIC,     "Telekinesis",               2,         4,   SPELL_NA, TRUE,      CURSOR_TELEKINESIS, CURSOR_NONE,        FALSE,           33, IS_CAST2, MIS_TELEKINESIS,   2,        8,        20,        40,      2500,        200 },
	{ SPL_HEALOTHER,     5, STYPE_MAGIC,     "Heal Other",         BOOK_MIN, STAFF_MIN,   SPELL_NA, TRUE,      CURSOR_HEALOTHER,   CURSOR_HEALOTHER,   TRUE,            17, IS_CAST8, MIS_HEALOTHER,     3,        1,        20,        40,      1000,         50 },
	{ SPL_FLARE,        25, STYPE_MAGIC,     "Blood Star",               14,        26,   SPELL_NA, TRUE,      CURSOR_NONE,        CURSOR_NONE,        FALSE,           70, IS_CAST2, MIS_FLARE,         2,       14,        20,        60,     27500,       1800 },
#ifdef HELLFIRE
	{ SPL_LIGHTWALL,    28, STYPE_LIGHTNING, "Lightning Wall",            3,         4,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_NONE,        FALSE,           27, IS_CAST4, MIS_LIGHTWALLC,    2,       16,         8,        16,      6000,        400 },
	{ SPL_IMMOLAT,      60, STYPE_FIRE,      "Immolation",               14,        20,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           87, IS_CAST2, MIS_FIRENOVAC,     3,       35,        16,        32,     21000,       1300 },
	{ SPL_FIRERING,     28, STYPE_FIRE,      "Ring of Fire",              5,        10,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,           27, IS_CAST2, MIS_FIRERING,      2,       16,         8,        16,      6000,        400 },
	{ SPL_RUNEFIRE,    255, STYPE_MAGIC,     "Rune of Fire",       SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    FALSE,           48, IS_CAST8, MIS_RUNEFIRE,      0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNELIGHT,   255, STYPE_MAGIC,     "Rune of Light",      SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    FALSE,           48, IS_CAST8, MIS_RUNELIGHT,     0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNENOVA,    255, STYPE_MAGIC,     "Rune of Nova",       SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    FALSE,           48, IS_CAST8, MIS_RUNENOVA,      0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNEIMMOLAT, 255, STYPE_MAGIC,     "Rune of Immolation", SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    FALSE,           48, IS_CAST8, MIS_RUNEIMMOLAT,   0,        0,         0,         0,      8000,        300 },
	{ SPL_RUNESTONE,   255, STYPE_MAGIC,     "Rune of Stone",      SPELL_NA,  SPELL_NA,   SPELL_NA, TRUE,      CURSOR_TELEPORT,    CURSOR_TELEPORT,    FALSE,           48, IS_CAST8, MIS_RUNESTONE,     0,        0,         0,         0,      8000,        300 },
	{ SPL_WHITTLE,       0, STYPE_MAGIC,     "Staff Whittle",      SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_RECHARGE,    CURSOR_RECHARGE,    TRUE,             0, IS_CAST6, MIS_WHITTLE,       0,        0,         0,         0,         0,          0 },
#endif
	{ SPL_WALK,          0, STYPE_MAGIC,     "Walk",               SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        TRUE,             0, IS_CAST6, 0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_WATTACK,       0, STYPE_MAGIC,     "Walk/Attack",        SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        TRUE,             0, IS_CAST6, 0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_ATTACK,        0, STYPE_MAGIC,     "Attack",             SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,            0, IS_CAST6, 0,                 0,        0,         0,         0,         0,          0 },
	{ SPL_BLOCK,         0, STYPE_MAGIC,     "Block",              SPELL_NA,  SPELL_NA,   SPELL_NA, FALSE,     CURSOR_NONE,        CURSOR_NONE,        FALSE,            0, IS_CAST6, 0,                 0,        0,         0,         0,         0,          0 },
	// clang-format on
};

DEVILUTION_END_NAMESPACE
