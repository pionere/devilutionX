/**
 * @file quests.cpp
 *
 * Implementation of functionality for handling quests.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

#define L3_WATER_PAL "Levels\\L3Data\\L3pwater.pal"

/** The pseudo random seeds to generate the levels. */
int32_t glSeedTbl[NUM_LEVELS];
/** Contains the informations to recreate the dynamic levels. */
DynLevelStruct gDynLevels[NUM_DYNLVLS];
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
/** the masks of the visited levels */
uint32_t guLvlVisited;
int gnSfxDelay;
int gnSfxNum;

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
		qs->_qmsg = qdata->_qdmsg;
	}

	SetRndSeed(glSeedTbl[DLV_HELL3]);
	quests[Q_DIABLO]._qvar1 = random_(0, 3);
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
			LoadPalette(L3_WATER_PAL);
		}
		if (gbWaterDone > 0) {
			palette_update_quest_palette(gbWaterDone);
			gbWaterDone--;
		}
	}
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

			if (sendmsg) {
				NetSendCmdQuest(Q_DIABLO, false); // recipient should not matter
			}
			ResyncDiablo();
		} else { //"Arch-Bishop Lazarus" - single
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_DEAD;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;
		}
		InitTriggers();

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
	case UMT_DIABLO:
		quests[Q_DIABLO]._qactive = QUEST_DONE;
		qn = Q_DIABLO;
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

// TODO: this is ugly...
void LoadPWaterPalette()
{
	if (currLvl._dLevelIdx == SL_POISONWATER && quests[Q_PWATER]._qvar1 == QV_PWATER_CLEAN)  {
		LoadPalette(L3_WATER_PAL);
	}
}

void ResyncBanner()
{
	// assert(currLvl._dLevelIdx == questlist[Q_BANNER]._qdlvl && quests[Q_BANNER]._qvar1 == QV_BANNER_ATTACK);
	int sx = pSetPieces[0]._spx + 3;
	int sy = pSetPieces[0]._spy + 3;
	DRLG_ChangeMap(sx, sy, sx + 3, sy + 3/*, false*/);
}

void ResyncDiablo()
{
	// assert(currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl && quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_DEAD);
	int sx = (pWarps[DWARP_EXIT]._wx - (1 + 2 + DBORDERX)) >> 1;
	int sy = (pWarps[DWARP_EXIT]._wy - (1 + 2 + DBORDERY)) >> 1;
	DRLG_ChangeMap(sx, sy, sx + 2, sy + 2);
}

void ResyncQuests()
{
	//int i;
	//BYTE lvl = currLvl._dLevelIdx;

	deltaload = true;

	InitTriggers();

	if (QuestStatus(Q_BANNER) && quests[Q_BANNER]._qvar1 == QV_BANNER_ATTACK) {
		ResyncBanner();
		/*if (quests[Q_BANNER]._qvar1 == QV_BANNER_TALK1)
			DRLG_ChangeMap(
			    setpc_w + setpc_x - 2,
			    setpc_h + setpc_y - 2,
			    setpc_w + setpc_x + 1,
			    setpc_h + setpc_y + 1, false);
		if (quests[Q_BANNER]._qvar1 == QV_BANNER_GIVEN) {
			DRLG_ChangeMap(
			    setpc_w + setpc_x - 2,
			    setpc_h + setpc_y - 2,
			    setpc_w + setpc_x + 1,
			    setpc_h + setpc_y + 1, false);
			DRLG_ChangeMap(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 2, (setpc_h >> 1) + setpc_y - 2, false);
			for (i = 0; i < numobjects; i++)
				SyncObjectAnim(objectactive[i]);
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
		}
		if (quests[Q_BANNER]._qvar1 == QV_BANNER_TALK2) {
			DRLG_ChangeMap(setpc_x, setpc_y, setpc_x + setpc_w + 1, setpc_y + setpc_h + 1, false);
			for (i = 0; i < numobjects; i++)
				SyncObjectAnim(objectactive[i]);
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1), 9);
		}*/
	}
	// do not activate the quest, otherwise the healer won't give a quest-log entry if visited before the level is cleared
	//if (lvl == SL_POISONWATER && quests[Q_PWATER]._qactive == QUEST_INIT)
	//	quests[Q_PWATER]._qactive = QUEST_ACTIVE;
	if (currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl && quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_DEAD) {
		ResyncDiablo();
	}

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
		if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
			if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_CENTRALOPEN)
				DRLG_ChangeMap(7, 11, 13, 18/*, true*/);
			if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_TALK1)
				DRLG_ChangeMap(7, 20, 11, 22/*, false*/);
			//for (i = 0; i < numobjects; i++)
			//	SyncObjectAnim(objectactive[i]);
		} else if (currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl) {
			// TODO: merge with InitDunTriggers?
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
	//if (quests[Q_DEFILER]._qactive == QUEST_INIT && lvl == questlist[Q_DEFILER]._qdlvl) {
	//	quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
	//	quests[Q_DEFILER]._qlog = TRUE;
	//	NetSendCmdQuest(Q_DEFILER, false); // recipient should not matter
	//}
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
	deltaload = false;
}

static void PrintQLString(int px, int py, unsigned y, const char* str)
{
	int width, sx, sy, tx;

	sx = px;
	sy = py + y * QPNL_LINE_SPACING;
	width = GetSmallStringWidth(str);
	// if (width < QPNL_LINE_WIDTH) {
		sx += (QPNL_LINE_WIDTH - width) >> 1;
	// }
	tx = sx;
	sx = PrintLimitedString(sx, sy, str, QPNL_LINE_WIDTH, COL_WHITE, FONT_KERN_SMALL);
	if (qline == y) {
		DrawSmallPentSpn(tx - FOCUS_SMALL, sx + 6, sy + 1);
	}
}

void DrawQuestLog()
{
	int px, py;
	unsigned i;

	px = SCREEN_X + gnWndQuestX;
	py = SCREEN_Y + gnWndQuestY;
	CelDraw(px, py + SPANEL_HEIGHT - 1, pQLogCel, 1);

	px += QPNL_BORDER;
	py += QPNL_BORDER + QPNL_TEXT_HEIGHT;
	for (i = 0; i < numqlines; i++) {
		PrintQLString(px, py, qtopline + i, questlist[qlist[i]]._qlstr);
	}
	PrintQLString(px, py, QPNL_MAXENTRIES, "Close Quest Log");
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

static void QuestlogUp()
{
	if (numqlines != 0) {
		if (qline == qtopline) {
			qline = QPNL_MAXENTRIES;
		} else if (qline == QPNL_MAXENTRIES) {
			qline = qtopline + numqlines - 1;
		} else {
			qline--;
		}
		PlaySfx(IS_TITLEMOV);
	}
}

static void QuestlogDown()
{
	if (numqlines != 0) {
		if (qline == QPNL_MAXENTRIES) {
			qline = qtopline;
		} else if (qline == qtopline + numqlines - 1) {
			qline = QPNL_MAXENTRIES;
		} else {
			qline++;
		}
		PlaySfx(IS_TITLEMOV);
	}
}

void QuestlogEnter()
{
	PlaySfx(IS_TITLSLCT);
	if (/*numqlines != 0 &&*/ qline != QPNL_MAXENTRIES)
		StartQTextMsg(quests[qlist[qline - qtopline]]._qmsg);
	else
		ToggleWindow(WND_QUEST);
}

void QuestlogMove(int dir)
{
	switch (dir) {
	case MDIR_UP:    QuestlogUp();    break;
	case MDIR_DOWN:  QuestlogDown();  break;
	case MDIR_LEFT:  ToggleWindow(WND_QUEST);  break;
	case MDIR_RIGHT: QuestlogEnter(); break;
	default: ASSUME_UNREACHABLE;   break;
	}
}

void CheckQuestlogClick(bool altAction)
{
	int y;

	if (altAction) {
		ToggleWindow(WND_QUEST);
		return;
	}
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
