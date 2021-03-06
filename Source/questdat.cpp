/**
 * @file questdat.cpp
 *
 * Implementation of all quest and level data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

const LevelData AllLevels[NUM_LEVELS] = {
	// clang-format off
					// dLevel, dSetLvl, dType,           dDunType,        dMusic,      dMicroTileLen, dBlocks, dLevelName,
/*DLV_TOWN*/		{       0, FALSE,   DTYPE_TOWN,      DTYPE_TOWN,      TMUSIC_TOWN,            16,      16, "Tristram",
					//  dAutomapData,             dSolidTable,                  dDunCels,                      dMegaTiles,                    dLvlPieces,                    dSpecCels,                     dPalName,                        dLoadCels,              dLoadPal,                dSetLvlPreDun,                 dSetLvlDun,                        dSetLvlDunX,   dSetLvlDunY,
#ifdef HELLFIRE
						NULL,                     "Levels\\TownData\\Town.SOL", "NLevels\\TownData\\Town.CEL", "NLevels\\TownData\\Town.TIL", "NLevels\\TownData\\Town.MIN", "Levels\\TownData\\TownS.CEL", "Levels\\TownData\\Town.pal",    "Gendata\\Cuttt.CEL",   "Gendata\\Cuttt.pal",    NULL,                          NULL,                                        0,             0,
#else
						NULL,                     "Levels\\TownData\\Town.SOL", "Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "Levels\\TownData\\Town.pal",    "Gendata\\Cuttt.CEL",   "Gendata\\Cuttt.pal",    NULL,                          NULL,                                        0,             0,
#endif
					//  dMonTypes,
						{ MT_INVALID },
					},
/*DLV_CATHEDRAL1*/	{       2, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 1",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
						{ MT_NZOMBIE, MT_RFALLSP, MT_WSKELAX, MT_RFALLSD, MT_NSCAV, MT_WSKELSD, MT_INVALID },
					},
/*DLV_CATHEDRAL2*/	{       4, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 2",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
						{ MT_NZOMBIE, MT_RFALLSP, MT_WSKELAX, MT_RFALLSD, MT_NSCAV, MT_WSKELSD, MT_BZOMBIE, MT_GZOMBIE,  MT_DFALLSP, MT_YFALLSP, MT_TSKELAX, MT_RSKELAX, MT_DFALLSD, MT_YFALLSD, MT_WSKELBW, MT_TSKELBW, MT_BSCAV, MT_TSKELSD, MT_SNEAK, MT_FIEND, MT_INVALID },
					},
/*DLV_CATHEDRAL3*/	{       6, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 3",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
						{ MT_NSCAV, MT_WSKELSD, MT_BZOMBIE, MT_GZOMBIE, MT_DFALLSP, MT_YFALLSP, MT_TSKELAX, MT_RSKELAX, MT_DFALLSD, MT_YFALLSD, MT_WSKELBW, MT_TSKELBW, MT_BSCAV, MT_TSKELSD, MT_SNEAK, MT_FIEND, MT_BLINK, MT_YZOMBIE, MT_BFALLSP, MT_XSKELAX, MT_BFALLSD, MT_WSCAV, MT_RSKELBW, MT_RSKELSD, MT_INVALID },
					},
/*DLV_CATHEDRAL4*/	{       8, FALSE,   DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Cathedral 4",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_%i.PAL",     "Gendata\\Cutl1d.CEL",  "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
						{ MT_GZOMBIE, MT_YFALLSP, MT_RSKELAX, MT_YFALLSD, MT_TSKELBW, MT_BSCAV, MT_TSKELSD, MT_SNEAK, MT_BLINK, MT_YZOMBIE, MT_BFALLSP, MT_XSKELAX, MT_BFALLSD, MT_WSCAV, MT_RSKELBW, MT_RSKELSD, MT_GLOOM, MT_YSCAV, MT_XSKELBW, MT_XSKELSD, MT_NGOATMC, MT_NGOATBW, MT_INVALID },
					},
/*DLV_CATACOMBS1*/	{      10, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 1",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_SNEAK, MT_BLINK, MT_YZOMBIE, MT_BFALLSP, MT_XSKELAX, MT_BFALLSD, MT_WSCAV, MT_RSKELBW, MT_RSKELSD, MT_GLOOM, MT_YSCAV, MT_XSKELBW, MT_XSKELSD, MT_NGOATMC, MT_NGOATBW, MT_BGOATBW, MT_BGOATMC, MT_STALKER, MT_FAT, MT_WINGED, MT_INVALID },
					},
/*DLV_CATACOMBS2*/	{      12, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 2",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_GLOOM, MT_YSCAV, MT_XSKELBW, MT_XSKELSD, MT_NGOATMC, MT_NGOATBW, MT_BGOATBW, MT_BGOATMC, MT_STALKER, MT_FAT, MT_WINGED, MT_UNSEEN, MT_RGOATMC, MT_FAMILIAR, MT_RGOATBW, MT_NACID, MT_INVALID },
					},
/*DLV_CATACOMBS3*/	{      14, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 3",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_BGOATBW, MT_BGOATMC, MT_STALKER, MT_FAT, MT_WINGED, MT_UNSEEN, MT_RGOATMC, MT_FAMILIAR, MT_RGOATBW, MT_NACID, MT_GGOATBW, MT_GGOATMC, MT_MUDMAN, MT_HORNED, MT_GARGOYLE, MT_INVALID },
					},
/*DLV_CATACOMBS4*/	{      16, FALSE,   DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Catacombs 4",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_%i.PAL",     "Gendata\\Cut2.CEL",    "Gendata\\Cut2.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_UNSEEN, MT_RGOATMC, MT_FAMILIAR, MT_RGOATBW, MT_NACID, MT_GGOATBW, MT_GGOATMC, MT_MUDMAN, MT_HORNED, MT_GARGOYLE, MT_MUDRUN, MT_ILLWEAV, MT_RACID, MT_TOAD, MT_NMAGMA, MT_YMAGMA, MT_INVALID },
					},
/*DLV_CAVES1*/		{      18, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 1",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_GGOATBW, MT_GGOATMC, MT_MUDMAN, MT_HORNED, MT_GARGOYLE, MT_MUDRUN, MT_ILLWEAV, MT_RACID, MT_TOAD, MT_NMAGMA, MT_YMAGMA, MT_BLOODCLW, MT_BMAGMA, MT_WMAGMA, MT_FROSTC, MT_STORM, MT_INVALID },
					},
/*DLV_CAVES2*/		{      20, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 2",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_MUDRUN, MT_ILLWEAV, MT_RACID, MT_TOAD, MT_YMAGMA, MT_BLOODCLW, MT_BMAGMA, MT_WMAGMA, MT_FROSTC, MT_STORM, MT_OBLORD, MT_BACID, MT_FLAYED, MT_RSTORM, MT_DEATHW, MT_MEGA, MT_INVALID },
					},
/*DLV_CAVES3*/		{      22, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 3",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_BLOODCLW, MT_BMAGMA, MT_WMAGMA, MT_FROSTC, MT_STORM, MT_OBLORD, MT_BACID, MT_FLAYED, MT_RSTORM, MT_DEATHW, MT_MEGA, MT_GUARD, MT_NSNAKE, MT_STORML, MT_INVALID },
					},
/*DLV_CAVES4*/		{      24, FALSE,   DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Caves 4",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3_%i.PAL",     "Gendata\\Cut3.CEL",    "Gendata\\Cut3.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_OBLORD, MT_BACID, MT_FLAYED, MT_RSTORM, MT_DEATHW, MT_MEGA, MT_GUARD, MT_NSNAKE, MT_STORML, MT_RSNAKE, MT_NBLACK, MT_SUCCUBUS, MT_VTEXLRD, MT_XACID, MT_MAEL, MT_INVALID },
					},
/*DLV_HELL1*/		{      26, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Hell 1",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_GUARD, MT_NSNAKE, MT_STORML, MT_RSNAKE, MT_NBLACK, MT_SUCCUBUS, MT_VTEXLRD, MT_XACID, MT_MAEL, MT_BALROG, MT_BSNAKE, MT_RTBLACK, MT_RBLACK, MT_SNOWWICH, MT_COUNSLR, MT_INVALID },
					},
/*DLV_HELL2*/		{      28, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Hell 2",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_RSNAKE, MT_NBLACK, MT_SUCCUBUS, MT_VTEXLRD, MT_XACID, MT_MAEL, MT_BALROG, MT_BSNAKE, MT_RTBLACK, MT_RBLACK, MT_SNOWWICH, MT_COUNSLR, MT_MAGISTR, MT_BTBLACK, MT_HLSPWN, MT_INVALID },
					},
/*DLV_HELL3*/		{      30, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Hell 3",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cut4.CEL",    "Gendata\\Cut4.pal",     NULL,                          NULL,                                        0,             0,
						{ MT_BALROG, MT_RTBLACK, MT_SNOWWICH, MT_MAGISTR, MT_BTBLACK, MT_HLSPWN, MT_GSNAKE, MT_SOLBRNR, MT_CABALIST, MT_INVALID },
					},
/*DLV_HELL4*/		{      32, FALSE,   DTYPE_HELL,      DTYPE_HELL,      TMUSIC_L4,              12,      16, "Diablo",
						"Levels\\L4Data\\L4.AMP", "Levels\\L4Data\\L4.SOL",     "Levels\\L4Data\\L4.CEL",      "Levels\\L4Data\\L4.TIL",      "Levels\\L4Data\\L4.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L4Data\\L4_%i.PAL",     "Gendata\\Cutgate.CEL", "Gendata\\Cutgate.pal",  NULL,                          NULL,                                        0,             0,
						{ MT_INVALID }, /* MT_RBLACK, MT_ADVOCATE */
					},
#ifdef HELLFIRE
/*DLV_NEST1*/		{      18, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 1",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_HELLBOAR, MT_STINGER, MT_PSYCHORB, MT_ARACHNON, MT_FELLTWIN, MT_UNRAV, MT_INVALID },
					},
/*DLV_NEST2*/		{      20, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 2",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_HELLBOAR, MT_STINGER, MT_PSYCHORB, MT_ARACHNON, MT_FELLTWIN, MT_UNRAV, MT_INVALID }, /* MT_HORKSPWN */
					},
/*DLV_NEST3*/		{      22, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 3",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_VENMTAIL, MT_NECRMORB, MT_SPIDLORD, MT_LASHWORM, MT_TORCHANT, MT_INVALID }, /* MT_HORKSPWN */
					},
/*DLV_NEST4*/		{      24, FALSE,   DTYPE_NEST,      DTYPE_CAVES,     TMUSIC_L6,              10,      10, "Nest 4",
						"NLevels\\L6Data\\L6.AMP", "NLevels\\L6Data\\L6.SOL",   "NLevels\\L6Data\\L6.CEL",     "NLevels\\L6Data\\L6.TIL",     "NLevels\\L6Data\\L6.MIN",     "Levels\\L1Data\\L1S.CEL",     "NLevels\\L6Data\\L6Base%i.PAL", "Nlevels\\Cutl6.CEL",   "Nlevels\\Cutl6.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_VENMTAIL, MT_NECRMORB, MT_SPIDLORD, MT_LASHWORM, MT_TORCHANT, MT_INVALID },
					},
/*DLV_CRYPT1*/		{      26, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 1",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_LRDSAYTR, MT_GRAVEDIG, MT_BIGFALL, MT_TOMBRAT, MT_FIREBAT, MT_LICH, MT_INVALID },
					},
/*DLV_CRYPT2*/		{      28, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 2",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_BIGFALL, MT_TOMBRAT, MT_FIREBAT, MT_SKLWING, MT_LICH, MT_CRYPTDMN, MT_INVALID },
					},
/*DLV_CRYPT3*/		{      30, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 3",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_CRYPTDMN, MT_HELLBAT, MT_BONEDEMN, MT_ARCHLICH, MT_BICLOPS, MT_FLESTHNG, MT_REAPER, MT_INVALID },
					},
/*DLV_CRYPT4*/		{      32, FALSE,   DTYPE_CRYPT,     DTYPE_CATHEDRAL, TMUSIC_L5,              10,      10, "Crypt 4",
						"NLevels\\L5Data\\L5.AMP", "NLevels\\L5Data\\L5.SOL",   "NLevels\\L5Data\\L5.CEL",     "NLevels\\L5Data\\L5.TIL",     "NLevels\\L5Data\\L5.MIN",     "NLevels\\L5Data\\L5S.CEL",    "NLevels\\L5Data\\L5Base.PAL",   "Nlevels\\Cutl5.CEL",   "Nlevels\\Cutl5.pal",    NULL,                          NULL,                                        0,             0,
						{ MT_HELLBAT, MT_BICLOPS, MT_FLESTHNG, MT_REAPER, MT_INVALID }, /* MT_ARCHLICH */
					},
#endif
					// dLevel, dSetLvl, dType,           dDunType,        dMusic,      dMicroTileLen, dBlocks, dLevelName,
/*DLV_BUTCHCHAMB*/	{       0, TRUE,    DTYPE_NONE,      DTYPE_NONE,      TMUSIC_TOWN,            10,      10, "",
					//  dAutomapData,             dSolidTable,                  dDunCels,                      dMegaTiles,                    dLvlPieces,                    dSpecCels,                     dPalName,                        dLoadCels,               dLoadPal,                dSetLvlPreDun,                 dSetLvlDun,                        dSetLvlDunX,   dSetLvlDunY,
						NULL,                     NULL,                         "Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "",                              "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   NULL,                          NULL,                                        0,             0,
					//  dMonTypes,
						{ MT_INVALID },
					},
/*DLV_SKELKING*/	{       6, TRUE,    DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Skeleton King's Lair",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_2.pal",      "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\SklKng1.DUN", "Levels\\L1Data\\SklKng2.DUN",   DBORDERX + 67, DBORDERY + 29,
						{ MT_INVALID },
					},
/*DLV_BONECHAMB*/	{      12, TRUE,    DTYPE_CATACOMBS, DTYPE_CATACOMBS, TMUSIC_L2,              10,      10, "Chamber of Bone",
						"Levels\\L2Data\\L2.AMP", "Levels\\L2Data\\L2.SOL",     "Levels\\L2Data\\L2.CEL",      "Levels\\L2Data\\L2.TIL",      "Levels\\L2Data\\L2.MIN",      "Levels\\L2Data\\L2S.CEL",     "Levels\\L2Data\\L2_2.pal",      "Gendata\\Cut2.CEL",     "Gendata\\Cut2.pal",     "Levels\\L2Data\\Bonecha2.DUN", "Levels\\L2Data\\Bonecha1.DUN", DBORDERX + 53, DBORDERY + 23,
						{ MT_INVALID },
					},
/*DLV_MAZE*/		{       0, TRUE,    DTYPE_NONE,      DTYPE_NONE,      TMUSIC_TOWN,            10,      10, "Maze",
						NULL,                     NULL,                         "Levels\\TownData\\Town.CEL",  "Levels\\TownData\\Town.TIL",  "Levels\\TownData\\Town.MIN",  "Levels\\TownData\\TownS.CEL", "Levels\\L1Data\\L1_5.pal",      "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   "Levels\\L1Data\\Lv1MazeA.DUN", "Levels\\L1Data\\Lv1MazeB.DUN", DBORDERX +  4, DBORDERY + 34,
						{ MT_INVALID },
					},
/*DLV_POISONWATER*/	{       4, TRUE,    DTYPE_CAVES,     DTYPE_CAVES,     TMUSIC_L3,              10,      10, "Poisoned Water Supply",
						"Levels\\L3Data\\L3.AMP", "Levels\\L3Data\\L3.SOL",     "Levels\\L3Data\\L3.CEL",      "Levels\\L3Data\\L3.TIL",      "Levels\\L3Data\\L3.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L3Data\\L3pfoul.pal",   "Gendata\\Cutl1d.CEL",   "Gendata\\Cutl1d.pal",   "Levels\\L3Data\\Foulwatr.DUN", "Levels\\L3Data\\Foulwatr.DUN", DBORDERX + 15, DBORDERY + 67,
						{ MT_INVALID },
					},
/*DLV_VILEBETRAYER*/{      30, TRUE,    DTYPE_CATHEDRAL, DTYPE_CATHEDRAL, TMUSIC_L1,              10,      10, "Archbishop Lazarus' Lair",
						"Levels\\L1Data\\L1.AMP", "Levels\\L1Data\\L1.SOL",     "Levels\\L1Data\\L1.CEL",      "Levels\\L1Data\\L1.TIL",      "Levels\\L1Data\\L1.MIN",      "Levels\\L1Data\\L1S.CEL",     "Levels\\L1Data\\L1_2.pal",      "Gendata\\Cutportr.CEL", "Gendata\\Cutportr.pal", "Levels\\L1Data\\Vile1.DUN",    "Levels\\L1Data\\Vile2.DUN",    DBORDERX + 19, DBORDERY + 20,
						{ MT_INVALID },
					},
	// clang-format on
};

/** Contains the data related to each quest_id. */
const QuestData questlist[NUM_QUESTS] = {
	// clang-format off
	          // _qdlvl,       _qslvl,           _qdmsg,        _qlstr
/*Q_ROCK*/    { DLV_CATACOMBS1, 0,               TEXT_INFRA5,   "The Magic Rock"           },
/*Q_MUSHROOM*/{ DLV_CAVES1,     0,               TEXT_MUSH8,    "Black Mushroom"           },
/*Q_GARBUD*/  { DLV_CATHEDRAL4, 0,               TEXT_GARBUD1,  "Gharbad The Weak"         },
/*Q_ZHAR*/    { DLV_CATACOMBS4, 0,               TEXT_ZHAR1,    "Zhar the Mad"             },
/*Q_VEIL*/    { DLV_HELL2,      0,               TEXT_VEIL9,    "Lachdanan"                },
/*Q_DIABLO*/  { DLV_HELL3,      0,               TEXT_VILE3,    "Diablo"                   },
/*Q_BUTCHER*/ { DLV_CATHEDRAL2, 0,               TEXT_BUTCH9,   "The Butcher"              },
/*Q_LTBANNER*/{ DLV_CATHEDRAL4, 0,               TEXT_BANNER2,  "Ogden's Sign"             },
/*Q_BLIND*/   { DLV_CATACOMBS3, 0,               TEXT_BLINDING, "Halls of the Blind"       },
/*Q_BLOOD*/   { DLV_CATACOMBS1, 0,               TEXT_BLOODY,   "Valor"                    },
/*Q_ANVIL*/   { DLV_CAVES2,     0,               TEXT_ANVIL5,   "Anvil of Fury"            },
/*Q_WARLORD*/ { DLV_HELL1,      0,               TEXT_BLOODWAR, "Warlord of Blood"         },
/*Q_SKELKING*/{ DLV_CATHEDRAL3, SL_SKELKING,     TEXT_KING2,    "The Curse of King Leoric" },
/*Q_PWATER*/  { DLV_CATHEDRAL2, SL_POISONWATER,  TEXT_POISON3,  "Poisoned Water Supply"    },
/*Q_SCHAMB*/  { DLV_CATACOMBS2, SL_BONECHAMB,    TEXT_BONER,    "The Chamber of Bone"      },
/*Q_BETRAYER*/{ DLV_HELL3,      SL_VILEBETRAYER, TEXT_VILE1,    "Archbishop Lazarus"       },
#ifdef HELLFIRE
/*Q_GRAVE*/   { DLV_NEST1,      0,               TEXT_GRAVE7,   "Grave Matters"            },
/*Q_FARMER*/  { DLV_CAVES1,     0,               TEXT_FARMER1,  "Farmer's Orchard"         },
/*Q_GIRL*/    { DLV_NEST1,      0,               TEXT_GIRL2,    "Little Girl"              },
/*Q_TRADER*/  { DLV_NEST3,      0,               TEXT_TRADER,   "Wandering Trader"         },
/*Q_DEFILER*/ { DLV_NEST1,      0,               TEXT_DM_NEST,  "The Defiler"              },
/*Q_NAKRUL*/  { DLV_CRYPT4,     0,               TEXT_NAKRUL1,  "Na-Krul"                  },
/*Q_JERSEY*/  { DLV_CAVES1,     0,               TEXT_JERSEY4,  "The Jersey's Jersey"      },
#endif
	// clang-format on
};

/** Contains the data related to quest gossip for each towner ID. */
const int Qtalklist[STORE_TOWNERS][NUM_QUESTS] = {
	// clang-format off
#ifdef HELLFIRE
	// Q_ROCK,      Q_MUSHROOM,  Q_GARBUD,   Q_ZHAR,      Q_VEIL,     Q_DIABLO,    Q_BUTCHER,   Q_LTBANNER,   Q_BLIND,     Q_BLOOD,     Q_ANVIL,      Q_WARLORD,    Q_SKELKING,  Q_PWATER,      Q_SCHAMB,   Q_BETRAYER,  Q_GRAVE,     Q_FARMER,  Q_GIRL,    Q_TRADER,  Q_DEFILER, Q_NAKRUL,  Q_JERSEY,
	{ TEXT_INFRA6,  TEXT_MUSH6,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL5, TEXT_NONE,   TEXT_BUTCH5, TEXT_BANNER6, TEXT_BLIND5, TEXT_BLOOD5, TEXT_ANVIL6,  TEXT_WARLRD5, TEXT_KING7,  TEXT_POISON7,  TEXT_BONE5, TEXT_VILE9,  TEXT_GRAVE2, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA3,  TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_VEIL3, TEXT_NONE,   TEXT_BUTCH3, TEXT_BANNER4, TEXT_BLIND3, TEXT_BLOOD3, TEXT_ANVIL3,  TEXT_WARLRD3, TEXT_KING5,  TEXT_POISON4,  TEXT_BONE3, TEXT_VILE7,  TEXT_GRAVE3, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA2,  TEXT_MUSH2,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL2, TEXT_NONE,   TEXT_BUTCH2, TEXT_NONE,    TEXT_BLIND2, TEXT_BLOOD2, TEXT_ANVIL2,  TEXT_WARLRD2, TEXT_KING3,  TEXT_POISON2,  TEXT_BONE2, TEXT_VILE4,  TEXT_GRAVE5, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA1,  TEXT_MUSH1,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL1, TEXT_VILE3,  TEXT_BUTCH1, TEXT_BANNER1, TEXT_BLIND1, TEXT_BLOOD1, TEXT_ANVIL1,  TEXT_WARLRD1, TEXT_KING1,  TEXT_POISON1,  TEXT_BONE1, TEXT_VILE2,  TEXT_GRAVE6, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA8,  TEXT_MUSH7,  TEXT_NONE,  TEXT_NONE,   TEXT_VEIL6, TEXT_NONE,   TEXT_BUTCH6, TEXT_BANNER7, TEXT_BLIND6, TEXT_BLOOD6, TEXT_ANVIL8,  TEXT_WARLRD6, TEXT_KING8,  TEXT_POISON8,  TEXT_BONE6, TEXT_VILE10, TEXT_GRAVE7, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
	{ TEXT_INFRA9,  TEXT_NONE,   TEXT_NONE,  TEXT_NONE,   TEXT_VEIL7, TEXT_NONE,   TEXT_BUTCH7, TEXT_BANNER8, TEXT_BLIND7, TEXT_BLOOD7, TEXT_ANVIL9,  TEXT_WARLRD7, TEXT_KING9,  TEXT_POISON9,  TEXT_BONE7, TEXT_VILE11, TEXT_GRAVE1, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE, TEXT_NONE },
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
