/**
 * @file cursor.cpp
 *
 * Implementation of cursor tracking functionality.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Pixel width of the current cursor image */
int cursW;
/** Pixel height of the current cursor image */
int cursH;
/** Current highlighted monster */
int pcursmonst = -1;
/** Width of current cursor in inventory cells */
int icursW28;
/** Height of current cursor in inventory cells */
int icursH28;
/** Cursor images CEL */
BYTE *pCursCels;
#ifdef HELLFIRE
BYTE *pCursCels2;
#endif

/** inv_item value */
char pcursinvitem;
/** Pixel width of the current cursor image */
int icursW;
/** Pixel height of the current cursor image */
int icursH;
/** Current highlighted item */
char pcursitem;
/** Current highlighted object */
char pcursobj;
/** Current highlighted player */
char pcursplr;
/** Current highlighted tile row */
int cursmx;
/** Current highlighted tile column */
int cursmy;
/** Previously highlighted monster */
int pcurstemp;
/** Index of current cursor image */
int pcurs;

/* rdata */
/** Maps from objcurs.cel frame number to frame width. */
const int InvItemWidth[] = {
	// clang-format off
	// Cursors
	0, 33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 23,
	// Items
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
#ifdef HELLFIRE
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	2 * 28, 2 * 28, 1 * 28, 1 * 28, 1 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28
#endif
	// clang-format on
};

/** Maps from objcurs.cel frame number to frame height. */
const int InvItemHeight[] = {
	// clang-format off
	// Cursors
	0, 29, 32, 32, 32, 32, 32, 32, 32, 32, 32, 35,
	// Items
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
#ifdef HELLFIRE
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	2 * 28, 2 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28
#endif
	// clang-format on
};

void InitCursor()
{
	assert(!pCursCels);
	pCursCels = LoadFileInMem("Data\\Inv\\Objcurs.CEL", NULL);
#ifdef HELLFIRE
	pCursCels2 = LoadFileInMem("Data\\Inv\\Objcurs2.CEL", NULL);
#endif
	ClearCursor();
}

void FreeCursor()
{
	MemFreeDbg(pCursCels);
#ifdef HELLFIRE
	MemFreeDbg(pCursCels2);
#endif
	ClearCursor();
}

void SetICursor(int i)
{
	icursW = InvItemWidth[i];
	icursH = InvItemHeight[i];
	icursW28 = icursW / 28;
	icursH28 = icursH / 28;
}

void SetCursor_(int i)
{
	pcurs = i;
	cursW = InvItemWidth[i];
	cursH = InvItemHeight[i];
	SetICursor(i);
}

void NewCursor(int i)
{
	SetCursor_(i);
}

void InitLevelCursor()
{
	SetCursor_(CURSOR_HAND);
	cursmx = ViewX;
	cursmy = ViewY;
	pcurstemp = -1;
	pcursmonst = -1;
	pcursobj = -1;
	pcursitem = -1;
	pcursplr = -1;
	ClearCursor();
}

void CheckTown()
{
	MissileStruct *mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		if (mis->_mitype == MIS_TOWN) {
			if (cursmx == mis->_mix - 1 && cursmy == mis->_miy
			    || cursmx == mis->_mix && cursmy == mis->_miy - 1
			    || cursmx == mis->_mix - 1 && cursmy == mis->_miy - 1
			    || cursmx == mis->_mix - 2 && cursmy == mis->_miy - 1
			    || cursmx == mis->_mix - 2 && cursmy == mis->_miy - 2
			    || cursmx == mis->_mix - 1 && cursmy == mis->_miy - 2
			    || cursmx == mis->_mix && cursmy == mis->_miy) {
				trigflag = TRUE;
				ClearPanel();
				strcpy(infostr, "Town Portal");
				sprintf(tempstr, "from %s", plr[mis->_misource]._pName);
				AddPanelString(tempstr, TRUE);
				cursmx = mis->_mix;
				cursmy = mis->_miy;
			}
		}
	}
}

void CheckRportal()
{
	MissileStruct *mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		if (mis->_mitype == MIS_RPORTAL) {
			if (cursmx == mis->_mix - 1 && cursmy == mis->_miy
			    || cursmx == mis->_mix && cursmy == mis->_miy - 1
			    || cursmx == mis->_mix - 1 && cursmy == mis->_miy - 1
			    || cursmx == mis->_mix - 2 && cursmy == mis->_miy - 1
			    || cursmx == mis->_mix - 2 && cursmy == mis->_miy - 2
			    || cursmx == mis->_mix - 1 && cursmy == mis->_miy - 2
			    || cursmx == mis->_mix && cursmy == mis->_miy) {
				trigflag = TRUE;
				ClearPanel();
				strcpy(infostr, "Portal to");
				if (!setlevel)
					strcpy(tempstr, "The Unholy Altar");
				else
					strcpy(tempstr, "level 15");
				AddPanelString(tempstr, TRUE);
				cursmx = mis->_mix;
				cursmy = mis->_miy;
			}
		}
	}
}

void CheckCursMove()
{
	int i, sx, sy, fx, fy, mx, my, tx, ty, px, py, xx, yy, mi, columns, rows, xo, yo;
	char bv;
	BOOL flipflag, flipx, flipy;

	sx = MouseX;
	sy = MouseY;

	if (PANELS_COVER) {
		if (chrflag || questlog) {
			if (sx >= SCREEN_WIDTH / 2) { /// BUGFIX: (sx >= SCREEN_WIDTH / 2) (fixed)
				sx -= SCREEN_WIDTH / 4;
			} else {
				sx = 0;
			}
		} else if (invflag || sbookflag) {
			if (sx <= SCREEN_WIDTH / 2) {
				sx += SCREEN_WIDTH / 4;
			} else {
				sx = 0;
			}
		}
	}
	if (sy > PANEL_TOP - 1 && MouseX >= PANEL_LEFT && MouseX < PANEL_LEFT + PANEL_WIDTH && track_isscrolling()) {
		sy = PANEL_TOP - 1;
	}

	if (!zoomflag) {
		sx >>= 1;
		sy >>= 1;
	}

	// Adjust by player offset and tile grid alignment
	CalcTileOffset(&xo, &yo);
	sx -= ScrollInfo._sxoff - xo;
	sy -= ScrollInfo._syoff - yo;

	// Predict the next frame when walking to avoid input jitter
	if (ScrollInfo._sdir != SDIR_NONE) {
		fx = plr[myplr]._pVar6 / 256;
		fy = plr[myplr]._pVar7 / 256;
		fx -= (plr[myplr]._pVar6 + plr[myplr]._pxvel) / 256;
		fy -= (plr[myplr]._pVar7 + plr[myplr]._pyvel) / 256;
		sx -= fx;
		sy -= fy;
	}

	// Convert to tile grid
	mx = ViewX;
	my = ViewY;

	TilesInView(&columns, &rows);
	int lrow = rows - RowsCoveredByPanel();

	// Center player tile on screen
	ShiftGrid(&mx, &my, -columns / 2, -lrow / 2);

	// Align grid
	if ((columns & 1) == 0 && (lrow & 1) == 0) {
		sy += TILE_HEIGHT / 2;
	} else if (columns & 1 && lrow & 1) {
		sx -= TILE_WIDTH / 2;
	} else if (columns & 1 && (lrow & 1) == 0) {
		my++;
	}

	if (!zoomflag) {
		sy -= TILE_HEIGHT / 4;
	}

	tx = sx / TILE_WIDTH;
	ty = sy / TILE_HEIGHT;
	ShiftGrid(&mx, &my, tx, ty);

	// Shift position to match diamond grid aligment
	px = sx % TILE_WIDTH;
	py = sy % TILE_HEIGHT;

	// Shift position to match diamond grid aligment
	flipy = py < (px >> 1);
	if (flipy) {
		my--;
	}
	flipx = py >= TILE_HEIGHT - (px >> 1);
	if (flipx) {
		mx++;
	}

	if (mx < 0) {
		mx = 0;
	}
	if (mx >= MAXDUNX) {
		mx = MAXDUNX - 1;
	}
	if (my < 0) {
		my = 0;
	}
	if (my >= MAXDUNY) {
		my = MAXDUNY - 1;
	}

	flipflag = flipy && flipx || (flipy || flipx) && px < TILE_WIDTH / 2;

	pcurstemp = pcursmonst;
	pcursmonst = -1;
	pcursobj = -1;
	pcursitem = -1;
	if (pcursinvitem != -1) {
		drawsbarflag = TRUE;
	}
	pcursinvitem = -1;
	pcursplr = -1;
	uitemflag = FALSE;
	panelflag = FALSE;
	trigflag = FALSE;

	if (plr[myplr]._pInvincible) {
		return;
	}
	if (pcurs >= CURSOR_FIRSTITEM || spselflag) {
		cursmx = mx;
		cursmy = my;
		return;
	}
	if (MouseY > PANEL_TOP && MouseX >= PANEL_LEFT && MouseX <= PANEL_LEFT + PANEL_WIDTH) {
		CheckPanelInfo();
		return;
	}
	if (doomflag) {
		return;
	}
	if (invflag && MouseX > RIGHT_PANEL && MouseY <= SPANEL_HEIGHT) {
		pcursinvitem = CheckInvHLight();
		return;
	}
	if (sbookflag && MouseX > RIGHT_PANEL && MouseY <= SPANEL_HEIGHT) {
		return;
	}
	if ((chrflag || questlog) && MouseX < SPANEL_WIDTH && MouseY <= SPANEL_HEIGHT) {
		return;
	}

	if (leveltype != DTYPE_TOWN) {
		if (pcurstemp != -1) {
			if (!flipflag && mx + 2 < MAXDUNX && my + 1 < MAXDUNY) {
				mi = dMonster[mx + 2][my + 1];
				if (mi != 0 && dFlags[mx + 2][my + 1] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
						cursmx = mx + 2; /// BUGFIX: 'mx + 2' (fixed)
						cursmy = my + 1; /// BUGFIX: 'my + 1' (fixed)
						pcursmonst = mi;
					}
				}
			}
			if (flipflag && mx + 1 < MAXDUNX && my + 2 < MAXDUNY) {
				mi = dMonster[mx + 1][my + 2];
				if (mi != 0 && dFlags[mx + 1][my + 2] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
						cursmx = mx + 1;
						cursmy = my + 2;
						pcursmonst = mi;
					}
				}
			}
			if (mx + 2 < MAXDUNX && my + 2 < MAXDUNY) {
				mi = dMonster[mx + 2][my + 2];
				if (mi != 0 && dFlags[mx + 2][my + 2] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
						cursmx = mx + 2;
						cursmy = my + 2;
						pcursmonst = mi;
					}
				}
			}
			if (mx + 1 < MAXDUNX && !flipflag) {
				mi = dMonster[mx + 1][my];
				if (mi != 0 && dFlags[mx + 1][my] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
						cursmx = mx + 1;
						cursmy = my;
						pcursmonst = mi;
					}
				}
			}
			if (my + 1 < MAXDUNY && flipflag) {
				mi = dMonster[mx][my + 1];
				if (mi != 0 && dFlags[mx][my + 1] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
						cursmx = mx;
						cursmy = my + 1;
						pcursmonst = mi;
					}
				}
			}
			mi = dMonster[mx][my];
			if (mi != 0 && dFlags[mx][my] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 1) {
					cursmx = mx;
					cursmy = my;
					pcursmonst = mi;
				}
			}
			if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY) {
				mi = dMonster[mx + 1][my + 1];
				if (mi != 0 && dFlags[mx + 1][my + 1] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
						cursmx = mx + 1;
						cursmy = my + 1;
						pcursmonst = mi;
					}
				}
			}
			if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_HIDDEN) {
				pcursmonst = -1;
				cursmx = mx;
				cursmy = my;
			}
#ifdef HELLFIRE
			if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_GOLEM && !(monster[pcursmonst]._mFlags & MFLAG_UNUSED)) {
				pcursmonst = -1;
			}
#else
			if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_GOLEM) {
				pcursmonst = -1;
			}
#endif
			if (pcursmonst != -1) {
				return;
			}
		}
		if (!flipflag && mx + 2 < MAXDUNX && my + 1 < MAXDUNY) {
			mi = dMonster[mx + 2][my + 1];
			if (mi != 0 && dFlags[mx + 2][my + 1] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
					cursmx = mx + 2;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
		}
		if (flipflag && mx + 1 < MAXDUNX && my + 2 < MAXDUNY) {
			mi = dMonster[mx + 1][my + 2];
			if (mi != 0 && dFlags[mx + 1][my + 2] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
					cursmx = mx + 1;
					cursmy = my + 2;
					pcursmonst = mi;
				}
			}
		}
		if (mx + 2 < MAXDUNX && my + 2 < MAXDUNY) {
			mi = dMonster[mx + 2][my + 2];
			if (mi != 0 && dFlags[mx + 2][my + 2] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
					cursmx = mx + 2;
					cursmy = my + 2;
					pcursmonst = mi;
				}
			}
		}
		if (!flipflag && mx + 1 < MAXDUNX) {
			mi = dMonster[mx + 1][my];
			if (mi != 0 && dFlags[mx + 1][my] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
					cursmx = mx + 1;
					cursmy = my;
					pcursmonst = mi;
				}
			}
		}
		if (flipflag && my + 1 < MAXDUNY) {
			mi = dMonster[mx][my + 1];
			if (mi != 0 && dFlags[mx][my + 1] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
					cursmx = mx;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
		}
		mi = dMonster[mx][my];
		if (mi != 0 && dFlags[mx][my] & BFLAG_LIT) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 1) {
				cursmx = mx;
				cursmy = my;
				pcursmonst = mi;
			}
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY) {
			mi = dMonster[mx + 1][my + 1];
			if (mi != 0 && dFlags[mx + 1][my + 1] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
					cursmx = mx + 1;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
		}
		if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_HIDDEN) {
			pcursmonst = -1;
			cursmx = mx;
			cursmy = my;
		}
#ifdef HELLFIRE
		if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_GOLEM && !(monster[pcursmonst]._mFlags & MFLAG_UNUSED)) {
			pcursmonst = -1;
		}
#else
		if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_GOLEM) {
			pcursmonst = -1;
		}
#endif
	} else {
		if (!flipflag && mx + 1 < MAXDUNX) {
			mi = dMonster[mx + 1][my];
			if (mi > 0) {
				pcursmonst = mi - 1;
				cursmx = mx + 1;
				cursmy = my;
			}
		}
		if (flipflag && my + 1 < MAXDUNY) {
			mi = dMonster[mx][my + 1];
			if (mi > 0) {
				pcursmonst = mi - 1;
				cursmx = mx;
				cursmy = my + 1;
			}
		}
		mi = dMonster[mx][my];
		if (mi > 0) {
			pcursmonst = mi - 1;
			cursmx = mx;
			cursmy = my;
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY) {
			mi = dMonster[mx + 1][my + 1];
			if (mi > 0) {
				pcursmonst = mi - 1;
				cursmx = mx + 1;
				cursmy = my + 1;
			}
		}
		if (pcursmonst != -1 && !towner[pcursmonst]._tSelFlag) {
			pcursmonst = -1;
		}
	}

	if (pcursmonst == -1) {
		if (!flipflag && mx + 1 < MAXDUNX) {
			bv = dPlayer[mx + 1][my];
			if (bv != 0) {
				bv = bv >= 0 ? bv - 1 : -(bv + 1);
				if (bv != myplr && plr[bv]._pHitPoints != 0) {
					cursmx = mx + 1;
					cursmy = my;
					pcursplr = bv;
				}
			}
		}
		if (flipflag && my + 1 < MAXDUNY) {
			bv = dPlayer[mx][my + 1];
			if (bv != 0) {
				bv = bv >= 0 ? bv - 1 : -(bv + 1);
				if (bv != myplr && plr[bv]._pHitPoints != 0) {
					cursmx = mx;
					cursmy = my + 1;
					pcursplr = bv;
				}
			}
		}
		bv = dPlayer[mx][my];
		if (bv != 0) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (bv != myplr) {
				cursmx = mx;
				cursmy = my;
				pcursplr = bv;
			}
		}
		if (dFlags[mx][my] & BFLAG_DEAD_PLAYER) {
			for (i = 0; i < MAX_PLRS; i++) {
				if (plr[i]._px == mx && plr[i]._py == my && i != myplr) {
					cursmx = mx;
					cursmy = my;
					pcursplr = i;
				}
			}
		}
		if (pcurs == CURSOR_RESURRECT) {
			for (xx = -1; xx < 2; xx++) {
				for (yy = -1; yy < 2; yy++) {
					if (mx + xx < MAXDUNX && my + yy < MAXDUNY && dFlags[mx + xx][my + yy] & BFLAG_DEAD_PLAYER) {
						for (i = 0; i < MAX_PLRS; i++) {
							if (plr[i]._px == mx + xx && plr[i]._py == my + yy && i != myplr) {
								cursmx = mx + xx;
								cursmy = my + yy;
								pcursplr = i;
							}
						}
					}
				}
			}
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY) {
			bv = dPlayer[mx + 1][my + 1];
			if (bv != 0) {
				bv = bv >= 0 ? bv - 1 : -(bv + 1);
				if (bv != myplr && plr[bv]._pHitPoints != 0) {
					cursmx = mx + 1;
					cursmy = my + 1;
					pcursplr = bv;
				}
			}
		}
	}
	if (pcursmonst == -1 && pcursplr == -1) {
		if (!flipflag && mx + 1 < MAXDUNX) {
			bv = dObject[mx + 1][my];
			if (bv != 0) {
				bv = bv >= 0 ? bv - 1 : -(bv + 1);
				if (object[bv]._oSelFlag >= 2) {
					cursmx = mx + 1;
					cursmy = my;
					pcursobj = bv;
				}
			}
		}
		if (flipflag && my + 1 < MAXDUNY) {
			bv = dObject[mx][my + 1];
			if (bv != 0) {
				bv = bv >= 0 ? bv - 1 : -(bv + 1);
				if (object[bv]._oSelFlag >= 2) {
					cursmx = mx;
					cursmy = my + 1;
					pcursobj = bv;
				}
			}
		}
		bv = dObject[mx][my];
		if (bv != 0) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (object[bv]._oSelFlag == 1 || object[bv]._oSelFlag == 3) {
				cursmx = mx;
				cursmy = my;
				pcursobj = bv;
			}
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY) {
			bv = dObject[mx + 1][my + 1];
			if (bv != 0) {
				bv = bv >= 0 ? bv - 1 : -(bv + 1);
				if (object[bv]._oSelFlag >= 2) {
					cursmx = mx + 1;
					cursmy = my + 1;
					pcursobj = bv;
				}
			}
		}
	}
	if (pcursplr == -1 && pcursobj == -1 && pcursmonst == -1) {
		if (!flipflag && mx + 1 < MAXDUNX) {
			bv = dItem[mx + 1][my];
			if (bv > 0) {
				bv--;
				if (item[bv]._iSelFlag >= 2) {
					cursmx = mx + 1;
					cursmy = my;
					pcursitem = bv;
				}
			}
		}
		if (flipflag && my + 1 < MAXDUNY) {
			bv = dItem[mx][my + 1];
			if (bv > 0) {
				bv--;
				if (item[bv]._iSelFlag >= 2) {
					cursmx = mx;
					cursmy = my + 1;
					pcursitem = bv;
				}
			}
		}
		bv = dItem[mx][my];
		if (bv > 0) {
			bv--;
			if (item[bv]._iSelFlag == 1 || item[bv]._iSelFlag == 3) {
				cursmx = mx;
				cursmy = my;
				pcursitem = bv;
			}
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY) {
			bv = dItem[mx + 1][my + 1];
			if (bv > 0) {
				bv--;
				if (item[bv]._iSelFlag >= 2) {
					cursmx = mx + 1;
					cursmy = my + 1;
					pcursitem = bv;
				}
			}
		}
		if (pcursitem == -1) {
			cursmx = mx;
			cursmy = my;
			CheckTrigForce();
			CheckTown();
			CheckRportal();
		}
	}

	if (pcurs == CURSOR_IDENTIFY) {
		pcursobj = -1;
		pcursmonst = -1;
		pcursitem = -1;
		cursmx = mx;
		cursmy = my;
	}
#ifdef HELLFIRE
	if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_GOLEM && !(monster[pcursmonst]._mFlags & MFLAG_UNUSED)) {
		pcursmonst = -1;
	}
#else
	if (pcursmonst != -1 && monster[pcursmonst]._mFlags & MFLAG_GOLEM) {
		pcursmonst = -1;
	}
#endif
}

DEVILUTION_END_NAMESPACE
