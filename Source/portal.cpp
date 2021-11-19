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

/*void SetPortalStats(int i, bool o, int x, int y, int lvl)
{
	portals[i]._wopen = o;
	portals[i].x = x;
	portals[i].y = y;
	portals[i].level = lvl;
}*/

void AddWarpMissile(int i, int x, int y)
{
	AddMissile(0, 0, x, y, 0, MIS_TOWN, 0, i, 0, 0, -1);
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

void AddInTownPortal(int i)
{
	AddWarpMissile(i, WarpDropX[i], WarpDropY[i]);
}

void ActivatePortal(int i, int x, int y, int lvl)
{
	// TODO: check data from internet
	assert(lvl != DLV_TOWN);
	portals[i]._wopen = true;
	portals[i].x = x;
	portals[i].y = y;
	portals[i].level = lvl;
}

void DeactivatePortal(int i)
{
	portals[i]._wopen = false;
}

bool PortalOnLevel(int i)
{
	return portals[i].level == currLvl._dLevelIdx || currLvl._dLevelIdx == DLV_TOWN;
}

void RemovePortalMissile(int pnum)
{
	MissileStruct *mis;
	int i, mi;

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		mis = &missile[mi];
		if (mis->_miType == MIS_TOWN && mis->_miSource == pnum) {
			dMissile[mis->_mix][mis->_miy] = 0;

			AddUnLight(mis->_miLid);

			DeleteMissile(mi, i);
		}
	}
}

void UseCurrentPortal(int p)
{
	portalindex = p;
	if (currLvl._dLevelIdx == DLV_TOWN && portalindex == mypnum) {
		NetSendCmd(CMD_DEACTIVATEPORTAL);
		//DeactivatePortal(portalindex);
	}
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
	ViewX++;
	ViewY++;
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
