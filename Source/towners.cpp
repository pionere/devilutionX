/**
 * @file towners.cpp
 *
 * Implementation of functionality for loading and spawning towners.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

unsigned _guCowMsg;
int numtowners;
unsigned _guCowClicks;
BYTE *pCowCels;
TownerStruct towner[MAX_TOWNERS];

/**
 * Maps from active cow sound effect index and player class to sound
 * effect ID for interacting with cows in Tristram.
 *
 * ref: enum _sfx_id
 * ref: enum plr_class
 */
const int snSFX[3][NUM_CLASSES] = {
#ifdef HELLFIRE
	{ PS_WARR52, PS_ROGUE52, PS_MAGE52, PS_MONK52, PS_ROGUE52, PS_WARR52 },
	{ PS_WARR49, PS_ROGUE49, PS_MAGE49, PS_MONK49, PS_ROGUE49, PS_WARR49 },
	{ PS_WARR50, PS_ROGUE50, PS_MAGE50, PS_MONK50, PS_ROGUE50, PS_WARR50 },
#else
	{ PS_WARR52, PS_ROGUE52, PS_MAGE52 },
	{ PS_WARR49, PS_ROGUE49, PS_MAGE49 },
	{ PS_WARR50, PS_ROGUE50, PS_MAGE50 },
#endif
};

/* data */
/** Specifies the animation frame sequence of a given NPC. */
char AnimOrder[6][144] = {
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
int TownCowX[] = { 48 + DBORDERX, 46 + DBORDERX, 49 + DBORDERX };
/** Specifies the start Y-coordinates of the cows in Tristram. */
int TownCowY[] = {  6 + DBORDERY,  4 + DBORDERY, 10 + DBORDERY };
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
/** Specifies the active sound effect ID for interacting with cows. */
int CowPlaying = -1;

static void CowSFX(int pnum)
{
	PlayerStruct *p;
	if (CowPlaying != -1 && effect_is_playing(CowPlaying))
		return;

	_guCowClicks++;

	p = &plr[pnum];
	if (_guCowClicks >= 8) {
		PlaySfxLoc(TSFX_COW1, p->_px, p->_py + 5);
		_guCowClicks = 4;
		CowPlaying = snSFX[_guCowMsg][p->_pClass]; /* snSFX is local */
		_guCowMsg++;
		if (_guCowMsg >= 3)
			_guCowMsg = 0;
	} else {
		CowPlaying = _guCowClicks == 4 ? TSFX_COW2 : TSFX_COW1;
	}

	PlaySfxLoc(CowPlaying, p->_px, p->_py);
}

static void InitCowAnim(int tnum, int dir)
{
	TownerStruct *tw;
	int i;

	tw = &towner[tnum];
	tw->_tNData = pCowCels;
	for (i = 0; i < lengthof(tw->_tNAnim); i++) {
		tw->_tNAnim[i] = CelGetFrameStart(pCowCels, i);
	}
	tw->_tNFrames = 12;

	tw->_tAnimData = tw->_tNAnim[dir];
	tw->_tAnimLen = 12;
	tw->_tAnimCnt = 0;
	tw->_tAnimDelay = 3;
	tw->_tAnimFrame = RandRange(1, 11);
}

static void InitTownerAnim(int tnum, BYTE *pAnim, int numFrames, int Delay)
{
	TownerStruct *tw;
	int i;

	tw = &towner[tnum];
	tw->_tNData = pAnim;
	for (i = 0; i < lengthof(tw->_tNAnim); i++) {
		tw->_tNAnim[i] = pAnim;
	}
	tw->_tNFrames = numFrames;

	tw->_tAnimData = pAnim;
	tw->_tAnimLen = numFrames;
	tw->_tAnimCnt = 0;
	tw->_tAnimDelay = Delay;
	tw->_tAnimFrame = 1;
}

static void InitTownerInfo(int tnum, int w, int type, int x, int y, int ao)
{
	TownerStruct *tw;

	dMonster[x][y] = tnum + 1;
	monster[tnum]._mfutx = monster[tnum]._mx = x;
	monster[tnum]._mfuty = monster[tnum]._my = y;
	tw = &towner[tnum];
	memset(tw, 0, sizeof(TownerStruct));
	tw->_tSelFlag = TRUE;
	tw->_tAnimWidth = w;
	tw->_tAnimWidth2 = (w - 64) >> 1;
	tw->_ttype = type;
	tw->_tx = x;
	tw->_ty = y;
	tw->_tAnimOrder = ao;
	tw->_tSeed = GetRndSeed();
}

/**
 * @brief Load Griswold into the game

 */
static void InitSmith()
{
	InitTownerInfo(numtowners, 96, TOWN_SMITH, 52 + DBORDERX, 53 + DBORDERY, 0);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\Smith\\SmithN.CEL", NULL), 16, 3);
	towner[numtowners]._tName = "Griswold the Blacksmith";
	numtowners++;
}

static void InitBarOwner()
{
	InitTownerInfo(numtowners, 96, TOWN_TAVERN, 45 + DBORDERX, 52 + DBORDERY, 3);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\TwnF\\TwnFN.CEL", NULL), 16, 3);
	towner[numtowners]._tName = "Ogden the Tavern owner";
	numtowners++;
}

static void InitTownDead()
{
	InitTownerInfo(numtowners, 96, TOWN_DEADGUY, 14 + DBORDERX, 22 + DBORDERY, -1);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\Butch\\Deadguy.CEL", NULL), 8, 6);
	towner[numtowners]._tName = "Wounded Townsman";
	numtowners++;
}

static void InitWitch()
{
	InitTownerInfo(numtowners, 96, TOWN_WITCH, 70 + DBORDERX, 10 + DBORDERY, 5);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\TownWmn1\\Witch.CEL", NULL), 19, 6);
	towner[numtowners]._tName = "Adria the Witch";
	numtowners++;
}

static void InitBarmaid()
{
	InitTownerInfo(numtowners, 96, TOWN_BMAID, 33 + DBORDERX, 56 + DBORDERY, -1);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\TownWmn1\\WmnN.CEL", NULL), 18, 6);
	towner[numtowners]._tName = "Gillian the Barmaid";
	numtowners++;
}

static void InitBoy()
{
	InitTownerInfo(numtowners, 96, TOWN_PEGBOY, 1 + DBORDERX, 43 + DBORDERY, -1);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\TownBoy\\PegKid1.CEL", NULL), 20, 6);
	towner[numtowners]._tName = "Wirt the Peg-legged boy";
	numtowners++;
}

static void InitHealer()
{
	InitTownerInfo(numtowners, 96, TOWN_HEALER, 45 + DBORDERX, 69 + DBORDERY, 1);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\Healer\\Healer.CEL", NULL), 20, 6);
	towner[numtowners]._tName = "Pepin the Healer";
	numtowners++;
}

static void InitTeller()
{
	InitTownerInfo(numtowners, 96, TOWN_STORY, 52 + DBORDERX, 61 + DBORDERY, 2);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\Strytell\\Strytell.CEL", NULL), 25, 3);
	towner[numtowners]._tName = "Cain the Elder";
	numtowners++;
}

static void InitDrunk()
{
	InitTownerInfo(numtowners, 96, TOWN_DRUNK, 61 + DBORDERX, 74 + DBORDERY, 4);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\Drunk\\TwnDrunk.CEL", NULL), 18, 3);
	towner[numtowners]._tName = "Farnham the Drunk";
	numtowners++;
}

static void InitCows()
{
	int i, dir;
	int x, y, xo, yo;

	pCowCels = LoadFileInMem("Towners\\Animals\\Cow.CEL", NULL);
	static_assert(lengthof(TownCowX) == lengthof(TownCowY), "Mismatching TownCow tables I.");
	static_assert(lengthof(TownCowX) == lengthof(TownCowDir), "Mismatching TownCow tables II.");
	for (i = 0; i < lengthof(TownCowX); i++) {
		x = TownCowX[i];
		y = TownCowY[i];
		dir = TownCowDir[i];
		InitTownerInfo(numtowners, 128, TOWN_COW, x, y, -1);
		InitCowAnim(numtowners, dir);
		towner[numtowners]._tName = "Cow";

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
static void InitFarmer()
{
	InitTownerInfo(numtowners, 96, TOWN_FARMER, 52 + DBORDERX, 6 + DBORDERY, -1);
	InitTownerAnim(numtowners, LoadFileInMem("Towners\\Farmer\\Farmrn2.CEL", NULL), 15, 3);
	towner[numtowners]._tName = "Lester the farmer";
	numtowners++;
}

static void InitCowFarmer()
{
	BYTE *pBuf;

	InitTownerInfo(numtowners, 96, TOWN_COWFARM, 51 + DBORDERX, 12 + DBORDERY, -1);
	if (quests[Q_JERSEY]._qactive != QUEST_DONE) {
		pBuf = LoadFileInMem("Towners\\Farmer\\cfrmrn2.CEL", NULL);
	} else {
		pBuf = LoadFileInMem("Towners\\Farmer\\mfrmrn2.CEL", NULL);
	}
	InitTownerAnim(numtowners, pBuf, 15, 3);
	towner[numtowners]._tName = "Complete Nut";
	numtowners++;
}

static void InitGirl()
{
	BYTE *pBuf;

	InitTownerInfo(numtowners, 96, TOWN_GIRL, 67 + DBORDERX, 33 + DBORDERY, -1);
	if (quests[Q_GIRL]._qactive == QUEST_ACTIVE) {
		pBuf = LoadFileInMem("Towners\\Girl\\Girlw1.CEL", NULL);
	} else {
		pBuf = LoadFileInMem("Towners\\Girl\\Girls1.CEL", NULL);
	}
	InitTownerAnim(numtowners, pBuf, 20, 6);
	towner[numtowners]._tName = "Celia";
	numtowners++;
}
#endif

void InitTowners()
{
	numtowners = 0;
	InitSmith();
	InitHealer();
	InitBarOwner();
	InitTeller();
	InitDrunk();
	InitWitch();
	InitBarmaid();
	InitBoy();
	if (quests[Q_BUTCHER]._qactive != QUEST_NOTAVAIL && quests[Q_BUTCHER]._qactive != QUEST_DONE)
		InitTownDead();
#ifdef HELLFIRE
	if (gbUseCowFarmer) {
		InitCowFarmer();
	} else { // if (quests[Q_FARMER]._qactive != QUEST_DONE || quests[Q_FARMER]._qlog) {
		InitFarmer(); // in vanilla hellfire the farmer was gone after the quest is completed, but there is no reason for that
	}
	if (gbUseTheoQuest && quests[Q_GIRL]._qactive != QUEST_NOTAVAIL) {
		InitGirl();
	}
#endif
	InitCows();
}

void FreeTownerGFX()
{
	int i;

	for (i = 0; i < numtowners; i++) {
		if (towner[i]._tNData == pCowCels) {
			towner[i]._tNData = NULL;
		} else if (towner[i]._tNData != NULL) {
			MemFreeDbg(towner[i]._tNData);
		}
	}

	MemFreeDbg(pCowCels);
}

static void TownCtrlMsg(TownerStruct *tw)
{
	PlayerStruct *p;
	int dx, dy;

	if (tw->_tListener != 0) {
		p = &plr[tw->_tListener - 1];
		dx = abs(tw->_tx - p->_px);
		dy = abs(tw->_ty - p->_py);
		if (dx >= 2 || dy >= 2) {
			tw->_tListener = 0;
			gbQtextflag = false;
			stream_stop();
		}
	}
}

void ProcessTowners()
{
	TownerStruct *tw;
	int i, ao;

	tw = towner;
	for (i = numtowners; i > 0; i--, tw++) {
		TownCtrlMsg(tw);
		if (tw->_ttype == TOWN_DEADGUY) {
			if (quests[Q_BUTCHER]._qactive != QUEST_INIT) {
				if (quests[Q_BUTCHER]._qactive != QUEST_ACTIVE || quests[Q_BUTCHER]._qlog) {
					if (!gbQtextflag) {
						tw->_tAnimDelay = 1000;
						tw->_tAnimFrame = 1;
						tw->_tName = "Slain Townsman";
					}
					tw->_tAnimCnt = 0;
				} else {
					if (gbQtextflag)
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

bool PlrHasItem(int pnum, int item, int *outidx)
{
	ItemStruct* pi;
	int i;

	pi = plr[pnum].InvList;
	for (i = 0; i < plr[pnum]._pNumInv; i++, pi++) {
		if (pi->_iIdx == item) {
			*outidx = i;
			return true;
		}
	}

	return false;
}

static bool PlrHasBeltItem(int pnum, int item)
{
	ItemStruct* pi;
	int i;

	pi = plr[pnum].SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++) {
		if (pi->_iIdx == item) {
			return true;
		}
	}

	return false;
}

static void TownerTalk(int store, int talk)
{
	if (gbQtextflag)
		return;

	_guCowClicks = 0;
	_guCowMsg = 0;
	gbSbookflag = false;
	gbInvflag = false;
	gbChrflag = false;
	gbQuestlog = false;
	gbDropGoldFlag = false; // should not be necessary, but ...
	InitQTextMsg(talk);
	StartStore(store);
}

void TalkToTowner(int pnum, int tnum)
{
	TownerStruct *tw;
	int i, dx, dy, qt, qn = Q_INVALID;
	bool msgSaid;

	tw = &towner[tnum];
	dx = abs(plr[pnum]._px - tw->_tx);
	dy = abs(plr[pnum]._py - tw->_ty);
	if (dx >= 2 || dy >= 2)
#ifdef _DEBUG
		if (!debug_mode_key_d)
#endif
			return;

	if (gbQtextflag) {
		return;
	}

	msgSaid = false;

	if (pcurs >= CURSOR_FIRSTITEM) {
		return;
	}

	switch (tw->_ttype) {
	case TOWN_TAVERN:
		qt = TEXT_NONE;
		if (!plr[pnum]._pLvlVisited[DLV_TOWN] && !msgSaid) {
			qt = TEXT_INTRO;
			msgSaid = true;
		}
		if (plr[pnum]._pLvlVisited[DLV_CATHEDRAL2] && quests[Q_SKELKING]._qactive != QUEST_NOTAVAIL) {
			if (quests[Q_SKELKING]._qactive == QUEST_INIT && !msgSaid) {
				quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
				quests[Q_SKELKING]._qlog = TRUE;
				qn = Q_SKELKING;
				qt = TEXT_KING2;
				msgSaid = true;
			}
			if (quests[Q_SKELKING]._qactive == QUEST_DONE && quests[Q_SKELKING]._qlog && !msgSaid) {
				quests[Q_SKELKING]._qlog = FALSE;
				// qn = Q_SKELKING; - let the others hear the final message
				qt = TEXT_KING4;
				msgSaid = true;
			}
		}
		if (plr[pnum]._pLvlVisited[DLV_CATHEDRAL3] && quests[Q_LTBANNER]._qactive != QUEST_NOTAVAIL) {
			if (quests[Q_LTBANNER]._qactive == QUEST_INIT && !msgSaid) {
				quests[Q_LTBANNER]._qactive = QUEST_ACTIVE;
				quests[Q_LTBANNER]._qlog = TRUE;
				qn = Q_LTBANNER;
				qt = TEXT_BANNER2;
				msgSaid = true;
			}
			if (quests[Q_LTBANNER]._qactive == QUEST_ACTIVE && PlrHasItem(pnum, IDI_BANNER, &i) && !msgSaid) {
				RemoveInvItem(pnum, i);
				SpawnUnique(UITEM_HARCREST, tw->_tx, tw->_ty + 1, true, true);
				quests[Q_LTBANNER]._qlog = FALSE;
				quests[Q_LTBANNER]._qvar1 = 2;
				qn = Q_LTBANNER;
				qt = TEXT_BANNER3;
				msgSaid = true;
			}
		}
		if (qt != TEXT_NONE) {
			tw->_tListener = pnum + 1;
			InitQTextMsg(qt);
		}
		TownerTalk(STORE_TAVERN, TEXT_OGDEN1);
		break;
	case TOWN_DEADGUY:
		if (quests[Q_BUTCHER]._qactive == QUEST_ACTIVE && quests[Q_BUTCHER]._qvar1 == 1) {
			tw->_tListener = pnum + 1;
			quests[Q_BUTCHER]._qvar1 = 1;
			i = sgSFXSets[SFXS_PLR_08][plr[pnum]._pClass];
			if (!effect_is_playing(i))
				PlaySFX(i);
			msgSaid = true;
		} else if (quests[Q_BUTCHER]._qactive == QUEST_DONE && quests[Q_BUTCHER]._qvar1 == 1) {
			quests[Q_BUTCHER]._qvar1 = 1;
			tw->_tListener = pnum + 1;
			msgSaid = true;
		} else if (quests[Q_BUTCHER]._qactive == QUEST_INIT || (quests[Q_BUTCHER]._qactive == QUEST_ACTIVE && quests[Q_BUTCHER]._qvar1 == 0)) {
			quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
			quests[Q_BUTCHER]._qlog = TRUE;
			quests[Q_BUTCHER]._qmsg = TEXT_BUTCH9;
			quests[Q_BUTCHER]._qvar1 = 1;
			NetSendCmdQuest(true, Q_BUTCHER, false);
			tw->_tListener = pnum + 1;
			InitQTextMsg(TEXT_BUTCH9);
			msgSaid = true;
		}
		break;
	case TOWN_SMITH:
		qt = TEXT_NONE;
		if (plr[pnum]._pLvlVisited[DLV_CATHEDRAL4] && quests[Q_ROCK]._qactive != QUEST_NOTAVAIL) {
			if (quests[Q_ROCK]._qvar1 <= 1) {
				quests[Q_ROCK]._qvar1 = 2;
				quests[Q_ROCK]._qactive = QUEST_ACTIVE;
				quests[Q_ROCK]._qlog = TRUE;
				qn = Q_ROCK;
				qt = TEXT_INFRA5;
				msgSaid = true;
			}
			if (quests[Q_ROCK]._qactive != QUEST_DONE && PlrHasItem(pnum, IDI_ROCK, &i) && !msgSaid) {
				RemoveInvItem(pnum, i);
				SpawnUnique(UITEM_INFRARING, tw->_tx, tw->_ty + 1, true, true);
				quests[Q_ROCK]._qlog = FALSE;
				qn = Q_ROCK;
				qt = TEXT_INFRA7;
				msgSaid = true;
			}
		}
		if (plr[pnum]._pLvlVisited[DLV_CAVES1] && quests[Q_ANVIL]._qactive != QUEST_NOTAVAIL) {
			if (quests[Q_ANVIL]._qvar1 <= 1 && !msgSaid) {
				quests[Q_ANVIL]._qvar1 = 2;
				quests[Q_ANVIL]._qactive = QUEST_ACTIVE;
				quests[Q_ANVIL]._qlog = TRUE;
				qn = Q_ANVIL;
				qt = TEXT_ANVIL5;
				msgSaid = true;
			}
			if (quests[Q_ANVIL]._qactive != QUEST_DONE && PlrHasItem(pnum, IDI_ANVIL, &i) && !msgSaid) {
				RemoveInvItem(pnum, i);
				SpawnUnique(UITEM_GRISWOLD, tw->_tx, tw->_ty + 1, true, true);
				quests[Q_ANVIL]._qactive = QUEST_DONE;
				quests[Q_ANVIL]._qlog = FALSE;
				qt = TEXT_ANVIL7;
				msgSaid = true;
			}
		}
		if (qt != TEXT_NONE) {
			tw->_tListener = pnum + 1;
			InitQTextMsg(qt);
		}
		TownerTalk(STORE_SMITH, TEXT_GRISWOLD1);
		break;
	case TOWN_WITCH:
		if (quests[Q_MUSHROOM]._qactive == QUEST_INIT && PlrHasItem(pnum, IDI_FUNGALTM, &i)) {
			RemoveInvItem(pnum, i);
			quests[Q_MUSHROOM]._qactive = QUEST_ACTIVE;
			quests[Q_MUSHROOM]._qlog = TRUE;
			quests[Q_MUSHROOM]._qvar1 = QS_TOMEGIVEN;
			tw->_tListener = pnum + 1;
			InitQTextMsg(TEXT_MUSH8);
			msgSaid = true;
		} else if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE) {
			if (quests[Q_MUSHROOM]._qvar1 >= QS_TOMEGIVEN && quests[Q_MUSHROOM]._qvar1 <= QS_MUSHPICKED) {
				if (PlrHasItem(pnum, IDI_MUSHROOM, &i)) {
					RemoveInvItem(pnum, i);
					quests[Q_MUSHROOM]._qvar1 = 5;
					Qtalklist[TOWN_HEALER][Q_MUSHROOM] = TEXT_MUSH3;
					Qtalklist[TOWN_WITCH][Q_MUSHROOM] = TEXT_NONE;
					quests[Q_MUSHROOM]._qmsg = TEXT_MUSH10;
					tw->_tListener = pnum + 1;
					InitQTextMsg(TEXT_MUSH10);
					msgSaid = true;
				} else if (quests[Q_MUSHROOM]._qmsg != TEXT_MUSH9) {
					quests[Q_MUSHROOM]._qmsg = TEXT_MUSH9;
					tw->_tListener = pnum + 1;
					InitQTextMsg(TEXT_MUSH9);
					msgSaid = true;
				}
			} else {
				if (PlrHasItem(pnum, IDI_SPECELIX, &i) || PlrHasBeltItem(pnum, IDI_SPECELIX)) {
					quests[Q_MUSHROOM]._qactive = QUEST_DONE;
					tw->_tListener = pnum + 1;
					InitQTextMsg(TEXT_MUSH12);
					msgSaid = true;
				} else if (PlrHasItem(pnum, IDI_BRAIN, &i) && quests[Q_MUSHROOM]._qvar2 != TEXT_MUSH11) {
					quests[Q_MUSHROOM]._qvar2 = TEXT_MUSH11;
					tw->_tListener = pnum + 1;
					InitQTextMsg(TEXT_MUSH11);
					msgSaid = true;
				}
			}
		}
		TownerTalk(STORE_WITCH, TEXT_ADRIA1);
		break;
	case TOWN_BMAID:
#ifdef HELLFIRE
		if (!plr[pnum]._pLvlVisited[DLV_CRYPT1] && PlrHasItem(pnum, IDI_MAPOFDOOM, &i)) {
			quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
			quests[Q_GRAVE]._qlog = TRUE;
			quests[Q_GRAVE]._qmsg = TEXT_GRAVE8;
			qn = Q_GRAVE;
			InitQTextMsg(TEXT_GRAVE8);
			msgSaid = true;
		}
#endif
		TownerTalk(STORE_BARMAID, TEXT_GILLIAN1);
		break;
	case TOWN_DRUNK:
		TownerTalk(STORE_DRUNK, TEXT_FARNHAM1);
		break;
	case TOWN_HEALER:
		if (gbMaxPlayers == 1) {
#ifdef HELLFIRE
			if (plr[pnum]._pLvlVisited[1] || plr[pnum]._pLvlVisited[5]) {
#else
			if (plr[pnum]._pLvlVisited[1]) {
#endif
				if (!msgSaid) {
				if (quests[Q_PWATER]._qactive == QUEST_INIT) {
					quests[Q_PWATER]._qactive = QUEST_ACTIVE;
					quests[Q_PWATER]._qlog = TRUE;
					quests[Q_PWATER]._qmsg = TEXT_POISON3;
					quests[Q_PWATER]._qvar1 = 1;
					tw->_tListener = pnum + 1;
					InitQTextMsg(TEXT_POISON3);
					msgSaid = true;
				} else if (quests[Q_PWATER]._qactive == QUEST_DONE && quests[Q_PWATER]._qvar1 != 2) {
					quests[Q_PWATER]._qvar1 = 2;
					SpawnUnique(UITEM_TRING, tw->_tx, tw->_ty + 1, true, true);
					tw->_tListener = pnum + 1;
					InitQTextMsg(TEXT_POISON5);
					msgSaid = true;
				}
			}
			}
			if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE && quests[Q_MUSHROOM]._qmsg == TEXT_MUSH10 && PlrHasItem(pnum, IDI_BRAIN, &i) && !msgSaid) {
				RemoveInvItem(pnum, i);
				SpawnQuestItemAround(IDI_SPECELIX, tw->_tx, tw->_ty, true);
				InitQTextMsg(TEXT_MUSH4);
				quests[Q_MUSHROOM]._qvar1 = QS_BRAINGIVEN;
				Qtalklist[TOWN_HEALER][Q_MUSHROOM] = TEXT_NONE;
			}
		}
		TownerTalk(STORE_HEALER, TEXT_PEPIN1);
		break;
	case TOWN_PEGBOY:
		TownerTalk(STORE_BOY, TEXT_WIRT1);
		break;
	case TOWN_STORY:
		qt = TEXT_NONE;
		if (gbMaxPlayers == 1) {
			if (quests[Q_BETRAYER]._qactive == QUEST_INIT && PlrHasItem(pnum, IDI_LAZSTAFF, &i)) {
				RemoveInvItem(pnum, i);
				quests[Q_BETRAYER]._qvar1 = 2;
				quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
				quests[Q_BETRAYER]._qlog = TRUE;
				qt = TEXT_VILE1;
				msgSaid = true;
				tw->_tListener = pnum + 1;
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == 7) {
				quests[Q_BETRAYER]._qvar1 = 8;
				quests[Q_DIABLO]._qlog = TRUE;
				qt = TEXT_VILE3;
				msgSaid = true;
				tw->_tListener = pnum + 1;
			}
		} else {
			if (quests[Q_BETRAYER]._qactive == QUEST_ACTIVE && !quests[Q_BETRAYER]._qlog) {
				quests[Q_BETRAYER]._qlog = TRUE;
				qn = Q_BETRAYER;
				qt = TEXT_VILE1;
				msgSaid = true;
				tw->_tListener = pnum + 1;
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == 7) {
				quests[Q_BETRAYER]._qvar1 = 8;
				qn = Q_BETRAYER;
				qt = TEXT_VILE3;
				msgSaid = true;
				tw->_tListener = pnum + 1;
				quests[Q_DIABLO]._qlog = TRUE;
				NetSendCmdQuest(true, Q_DIABLO, false);
			}
		}
		if (qt != TEXT_NONE)
			InitQTextMsg(qt);
		TownerTalk(STORE_STORY, TEXT_STORY1);
		break;
	case TOWN_COW:
		CowSFX(pnum);
		break;
#ifdef HELLFIRE
	case TOWN_FARMER:
		switch (quests[Q_FARMER]._qactive) {
		case QUEST_NOTAVAIL:
			qt = TEXT_NONE;
			break;
		case QUEST_INIT:
			if (PlrHasItem(pnum, IDI_RUNEBOMB, &i)) {
				quests[Q_FARMER]._qactive = QUEST_ACTIVE;
				quests[Q_FARMER]._qvar1 = 1;
				quests[Q_FARMER]._qlog = TRUE;
				quests[Q_FARMER]._qmsg = TEXT_FARMER1;
				qn = Q_FARMER;
				qt = TEXT_FARMER2;
			} else if (!plr[pnum]._pLvlVisited[DLV_CAVES1] && plr[pnum]._pLevel < 15) {
				if (plr[pnum]._pLvlVisited[DLV_CATACOMBS3])
					qt = TEXT_FARMER9;
				else if (plr[pnum]._pLvlVisited[DLV_CATACOMBS1])
					qt = TEXT_FARMER7;
				else if (plr[pnum]._pLvlVisited[DLV_CATHEDRAL2])
					qt = TEXT_FARMER5;
				else
					qt = TEXT_FARMER8;
			} else {
				quests[Q_FARMER]._qactive = QUEST_ACTIVE;
				quests[Q_FARMER]._qvar1 = 1;
				quests[Q_FARMER]._qlog = TRUE;
				quests[Q_FARMER]._qmsg = TEXT_FARMER1;
				qn = Q_FARMER;
				qt = TEXT_FARMER1;
				SpawnRewardItem(IDI_RUNEBOMB, tw->_tx, tw->_ty, false, true);
			}
			break;
		case QUEST_ACTIVE:
			qt = PlrHasItem(pnum, IDI_RUNEBOMB, &i) ? TEXT_FARMER2 : TEXT_FARMER3;
			break;
		case QUEST_DONE:
			if (quests[Q_FARMER]._qlog && quests[Q_FARMER]._qvar1 == pnum + 2) {
				quests[Q_FARMER]._qlog = FALSE;
				qn = Q_FARMER;
				qt = TEXT_FARMER4;
				SpawnRewardItem(IDI_MANA, tw->_tx, tw->_ty, false, true);
			} else {
				qt = TEXT_NONE;
			}
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		if (qt != TEXT_NONE) {
			InitQTextMsg(qt);
		}
		break;
	case TOWN_COWFARM:
		if (PlrHasItem(pnum, IDI_GREYSUIT, &i)) {
			RemoveInvItem(pnum, i);
			qt = TEXT_JERSEY7;
		} else if (PlrHasItem(pnum, IDI_BROWNSUIT, &i)) {
			RemoveInvItem(pnum, i);
			SpawnUnique(UITEM_BOVINE, tw->_tx + 1, tw->_ty, true, true);
			quests[Q_JERSEY]._qactive = QUEST_DONE;
			qn = Q_JERSEY;
			qt = TEXT_JERSEY8;
		} else if (PlrHasItem(pnum, IDI_RUNEBOMB, &i)) {
			quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
			quests[Q_JERSEY]._qvar1 = 1;
			quests[Q_JERSEY]._qmsg = TEXT_JERSEY4;
			quests[Q_JERSEY]._qlog = TRUE;
			qn = Q_JERSEY;
			qt = TEXT_JERSEY5;
		} else {
			switch (quests[Q_JERSEY]._qactive) {
			case QUEST_NOTAVAIL:
				qt = TEXT_NONE;
				break;
			case QUEST_INIT:
				switch (quests[Q_JERSEY]._qvar2++) {
				case 0: qt = TEXT_JERSEY1; break;
				case 1: qt = TEXT_JERSEY2; break;
				default: qt = TEXT_JERSEY3;
					quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
					qn = Q_JERSEY;
					break;
				}
				break;
			case QUEST_ACTIVE:
				if (quests[Q_JERSEY]._qvar1 != 0) {
					qt = TEXT_JERSEY5;
				} else if (!plr[pnum]._pLvlVisited[DLV_CAVES1] && plr[pnum]._pLevel < 15) {
					switch (random_(0, 4)) {
					case 0:
						qt = TEXT_JERSEY9;
						break;
					case 1:
						qt = TEXT_JERSEY10;
						break;
					case 2:
						qt = TEXT_JERSEY11;
						break;
					default:
						qt = TEXT_JERSEY12;
						break;
					}
				} else {
					quests[Q_JERSEY]._qvar1 = 1;
					quests[Q_JERSEY]._qmsg = TEXT_JERSEY4;
					quests[Q_JERSEY]._qlog = TRUE;
					qn = Q_JERSEY;
					qt = TEXT_JERSEY4;
					SpawnRewardItem(IDI_RUNEBOMB, tw->_tx, tw->_ty, false, true);
				}
				break;
			case QUEST_DONE:
				qt = TEXT_JERSEY1;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
		}
		if (qt != TEXT_NONE) {
			InitQTextMsg(qt);
		}
		break;
	case TOWN_GIRL:
		if (quests[Q_GIRL]._qactive != QUEST_ACTIVE) {
			qt = TEXT_NONE;
		} else if (PlrHasItem(pnum, IDI_THEODORE, &i)) {
			qt = TEXT_GIRL4;
			RemoveInvItem(pnum, i);
			SetRndSeed(tw->_tSeed);
			CreateAmulet(tw->_tx, tw->_ty, true, true);
			// quests[Q_GIRL]._qlog = FALSE;
			quests[Q_GIRL]._qactive = QUEST_DONE;
			qn = Q_GIRL;
		} else {
			if (quests[Q_GIRL]._qvar1 == 0) {
				if (quests[Q_GIRL]._qvar2++ == 0) {
					qt = TEXT_GIRL1;
				} else {
					qt = TEXT_GIRL2;
					quests[Q_GIRL]._qactive = QUEST_ACTIVE;
					quests[Q_GIRL]._qvar1 = 1;
					quests[Q_GIRL]._qlog = TRUE;
					quests[Q_GIRL]._qmsg = TEXT_GIRL2;
					qn = Q_GIRL;
				}
			} else {
				qt = TEXT_GIRL3;
			}
		}
		if (qt != TEXT_NONE) {
			InitQTextMsg(qt);
		}
		break;
#endif
	}
	if (qn != Q_INVALID)
		NetSendCmdQuest(true, qn, false);
}

DEVILUTION_END_NAMESPACE
