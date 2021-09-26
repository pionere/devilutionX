/**
 * @file quests.cpp
 *
 * Implementation of functionality for handling quests.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define QPNL_LINE_SPACING	24
#define QPNL_BORDER			10
#define QPNL_MAXENTRIES		((SPANEL_HEIGHT - 2 * QPNL_BORDER) / QPNL_LINE_SPACING)
#define QPNL_LINE_WIDTH		(SPANEL_WIDTH - 2 * QPNL_BORDER)
#define QPNL_TEXT_HEIGHT	12

/** Contains the quests of the current game. */
QuestStruct quests[NUM_QUESTS];
/** Quest-log panel CEL */
BYTE* pQLogCel;
/** the entries of the quest-log panel (quest_id) */
BYTE qlist[QPNL_MAXENTRIES];
/** Specifies whether the quest-log panel is shown. */
bool gbQuestlog;
/** the index of the first valid line on the quest-log panel */
unsigned qtopline;
/** the number of valid lines on the quest-log panel */
unsigned numqlines;
/** the index of the selected line on the quest-log panel */
unsigned qline;
BYTE gbTownWarps;
BYTE gbWaterDone;
BYTE gbDungMsgs;
BYTE gbDungMsgs2;
static_assert(NUM_LEVELS <= 32, "guLvlVisited can not maintain too many levels.");
uint32_t guLvlVisited;
int gnReturnLvlX;
int gnReturnLvlY;
int gnReturnLvl;

/**
 * A quest group containing the three quests the Butcher,
 * Ogden's Sign and Gharbad the Weak, which ensures that exactly
 * two of these three quests appear in any game.
 */
const int QuestGroup1[3] = { Q_BUTCHER, Q_LTBANNER, Q_GARBUD };
/**
 * A quest group containing the three quests Halls of the Blind,
 * the Magic Rock and Valor, which ensures that exactly two of
 * these three quests appear in any game.
 */
const int QuestGroup2[3] = { Q_BLIND, Q_ROCK, Q_BLOOD };
/**
 * A quest group containing the three quests Black Mushroom,
 * Zhar the Mad and Anvil of Fury, which ensures that exactly
 * two of these three quests appear in any game.
 */
const int QuestGroup3[3] = { Q_MUSHROOM, Q_ZHAR, Q_ANVIL };
/**
 * A quest group containing the two quests Lachdanan and Warlord
 * of Blood, which ensures that exactly one of these two quests
 * appears in any game.
 */
const int QuestGroup4[2] = { Q_VEIL, Q_WARLORD };
#ifdef HELLFIRE
/**
 * A quest group containing the two quests CowFarmer and Farmer,
 * which ensures that exactly one of these two quests
 * appears in any game.
 */
const int QuestGroup5[2] = { Q_JERSEY, Q_FARMER };
#endif

void InitQuestGFX()
{
	gbQuestlog = false;

	gbTownWarps = 0;
	gbWaterDone = 0;
	gbDungMsgs = 0;
	gbDungMsgs2 = 0;
	guLvlVisited = 0;

	pQLogCel = LoadFileInMem("Data\\Quest.CEL");
}

void FreeQuestGFX()
{
	MemFreeDbg(pQLogCel);
}

void InitQuests()
{
	QuestStruct *qs;
	const QuestData *qdata;
	int i;

	qs = quests;
	qdata = questlist;
	for (i = 0; i < NUM_QUESTS; i++, qs++, qdata++) {
		qs->_qactive = QUEST_INIT;
		qs->_qvar1 = 0;
		qs->_qvar2 = 0;
		qs->_qlog = FALSE;
		qs->_qtx = 0;
		qs->_qty = 0;
		qs->_qmsg = qdata->_qdmsg;
	}

	SetRndSeed(glSeedTbl[DLV_HELL3]);
	quests[Q_DIABLO]._qvar1 = random_(0, 3);
#ifdef _DEBUG
	if (!allquests) {
#endif
		if (random_(0, 2) != 0)
			quests[Q_PWATER]._qactive = QUEST_NOTAVAIL;
		else
			quests[Q_SKELKING]._qactive = QUEST_NOTAVAIL;
#ifdef HELLFIRE
		if (random_(0, 2) != 0)
			quests[Q_GIRL]._qactive = QUEST_NOTAVAIL;
#endif

		quests[QuestGroup1[random_(0, lengthof(QuestGroup1))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup2[random_(0, lengthof(QuestGroup2))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup3[random_(0, lengthof(QuestGroup3))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup4[random_(0, lengthof(QuestGroup4))]]._qactive = QUEST_NOTAVAIL;
#ifdef HELLFIRE
		quests[QuestGroup5[random_(0, lengthof(QuestGroup5))]]._qactive = QUEST_NOTAVAIL;
#endif
#ifdef _DEBUG
	}
	if (questdebug != -1)
		quests[questdebug]._qactive = QUEST_ACTIVE;
#endif

	if (IsMultiGame)
		quests[Q_BETRAYER]._qvar1 = 2;
	if (quests[Q_PWATER]._qactive == QUEST_NOTAVAIL)
		quests[Q_PWATER]._qvar1 = 2;
}

void CheckQuests()
{
	if (currLvl._dLevelIdx == SL_POISONWATER) {
		if (quests[Q_PWATER]._qvar1 != 2
		 && nummonsters == MAX_MINIONS) {
			quests[Q_PWATER]._qvar1 = 2;
			NetSendCmdQuest(Q_PWATER, true);
			PlaySfxLoc(IS_QUESTDN, myplr._px, myplr._py);
			gbWaterDone = 32;
			//quests[Q_PWATER]._qvar2 = 1; // LOADWATERPAL
			LoadPalette("Levels\\L3Data\\L3pwater.pal");
		}
		if (gbWaterDone > 0) {
			//if (quests[Q_PWATER]._qvar2 == 1) { // LOADWATERPAL
			//	quests[Q_PWATER]._qvar2 = 2;
			//	LoadPalette("Levels\\L3Data\\L3pwater.pal");
			//}
			palette_update_quest_palette(gbWaterDone);
			gbWaterDone--;
		}
	}
}

int ForceQuests()
{
	int i;

	for (i = 0; i < numtrigs; i++) {
		if (trigs[i]._tmsg == DVL_DWM_SETLVL) {
			/*      ^
			 *      |
			 * ----++------>
			 *    +++
			 *    ++|
			 *      |
			 */
			int dx = cursmx - (trigs[i]._tx - 1);
			int dy = cursmy - (trigs[i]._ty - 1);
			if (abs(dx) <= 1 && abs(dy) <= 1 &&	// select the 3x3 square around (-1;-1)
				abs(dx - dy) < 2) {				// exclude the top left and bottom right positions
				cursmx = trigs[i]._tx;
				cursmy = trigs[i]._ty;
				return i;
			}
		}
	}

	return -1;
}

bool QuestStatus(int qn)
{
	if (currLvl._dLevelIdx == questlist[qn]._qdlvl
	 && quests[qn]._qactive != QUEST_NOTAVAIL) {
		assert(!currLvl._dSetLvl);
		return true;
	}
	return false;
}

void CheckQuestKill(int mnum, bool sendmsg)
{
	int qn;

	switch (monsters[mnum]._uniqtype - 1) {
	case UMT_GARBUD: //"Gharbad the Weak"
		quests[Q_GARBUD]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = TEXT_QM_GARBUD;
		qn = Q_GARBUD;
		break;
	case UMT_SKELKING:
		quests[Q_SKELKING]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = TEXT_QM_SKING;
		qn = Q_SKELKING;
		break;
	case UMT_ZHAR: //"Zhar the Mad"
		quests[Q_ZHAR]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = TEXT_QM_ZHAR;
		qn = Q_ZHAR;
		break;
	case UMT_LAZARUS: //"Arch-Bishop Lazarus" - multi
		if (IsMultiGame) {
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = 7;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;

			InitTriggers();
			if (sendmsg) {
				NetSendCmdQuest(Q_DIABLO, false); // recipient should not matter
			}
		} else { //"Arch-Bishop Lazarus" - single
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = 7;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;
			InitVPReturnTrigger(false);
		}
		sfxdelay = 30;
		sfxdnum = TEXT_QM_LAZARUS;
		qn = Q_BETRAYER;
		break;
	case UMT_WARLORD: //"Warlord of Blood"
		quests[Q_WARLORD]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = TEXT_QM_WARLORD;
		qn = Q_WARLORD;
		break;
	case UMT_BUTCHER:
		quests[Q_BUTCHER]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = TEXT_QM_BUTCHER;
		qn = Q_BUTCHER;
		break;
#ifdef HELLFIRE
	case UMT_NAKRUL:
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
		qn = Q_NAKRUL;
		break;
#endif
	default:
		return;
	}
	if (sendmsg)
		NetSendCmdQuest(qn, false); // recipient should not matter
}

static void DrawButcher()
{
	int x, y;

	assert(setpc_w == 6);
	assert(setpc_h == 6);
	x = 2 * setpc_x + DBORDERX;
	y = 2 * setpc_y + DBORDERY;
	// fix transVal on the bottom left corner of the room
	DRLG_CopyTrans(x, y + 9, x + 1, y + 9);
	DRLG_CopyTrans(x, y + 10, x + 1, y + 10);
	// set transVal in the room
	DRLG_RectTrans(x + 3, y + 3, x + 10, y + 10);
}

static void DrawSkelKing()
{
	int x, y;

	x = 2 * setpc_x + DBORDERX;
	y = 2 * setpc_y + DBORDERY;
	// fix transVal on the bottom left corner of the box
	DRLG_CopyTrans(x, y + 11, x + 1, y + 11);
	DRLG_CopyTrans(x, y + 12, x + 1, y + 12);
	// fix transVal at the entrance - commented out because it makes the wall transparent
	//DRLG_CopyTrans(x + 13, y + 7, x + 12, y + 7);
	//DRLG_CopyTrans(x + 13, y + 8, x + 12, y + 8);
	// patch dSolidTable - L1.SOL
	nSolidTable[299] = true;

	quests[Q_SKELKING]._qtx = x + 12;
	quests[Q_SKELKING]._qty = y + 7;
}

static void DrawMap(const char* name, int bv)
{
	int x, y, i, j, v;
	BYTE *pMap;
	uint16_t rw, rh, *lm;

	pMap = LoadFileInMem(name);
	lm = (uint16_t *)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;
	assert(setpc_w == rw);
	assert(setpc_h == rh);
	x = setpc_x;
	y = setpc_y;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			if (*lm != 0) {
				v = SwapLE16(*lm);
			} else {
				v = bv;
			}
			dungeon[i][j] = v;
			lm++;
		}
	}
	mem_free_dbg(pMap);
}

static void DrawWarLord()
{
	DrawMap("Levels\\L4Data\\Warlord2.DUN", 6);
}

static void DrawSChamber()
{
	quests[Q_SCHAMB]._qtx = 2 * setpc_x + DBORDERX + 6;
	quests[Q_SCHAMB]._qty = 2 * setpc_y + DBORDERY + 7;

	DrawMap("Levels\\L2Data\\Bonestr1.DUN", 3);
	// 'patch' the map to place shadows
	// shadow of the external-left column
	dungeon[setpc_x][setpc_y + 4] = 48;
	dungeon[setpc_x][setpc_y + 5] = 50;
}

static void DrawPreMap(const char* name)
{
	int x, y, i, j;
	BYTE *pMap;
	uint16_t rw, rh, *lm;

	pMap = LoadFileInMem(name);
	lm = (uint16_t *)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;
	assert(setpc_w == rw);
	assert(setpc_h == rh);
	x = setpc_x;
	y = setpc_y;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			if (*lm != 0) {
				pdungeon[i][j] = SwapLE16(*lm);
			}
			lm++;
		}
	}
	mem_free_dbg(pMap);
}

static void DrawLTBanner()
{
	DrawPreMap("Levels\\L1Data\\Banner1.DUN");
}

static void DrawBlind()
{
	DrawMap("Levels\\L2Data\\Blind2.DUN", 3);
	// 'patch' the map to replace the door with wall
	dungeon[setpc_x + 4][setpc_y + 3] = 25;
}

static void DrawBlood()
{
	DrawMap("Levels\\L2Data\\Blood2.DUN", 3);
	// 'patch' the map to place pieces with closed doors
	dungeon[setpc_x + 4][setpc_y + 10] = 151;
	dungeon[setpc_x + 4][setpc_y + 15] = 151;
	dungeon[setpc_x + 5][setpc_y + 15] = 151;
	// 'patch' the map to place shadows
	// shadow of the external-left column -- do not place to prevent overwriting large decorations
	//dungeon[setpc_x - 1][setpc_y + 7] = 48;
	//dungeon[setpc_x - 1][setpc_y + 8] = 50;
	// shadow of the bottom-left column(s) -- one is missing
	dungeon[setpc_x + 1][setpc_y + 13] = 48;
	dungeon[setpc_x + 1][setpc_y + 14] = 50;
	// shadow of the internal column next to the pedistal
	dungeon[setpc_x + 5][setpc_y + 7] = 142;
	dungeon[setpc_x + 5][setpc_y + 8] = 50;
}

#ifdef HELLFIRE
static void DrawNakrul()
{
	DrawPreMap("NLevels\\L5Data\\Nakrul2.DUN");
}
#endif

void DRLG_CheckQuests()
{
	int i;

	for (i = 0; i < NUM_QUESTS; i++) {
		if (QuestStatus(i)) {
			switch (i) {
			case Q_BUTCHER:
				DrawButcher();
				break;
			case Q_LTBANNER:
				DrawLTBanner();
				break;
			case Q_BLIND:
				DrawBlind();
				break;
			case Q_BLOOD:
				DrawBlood();
				break;
			case Q_WARLORD:
				DrawWarLord();
				break;
			case Q_SKELKING:
				DrawSkelKing();
				break;
			case Q_SCHAMB:
				DrawSChamber();
				break;
#ifdef HELLFIRE
			case Q_NAKRUL:
				DrawNakrul();
				break;
#endif
			}
		}
	}
}

void SetReturnLvlPos()
{
	switch (myplr._pDunLevel) {
	case SL_SKELKING:
		gnReturnLvlX = quests[Q_SKELKING]._qtx + 1;
		gnReturnLvlY = quests[Q_SKELKING]._qty;
		gnReturnLvl = questlist[Q_SKELKING]._qdlvl;
		break;
	case SL_BONECHAMB:
		gnReturnLvlX = quests[Q_SCHAMB]._qtx + 1;
		gnReturnLvlY = quests[Q_SCHAMB]._qty;
		gnReturnLvl = questlist[Q_SCHAMB]._qdlvl;
		break;
	case SL_POISONWATER:
		gnReturnLvlX = quests[Q_PWATER]._qtx;
		gnReturnLvlY = quests[Q_PWATER]._qty + 1;
		gnReturnLvl = questlist[Q_PWATER]._qdlvl;
		break;
	case SL_VILEBETRAYER:
		gnReturnLvlX = quests[Q_BETRAYER]._qtx + 1;
		gnReturnLvlY = quests[Q_BETRAYER]._qty - 1;
		gnReturnLvl = questlist[Q_BETRAYER]._qdlvl;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void GetReturnLvlPos()
{
	ViewX = gnReturnLvlX;
	ViewY = gnReturnLvlY;
	//EnterLevel(gnReturnLvl);
}

void LoadPWaterPalette()
{
	// TODO: this is ugly...
	if (currLvl._dLevelIdx != SL_POISONWATER)
		return;

	if (quests[Q_PWATER]._qvar1 == 2) {
		//if (gbWaterDone == 0)
			LoadPalette("Levels\\L3Data\\L3pwater.pal");
		//else
		//	quests[Q_PWATER]._qvar2 = 1; // LOADWATERPAL
	}
	//else
	//	LoadPalette("Levels\\L3Data\\L3pfoul.pal");
}

static void ResyncBanner()
{
	if (quests[Q_LTBANNER]._qvar1 != 4) {
		// open the entrance of the setmap -> TODO: add these to Banner2.DUN ?
		ObjChangeMapResync(
		    setpc_w + setpc_x - 2,
		    setpc_h + setpc_y - 2,
		    setpc_w + setpc_x + 1,
		    setpc_h + setpc_y + 1);
		// TODO: add the opening of the entrance to Banner2.DUN?
	} else {
		ObjChangeMapResync(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
		//for (i = 0; i < numobjects; i++)
		//	SyncObjectAnim(objectactive[i]);
		BYTE tv = dTransVal[2 * setpc_x + 1 + DBORDERX][2 * (setpc_y + 6) + 1 + DBORDERY];
		DRLG_MRectTrans(setpc_x, setpc_y + 3, setpc_x + setpc_w - 1, setpc_y + setpc_h - 1, tv);
	}
}

void ResyncQuests()
{
	//int i;

	if (QuestStatus(Q_LTBANNER)) {
		ResyncBanner();
		/*if (quests[Q_LTBANNER]._qvar1 == 1)
			ObjChangeMapResync(
			    setpc_w + setpc_x - 2,
			    setpc_h + setpc_y - 2,
			    setpc_w + setpc_x + 1,
			    setpc_h + setpc_y + 1);
		if (quests[Q_LTBANNER]._qvar1 == 2) {
			ObjChangeMapResync(
			    setpc_w + setpc_x - 2,
			    setpc_h + setpc_y - 2,
			    setpc_w + setpc_x + 1,
			    setpc_h + setpc_y + 1);
			ObjChangeMapResync(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 2, (setpc_h >> 1) + setpc_y - 2);
			for (i = 0; i < numobjects; i++)
				SyncObjectAnim(objectactive[i]);
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
		}
		if (quests[Q_LTBANNER]._qvar1 == 3) {
			ObjChangeMapResync(setpc_x, setpc_y, setpc_x + setpc_w + 1, setpc_y + setpc_h + 1);
			for (i = 0; i < numobjects; i++)
				SyncObjectAnim(objectactive[i]);
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
		}*/
	}
	if (IsMultiGame) {
		// TODO: eliminate relative level-indices?
		//if (quests[Q_SKELKING]._qactive == QUEST_INIT
		//    && currLvl._dLevelIdx >= questlist[Q_SKELKING]._qdlvl - 1
		//    && currLvl._dLevelIdx <= questlist[Q_SKELKING]._qdlvl + 1) {
		//	quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
		//	NetSendCmdQuest(Q_SKELKING, false); // recipient should not matter
		//}
		//if (quests[Q_BUTCHER]._qactive == QUEST_INIT
		//	&& currLvl._dLevelIdx >= questlist[Q_BUTCHER]._qdlvl - 1
		//	&& currLvl._dLevelIdx <= questlist[Q_BUTCHER]._qdlvl + 1) {
		//	quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
		//	NetSendCmdQuest(Q_BUTCHER, false); // recipient should not matter
		//}
		if (quests[Q_BETRAYER]._qactive == QUEST_INIT && currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl - 1) {
			quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
			NetSendCmdQuest(Q_BETRAYER, false); // recipient should not matter
		}
	} else {
		if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
			if (quests[Q_BETRAYER]._qvar1 >= 4)
				ObjChangeMapResync(1, 11, 20, 18);
			if (quests[Q_BETRAYER]._qvar1 >= 6)
				ObjChangeMapResync(1, 18, 20, 24);
			if (quests[Q_BETRAYER]._qvar1 >= 7)
				InitVPReturnTrigger(true);
			//for (i = 0; i < numobjects; i++)
			//	SyncObjectAnim(objectactive[i]);
		}
		if (currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl) {
			if (quests[Q_BETRAYER]._qvar1 >= 2) {
				if (quests[Q_BETRAYER]._qvar1 == 2) {
					quests[Q_BETRAYER]._qvar1 = 3;
					InitVPEntryTrigger(false);
				} else {
					InitVPEntryTrigger(true);
				}
			}
		}
	}
#ifdef HELLFIRE
	if (quests[Q_DEFILER]._qactive == QUEST_INIT && currLvl._dLevelIdx == questlist[Q_DEFILER]._qdlvl) {
		quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
		quests[Q_DEFILER]._qlog = TRUE;
		NetSendCmdQuest(Q_DEFILER, false); // recipient should not matter
	}
	//if (quests[Q_NAKRUL]._qactive == QUEST_INIT && currLvl._dLevelIdx == questlist[Q_NAKRUL]._qdlvl - 1) {
	//	quests[Q_NAKRUL]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(Q_NAKRUL, false); // recipient should not matter
	//}
	//if (quests[Q_JERSEY]._qactive == QUEST_INIT && currLvl._dLevelIdx == questlist[Q_JERSEY]._qdlvl - 1) {
	//	quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(Q_JERSEY, false); // recipient should not matter
	//}
	if (quests[Q_GIRL]._qactive == QUEST_INIT && currLvl._dLevelIdx == questlist[Q_GIRL]._qdlvl) {
		quests[Q_GIRL]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(Q_GIRL, false); // recipient should not matter
		// TODO: send message to reinit the towners?
	}
#endif
}

static void PrintQLString(unsigned y, const char *str)
{
	int width, sx, sy, px;

	sx = /*x*/0 + QPNL_BORDER + SCREEN_X;
	sy = y * QPNL_LINE_SPACING + QPNL_BORDER + QPNL_TEXT_HEIGHT + SCREEN_Y;
	width = GetStringWidth(str);
	if (width < QPNL_LINE_WIDTH) {
		sx += (QPNL_LINE_WIDTH - width) >> 1;
	}
	px = qline == y ? sx : INT_MAX;
	sx = PrintLimitedString(sx, sy, str, QPNL_LINE_WIDTH, COL_WHITE);
	if (px != INT_MAX) {
		DrawPentSpn2(px - 20, sx + 6, sy + 1);
	}
}

void DrawQuestLog()
{
	unsigned i;

	CelDraw(SCREEN_X, SCREEN_Y + SPANEL_HEIGHT - 1, pQLogCel, 1, SPANEL_WIDTH);
	for (i = 0; i < numqlines; i++) {
		PrintQLString(qtopline + i, questlist[qlist[i]]._qlstr);
	}
	PrintQLString(QPNL_MAXENTRIES, "Close Quest Log");
}

void StartQuestlog()
{
	int i;

	numqlines = 0;
	for (i = 0; i < NUM_QUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && quests[i]._qlog) {
			qlist[numqlines] = i;
			numqlines++;
			if (numqlines == QPNL_MAXENTRIES)
				break;
		}
	}
	qline = qtopline = numqlines != 0 ? (QPNL_MAXENTRIES / 2) - (numqlines >> 1) : QPNL_MAXENTRIES;
}

void QuestlogUp()
{
	if (numqlines != 0) {
		if (qline == qtopline) {
			qline = QPNL_MAXENTRIES;
		} else if (qline == QPNL_MAXENTRIES) {
			qline = qtopline + numqlines - 1;
		} else {
			qline--;
		}
		PlaySFX(IS_TITLEMOV);
	}
}

void QuestlogDown()
{
	if (numqlines != 0) {
		if (qline == QPNL_MAXENTRIES) {
			qline = qtopline;
		} else if (qline == qtopline + numqlines - 1) {
			qline = QPNL_MAXENTRIES;
		} else {
			qline++;
		}
		PlaySFX(IS_TITLEMOV);
	}
}

void QuestlogEnter()
{
	PlaySFX(IS_TITLSLCT);
	if (/*numqlines != 0 &&*/ qline != QPNL_MAXENTRIES)
		InitQTextMsg(quests[qlist[qline - qtopline]]._qmsg);
	gbQuestlog = false;
}

void CheckQuestlog()
{
	int y;

	y = (MouseY - (QPNL_BORDER + QPNL_TEXT_HEIGHT / 2) + QPNL_LINE_SPACING / 2 + QPNL_LINE_SPACING) / QPNL_LINE_SPACING - 1;
	if (y != QPNL_MAXENTRIES) {
		y -= qtopline;
		if ((unsigned)y >= numqlines)
			return;
	}
	qline = y;
	QuestlogEnter();
}

void SetMultiQuest(int qn, int qa, int qlog, int qvar)
{
	QuestStruct *qs;

	qs = &quests[qn];
	if (qs->_qactive != QUEST_DONE) {
		if (qa > qs->_qactive)
			qs->_qactive = qa;
		qs->_qlog = qlog;
		if (qvar > qs->_qvar1)
			qs->_qvar1 = qvar;
	}
}

void PlayDungMsgs()
{
	sfxdelay = 0;
	if (IsMultiGame)
		return;

	if (currLvl._dLevelIdx == DLV_CATHEDRAL1 && !(gbDungMsgs & DMSG_CATHEDRAL)) {
		gbDungMsgs |= DMSG_CATHEDRAL;
		sfxdelay = 40;
		sfxdnum = TEXT_DM_CATHEDRAL;
	} else if (currLvl._dLevelIdx == DLV_CATACOMBS1 && !(gbDungMsgs & DMSG_CATACOMBS)) {
		gbDungMsgs |= DMSG_CATACOMBS;
		sfxdelay = 40;
		sfxdnum = TEXT_DM_CATACOMBS;
	} else if (currLvl._dLevelIdx == DLV_CAVES1 && !(gbDungMsgs & DMSG_CAVES)) {
		gbDungMsgs |= DMSG_CAVES;
		sfxdelay = 40;
		sfxdnum = TEXT_DM_CAVES;
	} else if (currLvl._dLevelIdx == DLV_HELL1 && !(gbDungMsgs & DMSG_HELL)) {
		gbDungMsgs |= DMSG_HELL;
		sfxdelay = 40;
		sfxdnum = TEXT_DM_HELL;
	} else if (currLvl._dLevelIdx == DLV_HELL4 && !(gbDungMsgs & DMSG_DIABLO)) {
		gbDungMsgs |= DMSG_DIABLO;
		sfxdelay = 40;
		sfxdnum = TEXT_DM_DIABLO;
#ifdef HELLFIRE
	} else if (currLvl._dLevelIdx == DLV_NEST1 && !(gbDungMsgs2 & DMSG2_DEFILER)) {
		gbDungMsgs2 |= DMSG2_DEFILER;
		sfxdelay = 10;
		sfxdnum = TEXT_DM_NEST;
	} else if (currLvl._dLevelIdx == DLV_NEST3 && !(gbDungMsgs2 & DMSG2_DEFILER1)) {
		gbDungMsgs2 |= DMSG2_DEFILER1;
		sfxdelay = 10;
		sfxdnum = TEXT_DM_DEFILER;
	} else if (currLvl._dLevelIdx == DLV_CRYPT1 && !(gbDungMsgs2 & DMSG2_DEFILER2)) {
		gbDungMsgs2 |= DMSG2_DEFILER2;
		sfxdelay = 30;
		sfxdnum = TEXT_DM_CRYPT;
#endif
	} else if (currLvl._dLevelIdx == SL_SKELKING && !(gbDungMsgs & DMSG_SKING)) {
		gbDungMsgs |= DMSG_SKING;
		sfxdelay = 30;
		sfxdnum = TEXT_DM_SKING;
	}
}

DEVILUTION_END_NAMESPACE
