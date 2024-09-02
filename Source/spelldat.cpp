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
#define ICN_SWP 32
#define ICN_RTK 48
#define ICN_RPB 19
#define ICN_RAS 25
#define ICN_CHR 34
#else
#define ICN_BLK 19
#define ICN_ATK 22
#define ICN_SWP 32
#define ICN_RTK 22
#define ICN_RPB 17
#define ICN_RAS 25
#define ICN_CHR 34
#endif

/*
	reused icons :  4
	unused icons :	// hellfire only
					51 - SPL_MANA/MAGI
					50 - SPL_JESTER
					43 - SPL_WARP
					44 - SPL_SEARCH
*/

/*
 * Data related to each spell ID.
 * sIcon: spelicon.cel frame number.
 */
const SpellData spelldata[NUM_SPELLS] = {
	// clang-format off
//                    sManaCost, sType,             sIcon, sNameText,         sBookLvl, sStaffLvl, sScrollLvl, sSkillFlags,     scCurs,           spCurs,             sUseFlags,                    sMinInt, sSFX,        sMissile,        sManaAdj, sMinMana, sStaffMin, sStaffMax, sBookCost, sStaffCost, 
/*SPL_NULL*/        {         0, 0,                    27, NULL,              SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_MELEE | SFLAG_RANGED,         0, 0,           0,                      0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_WALK*/        {         0, STYPE_NONE,           28, "Walk",            SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_ANY,                          0, 0,           0,                      0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_BLOCK*/       {         0, STYPE_NONE,      ICN_BLK, "Block",           SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_BLOCK,        0, 0,           0,                      0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_ATTACK*/      {         0, STYPE_NONE,      ICN_ATK, "Attack",          SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_MELEE,        0, 0,           0,                      0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_WHIPLASH*/    {         2, STYPE_NONE,           20, "Whiplash",               1,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_MELEE,        0, 0,           0,                      0,        2,         0,         0,      1000,          0, ALIGN64 },
/*SPL_WALLOP*/      {         6, STYPE_NONE,           33, "Wallop",                 2,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_MELEE,        0, 0,           0,                      0,        6,         0,         0,      1500,          0, ALIGN64 },
/*SPL_SWIPE*/       {         2, STYPE_NONE,      ICN_SWP, "Swipe",                  6,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_MELEE,        0, 0,           0,                      0,        2,         0,         0,      8000,          0, ALIGN64 },
/*SPL_RATTACK*/     {         0, STYPE_NONE,      ICN_RTK, "Ranged Attack",   SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,       0, 0,           MIS_ARROW,              0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_POINT_BLANK*/ {         2, STYPE_NONE,      ICN_RPB, "Point Blank",            4,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,       0, 0,           MIS_PBARROW,            0,        2,         0,         0,      5000,          0, ALIGN64 },
/*SPL_FAR_SHOT*/    {         2, STYPE_NONE,      ICN_RAS, "Far Shot",               8,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,       0, 0,           MIS_ASARROW,            0,        2,         0,         0,     10000,          0, ALIGN64 },
/*SPL_PIERCE_SHOT*/ {         2, STYPE_NONE,           20, "Pierce Shot",           14,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,       0, 0,           MIS_PCARROW,            0,        2,         0,         0,     16000,          0, ALIGN64 },
/*SPL_MULTI_SHOT*/  {         8, STYPE_NONE,            9, "Multi Shot",            16,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON | SFLAG_RANGED,       0, 0,           MIS_MLARROW,            0,        8,         0,         0,     20000,          0, ALIGN64 },
/*SPL_CHARGE*/      {         2, STYPE_MAGIC,     ICN_CHR, "Charge",                10,  SPELL_NA,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                      0, 0,           MIS_CHARGE,             0,        2,         0,         0,     12000,          0, ALIGN64 },
/*SPL_RAGE*/        {         7, STYPE_MAGIC,          23, "Rage",                   2,  SPELL_NA,          4, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_RAGE,                         0, IS_CAST6,    MIS_RAGE,               2,        2,         0,         0,      1500,        100, ALIGN64 },
/*SPL_FIREBOLT*/    {         6, STYPE_FIRE,            1, "Firebolt",               1,         2,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     15, IS_CAST2,    MIS_FIREBOLT,           1,        3,        40,        80,      1000,         20, ALIGN64 },
/*SPL_CBOLT*/       {         6, STYPE_LIGHTNING,      39, "Charged Bolt",           1,         2,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     25, IS_CAST2,    MIS_CBOLTC,             1,        4,        40,        80,      1000,         20, ALIGN64 },
/*SPL_HBOLT*/       {         7, STYPE_MAGIC,          42, "Acidbolt",               1,         2,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     20, IS_CAST2,    MIS_HBOLT,              1,        4,        40,        80,      1000,         20, ALIGN64 },
/*SPL_LIGHTNING*/   {        15, STYPE_LIGHTNING,       3, "Lightning",              4,         6,          4, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     20, IS_CAST4,    MIS_LIGHTNINGC,         1,        8,        20,        60,      3000,         30, ALIGN64 },
/*SPL_FLASH*/       {        30, STYPE_LIGHTNING,       4, "Flash",                  5,         8,          6, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     33, IS_CAST4,    MIS_FLASH,              3,       16,        20,        40,      7500,         40, ALIGN64 },
/*SPL_FIREWALL*/    {        28, STYPE_FIRE,            6, "Fire Wall",              3,         4,          4, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     27, IS_CAST2,    MIS_FIREWALLC,          3,       16,         8,        16,      6000,         80, ALIGN64 },
/*SPL_FIREBALL*/    {        20, STYPE_FIRE,           12, "Fireball",               8,        14,          8, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     48, IS_CAST2,    MIS_FIREBALL,           1,       14,        40,        80,      8000,         30, ALIGN64 },
/*SPL_METEOR*/      {        54, STYPE_MAGIC,          31, "Meteor",                24,        32,         16, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     42, IS_CAST2,    MIS_METEOR,             2,       30,        20,        60,     15000,         50, ALIGN64 },
/*SPL_BLOODBOIL*/   {        40, STYPE_MAGIC,          23, "Bloodboil",             28,        36,         20, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     50, IS_CAST2,    MIS_BLOODBOILC,         1,       30,        20,        60,     16000,         40, ALIGN64 },
/*SPL_CHAIN*/       {        30, STYPE_LIGHTNING,      16, "Chain Lightning",        8,        14,         10, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     54, IS_CAST2,    MIS_CHAIN,              2,       18,        20,        60,     11000,         45, ALIGN64 },
/*SPL_WAVE*/        {        35, STYPE_FIRE,           14, "Fire Wave",              9,        16,         10, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     54, IS_CAST2,    MIS_FIREWAVEC,          4,       20,        20,        60,     10000,         45, ALIGN64 },
/*SPL_NOVA*/        {        60, STYPE_MAGIC,          11, "Nova",                  14,        20,         14, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     87, IS_CAST4,    MIS_LIGHTNOVAC,         5,       35,        16,        32,     21000,         50, ALIGN64 },
/*SPL_INFERNO*/     {        11, STYPE_FIRE,           15, "Inferno",                3,         4,          1, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     20, IS_CAST2,    MIS_INFERNOC,           1,        6,        40,        80,      2000,         20, ALIGN64 },
/*SPL_ELEMENTAL*/   {        35, STYPE_FIRE,           38, "Elemental",              8,        12,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     68, IS_CAST2,    MIS_ELEMENTAL,          4,       20,        20,        60,     10500,         40, ALIGN64 },
/*SPL_FLARE*/       {        25, STYPE_MAGIC,          36, "Blood Star",            14,        26,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     70, IS_CAST2,    MIS_FLARE,              4,       14,        30,        80,     27500,         30, ALIGN64 },
/*SPL_POISON*/      {        22, STYPE_MAGIC,          25, "Poison",                18,        28,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     70, IS_CAST2,    MIS_POISON,             1,       18,        30,        70,     24500,         30, ALIGN64 },
/*SPL_WIND*/        {        24, STYPE_MAGIC,          22, "Wind",                  10,        18,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     44, IS_CAST2,    MIS_WIND,               2,       12,        40,        80,     12500,         35, ALIGN64 },
/*SPL_SHROUD*/      {        28, STYPE_MAGIC,          19, "Shroud",                20,        28,         18, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     36, IS_CAST2,    MIS_SHROUD,             1,       18,        20,        60,     20500,         80, ALIGN64 },
/*SPL_SWAMP*/       {        21, STYPE_MAGIC,           4, "Swamp",                 30,        40,         24, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     36, IS_CAST2,    MIS_SWAMPC,             1,       16,        20,        60,     18500,         70, ALIGN64 },
/*SPL_GUARDIAN*/    {        50, STYPE_FIRE,           18, "Guardian",               9,        16,         12, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     61, IS_CAST2,    MIS_GUARDIAN,           4,       30,        16,        32,     14000,         90, ALIGN64 },
/*SPL_GOLEM*/       {       100, STYPE_FIRE,           21, "Golem",                 11,        18,         10, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     81, IS_CAST2,    MIS_GOLEM,             10,       60,        16,        32,     18000,        200, ALIGN64 },
/*SPL_STONE*/       {        60, STYPE_MAGIC,           8, "Stone Curse",            6,        10,          6, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     51, IS_CAST2,    MIS_STONE,              4,       40,         8,        16,     12000,        160, ALIGN64 },
/*SPL_INFRA*/       {         0, STYPE_MAGIC,           9, "Infravision",     SPELL_NA,  SPELL_NA,          8, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     43, IS_CAST8,    MIS_INFRA,              0,        0,         0,         0,         0,        600, ALIGN64 },
/*SPL_MANASHIELD*/  {        33, STYPE_MAGIC,          13, "Mana Shield",            6,        10,          8, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_ANY,                         25, IS_CAST2,    MIS_MANASHIELD,         0,       33,         4,        10,     16000,        120, ALIGN64 },
/*SPL_ATTRACT*/     {         4, STYPE_LIGHTNING,      30, "Attract",                2,         4,          4, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     20, IS_CAST4,    MIS_ATTRACT,            0,        4,        40,        80,      2000,         30, ALIGN64 },
/*SPL_TELEKINESIS*/ {        15, STYPE_MAGIC,          40, "Telekinesis",            2,         4,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_TELEKINESIS, SFLAG_DUNGEON,                     33, IS_CAST2,    MIS_TELEKINESIS,        2,        8,        20,        40,      2500,        200, ALIGN64 },
/*SPL_TELEPORT*/    {        80, STYPE_MAGIC,          24, "Teleport",              14,        24,         14, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_ANY,                        105, IS_CAST6,    MIS_TELEPORT,           2,       40,        16,        32,     20000,        250, ALIGN64 },
/*SPL_RNDTELEPORT*/ {        11, STYPE_MAGIC,          28, "Phasing",                7,        12,          6, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     39, IS_CAST2,    MIS_RNDTELEPORT,        1,        4,        40,        80,      3500,        100, ALIGN64 },
/*SPL_TOWN*/        {        35, STYPE_MAGIC,           7, "Town Portal",            3,         6,          4, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_NONE,        SFLAG_DUNGEON,                     20, IS_CAST6,    MIS_TOWN,               3,       18,        20,        80,      3000,        200, ALIGN64 },
/*SPL_HEAL*/        {         5, STYPE_MAGIC,           2, "Healing",                1,         2,          1, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_ANY,                         17, IS_CAST8,    MIS_HEAL,               6,        1,        20,        40,      1000,         50, ALIGN64 },
/*SPL_HEALOTHER*/   {         5, STYPE_MAGIC,          10, "Heal Other",             1,         2,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_HEALOTHER,   SFLAG_ANY,                         17, IS_CAST8,    MIS_HEALOTHER,          6,        1,        20,        40,      1000,         50, ALIGN64 },
/*SPL_RESURRECT*/   {         0, STYPE_MAGIC,          41, "Resurrect",       SPELL_NA,        10,          1, SDFLAG_TARGETED, CURSOR_RESURRECT, CURSOR_RESURRECT,   SFLAG_ANY,                         20, IS_CAST8,    MIS_RESURRECT,          0,        0,        16,        32,      4000,        250, ALIGN64 },
/*SPL_IDENTIFY*/    {         0, STYPE_MAGIC,           5, "Identify",        SPELL_NA,  SPELL_NA,          1, 0,               CURSOR_IDENTIFY,  CURSOR_IDENTIFY,    SFLAG_ANY,                         20, IS_CAST6,    MIS_OPITEM,             0,        0,         0,         0,         0,        100, ALIGN64 },
/*SPL_OIL*/         {         0, STYPE_MAGIC,           5, NULL,              SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_OIL,         SFLAG_ANY,                          0, IS_CAST6,    MIS_OPITEM,             0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_REPAIR*/      {         0, STYPE_MAGIC,          26, "Item Repair",     SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_REPAIR,      SFLAG_ANY,                          0, SFX_SILENCE, MIS_REPAIR,             0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_RECHARGE*/    {         0, STYPE_MAGIC,          29, "Staff Recharge",  SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_RECHARGE,    SFLAG_ANY,                          0, IS_CAST6,    MIS_OPITEM,             0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_DISARM*/      {         0, STYPE_MAGIC,          37, "Trap Disarm",     SPELL_NA,  SPELL_NA,          1, SDFLAG_TARGETED, CURSOR_DISARM,    CURSOR_DISARM,      SFLAG_DUNGEON,                     20, IS_CAST6,    MIS_DISARM,             0,        0,         0,         0,         0,        150, ALIGN64 },
#ifdef HELLFIRE
/*SPL_BUCKLE*/      {         0, STYPE_MAGIC,          26, "Shield Buckle",   SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_IDENTIFY,    SFLAG_ANY,                          0, IS_CAST6,    MIS_OPITEM,             0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_WHITTLE*/     {         0, STYPE_MAGIC,          29, "Staff Whittle",   SPELL_NA,  SPELL_NA,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_RECHARGE,    SFLAG_ANY,                          0, IS_CAST6,    MIS_OPITEM,             0,        0,         0,         0,         0,          0, ALIGN64 },
/*SPL_LIGHTWALL*/// {        28, STYPE_LIGHTNING,      46, "Lightning Wall",         3,         4,   SPELL_NA, SDFLAG_TARGETED, CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     27, IS_CAST4,    MIS_LIGHTWALLC,         3,       16,         8,        16,      6000,        400, ALIGN64 },
/*SPL_IMMOLAT *///  {        60, STYPE_FIRE,           47, "Immolation",            14,        20,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     87, IS_CAST2,    MIS_FIRENOVAC,          5,       35,        16,        32,     21000,       1300, ALIGN64 },
/*SPL_FIRERING*/    {        28, STYPE_FIRE,           49, "Ring of Fire",           5,        10,   SPELL_NA, 0,               CURSOR_NONE,      CURSOR_NONE,        SFLAG_DUNGEON,                     27, IS_CAST2,    MIS_FIRERING,           3,       16,         8,        20,      6000,         80, ALIGN64 },
/*SPL_RUNEFIRE*/    {         0, STYPE_FIRE,           15, "Fire",            SPELL_NA,  SPELL_NA,          1, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_TELEPORT,    SFLAG_DUNGEON,                      0, IS_CAST8,    MIS_RUNEFIRE,           0,        0,         0,         0,         0,        100, ALIGN64 },
/*SPL_RUNELIGHT*/   {         0, STYPE_LIGHTNING,       3, "Lightning",       SPELL_NA,  SPELL_NA,          3, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_TELEPORT,    SFLAG_DUNGEON,                     13, IS_CAST8,    MIS_RUNELIGHT,          0,        0,         0,         0,         0,        200, ALIGN64 },
/*SPL_RUNENOVA*/    {         0, STYPE_LIGHTNING,      11, "Nova",            SPELL_NA,  SPELL_NA,          7, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_TELEPORT,    SFLAG_DUNGEON,                     42, IS_CAST8,    MIS_RUNENOVA,           0,        0,         0,         0,         0,        400, ALIGN64 },
/*SPL_RUNEWAVE*/    {         0, STYPE_FIRE,           14, "Fire Wave",       SPELL_NA,  SPELL_NA,          7, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_TELEPORT,    SFLAG_DUNGEON,                     42, IS_CAST8,    MIS_RUNEWAVE,           0,        0,         0,         0,         0,        500, ALIGN64 },
/*SPL_RUNESTONE*/   {         0, STYPE_MAGIC,           8, "Stone Curse",     SPELL_NA,  SPELL_NA,          7, SDFLAG_TARGETED, CURSOR_TELEPORT,  CURSOR_TELEPORT,    SFLAG_DUNGEON,                     25, IS_CAST8,    MIS_RUNESTONE,          0,        0,         0,         0,         0,        300, ALIGN64 },
#endif
	// clang-format on
};

DEVILUTION_END_NAMESPACE
