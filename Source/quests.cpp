/**
 * @file quests.cpp
 *
 * Implementation of functionality for handling quests.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int qtopline;
bool gbQuestlog;
BYTE *pQLogCel;
/** Contains the quests of the current game. */
QuestStruct quests[NUM_QUESTS];
int qline;
int qlist[NUM_QUESTS];
int numqlines;
int WaterDone;
int ReturnLvlX;
int ReturnLvlY;
int ReturnLvl;

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

void InitQuestGfx()
{
	gbQuestlog = false;
	WaterDone = 0;
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
		qs->_qlog = FALSE;
		qs->_qtx = 0;
		qs->_qty = 0;
		qs->_qidx = i;
		qs->_qvar1 = 0;
		qs->_qvar2 = 0;
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
			LoadPalette("Levels\\L3Data\\L3pwater.pal");
			WaterDone = 32;
		}
		if (WaterDone > 0) {
			palette_update_quest_palette(WaterDone);
			WaterDone--;
		}
	}
}

int ForceQuests()
{
	int i;

	for (i = 0; i < numtrigs; i++) {
		if (trigs[i]._tmsg == WM_DIABSETLVL) {
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

	switch (monster[mnum]._uniqtype - 1) {
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
	// fix transVal at the entrance
	DRLG_CopyTrans(x + 13, y + 7, x + 12, y + 7);
	DRLG_CopyTrans(x + 13, y + 8, x + 12, y + 8);
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
}

static void DrawBlood()
{
	DrawMap("Levels\\L2Data\\Blood2.DUN", 3);
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
		ReturnLvlX = quests[Q_SKELKING]._qtx + 1;
		ReturnLvlY = quests[Q_SKELKING]._qty;
		ReturnLvl = questlist[Q_SKELKING]._qdlvl;
		break;
	case SL_BONECHAMB:
		ReturnLvlX = quests[Q_SCHAMB]._qtx + 1;
		ReturnLvlY = quests[Q_SCHAMB]._qty;
		ReturnLvl = questlist[Q_SCHAMB]._qdlvl;
		break;
	case SL_POISONWATER:
		ReturnLvlX = quests[Q_PWATER]._qtx;
		ReturnLvlY = quests[Q_PWATER]._qty + 1;
		ReturnLvl = questlist[Q_PWATER]._qdlvl;
		break;
	case SL_VILEBETRAYER:
		ReturnLvlX = quests[Q_BETRAYER]._qtx + 1;
		ReturnLvlY = quests[Q_BETRAYER]._qty - 1;
		ReturnLvl = questlist[Q_BETRAYER]._qdlvl;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void GetReturnLvlPos()
{
	ViewX = ReturnLvlX;
	ViewY = ReturnLvlY;
	//EnterLevel(ReturnLvl);
}

void LoadPWaterPalette()
{
	// TODO: this is ugly...
	if (currLvl._dLevelIdx != SL_POISONWATER)
		return;

	if (quests[Q_PWATER]._qactive == QUEST_DONE)
		LoadPalette("Levels\\L3Data\\L3pwater.pal");
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
		//for (i = 0; i < nobjects; i++)
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
			for (i = 0; i < nobjects; i++)
				SyncObjectAnim(objectactive[i]);
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
		}
		if (quests[Q_LTBANNER]._qvar1 == 3) {
			ObjChangeMapResync(setpc_x, setpc_y, setpc_x + setpc_w + 1, setpc_y + setpc_h + 1);
			for (i = 0; i < nobjects; i++)
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
			//for (i = 0; i < nobjects; i++)
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

static void PrintQLString(int y, const char *str)
{
	int width, sx, sy, px;

	sx = /*x*/0 + 12 + SCREEN_X;
	sy = y * 24 + 10 + 12 + SCREEN_Y;
	width = GetStringWidth(str);
	if (width < 270) {
		sx += (270 - width) >> 1;
	}
	px = qline == y ? sx : INT_MAX;
	sx = PrintLimitedString(sx, sy, str, 270, COL_WHITE);
	if (px != INT_MAX) {
		DrawPentSpn2(px - 20, sx + 6, sy + 1);
	}
}

void DrawQuestLog()
{
	int i;

	CelDraw(SCREEN_X, SCREEN_Y + SPANEL_HEIGHT - 1, pQLogCel, 1, SPANEL_WIDTH);
	for (i = 0; i < numqlines; i++) {
		PrintQLString(qtopline + i, questlist[qlist[i]]._qlstr);
	}
	PrintQLString(11, "Close Quest Log");
}

void StartQuestlog()
{
	DWORD i;

	numqlines = 0;
	for (i = 0; i < NUM_QUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && quests[i]._qlog) {
			qlist[numqlines] = i;
			numqlines++;
		}
	}
	if (numqlines != 0) {
		qtopline = 5 - (numqlines >> 1);
		qline = qtopline;
	} else {
		// qtopline = 11;
		qline = 11;
	}
	gbQuestlog = true;
}

void QuestlogUp()
{
	if (numqlines != 0) {
		if (qline == qtopline) {
			qline = 11;
		} else if (qline == 11) {
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
		if (qline == 11) {
			qline = qtopline;
		} else if (qline == qtopline + numqlines - 1) {
			qline = 11;
		} else {
			qline++;
		}
		PlaySFX(IS_TITLEMOV);
	}
}

void QuestlogEnter()
{
	PlaySFX(IS_TITLSLCT);
	if (numqlines != 0 && qline != 11)
		InitQTextMsg(quests[qlist[qline - qtopline]]._qmsg);
	gbQuestlog = false;
}

void CheckQuestlog()
{
	int y;

	y = (MouseY - 10) / 24;
	if (y == 11 || (y >= qtopline && y < qtopline + numqlines)) {
		qline = y;
		QuestlogEnter();
	}
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

DEVILUTION_END_NAMESPACE
