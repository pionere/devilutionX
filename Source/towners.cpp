/**
 * @file towners.cpp
 *
 * Implementation of functionality for loading and spawning towners.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BOOL storeflag;
int sgnCowMsg;
int numtowners;
DWORD sgdwCowClicks;
/** unused 0x6AAC28 */
BOOL bannerflag;
BOOL boyloadflag;
BYTE *pCowCels;
TownerStruct towner[NUM_TOWNERS];

/**
 * Maps from active cow sound effect index and player class to sound
 * effect ID for interacting with cows in Tristram.
 *
 * ref: enum _sfx_id
 * ref: enum plr_class
 */
const int snSFX[3][NUM_CLASSES] = {
#ifdef HELLFIRE
	{ PS_WARR52, PS_ROGUE52, PS_MAGE52, PS_MONK52, 0 }, // BUGFIX: add warrior sounds for barbarian instead of 0 - walk sound
	{ PS_WARR49, PS_ROGUE49, PS_MAGE49, PS_MONK49, 0 },
	{ PS_WARR50, PS_ROGUE50, PS_MAGE50, PS_MONK50, 0 },
#else
	{ PS_WARR52, PS_ROGUE52, PS_MAGE52 },
	{ PS_WARR49, PS_ROGUE49, PS_MAGE49 },
	{ PS_WARR50, PS_ROGUE50, PS_MAGE50 },
#endif
};

/* data */
/** Specifies the animation frame sequence of a given NPC. */
char AnimOrder[6][148] = {
	{ 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	    14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
	    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	    14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
	    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	    14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
	    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	    14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
	    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	    14, 13, 12, 11, 10, 9, 8, 7, 6, 5,
	    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	    15, 5, 1, 1, 1, 1, 1, 1, 1, 1,
	    1, 1, 1, 1, 1, 1, 1, 2, 3, 4,
	    -1 },
	{ 1, 2, 3, 3, 2, 1, 20, 19, 19, 20,
	    1, 2, 3, 3, 2, 1, 20, 19, 19, 20,
	    1, 2, 3, 3, 2, 1, 20, 19, 19, 20,
	    1, 2, 3, 3, 2, 1, 20, 19, 19, 20,
	    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	    11, 12, 13, 14, 15, 16, 15, 14, 13, 12,
	    11, 10, 9, 8, 7, 6, 5, 4, 5, 6,
	    7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	    15, 14, 13, 12, 11, 10, 9, 8, 7, 6,
	    5, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	    13, 14, 15, 16, 17, 18, 19, 20, -1 },
	{ 1, 1, 25, 25, 24, 23, 22, 21, 20, 19,
	    18, 17, 16, 15, 16, 17, 18, 19, 20, 21,
	    22, 23, 24, 25, 25, 25, 1, 1, 1, 25,
	    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	    11, 12, 13, 14, 15, 14, 13, 12, 11, 10,
	    9, 8, 7, 6, 5, 4, 3, 2, 1, -1 },
	{ 1, 2, 3, 3, 2, 1, 16, 15, 14, 14,
	    16, 1, 2, 3, 3, 2, 1, 16, 15, 14,
	    14, 15, 16, 1, 2, 3, 3, 2, 1, 16,
	    15, 14, 14, 15, 16, 1, 2, 3, 3, 2,
	    1, 16, 15, 14, 14, 15, 16, 1, 2, 3,
	    3, 2, 1, 16, 15, 14, 14, 15, 16, 1,
	    2, 3, 3, 2, 1, 16, 15, 14, 14, 15,
	    16, 1, 2, 3, 3, 2, 1, 16, 15, 14,
	    14, 15, 16, 1, 2, 3, 2, 1, 16, 15,
	    14, 14, 15, 16, 1, 2, 3, 4, 5, 6,
	    7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	    -1 },
	{ 1, 1, 1, 2, 3, 4, 5, 6, 7, 8,
	    9, 10, 11, 11, 11, 11, 12, 13, 14, 15,
	    16, 17, 18, 18, 1, 1, 1, 18, 17, 16,
	    15, 14, 13, 12, 11, 10, 11, 12, 13, 14,
	    15, 16, 17, 18, 1, 2, 3, 4, 5, 5,
	    5, 4, 3, 2, -1 },
	{ 4, 4, 4, 5, 6, 6, 6, 5, 4, 15,
	    14, 13, 13, 13, 14, 15, 4, 5, 6, 6,
	    6, 5, 4, 4, 4, 5, 6, 6, 6, 5,
	    4, 15, 14, 13, 13, 13, 14, 15, 4, 5,
	    6, 6, 6, 5, 4, 4, 4, 5, 6, 6,
	    6, 5, 4, 15, 14, 13, 13, 13, 14, 15,
	    4, 5, 6, 6, 6, 5, 4, 3, 2, 1,
	    19, 18, 19, 1, 2, 1, 19, 18, 19, 1,
	    2, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	    10, 11, 12, 13, 14, 15, 15, 15, 14, 13,
	    13, 13, 13, 14, 15, 15, 15, 14, 13, 12,
	    12, 12, 11, 10, 10, 10, 9, 8, 9, 10,
	    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	    1, 2, 1, 19, 18, 19, 1, 2, 1, 2,
	    3, -1 }
};
/** Specifies the start X-coordinates of the cows in Tristram. */
int TownCowX[] = { 58, 56, 59 };
/** Specifies the start Y-coordinates of the cows in Tristram. */
int TownCowY[] = { 16, 14, 20 };
/** Specifies the start directions of the cows in Tristram. */
int TownCowDir[] = { DIR_SW, DIR_NW, DIR_N };
/** Maps from direction to X-coordinate delta, which is used when
 * placing cows in Tristram. A single cow may require space of up
 * to three tiles when being placed on the map.
 */
int cowoffx[8] = { -1, 0, -1, -1, -1, 0, -1, -1 };
/** Maps from direction to Y-coordinate delta, which is used when
 * placing cows in Tristram. A single cow may require space of up
 * to three tiles when being placed on the map.
 */
int cowoffy[8] = { -1, -1, -1, 0, -1, -1, -1, 0 };
/** Contains the data related to quest gossip for each towner ID. */
QuestTalkData Qtalklist[] = {
	// clang-format off
#ifdef HELLFIRE
	// _qinfra,     _qblkm,      _qgarb,      _qzhar,      _qveil,     _qmod,       _qbutch,     _qbol,        _qblind,     _qblood,     _qanvil,      _qwarlrd,     _qking,      _qpw,          _qbone,     _qvb,        _qgrv,       _qfarm, _qgirl, _qtrade, _qdefiler, _qnakrul, _qjersy, _qhf8
	{ TEXT_INFRA6,  TEXT_MUSH6,  -1,          -1,          TEXT_VEIL5, -1,          TEXT_BUTCH5, TEXT_BANNER6, TEXT_BLIND5, TEXT_BLOOD5, TEXT_ANVIL6,  TEXT_WARLRD5, TEXT_KING7,  TEXT_POISON7,  TEXT_BONE5, TEXT_VILE9,  TEXT_GRAVE2, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ TEXT_INFRA3,  -1,          -1,          -1,          TEXT_VEIL3, -1,          TEXT_BUTCH3, TEXT_BANNER4, TEXT_BLIND3, TEXT_BLOOD3, TEXT_ANVIL3,  TEXT_WARLRD3, TEXT_KING5,  TEXT_POISON4,  TEXT_BONE3, TEXT_VILE7,  TEXT_GRAVE3, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,           -1,          -1,          -1,           -1,           -1,          -1,            -1,         -1,          -1,          -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ TEXT_INFRA2,  TEXT_MUSH2,  -1,          -1,          TEXT_VEIL2, -1,          TEXT_BUTCH2, -1,           TEXT_BLIND2, TEXT_BLOOD2, TEXT_ANVIL2,  TEXT_WARLRD2, TEXT_KING3,  TEXT_POISON2,  TEXT_BONE2, TEXT_VILE4,  TEXT_GRAVE5, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ TEXT_INFRA1,  TEXT_MUSH1,  -1,          -1,          TEXT_VEIL1, TEXT_VILE3,  TEXT_BUTCH1, TEXT_BANNER1, TEXT_BLIND1, TEXT_BLOOD1, TEXT_ANVIL1,  TEXT_WARLRD1, TEXT_KING1,  TEXT_POISON1,  TEXT_BONE1, TEXT_VILE2,  TEXT_GRAVE6, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ TEXT_INFRA8,  TEXT_MUSH7,  -1,          -1,          TEXT_VEIL6, -1,          TEXT_BUTCH6, TEXT_BANNER7, TEXT_BLIND6, TEXT_BLOOD6, TEXT_ANVIL8,  TEXT_WARLRD6, TEXT_KING8,  TEXT_POISON8,  TEXT_BONE6, TEXT_VILE10, TEXT_GRAVE7, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ TEXT_INFRA9,  TEXT_MUSH9,  -1,          -1,          TEXT_VEIL7, -1,          TEXT_BUTCH7, TEXT_BANNER8, TEXT_BLIND7, TEXT_BLOOD7, TEXT_ANVIL9,  TEXT_WARLRD7, TEXT_KING9,  TEXT_POISON9,  TEXT_BONE7, TEXT_VILE11, TEXT_GRAVE1, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ TEXT_INFRA4,  TEXT_MUSH5,  -1,          -1,          TEXT_VEIL4, -1,          TEXT_BUTCH4, TEXT_BANNER5, TEXT_BLIND4, TEXT_BLOOD4, TEXT_ANVIL4,  TEXT_WARLRD4, TEXT_KING6,  TEXT_POISON6,  TEXT_BONE4, TEXT_VILE8,  TEXT_GRAVE8, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ TEXT_INFRA10, TEXT_MUSH13, -1,          -1,          TEXT_VEIL8, -1,          TEXT_BUTCH8, TEXT_BANNER9, TEXT_BLIND8, TEXT_BLOOD8, TEXT_ANVIL10, TEXT_WARLRD8, TEXT_KING10, TEXT_POISON10, TEXT_BONE8, TEXT_VILE12, TEXT_GRAVE9, -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,            -1,           -1,        -1,           -1,           -1,          -1,            -1,         -1,          -1,          -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,            -1,           -1,        -1,           -1,           -1,          -1,            -1,         -1,          -1,          -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,            -1,           -1,        -1,           -1,           -1,          -1,            -1,         -1,          -1,          -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,            -1,           -1,        -1,           -1,           -1,          -1,            -1,         -1,          -1,          -1,     -1,     -1,      -1,        -1,       -1,      -1 },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,            -1,           -1,        -1,           -1,           -1,          -1,            -1,         -1,          -1,          -1,     -1,     -1,      -1,        -1,       -1,      -1 },
#else
	// _qinfra,     _qblkm,      _qgarb,      _qzhar,      _qveil,      _qmod,       _qbutch,    _qbol,        _qblind,     _qblood,     _qanvil,      _qwarlrd,     _qking,      _qpw,          _qbone,     _qvb
	{ TEXT_INFRA6,  TEXT_MUSH6,  -1,          -1,          TEXT_VEIL5, -1,          TEXT_BUTCH5, TEXT_BANNER6, TEXT_BLIND5, TEXT_BLOOD5, TEXT_ANVIL6,  TEXT_WARLRD5, TEXT_KING7,  TEXT_POISON7,  TEXT_BONE5, TEXT_VILE9  },
	{ TEXT_INFRA3,  -1,          -1,          -1,          TEXT_VEIL3, -1,          TEXT_BUTCH3, TEXT_BANNER4, TEXT_BLIND3, TEXT_BLOOD3, TEXT_ANVIL3,  TEXT_WARLRD3, TEXT_KING5,  TEXT_POISON4,  TEXT_BONE3, TEXT_VILE7  },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,           -1,          -1,          -1,           -1,           -1,          -1,            -1,         -1          },
	{ TEXT_INFRA2,  TEXT_MUSH2,  -1,          -1,          TEXT_VEIL2, -1,          TEXT_BUTCH2, -1,           TEXT_BLIND2, TEXT_BLOOD2, TEXT_ANVIL2,  TEXT_WARLRD2, TEXT_KING3,  TEXT_POISON2,  TEXT_BONE2, TEXT_VILE4  },
	{ TEXT_INFRA1,  TEXT_MUSH1,  -1,          -1,          TEXT_VEIL1, TEXT_VILE3,  TEXT_BUTCH1, TEXT_BANNER1, TEXT_BLIND1, TEXT_BLOOD1, TEXT_ANVIL1,  TEXT_WARLRD1, TEXT_KING1,  TEXT_POISON1,  TEXT_BONE1, TEXT_VILE2  },
	{ TEXT_INFRA8,  TEXT_MUSH7,  -1,          -1,          TEXT_VEIL6, -1,          TEXT_BUTCH6, TEXT_BANNER7, TEXT_BLIND6, TEXT_BLOOD6, TEXT_ANVIL8,  TEXT_WARLRD6, TEXT_KING8,  TEXT_POISON8,  TEXT_BONE6, TEXT_VILE10 },
	{ TEXT_INFRA9,  TEXT_MUSH9,  -1,          -1,          TEXT_VEIL7, -1,          TEXT_BUTCH7, TEXT_BANNER8, TEXT_BLIND7, TEXT_BLOOD7, TEXT_ANVIL9,  TEXT_WARLRD7, TEXT_KING9,  TEXT_POISON9,  TEXT_BONE7, TEXT_VILE11 },
	{ TEXT_INFRA4,  TEXT_MUSH5,  -1,          -1,          TEXT_VEIL4, -1,          TEXT_BUTCH4, TEXT_BANNER5, TEXT_BLIND4, TEXT_BLOOD4, TEXT_ANVIL4,  TEXT_WARLRD4, TEXT_KING6,  TEXT_POISON6,  TEXT_BONE4, TEXT_VILE8  },
	{ TEXT_INFRA10, TEXT_MUSH13, -1,          -1,          TEXT_VEIL8, -1,          TEXT_BUTCH8, TEXT_BANNER9, TEXT_BLIND8, TEXT_BLOOD8, TEXT_ANVIL10, TEXT_WARLRD8, TEXT_KING10, TEXT_POISON10, TEXT_BONE8, TEXT_VILE12 },
	{ -1,           -1,          -1,          -1,          -1,         -1,          -1,          -1,           -1,          -1,          -1,           -1,           -1,          -1,            -1,         -1          },
	{ TEXT_KING1,   TEXT_KING1,  TEXT_KING1, TEXT_KING1,   TEXT_KING1, TEXT_KING1,  TEXT_KING1,  TEXT_KING1,   TEXT_KING1,  TEXT_KING1,  TEXT_KING1,   TEXT_KING1,   TEXT_KING1,  TEXT_KING1,    TEXT_KING1, TEXT_KING1  }
#endif
	// clang-format on
};
/** Specifies the active sound effect ID for interacting with cows. */
int CowPlaying = -1;

void SetTownerGPtrs(BYTE *pData, BYTE **pAnim)
{
	int i;
	DWORD *pFrameTable;

	pFrameTable = (DWORD *)pData;

	for (i = 0; i < 8; i++) {
		pAnim[i] = CelGetFrameStart(pData, i);
	}
}

void NewTownerAnim(int tnum, BYTE *pAnim, int numFrames, int Delay)
{
	towner[tnum]._tAnimData = pAnim;
	towner[tnum]._tAnimLen = numFrames;
	towner[tnum]._tAnimFrame = 1;
	towner[tnum]._tAnimCnt = 0;
	towner[tnum]._tAnimDelay = Delay;
}

void InitTownerInfo(int tnum, int w, int sel, int type, int x, int y, int ao, int tp)
{
	TownerStruct *tw = &towner[tnum];

	memset(tw, 0, sizeof(TownerStruct));
	tw->_tSelFlag = sel;
	tw->_tAnimWidth = w;
	tw->_tAnimWidth2 = (w - 64) >> 1;
	tw->_tMsgSaid = FALSE;
	tw->_ttype = type;
	tw->_tx = x;
	tw->_ty = y;
	dMonster[x][y] = tnum + 1;
	tw->_tAnimOrder = ao;
	tw->_tTenPer = tp;
	tw->_tSeed = GetRndSeed();
}

void InitQstSnds(int tnum)
{
	TNQ *tqst;
	int j, tl;
	tl = tnum;
	if (boyloadflag)
		tl++;
	tqst = towner[tnum].qsts;
	for (j = 0; j < MAXQUESTS; j++, tqst++) {
		tqst->_qsttype = quests[j]._qtype;
		tqst->_qstmsg = ((int *)(Qtalklist + tl))[j];
		if (((int *)(Qtalklist + tl))[j] != -1)
			tqst->_qstmsgact = TRUE;
		else
			tqst->_qstmsgact = FALSE;
	}
}

/**
 * @brief Load Griswold into the game

 */
void InitSmith()
{
	int i;

	InitTownerInfo(numtowners, 96, TRUE, TOWN_SMITH, 62, 63, 0, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\Smith\\SmithN.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 16;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_SW], towner[numtowners]._tNFrames, 3);
	strcpy(towner[numtowners]._tName, "Griswold the Blacksmith");
	numtowners++;
}

void InitBarOwner()
{
	int i;

	bannerflag = FALSE; // unused
	InitTownerInfo(numtowners, 96, TRUE, TOWN_TAVERN, 55, 62, 3, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\TwnF\\TwnFN.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 16;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_SW], towner[numtowners]._tNFrames, 3);
	strcpy(towner[numtowners]._tName, "Ogden the Tavern owner");
	numtowners++;
}

void InitTownDead()
{
	int i;

	InitTownerInfo(numtowners, 96, TRUE, TOWN_DEADGUY, 24, 32, -1, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\Butch\\Deadguy.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 8;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_N], towner[numtowners]._tNFrames, 6);
	strcpy(towner[numtowners]._tName, "Wounded Townsman");
	numtowners++;
}

void InitWitch()
{
	int i;

	InitTownerInfo(numtowners, 96, TRUE, TOWN_WITCH, 80, 20, 5, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\TownWmn1\\Witch.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 19;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_S], towner[numtowners]._tNFrames, 6);
	strcpy(towner[numtowners]._tName, "Adria the Witch");
	numtowners++;
}

void InitBarmaid()
{
	int i;

	InitTownerInfo(numtowners, 96, TRUE, TOWN_BMAID, 43, 66, -1, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\TownWmn1\\WmnN.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 18;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_S], towner[numtowners]._tNFrames, 6);
	strcpy(towner[numtowners]._tName, "Gillian the Barmaid");
	numtowners++;
}

void InitBoy()
{
	int i;

	boyloadflag = TRUE;
	InitTownerInfo(numtowners, 96, TRUE, TOWN_PEGBOY, 11, 53, -1, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\TownBoy\\PegKid1.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 20;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_S], towner[numtowners]._tNFrames, 6);
	strcpy(towner[numtowners]._tName, "Wirt the Peg-legged boy");
	numtowners++;
}

void InitHealer()
{
	int i;

	InitTownerInfo(numtowners, 96, TRUE, TOWN_HEALER, 55, 79, 1, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\Healer\\Healer.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 20;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_SE], towner[numtowners]._tNFrames, 6);
	strcpy(towner[numtowners]._tName, "Pepin the Healer");
	numtowners++;
}

void InitTeller()
{
	int i;

	InitTownerInfo(numtowners, 96, TRUE, TOWN_STORY, 62, 71, 2, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\Strytell\\Strytell.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 25;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_S], towner[numtowners]._tNFrames, 3);
	strcpy(towner[numtowners]._tName, "Cain the Elder");
	numtowners++;
}

void InitDrunk()
{
	int i;

	InitTownerInfo(numtowners, 96, TRUE, TOWN_DRUNK, 71, 84, 4, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\Drunk\\TwnDrunk.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 18;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_S], towner[numtowners]._tNFrames, 3);
	strcpy(towner[numtowners]._tName, "Farnham the Drunk");
	numtowners++;
}

void InitCows()
{
	int i, dir;
	int x, y, xo, yo;

	//if ( pCowCels )
	// assertion_failed(__LINE__, __FILE__, "! pCowCels");
	pCowCels = LoadFileInMem("Towners\\Animals\\Cow.CEL", NULL);
	for (i = 0; i < 3; i++) {
		x = TownCowX[i];
		y = TownCowY[i];
		dir = TownCowDir[i];
		InitTownerInfo(numtowners, 128, FALSE, TOWN_COW, x, y, -1, 10);
		towner[numtowners]._tNData = pCowCels;
		SetTownerGPtrs(towner[numtowners]._tNData, towner[numtowners]._tNAnim);
		towner[numtowners]._tNFrames = 12;
		NewTownerAnim(numtowners, towner[numtowners]._tNAnim[dir], towner[numtowners]._tNFrames, 3);
		towner[numtowners]._tAnimFrame = random_(0, 11) + 1;
		towner[numtowners]._tSelFlag = TRUE;
		strcpy(towner[numtowners]._tName, "Cow");

		xo = x + cowoffx[dir];
		yo = y + cowoffy[dir];
		if (dMonster[x][yo] == 0)
			dMonster[x][yo] = -(numtowners + 1);
		if (dMonster[xo][y] == 0)
			dMonster[xo][y] = -(numtowners + 1);
		if (dMonster[xo][yo] == 0)
			dMonster[xo][yo] = -(numtowners + 1);

		numtowners++;
	}
}

#ifdef HELLFIRE
void InitFarmer()
{
	int i;

	InitTownerInfo(numtowners, 96, 1, TOWN_FARMER, 62, 16, -1, 10);
	InitQstSnds(numtowners);
	towner[numtowners]._tNData = LoadFileInMem("Towners\\Farmer\\Farmrn2.CEL", NULL);
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 15;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_S], towner[numtowners]._tNFrames, 3);
	strcpy(towner[numtowners]._tName, "Lester the farmer");
	numtowners++;
}

void InitCowFarmer()
{
	int i;

	InitTownerInfo(numtowners, 96, 1, TOWN_COWFARM, 61, 22, -1, 10);
	InitQstSnds(numtowners);
	if (quests[Q_JERSEY]._qactive != 3) {
		towner[numtowners]._tNData = LoadFileInMem("Towners\\Farmer\\cfrmrn2.CEL", NULL);
	} else {
		towner[numtowners]._tNData = LoadFileInMem("Towners\\Farmer\\mfrmrn2.CEL", NULL);
	}
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 15;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_SW], towner[numtowners]._tNFrames, 3);
	strcpy(towner[numtowners]._tName, "Complete Nut");
	numtowners++;
}

void InitGirl()
{
	int i;

	InitTownerInfo(numtowners, 96, 1, TOWN_GIRL, 77, 43, -1, 10);
	InitQstSnds(numtowners);
	if (quests[Q_GIRL]._qactive != 3) {
		towner[numtowners]._tNData = LoadFileInMem("Towners\\Girl\\Girlw1.CEL", NULL);
	} else {
		towner[numtowners]._tNData = LoadFileInMem("Towners\\Girl\\Girls1.CEL", NULL);
	}
	for (i = 0; i < 8; i++) {
		towner[numtowners]._tNAnim[i] = towner[numtowners]._tNData;
	}
	towner[numtowners]._tNFrames = 20;
	NewTownerAnim(numtowners, towner[numtowners]._tNAnim[DIR_S], towner[numtowners]._tNFrames, 6);
	strcpy(towner[numtowners]._tName, "Celia");
	numtowners++;
}
#endif

void InitTowners()
{
	numtowners = 0;
	boyloadflag = FALSE;
	InitSmith();
	InitHealer();
	if (quests[Q_BUTCHER]._qactive != QUEST_NOTAVAIL && quests[Q_BUTCHER]._qactive != QUEST_DONE)
		InitTownDead();
	InitBarOwner();
	InitTeller();
	InitDrunk();
	InitWitch();
	InitBarmaid();
	InitBoy();
	InitCows();
#ifdef HELLFIRE
	if (UseCowFarmer) {
		InitCowFarmer();
	} else if (quests[Q_FARMER]._qactive != 10) {
		InitFarmer();
	}
	if (UseTheoQuest && plr->_pLvlVisited[17]) {
		InitGirl();
	}
#endif
}

void FreeTownerGFX()
{
	int i;

	for (i = 0; i < NUM_TOWNERS; i++) {
		if (towner[i]._tNData == pCowCels) {
			towner[i]._tNData = NULL;
		} else if (towner[i]._tNData) {
			MemFreeDbg(towner[i]._tNData);
		}
	}

	MemFreeDbg(pCowCels);
}

void TownCtrlMsg(TownerStruct *tw)
{
	int pnum;
	int dx, dy;

	if (tw->_tbtcnt != 0) {
		pnum = tw->_tVar1;
		dx = abs(tw->_tx - plr[pnum]._px);
		dy = abs(tw->_ty - plr[pnum]._py);
		if (dx >= 2 || dy >= 2) {
			tw->_tbtcnt = 0;
			qtextflag = FALSE;
			stream_stop();
		}
	}
}

void ProcessTowners()
{
	TownerStruct *tw;
	int i, ao;

	tw = towner;
	for (i = 0; i < numtowners; i++, tw++) {
		TownCtrlMsg(tw);
		if (tw->_ttype == TOWN_DEADGUY) {
			if (quests[Q_BUTCHER]._qactive != QUEST_INIT) {
				if (quests[Q_BUTCHER]._qactive != QUEST_ACTIVE || quests[Q_BUTCHER]._qlog) {
					if (!qtextflag) {
						tw->_tAnimDelay = 1000;
						tw->_tAnimFrame = 1;
						strcpy(tw->_tName, "Slain Townsman");
					}
					tw->_tAnimCnt = 0;
				} else {
					if (qtextflag)
						tw->_tAnimCnt = 0;
				}
			}
		}

		tw->_tAnimCnt++;
		if (tw->_tAnimCnt >= tw->_tAnimDelay) {
			tw->_tAnimCnt = 0;

			if (tw->_tAnimOrder >= 0) {
				ao = tw->_tAnimOrder;
				tw->_tAnimFrameCnt++;
				if (AnimOrder[ao][tw->_tAnimFrameCnt] == -1)
					tw->_tAnimFrameCnt = 0;

				tw->_tAnimFrame = AnimOrder[ao][tw->_tAnimFrameCnt];
			} else {
				tw->_tAnimFrame++;
				if (tw->_tAnimFrame > tw->_tAnimLen)
					tw->_tAnimFrame = 1;
			}
		}
	}
}

ItemStruct *PlrHasItem(int pnum, int item, int *outidx)
{
	ItemStruct* pi;
	int i;

	pi = plr[pnum].InvList;
	for (i = 0; i < plr[pnum]._pNumInv; i++, pi++) {
		if (pi->IDidx == item) {
			*outidx = i;
			return pi;
		}
	}

	return NULL;
}

void TownerTalk(int talk)
{
	sgdwCowClicks = 0;
	sgnCowMsg = 0;
	storeflag = TRUE;
	InitQTextMsg(talk);
}

void TalkToTowner(int pnum, int tnum)
{
	TownerStruct *tw;
	int i, dx, dy, rv1, rv2, rv3;
	ItemStruct *is;
#ifdef HELLFIRE
	int qt, t2;
#endif

	tw = &towner[tnum];
	dx = abs(plr[pnum]._px - tw->_tx);
	dy = abs(plr[pnum]._py - tw->_ty);
#ifdef _DEBUG
	if (!debug_mode_key_d && (dx >= 2 || dy >= 2)) {
		return;
	}
#else
	if (dx >= 2 || dy >= 2) {
		return;
	}
#endif

	if (qtextflag) {
		return;
	}

	tw->_tMsgSaid = FALSE;

	if (pcurs >= CURSOR_FIRSTITEM && !DropItemBeforeTrig()) {
		return;
	}

	switch (tw->_ttype) {
	case TOWN_TAVERN:
		if (!plr[pnum]._pLvlVisited[0] && !tw->_tMsgSaid) {
			tw->_tbtcnt = 150;
			tw->_tVar1 = pnum;
			InitQTextMsg(TEXT_INTRO);
			tw->_tMsgSaid = TRUE;
		}
		if ((plr[pnum]._pLvlVisited[2] || plr[pnum]._pLvlVisited[4]) && quests[Q_SKELKING]._qactive != QUEST_NOTAVAIL) {
#ifdef HELLFIRE
			if (quests[Q_SKELKING]._qactive != QUEST_NOTAVAIL)
#endif
			if (quests[Q_SKELKING]._qvar2 == 0 && !tw->_tMsgSaid) {
				quests[Q_SKELKING]._qvar2 = 1;
				quests[Q_SKELKING]._qlog = TRUE;
				if (quests[Q_SKELKING]._qactive == QUEST_INIT) {
					quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
					quests[Q_SKELKING]._qvar1 = 1;
				}
				tw->_tbtcnt = 150;
				tw->_tVar1 = pnum;
				InitQTextMsg(TEXT_KING2);
				tw->_tMsgSaid = TRUE;
				NetSendCmdQuest(TRUE, Q_SKELKING);
			}
			if (quests[Q_SKELKING]._qactive == QUEST_DONE && quests[Q_SKELKING]._qvar2 == 1 && !tw->_tMsgSaid) {
				quests[Q_SKELKING]._qvar2 = 2;
				quests[Q_SKELKING]._qvar1 = 2;
				tw->_tbtcnt = 150;
				tw->_tVar1 = pnum;
				InitQTextMsg(TEXT_KING4);
				tw->_tMsgSaid = TRUE;
				NetSendCmdQuest(TRUE, Q_SKELKING);
			}
		}
		if (gbMaxPlayers == 1) {
			if (plr[pnum]._pLvlVisited[3] && quests[Q_LTBANNER]._qactive != QUEST_NOTAVAIL) {
			if ((quests[Q_LTBANNER]._qactive == QUEST_INIT || quests[Q_LTBANNER]._qactive == QUEST_ACTIVE) && quests[Q_LTBANNER]._qvar2 == 0 && !tw->_tMsgSaid) {
				quests[Q_LTBANNER]._qvar2 = 1;
				if (quests[Q_LTBANNER]._qactive == QUEST_INIT) {
					quests[Q_LTBANNER]._qvar1 = 1;
					quests[Q_LTBANNER]._qactive = QUEST_ACTIVE;
				}
				quests[Q_LTBANNER]._qlog = TRUE;
				tw->_tbtcnt = 150;
				tw->_tVar1 = pnum;
				InitQTextMsg(TEXT_BANNER2);
				tw->_tMsgSaid = TRUE;
			}
#ifdef HELLFIRE
			}
			if (!tw->_tMsgSaid && PlrHasItem(pnum, IDI_BANNER, &i) != NULL) {
#else
				if (quests[Q_LTBANNER]._qvar2 == 1 && PlrHasItem(pnum, IDI_BANNER, &i) != NULL && !tw->_tMsgSaid) {
#endif
					quests[Q_LTBANNER]._qactive = QUEST_DONE;
					quests[Q_LTBANNER]._qvar1 = 3;
					RemoveInvItem(pnum, i);
					CreateItem(UITEM_HARCREST, tw->_tx, tw->_ty + 1);
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					InitQTextMsg(TEXT_BANNER3);
					tw->_tMsgSaid = TRUE;
				}
			}
#ifndef HELLFIRE
				}
#endif
		if (!qtextflag) {
			TownerTalk(TEXT_OGDEN1);
			if (storeflag) {
				StartStore(STORE_TAVERN);
			}
		}
		break;
	case TOWN_DEADGUY:
		if (quests[Q_BUTCHER]._qactive == QUEST_ACTIVE && quests[Q_BUTCHER]._qvar1 == 1) {
			tw->_tbtcnt = 150;
			tw->_tVar1 = pnum;
			quests[Q_BUTCHER]._qvar1 = 1;
			if (plr[pnum]._pClass == PC_WARRIOR && !effect_is_playing(PS_WARR8)) {
				PlaySFX(PS_WARR8);
			} else if (plr[pnum]._pClass == PC_ROGUE && !effect_is_playing(PS_ROGUE8)) {
				PlaySFX(PS_ROGUE8);
			} else if (plr[pnum]._pClass == PC_SORCERER && !effect_is_playing(PS_MAGE8)) {
				PlaySFX(PS_MAGE8);
#ifdef HELLFIRE
			} else if (plr[pnum]._pClass == PC_MONK && !effect_is_playing(PS_MONK8)) {
				PlaySFX(PS_MONK8);
			} else if (plr[pnum]._pClass == PC_BARD && !effect_is_playing(PS_ROGUE8)) {
				PlaySFX(PS_ROGUE8);
			} else if (plr[pnum]._pClass == PC_BARBARIAN && !effect_is_playing(PS_WARR8)) {
				PlaySFX(PS_WARR8);
#endif
			}
			tw->_tMsgSaid = TRUE;
		} else if (quests[Q_BUTCHER]._qactive == QUEST_DONE && quests[Q_BUTCHER]._qvar1 == 1) {
			quests[Q_BUTCHER]._qvar1 = 1;
			tw->_tbtcnt = 150;
			tw->_tVar1 = pnum;
			tw->_tMsgSaid = TRUE;
		} else if (quests[Q_BUTCHER]._qactive == QUEST_INIT || quests[Q_BUTCHER]._qactive == QUEST_ACTIVE && quests[Q_BUTCHER]._qvar1 == 0) {
			quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
			quests[Q_BUTCHER]._qlog = TRUE;
			quests[Q_BUTCHER]._qmsg = TEXT_BUTCH9;
			quests[Q_BUTCHER]._qvar1 = 1;
			tw->_tbtcnt = 50;
			tw->_tVar1 = pnum;
			tw->_tVar2 = 3;
			InitQTextMsg(TEXT_BUTCH9);
			tw->_tMsgSaid = TRUE;
			NetSendCmdQuest(TRUE, Q_BUTCHER);
		}
		break;
	case TOWN_SMITH:
		if (gbMaxPlayers == 1) {
			if (plr[pnum]._pLvlVisited[4] && quests[Q_ROCK]._qactive != QUEST_NOTAVAIL) {
#ifdef HELLFIRE
				if (quests[Q_ROCK]._qactive != QUEST_NOTAVAIL)
#endif
				if (quests[Q_ROCK]._qvar2 == 0) {
					quests[Q_ROCK]._qvar2 = 1;
					quests[Q_ROCK]._qlog = TRUE;
					if (quests[Q_ROCK]._qactive == QUEST_INIT) {
						quests[Q_ROCK]._qactive = QUEST_ACTIVE;
						quests[Q_ROCK]._qvar1 = 1;
					}
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					InitQTextMsg(TEXT_INFRA5);
					tw->_tMsgSaid = TRUE;
				}
#ifdef HELLFIRE
			}
			if (!tw->_tMsgSaid && PlrHasItem(pnum, IDI_ROCK, &i) != NULL) {
#else
				if (quests[Q_ROCK]._qvar2 == 1 && PlrHasItem(pnum, IDI_ROCK, &i) != NULL && !tw->_tMsgSaid) {
#endif
						quests[Q_ROCK]._qactive = QUEST_DONE;
						quests[Q_ROCK]._qvar2 = 2;
						quests[Q_ROCK]._qvar1 = 2;
						RemoveInvItem(pnum, i);
						CreateItem(UITEM_INFRARING, tw->_tx, tw->_ty + 1);
						tw->_tbtcnt = 150;
						tw->_tVar1 = pnum;
						InitQTextMsg(TEXT_INFRA7);
						tw->_tMsgSaid = TRUE;
					}
#ifndef HELLFIRE
				}
#endif
			if (plr[pnum]._pLvlVisited[9] && quests[Q_ANVIL]._qactive != QUEST_NOTAVAIL) {
				if ((quests[Q_ANVIL]._qactive == QUEST_INIT || quests[Q_ANVIL]._qactive == QUEST_ACTIVE) && quests[Q_ANVIL]._qvar2 == 0 && !tw->_tMsgSaid) {
					if (quests[Q_ROCK]._qvar2 == 2 || quests[Q_ROCK]._qactive == QUEST_ACTIVE && quests[Q_ROCK]._qvar2 == 1) {
						quests[Q_ANVIL]._qvar2 = 1;
						quests[Q_ANVIL]._qlog = TRUE;
						if (quests[Q_ANVIL]._qactive == QUEST_INIT) {
							quests[Q_ANVIL]._qactive = QUEST_ACTIVE;
							quests[Q_ANVIL]._qvar1 = 1;
						}
						tw->_tbtcnt = 150;
						tw->_tVar1 = pnum;
						InitQTextMsg(TEXT_ANVIL5);
						tw->_tMsgSaid = TRUE;
					}
				}
#ifdef HELLFIRE
			}
			if (!tw->_tMsgSaid && PlrHasItem(pnum, IDI_ANVIL, &i) != NULL) {
#else
				if (quests[Q_ANVIL]._qvar2 == 1 && PlrHasItem(pnum, IDI_ANVIL, &i) != NULL) {
					if (!tw->_tMsgSaid) {
#endif
						quests[Q_ANVIL]._qactive = QUEST_DONE;
						quests[Q_ANVIL]._qvar2 = 2;
						quests[Q_ANVIL]._qvar1 = 2;
						RemoveInvItem(pnum, i);
						CreateItem(UITEM_GRISWOLD, tw->_tx, tw->_ty + 1);
						tw->_tbtcnt = 150;
						tw->_tVar1 = pnum;
						InitQTextMsg(TEXT_ANVIL7);
						tw->_tMsgSaid = TRUE;
#ifndef HELLFIRE
					}
				}
#endif
			}
		}
		if (!qtextflag) {
			TownerTalk(TEXT_GRISWOLD1);
			if (storeflag) {
				StartStore(STORE_SMITH);
			}
		}
		break;
	case TOWN_WITCH:
		if (quests[Q_MUSHROOM]._qactive == QUEST_INIT && PlrHasItem(pnum, IDI_FUNGALTM, &i) != NULL) {
			RemoveInvItem(pnum, i);
			quests[Q_MUSHROOM]._qactive = QUEST_ACTIVE;
			quests[Q_MUSHROOM]._qlog = TRUE;
			quests[Q_MUSHROOM]._qvar1 = QS_TOMEGIVEN;
			tw->_tbtcnt = 150;
			tw->_tVar1 = pnum;
			InitQTextMsg(TEXT_MUSH8);
			tw->_tMsgSaid = TRUE;
		} else if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE) {
			if (quests[Q_MUSHROOM]._qvar1 >= QS_TOMEGIVEN && quests[Q_MUSHROOM]._qvar1 <= QS_MUSHPICKED) {
				if (PlrHasItem(pnum, IDI_MUSHROOM, &i) != NULL) {
					RemoveInvItem(pnum, i);
					quests[Q_MUSHROOM]._qvar1 = 5;
					Qtalklist[TOWN_HEALER]._qblkm = TEXT_MUSH3;
					Qtalklist[TOWN_WITCH]._qblkm = -1;
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					quests[Q_MUSHROOM]._qmsg = TEXT_MUSH10;
					InitQTextMsg(TEXT_MUSH10);
					tw->_tMsgSaid = TRUE;
				} else if (quests[Q_MUSHROOM]._qmsg != TEXT_MUSH9) {
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					quests[Q_MUSHROOM]._qmsg = TEXT_MUSH9;
					InitQTextMsg(TEXT_MUSH9);
					tw->_tMsgSaid = TRUE;
				}
			} else {
				is = PlrHasItem(pnum, IDI_SPECELIX, &i);
				if (is != NULL) {
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					InitQTextMsg(TEXT_MUSH12);
					quests[Q_MUSHROOM]._qactive = QUEST_DONE;
					tw->_tMsgSaid = TRUE;
					AllItemsList[is->IDidx].iUsable = TRUE;
				} else if (PlrHasItem(pnum, IDI_BRAIN, &i) != NULL && quests[Q_MUSHROOM]._qvar2 != TEXT_MUSH11) {
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					quests[Q_MUSHROOM]._qvar2 = TEXT_MUSH11;
					InitQTextMsg(TEXT_MUSH11);
					tw->_tMsgSaid = TRUE;
				}
			}
		}
		if (!qtextflag) {
			TownerTalk(TEXT_ADRIA1);
			if (storeflag) {
				StartStore(STORE_WITCH);
			}
		}
		break;
	case TOWN_BMAID:
#ifdef HELLFIRE
		if (plr[pnum]._pLvlVisited[21] == false && PlrHasItem(pnum, IDI_MAPOFDOOM, &i)) {
			quests[Q_GRAVE]._qactive = 2;
			quests[Q_GRAVE]._qlog = 1;
			quests[Q_GRAVE]._qmsg = TEXT_GRAVE8;
			InitQTextMsg(TEXT_GRAVE8);
			tw->_tMsgSaid = TRUE;
		}
#endif
		if (!qtextflag) {
			TownerTalk(TEXT_GILLIAN1);
			if (storeflag) {
				StartStore(STORE_BARMAID);
			}
		}
		break;
	case TOWN_DRUNK:
		if (!qtextflag) {
			TownerTalk(TEXT_FARNHAM1);
			if (storeflag) {
				StartStore(STORE_DRUNK);
			}
		}
		break;
	case TOWN_HEALER:
		if (gbMaxPlayers == 1) {
#ifdef HELLFIRE
			if (plr[pnum]._pLvlVisited[1] || plr[pnum]._pLvlVisited[5]) {
#else
			if (plr[pnum]._pLvlVisited[1]) {
#endif
				if (!tw->_tMsgSaid) {
				if (quests[Q_PWATER]._qactive == QUEST_INIT) {
					quests[Q_PWATER]._qactive = QUEST_ACTIVE;
					quests[Q_PWATER]._qlog = TRUE;
					quests[Q_PWATER]._qmsg = TEXT_POISON3;
					quests[Q_PWATER]._qvar1 = 1;
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					InitQTextMsg(TEXT_POISON3);
					tw->_tMsgSaid = TRUE;
				} else if (quests[Q_PWATER]._qactive == QUEST_DONE && quests[Q_PWATER]._qvar1 != 2) {
					quests[Q_PWATER]._qvar1 = 2;
					tw->_tbtcnt = 150;
					tw->_tVar1 = pnum;
					InitQTextMsg(TEXT_POISON5);
					CreateItem(UITEM_TRING, tw->_tx, tw->_ty + 1);
					tw->_tMsgSaid = TRUE;
				}
			}
			}
			if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE && quests[Q_MUSHROOM]._qmsg == TEXT_MUSH10 && PlrHasItem(pnum, IDI_BRAIN, &i) != NULL) {
				RemoveInvItem(pnum, i);
				SpawnQuestItem(IDI_SPECELIX, tw->_tx, tw->_ty + 1, 0, 0);
				InitQTextMsg(TEXT_MUSH4);
				quests[Q_MUSHROOM]._qvar1 = QS_BRAINGIVEN;
				Qtalklist[TOWN_HEALER]._qblkm = -1;
			}
		}
		if (!qtextflag) {
			TownerTalk(TEXT_PEPIN1);
			if (storeflag) {
				StartStore(STORE_HEALER);
			}
		}
		break;
	case TOWN_PEGBOY:
		if (!qtextflag) {
			TownerTalk(TEXT_WIRT1);
			if (storeflag) {
				StartStore(STORE_BOY);
			}
		}
		break;
	case TOWN_STORY:
		if (gbMaxPlayers == 1) {
			if (quests[Q_BETRAYER]._qactive == QUEST_INIT && PlrHasItem(pnum, IDI_LAZSTAFF, &i) != NULL) {
				RemoveInvItem(pnum, i);
				quests[Q_BETRAYER]._qvar1 = 2;
				tw->_tbtcnt = 150;
				tw->_tVar1 = pnum;
				InitQTextMsg(TEXT_VILE1);
				tw->_tMsgSaid = TRUE;
				quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
				quests[Q_BETRAYER]._qlog = TRUE;
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == 7) {
				quests[Q_BETRAYER]._qvar1 = 8;
				tw->_tbtcnt = 150;
				tw->_tVar1 = pnum;
				InitQTextMsg(TEXT_VILE3);
				tw->_tMsgSaid = TRUE;
				quests[Q_DIABLO]._qlog = TRUE;
			}
		}
		if (gbMaxPlayers != 1) {
			if (quests[Q_BETRAYER]._qactive == QUEST_ACTIVE && !quests[Q_BETRAYER]._qlog) {
				tw->_tbtcnt = 150;
				tw->_tVar1 = pnum;
				InitQTextMsg(TEXT_VILE1);
				tw->_tMsgSaid = TRUE;
				quests[Q_BETRAYER]._qlog = TRUE;
				NetSendCmdQuest(TRUE, Q_BETRAYER);
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == 7) {
				quests[Q_BETRAYER]._qvar1 = 8;
				tw->_tbtcnt = 150;
				tw->_tVar1 = pnum;
				InitQTextMsg(TEXT_VILE3);
				tw->_tMsgSaid = TRUE;
				NetSendCmdQuest(TRUE, Q_BETRAYER);
				quests[Q_DIABLO]._qlog = TRUE;
				NetSendCmdQuest(TRUE, Q_DIABLO);
			}
		}
		if (!qtextflag) {
			TownerTalk(TEXT_STORY1);
			if (storeflag) {
				StartStore(STORE_STORY);
			}
		}
		break;
	case TOWN_COW:
		if (!qtextflag)
			CowSFX(pnum);
		break;
#ifdef HELLFIRE
	case TOWN_FARMER:
		if (!qtextflag) {
			qt = 277;
			t2 = 1;
			switch (quests[Q_FARMER]._qactive) {
			case 0:
				if (PlrHasItem(pnum, IDI_RUNEBOMB, &i)) {
					qt = TEXT_FARMER2;
					quests[Q_FARMER]._qactive = 2;
					quests[Q_FARMER]._qvar1 = 1;
					quests[Q_FARMER]._qlog = 1;
					quests[Q_FARMER]._qmsg = TEXT_FARMER1;
					break;
				} else if (!plr[myplr]._pLvlVisited[9] && plr[myplr]._pLevel < 15) {
					qt = 309;
					if (plr[myplr]._pLvlVisited[2])
						qt = 281;
					if (plr[myplr]._pLvlVisited[5])
						qt = 308;
					if (plr[myplr]._pLvlVisited[7])
						qt = 310;
				} else {
					qt = TEXT_FARMER1;
					quests[Q_FARMER]._qactive = 2;
					quests[Q_FARMER]._qvar1 = 1;
					quests[Q_FARMER]._qlog = 1;
					quests[Q_FARMER]._qmsg = TEXT_FARMER1;
					SpawnRuneBomb(tw->_tx + 1, tw->_ty);
					t2 = 1;
					break;
				}
			case 2:
				if (PlrHasItem(pnum, IDI_RUNEBOMB, &i))
					qt = TEXT_FARMER2;
				else
					qt = TEXT_FARMER3;
				break;
			case 1:
				if (PlrHasItem(pnum, IDI_RUNEBOMB, &i)) {
					qt = TEXT_FARMER2;
					quests[Q_FARMER]._qactive = 2;
					quests[Q_FARMER]._qvar1 = 1;
					quests[Q_FARMER]._qmsg = TEXT_FARMER1;
					quests[Q_FARMER]._qlog = 1;
				} else if (!plr[myplr]._pLvlVisited[9] && plr[myplr]._pLevel < 15) {
					qt = TEXT_FARMER8;
					if (plr[myplr]._pLvlVisited[2]) {
						qt = TEXT_FARMER5;
					}
					if (plr[myplr]._pLvlVisited[5]) {
						qt = TEXT_FARMER7;
					}
					if (plr[myplr]._pLvlVisited[7]) {
						qt = TEXT_FARMER9;
					}
				} else {
					qt = TEXT_FARMER1;
					quests[Q_FARMER]._qactive = 2;
					quests[Q_FARMER]._qvar1 = 1;
					quests[Q_FARMER]._qlog = 1;
					quests[Q_FARMER]._qmsg = TEXT_FARMER1;
					SpawnRuneBomb(tw->_tx + 1, tw->_ty);
					t2 = 1;
				}
				break;
			case 3:
				qt = TEXT_FARMER4;
				SpawnRewardItem(IDI_AURIC, tw->_tx + 1, tw->_ty);
				quests[Q_FARMER]._qactive = 10;
				quests[Q_FARMER]._qlog = 0;
				t2 = 1;
				break;
			case 10:
				qt = -1;
				break;
			default:
				quests[Q_FARMER]._qactive = 0;
				qt = TEXT_FARMER4;
				break;
			}
			if (qt != -1) {
				if (t2)
				InitQTextMsg(qt);
				else
					PlaySFX(alltext[qt].sfxnr);
			}
			if (gbMaxPlayers != 1) {
				NetSendCmdQuest(TRUE, Q_FARMER);
			}
		}
		break;
	case TOWN_COWFARM:
		if (!qtextflag) {
			qt = 297;
			t2 = 1;
			if (PlrHasItem(pnum, IDI_GREYSUIT, &i)) {
				qt = TEXT_JERSEY7;
				RemoveInvItem(pnum, i);
			} else if (PlrHasItem(pnum, IDI_BROWNSUIT, &i)) {
				CreateItem(UITEM_BOVINE, tw->_tx + 1, tw->_ty);
				RemoveInvItem(pnum, i);
				qt = TEXT_JERSEY8;
				quests[Q_JERSEY]._qactive = 3;
			} else if (PlrHasItem(pnum, IDI_RUNEBOMB, &i)) {
				qt = TEXT_JERSEY5;
				quests[Q_JERSEY]._qactive = 2;
				quests[Q_JERSEY]._qvar1 = 1;
				quests[Q_JERSEY]._qmsg = TEXT_JERSEY4;
				quests[Q_JERSEY]._qlog = 1;
			} else {
				switch (quests[Q_JERSEY]._qactive) {
				case 0:
					qt = TEXT_JERSEY1;
					quests[Q_JERSEY]._qactive = 7;
					break;
				case 1:
					qt = TEXT_JERSEY1;
					quests[23]._qactive = 7;
					break;
				case 2:
					qt = TEXT_JERSEY5;
					break;
				case 3:
					qt = TEXT_JERSEY1;
					break;
				case 7:
					qt = TEXT_JERSEY2;
					quests[Q_JERSEY]._qactive = 8;
					break;
				case 8:
					qt = TEXT_JERSEY3;
					quests[Q_JERSEY]._qactive = 9;
					break;
				case 9:
					if (!plr[myplr]._pLvlVisited[9] && plr[myplr]._pLevel < 15) {
						switch (random_(0, 4) + 9) {
						case 9:
							qt = TEXT_JERSEY9;
							break;
						case 10:
							qt = TEXT_JERSEY10;
							break;
						case 11:
							qt = TEXT_JERSEY11;
							break;
						default:
							qt = TEXT_JERSEY12;
						}
							break;
					} else {
						qt = TEXT_JERSEY4;
						quests[Q_JERSEY]._qactive = 2;
						quests[Q_JERSEY]._qvar1 = 1;
						quests[Q_JERSEY]._qmsg = TEXT_JERSEY4;
						quests[Q_JERSEY]._qlog = 1;
						SpawnRuneBomb(tw->_tx + 1, tw->_ty);
						t2 = 1;
					}
					break;
				default:
					qt = TEXT_JERSEY5;
					quests[Q_JERSEY]._qactive = 0;
					break;
				}
			}
			if (qt != -1) {
				if (t2)
				InitQTextMsg(qt);
				else
					PlaySFX(alltext[qt].sfxnr);
			}
			if (gbMaxPlayers != 1) {
				NetSendCmdQuest(TRUE, Q_JERSEY);
			}
		}
		break;
	case TOWN_GIRL:
		if (!qtextflag) {
			qt = 282;
			t2 = 0;
			if (!PlrHasItem(pnum, IDI_THEODORE, &i) || quests[Q_GIRL]._qactive == 3) {
				switch (quests[Q_GIRL]._qactive) {
				case 0:
					qt = TEXT_GIRL2;
					quests[Q_GIRL]._qactive = 2;
					quests[Q_GIRL]._qvar1 = 1;
					quests[Q_GIRL]._qlog = 1;
					quests[Q_GIRL]._qmsg = TEXT_GIRL2;
					t2 = 1;
					break;
				case 1:
					qt = TEXT_GIRL2;
					quests[Q_GIRL]._qvar1 = 1;
					quests[Q_GIRL]._qlog = 1;
					quests[Q_GIRL]._qmsg = TEXT_GIRL2;
					quests[Q_GIRL]._qactive = 2;
					t2 = 1;
					break;
				case 2:
					qt = TEXT_GIRL3;
					t2 = 1;
					break;
				case 3:
					qt = -1;
					break;
				default:
					quests[Q_GIRL]._qactive = 0;
					qt = TEXT_GIRL1;
					break;
				}
			} else {
				qt = TEXT_GIRL4;
				RemoveInvItem(pnum, i);
				CreateAmulet(tw->_tx, tw->_ty, 13, 0, 1);
				quests[Q_GIRL]._qlog = 0;
				quests[Q_GIRL]._qactive = 3;
				t2 = 1;
			}
			if (qt != -1) {
				if (t2 != 0) {
					InitQTextMsg(qt);
				} else {
					PlaySFX(alltext[qt].sfxnr);
				}
			}
			if (gbMaxPlayers != 1) {
				NetSendCmdQuest(TRUE, Q_GIRL);
			}
		}
		break;
#endif
	}
}

void CowSFX(int pnum)
{
	if (CowPlaying != -1 && effect_is_playing(CowPlaying))
		return;

	sgdwCowClicks++;

#ifdef SPAWN
	if (sgdwCowClicks == 4) {
		sgdwCowClicks = 0;
		CowPlaying = TSFX_COW2;
	} else {
		CowPlaying = TSFX_COW1;
	}
#else
	if (sgdwCowClicks >= 8) {
		PlaySfxLoc(TSFX_COW1, plr[pnum]._px, plr[pnum]._py + 5);
		sgdwCowClicks = 4;
		CowPlaying = snSFX[sgnCowMsg][plr[pnum]._pClass]; /* snSFX is local */
		sgnCowMsg++;
		if (sgnCowMsg >= 3)
			sgnCowMsg = 0;
	} else {
		CowPlaying = sgdwCowClicks == 4 ? TSFX_COW2 : TSFX_COW1;
	}
#endif

	PlaySfxLoc(CowPlaying, plr[pnum]._px, plr[pnum]._py);
}

DEVILUTION_END_NAMESPACE
