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
 * two of these three quests appear in any single player game.
 */
int QuestGroup1[3] = { Q_BUTCHER, Q_LTBANNER, Q_GARBUD };
/**
 * A quest group containing the three quests Halls of the Blind,
 * the Magic Rock and Valor, which ensures that exactly two of
 * these three quests appear in any single player game.
 */
int QuestGroup2[3] = { Q_BLIND, Q_ROCK, Q_BLOOD };
/**
 * A quest group containing the three quests Black Mushroom,
 * Zhar the Mad and Anvil of Fury, which ensures that exactly
 * two of these three quests appear in any single player game.
 */
int QuestGroup3[3] = { Q_MUSHROOM, Q_ZHAR, Q_ANVIL };
/**
 * A quest group containing the two quests Lachdanan and Warlord
 * of Blood, which ensures that exactly one of these two quests
 * appears in any single player game.
 */
int QuestGroup4[2] = { Q_VEIL, Q_WARLORD };

void InitQuests()
{
	QuestStruct *qs;
	const QuestData *qdata;
	int i;

	if (gbMaxPlayers == 1) {
		qs = quests;
		for (i = NUM_QUESTS; i != 0; i--, qs++) {
			qs->_qactive = QUEST_NOTAVAIL;
		}
	} else {
		qs = quests;
		qdata = questlist;
		for (i = NUM_QUESTS; i != 0; i--, qs++, qdata++) {
			if (!(qdata->_qflags & QUEST_ANY)) {
				qs->_qactive = QUEST_NOTAVAIL;
			}
		}
	}

	Qtalklist[TOWN_HEALER][Q_MUSHROOM] = TEXT_NONE;
	Qtalklist[TOWN_WITCH][Q_MUSHROOM] = TEXT_MUSH9;

	gbQuestlog = false;
	WaterDone = 0;

	qs = quests;
	qdata = questlist;
	for (i = 0; i < NUM_QUESTS; i++, qs++, qdata++) {
		if (gbMaxPlayers != 1) {
			if (!(qdata->_qflags & QUEST_ANY))
				continue;
			qs->_qlevel = qdata->_qdmultlvl;
			if (!delta_quest_inited(i)) {
				qs->_qactive = QUEST_INIT;
				qs->_qvar1 = 0;
				qs->_qlog = FALSE;
			}
		} else {
			qs->_qactive = QUEST_INIT;
			qs->_qlevel = qdata->_qdlvl;
			qs->_qvar1 = 0;
			qs->_qlog = FALSE;
		}

		qs->_qslvl = qdata->_qslvl;
		qs->_qtx = 0;
		qs->_qty = 0;
		qs->_qidx = i;
		qs->_qvar2 = 0;
		qs->_qmsg = qdata->_qdmsg;
	}

#ifdef _DEBUG
	if (gbMaxPlayers == 1 && !allquests) {
#else
	if (gbMaxPlayers == 1) {
#endif
		SetRndSeed(glSeedTbl[DLV_HELL3]);
		if (random_(0, 2) != 0)
			quests[Q_PWATER]._qactive = QUEST_NOTAVAIL;
		else
			quests[Q_SKELKING]._qactive = QUEST_NOTAVAIL;

		quests[QuestGroup1[random_(0, lengthof(QuestGroup1))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup2[random_(0, lengthof(QuestGroup2))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup3[random_(0, lengthof(QuestGroup3))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup4[random_(0, lengthof(QuestGroup4))]]._qactive = QUEST_NOTAVAIL;
	}
#ifdef _DEBUG
	if (questdebug != -1)
		quests[questdebug]._qactive = QUEST_ACTIVE;
#endif

	if (gbMaxPlayers != 1)
		quests[Q_BETRAYER]._qvar1 = 2;
}

void CheckQuests()
{
	if (currLvl._dSetLvl) {
		if (currLvl._dLevelIdx == quests[Q_PWATER]._qslvl
		 && quests[Q_PWATER]._qactive == QUEST_ACTIVE
		 && nummonsters == MAX_MINIONS) {
			quests[Q_PWATER]._qactive = QUEST_DONE;
			PlaySfxLoc(IS_QUESTDN, plr[myplr]._px, plr[myplr]._py);
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
	if (currLvl._dLevelIdx == quests[qn]._qlevel
	 && quests[qn]._qactive != QUEST_NOTAVAIL) {
		assert(gbMaxPlayers == 1 || (questlist[qn]._qflags & QUEST_ANY));
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
		sfxdnum = sgSFXSets[SFXS_PLR_61][plr[myplr]._pClass];
		qn = Q_GARBUD;
		break;
	case UMT_SKELKING:
		quests[Q_SKELKING]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_82][plr[myplr]._pClass];
		qn = Q_SKELKING;
		break;
	case UMT_ZHAR: //"Zhar the Mad"
		quests[Q_ZHAR]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_62][plr[myplr]._pClass];
		qn = Q_ZHAR;
		break;
	case UMT_LAZURUS: //"Arch-Bishop Lazarus" - multi
		if (gbMaxPlayers != 1) {
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = 7;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;

			InitTriggers();
			if (sendmsg) {
				NetSendCmdQuest(true, Q_DIABLO, false); // recipient should not matter
			}
		} else { //"Arch-Bishop Lazarus" - single
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = 7;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;
			InitVPReturnTrigger();
		}
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_83][plr[myplr]._pClass];
		qn = Q_BETRAYER;
		break;
	case UMT_WARLORD: //"Warlord of Blood"
		quests[Q_WARLORD]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_94][plr[myplr]._pClass];
		qn = Q_WARLORD;
		break;
	case UMT_BUTCHER:
		quests[Q_BUTCHER]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_80][plr[myplr]._pClass];
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
		NetSendCmdQuest(true, qn, false); // recipient should not matter
}

static void DrawButcher()
{
	int x, y;

	x = 2 * setpc_x + DBORDERX;
	y = 2 * setpc_y + DBORDERY;
	DRLG_RectTrans(x + 3, y + 3, x + 10, y + 10);
}

static void DrawSkelKing(int x, int y)
{
	quests[Q_SKELKING]._qtx = 2 * x + DBORDERX + 12;
	quests[Q_SKELKING]._qty = 2 * y + DBORDERY + 7;
}

static void DrawMap(const char* name, int x, int y, int bv)
{
	int rw, rh;
	int i, j;
	BYTE *sp, *setp;
	int v;

	setp = LoadFileInMem(name, NULL);
	rw = *setp;
	sp = setp + 2;
	rh = *sp;
	sp += 2;
	setpc_w = rw;
	setpc_h = rh;
	setpc_x = x;
	setpc_y = y;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			if (*sp != 0) {
				v = *sp;
			} else {
				v = bv;
			}
			dungeon[i][j] = v;
			sp += 2;
		}
	}
	mem_free_dbg(setp);
}

static void DrawWarLord(int x, int y)
{
	DrawMap("Levels\\L4Data\\Warlord2.DUN", x, y, 6);
}

static void DrawSChamber(int x, int y)
{
	quests[Q_SCHAMB]._qtx = 2 * x + DBORDERX + 6;
	quests[Q_SCHAMB]._qty = 2 * y + DBORDERY + 7;

	DrawMap("Levels\\L2Data\\Bonestr1.DUN", x, y, 3);
}

static void DrawPreMap(const char* name, int x, int y)
{
	int rw, rh;
	int i, j;
	BYTE *sp, *setp;

	setp = LoadFileInMem(name, NULL);
	rw = *setp;
	sp = setp + 2;
	rh = *sp;
	sp += 2;
	setpc_w = rw;
	setpc_h = rh;
	setpc_x = x;
	setpc_y = y;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			if (*sp != 0) {
				pdungeon[i][j] = *sp;
			}
			sp += 2;
		}
	}
	mem_free_dbg(setp);
}

static void DrawLTBanner(int x, int y)
{
	DrawPreMap("Levels\\L1Data\\Banner1.DUN", x, y);
}

static void DrawBlind(int x, int y)
{
	DrawPreMap("Levels\\L2Data\\Blind1.DUN", x, y);
}

static void DrawBlood(int x, int y)
{
	DrawMap("Levels\\L2Data\\Blood2.DUN", x, y, 3);
}

#ifdef HELLFIRE
static void DrawNakrul(int x, int y)
{
	DrawPreMap("NLevels\\L5Data\\Nakrul2.DUN", x, y);
}
#endif

void DRLG_CheckQuests(int x, int y)
{
	int i;

	for (i = 0; i < NUM_QUESTS; i++) {
		if (QuestStatus(i)) {
			switch (i) {
			case Q_BUTCHER:
				DrawButcher();
				break;
			case Q_LTBANNER:
				DrawLTBanner(x, y);
				break;
			case Q_BLIND:
				DrawBlind(x, y);
				break;
			case Q_BLOOD:
				DrawBlood(x, y);
				break;
			case Q_WARLORD:
				DrawWarLord(x, y);
				break;
			case Q_SKELKING:
				DrawSkelKing(x, y);
				break;
			case Q_SCHAMB:
				DrawSChamber(x, y);
				break;
#ifdef HELLFIRE
			case Q_NAKRUL:
				DrawNakrul(x, y);
				break;
#endif
			}
		}
	}
}

void SetReturnLvlPos()
{
	switch (plr[myplr].plrlevel) {
	case SL_SKELKING:
		ReturnLvlX = quests[Q_SKELKING]._qtx + 1;
		ReturnLvlY = quests[Q_SKELKING]._qty;
		ReturnLvl = quests[Q_SKELKING]._qlevel;
		break;
	case SL_BONECHAMB:
		ReturnLvlX = quests[Q_SCHAMB]._qtx + 1;
		ReturnLvlY = quests[Q_SCHAMB]._qty;
		ReturnLvl = quests[Q_SCHAMB]._qlevel;
		break;
	case SL_POISONWATER:
		ReturnLvlX = quests[Q_PWATER]._qtx;
		ReturnLvlY = quests[Q_PWATER]._qty + 1;
		ReturnLvl = quests[Q_PWATER]._qlevel;
		break;
	case SL_VILEBETRAYER:
		ReturnLvlX = quests[Q_BETRAYER]._qtx + 1;
		ReturnLvlY = quests[Q_BETRAYER]._qty - 1;
		ReturnLvl = quests[Q_BETRAYER]._qlevel;
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
	EnterLevel(ReturnLvl);
}

void LoadPWaterPalette()
{
	// TODO: this is ugly...
	if (currLvl._dLevelIdx != quests[Q_PWATER]._qslvl) // || quests[Q_PWATER]._qslvl == 0 || quests[Q_PWATER]._qactive == QUEST_INIT) // || currLvl._dType != quests[Q_PWATER]._qlvltype)
		return;

	if (quests[Q_PWATER]._qactive == QUEST_DONE)
		LoadPalette("Levels\\L3Data\\L3pwater.pal");
	//else
	//	LoadPalette("Levels\\L3Data\\L3pfoul.pal");
}

static void ResyncBanner()
{
	int i;

	if (quests[Q_LTBANNER]._qvar1 != 4) {
		// open the entrance of the setmap -> TODO: add these to Banner2.DUN ?
		ObjChangeMapResync(
		    setpc_w + setpc_x - 2,
		    setpc_h + setpc_y - 2,
		    setpc_w + setpc_x + 1,
		    setpc_h + setpc_y + 1);
		// TODO: add the opening of the entrance to Banner2.DUN?
		//   in vanilla code the monsters room was also opened after the quest was activated
		//   which caused that the player could teleport to the room and kill them.
		//   if this was intended, they could have done it by adding them to Banner2.DUN too
		//   maybe it was just a bug...
	} else {
		ObjChangeMapResync(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
		for (i = 0; i < nobjects; i++)
			SyncObjectAnim(objectactive[i]);
		//DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
	}
}

void ResyncMPQuests()
{
	if (QuestStatus(Q_LTBANNER)) {
		ResyncBanner();
	}
	// TODO: eliminate relative level-indices?
	//if (quests[Q_SKELKING]._qactive == QUEST_INIT
	//    && currLvl._dLevelIdx >= quests[Q_SKELKING]._qlevel - 1
	//    && currLvl._dLevelIdx <= quests[Q_SKELKING]._qlevel + 1) {
	//	quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(true, Q_SKELKING, false); // recipient should not matter
	//}
	if (quests[Q_BUTCHER]._qactive == QUEST_INIT
	    && currLvl._dLevelIdx >= quests[Q_BUTCHER]._qlevel - 1
	    && currLvl._dLevelIdx <= quests[Q_BUTCHER]._qlevel + 1) {
		quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(true, Q_BUTCHER, false); // recipient should not matter
	}
	if (quests[Q_BETRAYER]._qactive == QUEST_INIT && currLvl._dLevelIdx == quests[Q_BETRAYER]._qlevel - 1) {
		quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(true, Q_BETRAYER, false); // recipient should not matter
	}
#ifdef HELLFIRE
	//if (quests[Q_GRAVE]._qactive == QUEST_INIT && currLvl._dLevelIdx == quests[Q_GRAVE]._qlevel - 1) {
	//	quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(true, Q_GRAVE, false); // recipient should not matter
	//}
	if (quests[Q_DEFILER]._qactive == QUEST_INIT && currLvl._dLevelIdx == quests[Q_DEFILER]._qlevel) {
		quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
		quests[Q_DEFILER]._qlog = TRUE;
		quests[Q_DEFILER]._qmsg = TEXT_DEFILER1;
		NetSendCmdQuest(true, Q_DEFILER, false); // recipient should not matter
	}
	//if (quests[Q_NAKRUL]._qactive == QUEST_INIT && currLvl._dLevelIdx == quests[Q_NAKRUL]._qlevel - 1) {
	//	quests[Q_NAKRUL]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(true, Q_NAKRUL, false); // recipient should not matter
	//}
	//if (quests[Q_JERSEY]._qactive == QUEST_INIT && currLvl._dLevelIdx == quests[Q_JERSEY]._qlevel - 1) {
	//	quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
	//	NetSendCmdQuest(true, Q_JERSEY, false); // recipient should not matter
	//}
	if (quests[Q_GIRL]._qactive == QUEST_INIT && currLvl._dLevelIdx == quests[Q_GIRL]._qlevel) {
		quests[Q_GIRL]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(true, Q_GIRL, false); // recipient should not matter
		// TODO: send message to reinit the towners?
	}
#endif
}

void ResyncQuests()
{
	int i;

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
	if (currLvl._dLevelIdx == quests[Q_MUSHROOM]._qlevel) {
		if (quests[Q_MUSHROOM]._qactive == QUEST_INIT && quests[Q_MUSHROOM]._qvar1 == QS_INIT) {
			SpawnQuestItemInArea(IDI_FUNGALTM, 5);
			quests[Q_MUSHROOM]._qvar1 = QS_TOMESPAWNED;
		} else {
			// TODO: why is this not done on currLvl._dLevelIdx == DLV_TOWN?
			if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE) {
				if (quests[Q_MUSHROOM]._qvar1 >= QS_MUSHGIVEN) {
					Qtalklist[TOWN_WITCH][Q_MUSHROOM] = TEXT_NONE;
					Qtalklist[TOWN_HEALER][Q_MUSHROOM] = TEXT_MUSH3;
				} else if (quests[Q_MUSHROOM]._qvar1 >= QS_BRAINGIVEN) {
					Qtalklist[TOWN_HEALER][Q_MUSHROOM] = TEXT_NONE;
				}
			}
		}
	}
	if (currLvl._dLevelIdx == quests[Q_VEIL]._qlevel + 1 && quests[Q_VEIL]._qactive == QUEST_ACTIVE && quests[Q_VEIL]._qvar1 == 0) {
		quests[Q_VEIL]._qvar1 = 1;
		SpawnQuestItemInArea(IDI_GLDNELIX, 5);
	}
	if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		if (quests[Q_BETRAYER]._qvar1 >= 4)
			ObjChangeMapResync(1, 11, 20, 18);
		if (quests[Q_BETRAYER]._qvar1 >= 6)
			ObjChangeMapResync(1, 18, 20, 24);
		if (quests[Q_BETRAYER]._qvar1 >= 7)
			InitVPReturnTrigger();
		for (i = 0; i < nobjects; i++)
			SyncObjectAnim(objectactive[i]);
	}
	if (currLvl._dLevelIdx == quests[Q_BETRAYER]._qlevel) {
		if (quests[Q_BETRAYER]._qvar1 >= 2) {
			InitVPEntryTrigger();
		}
	}
}

static void PrintQLString(int x, int y, bool cjustflag, const char *str, int col)
{
	int len, width, i, k, sx, sy;
	BYTE c;

	sx = x + 12 + SCREEN_X;
	sy = y * 24 + 10 + 12 + SCREEN_Y;
	len = strlen(str);
	k = 0;
	if (cjustflag) {
		width = 0;
		for (i = 0; i < len; i++)
			width += fontkern[fontframe[gbFontTransTbl[(BYTE)str[i]]]] + 1;
		if (width < 270)
			k = (270 - width) >> 1;
		sx += k;
	}
	if (qline == y) {
		CelDraw(sx - 20, sy + 1, pSPentSpn2Cels, PentSpn2Spin(), 12);
	}
	for (i = 0; i < len; i++) {
		c = fontframe[gbFontTransTbl[(BYTE)str[i]]];
		k += fontkern[c] + 1;
		if (c != '\0' && k <= 270) {
			PrintChar(sx, sy, c, col);
		}
		sx += fontkern[c] + 1;
	}
	if (qline == y) {
		CelDraw(cjustflag ? x + k + 36 + SCREEN_X : 294 - 12 + SCREEN_X - x, sy + 1, pSPentSpn2Cels, PentSpn2Spin(), 12);
	}
}

void DrawQuestLog()
{
	int i;

	CelDraw(SCREEN_X, SCREEN_Y + SPANEL_HEIGHT - 1, pQLogCel, 1, SPANEL_WIDTH);
	for (i = 0; i < numqlines; i++) {
		PrintQLString(0, qtopline + i, true, questlist[qlist[i]]._qlstr, COL_WHITE);
	}
	PrintQLString(0, 11, true, "Close Quest Log", COL_WHITE);
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
