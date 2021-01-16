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
		if (delta_portal_inited(i))
			portal[i].open = FALSE;
	}
}

void SetPortalStats(int i, BOOL o, int x, int y, int lvl, int lvltype)
{
	portal[i].open = o;
	portal[i].x = x;
	portal[i].y = y;
	portal[i].level = lvl;
	portal[i].ltype = lvltype;
	portal[i].setlvl = FALSE;
}

void AddWarpMissile(int i, int x, int y)
{
	dMissile[x][y] = 0;
	AddMissile(0, 0, x, y, 0, MIS_TOWN, 0, i, 0, -1);
}

void SyncPortals()
{
	int i, lvl;

	for (i = 0; i < MAXPORTAL; i++) {
		if (!portal[i].open)
			continue;
		lvl = currlevel;
		if (lvl == 0)
			AddWarpMissile(i, WarpDropX[i], WarpDropY[i]);
		else {
			if (setlevel)
				lvl = setlvlnum;
			if (portal[i].level == lvl)
				AddWarpMissile(i, portal[i].x, portal[i].y);
		}
	}
}

void AddInTownPortal(int i)
{
	AddWarpMissile(i, WarpDropX[i], WarpDropY[i]);
}

void ActivatePortal(int i, int x, int y, int lvl, int lvltype, BOOL sp)
{
	portal[i].open = TRUE;

	if (lvl != 0) {
		portal[i].x = x;
		portal[i].y = y;
		portal[i].level = lvl;
		portal[i].ltype = lvltype;
		portal[i].setlvl = sp;
	}
}

void DeactivatePortal(int i)
{
	portal[i].open = FALSE;
}

BOOL PortalOnLevel(int i)
{
	if (portal[i].level == currlevel)
		return TRUE;
	else
		return currlevel == 0;
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

			if (portal[pnum].level != 0)
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
	if (currlevel != 0) {
		setlevel = FALSE;
		currlevel = 0;
		plr[myplr].plrlevel = 0;
		leveltype = DTYPE_TOWN;
	} else {
		leveltype = portal[portalindex].ltype;
		currlevel = portal[portalindex].level;
		plr[myplr].plrlevel = currlevel;
		setlevel = portal[portalindex].setlvl;
		if (setlevel)
			setlvlnum = currlevel;
		if (portalindex == myplr) {
			NetSendCmd(TRUE, CMD_DEACTIVATEPORTAL);
			DeactivatePortal(portalindex);
		}
	}
}

void GetPortalLvlPos()
{
	if (currlevel == 0) {
		ViewX = WarpDropX[portalindex] + 1;
		ViewY = WarpDropY[portalindex] + 1;
	} else {
		ViewX = portal[portalindex].x;
		ViewY = portal[portalindex].y;

		if (portalindex != myplr) {
			ViewX++;
			ViewY++;
		}
	}
}

BOOL PosOkPortal(int x, int y)
{
	PortalStruct *ps;
	int i, lvl = currlevel;

	ps = portal;
	for (i = MAXPORTAL; i != 0; i--, ps++) {
		if (ps->open && ps->level == lvl && ((ps->x == x && ps->y == y) || (ps->x == x - 1 && ps->y == y - 1)))
			return FALSE;
	}
	return TRUE;
}

DEVILUTION_END_NAMESPACE
