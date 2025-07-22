/**
 * @file portal.cpp
 *
 * Implementation of functionality for handling town portals.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** In-game state of portals. */
PortalStruct portals[MAXPORTAL];
/** Current portal number (a portal array index). */
static int portalindex;

/** X-coordinate of each players portal in town. */
static_assert(MAXPORTAL <= 4, "Portal coordinates in town must be set.");
static const int WarpDropX[MAXPORTAL] = { 47 + DBORDERX, 49 + DBORDERX, 51 + DBORDERX, 53 + DBORDERX };
/** Y-coordinate of each players portal in town. */
static const int WarpDropY[MAXPORTAL] = { 30 + DBORDERY, 30 + DBORDERY, 30 + DBORDERY, 30 + DBORDERY };

void InitPortals()
{
	int i;

	for (i = 0; i < MAXPORTAL; i++) {
		portals[i]._rlevel = DLV_TOWN;
	}
}

/*void SetPortalStats(int pidx, int x, int y, int lvl)
{
	portals[pidx]._rx = x;
	portals[pidx]._ry = y;
	portals[pidx]._rlevel = lvl;
}*/

static void AddWarpMissile(int pidx, int x, int y)
{
	static_assert(MAXPORTAL == MAX_PLRS, "AddWarpMissile adds portal-missiles by portal-id.");
	AddMissile(0, 0, x, y, 0, MIS_TOWN, MST_NA, pidx, -1);
}

void SyncPortals()
{
	int i, lvl;

	for (i = 0; i < MAXPORTAL; i++) {
		if (portals[i]._rlevel == DLV_TOWN)
			continue;
		lvl = currLvl._dLevelIdx;
		if (lvl == DLV_TOWN)
			AddWarpMissile(i, WarpDropX[i], WarpDropY[i]);
		else {
			if (portals[i]._rlevel == lvl)
				AddWarpMissile(i, portals[i]._rx, portals[i]._ry);
		}
	}
}

void AddInTownPortal(int pidx)
{
	AddWarpMissile(pidx, WarpDropX[pidx], WarpDropY[pidx]);
}

void ActivatePortal(int pidx, int x, int y, int bLevel)
{
	// assert(bLevel != DLV_TOWN);
	net_assert(bLevel < NUM_LEVELS);
	portals[pidx]._rx = x;
	portals[pidx]._ry = y;
	portals[pidx]._rlevel = bLevel;
}

//static bool PortalOnLevel(int pidx)
//{
//	return portals[pidx]._rlevel == currLvl._dLevelIdx || currLvl._dLevelIdx == DLV_TOWN;
//}

void DeactivatePortal(int pidx)
{
	//if (PortalOnLevel(pidx)) - skip test because portals and missiles might be out of sync temporary
	{
		static_assert(MAXPORTAL == MAX_PLRS, "DeactivatePortal removes portal-missiles by portal-id.");
		RemovePortalMissile(pidx);
	}

	portals[pidx]._rlevel = DLV_TOWN;
}

void UseCurrentPortal(int pidx)
{
	portalindex = pidx;
}

void GetPortalLvlPos()
{
	if (currLvl._dLevelIdx == DLV_TOWN) {
		ViewX = WarpDropX[portalindex];
		ViewY = WarpDropY[portalindex];
	} else {
		ViewX = portals[portalindex]._rx;
		ViewY = portals[portalindex]._ry;
	}
}

bool PosOkPortal(int x, int y)
{
	int i, lvl = currLvl._dLevelIdx;

	for (i = 0; i < MAXPORTAL; i++) {
		if (portals[i]._rlevel == DLV_TOWN)
			continue;
		if (lvl == DLV_TOWN) {
			if (WarpDropX[i] == x && WarpDropY[i] == y)
				return false;
		} else {
			if (portals[i]._rx == x && portals[i]._ry == y)
				return false;
		}
	}
	return true;
}

DEVILUTION_END_NAMESPACE
