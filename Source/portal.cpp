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
int portalindex;

/** X-coordinate of each players portal in town. */
int WarpDropX[MAXPORTAL] = { 47 + DBORDERX, 49 + DBORDERX, 51 + DBORDERX, 53 + DBORDERX };
/** Y-coordinate of each players portal in town. */
int WarpDropY[MAXPORTAL] = { 30 + DBORDERY, 30 + DBORDERY, 30 + DBORDERY, 30 + DBORDERY };

void InitPortals()
{
	int i;

	for (i = 0; i < MAXPORTAL; i++) {
		portals[i]._wopen = false;
	}
}

/*void SetPortalStats(int pidx, bool o, int x, int y, int lvl)
{
	portals[pidx]._wopen = o;
	portals[pidx].x = x;
	portals[pidx].y = y;
	portals[pidx].level = lvl;
}*/

void AddWarpMissile(int pidx, int x, int y)
{
	AddMissile(0, 0, x, y, 0, MIS_TOWN, MST_NA, pidx, -1);
}

void SyncPortals()
{
	int i, lvl;

	for (i = 0; i < MAXPORTAL; i++) {
		if (!portals[i]._wopen)
			continue;
		lvl = currLvl._dLevelIdx;
		if (lvl == DLV_TOWN)
			AddWarpMissile(i, WarpDropX[i], WarpDropY[i]);
		else {
			if (portals[i].level == lvl)
				AddWarpMissile(i, portals[i].x, portals[i].y);
		}
	}
}

void AddInTownPortal(int pidx)
{
	AddWarpMissile(pidx, WarpDropX[pidx], WarpDropY[pidx]);
}

void ActivatePortal(int pidx, int x, int y, int lvl)
{
	assert(lvl != DLV_TOWN);
	portals[pidx]._wopen = true;
	portals[pidx].x = x;
	portals[pidx].y = y;
	portals[pidx].level = lvl;

	delta_open_portal(pidx, x, y, lvl);
}

static bool PortalOnLevel(int pidx)
{
	return portals[pidx].level == currLvl._dLevelIdx || currLvl._dLevelIdx == DLV_TOWN;
}

void RemovePortalMissile(int pidx)
{
	MissileStruct* mis;
	int i;

	if (!PortalOnLevel(pidx))
		return;

	static_assert(MAXPORTAL == MAX_PLRS, "RemovePortalMissile finds portal-missiles by portal-id.");
	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		if (mis->_miType == MIS_TOWN && mis->_miSource == pidx) {
			mis->_miDelFlag = TRUE;
			AddUnLight(mis->_miLid);
		}
	}
}

void DeactivatePortal(int pidx)
{
	portals[pidx]._wopen = false;

	RemovePortalMissile(pidx);
	delta_close_portal(pidx);
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
		ViewX = portals[portalindex].x;
		ViewY = portals[portalindex].y;
	}
}

bool PosOkPortal(int x, int y)
{
	int i, lvl = currLvl._dLevelIdx;

	for (i = 0; i < MAXPORTAL; i++) {
		if (!portals[i]._wopen)
			continue;
		if (lvl == DLV_TOWN) {
			if (WarpDropX[i] == x && WarpDropY[i] == y)
				return false;
		} else {
			if (portals[i].x == x && portals[i].y == y)
				return false;
		}
	}
	return true;
}

DEVILUTION_END_NAMESPACE
