/**
 * @file inv.cpp
 *
 * Implementation of player inventory.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BOOL invflag;
BYTE *pInvCels;
BOOL drawsbarflag;
int sgdwLastTime; // check name

/**
 * Maps from inventory slot to screen position. The inventory slots are
 * arranged as follows:
 *                          00 01
 *                          02 03   06
 *              07 08       19 20       13 14
 *              09 10       21 22       15 16
 *              11 12       23 24       17 18
 *                 04                   05
 *              25 26 27 28 29 30 31 32 33 34
 *              35 36 37 38 39 40 41 42 43 44
 *              45 46 47 48 49 50 51 52 53 54
 *              55 56 57 58 59 60 61 62 63 64
 * 65 66 67 68 69 70 71 72
 * @see graphics/inv/inventory.png
 */
const InvXY InvRect[] = {
	// clang-format off
	//  X,   Y
	{ 132,  31 }, // helmet
	{ 160,  31 }, // helmet
	{ 132,  59 }, // helmet
	{ 160,  59 }, // helmet
	{  45, 205 }, // left ring
	{ 247, 205 }, // right ring
	{ 204,  59 }, // amulet
	{  17, 104 }, // left hand
	{  46, 104 }, // left hand
	{  17, 132 }, // left hand
	{  46, 132 }, // left hand
	{  17, 160 }, // left hand
	{  46, 160 }, // left hand
	{ 247, 104 }, // right hand
	{ 276, 104 }, // right hand
	{ 247, 132 }, // right hand
	{ 276, 132 }, // right hand
	{ 247, 160 }, // right hand
	{ 276, 160 }, // right hand
	{ 132, 104 }, // chest
	{ 160, 104 }, // chest
	{ 132, 132 }, // chest
	{ 160, 132 }, // chest
	{ 132, 160 }, // chest
	{ 160, 160 }, // chest
	{  17, 250 }, // inv row 1
	{  46, 250 }, // inv row 1
	{  75, 250 }, // inv row 1
	{ 104, 250 }, // inv row 1
	{ 133, 250 }, // inv row 1
	{ 162, 250 }, // inv row 1
	{ 191, 250 }, // inv row 1
	{ 220, 250 }, // inv row 1
	{ 249, 250 }, // inv row 1
	{ 278, 250 }, // inv row 1
	{  17, 279 }, // inv row 2
	{  46, 279 }, // inv row 2
	{  75, 279 }, // inv row 2
	{ 104, 279 }, // inv row 2
	{ 133, 279 }, // inv row 2
	{ 162, 279 }, // inv row 2
	{ 191, 279 }, // inv row 2
	{ 220, 279 }, // inv row 2
	{ 249, 279 }, // inv row 2
	{ 278, 279 }, // inv row 2
	{  17, 308 }, // inv row 3
	{  46, 308 }, // inv row 3
	{  75, 308 }, // inv row 3
	{ 104, 308 }, // inv row 3
	{ 133, 308 }, // inv row 3
	{ 162, 308 }, // inv row 3
	{ 191, 308 }, // inv row 3
	{ 220, 308 }, // inv row 3
	{ 249, 308 }, // inv row 3
	{ 278, 308 }, // inv row 3
	{  17, 337 }, // inv row 4
	{  46, 337 }, // inv row 4
	{  75, 337 }, // inv row 4
	{ 104, 337 }, // inv row 4
	{ 133, 337 }, // inv row 4
	{ 162, 337 }, // inv row 4
	{ 191, 337 }, // inv row 4
	{ 220, 337 }, // inv row 4
	{ 249, 337 }, // inv row 4
	{ 278, 337 }, // inv row 4
	{ 205,  33 }, // belt
	{ 234,  33 }, // belt
	{ 263,  33 }, // belt
	{ 292,  33 }, // belt
	{ 321,  33 }, // belt
	{ 350,  33 }, // belt
	{ 379,  33 }, // belt
	{ 408,  33 }  // belt
	// clang-format on
};

/* data */
/** Specifies the starting inventory slots for placement of 2x2 items. */
int AP2x2Tbl[10] = { 8, 28, 6, 26, 4, 24, 2, 22, 0, 20 };

void FreeInvGFX()
{
	MemFreeDbg(pInvCels);
}

void InitInv()
{
	if (plr[myplr]._pClass == PC_WARRIOR) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv.CEL", NULL);
	} else if (plr[myplr]._pClass == PC_ROGUE) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv_rog.CEL", NULL);
	} else if (plr[myplr]._pClass == PC_SORCERER) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv_Sor.CEL", NULL);
#ifdef HELLFIRE
	} else if (plr[myplr]._pClass == PC_MONK) {
#ifdef SPAWN
		pInvCels = LoadFileInMem("Data\\Inv\\Inv.CEL", NULL);
#else
		pInvCels = LoadFileInMem("Data\\Inv\\Inv_Sor.CEL", NULL);
#endif
	} else if (plr[myplr]._pClass == PC_BARD) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv_rog.CEL", NULL);
	} else if (plr[myplr]._pClass == PC_BARBARIAN) {
		pInvCels = LoadFileInMem("Data\\Inv\\Inv.CEL", NULL);
#endif
	}

	invflag = FALSE;
	drawsbarflag = FALSE;
}

void InvDrawSlotBack(int X, int Y, int W, int H)
{
	BYTE *dst;

	assert(gpBuffer);

	dst = &gpBuffer[X + BUFFER_WIDTH * Y];

	int wdt, hgt;
	BYTE pix;

	for (hgt = H; hgt; hgt--, dst -= BUFFER_WIDTH + W) {
		for (wdt = W; wdt; wdt--) {
			pix = *dst;
			if (pix >= PAL16_BLUE) {
				if (pix <= PAL16_BLUE + 15)
					pix -= PAL16_BLUE - PAL16_BEIGE;
				else if (pix >= PAL16_GRAY)
					pix -= PAL16_GRAY - PAL16_BEIGE;
			}
			*dst++ = pix;
		}
	}
}

int InvItemColor(ItemStruct *is)
{
	if (!is->_iStatFlag)
		return ICOL_RED;
	if (is->_iMagical != ITEM_QUALITY_NORMAL)
		return ICOL_BLUE;
	return ICOL_WHITE;
}

/**
 * @brief Render the inventory panel to the back buffer
 */
void DrawInv()
{
	PlayerStruct *p;
	ItemStruct *is;
	int frame, frame_width, screen_x, screen_y, i, j, ii;
	BYTE *pBuff, *cCels;

	CelDraw(RIGHT_PANEL_X, 351 + SCREEN_Y, pInvCels, 1, SPANEL_WIDTH);

	cCels = pCursCels;

	p = &plr[myplr];
	is = &p->InvBody[INVLOC_HEAD];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + 133;
		screen_y = 59 + SCREEN_Y;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif
		if (pcursinvitem == INVITEM_HEAD) {
			CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}
	}

	is = &p->InvBody[INVLOC_RING_LEFT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + 48;
		screen_y = 205 + SCREEN_Y;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif

		if (pcursinvitem == INVITEM_RING_LEFT) {
			CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}
	}

	is = &p->InvBody[INVLOC_RING_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + 249;
		screen_y = 205 + SCREEN_Y;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif

		if (pcursinvitem == INVITEM_RING_RIGHT) {
			CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}
	}

	is = &p->InvBody[INVLOC_AMULET];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + 205;
		screen_y = 60 + SCREEN_Y;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif

		if (pcursinvitem == INVITEM_AMULET) {
			CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}
	}

	is = &p->InvBody[INVLOC_HAND_LEFT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + 17;
		screen_y = 160 + SCREEN_Y;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		if (frame_width == INV_SLOT_SIZE_PX)
			screen_x += 14;
		if (InvItemHeight[frame] != (3 * INV_SLOT_SIZE_PX))
			screen_y -= 14;

#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif

		if (pcursinvitem == INVITEM_HAND_LEFT) {
			CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}

		if (is->_iLoc == ILOC_TWOHAND) {
#ifdef HELLFIRE
			if (p->_pClass != PC_BARBARIAN
			    || is->_itype != ITYPE_SWORD
			        && is->_itype != ITYPE_MACE) {
#endif
				InvDrawSlotBack(RIGHT_PANEL_X + 248, 160 + SCREEN_Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);
				light_table_index = 0;
				cel_transparency_active = TRUE;

				pBuff = frame_width == INV_SLOT_SIZE_PX
					? &gpBuffer[SCREENXY(RIGHT_PANEL_X + 197, SCREEN_Y)]
					: &gpBuffer[SCREENXY(RIGHT_PANEL_X + 183, SCREEN_Y)];

				CelClippedBlitLightTrans(pBuff, cCels, frame, frame_width);

				cel_transparency_active = FALSE;
#ifdef HELLFIRE
			}
#endif
		}
	}

	is = &p->InvBody[INVLOC_HAND_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + 248;
		screen_y = 160 + SCREEN_Y;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		if (frame_width == INV_SLOT_SIZE_PX)
			screen_x += 13;
		else
			screen_x++;
		if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
			screen_y -= 14;

#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif

		if (pcursinvitem == INVITEM_HAND_RIGHT) {
			CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}
	}

	is = &p->InvBody[INVLOC_CHEST];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + 133;
		screen_y = 160 + SCREEN_Y;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif

		if (pcursinvitem == INVITEM_CHEST) {
			CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (p->InvGrid[i] != 0) {
			InvDrawSlotBack(
			    InvRect[i + SLOTXY_INV_FIRST].X + RIGHT_PANEL_X,
			    InvRect[i + SLOTXY_INV_FIRST].Y + SCREEN_Y - 1,
			    INV_SLOT_SIZE_PX,
			    INV_SLOT_SIZE_PX);
		}
	}

	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (p->InvGrid[j] > 0) { // first slot of an item
			ii = p->InvGrid[j] - 1;
			is = &p->InvList[ii];

			screen_x = InvRect[j + SLOTXY_INV_FIRST].X + RIGHT_PANEL_X;
			screen_y = InvRect[j + SLOTXY_INV_FIRST].Y + SCREEN_Y - 1;

			frame = is->_iCurs + CURSOR_FIRSTITEM;
			frame_width = InvItemWidth[frame];
#ifdef HELLFIRE
			if (frame > 179) {
				frame -= 179;
				cCels = pCursCels2;
			} else
				cCels = pCursCels;
#endif
			if (pcursinvitem == ii + INVITEM_INV_FIRST) {
				CelBlitOutline(
				    InvItemColor(is),
				    screen_x,
				    screen_y,
				    cCels, frame, frame_width);
			}

			if (is->_iStatFlag) {
				CelClippedDraw(
				    screen_x,
				    screen_y,
				    cCels, frame, frame_width);
			} else {
				CelDrawLightRed(
				    screen_x,
				    screen_y,
				    cCels, frame, frame_width, 1);
			}
		}
	}
}

void DrawInvBelt()
{
	ItemStruct *is;
	int i, frame, frame_width, screen_x, screen_y;
	BYTE fi, ff;
	BYTE *cCels;

	if (talkflag)
		return;

	DrawPanelBox(205, 21, 232, 28, PANEL_X + 205, PANEL_Y + 5);

	cCels = pCursCels;

	is = plr[myplr].SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, is++) {
		if (is->_itype == ITYPE_NONE) {
			continue;
		}
		screen_x = InvRect[i + SLOTXY_BELT_FIRST].X + PANEL_X;
		screen_y = InvRect[i + SLOTXY_BELT_FIRST].Y + PANEL_Y - 1;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);
		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
#ifdef HELLFIRE
		if (frame > 179) {
			frame -= 179;
			cCels = pCursCels2;
		} else
			cCels = pCursCels;
#endif
		if (pcursinvitem == i + INVITEM_BELT_FIRST) {
			if (!sgbControllerActive || invflag) {
				CelBlitOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
			}
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width, 1);
		}

		if (AllItemsList[is->IDidx].iUsable
		    && is->_iStatFlag
		    && is->_itype != ITYPE_GOLD) {
			fi = i + 49;
			ff = fontframe[gbFontTransTbl[fi]];
			PrintChar(screen_x + INV_SLOT_SIZE_PX - fontkern[ff], screen_y, ff, 0);
		}
	}
}

BOOL AutoPlace(int pnum, int ii, int sx, int sy, BOOL saveflag)
{
	PlayerStruct *p;
	int i, j, xx, yy;
	BOOL done;

	p = &plr[pnum];
	done = TRUE;
	yy = 10 * (ii / 10);
	if (yy < 0) {
		yy = 0;
	}
	for (j = 0; j < sy && done; j++) {
		if (yy >= NUM_INV_GRID_ELEM) {
			done = FALSE;
		}
		xx = ii % 10;
		if (xx < 0) {
			xx = 0;
		}
		for (i = 0; i < sx && done; i++) {
			if (xx >= 10) {
				done = FALSE;
			} else {
				done = p->InvGrid[xx + yy] == 0;
			}
			xx++;
		}
		yy += 10;
	}
	if (done && saveflag) {
		p->InvList[p->_pNumInv] = p->HoldItem;
		p->_pNumInv++;
		yy = 10 * (ii / 10);
		if (yy < 0) {
			yy = 0;
		}
		for (j = 0; j < sy; j++) {
			xx = ii % 10;
			if (xx < 0) {
				xx = 0;
			}
			for (i = 0; i < sx; i++) {
				if (i != 0 || j != sy - 1) {
					p->InvGrid[xx + yy] = -p->_pNumInv;
				} else {
					p->InvGrid[xx + yy] = p->_pNumInv;
				}
				xx++;
			}
			yy += 10;
		}
		CalcPlrScrolls(pnum);
	}
	return done;
}

BOOL SpecialAutoPlace(int pnum, int ii, int sx, int sy, BOOL saveflag)
{
	int i, j, xx, yy;
	BOOL done;

	done = TRUE;
	yy = 10 * (ii / 10);
	if (yy < 0) {
		yy = 0;
	}
	for (j = 0; j < sy && done; j++) {
		if (yy >= NUM_INV_GRID_ELEM) {
			done = FALSE;
		}
		xx = ii % 10;
		if (xx < 0) {
			xx = 0;
		}
		for (i = 0; i < sx && done; i++) {
			if (xx >= 10) {
				done = FALSE;
			} else {
				done = plr[pnum].InvGrid[xx + yy] == 0;
			}
			xx++;
		}
		yy += 10;
	}
	if (!done) {
		if (sx > 1 || sy > 1) {
			done = FALSE;
		} else {
			for (i = 0; i < MAXBELTITEMS; i++) {
				if (plr[pnum].SpdList[i]._itype == ITYPE_NONE) {
					done = TRUE;
					break;
				}
			}
		}
	}
	if (done && saveflag) {
		plr[pnum].InvList[plr[pnum]._pNumInv] = plr[pnum].HoldItem;
		plr[pnum]._pNumInv++;
		yy = 10 * (ii / 10);
		if (yy < 0) {
			yy = 0;
		}
		for (j = 0; j < sy; j++) {
			xx = ii % 10;
			if (xx < 0) {
				xx = 0;
			}
			for (i = 0; i < sx; i++) {
				if (i != 0 || j != sy - 1) {
					plr[pnum].InvGrid[xx + yy] = -plr[pnum]._pNumInv;
				} else {
					plr[pnum].InvGrid[xx + yy] = plr[pnum]._pNumInv;
				}
				xx++;
			}
			yy += 10;
		}
		CalcPlrScrolls(pnum);
	}
	return done;
}

#ifndef HELLFIRE
BOOL GoldAutoPlace(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi;
	BOOL done;
	int i, ii;
	int xx, yy;

	p = &plr[pnum];
	pi = p->InvList;
	done = FALSE;
	for (i = p->_pNumInv; i > 0 && !done; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			if (p->HoldItem._ivalue + pi->_ivalue <= GOLD_MAX_LIMIT) {
				SetGoldItemValue(pi, p->HoldItem._ivalue + pi->_ivalue);
				p->_pGold = CalculateGold(pnum);
				done = TRUE;
			}
		}
	}
	if (done)
		return done;
	pi = p->InvList;
	for (i = p->_pNumInv; i > 0 && !done; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD && pi->_ivalue < GOLD_MAX_LIMIT) {
			if (p->HoldItem._ivalue + pi->_ivalue <= GOLD_MAX_LIMIT) {
				SetGoldItemValue(pi, p->HoldItem._ivalue + pi->_ivalue);
				p->_pGold = CalculateGold(pnum);
				done = TRUE;
			}
		}
	}
	if (done)
		return done;

	for (i = 39; i >= 0 && !done; i--) {
		yy = 10 * (i / 10);
		xx = i % 10;
		if (p->InvGrid[xx + yy] == 0) {
			ii = p->_pNumInv;
			p->InvList[ii] = p->HoldItem;
			p->_pNumInv = p->_pNumInv + 1;
			p->InvGrid[xx + yy] = p->_pNumInv;
			SetGoldItemValue(&p->InvList[ii], p->HoldItem._ivalue);
			p->_pGold = CalculateGold(pnum);
			done = TRUE;
		}
	}

	return done;
}
#else
BOOL GoldAutoPlace(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int ii;
	int xx, yy;
	int max_gold, gold;
	BOOL done;

	p = &plr[pnum];
	pi = p->InvList;
	done = FALSE;
	for (int i = p->_pNumInv; i > 0 && !done; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			gold = pi->_ivalue + p->HoldItem._ivalue;
			if (gold <= MaxGold) {
				SetGoldItemValue(pi, gold);
				p->_pGold = CalculateGold(pnum);
				done = TRUE;
				p->HoldItem._ivalue = 0;
			} else {
				max_gold = MaxGold;
				if (pi->_ivalue < max_gold) {
					gold = max_gold - pi->_ivalue;
					SetGoldItemValue(pi, max_gold);
					SetGoldItemValue(&p->HoldItem, p->HoldItem._ivalue - gold);
					if (p->HoldItem._ivalue < 0) {
						p->HoldItem._ivalue = 0;
						done = TRUE;
					}
					GetPlrHandSeed(&p->HoldItem);
					NewCursor(p->HoldItem._iCurs + CURSOR_FIRSTITEM);
					p->_pGold = CalculateGold(pnum);
				}
			}
		}
	}
	if (!done)
		for (int i = 39; i >= 0 && !done; i--) {
			yy = 10 * (i / 10);
			xx = i % 10;
			if (p->InvGrid[xx + yy] == 0) {
				ii = p->_pNumInv;
				p->InvList[ii] = p->HoldItem;
				p->_pNumInv = p->_pNumInv + 1;
				p->InvGrid[xx + yy] = p->_pNumInv;
				SetGoldItemValue(&p->InvList[ii], p->HoldItem._ivalue);

				gold = p->HoldItem._ivalue;
				if (gold > MaxGold) {
					gold -= MaxGold;
					p->HoldItem._ivalue = gold;
					GetPlrHandSeed(&p->HoldItem);
					p->InvList[ii]._ivalue = MaxGold;
				} else {
					p->HoldItem._ivalue = 0;
					done = TRUE;
					p->_pGold = CalculateGold(pnum);
					NewCursor(CURSOR_HAND);
				}
			}
		}
	return done;
}
#endif

BOOL WeaponAutoPlace(int pnum)
{
#ifdef HELLFIRE
	if (plr[pnum]._pClass == PC_MONK)
		return FALSE;
#endif
	if (plr[pnum].HoldItem._iLoc != ILOC_TWOHAND
#ifdef HELLFIRE
	    || (plr[pnum]._pClass == PC_BARBARIAN && (plr[pnum].HoldItem._itype == ITYPE_SWORD || plr[pnum].HoldItem._itype == ITYPE_MACE))
#endif
	) {
#ifdef HELLFIRE
		if (plr[pnum]._pClass != PC_BARD)
#endif
		{
			if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iClass == ICLASS_WEAPON)
				return FALSE;
			if (plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_RIGHT]._iClass == ICLASS_WEAPON)
				return FALSE;
		}

		if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE) {
			NetSendCmdChItem(TRUE, INVLOC_HAND_LEFT);
			plr[pnum].InvBody[INVLOC_HAND_LEFT] = plr[pnum].HoldItem;
			return TRUE;
		}
		if (plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iLoc != ILOC_TWOHAND) {
			NetSendCmdChItem(TRUE, INVLOC_HAND_RIGHT);
			plr[pnum].InvBody[INVLOC_HAND_RIGHT] = plr[pnum].HoldItem;
			return TRUE;
		}
	} else if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE && plr[pnum].InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE) {
		NetSendCmdChItem(TRUE, INVLOC_HAND_LEFT);
		plr[pnum].InvBody[INVLOC_HAND_LEFT] = plr[pnum].HoldItem;
		return TRUE;
	}

	return FALSE;
}

int SwapItem(ItemStruct *a, ItemStruct *b)
{
	ItemStruct h;

	h = *a;
	*a = *b;
	*b = h;

	return h._iCurs + CURSOR_FIRSTITEM;
}

void CheckInvPaste(int pnum, int mx, int my)
{
	PlayerStruct *p;
	ItemStruct *holditem, *is, *wRight;
	int r, sx, sy;
	int i, j, xx, yy, ii;
	BOOL done, done2h;
	int il, cn, it, iv, ig, gt;
	ItemStruct tempitem;

	p = &plr[pnum];
	holditem = &p->HoldItem;
	SetICursor(holditem->_iCurs + CURSOR_FIRSTITEM);
	i = mx + (icursW >> 1);
	j = my + (icursH >> 1);
	sx = icursW28;
	sy = icursH28;
	for (r = 0; (DWORD)r < NUM_XY_SLOTS; r++) {
		int xo = RIGHT_PANEL;
		int yo = 0;
		if (r >= SLOTXY_BELT_FIRST) {
			xo = PANEL_LEFT;
			yo = PANEL_TOP;
		}

		if (i >= InvRect[r].X + xo && i < InvRect[r].X + xo + INV_SLOT_SIZE_PX) {
			if (j >= InvRect[r].Y + yo - INV_SLOT_SIZE_PX - 1 && j < InvRect[r].Y + yo) {
				break;
			}
		}
		if (r == SLOTXY_CHEST_LAST) {
			if ((sx & 1) == 0)
				i -= INV_SLOT_SIZE_PX / 2;
			if ((sy & 1) == 0) {
				j -= INV_SLOT_SIZE_PX / 2;
			}
		}
		if (r == SLOTXY_INV_LAST && (sy & 1) == 0)
			j += INV_SLOT_SIZE_PX / 2;
	}
	if (r == NUM_XY_SLOTS)
		return;
	il = ILOC_UNEQUIPABLE;
	if (r >= SLOTXY_HEAD_FIRST && r <= SLOTXY_HEAD_LAST)
		il = ILOC_HELM;
	if (r >= SLOTXY_RING_LEFT && r <= SLOTXY_RING_RIGHT)
		il = ILOC_RING;
	if (r == SLOTXY_AMULET)
		il = ILOC_AMULET;
	if (r >= SLOTXY_HAND_LEFT_FIRST && r <= SLOTXY_HAND_RIGHT_LAST)
		il = ILOC_ONEHAND;
	if (r >= SLOTXY_CHEST_FIRST && r <= SLOTXY_CHEST_LAST)
		il = ILOC_ARMOR;
	if (r >= SLOTXY_BELT_FIRST && r <= SLOTXY_BELT_LAST)
		il = ILOC_BELT;
	done = FALSE;
	if (holditem->_iLoc == il)
		done = TRUE;
	if (il == ILOC_ONEHAND && holditem->_iLoc == ILOC_TWOHAND) {
		il = ILOC_TWOHAND;
		done = TRUE;
	}
	if (holditem->_iLoc == ILOC_UNEQUIPABLE && il == ILOC_BELT) {
		if (sx == 1 && sy == 1) {
			done = TRUE;
			if (!AllItemsList[holditem->IDidx].iUsable)
				done = FALSE;
			if (!holditem->_iStatFlag)
				done = FALSE;
			if (holditem->_itype == ITYPE_GOLD)
				done = FALSE;
		}
	}

	if (il == ILOC_UNEQUIPABLE) {
		done = TRUE;
		it = 0;
		ii = r - SLOTXY_INV_FIRST;
		if (holditem->_itype == ITYPE_GOLD) {
			yy = 10 * (ii / 10);
			xx = ii % 10;
			iv = p->InvGrid[xx + yy];
			if (iv != 0) {
				if (iv > 0) {
					if (p->InvList[iv - 1]._itype != ITYPE_GOLD) {
						it = iv;
					}
				} else {
					it = -iv;
				}
			}
		} else {
			yy = 10 * ((ii / 10) - ((sy - 1) >> 1));
			if (yy < 0)
				yy = 0;
			for (j = 0; j < sy && done; j++) {
				if (yy >= NUM_INV_GRID_ELEM)
					done = FALSE;
				xx = (ii % 10) - ((sx - 1) >> 1);
				if (xx < 0)
					xx = 0;
				for (i = 0; i < sx && done; i++) {
					if (xx >= 10) {
						done = FALSE;
					} else {
						iv = p->InvGrid[xx + yy];
						if (iv != 0) {
							if (iv < 0)
								iv = -iv;
							if (it != 0) {
								if (it != iv)
									done = FALSE;
							} else
								it = iv;
						}
					}
					xx++;
				}
				yy += 10;
			}
		}
	}

	if (!done)
		return;

	if (il != ILOC_UNEQUIPABLE && il != ILOC_BELT && !holditem->_iStatFlag) {
		if (p->_pClass == PC_WARRIOR)
			PlaySFX(PS_WARR13);
		else if (p->_pClass == PC_ROGUE)
			PlaySFX(PS_ROGUE13);
		else if (p->_pClass == PC_SORCERER)
			PlaySFX(PS_MAGE13);
		return;
	}

	if (pnum == myplr)
		PlaySFX(ItemInvSnds[ItemCAnimTbl[holditem->_iCurs]]);

	cn = CURSOR_HAND;
	switch (il) {
	case ILOC_HELM:
		NetSendCmdChItem(FALSE, INVLOC_HEAD);
		is = &p->InvBody[INVLOC_HEAD];
		if (is->_itype == ITYPE_NONE)
			*is = *holditem;
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_RING:
		if (r == SLOTXY_RING_LEFT) {
			NetSendCmdChItem(FALSE, INVLOC_RING_LEFT);
			is = &p->InvBody[INVLOC_RING_LEFT];
			if (is->_itype == ITYPE_NONE)
				*is = *holditem;
			else
				cn = SwapItem(is, holditem);
		} else {
			NetSendCmdChItem(FALSE, INVLOC_RING_RIGHT);
			is = &p->InvBody[INVLOC_RING_RIGHT];
			if (is->_itype == ITYPE_NONE)
				*is = *holditem;
			else
				cn = SwapItem(is, holditem);
		}
		break;
	case ILOC_AMULET:
		NetSendCmdChItem(FALSE, INVLOC_AMULET);
		is = &p->InvBody[INVLOC_AMULET];
		if (is->_itype == ITYPE_NONE)
			*is = *holditem;
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_ONEHAND:
		is = &p->InvBody[INVLOC_HAND_LEFT];
		wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		if (r <= SLOTXY_HAND_LEFT_LAST) {
			if (is->_itype == ITYPE_NONE) {
				if (wRight->_itype == ITYPE_NONE || wRight->_iClass != holditem->_iClass) {
					NetSendCmdChItem(FALSE, INVLOC_HAND_LEFT);
					*is = *holditem;
				} else {
					NetSendCmdChItem(FALSE, INVLOC_HAND_RIGHT);
					cn = SwapItem(wRight, holditem);
				}
				break;
			}
			if (wRight->_itype == ITYPE_NONE || wRight->_iClass != holditem->_iClass) {
				NetSendCmdChItem(FALSE, INVLOC_HAND_LEFT);
				cn = SwapItem(is, holditem);
				break;
			}

			NetSendCmdChItem(FALSE, INVLOC_HAND_RIGHT);
			cn = SwapItem(wRight, holditem);
			break;
		}
		if (wRight->_itype == ITYPE_NONE) {
			if (is->_itype == ITYPE_NONE || is->_iLoc != ILOC_TWOHAND) {
				if (is->_itype == ITYPE_NONE || is->_iClass != holditem->_iClass) {
					NetSendCmdChItem(FALSE, INVLOC_HAND_RIGHT);
					*wRight = *holditem;
					break;
				}
				NetSendCmdChItem(FALSE, INVLOC_HAND_LEFT);
				cn = SwapItem(is, holditem);
				break;
			}
			NetSendCmdDelItem(FALSE, INVLOC_HAND_LEFT);
			NetSendCmdChItem(FALSE, INVLOC_HAND_RIGHT);
			SwapItem(wRight, is);
			cn = SwapItem(wRight, holditem);
			break;
		}

		if (is->_itype != ITYPE_NONE && is->_iClass == holditem->_iClass) {
			NetSendCmdChItem(FALSE, INVLOC_HAND_LEFT);
			cn = SwapItem(is, holditem);
			break;
		}
		NetSendCmdChItem(FALSE, INVLOC_HAND_RIGHT);
		cn = SwapItem(wRight, holditem);
		break;
	case ILOC_TWOHAND:
		is = &p->InvBody[INVLOC_HAND_LEFT];
		wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		NetSendCmdDelItem(FALSE, INVLOC_HAND_RIGHT);
		if (is->_itype != ITYPE_NONE && wRight->_itype != ITYPE_NONE) {
			tempitem = *holditem;
			if (wRight->_itype == ITYPE_SHIELD)
				*holditem = *wRight;
			else
				*holditem = *is;
			if (pnum == myplr)
				NewCursor(holditem->_iCurs + CURSOR_FIRSTITEM);
			else
				SetICursor(holditem->_iCurs + CURSOR_FIRSTITEM);
			done2h = FALSE;
			for (i = 0; i < NUM_INV_GRID_ELEM && !done2h; i++)
				done2h = AutoPlace(pnum, i, icursW28, icursH28, TRUE);
			*holditem = tempitem;
			if (pnum == myplr)
				NewCursor(holditem->_iCurs + CURSOR_FIRSTITEM);
			else
				SetICursor(holditem->_iCurs + CURSOR_FIRSTITEM);
			if (!done2h)
				return;

			if (wRight->_itype == ITYPE_SHIELD)
				wRight->_itype = ITYPE_NONE;
			else
				is->_itype = ITYPE_NONE;
		}

		if (is->_itype != ITYPE_NONE || wRight->_itype != ITYPE_NONE) {
			NetSendCmdChItem(FALSE, INVLOC_HAND_LEFT);
			if (is->_itype == ITYPE_NONE)
				SwapItem(is, wRight);
			cn = SwapItem(is, holditem);
		} else {
			NetSendCmdChItem(FALSE, INVLOC_HAND_LEFT);
			*is = *holditem;
		}
		if (is->_itype == ITYPE_STAFF && is->_iSpell != 0 && is->_iCharges > 0) {
			p->_pRSpell = is->_iSpell;
			p->_pRSplType = RSPLTYPE_CHARGES;
			force_redraw = 255;
		}
		break;
	case ILOC_ARMOR:
		NetSendCmdChItem(FALSE, INVLOC_CHEST);
		is = &p->InvBody[INVLOC_CHEST];
		if (is->_itype == ITYPE_NONE)
			*is = *holditem;
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_UNEQUIPABLE:
		if (holditem->_itype == ITYPE_GOLD && it == 0) {
			ii = r - SLOTXY_INV_FIRST;
			yy = 10 * (ii / 10);
			xx = ii % 10;
			il = p->InvGrid[yy + xx];
			if (il > 0) {
				il--;
				is = &p->InvList[il];
				gt = is->_ivalue;
				ig = holditem->_ivalue + gt;
				if (ig <= GOLD_MAX_LIMIT) {
					p->_pGold += holditem->_ivalue;
					SetGoldItemValue(is, ig);
				} else {
					ig = GOLD_MAX_LIMIT - gt;
					p->_pGold += ig;
					SetGoldItemValue(holditem, holditem->_ivalue - ig);
					SetGoldItemValue(is, GOLD_MAX_LIMIT);
					// BUGFIX: incorrect values here are leftover from beta
					if (holditem->_ivalue >= GOLD_MEDIUM_LIMIT)
						cn = 18;
					else if (holditem->_ivalue <= GOLD_SMALL_LIMIT)
						cn = 16;
					else
						cn = 17;
				}
			} else {
				il = p->_pNumInv;
				p->InvList[il] = *holditem;
				p->_pNumInv++;
				p->InvGrid[yy + xx] = p->_pNumInv;
				p->_pGold += holditem->_ivalue;
				if (holditem->_ivalue <= GOLD_MAX_LIMIT) {
					SetGoldItemValue(&p->InvList[il], holditem->_ivalue);
				}
			}
		} else {
			if (it == 0) {
				p->InvList[p->_pNumInv] = *holditem;
				p->_pNumInv++;
				it = p->_pNumInv;
			} else {
				il = it - 1;
				if (holditem->_itype == ITYPE_GOLD)
					p->_pGold += holditem->_ivalue;
				cn = SwapItem(&p->InvList[il], holditem);
				if (holditem->_itype == ITYPE_GOLD)
					p->_pGold = CalculateGold(pnum);
				for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
					if (p->InvGrid[i] == it)
						p->InvGrid[i] = 0;
					if (p->InvGrid[i] == -it)
						p->InvGrid[i] = 0;
				}
			}
			ii = r - SLOTXY_INV_FIRST;
			yy = 10 * (ii / 10 - ((sy - 1) >> 1));
			if (yy < 0)
				yy = 0;
			for (j = 0; j < sy; j++) {
				xx = (ii % 10 - ((sx - 1) >> 1));
				if (xx < 0)
					xx = 0;
				for (i = 0; i < sx; i++) {
					if (i != 0 || j != sy - 1)
						p->InvGrid[xx + yy] = -it;
					else
						p->InvGrid[xx + yy] = it;
					xx++;
				}
				yy += 10;
			}
		}
		break;
	case ILOC_BELT:
		ii = r - SLOTXY_BELT_FIRST;
		is = &p->SpdList[ii];
		if (holditem->_itype == ITYPE_GOLD) {
			if (is->_itype != ITYPE_NONE) {
				if (is->_itype == ITYPE_GOLD) {
					i = holditem->_ivalue + is->_ivalue;
					if (i <= GOLD_MAX_LIMIT) {
						SetGoldItemValue(is, i);
						p->_pGold += holditem->_ivalue;
					} else {
						i = GOLD_MAX_LIMIT - is->_ivalue;
						p->_pGold += i;
						holditem->_ivalue -= i;
						SetGoldItemValue(is, GOLD_MAX_LIMIT);

						// BUGFIX: incorrect values here are leftover from beta
						if (holditem->_ivalue >= GOLD_MEDIUM_LIMIT)
							cn = 18;
						else if (holditem->_ivalue <= GOLD_SMALL_LIMIT)
							cn = 16;
						else
							cn = 17;
					}
				} else {
					p->_pGold += holditem->_ivalue;
					cn = SwapItem(is, holditem);
				}
			} else {
				*is = p->HoldItem;
				p->_pGold += holditem->_ivalue;
			}
		} else if (is->_itype == ITYPE_NONE) {
			*is = *holditem;
		} else {
			cn = SwapItem(is, holditem);
			if (holditem->_itype == ITYPE_GOLD)
				p->_pGold = CalculateGold(pnum);
		}
		drawsbarflag = TRUE;
		break;
	}
	CalcPlrInv(pnum, TRUE);
	if (pnum == myplr) {
		if (cn == CURSOR_HAND)
			SetCursorPos(MouseX + (cursW >> 1), MouseY + (cursH >> 1));
		NewCursor(cn);
	}
}

void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, BOOL bId)
{
	PlayerStruct *p;

	RecreateItem(MAXITEMS, idx, wCI, seed, 0);

	p = &plr[pnum];
	p->HoldItem = item[MAXITEMS];

	if (bId) {
		p->HoldItem._iIdentified = TRUE;
	}

	if (bLoc < NUM_INVLOC) {
		p->InvBody[bLoc] = p->HoldItem;

		if (bLoc == INVLOC_HAND_LEFT && p->HoldItem._iLoc == ILOC_TWOHAND) {
			p->InvBody[INVLOC_HAND_RIGHT]._itype = ITYPE_NONE;
		} else if (bLoc == INVLOC_HAND_RIGHT && p->HoldItem._iLoc == ILOC_TWOHAND) {
			p->InvBody[INVLOC_HAND_LEFT]._itype = ITYPE_NONE;
		}
	}

	CalcPlrInv(pnum, TRUE);
}

void CheckInvCut(int pnum, int mx, int my)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int r;
	char ii;
	int iv, i, j, offs, ig;

	p = &plr[pnum];
	if (p->_pmode > PM_WALK3) {
		return;
	}

	if (dropGoldFlag) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	}

	for (r = 0; (DWORD)r < NUM_XY_SLOTS; r++) {
		int xo = RIGHT_PANEL;
		int yo = 0;
		if (r >= SLOTXY_BELT_FIRST) {
			xo = PANEL_LEFT;
			yo = PANEL_TOP;
		}

		// check which inventory rectangle the mouse is in, if any
		if (mx >= InvRect[r].X + xo
		    && mx <= InvRect[r].X + xo + INV_SLOT_SIZE_PX
		    && my >= InvRect[r].Y + yo - (INV_SLOT_SIZE_PX + 1)
		    && my < InvRect[r].Y + yo) {
			break;
		}
	}

	if (r == NUM_XY_SLOTS) {
		// not on an inventory slot rectangle
		return;
	}

	p->HoldItem._itype = ITYPE_NONE;

	if (r >= SLOTXY_HEAD_FIRST && r <= SLOTXY_HEAD_LAST) {
		pi = &p->InvBody[INVLOC_HEAD];
		if (pi->_itype == ITYPE_NONE)
			return;
		NetSendCmdDelItem(FALSE, INVLOC_HEAD);
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
	} else if (r == SLOTXY_RING_LEFT) {
		pi = &p->InvBody[INVLOC_RING_LEFT];
		if (pi->_itype == ITYPE_NONE)
			return;
		NetSendCmdDelItem(FALSE, INVLOC_RING_LEFT);
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
	} else if (r == SLOTXY_RING_RIGHT) {
		pi = &p->InvBody[INVLOC_RING_RIGHT];
		if (pi->_itype == ITYPE_NONE)
			return;
		NetSendCmdDelItem(FALSE, INVLOC_RING_RIGHT);
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
	} else if (r == SLOTXY_AMULET) {
		pi = &p->InvBody[INVLOC_AMULET];
		if (pi->_itype == ITYPE_NONE)
			return;
		NetSendCmdDelItem(FALSE, INVLOC_AMULET);
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
	} else if (r >= SLOTXY_HAND_LEFT_FIRST && r <= SLOTXY_HAND_LEFT_LAST) {
		pi = &p->InvBody[INVLOC_HAND_LEFT];
		if (pi->_itype == ITYPE_NONE)
			return;
		NetSendCmdDelItem(FALSE, INVLOC_HAND_LEFT);
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
	} else if (r >= SLOTXY_HAND_RIGHT_FIRST && r <= SLOTXY_HAND_RIGHT_LAST) {
		pi = &p->InvBody[INVLOC_HAND_RIGHT];
		if (pi->_itype == ITYPE_NONE)
			return;
		NetSendCmdDelItem(FALSE, INVLOC_HAND_RIGHT);
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
	} else if (r >= SLOTXY_CHEST_FIRST && r <= SLOTXY_CHEST_LAST) {
		pi = &p->InvBody[INVLOC_CHEST];
		if (pi->_itype == ITYPE_NONE)
			return;
		NetSendCmdDelItem(FALSE, INVLOC_CHEST);
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
	} else if (r >= SLOTXY_INV_FIRST && r <= SLOTXY_INV_LAST) {
		ig = r - SLOTXY_INV_FIRST;
		ii = p->InvGrid[ig];
		if (ii != 0) {
			iv = ii;
			if (ii <= 0) {
				iv = -ii;
			}

			for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
				if (p->InvGrid[i] == iv || p->InvGrid[i] == -iv) {
					p->InvGrid[i] = 0;
				}
			}

			iv--;

			p->HoldItem = p->InvList[iv];
			p->_pNumInv--;

			if (p->_pNumInv > 0 && p->_pNumInv != iv) {
				p->InvList[iv] = p->InvList[p->_pNumInv];

				for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
					if (p->InvGrid[j] == p->_pNumInv + 1) {
						p->InvGrid[j] = iv + 1;
					}
					if (p->InvGrid[j] == -(p->_pNumInv + 1)) {
						p->InvGrid[j] = -iv - 1;
					}
				}
			}
		}
	} else if (r >= SLOTXY_BELT_FIRST) {
		offs = r - SLOTXY_BELT_FIRST;
		pi = &p->SpdList[offs];
		if (pi->_itype == ITYPE_NONE)
			return;
		p->HoldItem = *pi;
		pi->_itype = ITYPE_NONE;
		drawsbarflag = TRUE;
	}

	if (p->HoldItem._itype != ITYPE_NONE) {
		if (p->HoldItem._itype == ITYPE_GOLD) {
			p->_pGold = CalculateGold(pnum);
		}

		CalcPlrInv(pnum, TRUE);
		ItemStatOk(pnum, &p->HoldItem);

		if (pnum == myplr) {
			PlaySFX(IS_IGRAB);
			NewCursor(p->HoldItem._iCurs + CURSOR_FIRSTITEM);
			SetCursorPos(mx - (cursW >> 1), MouseY - (cursH >> 1));
		}
	}
}

void inv_update_rem_item(int pnum, BYTE iv)
{
	if (iv < NUM_INVLOC) {
		plr[pnum].InvBody[iv]._itype = ITYPE_NONE;
	}

	if (plr[pnum]._pmode != PM_DEATH) {
		CalcPlrInv(pnum, TRUE);
	} else {
		CalcPlrInv(pnum, FALSE);
	}
}

void RemoveInvItem(int pnum, int iv)
{
	PlayerStruct *p;
	int i, j;

	p = &plr[pnum];
	iv++;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (p->InvGrid[i] == iv || p->InvGrid[i] == -iv) {
			p->InvGrid[i] = 0;
		}
	}

	iv--;
	p->_pNumInv--;

	i = p->_pNumInv;
	if (i > 0 && i != iv) {
		p->InvList[iv] = p->InvList[i];

		i++;
		iv++;
		for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
			if (p->InvGrid[j] == i) {
				p->InvGrid[j] = iv;
			} else if (p->InvGrid[j] == -i) {
				p->InvGrid[j] = -iv;
			}
		}
	}

	CalcPlrScrolls(pnum);
}

#ifdef HELLFIRE
/**
 * @brief This destroyes all items except gold
 */
BOOL inv_diablo_to_hellfire(int pnum)
{
	PlayerStruct *p;
	ItemStruct tmp;
	ItemStruct *pi;
	int i, old_item_cnt, new_item_index;

	p = &plr[pnum];
	if (p->_pgfxnum != 0) {
		p->_pgfxnum = 0;
		p->_pGFXLoad = 0;
		SetPlrAnims(pnum);
	}
	pi = p->InvBody;
	for (i = NUM_INVLOC; i != 0; i--, pi++) {
		pi->_itype = ITYPE_NONE;
	}
	old_item_cnt = p->_pNumInv;
	memset(p->InvGrid, 0, sizeof(p->InvGrid));
	p->_pNumInv = 0;
	pi = p->InvList;
	for (i = 0; i < old_item_cnt; i++, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			new_item_index = p->_pNumInv;
			// BUGFIX: new_item_index may be greater or equal to NUM_INV_GRID_ELEM
			tmp = *pi;
			pi->_itype = ITYPE_NONE;
			p->InvList[new_item_index] = tmp;
			p->_pNumInv++;
			p->InvGrid[i] = p->_pNumInv;
		} else {
			item->_itype = ITYPE_NONE;
		}
	}
	pi = p->SpdList;
	for (i = MAXBELTITEMS; i != 0; i--, pi++) {
		pi->_itype = ITYPE_NONE;
	}
	CalcPlrItemVals(pnum, FALSE);
	return FALSE;
}
#endif

void RemoveSpdBarItem(int pnum, int iv)
{
	plr[pnum].SpdList[iv]._itype = ITYPE_NONE;

	CalcPlrScrolls(pnum);

	force_redraw = 255;
}

void CheckInvItem()
{
	if (pcurs >= CURSOR_FIRSTITEM) {
		CheckInvPaste(myplr, MouseX, MouseY);
	} else {
		CheckInvCut(myplr, MouseX, MouseY);
	}
}

/**
 * Check for interactions with belt
 */
void CheckInvScrn()
{
	if (MouseX > 190 + PANEL_LEFT && MouseX < 437 + PANEL_LEFT
	    && MouseY > PANEL_TOP && MouseY < 33 + PANEL_TOP) {
		CheckInvItem();
	}
}

void CheckBookLevel(int pnum)
{
	int slvl;

	if (plr[pnum].HoldItem._iMiscId == IMISC_BOOK) {
		plr[pnum].HoldItem._iMinMag = spelldata[plr[pnum].HoldItem._iSpell].sMinInt;
		slvl = plr[pnum]._pSplLvl[plr[pnum].HoldItem._iSpell];
		while (slvl != 0) {
			plr[pnum].HoldItem._iMinMag += 20 * plr[pnum].HoldItem._iMinMag / 100;
			slvl--;
			if (plr[pnum].HoldItem._iMinMag + 20 * plr[pnum].HoldItem._iMinMag / 100 > 255) {
				plr[pnum].HoldItem._iMinMag = -1;
				slvl = 0;
			}
		}
	}
}

void CheckQuestItem(int pnum)
{
	PlayerStruct *p;
	ItemStruct *is;
	int idx;

	p = &plr[pnum];
	is = &p->HoldItem;
	idx = is->IDidx;
	if (idx == IDI_OPTAMULET)
		quests[Q_BLIND]._qactive = QUEST_DONE;
	else if (idx == IDI_MUSHROOM && quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE && quests[Q_MUSHROOM]._qvar1 == QS_MUSHSPAWNED) {
		sfxdelay = 10;
		if (p->_pClass == PC_WARRIOR) { // BUGFIX: Voice for this quest might be wrong in MP
			sfxdnum = PS_WARR95;
		} else if (p->_pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE95;
		} else if (p->_pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE95;
#ifdef HELLFIRE
		} else if (p->_pClass == PC_MONK) {
			sfxdnum = PS_MONK95;
		} else if (p->_pClass == PC_BARD) {
			sfxdnum = PS_ROGUE95;
		} else if (p->_pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR95;
#endif
		}
		quests[Q_MUSHROOM]._qvar1 = QS_MUSHPICKED;
	} else if (idx == IDI_ANVIL) {
		if (quests[Q_ANVIL]._qactive == QUEST_INIT) {
			quests[Q_ANVIL]._qactive = QUEST_ACTIVE;
			quests[Q_ANVIL]._qvar1 = 1;
		}
		if (quests[Q_ANVIL]._qlog) {
			sfxdelay = 10;
			if (plr[myplr]._pClass == PC_WARRIOR) {
				sfxdnum = PS_WARR89;
			} else if (plr[myplr]._pClass == PC_ROGUE) {
				sfxdnum = PS_ROGUE89;
			} else if (plr[myplr]._pClass == PC_SORCERER) {
				sfxdnum = PS_MAGE89;
#ifdef HELLFIRE
			} else if (plr[myplr]._pClass == PC_MONK) {
				sfxdnum = PS_MONK89;
			} else if (plr[myplr]._pClass == PC_BARD) {
				sfxdnum = PS_ROGUE89;
			} else if (plr[myplr]._pClass == PC_BARBARIAN) {
				sfxdnum = PS_WARR89;
#endif
			}
		}
	} else if (idx == IDI_GLDNELIX) {
		sfxdelay = 30;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR88;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE88;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE88;
#ifdef HELLFIRE
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK88;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE88;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR88;
#endif
		}
	} else if (idx == IDI_ROCK) {
		if (quests[Q_ROCK]._qactive == QUEST_INIT) {
			quests[Q_ROCK]._qactive = QUEST_ACTIVE;
			quests[Q_ROCK]._qvar1 = 1;
		}
		if (quests[Q_ROCK]._qlog) {
			sfxdelay = 10;
			if (plr[myplr]._pClass == PC_WARRIOR) {
				sfxdnum = PS_WARR87;
			} else if (plr[myplr]._pClass == PC_ROGUE) {
				sfxdnum = PS_ROGUE87;
			} else if (plr[myplr]._pClass == PC_SORCERER) {
				sfxdnum = PS_MAGE87;
#ifdef HELLFIRE
			} else if (plr[myplr]._pClass == PC_MONK) {
				sfxdnum = PS_MONK87;
			} else if (plr[myplr]._pClass == PC_BARD) {
				sfxdnum = PS_ROGUE87;
			} else if (plr[myplr]._pClass == PC_BARBARIAN) {
				sfxdnum = PS_WARR87;
#endif
			}
		}
	} else if (idx == IDI_ARMOFVAL) {
		quests[Q_BLOOD]._qactive = QUEST_DONE;
		sfxdelay = 20;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR91;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE91;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE91;
#ifdef HELLFIRE
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK91;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE91;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR91;
#endif
		}
#ifdef HELLFIRE
	} else if (idx == IDI_MAPOFDOOM) {
		quests[Q_GRAVE]._qlog = FALSE;
		quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
		quests[Q_GRAVE]._qvar1 = 1;
		sfxdelay = 10;
		if (plr[myplr]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR79;
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE79;
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE79;
		} else if (plr[myplr]._pClass == PC_MONK) {
			sfxdnum = PS_MONK79;
		} else if (plr[myplr]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE79;
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR79;
		}
	} else if (idx == IDI_NOTE1 || idx == IDI_NOTE2 || idx == IDI_NOTE3) {
		int item_num;
		int nn;
		ItemStruct tmp;
		if ((idx == IDI_NOTE1 || PlrHasItem(pnum, IDI_NOTE1, &nn))
		 && (idx == IDI_NOTE2 || PlrHasItem(pnum, IDI_NOTE2, &nn))
		 && (idx == IDI_NOTE3 || PlrHasItem(pnum, IDI_NOTE3, &nn))) {
			sfxdelay = 10;
			if (plr[myplr]._pClass == PC_WARRIOR) {
				sfxdnum = PS_WARR46;
			} else if (plr[myplr]._pClass == PC_ROGUE) {
				sfxdnum = PS_ROGUE46;
			} else if (plr[myplr]._pClass == PC_SORCERER) {
				sfxdnum = PS_MAGE46;
			} else if (plr[myplr]._pClass == PC_MONK) {
				sfxdnum = PS_MONK46;
			} else if (plr[myplr]._pClass == PC_BARD) {
				sfxdnum = PS_ROGUE46;
			} else if (plr[myplr]._pClass == PC_BARBARIAN) {
				sfxdnum = PS_WARR46;
			}
			if (idx != IDI_NOTE1) {
				PlrHasItem(pnum, IDI_NOTE1, &nn);
				RemoveInvItem(pnum, nn);
			}
			if (idx != IDI_NOTE2) {
				PlrHasItem(pnum, IDI_NOTE2, &nn);
				RemoveInvItem(pnum, nn);
			}
			if (idx != IDI_NOTE3) {
				PlrHasItem(pnum, IDI_NOTE3, &nn);
				RemoveInvItem(pnum, nn);
			}
			item_num = itemactive[0];
			tmp = item[item_num];
			GetItemAttrs(item_num, IDI_FULLNOTE, 16);
			SetupItem(item_num);
			p->HoldItem = item[item_num];
			item[item_num] = tmp;
		}
#endif
	}
}

void InvGetItem(int pnum, int ii)
{
	PlayerStruct *p;
	ItemStruct *is;
	int i;
#ifdef HELLFIRE
	BOOL cursor_updated;
#endif

	if (dropGoldFlag) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	}

	is = &item[ii];
	if (dItem[is->_ix][is->_iy] != 0) {
		p = &plr[pnum];
		if (myplr == pnum && pcurs >= CURSOR_FIRSTITEM)
			NetSendCmdPItem(TRUE, CMD_SYNCPUTITEM, p->_px, p->_py);
#ifdef HELLFIRE
		if (is->_iUid != 0)
#endif
			is->_iCreateInfo &= ~CF_PREGEN;
		p->HoldItem = *is;
		CheckQuestItem(pnum);
		CheckBookLevel(pnum);
		ItemStatOk(pnum, &p->HoldItem);
#ifdef HELLFIRE
		cursor_updated = FALSE;
		if (p->HoldItem._itype == ITYPE_GOLD && GoldAutoPlace(pnum))
			cursor_updated = TRUE;
#endif
		dItem[is->_ix][is->_iy] = 0;
#ifdef HELLFIRE
		if (currlevel == 21 && is->_ix == CornerStone.x && is->_iy == CornerStone.y) {
			CornerStone.item.IDidx = -1;
			CornerStone.item._itype = ITYPE_MISC;
			CornerStone.item._iSelFlag = FALSE;
			CornerStone.item._ix = 0;
			CornerStone.item._iy = 0;
			CornerStone.item._iAnimFlag = FALSE;
			CornerStone.item._iIdentified = FALSE;
			CornerStone.item._iPostDraw = FALSE;
		}
#endif
		i = 0;
		while (i < numitems) {
			if (itemactive[i] == ii) {
				DeleteItem(itemactive[i], i);
				i = 0;
			} else {
				i++;
			}
		}
		pcursitem = -1;
#ifdef HELLFIRE
		if (!cursor_updated)
#endif
			NewCursor(p->HoldItem._iCurs + CURSOR_FIRSTITEM);
	}
}

void AutoGetItem(int pnum, int ii)
{
	PlayerStruct *p;
	ItemStruct *is;
	int i, idx;
	int w, h;
	BOOL done;

	if (pcurs != CURSOR_HAND) {
		return;
	}

	if (dropGoldFlag) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	}

	is = &item[ii];
	if (ii != MAXITEMS) {
		if (dItem[is->_ix][is->_iy] == 0)
			return;
	}

	p = &plr[pnum];
#ifdef HELLFIRE
	if (is->_iUid != 0)
#endif
		is->_iCreateInfo &= ~CF_PREGEN;
	p->HoldItem = *is; /// BUGFIX: overwrites cursor item, allowing for belt dupe bug
	CheckQuestItem(pnum);
	CheckBookLevel(pnum);
	ItemStatOk(pnum, &p->HoldItem);
	SetICursor(p->HoldItem._iCurs + CURSOR_FIRSTITEM);
	if (p->HoldItem._itype == ITYPE_GOLD) {
		done = GoldAutoPlace(pnum);
#ifdef HELLFIRE
		if (!done)
			is->_ivalue = p->HoldItem._ivalue;
#endif
	} else {
		done = FALSE;
		if (((p->_pgfxnum & 0xF) == ANIM_ID_UNARMED || (p->_pgfxnum & 0xF) == ANIM_ID_UNARMED_SHIELD
#ifdef HELLFIRE
		        || p->_pClass == PC_BARD && ((p->_pgfxnum & 0xF) == ANIM_ID_MACE || (p->_pgfxnum & 0xF) == ANIM_ID_SWORD)
#endif
		            )
		    && p->_pmode <= PM_WALK3) {
			if (p->HoldItem._iStatFlag) {
				if (p->HoldItem._iClass == ICLASS_WEAPON) {
					done = WeaponAutoPlace(pnum);
					if (done)
						CalcPlrInv(pnum, TRUE);
				}
			}
		}
		if (!done) {
			w = icursW28;
			h = icursH28;
			if (w == 1 && h == 1) {
				idx = p->HoldItem.IDidx;
				if (p->HoldItem._iStatFlag && AllItemsList[idx].iUsable) {
					for (i = 0; i < MAXBELTITEMS && !done; i++) {
						if (p->SpdList[i]._itype == ITYPE_NONE) {
							p->SpdList[i] = p->HoldItem;
							CalcPlrScrolls(pnum);
							drawsbarflag = TRUE;
							done = TRUE;
						}
					}
				}
				for (i = 30; i <= 39 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 20; i <= 29 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 10; i <= 19 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 0; i <= 9 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 1 && h == 2) {
				for (i = 29; i >= 20 && !done; i--) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 9; i >= 0 && !done; i--) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 19; i >= 10 && !done; i--) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 1 && h == 3) {
				for (i = 0; i < 20 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 2 && h == 2) {
				for (i = 0; i < 10 && !done; i++) {
					done = AutoPlace(pnum, AP2x2Tbl[i], w, h, TRUE);
				}
				for (i = 21; i < 29 && !done; i += 2) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 1; i < 9 && !done; i += 2) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 10; i < 19 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
			if (w == 2 && h == 3) {
				for (i = 0; i < 9 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
				for (i = 10; i < 19 && !done; i++) {
					done = AutoPlace(pnum, i, w, h, TRUE);
				}
			}
		}
	}
	if (done) {
		dItem[is->_ix][is->_iy] = 0;
#ifdef HELLFIRE
		if (currlevel == 21 && is->_ix == CornerStone.x && is->_iy == CornerStone.y) {
			CornerStone.item.IDidx = -1;
			CornerStone.item._itype = ITYPE_MISC;
			CornerStone.item._iSelFlag = FALSE;
			CornerStone.item._ix = 0;
			CornerStone.item._iy = 0;
			CornerStone.item._iAnimFlag = FALSE;
			CornerStone.item._iIdentified = FALSE;
			CornerStone.item._iPostDraw = FALSE;
		}
#endif
		i = 0;
		while (i < numitems) {
			if (itemactive[i] == ii) {
				DeleteItem(itemactive[i], i);
				i = 0;
			} else {
				i++;
			}
		}
	} else {
		if (pnum == myplr) {
			if (p->_pClass == PC_WARRIOR) {
				PlaySFX(random_(0, 3) + PS_WARR14);
			} else if (p->_pClass == PC_ROGUE) {
				PlaySFX(random_(0, 3) + PS_ROGUE14);
			} else if (p->_pClass == PC_SORCERER) {
				PlaySFX(random_(0, 3) + PS_MAGE14);
#ifdef HELLFIRE
			} else if (p->_pClass == PC_MONK) {
				PlaySFX(random_(0, 3) + PS_MONK14);
			} else if (p->_pClass == PC_BARD) {
				PlaySFX(random_(0, 3) + PS_ROGUE14);
			} else if (p->_pClass == PC_BARBARIAN) {
				PlaySFX(random_(0, 3) + PS_WARR14);
#endif
			}
		}
		p->HoldItem = *is;
		RespawnItem(ii, TRUE);
		NetSendCmdPItem(TRUE, CMD_RESPAWNITEM, is->_ix, is->_iy);
		p->HoldItem._itype = ITYPE_NONE;
#ifdef HELLFIRE
		NewCursor(CURSOR_HAND);
#endif
	}
}

int FindGetItem(int idx, WORD ci, int iseed)
{
	int i, ii;

	for (i = 0; i < numitems; i++) {
		ii = itemactive[i];
		if (item[ii].IDidx == idx && item[ii]._iSeed == iseed && item[ii]._iCreateInfo == ci)
			return ii;
	}

	return -1;
}

void SyncGetItem(int x, int y, int idx, WORD ci, int iseed)
{
	ItemStruct *is;
	int i, ii;

	ii = dItem[x][y];
	if (ii != 0) {
		ii--;
		if (item[ii].IDidx == idx
		    && item[ii]._iSeed == iseed
		    && item[ii]._iCreateInfo == ci) {
			FindGetItem(idx, ci, iseed);
		} else {
			ii = FindGetItem(idx, ci, iseed);
		}
	} else {
		ii = FindGetItem(idx, ci, iseed);
	}

	if (ii != -1) {
		is = &item[ii];
		dItem[is->_ix][is->_iy] = 0;
#ifdef HELLFIRE
		if (currlevel == 21 && is->_ix == CornerStone.x && is->_iy == CornerStone.y) {
			CornerStone.item.IDidx = -1;
			CornerStone.item._itype = ITYPE_MISC;
			CornerStone.item._iSelFlag = FALSE;
			CornerStone.item._ix = 0;
			CornerStone.item._iy = 0;
			CornerStone.item._iAnimFlag = FALSE;
			CornerStone.item._iIdentified = FALSE;
			CornerStone.item._iPostDraw = FALSE;
		}
#endif
		i = 0;
		while (i < numitems) {
			if (itemactive[i] == ii) {
				DeleteItem(itemactive[i], i);
				FindGetItem(idx, ci, iseed);
#ifndef HELLFIRE
				/// ASSERT: assert(FindGetItem(idx,ci,iseed) == -1);
				FindGetItem(idx, ci, iseed); /* todo: replace with above */
#endif
				i = 0;
			} else {
				i++;
			}
		}
		/// ASSERT: assert(FindGetItem(idx, ci, iseed) == -1);
		FindGetItem(idx, ci, iseed); /* todo: replace with above */
	}
}

BOOL CanPut(int x, int y)
{
	char oi, oi2;

	if (dItem[x][y] != 0)
		return FALSE;
	if (nSolidTable[dPiece[x][y]])
		return FALSE;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return FALSE;
	}

	oi = dObject[x + 1][y + 1];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSelFlag != 0)
			return FALSE;
	}

	oi = dObject[x + 1][y];
	if (oi > 0) {
		oi2 = dObject[x][y + 1];
		if (oi2 > 0 && object[oi - 1]._oSelFlag != 0 && object[oi2 - 1]._oSelFlag != 0)
			return FALSE;
	}

	if (currlevel == 0 && dMonster[x][y] != 0)
		return FALSE;
	if (currlevel == 0 && dMonster[x + 1][y + 1] != 0)
		return FALSE;

	return TRUE;
}

BOOL TryInvPut()
{
	int px, py, dir;

	if (numitems >= MAXITEMS)
		return FALSE;

	px = plr[myplr]._px;
	py = plr[myplr]._py;
	dir = GetDirection(px, py, cursmx, cursmy);
	if (CanPut(px + offset_x[dir], py + offset_y[dir])) {
		return TRUE;
	}

	dir = (dir - 1) & 7;
	if (CanPut(px + offset_x[dir], py + offset_y[dir])) {
		return TRUE;
	}

	dir = (dir + 2) & 7;
	if (CanPut(px + offset_x[dir], py + offset_y[dir])) {
		return TRUE;
	}

	return CanPut(px, py);
}

void DrawInvMsg(const char *msg)
{
	DWORD dwTicks;

	dwTicks = SDL_GetTicks();
	if (dwTicks - sgdwLastTime >= 5000) {
		sgdwLastTime = dwTicks;
		ErrorPlrMsg(msg);
	}
}

int InvPutItem(int pnum, int x, int y)
{
	BOOL done;
	int dir, ii;
	int i, j, l;
	int xx, yy;
	int xp, yp;

	if (numitems >= MAXITEMS)
		return -1;

	if (FindGetItem(plr[pnum].HoldItem.IDidx, plr[pnum].HoldItem._iCreateInfo, plr[pnum].HoldItem._iSeed) != -1) {
		DrawInvMsg("A duplicate item has been detected.  Destroying duplicate...");
		SyncGetItem(x, y, plr[pnum].HoldItem.IDidx, plr[pnum].HoldItem._iCreateInfo, plr[pnum].HoldItem._iSeed);
	}

	dir = GetDirection(plr[pnum]._px, plr[pnum]._py, x, y);
	xx = x - plr[pnum]._px;
	yy = y - plr[pnum]._py;
	if (abs(xx) > 1 || abs(yy) > 1) {
		x = plr[pnum]._px + offset_x[dir];
		y = plr[pnum]._py + offset_y[dir];
	}
	if (!CanPut(x, y)) {
		dir = (dir - 1) & 7;
		x = plr[pnum]._px + offset_x[dir];
		y = plr[pnum]._py + offset_y[dir];
		if (!CanPut(x, y)) {
			dir = (dir + 2) & 7;
			x = plr[pnum]._px + offset_x[dir];
			y = plr[pnum]._py + offset_y[dir];
			if (!CanPut(x, y)) {
				done = FALSE;
				for (l = 1; l < 50 && !done; l++) {
					for (j = -l; j <= l && !done; j++) {
						yp = j + plr[pnum]._py;
						for (i = -l; i <= l && !done; i++) {
							xp = i + plr[pnum]._px;
							if (CanPut(xp, yp)) {
								done = TRUE;
								x = xp;
								y = yp;
							}
						}
					}
				}
				if (!done)
					return -1;
			}
		}
	}

#ifdef HELLFIRE
	if (currlevel == 0) {
		yp = cursmy;
		xp = cursmx;
		if (plr[pnum].HoldItem._iCurs == ICURS_RUNE_BOMB && xp >= 79 && xp <= 82 && yp >= 61 && yp <= 64) {
			NetSendCmdLocParam2(0, CMD_OPENHIVE, plr[pnum]._px, plr[pnum]._py, xx, yy);
			quests[Q_FARMER]._qactive = QUEST_DONE;
			if (gbMaxPlayers != 1) {
				NetSendCmdQuest(TRUE, Q_FARMER);
				return -1;
			}
			return -1;
		}
		if (plr[pnum].HoldItem.IDidx == IDI_MAPOFDOOM && xp >= 35 && xp <= 38 && yp >= 20 && yp <= 24) {
			NetSendCmd(FALSE, CMD_OPENCRYPT);
			quests[Q_GRAVE]._qactive = QUEST_DONE;
			if (gbMaxPlayers != 1) {
				NetSendCmdQuest(TRUE, Q_GRAVE);
			}
			return -1;
		}
	}
#endif
	ii = itemavail[0];
	dItem[x][y] = ii + 1;
	itemavail[0] = itemavail[MAXITEMS - (numitems + 1)];
	itemactive[numitems] = ii;
	item[ii] = plr[pnum].HoldItem;
	item[ii]._ix = x;
	item[ii]._iy = y;
	RespawnItem(ii, TRUE);
	numitems++;
#ifdef HELLFIRE
	if (currlevel == 21 && x == CornerStone.x && y == CornerStone.y) {
		CornerStone.item = item[ii];
		InitQTextMsg(TEXT_CORNSTN);
		quests[Q_CORNSTN]._qlog = FALSE;
		quests[Q_CORNSTN]._qactive = QUEST_DONE;
	}
#endif
	NewCursor(CURSOR_HAND);
	return ii;
}

int SyncPutItem(int pnum, int x, int y, int idx, WORD icreateinfo, int iseed, BOOL Id, int dur, int mdur, int ch, int mch, int ivalue, DWORD ibuff
#ifdef HELLFIRE
    ,
    int to_hit, int max_dam, int min_str, int min_mag, int min_dex, int ac
#endif
)
{
	BOOL done;
	int dir, ii;
	int i, j, k;
	int xx, yy;
	int px, py;

	if (numitems >= MAXITEMS)
		return -1;

	if (FindGetItem(idx, icreateinfo, iseed) != -1) {
		DrawInvMsg("A duplicate item has been detected from another player.");
		SyncGetItem(x, y, idx, icreateinfo, iseed);
	}

	px = plr[pnum]._px;
	py = plr[pnum]._py;
	dir = GetDirection(px, py, x, y);
	if (abs(x - px) > 1 || abs(y - py) > 1) {
		x = px + offset_x[dir];
		y = py + offset_y[dir];
	}
	if (!CanPut(x, y)) {
		dir = (dir - 1) & 7;
		x = px + offset_x[dir];
		y = py + offset_y[dir];
		if (!CanPut(x, y)) {
			dir = (dir + 2) & 7;
			x = px + offset_x[dir];
			y = py + offset_y[dir];
			if (!CanPut(x, y)) {
				done = FALSE;
				for (k = 1; k < 50 && !done; k++) {
					for (j = -k; j <= k && !done; j++) {
						yy = j + py;
						for (i = -k; i <= k && !done; i++) {
							xx = i + px;
							if (CanPut(xx, yy)) {
								done = TRUE;
								x = xx;
								y = yy;
							}
						}
					}
				}
				if (!done)
					return -1;
			}
		}
	}

	ii = itemavail[0];
	dItem[x][y] = ii + 1;
	itemavail[0] = itemavail[MAXITEMS - (numitems + 1)];
	itemactive[numitems] = ii;

	if (idx == IDI_EAR) {
		RecreateEar(ii, icreateinfo, iseed, Id, dur, mdur, ch, mch, ivalue, ibuff);
	} else {
		RecreateItem(ii, idx, icreateinfo, iseed, ivalue);
		if (Id)
			item[ii]._iIdentified = TRUE;
		item[ii]._iDurability = dur;
		item[ii]._iMaxDur = mdur;
		item[ii]._iCharges = ch;
		item[ii]._iMaxCharges = mch;
#ifdef HELLFIRE
		item[ii]._iPLToHit = to_hit;
		item[ii]._iMaxDam = max_dam;
		item[ii]._iMinStr = min_str;
		item[ii]._iMinMag = min_mag;
		item[ii]._iMinDex = min_dex;
		item[ii]._iAC = ac;
#endif
	}

	item[ii]._ix = x;
	item[ii]._iy = y;
	RespawnItem(ii, TRUE);
	numitems++;
#ifdef HELLFIRE
	if (currlevel == 21 && x == CornerStone.x && y == CornerStone.y) {
		CornerStone.item = item[ii];
		InitQTextMsg(TEXT_CORNSTN);
		quests[Q_CORNSTN]._qlog = 0;
		quests[Q_CORNSTN]._qactive = QUEST_DONE;
	}
#endif
	return ii;
}

char CheckInvHLight()
{
	int mx, my, r, ii, nGold;
	ItemStruct *pi;
	PlayerStruct *p;
	char rv;

	mx = MouseX;
	my = MouseY;
	for (r = 0; (DWORD)r < NUM_XY_SLOTS; r++) {
		int xo = RIGHT_PANEL;
		int yo = 0;
		if (r >= SLOTXY_BELT_FIRST) {
			xo = PANEL_LEFT;
			yo = PANEL_TOP;
		}

		if (mx >= InvRect[r].X + xo
		    && mx < InvRect[r].X + xo + (INV_SLOT_SIZE_PX + 1)
		    && my >= InvRect[r].Y + yo - (INV_SLOT_SIZE_PX + 1)
		    && my < InvRect[r].Y + yo) {
			break;
		}
	}

	if ((DWORD)r >= NUM_XY_SLOTS)
		return -1;

	rv = -1;
	infoclr = COL_WHITE;
	pi = NULL;
	p = &plr[myplr];
	ClearPanel();
	if (r >= SLOTXY_HEAD_FIRST && r <= SLOTXY_HEAD_LAST) {
		rv = INVLOC_HEAD;
		pi = &p->InvBody[rv];
	} else if (r == SLOTXY_RING_LEFT) {
		rv = INVLOC_RING_LEFT;
		pi = &p->InvBody[rv];
	} else if (r == SLOTXY_RING_RIGHT) {
		rv = INVLOC_RING_RIGHT;
		pi = &p->InvBody[rv];
	} else if (r == SLOTXY_AMULET) {
		rv = INVLOC_AMULET;
		pi = &p->InvBody[rv];
	} else if (r >= SLOTXY_HAND_LEFT_FIRST && r <= SLOTXY_HAND_LEFT_LAST) {
		rv = INVLOC_HAND_LEFT;
		pi = &p->InvBody[rv];
	} else if (r >= SLOTXY_HAND_RIGHT_FIRST && r <= SLOTXY_HAND_RIGHT_LAST) {
		pi = &p->InvBody[INVLOC_HAND_LEFT];
#ifdef HELLFIRE
		if (pi->_itype == ITYPE_NONE || pi->_iLoc != ILOC_TWOHAND
		    || (p->_pClass == PC_BARBARIAN && (pi->_itype == ITYPE_SWORD || pi->_itype == ITYPE_MACE))) {
#else
		if (pi->_itype == ITYPE_NONE || pi->_iLoc != ILOC_TWOHAND) {
#endif
			rv = INVLOC_HAND_RIGHT;
			pi = &p->InvBody[rv];
		} else {
			rv = INVLOC_HAND_LEFT;
		}
	} else if (r >= SLOTXY_CHEST_FIRST && r <= SLOTXY_CHEST_LAST) {
		rv = INVLOC_CHEST;
		pi = &p->InvBody[rv];
	} else if (r >= SLOTXY_INV_FIRST && r <= SLOTXY_INV_LAST) {
		r = abs(p->InvGrid[r - SLOTXY_INV_FIRST]);
		if (r == 0)
			return -1;
		ii = r - 1;
		rv = ii + INVITEM_INV_FIRST;
		pi = &p->InvList[ii];
	} else if (r >= SLOTXY_BELT_FIRST) {
		r -= SLOTXY_BELT_FIRST;
		drawsbarflag = TRUE;
		pi = &p->SpdList[r];
		if (pi->_itype == ITYPE_NONE)
			return -1;
		rv = r + INVITEM_BELT_FIRST;
	}

	if (pi->_itype == ITYPE_NONE)
		return -1;

	if (pi->_itype == ITYPE_GOLD) {
		nGold = pi->_ivalue;
		sprintf(infostr, "%i gold %s", nGold, get_pieces_str(nGold));
	} else {
		if (pi->_iMagical == ITEM_QUALITY_MAGIC) {
			infoclr = COL_BLUE;
		} else if (pi->_iMagical == ITEM_QUALITY_UNIQUE) {
			infoclr = COL_GOLD;
		}
		strcpy(infostr, pi->_iName);
		if (pi->_iIdentified) {
			strcpy(infostr, pi->_iIName);
			PrintItemDetails(pi);
		} else {
			PrintItemDur(pi);
		}
	}

	return rv;
}

void RemoveScroll(int pnum)
{
	int i;

	for (i = 0; i < plr[pnum]._pNumInv; i++) {
		if (plr[pnum].InvList[i]._itype != ITYPE_NONE
		    && (plr[pnum].InvList[i]._iMiscId == IMISC_SCROLL || plr[pnum].InvList[i]._iMiscId == IMISC_SCROLLT)
#ifdef HELLFIRE
		    && plr[pnum].InvList[i]._iSpell == plr[pnum]._pSpell) {
#else
		    && plr[pnum].InvList[i]._iSpell == plr[pnum]._pRSpell) {
#endif
			RemoveInvItem(pnum, i);
			CalcPlrScrolls(pnum);
			return;
		}
	}
	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr[pnum].SpdList[i]._itype != ITYPE_NONE
		    && (plr[pnum].SpdList[i]._iMiscId == IMISC_SCROLL || plr[pnum].SpdList[i]._iMiscId == IMISC_SCROLLT)
#ifdef HELLFIRE
			&& plr[pnum].SpdList[i]._iSpell == plr[pnum]._pSpell) {
#else
		    && plr[pnum].SpdList[i]._iSpell == plr[pnum]._pRSpell) {
#endif
			RemoveSpdBarItem(pnum, i);
			CalcPlrScrolls(pnum);
			return;
		}
	}
}

BOOL UseScroll()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	p = &plr[myplr];
	if (pcurs != CURSOR_HAND)
		return FALSE;
	if (leveltype == DTYPE_TOWN && !spelldata[p->_pRSpell].sTownSpell)
		return FALSE;

	pi = p->InvList;
	for (i = p->_pNumInv; i > 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE
		    && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
		    && pi->_iSpell == p->_pRSpell) {
			return TRUE;
		}
	}
	pi = p->SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++) {
		if (pi->_itype != ITYPE_NONE
		    && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
		    && pi->_iSpell == p->_pRSpell) {
			return TRUE;
		}
	}

	return FALSE;
}

void UseStaffCharge(int pnum)
{
	ItemStruct *is;

	is = &plr[pnum].InvBody[INVLOC_HAND_LEFT];
	if (is->_itype != ITYPE_NONE
	    && (is->_iMiscId == IMISC_STAFF
#ifdef HELLFIRE
	           || is->_iMiscId == IMISC_UNIQUE // BUGFIX: myplr->pnum (fixed)
#endif
	           )
	    && is->_iSpell == plr[pnum]._pRSpell
	    && is->_iCharges > 0) {
		is->_iCharges--;
		CalcPlrStaff(pnum);
	}
}

BOOL UseStaff()
{
	if (pcurs == CURSOR_HAND) {
		if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE
#ifdef HELLFIRE
		    && (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iMiscId == IMISC_STAFF || plr[myplr].InvBody[INVLOC_HAND_LEFT]._iMiscId == IMISC_UNIQUE)
#else
		    && plr[myplr].InvBody[INVLOC_HAND_LEFT]._iMiscId == IMISC_STAFF
#endif
		    && plr[myplr].InvBody[INVLOC_HAND_LEFT]._iSpell == plr[myplr]._pRSpell
		    && plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges > 0) {
			return TRUE;
		}
	}

	return FALSE;
}

void StartGoldDrop()
{
	initialDropGoldIndex = pcursinvitem;
	if (pcursinvitem <= INVITEM_INV_LAST)
		initialDropGoldValue = plr[myplr].InvList[pcursinvitem - INVITEM_INV_FIRST]._ivalue;
	else
		initialDropGoldValue = plr[myplr].SpdList[pcursinvitem - INVITEM_BELT_FIRST]._ivalue;
	dropGoldFlag = TRUE;
	dropGoldValue = 0;
	if (talkflag)
		control_reset_talk();
}

BOOL UseInvItem(int pnum, int cii)
{
	int iv;
	ItemStruct *is;
	BOOL speedlist;

	if (plr[pnum]._pInvincible && plr[pnum]._pHitPoints == 0 && pnum == myplr)
		return TRUE;
	if (pcurs != CURSOR_HAND)
		return TRUE;
	if (stextflag != STORE_NONE)
		return TRUE;
	if (cii <= INVITEM_HAND_RIGHT)
		return FALSE;

	if (cii <= INVITEM_INV_LAST) {
		iv = cii - INVITEM_INV_FIRST;
		is = &plr[pnum].InvList[iv];
		speedlist = FALSE;
	} else {
		if (talkflag)
			return TRUE;
		iv = cii - INVITEM_BELT_FIRST;
		is = &plr[pnum].SpdList[iv];
		speedlist = TRUE;
	}

	switch (is->IDidx) {
	case IDI_MUSHROOM:
		sfxdelay = 10;
		if (plr[pnum]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR95;
		} else if (plr[pnum]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE95;
		} else if (plr[pnum]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE95;
		}
#ifdef HELLFIRE
		else if (plr[pnum]._pClass == PC_MONK) {
			sfxdnum = PS_MONK95;
		} else if (plr[pnum]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE95;
		} else if (plr[pnum]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR95;
		}
#endif
		return TRUE;
	case IDI_FUNGALTM:
		PlaySFX(IS_IBOOK);
		sfxdelay = 10;
		if (plr[pnum]._pClass == PC_WARRIOR) {
			sfxdnum = PS_WARR29;
		} else if (plr[pnum]._pClass == PC_ROGUE) {
			sfxdnum = PS_ROGUE29;
		} else if (plr[pnum]._pClass == PC_SORCERER) {
			sfxdnum = PS_MAGE29;
#ifdef HELLFIRE
		} else if (plr[pnum]._pClass == PC_MONK) {
			sfxdnum = PS_MONK29;
		} else if (plr[pnum]._pClass == PC_BARD) {
			sfxdnum = PS_ROGUE29;
		} else if (plr[pnum]._pClass == PC_BARBARIAN) {
			sfxdnum = PS_WARR29;
#endif
		}
		return TRUE;
	}

	if (!AllItemsList[is->IDidx].iUsable)
		return FALSE;

	if (!is->_iStatFlag) {
		if (plr[pnum]._pClass == PC_WARRIOR) {
			PlaySFX(PS_WARR13);
		} else if (plr[pnum]._pClass == PC_ROGUE) {
			PlaySFX(PS_ROGUE13);
		} else if (plr[pnum]._pClass == PC_SORCERER) {
			PlaySFX(PS_MAGE13);
#ifdef HELLFIRE
		} else if (plr[pnum]._pClass == PC_MONK) {
			PlaySFX(PS_MONK13);
		} else if (plr[pnum]._pClass == PC_BARD) {
			PlaySFX(PS_ROGUE13);
		} else if (plr[pnum]._pClass == PC_BARBARIAN) {
			PlaySFX(PS_WARR13);
#endif
		}
		return TRUE;
	}

	if (is->_iMiscId == IMISC_NONE && is->_itype == ITYPE_GOLD) {
		StartGoldDrop();
		return TRUE;
	}

	if (dropGoldFlag) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	}

	if (is->_iMiscId == IMISC_SCROLL && currlevel == 0 && !spelldata[is->_iSpell].sTownSpell) {
		return TRUE;
	}

	if (is->_iMiscId == IMISC_SCROLLT && currlevel == 0 && !spelldata[is->_iSpell].sTownSpell) {
		return TRUE;
	}

#ifdef HELLFIRE
	if (is->_iMiscId > IMISC_RUNEFIRST && is->_iMiscId < IMISC_RUNELAST && currlevel == 0) {
		return TRUE;
	}
#endif

	if (is->_iMiscId == IMISC_BOOK)
		PlaySFX(IS_RBOOK);
	else if (pnum == myplr)
		PlaySFX(ItemInvSnds[ItemCAnimTbl[is->_iCurs]]);

	UseItem(pnum, is->_iMiscId, is->_iSpell);

#ifdef HELLFIRE
	if (is->_iMiscId == IMISC_NOTE) {
		InitQTextMsg(TEXT_BOOK9);
		invflag = FALSE;
		return TRUE;
	}
#endif
	if (speedlist) {
		RemoveSpdBarItem(pnum, iv);
		return TRUE;
	} else if (is->_iMiscId != IMISC_MAPOFDOOM) {
		RemoveInvItem(pnum, iv);
	}
	return TRUE;
}

int CalculateGold(int pnum)
{
	ItemStruct *pi;
	int i, gold;

	gold = 0;
	pi = plr[pnum].SpdList;
	for (i = MAXBELTITEMS; i != 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			gold += pi->_ivalue;
			force_redraw = 255;
		}
	}
	pi = plr[pnum].InvList;
	for (i = plr[pnum]._pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD)
			gold += pi->_ivalue;
	}

	return gold;
}

BOOL DropItemBeforeTrig()
{
	if (TryInvPut()) {
		NetSendCmdPItem(TRUE, CMD_PUTITEM, cursmx, cursmy);
		NewCursor(CURSOR_HAND);
		return TRUE;
	}

	return FALSE;
}

DEVILUTION_END_NAMESPACE
