/**
 * @file objdat.cpp
 *
 * Implementation of all object data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Maps from dun_object_id to object_id. */
const BYTE ObjConvTbl[128] = {
	0,
	OBJ_LEVER, // Q_SKELKING
	OBJ_CRUXM, // Q_SKELKING
	OBJ_CRUXR, // Q_SKELKING
	OBJ_CRUXL, // Q_SKELKING
	0, //OBJ_ANGEL,
	0, //OBJ_BANNERL,
	0, //OBJ_BANNERM,
	0, //OBJ_BANNERR,
	0,
	0,
	0,
	0,
	0,
	OBJ_BOOK2L, // Q_SCHAMB, Q_BETRAYER
	0, //OBJ_BOOK2R,
	OBJ_BCROSS, // Q_SCHAMB
	0,
	0, //OBJ_CANDLE1,
	OBJ_CANDLE2, // Q_SCHAMB
	0, //OBJ_CANDLEO,
	0, //OBJ_CAULDRON,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0, //OBJ_FLAMEHOLE,
	0,
	0,
	0,
	0,
	0,
	OBJ_MCIRCLE1, // Q_BETRAYER
	OBJ_MCIRCLE2, // Q_BETRAYER
	0, //OBJ_SKFIRE,
	0, //OBJ_SKPILE,
	0, //OBJ_SKSTICK1,
	0, //OBJ_SKSTICK2,
	0, //OBJ_SKSTICK3,
	0, //OBJ_SKSTICK4,
	0, //OBJ_SKSTICK5,
	0,
	0,
	0,
	0,
	0,
	0,
	OBJ_SWITCHSKL, // Q_SCHAMB
	0,
	0, //OBJ_TRAPL,
	0, //OBJ_TRAPR,
	OBJ_TORTURE1, // Q_BUTCHER
	OBJ_TORTURE2, // Q_BUTCHER
	OBJ_TORTURE3, // Q_BUTCHER
	OBJ_TORTURE4, // Q_BUTCHER
	OBJ_TORTURE5, // Q_BUTCHER
	0,
	0,
	0,
	0,
	0,
	0, //OBJ_NUDEW2R,
	0,
	0,
	0,
	0,
	OBJ_TNUDEM1, // Q_BUTCHER
	OBJ_TNUDEM2, // Q_BUTCHER
	OBJ_TNUDEM3, // Q_BUTCHER
	OBJ_TNUDEM4, // Q_BUTCHER
	OBJ_TNUDEW1, // Q_BUTCHER
	OBJ_TNUDEW2, // Q_BUTCHER
	OBJ_TNUDEW3, // Q_BUTCHER
	0, //OBJ_CHEST1,
	OBJ_CHEST1, // Q_SKELKING
	0, //OBJ_CHEST1,
	0, //OBJ_CHEST2,
	OBJ_CHEST2, // Q_SKELKING
	0, //OBJ_CHEST2,
	0, //OBJ_CHEST3,
	OBJ_CHEST3, // Q_SCHAMB
	0, //OBJ_CHEST3,
	0,
	0,
	0,
	0,
	0,
	0, //OBJ_PEDISTAL,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	OBJ_ALTBOY, // Q_BETRAYER
	0,
	0,
	OBJ_WARARMOR, // Q_WARLORD
	OBJ_WARWEAP, // Q_WARLORD
	OBJ_TORCHR2, // Q_BLOOD
	OBJ_TORCHL2, // Q_BLOOD
	0, //OBJ_MUSHPATCH,
	0, //OBJ_STAND,
	0, //OBJ_TORCHL,
	0, //OBJ_TORCHR,
	0, //OBJ_FLAMELVR,
	0, //OBJ_SARC,
	0, //OBJ_BARREL,
	0, //OBJ_BARRELEX,
	0, //OBJ_BOOKSHELF,
	0, //OBJ_BOOKCASEL,
	0, //OBJ_BOOKCASER,
	0, //OBJ_ARMORSTANDN,
	0, //OBJ_WEAPONRACKLN,
	0, //OBJ_BLOODFTN,
	0, //OBJ_PURIFYINGFTN,
	0, //OBJ_SHRINEL,
//	0, //OBJ_SHRINER,
//	0, //OBJ_GOATSHRINE,
//	0, //OBJ_MURKYFTN,
//	0, //OBJ_TEARFTN,
//	0, //OBJ_DECAP,
//	0, //OBJ_TCHEST1,
//	0, //OBJ_TCHEST2,
//	0, //OBJ_TCHEST3,
//	0, //OBJ_LAZSTAND,
//	0, //OBJ_BOOKSTAND,
//	0, //OBJ_BOOKSHELFR,
};

/** Contains the data related to each object ID. */
const ObjDataStruct AllObjects[NUM_OBJECTS] = {
	// clang-format off
					// oload, ofindex,        oLvlTypes,                                 oSetLvlType,     otheme,                  oquest,     oAnimFlag, oAnimBaseFrame, oAnimFrameLen, oAnimLen, oAnimWidth, oSolidFlag, oMissFlag, oLightFlag, oBreak, oSelFlag, oTrapFlag
/*OBJ_L1LIGHT*/		{      1, OFILE_L1BRAZ,   DTM_CATHEDRAL,                             DTYPE_CATHEDRAL, THEME_NONE,              Q_INVALID,       TRUE,              0,             1,       26,         64, TRUE,       TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_L1LDOOR*/		{      1, OFILE_L1DOORS,  DTM_CATHEDRAL,                             DTYPE_CATHEDRAL, THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_L1RDOOR*/		{      1, OFILE_L1DOORS,  DTM_CATHEDRAL,                             DTYPE_CATHEDRAL, THEME_NONE,              Q_INVALID,      FALSE,              2,             0,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_SKFIRE*/		{      3, OFILE_SKULFIRE, 0,                                         0,               THEME_SKELROOM,          Q_INVALID,       TRUE,              0,             2,       11,         96, TRUE,       TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_LEVER*/		{      1, OFILE_LEVER,    DTM_CATHEDRAL,                             0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CHEST1*/		{      1, OFILE_CHEST1,   DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CHEST2*/		{      1, OFILE_CHEST2,   DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CHEST3*/		{      1, OFILE_CHEST3,   DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CANDLE1*/		{      2, OFILE_L1BRAZ,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              0,             0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_CANDLE2*/		{      3, OFILE_CANDLE2,  0,                                         0,               THEME_SHRINE,            Q_INVALID,       TRUE,              0,             2,        4,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_CANDLEO*/		{      2, OFILE_L1BRAZ,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              0,             0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_BANNERL*/		{      3, OFILE_BANNER,   0,                                         0,               THEME_SKELROOM,          Q_INVALID,      FALSE,              2,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BANNERM*/		{      3, OFILE_BANNER,   0,                                         0,               THEME_SKELROOM,          Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BANNERR*/		{      3, OFILE_BANNER,   0,                                         0,               THEME_SKELROOM,          Q_INVALID,      FALSE,              3,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_SKPILE*/		{      2, OFILE_SKULPILE, 0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_SKSTICK1*/	{      2, OFILE_L1BRAZ,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              0,             0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK2*/	{      2, OFILE_L1BRAZ,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              0,             0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK3*/	{      2, OFILE_L1BRAZ,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              0,             0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK4*/	{      2, OFILE_L1BRAZ,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              0,             0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK5*/	{      2, OFILE_L1BRAZ,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              0,             0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_CRUXM*/		{      2, OFILE_CRUXSK1,  0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             1,       15,         96, TRUE,       FALSE,     TRUE,            1,        3, FALSE     },
/*OBJ_CRUXR*/		{      2, OFILE_CRUXSK2,  0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             1,       15,         96, TRUE,       FALSE,     TRUE,            1,        3, FALSE     },
/*OBJ_CRUXL*/		{      2, OFILE_CRUXSK3,  0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             1,       15,         96, TRUE,       FALSE,     TRUE,            1,        3, FALSE     },
/*OBJ_STAND*/		{      2, OFILE_ROCKSTAN, 0,                                         0,               THEME_NONE,              Q_ROCK,         FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_ANGEL*/		{      2, OFILE_ANGEL,    0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_BCROSS*/		{      2, OFILE_BURNCROS, 0,                                         0,               THEME_NONE,              Q_INVALID,       TRUE,              0,             0,       10,        160, TRUE,       FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_NUDEW2R*/		{      2, OFILE_NUDE2,    0,                                         0,               THEME_NONE,              Q_INVALID,       TRUE,              0,             3,        6,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_SWITCHSKL*/	{      1, OFILE_SWITCH4,  DTM_HELL,                                  0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_TNUDEM1*/		{      1, OFILE_TNUDEM,   DTM_HELL,                                  0,               THEME_NONE,              Q_BUTCHER,      FALSE,              1,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEM2*/		{      1, OFILE_TNUDEM,   DTM_HELL,                                  0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,              1,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEM3*/		{      1, OFILE_TNUDEM,   DTM_HELL,                                  0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,              3,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEM4*/		{      1, OFILE_TNUDEM,   DTM_HELL,                                  0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,              4,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEW1*/		{      1, OFILE_TNUDEW,   DTM_HELL,                                  0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,              1,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEW2*/		{      1, OFILE_TNUDEW,   DTM_HELL,                                  0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,              2,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEW3*/		{      1, OFILE_TNUDEW,   DTM_HELL,                                  0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,              3,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE1*/	{      1, OFILE_TSOUL,    DTM_HELL,                                  0,               THEME_NONE,              Q_BUTCHER,      FALSE,              1,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE2*/	{      1, OFILE_TSOUL,    DTM_HELL,                                  0,               THEME_NONE,              Q_BUTCHER,      FALSE,              2,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE3*/	{      1, OFILE_TSOUL,    DTM_HELL,                                  0,               THEME_NONE,              Q_BUTCHER,      FALSE,              3,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE4*/	{      1, OFILE_TSOUL,    DTM_HELL,                                  0,               THEME_NONE,              Q_BUTCHER,      FALSE,              4,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE5*/	{      1, OFILE_TSOUL,    DTM_HELL,                                  0,               THEME_NONE,              Q_BUTCHER,      FALSE,              5,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_L2LDOOR*/		{      1, OFILE_L2DOORS,  DTM_CATACOMBS,                             DTYPE_CATACOMBS, THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_L2RDOOR*/		{      1, OFILE_L2DOORS,  DTM_CATACOMBS,                             DTYPE_CATACOMBS, THEME_NONE,              Q_INVALID,      FALSE,              2,             0,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_TORCHL*/		{      1, OFILE_WTORCH4,  DTM_CATACOMBS,                             0,               THEME_NONE,              Q_INVALID,       TRUE,              0,             1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_TORCHR*/		{      1, OFILE_WTORCH3,  DTM_CATACOMBS,                             0,               THEME_NONE,              Q_INVALID,       TRUE,              0,             1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_TORCHL2*/		{      1, OFILE_WTORCH1,  DTM_CATACOMBS,                             0,               THEME_NONE,              Q_BLOOD,         TRUE,              0,             1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_TORCHR2*/		{      1, OFILE_WTORCH2,  DTM_CATACOMBS,                             0,               THEME_NONE,              Q_BLOOD,         TRUE,              0,             1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_SARC*/		{      1, OFILE_SARC,     DTM_CATHEDRAL,                             0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             3,        5,        128, TRUE,       TRUE,      TRUE,            0,        3, TRUE      },
/*OBJ_FLAMEHOLE*/	{      2, OFILE_FLAME1,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             1,       20,         96, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_FLAMELVR*/	{      2, OFILE_LEVER,    0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        2,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_WATER*/		{      2, OFILE_MINIWATR, 0,                                         0,               THEME_NONE,              Q_INVALID,       TRUE,              0,             1,       10,         64, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TRAPL*/		{      1, OFILE_TRAPHOLE, DTM_CATHEDRAL | DTM_CATACOMBS,             0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         64, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_TRAPR*/		{      1, OFILE_TRAPHOLE, DTM_CATHEDRAL | DTM_CATACOMBS,             0,               THEME_NONE,              Q_INVALID,      FALSE,              2,             0,        0,         64, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BOOKSHELF*/	{      2, OFILE_BCASE,    0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_WEAPRACK*/	{      2, OFILE_WEAPSTND, 0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_BARREL*/		{      1, OFILE_BARREL,   DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             1,        9,         96, TRUE,       TRUE,      TRUE,            1,        3, FALSE     },
/*OBJ_BARRELEX*/	{      1, OFILE_BARRELEX, DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             1,       10,         96, TRUE,       TRUE,      TRUE,            1,        3, FALSE     },
/*OBJ_SHRINEL*/		{      3, OFILE_LSHRINEG, 0,                                         0,               THEME_SHRINE,            Q_INVALID,      FALSE,              1,             1,       11,        128, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_SHRINER*/		{      3, OFILE_RSHRINEG, 0,                                         0,               THEME_SHRINE,            Q_INVALID,      FALSE,              1,             1,       11,        128, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_BOOKCASEL*/	{      3, OFILE_BCASE,    0,                                         0,               THEME_LIBRARY,           Q_INVALID,      FALSE,              3,             0,        0,         96, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_BOOKCASER*/	{      3, OFILE_BCASE,    0,                                         0,               THEME_LIBRARY,           Q_INVALID,      FALSE,              4,             0,        0,         96, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_BOOKCANDLE*/	{      3, OFILE_CANDLE2,  0,                                         0,               THEME_LIBRARY,           Q_INVALID,       TRUE,              0,             2,        4,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BLOODFTN*/	{      3, OFILE_BLOODFNT, 0,                                         0,               THEME_BLOODFOUNTAIN,     Q_INVALID,       TRUE,              0,             2,       10,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_DECAP*/		{      1, OFILE_DECAP,    DTM_HELL,                                  0,               THEME_DECAPITATED,       Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_TCHEST1*/		{      1, OFILE_CHEST1,   DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_TCHEST2*/		{      1, OFILE_CHEST2,   DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_TCHEST3*/		{      1, OFILE_CHEST3,   DTM_ANY,                                   0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_BOOK2L*/		{      2, OFILE_BOOK2,    0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BOOK2R*/		{      2, OFILE_BOOK2,    0,                                         0,               THEME_NONE,              Q_SCHAMB,       FALSE,              4,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BOOKSTAND*/	{      3, OFILE_BOOK2,    0,                                         0,               THEME_LIBRARY,           Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_SKELBOOK*/	{      3, OFILE_BOOK2,    0,                                         0,               THEME_SKELROOM,          Q_INVALID,      FALSE,              4,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BOOKLVR*/		{      2, OFILE_BOOK1,    0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BLINDBOOK*/	{      2, OFILE_BOOK1,    0,                                         0,               THEME_NONE,              Q_BLIND,        FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BLOODBOOK*/	{      2, OFILE_BOOK1,    0,                                         0,               THEME_NONE,              Q_BLOOD,        FALSE,              4,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_STEELTOME*/	{      2, OFILE_BOOK1,    0,                                         0,               THEME_NONE,              Q_WARLORD,      FALSE,              4,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_PEDISTAL*/	{      2, OFILE_PEDISTL,  0,                                         0,               THEME_NONE,              Q_BLOOD,        FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_L3LDOOR*/		{      1, OFILE_L3DOORS,  DTM_CAVES,                                 0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_L3RDOOR*/		{      1, OFILE_L3DOORS,  DTM_CAVES,                                 0,               THEME_NONE,              Q_INVALID,      FALSE,              2,             0,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_PURIFYINGFTN*/{      3, OFILE_PFOUNTN,  0,                                         0,               THEME_PURIFYINGFOUNTAIN, Q_INVALID,       TRUE,              0,             2,       10,        128, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_ARMORSTAND*/	{      3, OFILE_ARMSTAND, 0,                                         0,               THEME_ARMORSTAND,        Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_ARMORSTANDN*/	{      3, OFILE_ARMSTAND, 0,                                         0,               THEME_ARMORSTAND,        Q_INVALID,      FALSE,              2,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_GOATSHRINE*/	{      3, OFILE_GOATSHRN, 0,                                         0,               THEME_GOATSHRINE,        Q_INVALID,       TRUE,              0,             2,       10,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_CAULDRON*/	{      1, OFILE_CAULDREN, DTM_HELL,                                  0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_MURKYFTN*/	{      3, OFILE_MFOUNTN,  0,                                         0,               THEME_MURKYFOUNTAIN,     Q_INVALID,       TRUE,              0,             2,       10,        128, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_TEARFTN*/		{      3, OFILE_TFOUNTN,  0,                                         0,               THEME_TEARFOUNTAIN,      Q_INVALID,       TRUE,              0,             2,        4,        128, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_ALTBOY*/		{      2, OFILE_ALTBOY,   0,                                         0,               THEME_NONE,              Q_BETRAYER,     FALSE,              1,             0,        0,        128, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_MCIRCLE1*/	{      2, OFILE_MCIRL,    0,                                         0,               THEME_NONE,              Q_BETRAYER,     FALSE,              1,             0,        0,         96, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_MCIRCLE2*/	{      2, OFILE_MCIRL,    0,                                         0,               THEME_NONE,              Q_BETRAYER,     FALSE,              1,             0,        0,         96, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_STORYBOOK*/	{      1, OFILE_BKSLBRNT, DTM_CATHEDRAL | DTM_CATACOMBS | DTM_CAVES, 0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     }, // BUGFIX should only be loaded on level 1-12 (crypt masks as 1-4) (fixed)
/*OBJ_STORYCANDLE*/	{      1, OFILE_CANDLE2,  DTM_CATHEDRAL | DTM_CATACOMBS | DTM_CAVES, 0,               THEME_NONE,              Q_BETRAYER,      TRUE,              0,             2,        4,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_WARARMOR*/	{      2, OFILE_ARMSTAND, 0,                                         0,               THEME_NONE,              Q_WARLORD,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_WARWEAP*/		{      2, OFILE_WEAPSTND, 0,                                         0,               THEME_NONE,              Q_WARLORD,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_TBCROSS*/		{      2, OFILE_BURNCROS, 0,                                         0,               THEME_BRNCROSS,          Q_INVALID,       TRUE,              0,             1,       10,        160, TRUE,       FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_WEAPONRACKL*/	{      2, OFILE_WEAPSTND, 0,                                         0,               THEME_WEAPONRACK,        Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_WEAPONRACKLN*/{      2, OFILE_WEAPSTND, 0,                                         0,               THEME_WEAPONRACK,        Q_INVALID,      FALSE,              2,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_WEAPONRACKR*/	{      2, OFILE_WEAPSTND, 0,                                         0,               THEME_WEAPONRACK,        Q_INVALID,      FALSE,              3,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_WEAPONRACKRN*/{      2, OFILE_WEAPSTND, 0,                                         0,               THEME_WEAPONRACK,        Q_INVALID,      FALSE,              4,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_MUSHPATCH*/	{      2, OFILE_MUSHPTCH, 0,                                         0,               THEME_NONE,              Q_MUSHROOM,     FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_LAZSTAND*/	{      2, OFILE_LZSTAND,  0,                                         0,               THEME_NONE,              Q_BETRAYER,     FALSE,              1,             0,        0,        128, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_SLAINHERO*/	{      1, OFILE_DECAP,    DTM_CAVES,                                 0,               THEME_NONE,              Q_INVALID,      FALSE,              2,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_SIGNCHEST*/	{      2, OFILE_CHEST3,   0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              1,             0,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_BOOKSHELFR*/	{      2, OFILE_BCASE,    0,                                         0,               THEME_NONE,              Q_INVALID,      FALSE,              2,             0,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
	// clang-format on
};

/** Maps from object_graphic_id to object CEL name. */
const char *const ObjMasterLoadList[NUM_OFILE_TYPES] = {
	"L1Braz",
	"L1Doors",
	"Lever",
	"Chest1",
	"Chest2",
	"Banner",
	"SkulPile",
	"SkulFire",
	"SkulStik",
	"CruxSk1",
	"CruxSk2",
	"CruxSk3",
	"Book1",
	"Book2",
	"Rockstan",
	"Angel",
	"Chest3",
	"Burncros",
	"Candle2",
	"Nude2",
	"Switch4",
	"TNudeM",
	"TNudeW",
	"TSoul",
	"L2Doors",
	"WTorch4",
	"WTorch3",
	"Sarc",
	"Flame1",
	"Prsrplt1",
	"Traphole",
	"MiniWatr",
	"WTorch2",
	"WTorch1",
	"BCase",
	"BShelf",
	"WeapStnd",
	"Barrel",
	"Barrelex",
	"LShrineG",
	"RShrineG",
	"Bloodfnt",
	"Decap",
	"Pedistl",
	"L3Doors",
	"PFountn",
	"Armstand",
	"Goatshrn",
	"Cauldren",
	"MFountn",
	"TFountn",
	"Altboy",
	"Mcirl",
	"Bkslbrnt",
	"Mushptch",
	"LzStand"
};
#ifdef HELLFIRE
/** Maps from object_graphic_id to object CEL name (Hellfire Crypt overwrite). */
const char *const ObjCryptLoadList[NUM_OFILE_TYPES] = {
	"L1Braz",
	"L5Door",
	"L5Lever",
	"Chest1",
	"Chest2",
	"Banner",
	"SkulPile",
	"SkulFire",
	"SkulStik",
	"CruxSk1",
	"CruxSk2",
	"CruxSk3",
	"Book1",
	"Book2",
	"Rockstan",
	"Angel",
	"Chest3",
	"Burncros",
	"L5Light",
	"Nude2",
	"Switch4",
	"TNudeM",
	"TNudeW",
	"TSoul",
	"L2Doors",
	"WTorch4",
	"WTorch3",
	"L5Sarco",
	"Flame1",
	"Prsrplt1",
	"Traphole",
	"MiniWatr",
	"WTorch2",
	"WTorch1",
	"BCase",
	"BShelf",
	"WeapStnd",
	"Urn",
	"Urnexpld",
	"LShrineG",
	"RShrineG",
	"Bloodfnt",
	"Decap",
	"Pedistl",
	"L3Doors",
	"PFountn",
	"Armstand",
	"Goatshrn",
	"Cauldren",
	"MFountn",
	"TFountn",
	"Altboy",
	"Mcirl",
	"L5Books",
	"Mushptch",
	"LzStand",
};
/** Maps from object_graphic_id to object CEL name (Hellfire Nest overwrite). */
const char *const ObjNestLoadList[NUM_OFILE_TYPES] = {
	"L1Braz",
	"L1Doors",
	"Lever",
	"Chest1",
	"Chest2",
	"Banner",
	"SkulPile",
	"SkulFire",
	"SkulStik",
	"CruxSk1",
	"CruxSk2",
	"CruxSk3",
	"Book1",
	"Book2",
	"Rockstan",
	"Angel",
	"Chest3",
	"Burncros",
	"Candle2",
	"Nude2",
	"Switch4",
	"TNudeM",
	"TNudeW",
	"TSoul",
	"L2Doors",
	"WTorch4",
	"WTorch3",
	"Sarc",
	"Flame1",
	"Prsrplt1",
	"Traphole",
	"MiniWatr",
	"WTorch2",
	"WTorch1",
	"BCase",
	"BShelf",
	"WeapStnd",
	"L6Pod1",
	"L6Pod2",
	"LShrineG",
	"RShrineG",
	"Bloodfnt",
	"Decap",
	"Pedistl",
	"L3Doors",
	"PFountn",
	"Armstand",
	"Goatshrn",
	"Cauldren",
	"MFountn",
	"TFountn",
	"Altboy",
	"Mcirl",
	"Bkslbrnt",
	"Mushptch",
	"LzStand",
};
#endif

DEVILUTION_END_NAMESPACE
