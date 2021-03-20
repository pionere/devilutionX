/**
 * @file questdat.cpp
 *
 * Implementation of all quest and level data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

const LevelDataStruct AllLevels[NUMLEVELS] = {
	// clang-format off
					// dLevel, dType,           dDunType,        dMusic,
/*DLV_TOWN*/		{       0, DTYPE_TOWN,      DTYPE_TOWN,      TMUSIC_TOWN,
					//  dDunCels,                      dMegaTiles,                    dLvlPieces,                    dSpecCels,                     dLoadCels,              dLoadPal,                dAutomapData
#ifdef HELLFIRE
						"NLevels\\TownData\\Town.CEL", "NLevels\\TownData\\Town.TIL", "NLevels\\TownData\\Town.MIN", "Levels\\TownData\\TownS.CEL", "Gendata\\Cuttt.CEL",   "Gendata\\Cuttt.pal",    NULL,
#else
						"Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "Gendata\\Cuttt.CEL",   "Gendata\\Cuttt.pal",    NULL,
#endif
					},
/*DLV_CATHEDRAL1*/	{       1, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,
						"Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\L1.AMP",
					},
/*DLV_CATHEDRAL2*/	{       2, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,   
						"Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\L1.AMP",
					},
/*DLV_CATHEDRAL3*/	{       3, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,   
						"Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\L1.AMP",
					},
/*DLV_CATHEDRAL4*/	{       4, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,   
						"Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\L1.AMP",
					},
/*DLV_CATACOMBS1*/	{       5, DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,
						"Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     "Levels\\L2Data\\L2.AMP",
					},
/*DLV_CATACOMBS2*/	{       6, DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,   
						"Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     "Levels\\L2Data\\L2.AMP",
					},
/*DLV_CATACOMBS3*/	{       7, DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,   
						"Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     "Levels\\L2Data\\L2.AMP",
					},
/*DLV_CATACOMBS4*/	{       8, DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,   
						"Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     "Levels\\L2Data\\L2.AMP",
					},
/*DLV_CAVES1*/		{       9, DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,   
						"Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     "Levels\\L3Data\\L3.AMP",
					},
/*DLV_CAVES2*/		{      10, DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,   
						"Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     "Levels\\L3Data\\L3.AMP",
					},
/*DLV_CAVES3*/		{      11, DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,   
						"Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     "Levels\\L3Data\\L3.AMP",
					},
/*DLV_CAVES4*/		{      12, DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,   
						"Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     "Levels\\L3Data\\L3.AMP",
					},
/*DLV_HELL1*/		{      13, DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,   
						"Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     "Levels\\L4Data\\L4.AMP",
					},
/*DLV_HELL2*/		{      14, DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,   
						"Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     "Levels\\L4Data\\L4.AMP",
					},
/*DLV_HELL3*/		{      15, DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,   
						"Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     "Levels\\L4Data\\L4.AMP",
					},
/*DLV_HELL4*/		{      16, DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,   
						"Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Gendata\\Cutgate.CEL", "Gendata\\Cutgate.pal",  "Levels\\L4Data\\L4.AMP",
					},
#ifdef HELLFIRE
/*DLV_HIVE1*/		{       9, DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,   
						"NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    "NLevels\\L6Data\\L6.AMP",
					},
/*DLV_HIVE2*/		{      10, DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,   
						"NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    "NLevels\\L6Data\\L6.AMP",
					},
/*DLV_HIVE3*/		{      11, DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,   
						"NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    "NLevels\\L6Data\\L6.AMP",
					},
/*DLV_HIVE4*/		{      12, DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,   
						"NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    "NLevels\\L6Data\\L6.AMP",
					},
/*DLV_CRYPT1*/		{      14, DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,
						"NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    "NLevels\\L5Data\\L5.AMP",
					},
/*DLV_CRYPT2*/		{      15, DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,   
						"NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    "NLevels\\L5Data\\L5.AMP",
					},
/*DLV_CRYPT3*/		{      16, DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,   
						"NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    "NLevels\\L5Data\\L5.AMP",
					},
/*DLV_CRYPT4*/		{      17, DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,   
						"NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    "NLevels\\L5Data\\L5.AMP",
					},
#endif
	// clang-format on
};

const int gnLevelTypeTbl[NUMLEVELS] = { DTYPE_TOWN, 
	DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL,
	DTYPE_CATACOMBS, DTYPE_CATACOMBS, DTYPE_CATACOMBS, DTYPE_CATACOMBS,
	DTYPE_CAVES, DTYPE_CAVES, DTYPE_CAVES, DTYPE_CAVES,
	DTYPE_HELL, DTYPE_HELL, DTYPE_HELL, DTYPE_HELL,
#ifdef HELLFIRE
	DTYPE_CAVES, DTYPE_CAVES, DTYPE_CAVES, DTYPE_CAVES, // Hive
	DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, // Crypt
#endif
};

/** map from setlevel to setleveltype. */
const int gnSetLevelTypeTbl[NUM_SETLVL] = {
	DTYPE_NONE,			// //SL_BUTCHCHAMB = 0x0,
	DTYPE_CATHEDRAL,	// SL_SKELKING     = 0x1,
	DTYPE_CATACOMBS,	// SL_BONECHAMB    = 0x2,
	DTYPE_NONE,			// //SL_MAZE         = 0x3,
	DTYPE_CAVES,		// SL_POISONWATER  = 0x4,
	DTYPE_CATHEDRAL,	// SL_VILEBETRAYER = 0x5,
};

/** Contains the data related to each quest_id. */
const QuestData questlist[NUM_QUESTS] = {
	// clang-format off
	          // _qdlvl, _qdmultlvl, _qslvl,          _qflags,       _qdmsg,        _qlstr
/*Q_ROCK*/	  {       5,         -1, 0,               QUEST_SINGLE,  TEXT_INFRA5,   "The Magic Rock"           },
/*Q_MUSHROOM*/{       9,         -1, 0,               QUEST_SINGLE,  TEXT_MUSH8,    "Black Mushroom"           },
/*Q_GARBUD*/  {       4,         -1, 0,               QUEST_SINGLE,  TEXT_GARBUD1,  "Gharbad The Weak"         },
/*Q_ZHAR*/    {       8,         -1, 0,               QUEST_SINGLE,  TEXT_ZHAR1,    "Zhar the Mad"             },
/*Q_VEIL*/    {      14,         -1, 0,               QUEST_SINGLE,  TEXT_VEIL9,    "Lachdanan"                },
/*Q_DIABLO*/  {      15,         -1, 0,               QUEST_ANY,     TEXT_VILE3,    "Diablo"                   },
/*Q_BUTCHER*/ {       2,          2, 0,               QUEST_ANY,     TEXT_BUTCH9,   "The Butcher"              },
/*Q_LTBANNER*/{       4,         -1, 0,               QUEST_SINGLE,  TEXT_BANNER2,  "Ogden's Sign"             },
/*Q_BLIND*/   {       7,         -1, 0,               QUEST_SINGLE,  TEXT_BLINDING, "Halls of the Blind"       },
/*Q_BLOOD*/   {       5,         -1, 0,               QUEST_SINGLE,  TEXT_BLOODY,   "Valor"                    },
/*Q_ANVIL*/   {      10,         -1, 0,               QUEST_SINGLE,  TEXT_ANVIL5,   "Anvil of Fury"            },
/*Q_WARLORD*/ {      13,         -1, 0,               QUEST_SINGLE,  TEXT_BLOODWAR, "Warlord of Blood"         },
/*Q_SKELKING*/{       3,          3, SL_SKELKING,     QUEST_ANY,     TEXT_KING2,    "The Curse of King Leoric" },
/*Q_PWATER*/  {       2,         -1, SL_POISONWATER,  QUEST_SINGLE,  TEXT_POISON3,  "Poisoned Water Supply"    },
/*Q_SCHAMB*/  {       6,         -1, SL_BONECHAMB,    QUEST_SINGLE,  TEXT_BONER,    "The Chamber of Bone"      },
/*Q_BETRAYER*/{      15,         15, SL_VILEBETRAYER, QUEST_ANY,     TEXT_VILE1,    "Archbishop Lazarus"       },
#ifdef HELLFIRE
/*Q_GRAVE*/   {      17,         17, 0,               QUEST_ANY,     TEXT_GRAVE7,   "Grave Matters"            },
/*Q_FARMER*/  {       9,          9, 0,               QUEST_ANY,     TEXT_FARMER1,  "Farmer's Orchard"         },
/*Q_GIRL*/    {      17,         -1, 0,               QUEST_SINGLE,  TEXT_GIRL2,    "Little Girl"              },
/*Q_TRADER*/  {      19,         -1, 0,               QUEST_SINGLE,  TEXT_TRADER,   "Wandering Trader"         },
/*Q_DEFILER*/ {      17,         17, 0,               QUEST_ANY,     TEXT_DEFILER1, "The Defiler"              },
/*Q_NAKRUL*/  {      21,         21, 0,               QUEST_ANY,     TEXT_NAKRUL1,  "Na-Krul"                  },
/*Q_JERSEY*/  {       9,          9, 0,               QUEST_ANY,     TEXT_JERSEY4,  "The Jersey's Jersey"      },
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
