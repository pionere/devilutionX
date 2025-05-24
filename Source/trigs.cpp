/**
 * @file trigs.cpp
 *
 * Implementation of functionality for triggering events when the player enters an area.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

int numtrigs;
TriggerStruct trigs[MAXTRIGGERS];
BYTE gbOpenWarps;
BYTE gbTWarpFrom;

typedef struct TownTriggerStruct {
	BYTE tox;
	BYTE toy;
	BYTE tolvl;
	BYTE totype;
} TownTriggerStruct;
static const TownTriggerStruct townTrigs[NUM_TWARP] = {
	// clang-format off
/*TWARP_CATHEDRAL*/ { 15 + DBORDERX, 19 + DBORDERY, DLV_CATHEDRAL1, WRPT_TOWN_L1 },
/*TWARP_CATACOMB*/  { 39 + DBORDERX, 11 + DBORDERY, DLV_CATACOMBS1, WRPT_TOWN_L2 },
/*TWARP_CAVES*/     {  7 + DBORDERX, 59 + DBORDERY, DLV_CAVES1, WRPT_TOWN_L3 },
/*TWARP_HELL*/      { 30 + DBORDERX, 69 + DBORDERY, DLV_HELL1, WRPT_TOWN_L4 },
#ifdef HELLFIRE
/*TWARP_CRYPT*/     { 26 + DBORDERX, 14 + DBORDERY, DLV_CRYPT1, WRPT_TOWN_L5 },
/*TWARP_NEST*/      { 70 + DBORDERX, 52 + DBORDERY, DLV_NEST1, WRPT_TOWN_L6 },
#endif
	// clang-format on
};

/*
 * Initialize the triggers in town.
 * Only the available triggers are set up correctly (numtrigs won't reflect this!).
 */
static void InitTownTriggers()
{
	BYTE ow = gbOpenWarps;

	static_assert(NUM_TWARP <= lengthof(trigs), "Too many trigger in town.");
	numtrigs = 0;
	for (int i = 0; i < NUM_TWARP; i++, ow >>= 1) {
		if (ow & 1) {
			trigs[numtrigs]._tx = townTrigs[i].tox;
			trigs[numtrigs]._ty = townTrigs[i].toy;
			trigs[numtrigs]._tmsg = DVL_DWM_TWARPDN;
			trigs[numtrigs]._tlvl = townTrigs[i].tolvl;
			trigs[numtrigs]._ttype = townTrigs[i].totype;
			numtrigs++;
		}
	}
}

static void InitDunTriggers()
{
	numtrigs = 0;
	if (currLvl._dDynLvl) {
		return;
	}
	for (int i = lengthof(pWarps) - 1; i >= 0; i--) {
		if (pWarps[i]._wx == 0) {
			continue;
		}
		if (i == DWARP_EXIT && currLvl._dLevelIdx == DLV_HELL3 && quests[Q_BETRAYER]._qactive != QUEST_DONE) {
			continue;
		}
		int tmsg;
		switch (i) {
		case DWARP_EXIT:  tmsg = DVL_DWM_NEXTLVL; break;
		case DWARP_ENTRY: tmsg = DVL_DWM_PREVLVL; break;
		case DWARP_TOWN:  tmsg = DVL_DWM_TWARPUP; break;
		case DWARP_SIDE:  tmsg = DVL_DWM_SETLVL;  break;
		default: ASSUME_UNREACHABLE; break;
		}
		trigs[numtrigs]._tx = pWarps[i]._wx;
		trigs[numtrigs]._ty = pWarps[i]._wy;
		trigs[numtrigs]._ttype = pWarps[i]._wtype;
		trigs[numtrigs]._tlvl = pWarps[i]._wlvl;
		trigs[numtrigs]._tmsg = tmsg;
		numtrigs++;
	}
}

static void InitSetDunTriggers()
{
	numtrigs = 0;

	// TODO: set tlvl in drlg_*
	int tlvl;
	switch (currLvl._dLevelIdx) {
	case SL_SKELKING:
		tlvl = questlist[Q_SKELKING]._qdlvl;
		break;
	case SL_BONECHAMB:
		tlvl = questlist[Q_BCHAMB]._qdlvl;
		break;
	//case SL_MAZE:
	//	break;
	case SL_POISONWATER:
		tlvl = questlist[Q_PWATER]._qdlvl;
		break;
	case SL_VILEBETRAYER:
		if (quests[Q_BETRAYER]._qvar1 >= QV_BETRAYER_DEAD) {
			tlvl = questlist[Q_BETRAYER]._qdlvl;

			trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;     // DBORDERX + 19
			trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy - 4; // DBORDERX + 16
			trigs[numtrigs]._tlvl = tlvl;
			trigs[numtrigs]._ttype = WRPT_RPORTAL;
			trigs[numtrigs]._tmsg = DVL_DWM_RTNLVL;
			numtrigs++;
			// TODO: set BFLAG_MON_PROTECT | BFLAG_OBJ_PROTECT? test if the missile exists?
			AddMissile(0, 0, trigs[0]._tx, trigs[0]._ty, 0, MIS_RPORTAL, MST_NA, -1, deltaload ? -1 : 0);
		}
		return;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;
	trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
	trigs[numtrigs]._ttype = pWarps[DWARP_ENTRY]._wtype;
	trigs[numtrigs]._tlvl = tlvl;
	trigs[numtrigs]._tmsg = DVL_DWM_RTNLVL;
	numtrigs++;
}

void InitVPEntryTrigger(bool recreate)
{
	int i;

	i = numtrigs; // quests[Q_BETRAYER]._qactive == QUEST_DONE ? 2 : 1;

	trigs[i]._tx = pWarps[DWARP_EXIT]._wx - 3;
	trigs[i]._ty = pWarps[DWARP_EXIT]._wy - 3;
	trigs[i]._tmsg = DVL_DWM_SETLVL;
	trigs[i]._tlvl = questlist[Q_BETRAYER]._qslvl;
	trigs[i]._ttype = WRPT_RPORTAL;
	numtrigs = i + 1;

	AddMissile(0, 0, trigs[i]._tx, trigs[i]._ty, 0, MIS_RPORTAL, MST_NA, -1, recreate ? -1 : 0);
}

void InitView(int entry)
{
	int type;

	if (entry == ENTRY_PORTLVL) {
		GetPortalLvlPos();
		return;
	}

	if (currLvl._dLevelIdx == DLV_TOWN) {
		if (entry == ENTRY_MAIN) {
			// New game
			ViewX = 65 + DBORDERX;
			ViewY = 58 + DBORDERY;
		//} else if (entry == ENTRY_PREV) { // Cathedral
		//	ViewX = 15 + DBORDERX;
		//	ViewY = 21 + DBORDERY;
		} else if (entry == ENTRY_TWARPUP) {
			switch (gbTWarpFrom) {
			case TWARP_CATHEDRAL:
				ViewX = 15 + DBORDERX;
				ViewY = 21 + DBORDERY;
				break;
			case TWARP_CATACOMB:
				ViewX = 39 + DBORDERX;
				ViewY = 12 + DBORDERY;
				break;
			case TWARP_CAVES:
				ViewX = 8 + DBORDERX;
				ViewY = 59 + DBORDERY;
				break;
			case TWARP_HELL:
				ViewX = 30 + DBORDERX;
				ViewY = 70 + DBORDERY;
				break;
#ifdef HELLFIRE
			case TWARP_CRYPT:
				ViewX = 26 + DBORDERX;
				ViewY = 15 + DBORDERY;
				break;
			case TWARP_NEST:
				ViewX = 69 + DBORDERX;
				ViewY = 52 + DBORDERY;
				break;
#endif
			default:
				ASSUME_UNREACHABLE
				break;
			}
		} else if (entry == ENTRY_RETOWN) {
			// Restart in Town
			ViewX = 63 + DBORDERX;
			ViewY = 70 + DBORDERY;
		}
		return;
	}

	switch (entry) {
	case ENTRY_MAIN:
		type = DWARP_ENTRY;
		break;
	case ENTRY_PREV:
		type = DWARP_EXIT;
		break;
	case ENTRY_SETLVL:
	case ENTRY_DYNLVL:
		type = DWARP_ENTRY;
		break;
	case ENTRY_RTNLVL:
		type = DWARP_SIDE;
		if (pWarps[type]._wtype == WRPT_NONE) {
			// return from the betrayer side-map - TODO: better solution?
			assert(currLvl._dLevelIdx == DLV_HELL3);
			type = DWARP_EXIT;
			ViewX = pWarps[type]._wx;
			ViewY = pWarps[type]._wy;
			assert(pWarps[type]._wtype == WRPT_L4_PENTA);
			ViewX += -2;
			ViewY += -2;
			return;
		}
		break;
	case ENTRY_LOAD:    // set from the save file
	case ENTRY_PORTLVL: // should not happen
		return;
	case ENTRY_TWARPDN:
		type = DWARP_TOWN;
		// if (pWarps[type]._wtype == WRPT_NONE)
		//	type = DWARP_ENTRY; // MAIN vs TWARPDN from town
		break;
	case ENTRY_TWARPUP: // should not happen
	case ENTRY_RETOWN:  // should not happen
		return;
	}

	ViewX = pWarps[type]._wx;
	ViewY = pWarps[type]._wy;
	type = pWarps[type]._wtype;
	switch (type) {
	case WRPT_NONE:
		break; // should not happen
	case WRPT_L1_UP:
		ViewX += 1;
		ViewY += 2;
		break;
	case WRPT_L1_DOWN:
		ViewX += 0;
		ViewY += 1;
		break;
	case WRPT_L1_SKING:
		ViewX += 1;
		ViewY += 0;
		break;
	case WRPT_L1_PWATER:
		ViewX += 0;
		ViewY += 1;
		break;
	case WRPT_L2_UP:
		ViewX += 1;
		ViewY += 0;
		break;
	case WRPT_L2_DOWN:
		ViewX += -1;
		ViewY += 0;
		break;
	case WRPT_L3_UP:
		ViewX += 0;
		ViewY += 1;
		break;
	case WRPT_L3_DOWN:
		ViewX += 1;
		ViewY += 0;
		break;
	case WRPT_L4_UP:
		ViewX += 0;
		ViewY += 1;
		break;
	case WRPT_L4_DOWN:
		ViewX += 1;
		ViewY += 0;
		break;
	case WRPT_L4_PENTA:
		ViewX += 0;
		ViewY += 1;
		break;
	case WRPT_CIRCLE:
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static int ForceTrig()
{
	for (int i = numtrigs - 1; i >= 0; i--) {
		int dx = pcurspos.x - trigs[i]._tx;
		int dy = pcurspos.y - trigs[i]._ty;
		switch (trigs[i]._ttype) {
		case WRPT_RPORTAL:
			continue;
		case WRPT_TOWN_L1:
			/*
				+++
				 +x
				  +
			 */
			dx += 2;
			dy += 1;
			if (dx < 0 || dx > 2)
				continue;
			if (dy < 0 || dy > 2)
				continue;
			if (dy > dx)
				continue;
			break;
		case WRPT_L1_UP:
			/*
				++
				++
				 x+
			 */
			dx += 1;
			dy += 2;
			if (dx < 0 || dx > 2)
				continue;
			if (dy < 0 || dy > 2)
				continue;
			if (dx == 0 && dy == 2)
				continue;
			if (dx == 2 && dy != 2)
				continue;
			break;
		case WRPT_L1_DOWN:
			/*
				++
				++
				+x
			 */
			dx += 1;
			dy += 2;
			if (dx < 0 || dx > 1)
				continue;
			if (dy < 0 || dy > 2)
				continue;
			break;
		case WRPT_TOWN_L2:
		case WRPT_L3_UP:
		case WRPT_L1_PWATER:
		case WRPT_L1_SKING:
		case WRPT_L2_UP:
		case WRPT_L4_DOWN:
		case WRPT_L4_PENTA:
			/*
				++
				+x
			 */
			dx += 1;
			dy += 1;
			if (dx < 0 || dx > 1)
				continue;
			if (dy < 0 || dy > 1)
				continue;
			break;
		case WRPT_L2_DOWN:
			/*
				 x+
			 */
			dx += 1;
			dy += 0;
			if (dx < 0 || dx > 1)
				continue;
			if (dy < 0 || dy > 0)
				continue;
			break;
		case WRPT_TOWN_L3:
			/*
				+++
				 +x
			 */
			dx += 2;
			dy += 1;
			if (dx < 0 || dx > 2)
				continue;
			if (dy < 0 || dy > 1)
				continue;
			if (dx == 0 && dy != 0)
				continue;
			break;
		case WRPT_L4_UP:
			/*
				 ++
				 x+
			 */
			dx += 0;
			dy += 1;
			if (dx < 0 || dx > 1)
				continue;
			if (dy < 0 || dy > 1)
				continue;
			break;
		case WRPT_L3_DOWN:
			/*
				 +
				 +x
			 */
			dx += 1;
			dy += 1;
			if (dx < 0 || dx > 1)
				continue;
			if (dy < 0 || dy > 1)
				continue;
			if (dx == 1 && dy != 0)
				continue;
			break;
		case WRPT_TOWN_L4:
			/*
				 ++++
				+x+++
			 */
			dx += 1;
			dy += 1;
			if (dx < 0 || dx > 4)
				continue;
			if (dy < 0 || dy > 1)
				continue;
			if (dx == 0 && dy == 0)
				continue;
			break;
			break;
#ifdef HELLFIRE
		case WRPT_TOWN_L6:
			/*
				 x+
				 ++
			 */
			dx += 0;
			dy += 0;
			if (dx < 0 || dx > 1)
				continue;
			if (dy < 0 || dy > 1)
				continue;
			break;
		case WRPT_TOWN_L5:
			/*
				 ++
				 ++
				 x+
			 */
			dx += 0;
			dy += 2;
			if (dx < 0 || dx > 1)
				continue;
			if (dy < 0 || dy > 2)
				continue;
			break;
#endif
		default:
			ASSUME_UNREACHABLE
		}
		return i;
	}

	return TRIG_NONE;
}

void InitTriggers()
{
	if (!currLvl._dSetLvl) {
		if (currLvl._dType == DTYPE_TOWN)
			InitTownTriggers();
		else
			InitDunTriggers();
	} else {
		InitSetDunTriggers();
	}
}

void CheckTrigForce()
{
	pcurstrig = ForceTrig();
	if (TRIG_VALID(pcurstrig)) {
		pcurspos.x = trigs[pcurstrig]._tx;
		pcurspos.y = trigs[pcurstrig]._ty;
	}
}

void CheckTriggers()
{
	PlayerStruct* p;
	int i, fom, lvl;

	p = &myplr;
	if (p->_pmode != PM_STAND)
		return;

	for (i = 0; i < numtrigs; i++) {
		if (p->_px != trigs[i]._tx || p->_py != trigs[i]._ty) {
			continue;
		}

		fom = trigs[i]._tmsg;
		switch (fom) {
		case DVL_DWM_NEXTLVL:
			lvl = currLvl._dLevelIdx + 1;
			break;
		case DVL_DWM_PREVLVL:
			lvl = currLvl._dLevelIdx - 1;
			break;
		case DVL_DWM_RTNLVL:
		case DVL_DWM_SETLVL:
			lvl = trigs[i]._tlvl;
			break;
		case DVL_DWM_TWARPDN:
			lvl = trigs[i]._tlvl;
			break;
		case DVL_DWM_TWARPUP:
			lvl = DLV_TOWN;
			break;
		default:
			ASSUME_UNREACHABLE;
			break;
		}
		static_assert((int)DVL_DWM_TWARPUP < UCHAR_MAX, "CheckTriggers sends DVL_DWM_* value in BYTE field I.");
		static_assert((int)DVL_DWM_NEXTLVL < UCHAR_MAX, "CheckTriggers sends DVL_DWM_* value in BYTE field II.");
		static_assert((int)NUM_LEVELS < UCHAR_MAX, "CheckTriggers sends level index in BYTE field.");
		NetSendCmdNewLvl(fom, lvl);
		numtrigs = 0; // prevent triggering again
	}
}

DEVILUTION_END_NAMESPACE
