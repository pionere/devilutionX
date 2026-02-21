/**
 * @file objdat.cpp
 *
 * Implementation of all object data.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifndef HELLFIRE
#define OBJ_NAKRULBOOK 0
#define OBJ_NAKRULLEVER 0
#endif

/** Maps from dun_object_id to object_id. */
const BYTE ObjConvTbl[128] = {
	// clang-format off
	0,
	OBJ_LEVER, // SklKng2.DUN
	OBJ_CRUXM, // SklKng2.DUN
	OBJ_CRUXR, // SklKng2.DUN
	OBJ_CRUXL, // SklKng2.DUN
	OBJ_SARC,  // SklKng2.DUN
	0, //OBJ_BANNERL,
	0, //OBJ_BANNERM,
	0, //OBJ_BANNERR,
	0,
	0,
	0,
	0,
	0,
	OBJ_ANCIENTBOOK, // Bonecha1.DUN
	OBJ_BLOODBOOK, // Blood2.DUN (Q_BLOOD)
	OBJ_TBCROSS, // Bonecha1.DUN
	0,
	0, //OBJ_CANDLE1,
	OBJ_CANDLE2, // Bonecha1.DUN, Viles.DUN (Q_BETRAYER)
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
	OBJ_MCIRCLE1, // Vile2.DUN
	OBJ_MCIRCLE2, // Vile2.DUN
	0, //OBJ_SKFIRE,
	0, //OBJ_SKPILE,
	0, //OBJ_SKSTICK1,
	0, //OBJ_SKSTICK2,
	0, //OBJ_SKSTICK3,
	0, //OBJ_SKSTICK4,
	0, //OBJ_SKSTICK5,
	0,
	0,
	OBJ_VILEBOOK, // Vile2.DUN
	0,
	0,
	0,
	OBJ_SWITCHSKL, // Bonecha1.DUN, diab1.DUN, diab2a.DUN, diab3a.DUN
	0,
	0, //OBJ_TRAPL,
	0, //OBJ_TRAPR,
	OBJ_TORTUREL1, // Butcher.DUN (Q_BUTCHER)
	OBJ_TORTUREL2, // Butcher.DUN (Q_BUTCHER)
	OBJ_TORTURER1, // Butcher.DUN (Q_BUTCHER)
	OBJ_TORTURER2, // Butcher.DUN (Q_BUTCHER)
	OBJ_TORTUREL3, // Butcher.DUN (Q_BUTCHER)
	0,
	0,
	0,
	0,
	0,
	0, //OBJ_NUDEW2R,
	0,
	0,
	0,
	OBJ_LAZSTAND, // Viles.DUN (Q_BETRAYER)
	OBJ_TNUDEM, // Butcher.DUN (Q_BUTCHER)
	0,
	0,
	0,
	OBJ_TNUDEW, // Butcher.DUN (Q_BUTCHER)
	0,
	0,
	0, //OBJ_CHEST1,
	OBJ_CHEST1, // SklKng2.DUN
	0, //OBJ_CHEST1,
	0, //OBJ_CHEST2,
	OBJ_CHEST2, // SklKng2.DUN
	0, //OBJ_CHEST2,
	0, //OBJ_CHEST3,
	OBJ_CHEST3, // Bonecha1.DUN
	OBJ_NAKRULBOOK, // Nakrul1.DUN (Q_NAKRUL)
	OBJ_NAKRULLEVER, // Nakrul1.DUN (Q_NAKRUL)
	0,
	0,
	0,
	OBJ_SIGNCHEST, // Banner2.DUN (Q_BANNER)
	OBJ_PEDESTAL, // Blood2.DUN (Q_BLOOD)
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
	OBJ_ALTBOY, // L4Data/Vile1.DUN (Q_BETRAYER), L1Data/Vile2.DUN
	0,
	0,
	OBJ_ARMORSTANDN, //OBJ_ARMORSTAND, // Warlord2.DUN (Q_WARLORD) - changed to inactive versions to eliminate farming potential
	OBJ_WEAPONRACKLN, //OBJ_WEAPONRACKL, // Warlord2.DUN (Q_WARLORD)
	0, //OBJ_TORCHR1 (should be OBJ_TORCHL2), // Blood2.DUN (Q_BLOOD)
	0, //OBJ_TORCHL1, // Blood2.DUN (Q_BLOOD)
	0, //OBJ_MUSHPATCH,
	0, //OBJ_STAND,
	0, //OBJ_TORCHL2,
	0, //OBJ_TORCHR2,
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
//	0, //OBJ_BOOK2L,
//	0, //OBJ_BOOKSHELFR,
	// clang-format on
};

/** Contains the data related to each object ID. */
const ObjectData objectdata[NUM_OBJECTS] = {
	// clang-format off
//                     ofindex,        oLvlTypes,                                 otheme,                  oquest,     oBaseFrame, oLightRadius, oProc,      oModeFlags,             oMissFlag, oDoorFlag, oSelFlag, oPreFlag, oTrapFlag, 
/*OBJ_L1LIGHT*/      { OFILE_L1BRAZ,   DTM_CATHEDRAL,                             THEME_NONE,              Q_INVALID,           0,           14, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT if FLICKER_LIGHT
/*OBJ_L1LDOOR*/      { OFILE_L1DOORS,  DTM_CATHEDRAL,                             THEME_NONE,              Q_INVALID,           1,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_LEFT,         3, FALSE,    TRUE,      ALIGN },
/*OBJ_L1RDOOR*/      { OFILE_L1DOORS,  DTM_CATHEDRAL,                             THEME_NONE,              Q_INVALID,           2,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_RIGHT,        3, FALSE,    TRUE,      ALIGN },
/*OBJ_SKFIRE*/       { OFILE_SKULFIRE, 0,                                         THEME_SKELROOM,          Q_INVALID,           0,            8, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_LEVER*/        { OFILE_LEVER,    0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_CHEST1*/       { OFILE_CHEST1,   DTM_ANY,                                   THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    TRUE,      ALIGN },
/*OBJ_CHEST2*/       { OFILE_CHEST2,   DTM_ANY,                                   THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    TRUE,      ALIGN },
/*OBJ_CHEST3*/       { OFILE_CHEST3,   DTM_ANY,                                   THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    TRUE,      ALIGN },
/*OBJ_CANDLE1*///    { OFILE_CANDLE,   0,                                         THEME_NONE,              Q_INVALID,           0,            7, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_CANDLE2*/      { OFILE_CANDLE2,  0,                                         THEME_SHRINE,            Q_INVALID,           0,            7, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_CANDLEO*///    { OFILE_CANDLABR, 0,                                         THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_BANNERL*/      { OFILE_BANNER,   0,                                         THEME_SKELROOM,          Q_INVALID,           2,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_BANNERM*/      { OFILE_BANNER,   0,                                         THEME_SKELROOM,          Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_BANNERR*/      { OFILE_BANNER,   0,                                         THEME_SKELROOM,          Q_INVALID,           3,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_SKPILE*///     { OFILE_SKULPILE, 0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_SKSTICK1*///   { OFILE_SKULSTIK, 0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_SKSTICK2*///   { OFILE_SKULSTIK, 0,                                         THEME_NONE,              Q_INVALID,           2,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_SKSTICK3*///   { OFILE_SKULSTIK, 0,                                         THEME_NONE,              Q_INVALID,           3,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_SKSTICK4*///   { OFILE_SKULSTIK, 0,                                         THEME_NONE,              Q_INVALID,           4,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_SKSTICK5*///   { OFILE_SKULSTIK, 0,                                         THEME_NONE,              Q_INVALID,           5,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_CRUXM*/        { OFILE_CRUXSK1,  0,                                         THEME_NONE,              Q_INVALID,          16,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_CRUXR*/        { OFILE_CRUXSK2,  0,                                         THEME_NONE,              Q_INVALID,          16,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_CRUXL*/        { OFILE_CRUXSK3,  0,                                         THEME_NONE,              Q_INVALID,          16,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_STAND*/        { OFILE_ROCKSTAN, 0,                                         THEME_NONE,              Q_ROCK,              1,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, TRUE,     FALSE,     ALIGN },
/*OBJ_ANGEL*///      { OFILE_ANGEL,    0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_NUDEW2R*///    { OFILE_NUDE2,    0,                                         THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_SWITCHSKL*/    { OFILE_SWITCH4,  DTM_HELL,                                  THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    TRUE,      ALIGN },
/*OBJ_TNUDEM*/       { OFILE_TNUDEM,   DTM_HELL,                                  THEME_TORTURE,           Q_BUTCHER,           0,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, TRUE,     FALSE,     ALIGN },
/*OBJ_TNUDEW*/       { OFILE_TNUDEW,   DTM_HELL,                                  THEME_TORTURE,           Q_BUTCHER,           0,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, TRUE,     FALSE,     ALIGN },
/*OBJ_TORTUREL1*/    { OFILE_TSOUL,    DTM_HELL,                                  THEME_NONE,              Q_BUTCHER,           1,            0, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_TORTUREL2*/    { OFILE_TSOUL,    DTM_HELL,                                  THEME_NONE,              Q_BUTCHER,           2,            0, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_TORTUREL3*/    { OFILE_TSOUL,    DTM_HELL,                                  THEME_NONE,              Q_BUTCHER,           5,            0, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_TORTURER1*/    { OFILE_TSOUL,    DTM_HELL,                                  THEME_NONE,              Q_BUTCHER,           3,            0, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_TORTURER2*/    { OFILE_TSOUL,    DTM_HELL,                                  THEME_NONE,              Q_BUTCHER,           4,            0, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_TORTURER3*/    { OFILE_TSOUL,    DTM_HELL,                                  THEME_NONE,              Q_BUTCHER,           6,            0, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_L2LDOOR*/      { OFILE_L2DOORS,  DTM_CATACOMBS,                             THEME_NONE,              Q_INVALID,           1,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_LEFT,         3, FALSE,    TRUE,      ALIGN },
/*OBJ_L2RDOOR*/      { OFILE_L2DOORS,  DTM_CATACOMBS,                             THEME_NONE,              Q_INVALID,           2,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_RIGHT,        3, FALSE,    TRUE,      ALIGN },
/*OBJ_TORCHL1*/      { OFILE_WTORCH1,  DTM_CATACOMBS,                             THEME_NONE,              Q_BLOOD,             0,  OLF_XO | 12, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_TORCHL2*/      { OFILE_WTORCH4,  DTM_CATACOMBS,                             THEME_NONE,              Q_INVALID,           0,           12, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_TORCHR1*/      { OFILE_WTORCH2,  DTM_CATACOMBS,                             THEME_NONE,              Q_BLOOD,             0,  OLF_YO | 12, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_TORCHR2*/      { OFILE_WTORCH3,  DTM_CATACOMBS,                             THEME_NONE,              Q_INVALID,           0,           12, OPF_NONE,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_SARC*/         { OFILE_SARC,     DTM_CATHEDRAL,                             THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    TRUE,      ALIGN },
/*OBJ_FLAMEHOLE*///  { OFILE_FLAME1,   0,                                         THEME_NONE,              Q_INVALID,           0,            0, OPF_FLTRP,  OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_FLAMELVR*///   { OFILE_LEVER,    0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_WATER*///      { OFILE_MINIWATR, 0,                                         THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_TRAPL*/        { OFILE_TRAPHOLE, DTM_CATHEDRAL | DTM_CATACOMBS | DTM_CRYPT, THEME_NONE,              Q_INVALID,           1,            0, OPF_TRAP,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_TRAPR*/        { OFILE_TRAPHOLE, DTM_CATHEDRAL | DTM_CATACOMBS | DTM_CRYPT, THEME_NONE,              Q_INVALID,           2,            0, OPF_TRAP,   OMF_NONE,               TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_BOOKSHELF*///  { OFILE_BCASE,    0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_WEAPRACK*///   { OFILE_WEAPSTND, 0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_BARREL*/       { OFILE_BARREL,   DTM_ANY & ~(DTM_CRYPT | DTM_NEST),         THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_BARRELEX*/     { OFILE_BARRELEX, DTM_ANY & ~(DTM_CRYPT | DTM_NEST),         THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_SHRINEL*/      { OFILE_LSHRINEG, 0,                                         THEME_SHRINE,            Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, TRUE,     FALSE,     ALIGN },
/*OBJ_SHRINER*/      { OFILE_RSHRINEG, 0,                                         THEME_SHRINE,            Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, TRUE,     FALSE,     ALIGN },
/*OBJ_BOOKCASEL*/    { OFILE_BCASE,    0,                                         THEME_LIBRARY,           Q_INVALID,           3,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, TRUE,     FALSE,     ALIGN },
/*OBJ_BOOKCASER*/    { OFILE_BCASE,    0,                                         THEME_LIBRARY,           Q_INVALID,           4,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, TRUE,     FALSE,     ALIGN },
/*OBJ_BOOKCANDLE*/   { OFILE_CANDLE2,  0,                                         THEME_LIBRARY,           Q_INVALID,           0,            7, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT 5
/*OBJ_BLOODFTN*/     { OFILE_BLOODFNT, 0,                                         THEME_BLOODFOUNTAIN,     Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_DECAP*/        { OFILE_DECAP,    DTM_HELL,                                  THEME_DECAPITATED,       Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, TRUE,     FALSE,     ALIGN },
/*OBJ_TCHEST1*/      { OFILE_CHEST1,   DTM_ANY & ~DTM_CATHEDRAL,                  THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_TCHEST2*/      { OFILE_CHEST2,   DTM_ANY & ~DTM_CATHEDRAL,                  THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_TCHEST3*/      { OFILE_CHEST3,   DTM_ANY & ~DTM_CATHEDRAL,                  THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_ANCIENTBOOK*/  { OFILE_BOOK2,    0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_VILEBOOK*/     { OFILE_BOOK2,    0,                                         THEME_NONE,              Q_INVALID,           4,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_MYTHICBOOK*/   { OFILE_BOOK2,    0,                                         THEME_NONE,              Q_BCHAMB,            4,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_BOOK2L*/       { OFILE_BOOK2,    0,                                         THEME_LIBRARY,           Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_BOOK2R*/       { OFILE_BOOK2,    0,                                         THEME_SKELROOM,          Q_INVALID,           4,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_BOOKLVR*///    { OFILE_BOOK1,    0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_BLINDBOOK*/    { OFILE_BOOK1,    0,                                         THEME_NONE,              Q_BLIND,             1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_BLOODBOOK*/    { OFILE_BOOK1,    0,                                         THEME_NONE,              Q_BLOOD,             4,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_STEELTOME*/    { OFILE_BOOK1,    0,                                         THEME_NONE,              Q_WARLORD,           4,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_PEDESTAL*/     { OFILE_PEDISTL,  0,                                         THEME_NONE,              Q_BLOOD,             1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_L3LDOOR*/      { OFILE_L3DOORS,  DTM_CAVES,                                 THEME_NONE,              Q_INVALID,           2,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_LEFT,         3, FALSE,    TRUE,      ALIGN },
/*OBJ_L3RDOOR*/      { OFILE_L3DOORS,  DTM_CAVES,                                 THEME_NONE,              Q_INVALID,           1,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_RIGHT,        3, FALSE,    TRUE,      ALIGN },
/*OBJ_PURIFYINGFTN*/ { OFILE_PFOUNTN,  0,                                         THEME_PURIFYINGFOUNTAIN, Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_ARMORSTAND*/   { OFILE_ARMSTAND, 0,                                         THEME_ARMORSTAND,        Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_ARMORSTANDN*/  { OFILE_ARMSTAND, 0,                                         THEME_ARMORSTAND,        Q_WARLORD,           2,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_GOATSHRINE*/   { OFILE_GOATSHRN, 0,                                         THEME_GOATSHRINE,        Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_CAULDRON*/     { OFILE_CAULDREN, DTM_HELL,                                  THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_MURKYFTN*/     { OFILE_MFOUNTN,  0,                                         THEME_MURKYFOUNTAIN,     Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_TEARFTN*/      { OFILE_TFOUNTN,  0,                                         THEME_TEARFOUNTAIN,      Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_ALTBOY*/       { OFILE_ALTBOY,   0,                                         THEME_NONE,              Q_BETRAYER,          1,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_MCIRCLE1*/     { OFILE_MCIRL,    0,                                         THEME_NONE,              Q_INVALID,           1,            0, OPF_CIRCLE, OMF_FLOOR,              TRUE,      ODT_NONE,         0, TRUE,     FALSE,     ALIGN },
/*OBJ_MCIRCLE2*/     { OFILE_MCIRL,    0,                                         THEME_NONE,              Q_INVALID,           3,            0, OPF_CIRCLE, OMF_FLOOR,              TRUE,      ODT_NONE,         0, TRUE,     FALSE,     ALIGN },
/*OBJ_STORYBOOK*/    { OFILE_BKSLBRNT, DTM_CATHEDRAL | DTM_CATACOMBS | DTM_CAVES, THEME_NONE,              Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN }, // BUGFIX should only be loaded on level 1-12 (crypt masks as 1-4) (fixed)
/*OBJ_STORYCANDLE*/  { OFILE_CANDLE2,  DTM_CATHEDRAL | DTM_CATACOMBS | DTM_CAVES, THEME_NONE,              Q_BETRAYER,          0,            7, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT
/*OBJ_TBCROSS*/      { OFILE_BURNCROS, 0,                                         THEME_BRNCROSS,          Q_INVALID,           0,           15, OPF_BCROSS, OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // + OPF_LIGHT 5
/*OBJ_WEAPONRACKL*/  { OFILE_WEAPSTND, 0,                                         THEME_WEAPONRACK,        Q_INVALID,           1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_WEAPONRACKLN*/ { OFILE_WEAPSTND, 0,                                         THEME_WEAPONRACK,        Q_WARLORD,           2,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_WEAPONRACKR*/  { OFILE_WEAPSTND, 0,                                         THEME_WEAPONRACK,        Q_INVALID,           3,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, FALSE,     ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_WEAPONRACKRN*/ { OFILE_WEAPSTND, 0,                                         THEME_WEAPONRACK,        Q_INVALID,           4,            0, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
/*OBJ_MUSHPATCH*/    { OFILE_MUSHPTCH, 0,                                         THEME_NONE,              Q_MUSHROOM,          1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_LAZSTAND*/     { OFILE_LZSTAND,  0,                                         THEME_NONE,              Q_BETRAYER,          1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_SLAINHERO*///  { OFILE_DECAP,    DTM_CAVES,                                 THEME_NONE,              Q_INVALID,           2,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_SIGNCHEST*/    { OFILE_CHEST3,   0,                                         THEME_NONE,              Q_BANNER,            1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_BOOKSHELFR*/// { OFILE_BCASE,    0,                                         THEME_NONE,              Q_INVALID,           2,            0, OPF_NONE,   OMF_FLOOR,              FALSE,     ODT_NONE,         0, FALSE,    FALSE,     ALIGN },
#ifdef HELLFIRE
/*OBJ_L5LDOOR*/      { OFILE_L5DOOR,   DTM_CRYPT,                                 THEME_NONE,              Q_INVALID,           1,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_LEFT,         3, FALSE,    TRUE,      ALIGN },
/*OBJ_L5RDOOR*/      { OFILE_L5DOOR,   DTM_CRYPT,                                 THEME_NONE,              Q_INVALID,           2,            0, OPF_DOOR,   OMF_ACTIVE,             FALSE,     ODT_RIGHT,        3, FALSE,    TRUE,      ALIGN },
/*OBJ_L5SARC*/       { OFILE_L5SARCO,  DTM_CRYPT,                                 THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    TRUE,      ALIGN },
/*OBJ_URN*/          { OFILE_URN,      DTM_CRYPT,                                 THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_URNEX*/        { OFILE_URNEXPLD, DTM_CRYPT,                                 THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_L5CANDLE*/     { OFILE_L5CANDLE, DTM_CRYPT,                                 THEME_NONE,              Q_INVALID,           1,           10, OPF_NONE,   OMF_FLOOR,              TRUE,      ODT_NONE,         0, FALSE,    FALSE,     ALIGN }, // OPF_LIGHT
/*OBJ_L5BOOK*/       { OFILE_L5BOOKS,  DTM_CRYPT,                                 THEME_NONE,              Q_INVALID,           3,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_NAKRULBOOK*/   { OFILE_L5BOOKS,  0,                                         THEME_NONE,              Q_NAKRUL,            3,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_NAKRULLEVER*/  { OFILE_L5LEVER,  0,                                         THEME_NONE,              Q_NAKRUL,            1,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         1, FALSE,    FALSE,     ALIGN },
/*OBJ_POD*/          { OFILE_L6POD1,   DTM_NEST,                                  THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
/*OBJ_PODEX*/        { OFILE_L6POD2,   DTM_NEST,                                  THEME_NONE,              Q_INVALID,           0,            0, OPF_NONE,   OMF_FLOOR | OMF_ACTIVE, TRUE,      ODT_NONE,         3, FALSE,    FALSE,     ALIGN },
#endif
	// clang-format on
};

const ObjFileData objfiledata[NUM_OFILE_TYPES] = {
	// clang-format off
//                   ofName,     oSFX,        oSFXCnt, oAnimFlag, oAnimFrameLen, oAnimLen, oSolidFlag, oBreak,          
/*OFILE_L1BRAZ*/   { "L1Braz",   SFX_NONE,          0, OAM_LOOP,              1,       26, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_L1DOORS*/  { "L1Doors",  IS_DOOROPEN,       2, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_LEVER*/    { "Lever",    SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_CHEST1*/   { "Chest1",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_CHEST2*/   { "Chest2",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_BANNER*/   { "Banner",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_SKULPILE*///{ "SkulPile", SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_SKULFIRE*/ { "SkulFire", SFX_NONE,          0, OAM_LOOP,              2,       11, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_SKULSTIK*///{ "SkulStik", SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_CRUXSK1*/  { "CruxSk1",  SFX_NONE,          0, OAM_SINGLE,            1,       15, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_CRUXSK2*/  { "CruxSk2",  SFX_NONE,          0, OAM_SINGLE,            1,       15, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_CRUXSK3*/  { "CruxSk3",  SFX_NONE,          0, OAM_SINGLE,            1,       15, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_BOOK1*/    { "Book1",    SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_BOOK2*/    { "Book2",    SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_ROCKSTAN*/ { "Rockstan", SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_ANGEL*///  { "Angel",    SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_GHOST*///  { "Ghost",    SFX_NONE,          0, OAM_LOOP,              0,       14, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_CHEST3*/   { "Chest3",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_BURNCROS*/ { "Burncros", SFX_NONE,          0, OAM_LOOP,              1,       10, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_CANDLE*/// { "Candle",   SFX_NONE,          0, OAM_NONE,              0,        9, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_CANDLE2*/  { "Candle2",  SFX_NONE,          0, OAM_LOOP,              2,        4, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_CANDLABR*///{ "Candlabr", SFX_NONE,          0, OAM_NONE,              0,        1, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_NUDE2*///  { "Nude2",    SFX_NONE,          0, OAM_LOOP,              3,        6, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_SWITCH2*///{ "Switch2",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_SWITCH3*///{ "Switch3",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_SWITCH4*/  { "Switch4",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_TNUDEM*/   { "TNudeM",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_TNUDEW*/   { "TNudeW",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_TSOUL*/    { "TSoul",    SFX_NONE,          0, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_L2DOORS*/  { "L2Doors",  IS_DOOROPEN,       2, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_WTORCH4*/  { "WTorch4",  SFX_NONE,          0, OAM_LOOP,              1,        9, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_WTORCH3*/  { "WTorch3",  SFX_NONE,          0, OAM_LOOP,              1,        9, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_SARC*/     { "Sarc",     SFX_NONE,          0, OAM_SINGLE,            3,        5, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_FLAME1*/// { "Flame1",   SFX_NONE,          0, OAM_LOOP,              1,       20, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_FLAME3*/// { "Flame3",   SFX_NONE,          0, OAM_LOOP,              1,       22, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_FIREWAL1*///{ "Firewal1", SFX_NONE,          0, OAM_LOOP,              1,       13, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_EXPLOD1*///{ "Explod1",  SFX_NONE,          0, OAM_LOOP,              1,       11, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_EXPLOD2*///{ "Explod2",  SFX_NONE,          0, OAM_LOOP,              1,       11, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_VAPOR1*/// { "Vapor1",   SFX_NONE,          0, OAM_LOOP,              1,       13, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_PRSRPLT1*///{ "Prsrplt1", SFX_NONE,          0, OAM_NONE,              0,       10, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_TRAPHOLE*/ { "Traphole", SFX_NONE,          0, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_DIRTFALL*///{ "Dirtfall", SFX_NONE,          0, OAM_LOOP,              0,       10, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_WATER*///  { "Water",    SFX_NONE,          0, OAM_LOOP,              1,       10, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_MINIWATR*///{ "MiniWatr", SFX_NONE,          0, OAM_LOOP,              1,       10, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_WTORCH2*/  { "WTorch2",  SFX_NONE,          0, OAM_LOOP,              1,        9, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_WTORCH1*/  { "WTorch1",  SFX_NONE,          0, OAM_LOOP,              1,        9, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_BCASE*/    { "BCase",    SFX_NONE,          0, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_BSHELF*/// { "BShelf",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_WEAPSTND*/ { "WeapStnd", SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_BKURNS*/// { "Bkurns",   SFX_NONE,          1, OAM_NONE,              1,       10, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_WATERJUG*///{ "Waterjug", SFX_NONE,          1, OAM_NONE,              1,        4, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_BARREL*/   { "Barrel",   IS_BARREL,         1, OAM_SINGLE,            1,        9, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_BARRELEX*/ { "Barrelex", IS_BARLFIRE,       1, OAM_SINGLE,            1,       10, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_LSHRINEG*/ { "LShrineG", IS_MAGIC,          2, OAM_SINGLE,            1,       11, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_RSHRINEG*/ { "RShrineG", IS_MAGIC,          2, OAM_SINGLE,            1,       11, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_BLOODFNT*/ { "Bloodfnt", SFX_NONE,          0, OAM_LOOP,              2,       10, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_DECAP*/    { "Decap",    SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_PEDISTL*/  { "Pedistl",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_L3DOORS*/  { "L3Doors",  IS_DOOROPEN,       2, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_PFOUNTN*/  { "PFountn",  SFX_NONE,          0, OAM_LOOP,              2,       10, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_ARMSTAND*/ { "Armstand", SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_GOATSHRN*/ { "Goatshrn", LS_GSHRINE,        1, OAM_LOOP,              2,       10, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_CAULDREN*/ { "Cauldren", LS_CALDRON,        1, OAM_SINGLE,            2,        3, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_MFOUNTN*/  { "MFountn",  SFX_NONE,          0, OAM_LOOP,              2,       10, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_TFOUNTN*/  { "TFountn",  SFX_NONE,          0, OAM_LOOP,              2,        4, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_ALTBOY*/   { "Altboy",   SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_MCIRL*/    { "Mcirl",    SFX_NONE,          0, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_BKSLBRNT*/ { "Bkslbrnt", SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_MUSHPTCH*/ { "Mushptch", SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_LZSTAND*/  { "LzStand",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
#ifdef HELLFIRE
/*OFILE_L5DOOR*/   { "L5Door",   IS_CROPEN,         2, OAM_NONE,              0,        0, FALSE,      OBM_UNBREAKABLE, ALIGN },
/*OFILE_L5SARCO*/  { "L5Sarco",  SFX_NONE,          0, OAM_SINGLE,            3,        5, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_URN*/      { "Urn",      IS_POPPOP2,        1, OAM_SINGLE,            1,        9, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_URNEXPLD*/ { "Urnexpld", IS_POPPOP3,        1, OAM_SINGLE,            1,       10, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_L5BOOKS*/  { "L5Books",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_L5CANDLE*/ { "L5Light",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_L5LEVER*/  { "L5Lever",  SFX_NONE,          0, OAM_NONE,              0,        0, TRUE,       OBM_UNBREAKABLE, ALIGN },
/*OFILE_L6POD1*/   { "L6Pod1",   IS_POPPOP5,        1, OAM_SINGLE,            1,        9, TRUE,       OBM_BREAKABLE,   ALIGN },
/*OFILE_L6POD2*/   { "L6Pod2",   IS_POPPOP8,        1, OAM_SINGLE,            1,       10, TRUE,       OBM_BREAKABLE,   ALIGN },
#endif
	// clang-format on
};

DEVILUTION_END_NAMESPACE
