/**
 * @file questdat.cpp
 *
 * Implementation of all quest and level data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

const LevelDataStruct AllLevels[NUMLEVELS + NUM_SETLVL] = {
	// clang-format off
					// dLevel, dSetLvl, dType,           dDunType,        dMusic,      dMicroTileLen, dBlocks, dLevelName,
/*DLV_TOWN*/		{       0, FALSE,   DTYPE_TOWN,      DTYPE_TOWN,      TMUSIC_TOWN,            16,      16, "Tristram",
					//  dAutomapData,             dSolidTable,                  dDunCels,                      dMegaTiles,                    dLvlPieces,                    dSpecCels,                     dPalName,                        dLoadCels,              dLoadPal,                dSetLvlPreDun,                 dSetLvlDun,                        dSetLvlDunX,   dSetLvlDunY,
#ifdef HELLFIRE
						NULL,                     "Levels\\TownData\\Town.SOL", "NLevels\\TownData\\Town.CEL", "NLevels\\TownData\\Town.TIL", "NLevels\\TownData\\Town.MIN", "Levels\\TownData\\TownS.CEL", "Levels\\TownData\\Town.pal",    "Gendata\\Cuttt.CEL",   "Gendata\\Cuttt.pal",    NULL,                          NULL,                                        0,             0,
#else
						NULL,                     "Levels\\TownData\\Town.SOL", "Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "Levels\\TownData\\Town.pal",    "Gendata\\Cuttt.CEL",   "Gendata\\Cuttt.pal",    NULL,                          NULL,                                        0,             0,
#endif
					},
/*DLV_CATHEDRAL1*/	{       1, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 1",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
					},
/*DLV_CATHEDRAL2*/	{       2, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 2",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
					},
/*DLV_CATHEDRAL3*/	{       3, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 3",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
					},
/*DLV_CATHEDRAL4*/	{       4, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 4",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
					},
/*DLV_CATACOMBS1*/	{       5, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 1",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_CATACOMBS2*/	{       6, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 2",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_CATACOMBS3*/	{       7, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 3",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_CATACOMBS4*/	{       8, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 4",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_CAVES1*/		{       9, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 1",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_CAVES2*/		{      10, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 2",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_CAVES3*/		{      11, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 3",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_CAVES4*/		{      12, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 4",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_HELL1*/		{      13, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Hell 1",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_HELL2*/		{      14, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Hell 2",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_HELL3*/		{      15, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Hell 3",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     NULL,                          NULL,                                        0,             0,
					},
/*DLV_HELL4*/		{      16, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Diablo",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cutgate.CEL", "Gendata\\Cutgate.pal",  NULL,                          NULL,                                        0,             0,
					},
#ifdef HELLFIRE
/*DLV_NEST1*/		{       9, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 1",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
					},
/*DLV_NEST2*/		{      10, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 2",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
					},
/*DLV_NEST3*/		{      11, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 3",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
					},
/*DLV_NEST4*/		{      12, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 4",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
					},
/*DLV_CRYPT1*/		{      14, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 1",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
					},
/*DLV_CRYPT2*/		{      15, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 2",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
					},
/*DLV_CRYPT3*/		{      16, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 3",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
					},
/*DLV_CRYPT4*/		{      17, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 4",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
					},
#endif
					// dLevel, dSetLvl, dType,           dDunType,        dMusic,      dMicroTileLen, dBlocks, dLevelName,
/*DLV_BUTCHCHAMB*/	{       0, TRUE,    DTYPE_NONE,      DTYPE_NONE,      TMUSIC_TOWN,            10,      10, "",
					//  dAutomapData,             dSolidTable,                  dDunCels,                      dMegaTiles,                    dLvlPieces,                    dSpecCels,                     dPalName,                        dLoadCels,               dLoadPal,                dSetLvlPreDun,                 dSetLvlDun,                        dSetLvlDunX,   dSetLvlDunY,
						NULL,                     NULL,                         "Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "",                              "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
					},
/*DLV_SKELKING*/	{       3, TRUE,    DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Skeleton King's Lair",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_2.pal",      "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\SklKng1.DUN", "Levels\\L1Data\\SklKng2.DUN",   DBORDERX + 67, DBORDERY + 29,
					},
/*DLV_BONECHAMB*/	{       6, TRUE,    DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Chamber of Bone",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_2.pal",      "Gendata\\Cut2.CEL",     "Gendata\\Cut2.pal",     "Levels\\L2Data\\Bonecha2.DUN", "Levels\\L2Data\\Bonecha1.DUN", DBORDERX + 53, DBORDERY + 23,
					},
/*DLV_MAZE*/		{       0, TRUE,    DTYPE_NONE,      DTYPE_NONE,      TMUSIC_TOWN,            10,      10, "Maze",
						NULL,                     NULL,                         "Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "Levels\\L1Data\\L1_5.pal",      "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\Lv1MazeA.DUN", "Levels\\L1Data\\Lv1MazeB.DUN", DBORDERX +  4, DBORDERY + 34,
					},
/*DLV_POISONWATER*/	{       2, TRUE,    DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Poisoned Water Supply",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3pfoul.pal",   "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   "Levels\\L3Data\\Foulwatr.DUN", "Levels\\L3Data\\Foulwatr.DUN", DBORDERX + 15, DBORDERY + 67,
					},
/*DLV_VILEBETRAYER*/{      15, TRUE,    DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Archbishop Lazarus' Lair",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_2.pal",      "Gendata\\Cutportr.CEL", "Gendata\\Cutportr.pal", "Levels\\L1Data\\Vile1.DUN",    "Levels\\L1Data\\Vile2.DUN",    DBORDERX + 19, DBORDERY + 20,
					},
	// clang-format on
};

/** Contains the data related to each quest_id. */
const QuestData questlist[NUM_QUESTS] = {
	// clang-format off
	          // _qdlvl,            _qdmultlvl, _qslvl,          _qflags,       _qdmsg,        _qlstr
/*Q_ROCK*/    { DLV_CATACOMBS1,             -1, 0,               QUEST_SINGLE,  TEXT_INFRA5,   "The Magic Rock"           },
/*Q_MUSHROOM*/{ DLV_CAVES1,                 -1, 0,               QUEST_SINGLE,  TEXT_MUSH8,    "Black Mushroom"           },
/*Q_GARBUD*/  { DLV_CATHEDRAL4,             -1, 0,               QUEST_SINGLE,  TEXT_GARBUD1,  "Gharbad The Weak"         },
/*Q_ZHAR*/    { DLV_CATACOMBS4,             -1, 0,               QUEST_SINGLE,  TEXT_ZHAR1,    "Zhar the Mad"             },
/*Q_VEIL*/    { DLV_HELL2,                  -1, 0,               QUEST_SINGLE,  TEXT_VEIL9,    "Lachdanan"                },
/*Q_DIABLO*/  { DLV_HELL3,                  -1, 0,               QUEST_ANY,     TEXT_VILE3,    "Diablo"                   },
/*Q_BUTCHER*/ { DLV_CATHEDRAL2, DLV_CATHEDRAL2, 0,               QUEST_ANY,     TEXT_BUTCH9,   "The Butcher"              },
/*Q_LTBANNER*/{ DLV_CATHEDRAL4,             -1, 0,               QUEST_SINGLE,  TEXT_BANNER2,  "Ogden's Sign"             },
/*Q_BLIND*/   { DLV_CATACOMBS3,             -1, 0,               QUEST_SINGLE,  TEXT_BLINDING, "Halls of the Blind"       },
/*Q_BLOOD*/   { DLV_CATACOMBS1,             -1, 0,               QUEST_SINGLE,  TEXT_BLOODY,   "Valor"                    },
/*Q_ANVIL*/   { DLV_CAVES2,                 -1, 0,               QUEST_SINGLE,  TEXT_ANVIL5,   "Anvil of Fury"            },
/*Q_WARLORD*/ { DLV_HELL1,                  -1, 0,               QUEST_SINGLE,  TEXT_BLOODWAR, "Warlord of Blood"         },
/*Q_SKELKING*/{ DLV_CATHEDRAL3, DLV_CATHEDRAL3, SL_SKELKING,     QUEST_ANY,     TEXT_KING2,    "The Curse of King Leoric" },
/*Q_PWATER*/  { DLV_CATHEDRAL2,             -1, SL_POISONWATER,  QUEST_SINGLE,  TEXT_POISON3,  "Poisoned Water Supply"    },
/*Q_SCHAMB*/  { DLV_CATACOMBS2,             -1, SL_BONECHAMB,    QUEST_SINGLE,  TEXT_BONER,    "The Chamber of Bone"      },
/*Q_BETRAYER*/{ DLV_HELL3,           DLV_HELL3, SL_VILEBETRAYER, QUEST_ANY,     TEXT_VILE1,    "Archbishop Lazarus"       },
#ifdef HELLFIRE
/*Q_GRAVE*/   { DLV_NEST1,           DLV_NEST1, 0,               QUEST_ANY,     TEXT_GRAVE7,   "Grave Matters"            },
/*Q_FARMER*/  { DLV_CAVES1,         DLV_CAVES1, 0,               QUEST_ANY,     TEXT_FARMER1,  "Farmer's Orchard"         },
/*Q_GIRL*/    { DLV_NEST1,                  -1, 0,               QUEST_SINGLE,  TEXT_GIRL2,    "Little Girl"              },
/*Q_TRADER*/  { DLV_NEST3,                  -1, 0,               QUEST_SINGLE,  TEXT_TRADER,   "Wandering Trader"         },
/*Q_DEFILER*/ { DLV_NEST1,           DLV_NEST1, 0,               QUEST_ANY,     TEXT_DEFILER1, "The Defiler"              },
/*Q_NAKRUL*/  { DLV_CRYPT4,         DLV_CRYPT4, 0,               QUEST_ANY,     TEXT_NAKRUL1,  "Na-Krul"                  },
/*Q_JERSEY*/  { DLV_CAVES1,         DLV_CAVES1, 0,               QUEST_ANY,     TEXT_JERSEY4,  "The Jersey's Jersey"      },
#endif
	// clang-format on
};

/** Contains the data related to quest gossip for each towner ID. */
int Qtalklist[STORE_TOWNERS][NUM_QUESTS] = {
	// clang-format off
#ifdef HELLFIRE
	// Q_ROCK,      Q_MUSHROOM,  Q_GARBUD,   Q_ZHAR,      Q_VEIL,     Q_DIABLO,    Q_BUTCHER,   Q_LTBANNER,   Q_BLIND,     Q_BLOOD,     Q_ANVIL,      Q_WARLORD,    Q_SKELKING,  Q_PWATER,      Q_SCHAMB,   Q_BETRAYER,  Q_GRAVE,     Q_FARMER,  Q_GIRL,    Q_TRADER,  Q_DEFILER, Q_NAKRUL,  Q_JERSEY,
	{ TEXT_INFRA6,  TEXT_MUSH6,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL5, TEXT_NONE,   TEXT_BUTCH5, TEXT_BANNER6, TEXT_BLIND5, TEXT_BLOOD5, TEXT_ANVIL6,  TEXT_WARLRD5, TEXT_KING7,  TEXT_POISON7,  TEXT_BONE5, TEXT_VILE9,  TEXT_GRAVE2, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA3,  TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_VEIL3, TEXT_NONE,   TEXT_BUTCH3, TEXT_BANNER4, TEXT_BLIND3, TEXT_BLOOD3, TEXT_ANVIL3,  TEXT_WARLRD3, TEXT_KING5,  TEXT_POISON4,  TEXT_BONE3, TEXT_VILE7,  TEXT_GRAVE3, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA2,  TEXT_MUSH2,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL2, TEXT_NONE,   TEXT_BUTCH2, TEXT_NONE,    TEXT_BLIND2, TEXT_BLOOD2, TEXT_ANVIL2,  TEXT_WARLRD2, TEXT_KING3,  TEXT_POISON2,  TEXT_BONE2, TEXT_VILE4,  TEXT_GRAVE5, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA1,  TEXT_MUSH1,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL1, TEXT_VILE3,  TEXT_BUTCH1, TEXT_BANNER1, TEXT_BLIND1, TEXT_BLOOD1, TEXT_ANVIL1,  TEXT_WARLRD1, TEXT_KING1,  TEXT_POISON1,  TEXT_BONE1, TEXT_VILE2,  TEXT_GRAVE6, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA8,  TEXT_MUSH7,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL6, TEXT_NONE,   TEXT_BUTCH6, TEXT_BANNER7, TEXT_BLIND6, TEXT_BLOOD6, TEXT_ANVIL8,  TEXT_WARLRD6, TEXT_KING8,  TEXT_POISON8,  TEXT_BONE6, TEXT_VILE10, TEXT_GRAVE7, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA9,  TEXT_MUSH9,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL7, TEXT_NONE,   TEXT_BUTCH7, TEXT_BANNER8, TEXT_BLIND7, TEXT_BLOOD7, TEXT_ANVIL9,  TEXT_WARLRD7, TEXT_KING9,  TEXT_POISON9,  TEXT_BONE7, TEXT_VILE11, TEXT_GRAVE1, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA4,  TEXT_MUSH5,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL4, TEXT_NONE,   TEXT_BUTCH4, TEXT_BANNER5, TEXT_BLIND4, TEXT_BLOOD4, TEXT_ANVIL4,  TEXT_WARLRD4, TEXT_KING6,  TEXT_POISON6,  TEXT_BONE4, TEXT_VILE8,  TEXT_GRAVE8, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA10, TEXT_MUSH13, TEXT_NONE,  TEXT_NONE,   TEXT_VEIL8, TEXT_NONE,   TEXT_BUTCH8, TEXT_BANNER9, TEXT_BLIND8, TEXT_BLOOD8, TEXT_ANVIL10, TEXT_WARLRD8, TEXT_KING10, TEXT_POISON10, TEXT_BONE8, TEXT_VILE12, TEXT_GRAVE9, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
//	{ TEXT_NONE,    TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_NONE,   TEXT_NONE,    TEXT_NONE,   TEXT_NONE,   TEXT_NONE,    TEXT_NONE,    TEXT_NONE,   TEXT_NONE,     TEXT_NONE,  TEXT_NONE,   TEXT_NONE,   TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
#else
	// Q_ROCK,      Q_MUSHROOM,  Q_GARBUD,   Q_ZHAR,      Q_VEIL,     Q_DIABLO,    Q_BUTCHER,   Q_LTBANNER,   Q_BLIND,     Q_BLOOD,     Q_ANVIL,      Q_WARLORD,    Q_SKELKING,  Q_PWATER,      Q_SCHAMB,   Q_BETRAYER
	{ TEXT_INFRA6,  TEXT_MUSH6,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL5, TEXT_NONE,   TEXT_BUTCH5, TEXT_BANNER6, TEXT_BLIND5, TEXT_BLOOD5, TEXT_ANVIL6,  TEXT_WARLRD5, TEXT_KING7,  TEXT_POISON7,  TEXT_BONE5, TEXT_VILE9  },
	{ TEXT_INFRA3,  TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_VEIL3, TEXT_NONE,   TEXT_BUTCH3, TEXT_BANNER4, TEXT_BLIND3, TEXT_BLOOD3, TEXT_ANVIL3,  TEXT_WARLRD3, TEXT_KING5,  TEXT_POISON4,  TEXT_BONE3, TEXT_VILE7  },
	{ TEXT_INFRA2,  TEXT_MUSH2,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL2, TEXT_NONE,   TEXT_BUTCH2, TEXT_NONE,    TEXT_BLIND2, TEXT_BLOOD2, TEXT_ANVIL2,  TEXT_WARLRD2, TEXT_KING3,  TEXT_POISON2,  TEXT_BONE2, TEXT_VILE4  },
	{ TEXT_INFRA1,  TEXT_MUSH1,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL1, TEXT_VILE3,  TEXT_BUTCH1, TEXT_BANNER1, TEXT_BLIND1, TEXT_BLOOD1, TEXT_ANVIL1,  TEXT_WARLRD1, TEXT_KING1,  TEXT_POISON1,  TEXT_BONE1, TEXT_VILE2  },
	{ TEXT_INFRA8,  TEXT_MUSH7,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL6, TEXT_NONE,   TEXT_BUTCH6, TEXT_BANNER7, TEXT_BLIND6, TEXT_BLOOD6, TEXT_ANVIL8,  TEXT_WARLRD6, TEXT_KING8,  TEXT_POISON8,  TEXT_BONE6, TEXT_VILE10 },
	{ TEXT_INFRA9,  TEXT_MUSH9,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL7, TEXT_NONE,   TEXT_BUTCH7, TEXT_BANNER8, TEXT_BLIND7, TEXT_BLOOD7, TEXT_ANVIL9,  TEXT_WARLRD7, TEXT_KING9,  TEXT_POISON9,  TEXT_BONE7, TEXT_VILE11 },
	{ TEXT_INFRA4,  TEXT_MUSH5,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL4, TEXT_NONE,   TEXT_BUTCH4, TEXT_BANNER5, TEXT_BLIND4, TEXT_BLOOD4, TEXT_ANVIL4,  TEXT_WARLRD4, TEXT_KING6,  TEXT_POISON6,  TEXT_BONE4, TEXT_VILE8  },
	{ TEXT_INFRA10, TEXT_MUSH13, TEXT_NONE,  TEXT_NONE,   TEXT_VEIL8, TEXT_NONE,   TEXT_BUTCH8, TEXT_BANNER9, TEXT_BLIND8, TEXT_BLOOD8, TEXT_ANVIL10, TEXT_WARLRD8, TEXT_KING10, TEXT_POISON10, TEXT_BONE8, TEXT_VILE12 },
//	{ TEXT_KING1,   TEXT_KING1,  TEXT_KING1, TEXT_KING1,  TEXT_KING1, TEXT_KING1,  TEXT_KING1,  TEXT_KING1,   TEXT_KING1,  TEXT_KING1,  TEXT_KING1,   TEXT_KING1,   TEXT_KING1,  TEXT_KING1,    TEXT_KING1, TEXT_KING1  }
#endif
	// clang-format on
};

DEVILUTION_END_NAMESPACE
