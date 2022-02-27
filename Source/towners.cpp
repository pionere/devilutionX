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
BYTE* pCowCels;
TownerStruct towners[MAX_TOWNERS];

/**
 * Maps from active cow sound effect index and player class to sound
 * effect ID for interacting with cows in Tristram.
 *
 * ref: enum _sfx_id
 * ref: enum plr_class
 */
const int snSFX[3][NUM_CLASSES] = {
	// clang-format off
#ifdef HELLFIRE
	{ PS_WARR52, PS_ROGUE52, PS_MAGE52, PS_MONK52, PS_ROGUE52, PS_WARR52 },
	{ PS_WARR49, PS_ROGUE49, PS_MAGE49, PS_MONK49, PS_ROGUE49, PS_WARR49 },
	{ PS_WARR50, PS_ROGUE50, PS_MAGE50, PS_MONK50, PS_ROGUE50, PS_WARR50 },
#else
	{ PS_WARR52, PS_ROGUE52, PS_MAGE52 },
	{ PS_WARR49, PS_ROGUE49, PS_MAGE49 },
	{ PS_WARR50, PS_ROGUE50, PS_MAGE50 },
#endif
	// clang-format on
};

/** Specifies the animation frame sequence of a given NPC. */
const char AnimOrder[6][144] = {
	// clang-format off
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
	    15, 16, 1, 1, 1, 1, 1, 1, 1, 1,
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
	    15, 16, 1, 2, 3, 3, 2, 1, 16, 15,
	    14, 14, 15, 16, 1, 2, 3, 3, 2, 1,
	    16, 15, 14, 14, 15, 16, 1, 2, 3, 3,
	    2, 1, 16, 15, 14, 14, 15, 16, 1, 2,
	    3, 3, 2, 1, 16, 15, 14, 14, 15, 16,
	    1, 2, 3, 3, 2, 1, 16, 15, 14, 14,
	    15, 16, 1, 2, 3, 3, 2, 1, 16, 15,
	    14, 14, 15, 16, 1, 2, 3, 2, 1, 16,
	    15, 14, 14, 15, 16, 1, 2, 3, 4, 5,
	    6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	    16, -1 },
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
	// clang-format on
};

#define TPOS_SMITH		52 + DBORDERX, 53 + DBORDERY
#define TPOS_TAVERN		45 + DBORDERX, 52 + DBORDERY
#define TPOS_HEALER		45 + DBORDERX, 69 + DBORDERY
#define TPOS_COWFARM	51 + DBORDERX, 12 + DBORDERY
#define TPOS_FARMER		52 + DBORDERX, 6 + DBORDERY
#define TPOS_GIRL		67 + DBORDERX, 33 + DBORDERY

/** Specifies the start X-coordinates of the cows in Tristram. */
const int TownCowX[] = { 48 + DBORDERX, 46 + DBORDERX, 49 + DBORDERX };
/** Specifies the start Y-coordinates of the cows in Tristram. */
const int TownCowY[] = {  6 + DBORDERY,  4 + DBORDERY, 10 + DBORDERY };
/** Specifies the start directions of the cows in Tristram. */
const int TownCowDir[] = { DIR_SW, DIR_NW, DIR_N };
/** Maps from direction to X-coordinate delta, which is used when
 * placing cows in Tristram. A single cow may require space of up
 * to three tiles when being placed on the map.
 */
const int cowoffx[8] = { -1, 0, -1, -1, -1, 0, -1, -1 };
/** Maps from direction to Y-coordinate delta, which is used when
 * placing cows in Tristram. A single cow may require space of up
 * to three tiles when being placed on the map.
 */
const int cowoffy[8] = { -1, -1, -1, 0, -1, -1, -1, 0 };
/** Specifies the active sound effect ID for interacting with cows. */
int CowPlaying = -1;

static void CowSFX(int pnum)
{
	PlayerStruct* p;
	if (CowPlaying != -1 && effect_is_playing(CowPlaying))
		return;

	_guCowClicks++;

	p = &plr;
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
	TownerStruct* tw;

	tw = &towners[tnum];

	tw->_tAnimData = const_cast<BYTE*>(CelGetFrameStart(pCowCels, dir));
	tw->_tAnimLen = 12;
	tw->_tAnimOrder = -1;
	tw->_tAnimCnt = 0;
	tw->_tAnimFrameLen = 3;
	tw->_tAnimFrame = RandRange(1, 11);
	tw->_tAnimWidth = 128;
	tw->_tAnimXOffset = (128 - TILE_WIDTH) >> 1;
}

static void InitTownerAnim(int tnum, const char* pAnimFile, int Delay, int numFrames, int ao)
{
	TownerStruct* tw;

	tw = &towners[tnum];

	assert(tw->_tAnimData == NULL);
	tw->_tAnimData = LoadFileInMem(pAnimFile);
	tw->_tAnimFrameLen = Delay;
	tw->_tAnimLen = numFrames;
	tw->_tAnimOrder = ao;
	tw->_tAnimCnt = 0;
	tw->_tAnimFrameCnt = 0;
	tw->_tAnimFrame = 1;
	tw->_tAnimWidth = 96;
	tw->_tAnimXOffset = (96 - TILE_WIDTH) >> 1;
}

#ifdef HELLFIRE
static void ReInitTownerAnim(int ttype, const char* pAnimFile)
{
	int i;

	for (i = 0; i < numtowners; i++) {
		if (towners[i]._ttype != ttype)
			continue;
		MemFreeDbg(towners[i]._tAnimData);
		towners[i]._tAnimData = LoadFileInMem(pAnimFile);
		break;
	}
}
#endif

static void InitTownerInfo(int tnum, const char* name, int type, int x, int y)
{
	TownerStruct* tw;

	dMonster[x][y] = tnum + 1;
	monsters[tnum]._mfutx = monsters[tnum]._mx = x;
	monsters[tnum]._mfuty = monsters[tnum]._my = y;
	monsters[tnum].mtalkmsg = TEXT_KING1;
	// monsters[tnum]._mhitpoints = 1 << 6; -- no longer necessary since PosOkPlayer does not check the monster's hp
#if DEBUG_MODE || DEV_MODE
	// TODO: set to prevent assert fail in CanTalkToMonst
	monsters[tnum]._mgoal = MGOAL_TALKING;
#endif // DEBUG_MODE || DEV_MODE
	tw = &towners[tnum];
	memset(tw, 0, sizeof(TownerStruct));
	static_assert(STORE_NONE == 0, "InitTownerTalk skipped by using zfill instead.");
	// tw->_tListener = MAX_PLRS;
	//tw->_tSelFlag = TRUE;
	tw->_tName = name;
	tw->_ttype = type;
	tw->_tx = x;
	tw->_ty = y;
	tw->_tSeed = GetRndSeed();
}

static void InitTownerTalk(int tnum, int store_id, int store_talk, int gossip_start, int gossip_end)
{
	TownerStruct* tw;

	tw = &towners[tnum];
	tw->_tStoreId = store_id;
	tw->_tStoreTalk = store_talk;
	tw->_tGossipStart = gossip_start;
	tw->_tGossipEnd = gossip_end;
}

/**
 * @brief Load Griswold into the game

 */
static void InitSmith()
{
	InitTownerInfo(numtowners, "Griswold the Blacksmith", TOWN_SMITH, TPOS_SMITH);
	InitTownerTalk(numtowners, STORE_SMITH, TEXT_GRISWOLD1, TEXT_GRISWOLD2, TEXT_GRISWOLD13);
	InitTownerAnim(numtowners, "Towners\\Smith\\SmithN.CEL", 3, 16, 0);
	numtowners++;
}

static void InitBarOwner()
{
	InitTownerInfo(numtowners, "Ogden the Tavern owner", TOWN_TAVERN, TPOS_TAVERN);
	InitTownerTalk(numtowners, STORE_TAVERN, TEXT_OGDEN1, TEXT_OGDEN2, TEXT_OGDEN10);
	InitTownerAnim(numtowners, "Towners\\TwnF\\TwnFN.CEL", 3, 16, 3);
	numtowners++;
}

static void InitTownDead()
{
	InitTownerInfo(numtowners, "Wounded Townsman", TOWN_DEADGUY, 14 + DBORDERX, 22 + DBORDERY);
	InitTownerAnim(numtowners, "Towners\\Butch\\Deadguy.CEL", 6, 8, -1);
	numtowners++;
}

static void InitWitch()
{
	InitTownerInfo(numtowners, "Adria the Witch", TOWN_WITCH, 70 + DBORDERX, 10 + DBORDERY);
	InitTownerTalk(numtowners, STORE_WITCH, TEXT_ADRIA1, TEXT_ADRIA2, TEXT_ADRIA13);
	InitTownerAnim(numtowners, "Towners\\TownWmn1\\Witch.CEL", 6, 19, 5);
	numtowners++;
}

static void InitBarmaid()
{
	InitTownerInfo(numtowners, "Gillian the Barmaid", TOWN_BMAID, 33 + DBORDERX, 56 + DBORDERY);
	InitTownerTalk(numtowners, STORE_BARMAID, TEXT_GILLIAN1, TEXT_GILLIAN2, TEXT_GILLIAN10);
	InitTownerAnim(numtowners, "Towners\\TownWmn1\\WmnN.CEL", 6, 18, -1);
	numtowners++;
}

static void InitBoy()
{
	InitTownerInfo(numtowners, "Wirt the Peg-legged boy", TOWN_PEGBOY, 1 + DBORDERX, 43 + DBORDERY);
	InitTownerTalk(numtowners, STORE_BOY, TEXT_WIRT1, TEXT_WIRT2, TEXT_WIRT12);
	InitTownerAnim(numtowners, "Towners\\TownBoy\\PegKid1.CEL", 6, 20, -1);
	numtowners++;
}

static void InitHealer()
{
	InitTownerInfo(numtowners, "Pepin the Healer", TOWN_HEALER, TPOS_HEALER);
	InitTownerTalk(numtowners, STORE_HEALER, TEXT_PEPIN1, TEXT_PEPIN2, TEXT_PEPIN11);
	InitTownerAnim(numtowners, "Towners\\Healer\\Healer.CEL", 6, 20, 1);
	numtowners++;
}

static void InitTeller()
{
	InitTownerInfo(numtowners, "Cain the Elder", TOWN_STORY, 52 + DBORDERX, 61 + DBORDERY);
	InitTownerTalk(numtowners, STORE_STORY, TEXT_STORY1, TEXT_STORY2, TEXT_STORY11);
	InitTownerAnim(numtowners, "Towners\\Strytell\\Strytell.CEL", 3, 25, 2);
	numtowners++;
}

static void InitDrunk()
{
	InitTownerInfo(numtowners, "Farnham the Drunk", TOWN_DRUNK, 61 + DBORDERX, 74 + DBORDERY);
	InitTownerTalk(numtowners, STORE_DRUNK, TEXT_FARNHAM1, TEXT_FARNHAM2, TEXT_FARNHAM13);
	InitTownerAnim(numtowners, "Towners\\Drunk\\TwnDrunk.CEL", 3, 18, 4);
	numtowners++;
}

static void InitCows()
{
	int i, dir;
	int x, y, xo, yo;
	assert(pCowCels == NULL);
	pCowCels = LoadFileInMem("Towners\\Animals\\Cow.CEL");
	static_assert(lengthof(TownCowX) == lengthof(TownCowY), "Mismatching TownCow tables I.");
	static_assert(lengthof(TownCowX) == lengthof(TownCowDir), "Mismatching TownCow tables II.");
	for (i = 0; i < lengthof(TownCowX); i++) {
		x = TownCowX[i];
		y = TownCowY[i];
		dir = TownCowDir[i];
		InitTownerInfo(numtowners, "Cow", TOWN_COW, x, y);
		InitCowAnim(numtowners, dir);

		xo = x + cowoffx[dir];
		yo = y + cowoffy[dir];

		//assert(dMonster[xo][yo] == 0);
		dMonster[xo][yo] = -(numtowners + 1);
		if (xo != x && yo != y) {
			//assert(dMonster[x][yo] == 0);
			//assert(dMonster[xo][y] == 0);
			dMonster[x][yo] = -(numtowners + 1);
			dMonster[xo][y] = -(numtowners + 1);
		}

		numtowners++;
	}
}

#ifdef HELLFIRE
static void InitFarmer()
{
	InitTownerInfo(numtowners, "Lester the farmer", TOWN_FARMER, TPOS_FARMER);
	InitTownerAnim(numtowners, "Towners\\Farmer\\Farmrn2.CEL", 3, 15, -1);
	numtowners++;
}

static void InitCowFarmer()
{
	const char* pAnimFile;

	InitTownerInfo(numtowners, "Complete Nut", TOWN_COWFARM, TPOS_COWFARM);
	pAnimFile = quests[Q_JERSEY]._qactive != QUEST_DONE ? "Towners\\Farmer\\cfrmrn2.CEL" : "Towners\\Farmer\\mfrmrn2.CEL";
	InitTownerAnim(numtowners, pAnimFile, 3, 15, -1);
	numtowners++;
}

static void InitGirl()
{
	const char* pAnimFile;

	InitTownerInfo(numtowners, "Celia", TOWN_GIRL, TPOS_GIRL);
	pAnimFile = quests[Q_GIRL]._qactive == QUEST_ACTIVE ? "Towners\\Girl\\Girlw1.CEL" : "Towners\\Girl\\Girls1.CEL";
	InitTownerAnim(numtowners, pAnimFile, 6, 20, -1);
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
	if (quests[Q_JERSEY]._qactive != QUEST_NOTAVAIL) {
		InitCowFarmer();
	}
	if (quests[Q_FARMER]._qactive != QUEST_NOTAVAIL) { // if (quests[Q_FARMER]._qactive != QUEST_DONE || quests[Q_FARMER]._qlog) {
		InitFarmer(); // in vanilla hellfire the farmer was gone after the quest is completed, but there is no reason for that
	}
	if (quests[Q_GIRL]._qactive != QUEST_NOTAVAIL) {
		InitGirl();
	}
#endif
	InitCows();
}

void FreeTownerGFX()
{
	int i;

	for (i = 0; i < numtowners; i++) {
		if (towners[i]._ttype == TOWN_COW) {
			towners[i]._tAnimData = NULL;
		} else {
			MemFreeDbg(towners[i]._tAnimData);
		}
	}

	MemFreeDbg(pCowCels);
}

static void TownCtrlMsg(TownerStruct* tw)
{
	/*PlayerStruct* p;
	int dx, dy;

	if (tw->_tListener != MAX_PLRS) {
		p = &plx(tw->_tListener);
		dx = abs(tw->_tx - p->_px);
		dy = abs(tw->_ty - p->_py);
		if (dx >= 2 || dy >= 2) {
			tw->_tListener = MAX_PLRS;
			stream_stop();
		}
	}*/
}

void ProcessTowners()
{
	TownerStruct* tw;
	int i, ao;

	tw = towners;
	for (i = numtowners; i > 0; i--, tw++) {
		TownCtrlMsg(tw);
		if (tw->_ttype == TOWN_DEADGUY) {
			if (quests[Q_BUTCHER]._qactive != QUEST_INIT) {
				//if (quests[Q_BUTCHER]._qactive != QUEST_ACTIVE || quests[Q_BUTCHER]._qlog) {
					if (!gbQtextflag) {
						//tw->_tAnimFrameLen = 1000;
						tw->_tAnimFrame = 1;
						tw->_tName = "Slain Townsman";
					}
					continue; //tw->_tAnimCnt = 0;
				/*} else {
					if (gbQtextflag)
						tw->_tAnimCnt = 0;
				}*/
			}
		}

		tw->_tAnimCnt++;
		if (tw->_tAnimCnt >= tw->_tAnimFrameLen) {
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

bool PlrHasStorageItem(int pnum, int item, int* outidx)
{
	ItemStruct* pi;
	int i;

	pi = plr._pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
		if (pi->_iIdx == item && pi->_itype != ITYPE_NONE && pi->_itype != ITYPE_PLACEHOLDER) {
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

	pi = plr._pSpdList;
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
	ClearPanels();
	InitQTextMsg(talk);
	StartStore(store);
}

void SyncTownerQ(int pnum, int idx)
{
	int i;

	// assert(plr._pmode != PM_DEATH);
	// assert(plr._pDunLevel == DLV_TOWN);

	if (!PlrHasStorageItem(pnum, idx, &i))
		return;

	switch (idx) {
	case IDI_BANNER:
		if (quests[Q_LTBANNER]._qvar1 >= 2)
			return;
		quests[Q_LTBANNER]._qlog = FALSE;
		quests[Q_LTBANNER]._qvar1 = 2;
		if (currLvl._dLevelIdx == DLV_TOWN)
			SpawnUnique(UITEM_HARCREST, TPOS_TAVERN + 1, pnum == mypnum, false);
		break;
	case IDI_ROCK:
		if (quests[Q_ROCK]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_ROCK]._qactive = QUEST_DONE;
		//quests[Q_ROCK]._qlog = FALSE;
		if (currLvl._dLevelIdx == DLV_TOWN)
			SpawnUnique(UITEM_INFRARING, TPOS_SMITH + 1, pnum == mypnum, false);
		break;
	case IDI_ANVIL:
		if (quests[Q_ANVIL]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_ANVIL]._qactive = QUEST_DONE;
		//quests[Q_ANVIL]._qlog = FALSE;
		if (currLvl._dLevelIdx == DLV_TOWN)
			SpawnUnique(UITEM_GRISWOLD, TPOS_SMITH + 1, pnum == mypnum, false);
		break;
	case IDI_FUNGALTM:
		if (quests[Q_MUSHROOM]._qactive != QUEST_INIT)
			return;
		quests[Q_MUSHROOM]._qactive = QUEST_ACTIVE;
		quests[Q_MUSHROOM]._qlog = TRUE;
		quests[Q_MUSHROOM]._qvar1 = QS_TOMEGIVEN;
		break;
	case IDI_MUSHROOM:
		if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 >= QS_MUSHGIVEN)
			return;
		quests[Q_MUSHROOM]._qvar1 = QS_MUSHGIVEN;
		quests[Q_MUSHROOM]._qmsg = TEXT_MUSH10;
		break;
	case IDI_BRAIN:
		if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 >= QS_MUSHGIVEN)
			return;
		quests[Q_MUSHROOM]._qvar1 = QS_BRAINGIVEN;
		quests[Q_MUSHROOM]._qmsg = TEXT_MUSH4;
		if (currLvl._dLevelIdx == DLV_TOWN)
			SpawnQuestItemAround(IDI_SPECELIX, TPOS_HEALER + 1, pnum == mypnum/*, false*/);
		break;
	case IDI_LAZSTAFF:
		if (quests[Q_BETRAYER]._qvar1 >= 2 /*|| quests[Q_BETRAYER]._qactive != QUEST_ACTIVE*/)
			return;
		quests[Q_BETRAYER]._qvar1 = 2;
		quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
		quests[Q_BETRAYER]._qlog = TRUE;
		break;
#ifdef HELLFIRE
	case IDI_GRAYSUIT:
		break;
	case IDI_BROWNSUIT:
		if (quests[Q_JERSEY]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_JERSEY]._qactive = QUEST_DONE;
		if (currLvl._dLevelIdx == DLV_TOWN) {
			SpawnUnique(UITEM_BOVINE, TPOS_COWFARM, pnum == mypnum, false);
			ReInitTownerAnim(TOWN_COWFARM, "Towners\\Farmer\\mfrmrn2.CEL");
		}
		break;
	case IDI_THEODORE:
		if (quests[Q_GIRL]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_GIRL]._qactive = QUEST_DONE;
		// quests[Q_GIRL]._qlog = FALSE;
		if (currLvl._dLevelIdx == DLV_TOWN) {
			WORD wCI = plr._pInvList[i]._iCreateInfo;  // the amulet inherits the level of THEODORE
			SetRndSeed(plr._pInvList[i]._iSeed); // and uses its seed
			CreateAmulet(wCI, TPOS_GIRL, pnum == mypnum);
			ReInitTownerAnim(TOWN_GIRL, "Towners\\Girl\\Girls1.CEL");
		}
		break;
#endif
	default:
		return;
	}

	SyncPlrStorageRemove(pnum, i);
}

void TalkToTowner(int tnum)
{
	TownerStruct* tw;
	int i, qt, qn, pnum = mypnum;

	tw = &towners[tnum];
	if (gbQtextflag) {
		return;
	}

	if (pcurs >= CURSOR_FIRSTITEM) {
		return;
	}

	qt = TEXT_NONE;
	qn = Q_INVALID;

	switch (tw->_ttype) {
	case TOWN_TAVERN:
		if (!IsLvlVisited(DLV_CATHEDRAL1) && plr._pLevel == 1) {
			qt = TEXT_INTRO;
			break;
		}
		if (IsLvlVisited(DLV_CATHEDRAL3) && quests[Q_SKELKING]._qactive != QUEST_NOTAVAIL) {
			if (quests[Q_SKELKING]._qactive == QUEST_INIT) {
				quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
				quests[Q_SKELKING]._qlog = TRUE;
				qn = Q_SKELKING;
				qt = TEXT_KING2;
				break;
			}
			if (quests[Q_SKELKING]._qactive == QUEST_DONE && quests[Q_SKELKING]._qlog) {
				quests[Q_SKELKING]._qlog = FALSE;
				// qn = Q_SKELKING; - let the others hear the final message
				qt = TEXT_KING4;
				break;
			}
		}
		if (IsLvlVisited(DLV_CATHEDRAL4) && quests[Q_LTBANNER]._qactive != QUEST_NOTAVAIL) {
			if (quests[Q_LTBANNER]._qactive == QUEST_INIT) {
				quests[Q_LTBANNER]._qactive = QUEST_ACTIVE;
				quests[Q_LTBANNER]._qlog = TRUE;
				qn = Q_LTBANNER;
				qt = TEXT_BANNER2;
			} else if (quests[Q_LTBANNER]._qactive == QUEST_ACTIVE && PlrHasStorageItem(pnum, IDI_BANNER, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_BANNER);
				qt = TEXT_BANNER3;
			}
		}
		break;
	case TOWN_DEADGUY:
		if (quests[Q_BUTCHER]._qactive == QUEST_ACTIVE /*&& quests[Q_BUTCHER]._qvar1 == 1*/) {
			i = sgSFXSets[SFXS_PLR_08][plr._pClass];
			if (!effect_is_playing(i)) {
				// tw->_tListener = pnum;
				PlaySFX(i);
			}
		} else if (quests[Q_BUTCHER]._qactive == QUEST_INIT /*|| (quests[Q_BUTCHER]._qactive == QUEST_ACTIVE && quests[Q_BUTCHER]._qvar1 == 0)*/) {
			quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
			quests[Q_BUTCHER]._qlog = TRUE;
			// quests[Q_BUTCHER]._qmsg = TEXT_BUTCH9;
			//quests[Q_BUTCHER]._qvar1 = 1;
			qn = Q_BUTCHER;
			qt = TEXT_BUTCH9;
		}
		break;
	case TOWN_SMITH:
		if (IsLvlVisited(DLV_CATACOMBS1) /*&& quests[Q_ROCK]._qactive != QUEST_NOTAVAIL*/) {
			if (quests[Q_ROCK]._qactive == QUEST_INIT) {
				quests[Q_ROCK]._qactive = QUEST_ACTIVE;
				quests[Q_ROCK]._qlog = TRUE;
				qn = Q_ROCK;
				qt = TEXT_INFRA5;
				break;
			}
			if (quests[Q_ROCK]._qactive == QUEST_ACTIVE && PlrHasStorageItem(pnum, IDI_ROCK, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_ROCK);
				qt = TEXT_INFRA7;
				break;
			}
		}
		if (IsLvlVisited(DLV_CAVES2) /*&& quests[Q_ANVIL]._qactive != QUEST_NOTAVAIL*/) {
			if (quests[Q_ANVIL]._qactive == QUEST_INIT) {
				quests[Q_ANVIL]._qactive = QUEST_ACTIVE;
				quests[Q_ANVIL]._qlog = TRUE;
				qn = Q_ANVIL;
				qt = TEXT_ANVIL5;
			} else if (quests[Q_ANVIL]._qactive == QUEST_ACTIVE && PlrHasStorageItem(pnum, IDI_ANVIL, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_ANVIL);
				qt = TEXT_ANVIL7;
			}
		}
		break;
	case TOWN_WITCH:
		if (quests[Q_MUSHROOM]._qactive == QUEST_INIT && PlrHasStorageItem(pnum, IDI_FUNGALTM, &i)) {
			NetSendCmdParam1(CMD_QTOWNER, IDI_FUNGALTM);
			qt = TEXT_MUSH8;
		} else if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE) {
			if (quests[Q_MUSHROOM]._qvar1 < QS_MUSHGIVEN) {
				if (PlrHasStorageItem(pnum, IDI_MUSHROOM, &i)) {
					NetSendCmdParam1(CMD_QTOWNER, IDI_MUSHROOM);
					qt = TEXT_MUSH10;
					break;
				}
				if (quests[Q_MUSHROOM]._qmsg != TEXT_MUSH9) {
					quests[Q_MUSHROOM]._qmsg = TEXT_MUSH9;
					qn = Q_MUSHROOM;
					qt = TEXT_MUSH9;
					break;
				}
			}
			if (PlrHasStorageItem(pnum, IDI_SPECELIX, &i) || PlrHasBeltItem(pnum, IDI_SPECELIX)) {
				quests[Q_MUSHROOM]._qactive = QUEST_DONE;
				qn = Q_MUSHROOM;
				qt = TEXT_MUSH12;
			} else if (PlrHasStorageItem(pnum, IDI_BRAIN, &i) && quests[Q_MUSHROOM]._qvar2 != TEXT_MUSH11) {
				quests[Q_MUSHROOM]._qvar2 = TEXT_MUSH11;
				qt = TEXT_MUSH11;
			}
		}
		break;
	case TOWN_BMAID:
#ifdef HELLFIRE
		if (PlrHasStorageItem(pnum, IDI_MAPOFDOOM, &i) && quests[Q_GRAVE]._qactive == QUEST_INIT) {
			quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
			quests[Q_GRAVE]._qlog = TRUE;
			quests[Q_GRAVE]._qmsg = TEXT_GRAVE8;
			qn = Q_GRAVE;
			qt = TEXT_GRAVE8;
		}
#endif
		break;
	case TOWN_DRUNK:
		break;
	case TOWN_HEALER:
		if (quests[Q_PWATER]._qactive == QUEST_INIT && quests[Q_PWATER]._qvar1 != 2 && IsLvlVisited(DLV_CATHEDRAL2)) {
			quests[Q_PWATER]._qactive = QUEST_ACTIVE;
			quests[Q_PWATER]._qlog = TRUE;
			// quests[Q_PWATER]._qmsg = TEXT_POISON3;
			// quests[Q_PWATER]._qvar1 = 1;
			qn = Q_PWATER;
			qt = TEXT_POISON3;
		} else if ((quests[Q_PWATER]._qactive == QUEST_INIT || quests[Q_PWATER]._qactive == QUEST_ACTIVE)
		 && quests[Q_PWATER]._qvar1 == 2) {
			quests[Q_PWATER]._qactive = QUEST_DONE;
			SpawnUnique(UITEM_TRING, TPOS_HEALER + 1, false, true);
			qn = Q_PWATER;
			qt = TEXT_POISON5;
		} else if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE
		 && quests[Q_MUSHROOM]._qvar1 < QS_BRAINGIVEN) {
			if (PlrHasStorageItem(pnum, IDI_BRAIN, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_BRAIN);
				qt = TEXT_MUSH4;
			} else if (quests[Q_MUSHROOM]._qvar1 >= QS_MUSHGIVEN && quests[Q_MUSHROOM]._qvar2 != TEXT_MUSH3) {
				quests[Q_MUSHROOM]._qvar2 = TEXT_MUSH3;
				qt = TEXT_MUSH3;
			}
		}
		break;
	case TOWN_PEGBOY:
		break;
	case TOWN_STORY:
		if (!IsMultiGame) {
			if (quests[Q_BETRAYER]._qactive == QUEST_INIT && PlrHasStorageItem(pnum, IDI_LAZSTAFF, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_LAZSTAFF);
				qt = TEXT_VILE1;
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == 7) {
				quests[Q_BETRAYER]._qvar1 = 8;
				quests[Q_DIABLO]._qlog = TRUE;
				qt = TEXT_VILE3;
			}
		} else {
			if (quests[Q_BETRAYER]._qactive == QUEST_ACTIVE && !quests[Q_BETRAYER]._qlog) {
				quests[Q_BETRAYER]._qlog = TRUE;
				qn = Q_BETRAYER;
				qt = TEXT_VILE1;
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == 7) {
				quests[Q_BETRAYER]._qvar1 = 8;
				qn = Q_BETRAYER;
				qt = TEXT_VILE3;
				quests[Q_DIABLO]._qlog = TRUE;
				NetSendCmdQuest(Q_DIABLO, false);
			}
		}
		break;
	case TOWN_COW:
		CowSFX(pnum);
		break;
#ifdef HELLFIRE
	case TOWN_FARMER:
		switch (quests[Q_FARMER]._qactive) {
		case QUEST_NOTAVAIL:
			break;
		case QUEST_INIT:
			if (PlrHasStorageItem(pnum, IDI_RUNEBOMB, &i)) {
				quests[Q_FARMER]._qactive = QUEST_ACTIVE;
				quests[Q_FARMER]._qvar1 = 1;
				quests[Q_FARMER]._qlog = TRUE;
				// quests[Q_FARMER]._qmsg = TEXT_FARMER1;
				qn = Q_FARMER;
				qt = TEXT_FARMER2;
			} else if (!IsLvlVisited(DLV_CAVES1) && plr._pLevel < 15) {
				if (IsLvlVisited(DLV_CATACOMBS3))
					qt = TEXT_FARMER9;
				else if (IsLvlVisited(DLV_CATACOMBS1))
					qt = TEXT_FARMER7;
				else if (IsLvlVisited(DLV_CATHEDRAL2))
					qt = TEXT_FARMER5;
				else
					qt = TEXT_FARMER8;
			} else {
				quests[Q_FARMER]._qactive = QUEST_ACTIVE;
				quests[Q_FARMER]._qvar1 = 1;
				quests[Q_FARMER]._qlog = TRUE;
				// quests[Q_FARMER]._qmsg = TEXT_FARMER1;
				qn = Q_FARMER;
				qt = TEXT_FARMER1;
				SpawnRewardItem(IDI_RUNEBOMB, TPOS_FARMER, false, true);
			}
			break;
		case QUEST_ACTIVE:
			qt = PlrHasStorageItem(pnum, IDI_RUNEBOMB, &i) ? TEXT_FARMER2 : TEXT_FARMER3;
			break;
		case QUEST_DONE:
			if (quests[Q_FARMER]._qlog && quests[Q_FARMER]._qvar1 == pnum + 2) {
				quests[Q_FARMER]._qlog = FALSE;
				qn = Q_FARMER;
				qt = TEXT_FARMER4;
				SpawnRewardItem(IDI_MANA, TPOS_FARMER, false, true);
			}
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		break;
	case TOWN_COWFARM:
		switch (quests[Q_JERSEY]._qactive) {
		case QUEST_NOTAVAIL:
			break;
		case QUEST_INIT:
			if (PlrHasStorageItem(pnum, IDI_RUNEBOMB, &i)) {
				quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
				quests[Q_JERSEY]._qvar1 = 1;
				// quests[Q_JERSEY]._qmsg = TEXT_JERSEY4;
				quests[Q_JERSEY]._qlog = TRUE;
				qn = Q_JERSEY;
				qt = TEXT_JERSEY5;
				break;
			}
			switch (quests[Q_JERSEY]._qvar2++) {
			case 0: qt = TEXT_JERSEY1; break;
			case 1: qt = TEXT_JERSEY2; break;
			default:
				qt = TEXT_JERSEY3;
				quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
				qn = Q_JERSEY;
				break;
			}
			break;
		case QUEST_ACTIVE:
			if (PlrHasStorageItem(pnum, IDI_GRAYSUIT, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_GRAYSUIT);
				qt = TEXT_JERSEY7;
				break;
			} else if (PlrHasStorageItem(pnum, IDI_BROWNSUIT, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_BROWNSUIT);
				qt = TEXT_JERSEY8;
				break;
			}
			if (quests[Q_JERSEY]._qvar1 != 0) {
				qt = TEXT_JERSEY5;
			} else if (!IsLvlVisited(DLV_CAVES1) && plr._pLevel < 15) {
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
				// quests[Q_JERSEY]._qmsg = TEXT_JERSEY4;
				quests[Q_JERSEY]._qlog = TRUE;
				qn = Q_JERSEY;
				qt = TEXT_JERSEY4;
				SpawnRewardItem(IDI_RUNEBOMB, TPOS_COWFARM, false, true);
			}
			break;
		case QUEST_DONE:
			qt = TEXT_JERSEY1;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		break;
	case TOWN_GIRL:
		if (quests[Q_GIRL]._qactive == QUEST_ACTIVE) {
			if (PlrHasStorageItem(pnum, IDI_THEODORE, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_THEODORE);
				qt = TEXT_GIRL4;
			} else if (quests[Q_GIRL]._qvar1 == 0) {
				if (quests[Q_GIRL]._qvar2++ == 0) {
					qt = TEXT_GIRL1;
				} else {
					qt = TEXT_GIRL2;
					quests[Q_GIRL]._qvar1 = 1;
					quests[Q_GIRL]._qlog = TRUE;
					// quests[Q_GIRL]._qmsg = TEXT_GIRL2;
					qn = Q_GIRL;
				}
			} else {
				qt = TEXT_GIRL3;
			}
		}
		break;
#endif
	}
	if (qn != Q_INVALID)
		NetSendCmdQuest(qn, false);
	if (qt != TEXT_NONE) {
		// tw->_tListener = pnum;
		InitQTextMsg(qt);
	} else if (tw->_tStoreId != STORE_NONE) {
		TownerTalk(tw->_tStoreId, tw->_tStoreTalk);
	}
}

DEVILUTION_END_NAMESPACE
