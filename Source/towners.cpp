/**
 * @file towners.cpp
 *
 * Implementation of functionality for loading and spawning towners.
 */
#include "all.h"
#include "engine/render/render.h"

DEVILUTION_BEGIN_NAMESPACE

static int _guCowMsg;
int numtowners;
static unsigned _guCowClicks;
static BYTE* pCowCels;

/**
 * Maps from active cow sound effect index and player class to sound
 * effect ID for interacting with cows in Tristram.
 *
 * ref: enum _sfx_id
 * ref: enum plr_class
 */
static const int snSFX[3][NUM_CLASSES] = {
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
static const int8_t AnimOrder[6][144] = {
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

/** Contains the TEXT_-ranges related to gossip for each towner ID. */
const int GossipList[STORE_TOWNERS][2] = {
	// clang-format off
/* TOWN_SMITH */  { TEXT_SMITH2,   TEXT_SMITH13   },
/* TOWN_HEALER */ { TEXT_HEALER2,  TEXT_HEALER11  },
/* TOWN_TAVERN */ { TEXT_TAVERN2,  TEXT_TAVERN10  },
/* TOWN_STORY */  { TEXT_STORY2,   TEXT_STORY11   },
/* TOWN_DRUNK */  { TEXT_DRUNK2,   TEXT_DRUNK13   },
/* TOWN_WITCH */  { TEXT_WITCH2,   TEXT_WITCH13   },
/* TOWN_BARMAID */{ TEXT_BARMAID2, TEXT_BARMAID10 },
/* TOWN_PEGBOY */ { TEXT_PEGBOY2,  TEXT_PEGBOY12  },
	// clang-format on
};

#define TPOS_SMITH   52 + DBORDERX, 53 + DBORDERY
#define TPOS_TAVERN  45 + DBORDERX, 52 + DBORDERY
#define TPOS_HEALER  45 + DBORDERX, 69 + DBORDERY
#define TPOS_COWFARM 51 + DBORDERX, 12 + DBORDERY
#define TPOS_FARMER  52 + DBORDERX, 6 + DBORDERY
#define TPOS_GIRL    67 + DBORDERX, 33 + DBORDERY

/** Specifies the active sound effect ID for interacting with cows. */
static int CowPlaying = SFX_NONE;

static void CowSFX(MonsterStruct* cow, int pnum)
{
	if (SFX_VALID(CowPlaying) && IsSfxStreaming(CowPlaying))
		return;

	_guCowClicks++;

	if (_guCowClicks < 8) {
		CowPlaying = _guCowClicks == 4 ? TSFX_COW2 : TSFX_COW1;
		PlaySfxLoc(CowPlaying, cow->_mx, cow->_my);
		return;
	}

	_guCowClicks = 4;
	CowPlaying = snSFX[_guCowMsg][plr._pClass];
	PlaySfxLoc(CowPlaying, plr._px, plr._py);
	_guCowMsg++;
	if (_guCowMsg >= lengthof(snSFX))
		_guCowMsg = 0;
}

static void InitCowAnim(int tnum, const BYTE* anim)
{
	MonsterStruct* tw;

	tw = &monsters[tnum];

	tw->_mAnimData = anim;                                   // TNR_ANIM_DATA
	tw->_mAnimFrameLen = 3;                                  // TNR_ANIM_FRAME_LEN
	tw->_mAnimLen = SwapLE32(*(DWORD*)anim);                 // TNR_ANIM_LEN
	tw->_mVar1 = -1;                                         // TNR_ANIM_ORDER
	tw->_mAnimCnt = 0;                                       // TNR_ANIM_CNT
	tw->_mAnimFrame = RandRange(1, tw->_mAnimLen);           // TNR_ANIM_FRAME
	tw->_mAnimWidth = 128 * ASSET_MPL;                       // TNR_ANIM_WIDTH
	tw->_mAnimXOffset = (tw->_mAnimWidth - TILE_WIDTH) >> 1; // TNR_ANIM_X_OFFSET
}

static void InitTownerAnim(int tnum, const char* pAnimFile, int Delay, int ao)
{
	MonsterStruct* tw;

	tw = &monsters[tnum];

	// commented out, because it might be populated by InitMonster
	// assert(tw->_mAnimData == NULL);
	const BYTE* anim = LoadFileInMem(pAnimFile);
	tw->_mAnimData = anim;                                   // TNR_ANIM_DATA
	tw->_mAnimFrameLen = Delay;                              // TNR_ANIM_FRAME_LEN
	tw->_mAnimLen = SwapLE32(*(DWORD*)anim);                 // TNR_ANIM_LEN
	tw->_mVar1 = ao;                                         // TNR_ANIM_ORDER
	tw->_mVar2 = 0;                                          // TNR_ANIM_FRAME_CNT
	tw->_mAnimCnt = 0;                                       // TNR_ANIM_CNT
	tw->_mAnimFrame = 1;                                     // TNR_ANIM_FRAME
	tw->_mAnimWidth = 96 * ASSET_MPL;                        // TNR_ANIM_WIDTH
	tw->_mAnimXOffset = (tw->_mAnimWidth - TILE_WIDTH) >> 1; // TNR_ANIM_X_OFFSET
}

#ifdef HELLFIRE
static void ReInitTownerAnim(int ttype, const char* pAnimFile)
{
	int i;

	for (i = MAX_MINIONS; i < numtowners; i++) {
		if (monsters[i]._mType != ttype) // TNR_TYPE
			continue;
		MemFreeConst(monsters[i]._mAnimData); // TNR_ANIM_DATA
		monsters[i]._mAnimData = LoadFileInMem(pAnimFile);
		break;
	}
}
#endif

static void InitTownerInfo(int tnum, const char* name, int type, int x, int y, int selFlag)
{
	MonsterStruct* tw;

	tw = &monsters[tnum];

	// set dMonster for CheckCursMove
	dMonster[x][y] = tnum + 1;
	tw->_mType = type; // TNR_TYPE
	// set position for DrawInfoStr and FindTowner
	tw->_mx = x;
	tw->_my = y;
	// set future position for CheckNewPath
	tw->_mfutx = x;
	tw->_mfuty = y;
	tw->_mgoal = MGOAL_TALKING;  // for CanTalkToMonst
	tw->_mgoalvar1 = STORE_NONE; // TNR_STORE for TalkToTowner
#if DEBUG_MODE || DEV_MODE
	tw->_mgoalvar2 = TEXT_KING1; // TALK_MESSAGE for DoActionBtnCmd(CanTalkToMonst)
#endif
	// set _mhitpoints, _mSelFlag and _mFlags for CheckCursMove
	tw->_mhitpoints = 1 << 6;
	tw->_mSelFlag = selFlag; // TNR_SELFLAG
	tw->_mFlags = 0;
	// _mmaxhp for DrawInfoStr
	tw->_mmaxhp = 0;
	// set _mName, _muniqtype for DrawInfoStr
	tw->_mName = name; // TNR_NAME
	// tw->_muniqtype = 0;
	// set _mRndSeed for S_TalkEnter
	tw->_mRndSeed = NextRndSeed(); // TNR_SEED
	// tw->_mListener = MAX_PLRS; // TNR_LISTENER
}

static void InitTownerTalk(int tnum, int store_id, int store_talk)
{
	monsters[tnum]._mgoalvar1 = store_id;   // TNR_STORE
	monsters[tnum]._mgoalvar2 = store_talk; // TNR_TALK, TALK_MESSAGE
}

/**
 * @brief Load Griswold into the game

 */
static void InitSmith()
{
	InitTownerInfo(numtowners, "Griswold the Blacksmith", TOWN_SMITH, TPOS_SMITH, 7);
	InitTownerTalk(numtowners, STORE_SMITH, TEXT_SMITH1);
	InitTownerAnim(numtowners, "Towners\\Smith\\SmithN.CEL", 3, 0);
	numtowners++;
}

static void InitTavern()
{
	InitTownerInfo(numtowners, "Ogden the Tavern owner", TOWN_TAVERN, TPOS_TAVERN, 7);
	InitTownerTalk(numtowners, STORE_TAVERN, TEXT_TAVERN1);
	InitTownerAnim(numtowners, "Towners\\TwnF\\TwnFN.CEL", 3, 3);
	numtowners++;
}

static void InitDeadguy()
{
	InitTownerInfo(numtowners, "Wounded Townsman", TOWN_DEADGUY, 14 + DBORDERX, 22 + DBORDERY, 1);
	InitTownerAnim(numtowners, "Towners\\Butch\\Deadguy.CEL", 6, -1);
	numtowners++;
}

static void InitWitch()
{
	InitTownerInfo(numtowners, "Adria the Witch", TOWN_WITCH, 70 + DBORDERX, 10 + DBORDERY, 3);
	InitTownerTalk(numtowners, STORE_WITCH, TEXT_WITCH1);
	InitTownerAnim(numtowners, "Towners\\TownWmn1\\Witch.CEL", 6, 5);
	numtowners++;
}

static void InitBarmaid()
{
	InitTownerInfo(numtowners, "Gillian the Barmaid", TOWN_BARMAID, 33 + DBORDERX, 56 + DBORDERY, 3);
	InitTownerTalk(numtowners, STORE_BARMAID, TEXT_BARMAID1);
	InitTownerAnim(numtowners, "Towners\\TownWmn1\\WmnN.CEL", 6, -1);
	numtowners++;
}

static void InitPegboy()
{
	InitTownerInfo(numtowners, "Wirt the Peg-legged boy", TOWN_PEGBOY, 1 + DBORDERX, 43 + DBORDERY, 3);
	InitTownerTalk(numtowners, STORE_PEGBOY, TEXT_PEGBOY1);
	InitTownerAnim(numtowners, "Towners\\TownBoy\\PegKid1.CEL", 6, -1);
	numtowners++;
}

static void InitHealer()
{
	InitTownerInfo(numtowners, "Pepin the Healer", TOWN_HEALER, TPOS_HEALER, 7);
	InitTownerTalk(numtowners, STORE_HEALER, TEXT_HEALER1);
	InitTownerAnim(numtowners, "Towners\\Healer\\Healer.CEL", 6, 1);
	numtowners++;
}

static void InitStory()
{
	InitTownerInfo(numtowners, "Cain the Elder", TOWN_STORY, 52 + DBORDERX, 61 + DBORDERY, 7);
	InitTownerTalk(numtowners, STORE_STORY, TEXT_STORY1);
	InitTownerAnim(numtowners, "Towners\\Strytell\\Strytell.CEL", 3, 2);
	numtowners++;
}

static void InitDrunk()
{
	InitTownerInfo(numtowners, "Farnham the Drunk", TOWN_DRUNK, 61 + DBORDERX, 74 + DBORDERY, 3);
	InitTownerTalk(numtowners, STORE_DRUNK, TEXT_DRUNK1);
	InitTownerAnim(numtowners, "Towners\\Drunk\\TwnDrunk.CEL", 3, 4);
	numtowners++;
}

static void InitPriest()
{
	InitTownerInfo(numtowners, "Tremain the Priest", TOWN_PRIEST, 63 + DBORDERX, 69 + DBORDERY, 3);
	InitTownerTalk(numtowners, STORE_PRIEST, TEXT_PRIEST1);
	InitTownerAnim(numtowners, "Towners\\Priest\\Priest8.CEL", 4, -1);
	numtowners++;
}

static void InitCows()
{
	/** Specifies the start X-coordinates of the cows. */
	const BYTE TownCowX[] = { 48 + DBORDERX, 46 + DBORDERX, 49 + DBORDERX };
	/** Specifies the start Y-coordinates of the cows. */
	const BYTE TownCowY[] = {  6 + DBORDERY,  4 + DBORDERY, 10 + DBORDERY };
	/** Specifies the start directions of the cows. */
	const BYTE TownCowDir[] = { DIR_SW, DIR_NW, DIR_N };
	/** Maps from direction to X-coordinate delta, which is used when
	 * placing cows in Tristram. A single cow may require space of up
	 * to four tiles when being placed on the map.
	 */
	const int8_t cowoffx[NUM_DIRS] = { -1, 0, -1, -1, -1, 0, -1, -1 };
	/** Maps from direction to Y-coordinate delta, which is used when
	 * placing cows in Tristram. A single cow may require space of up
	 * to four tiles when being placed on the map.
	 */
	const int8_t cowoffy[NUM_DIRS] = { -1, -1, -1, 0, -1, -1, -1, 0 };
	/** Specifies the offsets from the cows to reserve space on the map. */
	const int8_t TownCowXOff[] = { cowoffx[TownCowDir[0]], cowoffx[TownCowDir[1]], cowoffx[TownCowDir[2]] };
	const int8_t TownCowYOff[] = { cowoffy[TownCowDir[0]], cowoffy[TownCowDir[1]], cowoffy[TownCowDir[2]] };
	const BYTE* cowAnims[NUM_DIRS];
	int i, dir;
	int x, y, xo, yo;

	assert(pCowCels == NULL);
	pCowCels = LoadFileInMem("Towners\\Animals\\Cow.CEL");
	LoadFrameGroups(pCowCels, cowAnims);
	static_assert(lengthof(TownCowX) == lengthof(TownCowY), "Mismatching TownCow tables I.");
	static_assert(lengthof(TownCowX) == lengthof(TownCowDir), "Mismatching TownCow tables II.");
	for (i = 0; i < lengthof(TownCowX); i++) {
		x = TownCowX[i];
		y = TownCowY[i];
		dir = TownCowDir[i];
		InitTownerInfo(numtowners, "Cow", TOWN_COW, x, y, 3);
		InitCowAnim(numtowners, cowAnims[dir]);

		xo = x + TownCowXOff[i];
		yo = y + TownCowYOff[i];

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
	InitTownerInfo(numtowners, "Lester the farmer", TOWN_FARMER, TPOS_FARMER, 7);
	InitTownerAnim(numtowners, "Towners\\Farmer\\Farmrn2.CEL", 3, -1);
	numtowners++;
}

static void InitCowFarmer()
{
	const char* pAnimFile;

	InitTownerInfo(numtowners, "Complete Nut", TOWN_COWFARM, TPOS_COWFARM, 7);
	pAnimFile = quests[Q_JERSEY]._qactive != QUEST_DONE ? "Towners\\Farmer\\cfrmrn2.CEL" : "Towners\\Farmer\\mfrmrn2.CEL";
	InitTownerAnim(numtowners, pAnimFile, 3, -1);
	numtowners++;
}

static void InitGirl()
{
	const char* pAnimFile;

	InitTownerInfo(numtowners, "Celia", TOWN_GIRL, TPOS_GIRL, 3);
	pAnimFile = quests[Q_GIRL]._qactive != QUEST_DONE ? "Towners\\Girl\\Girlw1.CEL" : "Towners\\Girl\\Girls1.CEL";
	InitTownerAnim(numtowners, pAnimFile, 6, -1);
	numtowners++;
}
#endif

void InitTowners()
{
	assert(nummonsters == MAX_MINIONS);
	numtowners = MAX_MINIONS;
	InitSmith();
	InitHealer();
	InitTavern();
	InitStory();
	InitDrunk();
	InitWitch();
	InitBarmaid();
	InitPegboy();
	InitPriest();
	if (quests[Q_BUTCHER]._qactive != QUEST_NOTAVAIL) { // if (quests[Q_BUTCHER]._qactive != QUEST_DONE) {
		InitDeadguy(); // in vanilla game the dead body was gone after the quest is completed, but it might cause de-sync
	}
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

	for (i = MAX_MINIONS; i < numtowners; i++) {
		if (monsters[i]._mType == TOWN_COW) { // TNR_TYPE
			monsters[i]._mAnimData = NULL;    // TNR_ANIM_DATA
		} else {
			MemFreeConst(monsters[i]._mAnimData);
		}
	}
	MemFreeDbg(pCowCels);

	numtowners = 0;
}

void ProcessTowners()
{
	MonsterStruct* tw;
	int i, ao;

	for (i = MAX_MINIONS; i < numtowners; i++) {
		tw = &monsters[i];
		if (tw->_mType == TOWN_DEADGUY) { // TNR_TYPE
			if (quests[Q_BUTCHER]._qactive != QUEST_INIT) {
				//if (quests[Q_BUTCHER]._qactive != QUEST_ACTIVE || quests[Q_BUTCHER]._qlog) {
					if (!gbQtextflag) {
						//tw->_mAnimFrameLen = 1000;
						tw->_mAnimFrame = 1;           // TNR_ANIM_FRAME
						tw->_mName = "Slain Townsman"; // TNR_NAME
					}
					continue; //tw->_tAnimCnt = 0;
				/*} else {
					if (gbQtextflag)
						tw->_tAnimCnt = 0; // TNR_ANIM_CNT
				}*/
			}
		}

		tw->_mAnimCnt++; // TNR_ANIM_CNT
		if (tw->_mAnimCnt >= tw->_mAnimFrameLen) { // TNR_ANIM_FRAME_LEN
			tw->_mAnimCnt = 0;

			if (tw->_mVar1 >= 0) { // TNR_ANIM_ORDER
				ao = tw->_mVar1;
				tw->_mVar2++; // TNR_ANIM_FRAME_CNT
				if (AnimOrder[ao][tw->_mVar2] == -1)
					tw->_mVar2 = 0;

				tw->_mAnimFrame = AnimOrder[ao][tw->_mVar2]; // TNR_ANIM_FRAME, TNR_ANIM_FRAME_CNT
			} else {
				tw->_mAnimFrame++;                   // TNR_ANIM_FRAME
				if (tw->_mAnimFrame > tw->_mAnimLen) // TNR_ANIM_LEN
					tw->_mAnimFrame = 1;
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

void SyncTownerQ(int pnum, int idx)
{
	int i;

	// assert(plr._pmode != PM_DEATH);
	net_assert(plr._pDunLevel == DLV_TOWN);

	if (!PlrHasStorageItem(pnum, idx, &i))
		return;

	switch (idx) {
	case IDI_BANNER:
		if (quests[Q_BANNER]._qvar1 >= QV_BANNER_GIVEN)
			return;
		quests[Q_BANNER]._qlog = FALSE;
		quests[Q_BANNER]._qvar1 = QV_BANNER_GIVEN;
		if (pnum == mypnum)
			SpawnUnique(UITEM_HARCREST, TPOS_TAVERN, ICM_SEND_FLIP);
		break;
	case IDI_ROCK:
		if (quests[Q_ROCK]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_ROCK]._qactive = QUEST_DONE;
		//quests[Q_ROCK]._qlog = FALSE;
		if (pnum == mypnum)
			SpawnUnique(UITEM_INFRARING, TPOS_SMITH, ICM_SEND_FLIP);
		break;
	case IDI_ANVIL:
		if (quests[Q_ANVIL]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_ANVIL]._qactive = QUEST_DONE;
		//quests[Q_ANVIL]._qlog = FALSE;
		if (pnum == mypnum)
			SpawnUnique(UITEM_GRISWOLD, TPOS_SMITH, ICM_SEND_FLIP);
		break;
	case IDI_FUNGALTM:
		if (quests[Q_MUSHROOM]._qactive != QUEST_INIT)
			return;
		quests[Q_MUSHROOM]._qactive = QUEST_ACTIVE;
		quests[Q_MUSHROOM]._qlog = TRUE;
		quests[Q_MUSHROOM]._qvar1 = QV_MUSHROOM_TOMEGIVEN;
		break;
	case IDI_MUSHROOM:
		if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 >= QV_MUSHROOM_MUSHGIVEN)
			return;
		quests[Q_MUSHROOM]._qvar1 = QV_MUSHROOM_MUSHGIVEN;
		quests[Q_MUSHROOM]._qmsg = TEXT_MUSH10;
		break;
	case IDI_BRAIN:
		if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 >= QV_MUSHROOM_BRAINGIVEN)
			return;
		quests[Q_MUSHROOM]._qvar1 = QV_MUSHROOM_BRAINGIVEN;
		quests[Q_MUSHROOM]._qmsg = TEXT_MUSH4;
		if (pnum == mypnum)
			SpawnQuestItemAt(IDI_SPECELIX, TPOS_HEALER, ICM_SEND_FLIP);
		break;
	case IDI_LAZSTAFF:
		if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_STAFFGIVEN /*|| quests[Q_BETRAYER]._qactive != QUEST_ACTIVE*/)
			return;
		quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_STAFFGIVEN;
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
			if (pnum == mypnum)
				SpawnUnique(UITEM_BOVINE, TPOS_COWFARM, ICM_SEND_FLIP);
			ReInitTownerAnim(TOWN_COWFARM, "Towners\\Farmer\\mfrmrn2.CEL");
		}
		break;
	case IDI_THEODORE:
		if (quests[Q_GIRL]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_GIRL]._qactive = QUEST_DONE;
		// quests[Q_GIRL]._qlog = FALSE;
		if (currLvl._dLevelIdx == DLV_TOWN) {
			if (pnum == mypnum) {
				uint16_t wCI = plr._pInvList[i]._iCreateInfo; // the amulet inherits the level of THEODORE
				SetRndSeed(plr._pInvList[i]._iSeed);          // and uses its seed
				SpawnAmulet(wCI, TPOS_GIRL/*, true*/);
			}
			ReInitTownerAnim(TOWN_GIRL, "Towners\\Girl\\Girls1.CEL");
		}
		break;
#endif
	default:
		return;
	}

	SyncPlrStorageRemove(pnum, i);
}

void TalkToTowner(int tnum, int pnum)
{
	MonsterStruct* tw;
	int i, qt, qn;

	tw = &monsters[tnum];
	if (pnum != mypnum) {
		return;
	}
	if (gbQtextflag) {
		return;
	}

	if (pcursicon >= CURSOR_FIRSTITEM) {
		return;
	}

	qt = TEXT_NONE;
	qn = Q_INVALID;

	switch (tw->_mType) { // TNR_TYPE
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
	case TOWN_HEALER:
		if (quests[Q_PWATER]._qactive == QUEST_INIT && quests[Q_PWATER]._qvar1 != QV_PWATER_CLEAN && IsLvlVisited(DLV_CATHEDRAL2)) {
			quests[Q_PWATER]._qactive = QUEST_ACTIVE;
			quests[Q_PWATER]._qlog = TRUE;
			// quests[Q_PWATER]._qmsg = TEXT_POISON3;
			// quests[Q_PWATER]._qvar1 = 1;
			qn = Q_PWATER;
			qt = TEXT_POISON3;
		} else if ((quests[Q_PWATER]._qactive == QUEST_INIT || quests[Q_PWATER]._qactive == QUEST_ACTIVE)
		 && quests[Q_PWATER]._qvar1 == QV_PWATER_CLEAN) {
			quests[Q_PWATER]._qactive = QUEST_DONE;
			SpawnUnique(UITEM_TRING, TPOS_HEALER, ICM_SEND_FLIP);
			qn = Q_PWATER;
			qt = TEXT_POISON5;
		} else if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE
		 && quests[Q_MUSHROOM]._qvar1 < QV_MUSHROOM_BRAINGIVEN) {
			if (PlrHasStorageItem(pnum, IDI_BRAIN, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_BRAIN);
				qt = TEXT_MUSH4;
			} else if (quests[Q_MUSHROOM]._qvar1 >= QV_MUSHROOM_MUSHGIVEN && quests[Q_MUSHROOM]._qvar2 != TEXT_MUSH3) {
				quests[Q_MUSHROOM]._qvar2 = TEXT_MUSH3;
				qt = TEXT_MUSH3;
			}
		}
		break;
	case TOWN_TAVERN:
		if (!IsLvlVisited(DLV_CATHEDRAL1) && plr._pLevel == 1 && quests[Q_DIABLO]._qvar2 == 0) {
			quests[Q_DIABLO]._qvar2 = 1;
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
		if (IsLvlVisited(DLV_CATHEDRAL4) && quests[Q_BANNER]._qactive != QUEST_NOTAVAIL) {
			if (quests[Q_BANNER]._qactive == QUEST_INIT) {
				quests[Q_BANNER]._qactive = QUEST_ACTIVE;
				quests[Q_BANNER]._qlog = TRUE;
				qn = Q_BANNER;
				qt = TEXT_BANNER2;
			} else if (quests[Q_BANNER]._qactive == QUEST_ACTIVE && PlrHasStorageItem(pnum, IDI_BANNER, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_BANNER);
				qt = TEXT_BANNER3;
			}
		}
		break;
	case TOWN_STORY:
		if (!IsMultiGame) {
			if (quests[Q_BETRAYER]._qactive == QUEST_INIT && PlrHasStorageItem(pnum, IDI_LAZSTAFF, &i)) {
				NetSendCmdParam1(CMD_QTOWNER, IDI_LAZSTAFF);
				qt = TEXT_VILE1;
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == QV_BETRAYER_DEAD) {
				quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_CAIN;
				quests[Q_DIABLO]._qlog = TRUE;
				qt = TEXT_VILE3;
			}
		} else {
			if (quests[Q_BETRAYER]._qactive == QUEST_INIT && IsLvlVisited(DLV_HELL2)) {
				quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
				quests[Q_BETRAYER]._qlog = TRUE;
				qn = Q_BETRAYER;
				qt = TEXT_VILE1;
			} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE && quests[Q_BETRAYER]._qvar1 == QV_BETRAYER_DEAD) {
				quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_CAIN;
				qn = Q_BETRAYER;
				qt = TEXT_VILE3;
				quests[Q_DIABLO]._qlog = TRUE;
				NetSendCmdQuest(Q_DIABLO, false);
			}
		}
		break;
	case TOWN_DRUNK:
		break;
	case TOWN_WITCH:
		if (quests[Q_MUSHROOM]._qactive == QUEST_INIT && PlrHasStorageItem(pnum, IDI_FUNGALTM, &i)) {
			NetSendCmdParam1(CMD_QTOWNER, IDI_FUNGALTM);
			qt = TEXT_MUSH8;
		} else if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE) {
			if (quests[Q_MUSHROOM]._qvar1 < QV_MUSHROOM_MUSHGIVEN) {
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
	case TOWN_BARMAID:
#ifdef HELLFIRE
		if (PlrHasStorageItem(pnum, IDI_FANG, &i) && quests[Q_GRAVE]._qactive == QUEST_INIT) {
			quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
			quests[Q_GRAVE]._qlog = TRUE;
			//quests[Q_GRAVE]._qmsg = TEXT_GRAVE8;
			qn = Q_GRAVE;
			qt = TEXT_GRAVE8;
		}
#endif
		break;
	case TOWN_PEGBOY:
	case TOWN_PRIEST:
		break;
	case TOWN_DEADGUY:
		switch (quests[Q_BUTCHER]._qactive) {
		case QUEST_INIT:
			quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
			quests[Q_BUTCHER]._qlog = TRUE;
			// quests[Q_BUTCHER]._qmsg = TEXT_BUTCH9;
			//quests[Q_BUTCHER]._qvar1 = 1;
			qn = Q_BUTCHER;
			qt = TEXT_BUTCH9;
			break;
		case QUEST_ACTIVE:
			i = sgSFXSets[SFXS_PLR_08][plr._pClass];
			if (!IsSfxStreaming(i)) {
				// tw->_mListener = pnum;  // TNR_LISTENER
				PlaySfx(i);
			}
			break;
		case QUEST_DONE:
			i = sgSFXSets[SFXS_PLR_09][plr._pClass];
			if (!IsSfxStreaming(i)) {
				// tw->_mListener = pnum;  // TNR_LISTENER
				PlaySfx(i);
			}
			break;
		}
		break;
#ifdef HELLFIRE
	case TOWN_FARMER:
		switch (quests[Q_FARMER]._qactive) {
		case QUEST_NOTAVAIL:
			break;
		case QUEST_INIT:
			if (PlrHasStorageItem(pnum, IDI_RUNEBOMB, &i)) {
				quests[Q_FARMER]._qactive = QUEST_ACTIVE;
				quests[Q_FARMER]._qvar1 = QV_FARMER_BOMBGIVEN;
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
				quests[Q_FARMER]._qvar1 = QV_FARMER_BOMBGIVEN;
				quests[Q_FARMER]._qlog = TRUE;
				// quests[Q_FARMER]._qmsg = TEXT_FARMER1;
				qn = Q_FARMER;
				qt = TEXT_FARMER1;
				SpawnQuestItemAt(IDI_RUNEBOMB, TPOS_FARMER, ICM_SEND_FLIP);
			}
			break;
		case QUEST_ACTIVE:
			qt = PlrHasStorageItem(pnum, IDI_RUNEBOMB, &i) ? TEXT_FARMER2 : TEXT_FARMER3;
			break;
		case QUEST_DONE:
			if (quests[Q_FARMER]._qlog && quests[Q_FARMER]._qvar1 == pnum + QV_FARMER_BOMBUSED) {
				quests[Q_FARMER]._qlog = FALSE;
				qn = Q_FARMER; // not necessary...
				qt = TEXT_FARMER4;
				SpawnQuestItemAt(IDI_MANA, TPOS_FARMER, ICM_SEND_FLIP);
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
				quests[Q_JERSEY]._qvar1 = QV_JERSEY_BOMBGIVEN;
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
			if (quests[Q_JERSEY]._qvar1 != QV_INIT) {
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
				quests[Q_JERSEY]._qvar1 = QV_JERSEY_BOMBGIVEN;
				// quests[Q_JERSEY]._qmsg = TEXT_JERSEY4;
				quests[Q_JERSEY]._qlog = TRUE;
				qn = Q_JERSEY;
				qt = TEXT_JERSEY4;
				SpawnQuestItemAt(IDI_RUNEBOMB, TPOS_COWFARM, ICM_SEND_FLIP);
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
		if (quests[Q_GIRL]._qactive == QUEST_ACTIVE && PlrHasStorageItem(pnum, IDI_THEODORE, &i)) {
			NetSendCmdParam1(CMD_QTOWNER, IDI_THEODORE);
			qt = TEXT_GIRL4;
		} else if (quests[Q_GIRL]._qactive != QUEST_DONE) {
			if (quests[Q_GIRL]._qvar1 == QV_INIT) {
				if (quests[Q_GIRL]._qvar2++ == 0) {
					qt = TEXT_GIRL1;
				} else {
					qt = TEXT_GIRL2;
					quests[Q_GIRL]._qactive = QUEST_ACTIVE;
					quests[Q_GIRL]._qvar1 = QV_GIRL_TALK1;
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
	case TOWN_COW:
		CowSFX(tw, pnum);
		return;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (qn != Q_INVALID)
		NetSendCmdQuest(qn, false);
	if (qt != TEXT_NONE) {
		// tw->_mListener = pnum; // TNR_LISTENER
		StartQTextMsg(qt);
	} else if (tw->_mgoalvar1 != STORE_NONE) { // TNR_STORE
		// assert(!gbQtextflag);
		ClearPanels();
		gamemenu_off();
		StartQTextMsg(tw->_mgoalvar2); // TNR_TALK, TALK_MESSAGE
		StartStore(tw->_mgoalvar1);    // TNR_STORE
	}
	_guCowClicks = 0;
	_guCowMsg = 0;
}

DEVILUTION_END_NAMESPACE
