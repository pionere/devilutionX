/**
 * @file objdat.cpp
 *
 * Implementation of all object data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Maps from dun_object_id to object_id. */
const int ObjTypeConv[] = {
	0,
	OBJ_LEVER,
	OBJ_CRUX1,
	OBJ_CRUX2,
	OBJ_CRUX3,
	OBJ_ANGEL,
	OBJ_BANNERL,
	OBJ_BANNERM,
	OBJ_BANNERR,
	0,
	0,
	0,
	0,
	0,
	OBJ_BOOK2L,
	OBJ_BOOK2R,
	OBJ_BCROSS,
	0,
	OBJ_CANDLE1,
	OBJ_CANDLE2,
	OBJ_CANDLEO,
	OBJ_CAULDRON,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	OBJ_FLAMEHOLE,
	0,
	0,
	0,
	0,
	0,
	OBJ_MCIRCLE1,
	OBJ_MCIRCLE2,
	OBJ_SKFIRE,
	OBJ_SKPILE,
	OBJ_SKSTICK1,
	OBJ_SKSTICK2,
	OBJ_SKSTICK3,
	OBJ_SKSTICK4,
	OBJ_SKSTICK5,
	0,
	0,
	0,
	0,
	0,
	0,
	OBJ_SWITCHSKL,
	0,
	OBJ_TRAPL,
	OBJ_TRAPR,
	OBJ_TORTURE1,
	OBJ_TORTURE2,
	OBJ_TORTURE3,
	OBJ_TORTURE4,
	OBJ_TORTURE5,
	0,
	0,
	0,
	0,
	0,
	OBJ_NUDEW2R,
	0,
	0,
	0,
	0,
	OBJ_TNUDEM1,
	OBJ_TNUDEM2,
	OBJ_TNUDEM3,
	OBJ_TNUDEM4,
	OBJ_TNUDEW1,
	OBJ_TNUDEW2,
	OBJ_TNUDEW3,
	OBJ_CHEST1,
	OBJ_CHEST1,
	OBJ_CHEST1,
	OBJ_CHEST2,
	OBJ_CHEST2,
	OBJ_CHEST2,
	OBJ_CHEST3,
	OBJ_CHEST3,
	OBJ_CHEST3,
	0,
	0,
	0,
	0,
	0,
	OBJ_PEDISTAL,
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
	OBJ_ALTBOY,
	0,
	0,
	OBJ_WARARMOR,
	OBJ_WARWEAP,
	OBJ_TORCHR2,
	OBJ_TORCHL2,
	OBJ_MUSHPATCH,
	OBJ_STAND,
	OBJ_TORCHL,
	OBJ_TORCHR,
	OBJ_FLAMELVR,
	OBJ_SARC,
	OBJ_BARREL,
	OBJ_BARRELEX,
	OBJ_BOOKSHELF,
	OBJ_BOOKCASEL,
	OBJ_BOOKCASER,
	OBJ_ARMORSTANDN,
	OBJ_WEAPONRACKLN,
	OBJ_BLOODFTN,
	OBJ_PURIFYINGFTN,
	OBJ_SHRINEL,
	OBJ_SHRINER,
	OBJ_GOATSHRINE,
	OBJ_MURKYFTN,
	OBJ_TEARFTN,
	OBJ_DECAP,
	OBJ_TCHEST1,
	OBJ_TCHEST2,
	OBJ_TCHEST3,
	OBJ_LAZSTAND,
	OBJ_BOOKSTAND,
	OBJ_BOOKSHELFR,
};

/** Contains the data related to each object ID. */
const ObjDataStruct AllObjects[NUM_OBJECTS] = {
	// clang-format off
					// oload, ofindex,   ominlvl, omaxlvl, olvltype,        otheme,                  oquest,     oAnimFlag, oAnimDelay, oAnimLen, oAnimWidth, oSolidFlag, oMissFlag, oLightFlag, oBreak, oSelFlag, oTrapFlag
/*OBJ_L1LIGHT*/		{      1, OFILE_L1BRAZ,    1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,              TRUE,          1,       26,         64, TRUE,       TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_L1LDOOR*/		{      1, OFILE_L1DOORS,   1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,             FALSE,          1,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_L1RDOOR*/		{      1, OFILE_L1DOORS,   1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,             FALSE,          2,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_SKFIRE*/		{      3, OFILE_SKULFIRE,  0,       0, 0,               THEME_SKELROOM,          -1,              TRUE,          2,       11,         96, TRUE,       TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_LEVER*/		{      1, OFILE_LEVER,     1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,             FALSE,          1,        1,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CHEST1*/		{      1, OFILE_CHEST1,    1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CHEST2*/		{      1, OFILE_CHEST2,    1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CHEST3*/		{      1, OFILE_CHEST3,    1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_CANDLE1*/		{      2, OFILE_L1BRAZ,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_CANDLE2*/		{      3, OFILE_CANDLE2,   0,       0, 0,               THEME_SHRINE,            -1,              TRUE,          2,        4,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_CANDLEO*/		{      2, OFILE_L1BRAZ,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_BANNERL*/		{      3, OFILE_BANNER,    0,       0, 0,               THEME_SKELROOM,          -1,             FALSE,          2,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BANNERM*/		{      3, OFILE_BANNER,    0,       0, 0,               THEME_SKELROOM,          -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BANNERR*/		{      3, OFILE_BANNER,    0,       0, 0,               THEME_SKELROOM,          -1,             FALSE,          3,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_SKPILE*/		{      2, OFILE_SKULPILE,  1,       4, 0,               THEME_NONE,              -1,             FALSE,          1,        1,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_SKSTICK1*/	{      2, OFILE_L1BRAZ,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK2*/	{      2, OFILE_L1BRAZ,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK3*/	{      2, OFILE_L1BRAZ,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK4*/	{      2, OFILE_L1BRAZ,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_SKSTICK5*/	{      2, OFILE_L1BRAZ,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          0,        0,          0, FALSE,      FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_CRUX1*/		{      2, OFILE_CRUXSK1,   0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,       15,         96, TRUE,       FALSE,     TRUE,            1,        3, FALSE     },
/*OBJ_CRUX2*/		{      2, OFILE_CRUXSK2,   0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,       15,         96, TRUE,       FALSE,     TRUE,            1,        3, FALSE     },
/*OBJ_CRUX3*/		{      2, OFILE_CRUXSK3,   0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,       15,         96, TRUE,       FALSE,     TRUE,            1,        3, FALSE     },
/*OBJ_STAND*/		{      1, OFILE_ROCKSTAN,  5,       5, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_ANGEL*/		{      2, OFILE_ANGEL,     0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_BOOK2L*/		{      2, OFILE_BOOK2,     0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BCROSS*/		{      2, OFILE_BURNCROS,  0,       0, 0,               THEME_NONE,              -1,              TRUE,          0,       10,        160, TRUE,       FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_NUDEW2R*/		{      2, OFILE_NUDE2,     0,       0, 0,               THEME_NONE,              -1,              TRUE,          3,        6,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_SWITCHSKL*/	{      1, OFILE_SWITCH4,  16,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, TRUE      },
/*OBJ_TNUDEM1*/		{      1, OFILE_TNUDEM,   13,      16, 0,               THEME_NONE,              Q_BUTCHER,      FALSE,          1,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEM2*/		{      1, OFILE_TNUDEM,   13,      16, 0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,          2,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEM3*/		{      1, OFILE_TNUDEM,   13,      16, 0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,          3,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEM4*/		{      1, OFILE_TNUDEM,   13,      16, 0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,          4,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEW1*/		{      1, OFILE_TNUDEW,   13,      16, 0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,          1,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEW2*/		{      1, OFILE_TNUDEW,   13,      16, 0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,          2,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TNUDEW3*/		{      1, OFILE_TNUDEW,   13,      16, 0,               THEME_TORTURE,           Q_BUTCHER,      FALSE,          3,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE1*/	{      1, OFILE_TSOUL,    13,      16, 0,               THEME_NONE,              Q_BUTCHER,      FALSE,          1,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE2*/	{      1, OFILE_TSOUL,    13,      16, 0,               THEME_NONE,              Q_BUTCHER,      FALSE,          2,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE3*/	{      1, OFILE_TSOUL,    13,      16, 0,               THEME_NONE,              Q_BUTCHER,      FALSE,          3,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE4*/	{      1, OFILE_TSOUL,    13,      16, 0,               THEME_NONE,              Q_BUTCHER,      FALSE,          4,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_TORTURE5*/	{      1, OFILE_TSOUL,    13,      16, 0,               THEME_NONE,              Q_BUTCHER,      FALSE,          5,        0,        128, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_BOOK2R*/		{      1, OFILE_BOOK2,     6,       6, 0,               THEME_NONE,              -1,             FALSE,          4,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_L2LDOOR*/		{      1, OFILE_L2DOORS,   5,       8, DTYPE_CATACOMBS, THEME_NONE,              -1,             FALSE,          1,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_L2RDOOR*/		{      1, OFILE_L2DOORS,   5,       8, DTYPE_CATACOMBS, THEME_NONE,              -1,             FALSE,          2,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_TORCHL*/		{      1, OFILE_WTORCH4,   5,       8, DTYPE_CATACOMBS, THEME_NONE,              -1,              TRUE,          1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_TORCHR*/		{      1, OFILE_WTORCH3,   5,       8, DTYPE_CATACOMBS, THEME_NONE,              -1,              TRUE,          1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_TORCHL2*/		{      1, OFILE_WTORCH1,   5,       8, DTYPE_CATACOMBS, THEME_NONE,              -1,              TRUE,          1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_TORCHR2*/		{      1, OFILE_WTORCH2,   5,       8, DTYPE_CATACOMBS, THEME_NONE,              -1,              TRUE,          1,        9,         96, FALSE,      TRUE,      FALSE,           0,        0, FALSE     },
/*OBJ_SARC*/		{      1, OFILE_SARC,      1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,             FALSE,          1,        5,        128, TRUE,       TRUE,      TRUE,            0,        3, TRUE      },
/*OBJ_FLAMEHOLE*/	{      2, OFILE_FLAME1,    1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,             FALSE,          1,       20,         96, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_FLAMELVR*/	{      2, OFILE_LEVER,     1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,             FALSE,          1,        2,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_WATER*/		{      2, OFILE_MINIWATR,  1,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,              TRUE,          1,       10,         64, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_BOOKLVR*/		{      1, OFILE_BOOK1,     3,       4, DTYPE_CATHEDRAL, THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_TRAPL*/		{      1, OFILE_TRAPHOLE,  1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         64, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_TRAPR*/		{      1, OFILE_TRAPHOLE,  1,      16, 0,               THEME_NONE,              -1,             FALSE,          2,        0,         64, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BOOKSHELF*/	{      2, OFILE_BCASE,     0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_WEAPRACK*/	{      2, OFILE_WEAPSTND,  0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_BARREL*/		{      1, OFILE_BARREL,    1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        9,         96, TRUE,       TRUE,      TRUE,            1,        3, FALSE     },
/*OBJ_BARRELEX*/	{      1, OFILE_BARRELEX,  1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,       10,         96, TRUE,       TRUE,      TRUE,            1,        3, FALSE     },
/*OBJ_SHRINEL*/		{      3, OFILE_LSHRINEG,  0,       0, 0,               THEME_SHRINE,            -1,             FALSE,          1,       11,        128, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_SHRINER*/		{      3, OFILE_RSHRINEG,  0,       0, 0,               THEME_SHRINE,            -1,             FALSE,          1,       11,        128, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_SKELBOOK*/	{      3, OFILE_BOOK2,     0,       0, 0,               THEME_SKELROOM,          -1,             FALSE,          4,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BOOKCASEL*/	{      3, OFILE_BCASE,     0,       0, 0,               THEME_LIBRARY,           -1,             FALSE,          3,        0,         96, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_BOOKCASER*/	{      3, OFILE_BCASE,     0,       0, 0,               THEME_LIBRARY,           -1,             FALSE,          4,        0,         96, FALSE,      FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_BOOKSTAND*/	{      3, OFILE_BOOK2,     0,       0, 0,               THEME_LIBRARY,           -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BOOKCANDLE*/	{      3, OFILE_CANDLE2,   0,       0, 0,               THEME_LIBRARY,           -1,              TRUE,          2,        4,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_BLOODFTN*/	{      3, OFILE_BLOODFNT,  0,       0, 0,               THEME_BLOODFOUNTAIN,     -1,              TRUE,          2,       10,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_DECAP*/		{      1, OFILE_DECAP,    13,      16, 0,               THEME_DECAPITATED,       -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_TCHEST1*/		{      1, OFILE_CHEST1,    1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_TCHEST2*/		{      1, OFILE_CHEST2,    1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_TCHEST3*/		{      1, OFILE_CHEST3,    1,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_BLINDBOOK*/	{      1, OFILE_BOOK1,     7,       7, DTYPE_CATACOMBS, THEME_NONE,              Q_BLIND,        FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_BLOODBOOK*/	{      1, OFILE_BOOK1,     5,       5, DTYPE_CATACOMBS, THEME_NONE,              Q_BLOOD,        FALSE,          4,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_PEDISTAL*/	{      1, OFILE_PEDISTL,   5,       5, DTYPE_CATACOMBS, THEME_NONE,              Q_BLOOD,        FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_L3LDOOR*/		{      1, OFILE_L3DOORS,   9,      12, DTYPE_CAVES,     THEME_NONE,              -1,             FALSE,          1,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_L3RDOOR*/		{      1, OFILE_L3DOORS,   9,      12, DTYPE_CAVES,     THEME_NONE,              -1,             FALSE,          2,        0,         64, FALSE,      FALSE,     TRUE,            0,        3, TRUE      },
/*OBJ_PURIFYINGFTN*/{      3, OFILE_PFOUNTN,   0,       0, 0,               THEME_PURIFYINGFOUNTAIN, -1,              TRUE,          2,       10,        128, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_ARMORSTAND*/	{      3, OFILE_ARMSTAND,  0,       0, 0,               THEME_ARMORSTAND,        -1,             FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_ARMORSTANDN*/	{      3, OFILE_ARMSTAND,  0,       0, 0,               THEME_ARMORSTAND,        -1,             FALSE,          2,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_GOATSHRINE*/	{      3, OFILE_GOATSHRN,  0,       0, 0,               THEME_GOATSHRINE,        -1,              TRUE,          2,       10,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_CAULDRON*/	{      1, OFILE_CAULDREN, 13,      16, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_MURKYFTN*/	{      3, OFILE_MFOUNTN,   0,       0, 0,               THEME_MURKYFOUNTAIN,     -1,              TRUE,          2,       10,        128, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_TEARFTN*/		{      3, OFILE_TFOUNTN,   0,       0, 0,               THEME_TEARFOUNTAIN,      -1,              TRUE,          2,        4,        128, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_ALTBOY*/		{      1, OFILE_ALTBOY,    0,       0, DTYPE_CATHEDRAL, THEME_NONE,              Q_BETRAYER,     FALSE,          1,        0,        128, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_MCIRCLE1*/	{      1, OFILE_MCIRL,     0,       0, DTYPE_CATHEDRAL, THEME_NONE,              Q_BETRAYER,     FALSE,          1,        0,         96, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_MCIRCLE2*/	{      1, OFILE_MCIRL,     0,       0, DTYPE_CATHEDRAL, THEME_NONE,              Q_BETRAYER,     FALSE,          1,        0,         96, FALSE,      TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_STORYBOOK*/	{      1, OFILE_BKSLBRNT,  1,      12, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     }, // BUGFIX should only be loaded on level 1-12 (crypt masks as 1-4) (fixed)
/*OBJ_STORYCANDLE*/	{      1, OFILE_CANDLE2,   1,      12, 0,               THEME_NONE,              Q_BETRAYER,      TRUE,          2,        4,         96, TRUE,       TRUE,      TRUE,            0,        0, FALSE     },
/*OBJ_STEELTOME*/	{      1, OFILE_BOOK1,    13,      13, DTYPE_HELL,      THEME_NONE,              Q_WARLORD,      FALSE,          4,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_WARARMOR*/	{      1, OFILE_ARMSTAND, 13,      13, 0,               THEME_NONE,              Q_WARLORD,      FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_WARWEAP*/		{      2, OFILE_WEAPSTND, 13,      13, 0,               THEME_NONE,              Q_WARLORD,      FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_TBCROSS*/		{      2, OFILE_BURNCROS,  0,       0, 0,               THEME_BRNCROSS,          -1,              TRUE,          0,       10,        160, TRUE,       FALSE,     FALSE,           0,        0, FALSE     },
/*OBJ_WEAPONRACKL*/	{      2, OFILE_WEAPSTND,  0,       0, 0,               THEME_WEAPONRACK,        -1,             FALSE,          1,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_WEAPONRACKLN*/{      2, OFILE_WEAPSTND,  0,       0, 0,               THEME_WEAPONRACK,        -1,             FALSE,          2,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_WEAPONRACKR*/	{      2, OFILE_WEAPSTND,  0,       0, 0,               THEME_WEAPONRACK,        -1,             FALSE,          3,        0,         96, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_WEAPONRACKRN*/{      2, OFILE_WEAPSTND,  0,       0, 0,               THEME_WEAPONRACK,        -1,             FALSE,          4,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
/*OBJ_MUSHPATCH*/	{      2, OFILE_MUSHPTCH,  0,       0, 0,               THEME_NONE,              Q_MUSHROOM,     FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        3, FALSE     },
/*OBJ_LAZSTAND*/	{      2, OFILE_LZSTAND,   0,       0, 0,               THEME_NONE,              Q_BETRAYER,     FALSE,          1,        0,        128, TRUE,       FALSE,     TRUE,            0,        3, FALSE     },
/*OBJ_SLAINHERO*/	{      1, OFILE_DECAP,     9,       9, DTYPE_CAVES,     THEME_NONE,              -1,             FALSE,          2,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_SIGNCHEST*/	{      2, OFILE_CHEST3,    0,       0, 0,               THEME_NONE,              -1,             FALSE,          1,        0,         96, TRUE,       TRUE,      TRUE,            0,        1, FALSE     },
/*OBJ_BOOKSHELFR*/	{      2, OFILE_BCASE,     0,       0, 0,               THEME_NONE,              -1,             FALSE,          2,        0,         96, TRUE,       FALSE,     TRUE,            0,        0, FALSE     },
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
/** Maps from object_graphic_id to object CEL name (Hellfire Hive overwrite). */
const char *const ObjHiveLoadList[NUM_OFILE_TYPES] = {
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
