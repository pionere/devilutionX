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

static void InitNoTriggers()
{
	numtrigs = 0;
}

/*
 * Initialize the triggers in town.
 * Only the available triggers are set up correctly (numtrigs won't reflect this!).
 */
static void InitTownTriggers()
{
	assert(gbOpenWarps & (1 << TWARP_CATHEDRAL));
	trigs[TWARP_CATHEDRAL]._tx = 15 + DBORDERX;
	trigs[TWARP_CATHEDRAL]._ty = 19 + DBORDERY;
	trigs[TWARP_CATHEDRAL]._tmsg = DVL_DWM_TWARPDN;
	trigs[TWARP_CATHEDRAL]._tlvl = DLV_CATHEDRAL1;
	trigs[TWARP_CATHEDRAL]._ttype = TRT_TOWN_L1;

	if (gbOpenWarps & (1 << TWARP_CATACOMB)) {
		trigs[TWARP_CATACOMB]._tx = 39 + DBORDERX;
		trigs[TWARP_CATACOMB]._ty = 11 + DBORDERY;
		trigs[TWARP_CATACOMB]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_CATACOMB]._tlvl = DLV_CATACOMBS1;
		trigs[TWARP_CATACOMB]._ttype = TRT_TOWN_L2;
	}
	if (gbOpenWarps & (1 << TWARP_CAVES)) {
		trigs[TWARP_CAVES]._tx = 7 + DBORDERX;
		trigs[TWARP_CAVES]._ty = 59 + DBORDERY;
		trigs[TWARP_CAVES]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_CAVES]._tlvl = DLV_CAVES1;
		trigs[TWARP_CAVES]._ttype = TRT_TOWN_L3;
	}
	if (gbOpenWarps & (1 << TWARP_HELL)) {
		trigs[TWARP_HELL]._tx = 30 + DBORDERX;
		trigs[TWARP_HELL]._ty = 69 + DBORDERY;
		trigs[TWARP_HELL]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_HELL]._tlvl = DLV_HELL1;
		trigs[TWARP_HELL]._ttype = TRT_TOWN_L4;
	}
#ifdef HELLFIRE
	if (gbOpenWarps & (1 << TWARP_NEST)) {
		trigs[TWARP_NEST]._tx = 70 + DBORDERX;
		trigs[TWARP_NEST]._ty = 52 + DBORDERY;
		trigs[TWARP_NEST]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_NEST]._tlvl = DLV_NEST1;
		trigs[TWARP_NEST]._ttype = TRT_TOWN_L6;
	}
	if (gbOpenWarps & (1 << TWARP_CRYPT)) {
		trigs[TWARP_CRYPT]._tx = 26 + DBORDERX;
		trigs[TWARP_CRYPT]._ty = 14 + DBORDERY;
		trigs[TWARP_CRYPT]._tmsg = DVL_DWM_TWARPDN;
		trigs[TWARP_CRYPT]._tlvl = DLV_CRYPT1;
		trigs[TWARP_CRYPT]._ttype = TRT_TOWN_L5;
	}
#endif
	static_assert(NUM_TWARP <= lengthof(trigs), "Too many trigger in town.");
	numtrigs = NUM_TWARP;
}

static void InitL1Triggers()
{
	numtrigs = 0;
	if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		trigs[numtrigs]._ttype = TRT_L1_UP;
		numtrigs++;
	}
	// if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		trigs[numtrigs]._ttype = TRT_L1_DOWN;
		numtrigs++;
	// }
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		trigs[numtrigs]._ttype = TRT_L1_TOWN;
		numtrigs++;
	}
	if (pWarps[DWARP_SIDE]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_SIDE]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_SIDE]._wy;
		if (currLvl._dLevelIdx == questlist[Q_SKELKING]._qdlvl) { // TODO: add qn to pWarps?
			trigs[numtrigs]._tlvl = questlist[Q_SKELKING]._qslvl;
			trigs[numtrigs]._ty += 1;
			trigs[numtrigs]._ttype = TRT_L1_SKING;
		} else {
			trigs[numtrigs]._tlvl = questlist[Q_PWATER]._qslvl;
			trigs[numtrigs]._tx += 1;
			trigs[numtrigs]._ttype = TRT_L1_PWATER;
		}
		trigs[numtrigs]._tmsg = DVL_DWM_SETLVL;
		numtrigs++;
	}
}

static void InitL2Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		trigs[numtrigs]._ttype = TRT_L2_UP;
		numtrigs++;
	// }
	// if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		trigs[numtrigs]._ttype = TRT_L2_DOWN;
		numtrigs++;
	// }
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		trigs[numtrigs]._ttype = TRT_L2_TOWN;
		numtrigs++;
	}
	if (pWarps[DWARP_SIDE]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_SIDE]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_SIDE]._wy + 1;
		trigs[numtrigs]._tlvl = questlist[Q_BCHAMB]._qslvl;
		trigs[numtrigs]._tmsg = DVL_DWM_SETLVL;
		trigs[numtrigs]._ttype = TRT_L2_BCHAMB;
		numtrigs++;
	}
}

static void InitL3Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		trigs[numtrigs]._ttype = TRT_L3_UP;
		numtrigs++;
	// }
	// if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		trigs[numtrigs]._ttype = TRT_L3_DOWN;
		numtrigs++;
	// }
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		trigs[numtrigs]._ttype = TRT_L3_TOWN;
		numtrigs++;
	}
}

static void InitL4Triggers()
{
	numtrigs = 0;
	// if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		trigs[numtrigs]._ttype = TRT_L4_UP;
		numtrigs++;
	// }
	if (pWarps[DWARP_EXIT]._wx != 0) {
		if (currLvl._dLevelIdx != DLV_HELL3) {
			trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx + 1;
			trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
			trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
			trigs[numtrigs]._ttype = TRT_L4_DOWN;
			numtrigs++;
		} else if (quests[Q_BETRAYER]._qactive == QUEST_DONE) {
			trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx + 1;
			trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
			trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
			trigs[numtrigs]._ttype = TRT_L4_DOWN;
			numtrigs++;
		}
	}
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		trigs[numtrigs]._ttype = TRT_L4_TOWN;
		numtrigs++;
	}
}

#ifdef HELLFIRE
static void InitL5Triggers()
{
	numtrigs = 0;
	if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		trigs[numtrigs]._ttype = TRT_L5_UP;
		numtrigs++;
	}
	if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		trigs[numtrigs]._ttype = TRT_L5_DOWN;
		numtrigs++;
	}
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		trigs[numtrigs]._ttype = TRT_L5_TOWN;
		numtrigs++;
	}
}

static void InitL6Triggers()
{
	numtrigs = 0;
	if (pWarps[DWARP_ENTRY]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_ENTRY]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_ENTRY]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_PREVLVL;
		trigs[numtrigs]._ttype = TRT_L6_UP;
		numtrigs++;
	}
	if (pWarps[DWARP_EXIT]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_EXIT]._wx;
		trigs[numtrigs]._ty = pWarps[DWARP_EXIT]._wy + 1;
		trigs[numtrigs]._tmsg = DVL_DWM_NEXTLVL;
		trigs[numtrigs]._ttype = TRT_L6_DOWN;
		numtrigs++;
	}
	if (pWarps[DWARP_TOWN]._wx != 0) {
		trigs[numtrigs]._tx = pWarps[DWARP_TOWN]._wx + 1;
		trigs[numtrigs]._ty = pWarps[DWARP_TOWN]._wy;
		trigs[numtrigs]._tmsg = DVL_DWM_TWARPUP;
		trigs[numtrigs]._ttype = TRT_L6_TOWN;
		numtrigs++;
	}
}
#endif

static void InitSKingTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx; // DBORDERX + 66
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy; // DBORDERY + 26
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_SKELKING]._qdlvl;
	trigs[0]._ttype = TRT_SKING_L1;
}

static void InitSChambTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx;     // DBORDERX + 54
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy + 1; // DBORDERY + 23
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_BCHAMB]._qdlvl;
	trigs[0]._ttype = TRT_BCHAMB_L2;
}

static void InitPWaterTriggers()
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx;     // DBORDERX + 14
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy + 1; // DBORDERY + 67
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_PWATER]._qdlvl;
	trigs[0]._ttype = TRT_PWATER_L1;
}

void InitVPEntryTrigger(bool recreate)
{
	int i;

	i = numtrigs; // quests[Q_BETRAYER]._qactive == QUEST_DONE ? 2 : 1;

	trigs[i]._tx = pWarps[DWARP_EXIT]._wx - 2;
	trigs[i]._ty = pWarps[DWARP_EXIT]._wy - 2;
	trigs[i]._tmsg = DVL_DWM_SETLVL;
	trigs[i]._tlvl = questlist[Q_BETRAYER]._qslvl;
	trigs[i]._ttype = TRT_L4_BETR;
	numtrigs = i + 1;

	AddMissile(0, 0, trigs[i]._tx, trigs[i]._ty, 0, MIS_RPORTAL, MST_NA, -1, recreate ? -1 : 0);
}

void InitVPReturnTrigger(bool recreate)
{
	numtrigs = 1;
	trigs[0]._tx = pWarps[DWARP_ENTRY]._wx;     // DBORDERX + 19
	trigs[0]._ty = pWarps[DWARP_ENTRY]._wy - 4; // DBORDERX + 16
	trigs[0]._tmsg = DVL_DWM_RTNLVL;
	trigs[0]._tlvl = questlist[Q_BETRAYER]._qdlvl;
	trigs[0]._ttype = TRT_BETR_L4;
	// TODO: set BFLAG_MON_PROTECT | BFLAG_OBJ_PROTECT?
	AddMissile(0, 0, trigs[0]._tx, trigs[0]._ty, 0, MIS_RPORTAL, MST_NA, -1, recreate ? -1 : 0);
}

void InitView(int entry)
{
	int type;

	if (entry == ENTRY_WARPLVL) {
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
			ViewX += -1;
			ViewY += -1;
			return;
		}
		break;
	case ENTRY_LOAD:    // set from the save file
	case ENTRY_WARPLVL: // should not happen
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
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L1_SKING:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L1_PWATER:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L2_UP:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L2_DOWN:
		ViewX += -1;
		ViewY += 1;
		break;
	case WRPT_L3_UP:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L3_DOWN:
		ViewX += 1;
		ViewY += 1;
		break;
	case WRPT_L4_UP:
		ViewX += 1;
		ViewY += 2;
		break;
	case WRPT_L4_DOWN:
		ViewX += 2;
		ViewY += 1;
		break;
	case WRPT_L4_PENTA:
		ViewX += 1;
		ViewY += 2;
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
		case TRT_L4_BETR:
		case TRT_BETR_L4:
			continue;
		case TRT_TOWN_L1:
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
		case TRT_L1_UP:
		case TRT_L1_TOWN:
		case TRT_SKING_L1:
#ifdef HELLFIRE
		case TRT_L5_UP:
		case TRT_L5_TOWN:
#endif
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
		case TRT_L1_DOWN:
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
		case TRT_TOWN_L2:
		case TRT_L3_UP:
		case TRT_L3_TOWN:
		case TRT_L1_PWATER:
		case TRT_L1_SKING:
		case TRT_L4_DOWN:
		case TRT_L4_PENTA:
#ifdef HELLFIRE
		case TRT_L5_DOWN:
		case TRT_L6_UP:
		case TRT_L6_TOWN:
		case TRT_L6_DOWN:
#endif
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
		case TRT_L2_UP:
		case TRT_L2_TOWN:
		case TRT_L2_BCHAMB:
			/*
				 +
				 x
			 */
			dx += 0;
			dy += 1;
			if (dx < 0 || dx > 0)
				continue;
			if (dy < 0 || dy > 1)
				continue;
			break;
		case TRT_L2_DOWN:
		case TRT_BCHAMB_L2:
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
		case TRT_TOWN_L3:
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
		case TRT_L4_UP:
		case TRT_L4_TOWN:
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
		case TRT_L3_DOWN:
		case TRT_PWATER_L1:
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
		case TRT_TOWN_L4:
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
		case TRT_TOWN_L6:
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
		case TRT_TOWN_L5:
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

	return -1;
}

void InitTriggers()
{
	if (!currLvl._dSetLvl) {
		switch (currLvl._dType) {
		case DTYPE_TOWN:
			InitTownTriggers();
			return;
		case DTYPE_CATHEDRAL:
			InitL1Triggers();
			break;
		case DTYPE_CATACOMBS:
			InitL2Triggers();
			break;
		case DTYPE_CAVES:
			InitL3Triggers();
			break;
		case DTYPE_HELL:
			InitL4Triggers();
			break;
#ifdef HELLFIRE
		case DTYPE_CRYPT:
			InitL5Triggers();
			break;
		case DTYPE_NEST:
			InitL6Triggers();
			break;
#endif
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} else {
		switch (currLvl._dLevelIdx) {
		case SL_SKELKING:
			InitSKingTriggers();
			break;
		case SL_BONECHAMB:
			InitSChambTriggers();
			break;
		//case SL_MAZE:
		//	break;
		case SL_POISONWATER:
			InitPWaterTriggers();
			break;
		case SL_VILEBETRAYER:
			InitNoTriggers();
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}
}

void CheckTrigForce()
{
	pcurstrig = ForceTrig();
	if (pcurstrig != -1) {
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
			if (!(gbOpenWarps & (1 << i)))
				continue;
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
		NetSendCmdBParam2(CMD_NEWLVL, fom, lvl);
		numtrigs = 0; // prevent triggering again
	}
}

DEVILUTION_END_NAMESPACE
