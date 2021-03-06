/**
 * @file portal.cpp
 *
 * Implementation of functionality for handling town portals.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** In-game state of portals. */
PortalStruct portal[MAXPORTAL];
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
		if (!delta_portal_inited(i))
			portal[i]._wopen = false;
	}
}

/*void SetPortalStats(int i, bool o, int x, int y, int lvl)
{
	portal[i]._wopen = o;
	portal[i].x = x;
	portal[i].y = y;
	portal[i].level = lvl;
}*/

void AddWarpMissile(int i, int x, int y)
{
	dMissile[x][y] = 0;
	AddMissile(0, 0, x, y, 0, MIS_TOWN, 0, i, 0, 0, -1);
}

void SyncPortals()
{
	int i, lvl;

	for (i = 0; i < MAXPORTAL; i++) {
		if (!portal[i]._wopen)
			continue;
		lvl = currLvl._dLevelIdx;
		if (lvl == DLV_TOWN)
			AddWarpMissile(i, WarpDropX[i], WarpDropY[i]);
		else {
			if (portal[i].level == lvl)
				AddWarpMissile(i, portal[i].x, portal[i].y);
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
	portal[i]._wopen = true;
	portal[i].x = x;
	portal[i].y = y;
	portal[i].level = lvl;
}

void DeactivatePortal(int i)
{
	portal[i]._wopen = false;
}

bool PortalOnLevel(int i)
{
	return portal[i].level == currLvl._dLevelIdx || currLvl._dLevelIdx == DLV_TOWN;
}

void RemovePortalMissile(int pnum)
{
	MissileStruct *mis;
	int i, mi;

	for (i = 0; i < nummissiles; i++) {
		mi = missileactive[i];
		mis = &missile[mi];
		if (mis->_miType == MIS_TOWN && mis->_miSource == pnum) {
			dFlags[mis->_mix][mis->_miy] &= ~BFLAG_MISSILE;
			dMissile[mis->_mix][mis->_miy] = 0;

			AddUnLight(mis->_miLid);

			DeleteMissile(mi, i);
		}
	}
}

void SetCurrentPortal(int p)
{
	portalindex = p;
}

void GetPortalLevel()
{
	if (currLvl._dLevelIdx != DLV_TOWN) {
		EnterLevel(DLV_TOWN);
		assert(myplr.plrlevel == DLV_TOWN);
	} else {
		EnterLevel(portal[portalindex].level);
		assert(myplr.plrlevel == currLvl._dLevelIdx);
		if (portalindex == mypnum) {
			NetSendCmd(true, CMD_DEACTIVATEPORTAL);
			DeactivatePortal(portalindex);
		}
	}
}

void GetPortalLvlPos()
{
	if (currLvl._dLevelIdx == DLV_TOWN) {
		ViewX = WarpDropX[portalindex];
		ViewY = WarpDropY[portalindex];
	} else {
		ViewX = portal[portalindex].x;
		ViewY = portal[portalindex].y;
	}
	ViewX++;
	ViewY++;
}

bool PosOkPortal(int x, int y)
{
	PortalStruct *ps;
	int i, lvl = currLvl._dLevelIdx;

	ps = portal;
	for (i = MAXPORTAL; i != 0; i--, ps++) {
		if (ps->_wopen && ps->level == lvl && ((ps->x == x && ps->y == y) || (ps->x == x - 1 && ps->y == y - 1)))
			return false;
	}
	return true;
}

DEVILUTION_END_NAMESPACE
