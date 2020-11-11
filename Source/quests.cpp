/**
 * @file quests.cpp
 *
 * Implementation of functionality for handling quests.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int qtopline;
BOOL questlog;
BYTE *pQLogCel;
/** Contains the quests of the current game. */
QuestStruct quests[MAXQUESTS];
int qline;
int qlist[MAXQUESTS];
int numqlines;
int WaterDone;
int ReturnLvlX;
int ReturnLvlY;
int ReturnLvlT;
int ReturnLvl;

/** Contains the data related to each quest_id. */
QuestData questlist[MAXQUESTS] = {
	// clang-format off
	// _qdlvl, _qdmultlvl, _qlvlt,          _qdtype,     _qdrnd,   _qslvl,  _qflags,       _qdmsg,        _qlstr
	{       5,         -1, DTYPE_NONE,      Q_ROCK,      100,      0,       QUEST_SINGLE,  TEXT_INFRA5,   "The Magic Rock"           },
	{       9,         -1, DTYPE_NONE,      Q_MUSHROOM,  100,      0,       QUEST_SINGLE,  TEXT_MUSH8,    "Black Mushroom"           },
	{       4,         -1, DTYPE_NONE,      Q_GARBUD,    100,      0,       QUEST_SINGLE,  TEXT_GARBUD1,  "Gharbad The Weak"         },
	{       8,         -1, DTYPE_NONE,      Q_ZHAR,      100,      0,       QUEST_SINGLE,  TEXT_ZHAR1,    "Zhar the Mad"             },
	{      14,         -1, DTYPE_NONE,      Q_VEIL,      100,      0,       QUEST_SINGLE,  TEXT_VEIL9,    "Lachdanan"                },
	{      15,         -1, DTYPE_NONE,      Q_DIABLO,    100,      0,       QUEST_ANY,     TEXT_VILE3,    "Diablo"                   },
	{       2,          2, DTYPE_NONE,      Q_BUTCHER,   100,      0,       QUEST_ANY,     TEXT_BUTCH9,   "The Butcher"              },
	{       4,         -1, DTYPE_NONE,      Q_LTBANNER,  100,      0,       QUEST_SINGLE,  TEXT_BANNER2,  "Ogden's Sign"             },
	{       7,         -1, DTYPE_NONE,      Q_BLIND,     100,      0,       QUEST_SINGLE,  TEXT_BLINDING, "Halls of the Blind"       },
	{       5,         -1, DTYPE_NONE,      Q_BLOOD,     100,      0,       QUEST_SINGLE,  TEXT_BLOODY,   "Valor"                    },
	{      10,         -1, DTYPE_NONE,      Q_ANVIL,     100,      0,       QUEST_SINGLE,  TEXT_ANVIL5,   "Anvil of Fury"            },
	{      13,         -1, DTYPE_NONE,      Q_WARLORD,   100,      0,       QUEST_SINGLE,  TEXT_BLOODWAR, "Warlord of Blood"         },
	{       3,          3, DTYPE_CATHEDRAL, Q_SKELKING,  100,      1,       QUEST_ANY,     TEXT_KING2,    "The Curse of King Leoric" },
	{       2,         -1, DTYPE_CAVES,     Q_PWATER,    100,      4,       QUEST_SINGLE,  TEXT_POISON3,  "Poisoned Water Supply"    },
	{       6,         -1, DTYPE_CATACOMBS, Q_SCHAMB,    100,      2,       QUEST_SINGLE,  TEXT_BONER,    "The Chamber of Bone"      },
	{      15,         15, DTYPE_CATHEDRAL, Q_BETRAYER,  100,      5,       QUEST_ANY,     TEXT_VILE1,    "Archbishop Lazarus"       },
#ifdef HELLFIRE
	{      17,         17, DTYPE_NONE,      Q_GRAVE,     100,      0,       QUEST_ANY,     TEXT_GRAVE7,   "Grave Matters"            },
	{       9,          9, DTYPE_NONE,      Q_FARMER,    100,      0,       QUEST_ANY,     TEXT_FARMER1,  "Farmer's Orchard"         },
	{      17,         -1, DTYPE_NONE,      Q_GIRL,      100,      0,       QUEST_SINGLE,  TEXT_GIRL2,    "Little Girl"              },
	{      19,         -1, DTYPE_NONE,      Q_TRADER,    100,      0,       QUEST_SINGLE,  TEXT_TRADER,   "Wandering Trader"         },
	{      17,         17, DTYPE_NONE,      Q_DEFILER,   100,      0,       QUEST_ANY,     TEXT_DEFILER1, "The Defiler"              },
	{      21,         21, DTYPE_NONE,      Q_NAKRUL,    100,      0,       QUEST_ANY,     TEXT_NAKRUL1,  "Na-Krul"                  },
	{      21,         -1, DTYPE_NONE,      Q_CORNSTN,   100,      0,       QUEST_SINGLE,  TEXT_CORNSTN,  "Cornerstone of the World" },
	{       9,          9, DTYPE_NONE,      Q_JERSEY,    100,      0,       QUEST_ANY,     TEXT_JERSEY4,  "The Jersey's Jersey"      },
#endif
	// clang-format on
};
/**
 * Specifies a delta in X-coordinates from the quest entrance for
 * which the hover text of the cursor will be visible.
 */
char questxoff[7] = { 0, -1, 0, -1, -2, -1, -2 };
/**
 * Specifies a delta in Y-coordinates from the quest entrance for
 * which the hover text of the cursor will be visible.
 */
char questyoff[7] = { 0, 0, -1, -1, -1, -2, -2 };
const char *const questtrigstr[5] = {
	"King Leoric's Tomb",
	"The Chamber of Bone",
	"Maze",
	"A Dark Passage",
	"Unholy Altar"
};
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
	QuestData *qdata;
	int i, initiatedQuests;

	if (gbMaxPlayers == 1) {
		qs = quests;
		for (i = MAXQUESTS; i != 0; i--, qs++) {
			qs->_qactive = QUEST_NOTAVAIL;
		}
	} else {
		qs = quests;
		qdata = questlist;
		for (i = MAXQUESTS; i != 0; i--, qs++, qdata++) {
			if (!(qdata->_qflags & QUEST_ANY)) {
				qs->_qactive = QUEST_NOTAVAIL;
			}
		}
	}

	questlog = FALSE;
	WaterDone = 0;
	initiatedQuests = 0;

	qs = quests;
	qdata = questlist;
	for (i = 0; i < MAXQUESTS; i++, qs++, qdata++) {
		if (gbMaxPlayers != 1 && !(qdata->_qflags & QUEST_ANY))
			continue;
			qs->_qtype = qdata->_qdtype;
			if (gbMaxPlayers != 1) {
				qs->_qlevel = qdata->_qdmultlvl;
				if (!delta_quest_inited(initiatedQuests)) {
					qs->_qactive = QUEST_INIT;
					qs->_qvar1 = 0;
					qs->_qlog = FALSE;
				}
				initiatedQuests++;
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
			qs->_qlvltype = qdata->_qlvlt;
			qs->_qvar2 = 0;
			qs->_qmsg = qdata->_qdmsg;
	}

	if (gbMaxPlayers == 1) {
		SetRndSeed(glSeedTbl[15]);
		if (random_(0, 2))
			quests[Q_PWATER]._qactive = QUEST_NOTAVAIL;
		else
			quests[Q_SKELKING]._qactive = QUEST_NOTAVAIL;

		quests[QuestGroup1[random_(0, sizeof(QuestGroup1) / sizeof(int))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup2[random_(0, sizeof(QuestGroup2) / sizeof(int))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup3[random_(0, sizeof(QuestGroup3) / sizeof(int))]]._qactive = QUEST_NOTAVAIL;
		quests[QuestGroup4[random_(0, sizeof(QuestGroup4) / sizeof(int))]]._qactive = QUEST_NOTAVAIL;
	}
#ifdef _DEBUG
	if (questdebug != -1)
		quests[questdebug]._qactive = QUEST_ACTIVE;
#endif

#ifdef SPAWN
	for (i = 0; i < MAXQUESTS; i++) {
		quests[i]._qactive = QUEST_NOTAVAIL;
	}
#endif

	if (quests[Q_SKELKING]._qactive == QUEST_NOTAVAIL)
		quests[Q_SKELKING]._qvar2 = 2;
	if (quests[Q_ROCK]._qactive == QUEST_NOTAVAIL)
		quests[Q_ROCK]._qvar2 = 2;
	quests[Q_LTBANNER]._qvar1 = 1;
	if (gbMaxPlayers != 1)
		quests[Q_BETRAYER]._qvar1 = 2;
}

void CheckQuests()
{
#ifdef SPAWN
	return;
#endif

	QuestStruct *qs;
	int i, rportx, rporty;

	qs = &quests[Q_BETRAYER];
	if (QuestStatus(Q_BETRAYER) && gbMaxPlayers != 1 && qs->_qvar1 == 2) {
		AddObject(OBJ_ALTBOY, 2 * setpc_x + DBORDERX + 4, 2 * setpc_y + DBORDERY + 6);
		qs->_qvar1 = 3;
		NetSendCmdQuest(TRUE, Q_BETRAYER);
	}

	if (gbMaxPlayers != 1) {
		return;
	}

	if (currlevel == qs->_qlevel
	    && !setlevel
	    && qs->_qvar1 >= 2
	    && (qs->_qactive == QUEST_ACTIVE || qs->_qactive == QUEST_DONE)
	    && (qs->_qvar2 == 0 || qs->_qvar2 == 2)) {
		qs->_qtx = 2 * qs->_qtx + DBORDERX;
		qs->_qty = 2 * qs->_qty + DBORDERY;
		rportx = qs->_qtx;
		rporty = qs->_qty;
		AddMissile(rportx, rporty, rportx, rporty, 0, MIS_RPORTAL, 0, myplr, 0, 0);
		qs->_qvar2 = 1;
		if (qs->_qactive == QUEST_ACTIVE) {
			qs->_qvar1 = 3;
		}
	}

	if (qs->_qactive == QUEST_DONE
	    && setlevel
	    && setlvlnum == SL_VILEBETRAYER
	    && qs->_qvar2 == 4) {
		rportx = DBORDERX + 19;
		rporty = DBORDERY + 16;
		AddMissile(rportx, rporty, rportx, rporty, 0, MIS_RPORTAL, 0, myplr, 0, 0);
		qs->_qvar2 = 3;
	}

	if (setlevel) {
		qs = &quests[Q_PWATER];
		if (setlvlnum == qs->_qslvl
		    && qs->_qactive != QUEST_INIT
		    && leveltype == qs->_qlvltype
		    && nummonsters == MAX_PLRS
		    && qs->_qactive != QUEST_DONE) {
			qs->_qactive = QUEST_DONE;
			PlaySfxLoc(IS_QUESTDN, plr[myplr]._px, plr[myplr]._py);
			LoadPalette("Levels\\L3Data\\L3pwater.pal");
			WaterDone = 32;
		}
		if (WaterDone > 0) {
			palette_update_quest_palette(WaterDone);
			WaterDone--;
		}
	} else if (plr[myplr]._pmode == PM_STAND) {
		qs = quests;
		for (i = MAXQUESTS; i != 0; i--, qs++) {
			if (currlevel == qs->_qlevel
			    && qs->_qslvl != 0
			    && qs->_qactive != QUEST_NOTAVAIL
			    && plr[myplr]._px == qs->_qtx
			    && plr[myplr]._py == qs->_qty) {
				if (qs->_qlvltype != DTYPE_NONE) {
					setlvltype = qs->_qlvltype;
				}
				StartNewLvl(myplr, WM_DIABSETLVL, qs->_qslvl);
			}
		}
	}
}

BOOL ForceQuests()
{
	int i, j, qx, qy, ql;

#ifdef SPAWN
	return FALSE;
#endif

	if (gbMaxPlayers != 1) {
		return FALSE;
	}

	for (i = 0; i < MAXQUESTS; i++) {
		if (i != Q_BETRAYER && currlevel == quests[i]._qlevel && quests[i]._qslvl != 0) {
			ql = quests[quests[i]._qidx]._qslvl - 1;
			qx = quests[i]._qtx;
			qy = quests[i]._qty;

			for (j = 0; j < 7; j++) {
				if (qx + questxoff[j] == cursmx && qy + questyoff[j] == cursmy) {
					sprintf(infostr, "To %s", questtrigstr[ql]);
					cursmx = qx;
					cursmy = qy;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL QuestStatus(int qn)
{
	return currlevel == quests[qn]._qlevel
		&& quests[qn]._qactive != QUEST_NOTAVAIL
		&& (gbMaxPlayers == 1 || (questlist[qn]._qflags & QUEST_ANY))
		&& !setlevel;
}

void CheckQuestKill(int mnum, BOOL sendmsg)
{
	int i, j;

#ifdef SPAWN
	return;
#endif

	switch (monster[mnum]._uniqtype - 1) {
	case UMT_GARBUD: //"Gharbad the Weak"
		quests[Q_GARBUD]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_61][plr[myplr]._pClass];
		break;
	case UMT_SKELKING:
		quests[Q_SKELKING]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_82][plr[myplr]._pClass];
		if (sendmsg)
			NetSendCmdQuest(TRUE, Q_SKELKING);
		break;
	case UMT_ZHAR: //"Zhar the Mad"
		quests[Q_ZHAR]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_62][plr[myplr]._pClass];
		break;
	case UMT_LAZURUS: //"Arch-Bishop Lazarus" - multi
		if (gbMaxPlayers != 1) {
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			quests[Q_BETRAYER]._qvar1 = 7;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;

			for (j = 0; j < MAXDUNY; j++) {
				for (i = 0; i < MAXDUNX; i++) {
					if (dPiece[i][j] == 370) {
						trigs[numtrigs]._tx = i;
						trigs[numtrigs]._ty = j;
						trigs[numtrigs]._tmsg = WM_DIABNEXTLVL;
						numtrigs++;
					}
				}
			}
			if (sendmsg) {
				NetSendCmdQuest(TRUE, Q_BETRAYER);
				NetSendCmdQuest(TRUE, Q_DIABLO);
			}
		} else { //"Arch-Bishop Lazarus" - single
			quests[Q_BETRAYER]._qactive = QUEST_DONE;
			InitVPTriggers();
			quests[Q_BETRAYER]._qvar1 = 7;
			quests[Q_BETRAYER]._qvar2 = 4;
			quests[Q_DIABLO]._qactive = QUEST_ACTIVE;
			AddMissile(35, 32, 35, 32, 0, MIS_RPORTAL, 0, myplr, 0, 0);
		}
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_83][plr[myplr]._pClass];
		break;
	case UMT_WARLORD: //"Warlord of Blood"
		quests[Q_WARLORD]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_94][plr[myplr]._pClass];
		break;
	case UMT_BUTCHER:
		quests[Q_BUTCHER]._qactive = QUEST_DONE;
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_80][plr[myplr]._pClass];
		if (sendmsg)
			NetSendCmdQuest(TRUE, Q_BUTCHER);
		break;
	}
}

static void DrawButcher()
{
	int x, y;

	x = 2 * setpc_x + DBORDERX;
	y = 2 * setpc_y + DBORDERY;
	DRLG_RectTrans(x + 3, y + 3, x + 10, y + 10);
}

static void DrawSkelKing(int qn, int x, int y)
{
	quests[qn]._qtx = 2 * x + DBORDERX + 12;
	quests[qn]._qty = 2 * y + DBORDERY + 7;
}

static void DrawWarLord(int x, int y)
{
	int rw, rh;
	int i, j;
	BYTE *sp, *setp;
	int v;

	setp = LoadFileInMem("Levels\\L4Data\\Warlord2.DUN", NULL);
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
				v = 6;
			}
			dungeon[i][j] = v;
			sp += 2;
		}
	}
	mem_free_dbg(setp);
}

static void DrawSChamber(int qn, int x, int y)
{
	int i, j;
	int rw, rh;
	int xx, yy;
	BYTE *sp, *setp;
	int v;

	setp = LoadFileInMem("Levels\\L2Data\\Bonestr1.DUN", NULL);
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
				v = 3;
			}
			dungeon[i][j] = v;
			sp += 2;
		}
	}
	quests[qn]._qtx = 2 * x + 22;
	quests[qn]._qty = 2 * y + 23;
	mem_free_dbg(setp);
}

static void DrawLTBanner(int x, int y)
{
	int rw, rh;
	int i, j;
	BYTE *sp, *setp;

	setp = LoadFileInMem("Levels\\L1Data\\Banner1.DUN", NULL);
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

static void DrawBlind(int x, int y)
{
	int rw, rh;
	int i, j;
	BYTE *sp, *setp;

	setp = LoadFileInMem("Levels\\L2Data\\Blind1.DUN", NULL);
	rw = *setp;
	sp = setp + 2;
	rh = *sp;
	sp += 2;
	setpc_x = x;
	setpc_y = y;
	setpc_w = rw;
	setpc_h = rh;
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

static void DrawBlood(int x, int y)
{
	int rw, rh;
	int i, j;
	BYTE *sp, *setp;

	setp = LoadFileInMem("Levels\\L2Data\\Blood2.DUN", NULL);
	rw = *setp;
	sp = setp + 2;
	rh = *sp;
	sp += 2;
	setpc_x = x;
	setpc_y = y;
	setpc_w = rw;
	setpc_h = rh;
	rw += x;
	rh += y;
	for (j = y; j < rh; j++) {
		for (i = x; i < rw; i++) {
			if (*sp != 0) {
				dungeon[i][j] = *sp;
			}
			sp += 2;
		}
	}
	mem_free_dbg(setp);
}

void DRLG_CheckQuests(int x, int y)
{
	int i;

	for (i = 0; i < MAXQUESTS; i++) {
		if (QuestStatus(i)) {
			switch (quests[i]._qtype) {
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
				DrawSkelKing(i, x, y);
				break;
			case Q_SCHAMB:
				DrawSChamber(i, x, y);
				break;
			}
		}
	}
}

void SetReturnLvlPos()
{
	switch (setlvlnum) {
	case SL_SKELKING:
		ReturnLvlX = quests[Q_SKELKING]._qtx + 1;
		ReturnLvlY = quests[Q_SKELKING]._qty;
		ReturnLvl = quests[Q_SKELKING]._qlevel;
		ReturnLvlT = DTYPE_CATHEDRAL;
		break;
	case SL_BONECHAMB:
		ReturnLvlX = quests[Q_SCHAMB]._qtx + 1;
		ReturnLvlY = quests[Q_SCHAMB]._qty;
		ReturnLvl = quests[Q_SCHAMB]._qlevel;
		ReturnLvlT = DTYPE_CATACOMBS;
		break;
	case SL_POISONWATER:
		ReturnLvlX = quests[Q_PWATER]._qtx;
		ReturnLvlY = quests[Q_PWATER]._qty + 1;
		ReturnLvl = quests[Q_PWATER]._qlevel;
		ReturnLvlT = DTYPE_CATHEDRAL;
		break;
	case SL_VILEBETRAYER:
		ReturnLvlX = quests[Q_BETRAYER]._qtx + 1;
		ReturnLvlY = quests[Q_BETRAYER]._qty - 1;
		ReturnLvl = quests[Q_BETRAYER]._qlevel;
		ReturnLvlT = DTYPE_HELL;
		break;
	}
}

void GetReturnLvlPos()
{
	if (quests[Q_BETRAYER]._qactive == QUEST_DONE)
		quests[Q_BETRAYER]._qvar2 = 2;
	ViewX = ReturnLvlX;
	ViewY = ReturnLvlY;
	currlevel = ReturnLvl;
	leveltype = ReturnLvlT;
}

void ResyncMPQuests()
{
#ifdef SPAWN
	return;
#endif
	if (quests[Q_SKELKING]._qactive == QUEST_INIT
	    && currlevel >= quests[Q_SKELKING]._qlevel - 1
	    && currlevel <= quests[Q_SKELKING]._qlevel + 1) {
		quests[Q_SKELKING]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(TRUE, Q_SKELKING);
	}
	if (quests[Q_BUTCHER]._qactive == QUEST_INIT
	    && currlevel >= quests[Q_BUTCHER]._qlevel - 1
	    && currlevel <= quests[Q_BUTCHER]._qlevel + 1) {
		quests[Q_BUTCHER]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(TRUE, Q_BUTCHER);
	}
	if (quests[Q_BETRAYER]._qactive == QUEST_INIT && currlevel == quests[Q_BETRAYER]._qlevel - 1) {
		quests[Q_BETRAYER]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(TRUE, Q_BETRAYER);
	}
	if (QuestStatus(Q_BETRAYER))
		AddObject(OBJ_ALTBOY, 2 * setpc_x + DBORDERX + 4, 2 * setpc_y + DBORDERY + 6);
#ifdef HELLFIRE
	if (quests[Q_GRAVE]._qactive == QUEST_INIT && currlevel == quests[Q_GRAVE]._qlevel - 1) {
		quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(TRUE, Q_GRAVE);
	}
	if (quests[Q_DEFILER]._qactive == QUEST_INIT && currlevel == quests[Q_DEFILER]._qlevel - 1) {
		quests[Q_DEFILER]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(TRUE, Q_DEFILER);
	}
	if (quests[Q_NAKRUL]._qactive == QUEST_INIT && currlevel == quests[Q_NAKRUL]._qlevel - 1) {
		quests[Q_NAKRUL]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(TRUE, Q_NAKRUL);
	}
	if (quests[Q_JERSEY]._qactive == QUEST_INIT && currlevel == quests[Q_JERSEY]._qlevel - 1) {
		quests[Q_JERSEY]._qactive = QUEST_ACTIVE;
		NetSendCmdQuest(TRUE, Q_JERSEY);
	}
#endif
}

void ResyncQuests()
{
	int i, tren, x, y;

#ifdef SPAWN
	return;
#endif

	if (setlevel && setlvlnum == quests[Q_PWATER]._qslvl && quests[Q_PWATER]._qactive != QUEST_INIT && leveltype == quests[Q_PWATER]._qlvltype) {

		if (quests[Q_PWATER]._qactive == QUEST_DONE)
			LoadPalette("Levels\\L3Data\\L3pwater.pal");
		else
			LoadPalette("Levels\\L3Data\\L3pfoul.pal");

		for (i = 0; i <= 32; i++)
			palette_update_quest_palette(i);
	}

	if (QuestStatus(Q_LTBANNER)) {
		if (quests[Q_LTBANNER]._qvar1 == 1)
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
			tren = TransVal;
			TransVal = 9;
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1));
			TransVal = tren;
		}
		if (quests[Q_LTBANNER]._qvar1 == 3) {
			x = setpc_x;
			y = setpc_y;
			ObjChangeMapResync(x, y, x + setpc_w + 1, y + setpc_h + 1);
			for (i = 0; i < nobjects; i++)
				SyncObjectAnim(objectactive[i]);
			tren = TransVal;
			TransVal = 9;
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1));
			TransVal = tren;
		}
	}
	if (currlevel == quests[Q_MUSHROOM]._qlevel) {
		if (quests[Q_MUSHROOM]._qactive == QUEST_INIT && quests[Q_MUSHROOM]._qvar1 == QS_INIT) {
			SpawnQuestItem(IDI_FUNGALTM, 0, 0, 5, 1);
			quests[Q_MUSHROOM]._qvar1 = QS_TOMESPAWNED;
		} else {
			if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE) {
				if (quests[Q_MUSHROOM]._qvar1 >= QS_MUSHGIVEN) {
					Qtalklist[TOWN_WITCH]._qblkm = -1;
					Qtalklist[TOWN_HEALER]._qblkm = TEXT_MUSH3;
				} else if (quests[Q_MUSHROOM]._qvar1 >= QS_BRAINGIVEN) {
					Qtalklist[TOWN_HEALER]._qblkm = -1;
				}
			}
		}
	}
	if (currlevel == quests[Q_VEIL]._qlevel + 1 && quests[Q_VEIL]._qactive == QUEST_ACTIVE && quests[Q_VEIL]._qvar1 == 0) {
		quests[Q_VEIL]._qvar1 = 1;
		SpawnQuestItem(IDI_GLDNELIX, 0, 0, 5, 1);
	}
	if (setlevel && setlvlnum == SL_VILEBETRAYER) {
		if (quests[Q_BETRAYER]._qvar1 >= 4)
			ObjChangeMapResync(1, 11, 20, 18);
		if (quests[Q_BETRAYER]._qvar1 >= 6)
			ObjChangeMapResync(1, 18, 20, 24);
		if (quests[Q_BETRAYER]._qvar1 >= 7)
			InitVPTriggers();
		for (i = 0; i < nobjects; i++)
			SyncObjectAnim(objectactive[i]);
	}
	if (currlevel == quests[Q_BETRAYER]._qlevel
	    && !setlevel
	    && (quests[Q_BETRAYER]._qvar2 == 1 || quests[Q_BETRAYER]._qvar2 >= 3)
	    && (quests[Q_BETRAYER]._qactive == QUEST_ACTIVE || quests[Q_BETRAYER]._qactive == QUEST_DONE)) {
		quests[Q_BETRAYER]._qvar2 = 2;
	}
}

static void PrintQLString(int x, int y, BOOL cjustflag, const char *str, int col)
{
	int len, width, i, k, sx, sy;
	BYTE c;

	sx = x + 32 + SCREEN_X;
	sy = y * 12 + 44 + SCREEN_Y;
	len = strlen(str);
	k = 0;
	if (cjustflag) {
		width = 0;
		for (i = 0; i < len; i++)
			width += fontkern[fontframe[gbFontTransTbl[(BYTE)str[i]]]] + 1;
		if (width < 257)
			k = (257 - width) >> 1;
		sx += k;
	}
	if (qline == y) {
		CelDraw(sx - 20, sy + 1, pSPentSpn2Cels, PentSpn2Spin(), 12);
	}
	for (i = 0; i < len; i++) {
		c = fontframe[gbFontTransTbl[(BYTE)str[i]]];
		k += fontkern[c] + 1;
		if (c != '\0' && k <= 257) {
			PrintChar(sx, sy, c, col);
		}
		sx += fontkern[c] + 1;
	}
	if (qline == y) {
		CelDraw(cjustflag ? x + k + 36 + SCREEN_X : 276 + SCREEN_X - x, sy + 1, pSPentSpn2Cels, PentSpn2Spin(), 12);
	}
}

void DrawQuestLog()
{
	int y, i;

	PrintQLString(0, 2, TRUE, "Quest Log", 3);
	CelDraw(SCREEN_X, SCREEN_Y + 351, pQLogCel, 1, SPANEL_WIDTH);
	y = qtopline;
	for (i = 0; i < numqlines; i++) {
		PrintQLString(0, y, TRUE, questlist[qlist[i]]._qlstr, 0);
		y += 2;
	}
	PrintQLString(0, 22, TRUE, "Close Quest Log", 0);
}

void StartQuestlog()
{
	DWORD i;

	numqlines = 0;
	for (i = 0; i < MAXQUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && quests[i]._qlog) {
			qlist[numqlines] = i;
			numqlines++;
		}
	}
	if (numqlines > 5) {
		qtopline = 5 - (numqlines >> 1);
	} else {
		qtopline = 8;
	}
	qline = 22;
	if (numqlines != 0)
		qline = qtopline;
	questlog = TRUE;
}

void QuestlogUp()
{
	if (numqlines != 0) {
		if (qline == qtopline) {
			qline = 22;
		} else if (qline == 22) {
			qline = qtopline + 2 * numqlines - 2;
		} else {
			qline -= 2;
		}
		PlaySFX(IS_TITLEMOV);
	}
}

void QuestlogDown()
{
	if (numqlines != 0) {
		if (qline == 22) {
			qline = qtopline;
		} else if (qline == qtopline + 2 * numqlines - 2) {
			qline = 22;
		} else {
			qline += 2;
		}
		PlaySFX(IS_TITLEMOV);
	}
}

void QuestlogEnter()
{
	PlaySFX(IS_TITLSLCT);
	if (numqlines != 0 && qline != 22)
		InitQTextMsg(quests[qlist[(qline - qtopline) >> 1]]._qmsg);
	questlog = FALSE;
}

BOOL CheckQuestlog()
{
	int y, i;

	if (MouseX <= 32 || MouseX >= 288 || MouseY <= 32 || MouseY >= 308)
		return FALSE;

	y = (MouseY - 32) / 12;
	for (i = 0; i < numqlines; i++) {
		if (y == qtopline + 2 * i) {
			qline = y;
			QuestlogEnter();
		}
	}
	if (y == 22) {
		qline = 22;
		QuestlogEnter();
	}
	return TRUE;
}

void SetMultiQuest(int qn, int s, int l, int v1)
{
#ifdef SPAWN
	return;
#endif

	if (quests[qn]._qactive != QUEST_DONE) {
		if (s > quests[qn]._qactive)
			quests[qn]._qactive = s;
		quests[qn]._qlog |= l;
		if (v1 > quests[qn]._qvar1)
			quests[qn]._qvar1 = v1;
	}
}

DEVILUTION_END_NAMESPACE
