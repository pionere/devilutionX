/**
 * @file quests.cpp
 *
 * Implementation of functionality for handling quests.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

/** Contains the quests of the current game. */
QuestStruct quests[NUM_QUESTS];
/** Quest-log panel CEL */
CelImageBuf* pQLogCel;
/** the entries of the quest-log panel (quest_id) */
BYTE qlist[QPNL_MAXENTRIES];
/** the index of the first valid line on the quest-log panel */
unsigned qtopline;
/** the number of valid lines on the quest-log panel */
unsigned numqlines;
/** the index of the selected line on the quest-log panel */
unsigned qline;
BYTE gbTownWarps;
BYTE gbWaterDone;
static_assert(NUM_LEVELS <= 32, "guLvlVisited can not maintain too many levels.");
uint32_t guLvlVisited;
int gnSfxDelay;
int gnSfxNum;
int gnReturnLvlX;
int gnReturnLvlY;
int gnReturnLvl;

/**
 * A quest group containing the three quests the Butcher,
 * Ogden's Sign and Gharbad the Weak, which ensures that exactly
 * two of these three quests appear in any game.
 */
static const int QuestGroup1[3] = { Q_BUTCHER, Q_BANNER, Q_GARBUD };
/**
 * A quest group containing the three quests Halls of the Blind,
 * the Magic Rock and Valor, which ensures that exactly two of
 * these three quests appear in any game.
 */
static const int QuestGroup2[3] = { Q_BLIND, Q_ROCK, Q_BLOOD };
/**
 * A quest group containing the three quests Black Mushroom,
 * Zhar the Mad and Anvil of Fury, which ensures that exactly
 * two of these three quests appear in any game.
 */
static const int QuestGroup3[3] = { Q_MUSHROOM, Q_ZHAR, Q_ANVIL };
/*
 * Other quest groups:
 * { Q_SKELKING, Q_PWATER }
 * { Q_VEIL, Q_WARLORD }
 * { Q_JERSEY, Q_FARMER }
 */

void InitQuestGFX()
{
	gbTownWarps = 0;
	gbWaterDone = 0;
	guLvlVisited = 0;

	assert(pQLogCel == NULL);
	pQLogCel = CelLoadImage("Data\\Quest.CEL", SPANEL_WIDTH);
}

void FreeQuestGFX()
{
	MemFreeDbg(pQLogCel);
}

void InitQuests()
{
	QuestStruct* qs;
	const QuestData* qdata;
	int i;

	qs = quests;
	qdata = questlist;
	for (i = 0; i < NUM_QUESTS; i++, qs++, qdata++) {
		qs->_qactive = QUEST_INIT;
		qs->_qvar1 = QV_INIT;
		qs->_qvar2 = 0;
		qs->_qlog = FALSE;
		qs->_qtx = 0;
		qs->_qty = 0;
		qs->_qmsg = qdata->_qdmsg;
	}

	SetRndSeed(glSeedTbl[DLV_HELL3]);
	quests[Q_DIABLO]._qvar1 = random_(0, 3);
#if DEBUG_MODE
	if (!allquests) {
#endif
		quests[random_(0, 2) != 0 ? Q_SKELKING : Q_PWATER]._qactive = QUEST_NOTAVAIL;
#ifdef HELLFIRE
		if (random_(0, 2) != 0)
			quests[Q_GIRL]._qactive = QUEST_NOTAVAIL;
#endif

		quests[QuestGroup1[random_(0, lengthof(QuestGroup1))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup2[random_(0, lengthof(QuestGroup2))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup3[random_(0, lengthof(QuestGroup3))]]._qactive = QUEST_NOTAVAIL;
		quests[random_(0, 2) != 0 ? Q_VEIL : Q_WARLORD]._qactive = QUEST_NOTAVAIL;
#ifdef HELLFIRE
		quests[random_(0, 2) != 0 ? Q_FARMER : Q_JERSEY]._qactive = QUEST_NOTAVAIL;
#endif
#if DEBUG_MODE
	}
	if (questdebug != -1)
		quests[questdebug]._qactive = QUEST_ACTIVE;
#endif

	if (quests[Q_PWATER]._qactive == QUEST_NOTAVAIL)
		quests[Q_PWATER]._qvar1 = QV_PWATER_CLEAN;
}

void CheckQuests()
{
	if (currLvl._dLevelIdx == SL_POISONWATER) {
		if (quests[Q_PWATER]._qvar1 != QV_PWATER_CLEAN
		 && nummonsters == MAX_MINIONS) {
			quests[Q_PWATER]._qvar1 = QV_PWATER_CLEAN;
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
			int dx = pcurspos.x - (trigs[i]._tx - 1);
			int dy = pcurspos.y - (trigs[i]._ty - 1);
			if (abs(dx) <= 1 && abs(dy) <= 1 // select the 3x3 square around (-1;-1)
			 && abs(dx - dy) < 2) {          // exclude the top left and bottom right positions
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

	switch (monsters[mnum]._muniqtype - 1) {
	case UMT_GARBUD: //"Gharbad the Weak"
		quests[Q_GARBUD]._qactive = QUEST_DONE;
		gnSfxDelay = 30;
		gnSfxNum = TEXT_QM_GARBUD;
		qn = Q_GARBUD;
		break;
	case UMT_SKELKING:
		quests[Q_SKELKING]._qactive = QUEST_DONE;
		gnSfxDelay = 30;
		gnSfxNum = TEXT_QM_SKING;
		qn = Q_SKELKING;
		break;
	case UMT_ZHAR: //"Zhar the Mad"
		quests[Q_ZHAR]._qactive = QUEST_DONE;
		gnSfxDelay = 30;
		gnSfxNum = TEXT_QM_ZHAR;
		qn = Q_ZHAR;
		break;
	case UMT_LAZARUS: //"Arch-Bishop Lazarus" - multi
		if (IsMultiGame) {
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_DEAD;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;

			InitTriggers();
			if (sendmsg) {
				NetSendCmdQuest(Q_DIABLO, false); // recipient should not matter
			}
		} else { //"Arch-Bishop Lazarus" - single
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_DEAD;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;
			InitVPReturnTrigger(false);
		}
		gnSfxDelay = 30;
		gnSfxNum = TEXT_QM_LAZARUS;
		qn = Q_BETRAYER;
		break;
	case UMT_WARLORD: //"Warlord of Blood"
		quests[Q_WARLORD]._qactive = QUEST_DONE;
		gnSfxDelay = 30;
		gnSfxNum = TEXT_QM_WARLORD;
		qn = Q_WARLORD;
		break;
	case UMT_BUTCHER:
		quests[Q_BUTCHER]._qactive = QUEST_DONE;
		gnSfxDelay = 30;
		gnSfxNum = TEXT_QM_BUTCHER;
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
	// patch dSolidTable - L1.SOL - commented out because 299 is used elsewhere
	//nSolidTable[299] = true;

	quests[Q_SKELKING]._qtx = x + 12;
	quests[Q_SKELKING]._qty = y + 7;
}

static void DrawMap(const char* name, int bv)
{
	int x, y, rw, rh, i, j;
	BYTE* pMap;
	BYTE* sp;

	pMap = LoadFileInMem(name);
	rw = pMap[0];
	rh = pMap[2];

	sp = &pMap[4];
	assert(setpc_w == rw);
	assert(setpc_h == rh);
	x = setpc_x;
	y = setpc_y;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			dungeon[i][j] = *sp != 0 ? *sp : bv;
			sp += 2;
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
	quests[Q_BCHAMB]._qtx = 2 * setpc_x + DBORDERX + 6;
	quests[Q_BCHAMB]._qty = 2 * setpc_y + DBORDERY + 7;

	DrawMap("Levels\\L2Data\\Bonestr1.DUN", 3);
	// patch the map - Bonestr1.DUN
	// shadow of the external-left column
	dungeon[setpc_x][setpc_y + 4] = 48;
	dungeon[setpc_x][setpc_y + 5] = 50;
}

static void DrawLTBanner()
{
	DrawMap("Levels\\L1Data\\Banner2.DUN", 13);
	// patch the map - Banner2.DUN
	// replace the wall with door
	dungeon[setpc_x + 7][setpc_y + 6] = 193;
}

static void DrawBlind()
{
	DrawMap("Levels\\L2Data\\Blind2.DUN", 3);
	// patch the map - Blind2.DUN
	// replace the door with wall
	dungeon[setpc_x + 4][setpc_y + 3] = 25;
}

static void DrawBlood()
{
	DrawMap("Levels\\L2Data\\Blood2.DUN", 3);
	// patch the map - Blood2.DUN
	// place pieces with closed doors
	dungeon[setpc_x + 4][setpc_y + 10] = 151;
	dungeon[setpc_x + 4][setpc_y + 15] = 151;
	dungeon[setpc_x + 5][setpc_y + 15] = 151;
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
	DrawMap("NLevels\\L5Data\\Nakrul1.DUN", 13);
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
			case Q_BANNER:
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
			case Q_BCHAMB:
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
		gnReturnLvlX = quests[Q_BCHAMB]._qtx + 1;
		gnReturnLvlY = quests[Q_BCHAMB]._qty;
		gnReturnLvl = questlist[Q_BCHAMB]._qdlvl;
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

	if (quests[Q_PWATER]._qvar1 == QV_PWATER_CLEAN) {
		//if (gbWaterDone == 0)
			LoadPalette("Levels\\L3Data\\L3pwater.pal");
		//else
		//	quests[Q_PWATER]._qvar2 = 1; // LOADWATERPAL
	}
	//else
	//	LoadPalette("Levels\\L3Data\\L3pfoul.pal");
}

void ResyncBanner()
{
	if (quests[Q_BANNER]._qvar1 == QV_BANNER_ATTACK) {
		ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h/*, false*/);
		//for (i = 0; i < numobjects; i++)
		//	SyncObjectAnim(objectactive[i]);
		BYTE tv = dTransVal[2 * setpc_x + 1 + DBORDERX][2 * (setpc_y + 6) + 1 + DBORDERY];
		DRLG_MRectTrans(setpc_x, setpc_y + 3, setpc_x + setpc_w - 1, setpc_y + setpc_h - 1, tv);
	}
}

void ResyncQuests()
{
	//int i;
	BYTE lvl = currLvl._dLevelIdx;

	if (QuestStatus(Q_BANNER)) {
		ResyncBanner();
		/*if (quests[Q_BANNER]._qvar1 == QV_BANNER_TALK1)
			ObjChangeMap(
			    setpc_w + setpc_x - 2,
			    setpc_h + setpc_y - 2,
			    setpc_w + setpc_x + 1,
			    setpc_h + setpc_y + 1, false);
		if (quests[Q_BANNER]._qvar1 == QV_BANNER_GIVEN) {
			ObjChangeMap(
			    setpc_w + setpc_x - 2,
			    setpc_h + setpc_y - 2,
			    setpc_w + setpc_x + 1,
			    setpc_h + setpc_y + 1, false);
			ObjChangeMap(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 2, (setpc_h >> 1) + setpc_y - 2, false);
			for (i = 0; i < numobjects; i++)
				SyncObjectAnim(objectactive[i]);
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
		}
		if (quests[Q_BANNER]._qvar1 == QV_BANNER_TALK2) {
			ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w + 1, setpc_y + setpc_h + 1, false);
			for (i = 0; i < numobjects; i++)
				SyncObjectAnim(objectactive[i]);
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
		}*/
	}
	// do not activate the quest, otherwise the healer won't give a quest-log entry if visited before the level is cleared
	//if (lvl == SL_POISONWATER && quests[Q_PWATER]._qactive == QUEST_INIT)
	//	quests[Q_PWATER]._qactive = QUEST_ACTIVE;
	if (IsMultiGame) {
		// TODO: eliminate relative level-indices?
		//if (quests[Q_SKELKING]._qactive == QUEST_INIT
		//    && lvl >= questlist[Q_SKELKING]._qdlvl - 1
		//    && lvl <= questlist[Q_SKELKING]._qdlvl + 1) {
		//	quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
		//	NetSendCmdQuest(Q_SKELKING, false); // recipient should not matter
		//}
		//if (quests[Q_BUTCHER]._qactive == QUEST_INIT
		//	&& lvl >= questlist[Q_BUTCHER]._qdlvl - 1
		//	&& lvl <= questlist[Q_BUTCHER]._qdlvl + 1) {
		//	quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
		//	NetSendCmdQuest(Q_BUTCHER, false); // recipient should not matter
		//}
		//if (lvl == questlist[Q_BETRAYER]._qdlvl - 1 && quests[Q_BETRAYER]._qactive == QUEST_INIT) {
		//	quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
		//	NetSendCmdQuest(Q_BETRAYER, false); // recipient should not matter
		//}
	} else {
		if (lvl == SL_VILEBETRAYER) {
			if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_CENTRALOPEN)
				ObjChangeMap(7, 11, 13, 18/*, true*/);
			if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_TALK1)
				ObjChangeMap(7, 20, 11, 22/*, false*/);
			if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_DEAD)
				InitVPReturnTrigger(true);
			//for (i = 0; i < numobjects; i++)
			//	SyncObjectAnim(objectactive[i]);
		}
		if (lvl == questlist[Q_BETRAYER]._qdlvl) {
			if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_STAFFGIVEN) {
				if (quests[Q_BETRAYER]._qvar1 == QV_BETRAYER_STAFFGIVEN) {
					quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_PORTALOPEN;
					InitVPEntryTrigger(false);
				} else {
					InitVPEntryTrigger(true);
				}
			}
		}
	}
#ifdef HELLFIRE
	if (quests[Q_DEFILER]._qactive == QUEST_INIT && lvl == questlist[Q_DEFILER]._qdlvl) {
		quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
		quests[Q_DEFILER]._qlog = TRUE;
		NetSendCmdQuest(Q_DEFILER, false); // recipient should not matter
	}
	//if (quests[Q_NAKRUL]._qactive == QUEST_INIT && lvl == questlist[Q_NAKRUL]._qdlvl - 1) {
	//	quests[Q_NAKRUL]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(Q_NAKRUL, false); // recipient should not matter
	//}
	//if (quests[Q_JERSEY]._qactive == QUEST_INIT && lvl == questlist[Q_JERSEY]._qdlvl - 1) {
	//	quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(Q_JERSEY, false); // recipient should not matter
	//}
	//if (quests[Q_GIRL]._qactive == QUEST_INIT && lvl == questlist[Q_GIRL]._qdlvl) {
	//	quests[Q_GIRL]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(Q_GIRL, false); // recipient should not matter
	//	// TODO: send message to reinit the towners?
	//}
#endif
}

static void PrintQLString(unsigned y, const char* str)
{
	int width, sx, sy, px;

	sx = /*x*/0 + QPNL_BORDER + SCREEN_X + gnWndQuestX;
	sy = y * QPNL_LINE_SPACING + QPNL_BORDER + QPNL_TEXT_HEIGHT + SCREEN_Y + gnWndQuestY;
	width = GetSmallStringWidth(str);
	if (width < QPNL_LINE_WIDTH) {
		sx += (QPNL_LINE_WIDTH - width) >> 1;
	}
	px = qline == y ? sx : INT_MAX;
	sx = PrintLimitedString(sx, sy, str, QPNL_LINE_WIDTH, COL_WHITE);
	if (px != INT_MAX) {
		DrawSmallPentSpn(px - FOCUS_SMALL, sx + 6, sy + 1);
	}
}

void DrawQuestLog()
{
	unsigned i;

	CelDraw(SCREEN_X + gnWndQuestX, SCREEN_Y + gnWndQuestY + SPANEL_HEIGHT - 1, pQLogCel, 1);
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
		StartQTextMsg(quests[qlist[qline - qtopline]]._qmsg);
	else
		ToggleWindow(WND_QUEST);
}

void CheckQuestlogClick()
{
	int y;

	y = (MousePos.y - (gnWndQuestY + QPNL_BORDER + QPNL_TEXT_HEIGHT / 2) + QPNL_LINE_SPACING / 2 + QPNL_LINE_SPACING) / QPNL_LINE_SPACING - 1;
	if (y != QPNL_MAXENTRIES) {
		if ((unsigned)(y - qtopline) >= numqlines) {
			StartWndDrag(WND_QUEST);
			return;
		}
	}
	qline = y;
	QuestlogEnter();
}

void SetMultiQuest(int qn, int qa, int qlog, int qvar)
{
	QuestStruct* qs;

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
	gnSfxDelay = 0;
	if (IsMultiGame || IsLvlVisited(currLvl._dLevelIdx))
		return;

	if (currLvl._dLevelIdx == DLV_CATHEDRAL1) {
		gnSfxDelay = 40;
		gnSfxNum = TEXT_DM_CATHEDRAL;
	} else if (currLvl._dLevelIdx == DLV_CATACOMBS1) {
		gnSfxDelay = 40;
		gnSfxNum = TEXT_DM_CATACOMBS;
	} else if (currLvl._dLevelIdx == DLV_CAVES1) {
		gnSfxDelay = 40;
		gnSfxNum = TEXT_DM_CAVES;
	} else if (currLvl._dLevelIdx == DLV_HELL1) {
		gnSfxDelay = 40;
		gnSfxNum = TEXT_DM_HELL;
	} else if (currLvl._dLevelIdx == DLV_HELL4) {
		gnSfxDelay = 40;
		gnSfxNum = TEXT_DM_DIABLO;
#ifdef HELLFIRE
	} else if (currLvl._dLevelIdx == DLV_NEST1) {
		gnSfxDelay = 10;
		gnSfxNum = TEXT_DM_NEST;
	} else if (currLvl._dLevelIdx == DLV_NEST3) {
		gnSfxDelay = 10;
		gnSfxNum = TEXT_DM_DEFILER;
	} else if (currLvl._dLevelIdx == DLV_CRYPT1) {
		gnSfxDelay = 30;
		gnSfxNum = TEXT_DM_CRYPT;
#endif
	} else if (currLvl._dLevelIdx == SL_SKELKING) {
		gnSfxDelay = 30;
		gnSfxNum = TEXT_DM_SKING;
	}
}

DEVILUTION_END_NAMESPACE
