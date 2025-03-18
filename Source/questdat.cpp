/**
 * @file questdat.cpp
 *
 * Implementation of all quest and level data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

const LevelData AllLevels[NUM_FIXLVLS] = {
	// clang-format off
					// dLevel, dSetLvl, dType,           dDunType,      dMusic,      dfindex, dMicroTileLen, dBlocks, dLevelName,
/*DLV_TOWN*/		{       0, FALSE,   DTYPE_TOWN,      DGT_TOWN,      TMUSIC_TOWN, LFILE_TOWN,         16,      16, "Tristram",
					//  dPalName,                        dLoadCels,               dLoadPal,                dLoadBarOnTop, dLoadBarColor, dMonDensity, dObjDensity, dSetLvlDunX,   dSetLvlDunY,              dSetLvlWarp,  dSetLvlPiece,
						"Levels\\TownData\\Town.pal",    "Gendata\\Cuttt.CEL",    "Gendata\\Cuttt.pal",            FALSE,            43, 0,           0,           0,             0,                        WRPT_NONE,    SPT_NONE,
					//  dMonTypes,
						{ MT_INVALID },
					},
/*DLV_CATHEDRAL1*/	{       2, FALSE,   DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_L1,   LFILE_L1,           10,      10, "Cathedral 1",
						"Levels\\L1Data\\L1_%d.PAL",     "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 40,          24,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_NZOMBIE, MT_RFALLSP, MT_WSKELAX, MT_RFALLSD, MT_NSCAV, MT_WSKELSD, MT_INVALID },
					},
/*DLV_CATHEDRAL2*/	{       4, FALSE,   DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_L1,   LFILE_L1,           10,      10, "Cathedral 2",
						"Levels\\L1Data\\L1_%d.PAL",     "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 40,          24,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_NZOMBIE, MT_RFALLSP, MT_WSKELAX, MT_RFALLSD, MT_NSCAV, MT_WSKELSD, MT_BZOMBIE, MT_GZOMBIE,  MT_DFALLSP, MT_YFALLSP, MT_TSKELAX, MT_RSKELAX, MT_DFALLSD, MT_YFALLSD, MT_WSKELBW, MT_TSKELBW, MT_BSCAV, MT_TSKELSD, MT_NSNEAK, MT_RBAT, MT_INVALID },
					},
/*DLV_CATHEDRAL3*/	{       6, FALSE,   DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_L1,   LFILE_L1,           10,      10, "Cathedral 3",
						"Levels\\L1Data\\L1_%d.PAL",     "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 40,          24,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_NSCAV, MT_WSKELSD, MT_BZOMBIE, MT_GZOMBIE, MT_DFALLSP, MT_YFALLSP, MT_TSKELAX, MT_RSKELAX, MT_DFALLSD, MT_YFALLSD, MT_WSKELBW, MT_TSKELBW, MT_BSCAV, MT_TSKELSD, MT_NSNEAK, MT_RBAT, MT_NBAT, MT_YZOMBIE, MT_BFALLSP, MT_XSKELAX, MT_BFALLSD, MT_WSCAV, MT_RSKELBW, MT_RSKELSD, MT_INVALID },
					},
/*DLV_CATHEDRAL4*/	{       8, FALSE,   DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_L1,   LFILE_L1,           10,      10, "Cathedral 4",
						"Levels\\L1Data\\L1_%d.PAL",     "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 40,          24,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_GZOMBIE, MT_YFALLSP, MT_RSKELAX, MT_YFALLSD, MT_TSKELBW, MT_BSCAV, MT_TSKELSD, MT_NSNEAK, MT_NBAT, MT_YZOMBIE, MT_BFALLSP, MT_XSKELAX, MT_BFALLSD, MT_WSCAV, MT_RSKELBW, MT_RSKELSD, MT_GBAT, MT_YSCAV, MT_XSKELBW, MT_XSKELSD, MT_NGOATMC, MT_NGOATBW, MT_INVALID },
					},
/*DLV_CATACOMBS1*/	{      10, FALSE,   DTYPE_CATACOMBS, DGT_CATACOMBS, TMUSIC_L2,   LFILE_L2,           10,      10, "Catacombs 1",
						"Levels\\L2Data\\L2_%d.PAL",     "Gendata\\Cut2.CEL",     "Gendata\\Cut2.pal",              TRUE,           254, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_NSNEAK, MT_NBAT, MT_YZOMBIE, MT_BFALLSP, MT_XSKELAX, MT_BFALLSD, MT_WSCAV, MT_RSKELBW, MT_RSKELSD, MT_GBAT, MT_YSCAV, MT_XSKELBW, MT_XSKELSD, MT_NGOATMC, MT_NGOATBW, MT_BGOATBW, MT_BGOATMC, MT_RSNEAK, MT_NFAT, MT_NGARG, MT_INVALID },
					},
/*DLV_CATACOMBS2*/	{      12, FALSE,   DTYPE_CATACOMBS, DGT_CATACOMBS, TMUSIC_L2,   LFILE_L2,           10,      10, "Catacombs 2",
						"Levels\\L2Data\\L2_%d.PAL",     "Gendata\\Cut2.CEL",     "Gendata\\Cut2.pal",              TRUE,           254, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_GBAT, MT_YSCAV, MT_XSKELBW, MT_XSKELSD, MT_NGOATMC, MT_NGOATBW, MT_BGOATBW, MT_BGOATMC, MT_RSNEAK, MT_NFAT, MT_NGARG, MT_BSNEAK, MT_RGOATMC, MT_XBAT, MT_RGOATBW, MT_NACID, MT_INVALID },
					},
/*DLV_CATACOMBS3*/	{      14, FALSE,   DTYPE_CATACOMBS, DGT_CATACOMBS, TMUSIC_L2,   LFILE_L2,           10,      10, "Catacombs 3",
						"Levels\\L2Data\\L2_%d.PAL",     "Gendata\\Cut2.CEL",     "Gendata\\Cut2.pal",              TRUE,           254, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_BGOATBW, MT_BGOATMC, MT_RSNEAK, MT_NFAT, MT_NGARG, MT_BSNEAK, MT_RGOATMC, MT_XBAT, MT_RGOATBW, MT_NACID, MT_GGOATBW, MT_GGOATMC, MT_BFAT, MT_NRHINO, MT_XGARG, MT_INVALID },
					},
/*DLV_CATACOMBS4*/	{      16, FALSE,   DTYPE_CATACOMBS, DGT_CATACOMBS, TMUSIC_L2,   LFILE_L2,           10,      10, "Catacombs 4",
						"Levels\\L2Data\\L2_%d.PAL",     "Gendata\\Cut2.CEL",     "Gendata\\Cut2.pal",              TRUE,           254, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_BSNEAK, MT_RGOATMC, MT_XBAT, MT_RGOATBW, MT_NACID, MT_GGOATBW, MT_GGOATMC, MT_BFAT, MT_NRHINO, MT_XGARG, MT_XRHINO, MT_YSNEAK, MT_RACID, MT_XFAT, MT_NMAGMA, MT_YMAGMA, MT_INVALID },
					},
/*DLV_CAVES1*/		{      18, FALSE,   DTYPE_CAVES,     DGT_CAVES,     TMUSIC_L3,   LFILE_L3,           10,      10, "Caves 1",
						"Levels\\L3Data\\L3_%d.PAL",     "Gendata\\Cut3.CEL",     "Gendata\\Cut3.pal",             FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_GGOATBW, MT_GGOATMC, MT_BFAT, MT_NRHINO, MT_XGARG, MT_XRHINO, MT_YSNEAK, MT_RACID, MT_XFAT, MT_NMAGMA, MT_YMAGMA, MT_DGARG, MT_BMAGMA, MT_WMAGMA, MT_BRHINO, MT_RTHIN, MT_INVALID },
					},
/*DLV_CAVES2*/		{      20, FALSE,   DTYPE_CAVES,     DGT_CAVES,     TMUSIC_L3,   LFILE_L3,           10,      10, "Caves 2",
						"Levels\\L3Data\\L3_%d.PAL",     "Gendata\\Cut3.CEL",     "Gendata\\Cut3.pal",             FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_XRHINO, MT_YSNEAK, MT_RACID, MT_XFAT, MT_YMAGMA, MT_DGARG, MT_BMAGMA, MT_WMAGMA, MT_BRHINO, MT_RTHIN, MT_DRHINO, MT_BACID, MT_RFAT, MT_NTHIN, MT_BGARG, MT_NMEGA, MT_INVALID },
					},
/*DLV_CAVES3*/		{      22, FALSE,   DTYPE_CAVES,     DGT_CAVES,     TMUSIC_L3,   LFILE_L3,           10,      10, "Caves 3",
						"Levels\\L3Data\\L3_%d.PAL",     "Gendata\\Cut3.CEL",     "Gendata\\Cut3.pal",             FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_DGARG, MT_BMAGMA, MT_WMAGMA, MT_BRHINO, MT_RTHIN, MT_DRHINO, MT_BACID, MT_RFAT, MT_NTHIN, MT_BGARG, MT_NMEGA, MT_DMEGA, MT_NSNAKE, MT_XTHIN, MT_INVALID },
					},
/*DLV_CAVES4*/		{      24, FALSE,   DTYPE_CAVES,     DGT_CAVES,     TMUSIC_L3,   LFILE_L3,           10,      10, "Caves 4",
						"Levels\\L3Data\\L3_%d.PAL",     "Gendata\\Cut3.CEL",     "Gendata\\Cut3.pal",             FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_DRHINO, MT_BACID, MT_RFAT, MT_NTHIN, MT_BGARG, MT_NMEGA, MT_DMEGA, MT_NSNAKE, MT_XTHIN, MT_RSNAKE, MT_NBLACK, MT_NSUCC, MT_BMEGA, MT_XACID, MT_GTHIN, MT_INVALID },
					},
/*DLV_HELL1*/		{      26, FALSE,   DTYPE_HELL,      DGT_HELL,      TMUSIC_L4,   LFILE_L4,           12,      16, "Hell 1",
						"Levels\\L4Data\\L4_%d.PAL",     "Gendata\\Cut4.CEL",     "Gendata\\Cut4.pal",             FALSE,            43, 46,          30,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_DMEGA, MT_NSNAKE, MT_XTHIN, MT_RSNAKE, MT_NBLACK, MT_NSUCC, MT_BMEGA, MT_XACID, MT_GTHIN, MT_RMEGA, MT_BSNAKE, MT_RBLACK, MT_GBLACK, MT_GSUCC, MT_NMAGE, MT_INVALID },
					},
/*DLV_HELL2*/		{      28, FALSE,   DTYPE_HELL,      DGT_HELL,      TMUSIC_L4,   LFILE_L4,           12,      16, "Hell 2",
						"Levels\\L4Data\\L4_%d.PAL",     "Gendata\\Cut4.CEL",     "Gendata\\Cut4.pal",             FALSE,            43, 46,          30,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_RSNAKE, MT_NBLACK, MT_NSUCC, MT_BMEGA, MT_XACID, MT_GTHIN, MT_RMEGA, MT_BSNAKE, MT_RBLACK, MT_GBLACK, MT_GSUCC, MT_NMAGE, MT_GMAGE, MT_SMAGE, MT_BBLACK, MT_RSUCC, MT_INVALID },
					},
/*DLV_HELL3*/		{      30, FALSE,   DTYPE_HELL,      DGT_HELL,      TMUSIC_L4,   LFILE_L4,           12,      16, "Hell 3",
						"Levels\\L4Data\\L4_%d.PAL",     "Gendata\\Cut4.CEL",     "Gendata\\Cut4.pal",             FALSE,            43, 46,          30,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_RMEGA, MT_RBLACK, MT_GSUCC, MT_GMAGE, MT_BBLACK, MT_RSUCC, MT_GSNAKE, MT_BSUCC, MT_XMAGE, MT_SMAGE, MT_OMAGE, MT_INVALID },
					},
/*DLV_HELL4*/		{      32, FALSE,   DTYPE_HELL,      DGT_HELL,      TMUSIC_L4,   LFILE_L4,           12,      16, "Diablo",
						"Levels\\L4Data\\L4_%d.PAL",     "Gendata\\Cutgate.CEL",  "Gendata\\Cutgate.pal",          FALSE,            43, 46,          30,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_INVALID }, /* MT_GBLACK, MT_BMAGE, MT_NBLACK, MT_DIABLO */
					},
#ifdef HELLFIRE
/*DLV_NEST1*/		{      18, FALSE,   DTYPE_NEST,      DGT_CAVES,     TMUSIC_L6,   LFILE_L6,           10,      10, "Nest 1",
						"NLevels\\L6Data\\L6Base%d.PAL", "NLevels\\Cutl6.CEL",    "NLevels\\Cutl6.pal",            FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_HELLBOAR, MT_STINGER, MT_PSYCHORB, MT_ARACHNON, MT_FELLTWIN, MT_UNRAV, MT_INVALID },
					},
/*DLV_NEST2*/		{      20, FALSE,   DTYPE_NEST,      DGT_CAVES,     TMUSIC_L6,   LFILE_L6,           10,      10, "Nest 2",
						"NLevels\\L6Data\\L6Base%d.PAL", "NLevels\\Cutl6.CEL",    "NLevels\\Cutl6.pal",            FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_HELLBOAR, MT_STINGER, MT_PSYCHORB, MT_ARACHNON, MT_FELLTWIN, MT_UNRAV, MT_INVALID }, /* MT_HORKSPWN */
					},
/*DLV_NEST3*/		{      22, FALSE,   DTYPE_NEST,      DGT_CAVES,     TMUSIC_L6,   LFILE_L6,           10,      10, "Nest 3",
						"NLevels\\L6Data\\L6Base%d.PAL", "NLevels\\Cutl6.CEL",    "NLevels\\Cutl6.pal",            FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_VENMTAIL, MT_NECRMORB, MT_SPIDLORD, MT_LASHWORM, MT_TORCHANT, MT_INVALID }, /* MT_HORKSPWN, MT_HORKDMN */
					},
/*DLV_NEST4*/		{      24, FALSE,   DTYPE_NEST,      DGT_CAVES,     TMUSIC_L6,   LFILE_L6,           10,      10, "Nest 4",
						"NLevels\\L6Data\\L6Base%d.PAL", "NLevels\\Cutl6.CEL",    "NLevels\\Cutl6.pal",            FALSE,            43, 50,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_VENMTAIL, MT_NECRMORB, MT_SPIDLORD, MT_LASHWORM, MT_TORCHANT, MT_INVALID }, /* MT_DEFILER */
					},
/*DLV_CRYPT1*/		{      26, FALSE,   DTYPE_CRYPT,     DGT_CATHEDRAL, TMUSIC_L5,   LFILE_L5,           10,      10, "Crypt 1",
						"NLevels\\L5Data\\L5Base.PAL",   "NLevels\\Cutl5.CEL",    "NLevels\\Cutl5.pal",            FALSE,            43, 48,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_LRDSAYTR, MT_GRAVEDIG, MT_BIGFALL, MT_TOMBRAT, MT_FIREBAT, MT_LICH, MT_INVALID },
					},
/*DLV_CRYPT2*/		{      28, FALSE,   DTYPE_CRYPT,     DGT_CATHEDRAL, TMUSIC_L5,   LFILE_L5,           10,      10, "Crypt 2",
						"NLevels\\L5Data\\L5Base.PAL",   "NLevels\\Cutl5.CEL",    "NLevels\\Cutl5.pal",            FALSE,            43, 48,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_BIGFALL, MT_TOMBRAT, MT_FIREBAT, MT_SKLWING, MT_LICH, MT_CRYPTDMN, MT_INVALID },
					},
/*DLV_CRYPT3*/		{      30, FALSE,   DTYPE_CRYPT,     DGT_CATHEDRAL, TMUSIC_L5,   LFILE_L5,           10,      10, "Crypt 3",
						"NLevels\\L5Data\\L5Base.PAL",   "NLevels\\Cutl5.CEL",    "NLevels\\Cutl5.pal",            FALSE,            43, 48,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_CRYPTDMN, MT_HELLBAT, MT_BONEDEMN, MT_ARCHLICH, MT_BICLOPS, MT_FLESTHNG, MT_REAPER, MT_INVALID },
					},
/*DLV_CRYPT4*/		{      32, FALSE,   DTYPE_CRYPT,     DGT_CATHEDRAL, TMUSIC_L5,   LFILE_L5,           10,      10, "Crypt 4",
						"NLevels\\L5Data\\L5Base.PAL",   "NLevels\\Cutl5.CEL",    "NLevels\\Cutl5.pal",            FALSE,            43, 48,          32,          0,             0,                        WRPT_NONE,    SPT_NONE,
						{ MT_HELLBAT, MT_BICLOPS, MT_FLESTHNG, MT_REAPER, MT_INVALID }, /* MT_ARCHLICH, MT_NAKRUL */
					},
#endif
					// dLevel, dSetLvl, dType,           dDunType,      dMusic,      dfindex, dMicroTileLen, dBlocks, dLevelName,
/*SL_BUTCHCHAMB*//*	{       0, TRUE,    DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_TOWN, LFILE_L1,           16,      16, "",
					//  dPalName,                        dLoadCels,               dLoadPal,                dLoadBarOnTop, dLoadBarColor, dMonDensity, dObjDensity, dSetLvlDunX,   dSetLvlDunY,              dSetLvlWarp,  dSetLvlPiece,
						"",                              "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 0,           0,           0,             0,                        WRPT_NONE,    SPT_NONE,
					//  dMonTypes,
						{ MT_INVALID },
					},*/
/*SL_SKELKING*/		{       6, TRUE,    DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_L1,   LFILE_L1,           10,      10, "Skeleton King's Lair",
						"Levels\\L1Data\\L1_2.pal",      "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 120,         0,           DBORDERX+2*33, DBORDERY+2*13,            WRPT_L1_UP,   SPT_LVL_SKELKING,
						{ MT_RSKELAX, MT_TSKELBW, MT_RSKELBW, MT_XSKELBW, MT_RSKELSD, MT_INVALID },
					},
/*SL_BONECHAMB*/	{      12, TRUE,    DTYPE_CATACOMBS, DGT_CATACOMBS, TMUSIC_L2,   LFILE_L2,           10,      10, "Chamber of Bone",
						"Levels\\L2Data\\L2_2.pal",      "Gendata\\Cut2.CEL",     "Gendata\\Cut2.pal",              TRUE,           254, 0,           0,           DBORDERX+2*27, DBORDERY+2*11+1,          WRPT_L2_DOWN, SPT_LVL_BCHAMB,
						{ MT_INVALID }, /* MT_XSKELSD, MT_BSNEAK, MT_NRHINO */
					},
/*SL_MAZE*//*		{       0, TRUE,    DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_TOWN, LFILE_L1,           10,      10, "Maze",
						"Levels\\L1Data\\L1_5.pal",      "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 0,           0,           DBORDERX+4,    DBORDERY+34,              WRPT_NONE,    SPT_MAZE,
						{ MT_INVALID }, / * MT_XRHINO * /
					},*/
/*SL_POISONWATER*/	{       4, TRUE,    DTYPE_CAVES,     DGT_CAVES,     TMUSIC_L3,   LFILE_L3,           10,      10, "Poisoned Water Supply",
						"Levels\\L3Data\\L3pfoul.pal",   "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",            TRUE,           138, 100,         0,           DBORDERX+2*7,  DBORDERY+2*33+1,          WRPT_L3_DOWN, SPT_LVL_PWATER,
						{ MT_DFALLSP, MT_YFALLSD, MT_INVALID }, /* , MT_NGOATMC, MT_NGOATBW */
					},
/*SL_VILEBETRAYER*/	{      30, TRUE,    DTYPE_CATHEDRAL, DGT_CATHEDRAL, TMUSIC_L1,   LFILE_L1,           10,      10, "Archbishop Lazarus' Lair",
						"Levels\\L1Data\\L1_2.pal",      "Gendata\\Cutportr.CEL", "Gendata\\Cutportr.pal",         FALSE,            43, 0,           0,           DBORDERX+2*9+1, DBORDERY+2*10,           WRPT_CIRCLE,  SPT_LVL_BETRAYER,
						{ MT_INVALID }, /* MT_RSUCC, MT_BMAGE */
					},
	// clang-format on
};

const LevelFileData levelfiledata[NUM_LFILE_TYPES] = {
	// clang-format off
//               dSubtileSettings,              dTileFlags,                dMicroCels,                    dMegaTiles,                    dMiniTiles,                    dSpecCels,                     dLightTrns,
#ifdef HELLFIRE
/*LFILE_TOWN*/ { "NLevels\\TownData\\Town.SLA", NULL,                      "NLevels\\TownData\\Town.CEL", "NLevels\\TownData\\Town.TIL", "NLevels\\TownData\\Town.MIN", "Levels\\TownData\\TownS.CEL", "Levels\\TownData\\Town.TRS" },
#else
/*LFILE_TOWN*/ { "Levels\\TownData\\Town.SLA",  NULL,                      "Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "Levels\\TownData\\Town.TRS" },
#endif
/*LFILE_L1*/   { "Levels\\L1Data\\L1.SLA",      "Levels\\L1Data\\L1.TLA",  "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1.TRS"     },
/*LFILE_L2*/   { "Levels\\L2Data\\L2.SLA",      "Levels\\L2Data\\L2.TLA",  "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2.TRS"     },
/*LFILE_L3*/   { "Levels\\L3Data\\L3.SLA",      "Levels\\L3Data\\L3.TLA",  "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      NULL,                          "Levels\\L3Data\\L3.TRS"     },
/*LFILE_L4*/   { "Levels\\L4Data\\L4.SLA",      "Levels\\L4Data\\L4.TLA",  "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      NULL,                          "Levels\\L4Data\\L4.TRS"     },
#ifdef HELLFIRE
/*LFILE_L5*/   { "NLevels\\L5Data\\L5.SLA",     "NLevels\\L5Data\\L5.TLA", "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5.TRS"    },
/*LFILE_L6*/   { "NLevels\\L6Data\\L6.SLA",     "NLevels\\L6Data\\L6.TLA", "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     NULL,                          "NLevels\\L6Data\\L6.TRS"    },
#endif
	// clang-format on
};

/** Contains the data related to each quest_id. */
const QuestData questlist[NUM_QUESTS] = {
	// clang-format off
	          // _qdlvl,       _qslvl,           _qdmsg,        _qlstr
/*Q_BUTCHER*/ { DLV_CATHEDRAL2, 0,               TEXT_BUTCH9,   "The Butcher"              },
/*Q_PWATER*/  { DLV_CATHEDRAL2, SL_POISONWATER,  TEXT_POISON3,  "Poisoned Water Supply"    },
/*Q_SKELKING*/{ DLV_CATHEDRAL3, SL_SKELKING,     TEXT_KING2,    "The Curse of King Leoric" },
/*Q_BANNER*/  { DLV_CATHEDRAL4, 0,               TEXT_BANNER2,  "Ogden's Sign"             },
/*Q_GARBUD*/  { DLV_CATHEDRAL4, 0,               TEXT_GARBUD1,  "Gharbad The Weak"         },
/*Q_ROCK*/    { DLV_CATACOMBS1, 0,               TEXT_INFRA5,   "The Magic Rock"           },
/*Q_BLOOD*/   { DLV_CATACOMBS1, 0,               TEXT_BLOODY,   "Valor"                    },
/*Q_BCHAMB*/  { DLV_CATACOMBS2, SL_BONECHAMB,    TEXT_BONER,    "The Chamber of Bone"      },
/*Q_BLIND*/   { DLV_CATACOMBS3, 0,               TEXT_BLINDING, "Halls of the Blind"       },
/*Q_ZHAR*/    { DLV_CATACOMBS4, 0,               TEXT_ZHAR1,    "Zhar the Mad"             },
/*Q_MUSHROOM*/{ DLV_CAVES1,     0,               TEXT_MUSH8,    "Black Mushroom"           },
/*Q_ANVIL*/   { DLV_CAVES2,     0,               TEXT_ANVIL5,   "Anvil of Fury"            },
/*Q_WARLORD*/ { DLV_HELL1,      0,               TEXT_BLOODWAR, "Warlord of Blood"         },
/*Q_VEIL*/    { DLV_HELL2,      0,               TEXT_VEIL9,    "Lachdanan"                },
/*Q_BETRAYER*/{ DLV_HELL3,      SL_VILEBETRAYER, TEXT_VILE1,    "Archbishop Lazarus"       },
/*Q_DIABLO*/  { DLV_HELL3,      0,               TEXT_VILE3,    "Diablo"                   },
#ifdef HELLFIRE
/*Q_JERSEY*/  { DLV_CAVES1,     0,               TEXT_JERSEY4,  "The Jersey's Jersey"      },
/*Q_FARMER*/  { DLV_CAVES1,     0,               TEXT_FARMER1,  "Farmer's Orchard"         },
/*Q_GIRL*/    { DLV_NEST1,      0,               TEXT_GIRL2,    "Little Girl"              },
/*Q_DEFILER*/ { DLV_NEST1,      0,               TEXT_DM_NEST,  "The Defiler"              },
/*Q_TRADER*///{ DLV_NEST3,      0,               TEXT_TRADER,   "Wandering Trader"         },
/*Q_GRAVE*/   { DLV_CRYPT1,     0,               TEXT_GRAVE8,   "Grave Matters"            },
/*Q_NAKRUL*/  { DLV_CRYPT4,     0,               TEXT_NONE,     "Na-Krul"                  },
#endif
	// clang-format on
};

const SetPieceData setpiecedata[NUM_SPT_TYPES] = {
	// clang-format off
//                     _spdDunFile,                   _spdPreDunFile
/*SPT_NONE*/         { NULL,                            NULL                           },
/*SPT_BANNER*/       { "Levels\\L1Data\\Banner1.DUN",   "Levels\\L1Data\\Banner2.DUN"  },
/*SPT_SKELKING*/     { "Levels\\L1Data\\SKngDO.DUN",    NULL                           },
/*SPT_BUTCHER*/      { "Levels\\L1Data\\Butcher.DUN",   NULL                           },
/*SPT_MAZE*///       { "Levels\\L1Data\\Lv1MazeA.DUN",  "Levels\\L1Data\\Lv1MazeB.DUN" },
/*SPT_BLIND*/        { "Levels\\L2Data\\Blind1.DUN",    "Levels\\L2Data\\Blind2.DUN"   },
/*SPT_BLOOD*/        { "Levels\\L2Data\\Blood1.DUN",    "Levels\\L2Data\\Blood2.DUN"   },
/*SPT_BCHAMB*/       { "Levels\\L2Data\\Bonestr2.DUN",  "Levels\\L2Data\\Bonestr1.DUN" },
/*SPT_ANVIL*/        { "Levels\\L3Data\\Anvil.DUN",     NULL                           },
/*SPT_BETRAY_S*/     { "Levels\\L4Data\\Viles.DUN",     NULL                           },
/*SPT_BETRAY_M*/     { "Levels\\L4Data\\Vile1.DUN",     NULL                           },
/*SPT_WARLORD*/      { "Levels\\L4Data\\Warlord.DUN",   "Levels\\L4Data\\Warlord2.DUN" },
/*SPT_DIAB_QUAD_1*/  { "Levels\\L4Data\\diab1.DUN",     NULL                           },
/*SPT_DIAB_QUAD_2*/  { "Levels\\L4Data\\diab2b.DUN",    "Levels\\L4Data\\diab2a.DUN"   },
/*SPT_DIAB_QUAD_3*/  { "Levels\\L4Data\\diab3b.DUN",    "Levels\\L4Data\\diab3a.DUN"   },
/*SPT_DIAB_QUAD_4*/  { "Levels\\L4Data\\diab4b.DUN",    "Levels\\L4Data\\diab4a.DUN"   },
/*SPT_LVL_SKELKING*/ { "Levels\\L1Data\\SklKng1.DUN",   "Levels\\L1Data\\SklKng2.DUN"  },
/*SPT_LVL_BCHAMB*/   { "Levels\\L2Data\\Bonecha2.DUN",  "Levels\\L2Data\\Bonecha1.DUN" },
/*SPT_LVL_PWATER*/   { "Levels\\L3Data\\Foulwatr.DUN",  NULL                           },
/*SPT_LVL_BETRAYER*/ { "Levels\\L1Data\\Vile1.DUN",     "Levels\\L1Data\\Vile2.DUN"    },
#ifdef HELLFIRE
/*SPT_NAKRUL*/       { "NLevels\\L5Data\\Nakrul2.DUN",  "NLevels\\L5Data\\Nakrul1.DUN" },
#endif
	// clang-format on
};

/** Contains the data related to quest gossip for each towner ID. */
const int Qtalklist[STORE_TOWNERS][NUM_QUESTS] = {
	// clang-format off
#ifdef HELLFIRE
	// Q_BUTCHER,  Q_PWATER,      Q_SKELKING,  Q_BANNER,     Q_GARBUD,  Q_ROCK,       Q_BLOOD,     Q_BCHAMB,   Q_BLIND,     Q_ZHAR,    Q_MUSHROOM,  Q_ANVIL,      Q_WARLORD,    Q_VEIL,     Q_BETRAYER,  Q_DIABLO,   Q_JERSEY,  Q_FARMER,  Q_GIRL,    Q_DEFILER,  Q_TRADER,   Q_GRAVE,     Q_NAKRUL
	{ TEXT_BUTCH5, TEXT_POISON7,  TEXT_KING7,  TEXT_BANNER6, TEXT_NONE,	TEXT_INFRA6,  TEXT_BLOOD5, TEXT_BONE5, TEXT_BLIND5, TEXT_NONE, TEXT_MUSH6,  TEXT_ANVIL6,  TEXT_WARLRD5, TEXT_VEIL5, TEXT_VILE9,  TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,TEXT_GRAVE2*/TEXT_NONE, TEXT_NONE },
	{ TEXT_BUTCH3, TEXT_POISON4,  TEXT_KING5,  TEXT_BANNER4, TEXT_NONE,	TEXT_INFRA3,  TEXT_BLOOD3, TEXT_BONE3, TEXT_BLIND3, TEXT_NONE, TEXT_NONE,   TEXT_ANVIL3,  TEXT_WARLRD3, TEXT_VEIL3, TEXT_VILE7,  TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,TEXT_GRAVE3*/TEXT_NONE, TEXT_NONE },
	{ TEXT_BUTCH2, TEXT_POISON2,  TEXT_KING3,  TEXT_NONE,    TEXT_NONE,	TEXT_INFRA2,  TEXT_BLOOD2, TEXT_BONE2, TEXT_BLIND2, TEXT_NONE, TEXT_MUSH2,  TEXT_ANVIL2,  TEXT_WARLRD2, TEXT_VEIL2, TEXT_VILE4,  TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,TEXT_GRAVE5*/TEXT_NONE, TEXT_NONE },
	{ TEXT_BUTCH1, TEXT_POISON1,  TEXT_KING1,  TEXT_BANNER1, TEXT_NONE,	TEXT_INFRA1,  TEXT_BLOOD1, TEXT_BONE1, TEXT_BLIND1, TEXT_NONE, TEXT_MUSH1,  TEXT_ANVIL1,  TEXT_WARLRD1, TEXT_VEIL1, TEXT_VILE2,  TEXT_VILE3, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,TEXT_GRAVE6*/TEXT_NONE, TEXT_NONE },
	{ TEXT_BUTCH6, TEXT_POISON8,  TEXT_KING8,  TEXT_BANNER7, TEXT_NONE,	TEXT_INFRA8,  TEXT_BLOOD6, TEXT_BONE6, TEXT_BLIND6, TEXT_NONE, TEXT_MUSH7,  TEXT_ANVIL8,  TEXT_WARLRD6, TEXT_VEIL6, TEXT_VILE10, TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,TEXT_GRAVE7*/TEXT_NONE, TEXT_NONE },
	{ TEXT_BUTCH7, TEXT_POISON9,  TEXT_KING9,  TEXT_BANNER8, TEXT_NONE,	TEXT_INFRA9,  TEXT_BLOOD7, TEXT_BONE7, TEXT_BLIND7, TEXT_NONE, TEXT_NONE,   TEXT_ANVIL9,  TEXT_WARLRD7, TEXT_VEIL7, TEXT_VILE11, TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,TEXT_GRAVE1*/TEXT_NONE, TEXT_NONE },
	{ TEXT_BUTCH4, TEXT_POISON6,  TEXT_KING6,  TEXT_BANNER5, TEXT_NONE,	TEXT_INFRA4,  TEXT_BLOOD4, TEXT_BONE4, TEXT_BLIND4, TEXT_NONE, TEXT_MUSH5,  TEXT_ANVIL4,  TEXT_WARLRD4, TEXT_VEIL4, TEXT_VILE8,  TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,*/TEXT_GRAVE8, TEXT_NONE },
	{ TEXT_BUTCH8, TEXT_POISON10, TEXT_KING10, TEXT_BANNER9, TEXT_NONE,	TEXT_INFRA10, TEXT_BLOOD8, TEXT_BONE8, TEXT_BLIND8, TEXT_NONE, TEXT_MUSH13, TEXT_ANVIL10, TEXT_WARLRD8, TEXT_VEIL8, TEXT_VILE12, TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,TEXT_GRAVE9*/TEXT_NONE, TEXT_NONE },
//	{ TEXT_NONE,   TEXT_NONE,     TEXT_NONE,   TEXT_NONE,    TEXT_NONE, TEXT_NONE,    TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_NONE, TEXT_NONE,   TEXT_NONE,    TEXT_NONE,    TEXT_NONE,  TEXT_NONE,   TEXT_NONE,  TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE,/*TEXT_NONE,*/TEXT_NONE,   TEXT_NONE },
#else
	// Q_BUTCHER,  Q_PWATER,      Q_SKELKING,  Q_BANNER,     Q_GARBUD,	Q_ROCK,       Q_BLOOD,     Q_BCHAMB,   Q_BLIND,     Q_ZHAR,    Q_MUSHROOM,  Q_ANVIL,      Q_WARLORD,    Q_VEIL,     Q_BETRAYER,  Q_DIABLO
	{ TEXT_BUTCH5, TEXT_POISON7,  TEXT_KING7,  TEXT_BANNER6, TEXT_NONE,	TEXT_INFRA6,  TEXT_BLOOD5, TEXT_BONE5, TEXT_BLIND5, TEXT_NONE, TEXT_MUSH6,  TEXT_ANVIL6,  TEXT_WARLRD5, TEXT_VEIL5, TEXT_VILE9,  TEXT_NONE  },
	{ TEXT_BUTCH3, TEXT_POISON4,  TEXT_KING5,  TEXT_BANNER4, TEXT_NONE,	TEXT_INFRA3,  TEXT_BLOOD3, TEXT_BONE3, TEXT_BLIND3, TEXT_NONE, TEXT_NONE,   TEXT_ANVIL3,  TEXT_WARLRD3, TEXT_VEIL3, TEXT_VILE7,  TEXT_NONE  },
	{ TEXT_BUTCH2, TEXT_POISON2,  TEXT_KING3,  TEXT_NONE,    TEXT_NONE,	TEXT_INFRA2,  TEXT_BLOOD2, TEXT_BONE2, TEXT_BLIND2, TEXT_NONE, TEXT_MUSH2,  TEXT_ANVIL2,  TEXT_WARLRD2, TEXT_VEIL2, TEXT_VILE4,  TEXT_NONE  },
	{ TEXT_BUTCH1, TEXT_POISON1,  TEXT_KING1,  TEXT_BANNER1, TEXT_NONE,	TEXT_INFRA1,  TEXT_BLOOD1, TEXT_BONE1, TEXT_BLIND1, TEXT_NONE, TEXT_MUSH1,  TEXT_ANVIL1,  TEXT_WARLRD1, TEXT_VEIL1, TEXT_VILE2,  TEXT_VILE3 },
	{ TEXT_BUTCH6, TEXT_POISON8,  TEXT_KING8,  TEXT_BANNER7, TEXT_NONE,	TEXT_INFRA8,  TEXT_BLOOD6, TEXT_BONE6, TEXT_BLIND6, TEXT_NONE, TEXT_MUSH7,  TEXT_ANVIL8,  TEXT_WARLRD6, TEXT_VEIL6, TEXT_VILE10, TEXT_NONE  },
	{ TEXT_BUTCH7, TEXT_POISON9,  TEXT_KING9,  TEXT_BANNER8, TEXT_NONE,	TEXT_INFRA9,  TEXT_BLOOD7, TEXT_BONE7, TEXT_BLIND7, TEXT_NONE, TEXT_MUSH9,  TEXT_ANVIL9,  TEXT_WARLRD7, TEXT_VEIL7, TEXT_VILE11, TEXT_NONE  },
	{ TEXT_BUTCH4, TEXT_POISON6,  TEXT_KING6,  TEXT_BANNER5, TEXT_NONE,	TEXT_INFRA4,  TEXT_BLOOD4, TEXT_BONE4, TEXT_BLIND4, TEXT_NONE, TEXT_MUSH5,  TEXT_ANVIL4,  TEXT_WARLRD4, TEXT_VEIL4, TEXT_VILE8,  TEXT_NONE  },
	{ TEXT_BUTCH8, TEXT_POISON10, TEXT_KING10, TEXT_BANNER9, TEXT_NONE,	TEXT_INFRA10, TEXT_BLOOD8, TEXT_BONE8, TEXT_BLIND8, TEXT_NONE, TEXT_MUSH13, TEXT_ANVIL10, TEXT_WARLRD8, TEXT_VEIL8, TEXT_VILE12, TEXT_NONE  },
//	{ TEXT_NONE,   TEXT_NONE,     TEXT_NONE,   TEXT_NONE,    TEXT_NONE, TEXT_NONE,    TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_NONE, TEXT_NONE,   TEXT_NONE,    TEXT_NONE,    TEXT_NONE,  TEXT_NONE,   TEXT_NONE  },
#endif
	// clang-format on
};

DEVILUTION_END_NAMESPACE
