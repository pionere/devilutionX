/**
 * @file inv.cpp
 *
 * Implementation of player inventory.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

BOOL invflag;
BYTE *pInvCels;
BYTE *pBeltCels;
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
const InvXY InvRect[NUM_XY_SLOTS] = {
	// clang-format off
	//  X,   Y
	{ 121,                    29 },                       // helmet
	{ 121 + INV_SLOT_SIZE_PX, 29 },                       // helmet
	{ 121,                    29 +    INV_SLOT_SIZE_PX }, // helmet
	{ 121 + INV_SLOT_SIZE_PX, 29 +    INV_SLOT_SIZE_PX }, // helmet
	{  61, 171 }, // left ring
	{ 206, 171 }, // right ring
	{ 187,  45 }, // amulet
	{  47,                    82 },                      // left hand
	{  47 + INV_SLOT_SIZE_PX, 82 },                      // left hand
	{  47,                    82 +   INV_SLOT_SIZE_PX }, // left hand
	{  47 + INV_SLOT_SIZE_PX, 82 +   INV_SLOT_SIZE_PX }, // left hand
	{  57,                    82 + 2*INV_SLOT_SIZE_PX }, // left hand
	{  47 + INV_SLOT_SIZE_PX, 82 + 2*INV_SLOT_SIZE_PX }, // left hand
	{ 192,                    82 },                      // right hand
	{ 192 + INV_SLOT_SIZE_PX, 82 },                      // right hand
	{ 192,                    82 +   INV_SLOT_SIZE_PX }, // right hand
	{ 192 + INV_SLOT_SIZE_PX, 82 +   INV_SLOT_SIZE_PX }, // right hand
	{ 192,                    82 + 2*INV_SLOT_SIZE_PX }, // right hand
	{ 192 + INV_SLOT_SIZE_PX, 82 + 2*INV_SLOT_SIZE_PX }, // right hand
	{ 121,                    93 },                      // chest
	{ 121 + INV_SLOT_SIZE_PX, 93 },                      // chest
	{ 121,                    93 +   INV_SLOT_SIZE_PX }, // chest
	{ 121 + INV_SLOT_SIZE_PX, 93 +   INV_SLOT_SIZE_PX }, // chest
	{ 121,                    93 + 2*INV_SLOT_SIZE_PX }, // chest
	{ 121 + INV_SLOT_SIZE_PX, 93 + 2*INV_SLOT_SIZE_PX }, // chest
	{  2 + 0 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 1 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 2 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 3 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 4 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 5 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 6 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 7 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 8 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 9 * (INV_SLOT_SIZE_PX + 1), 206 }, // inv row 1
	{  2 + 0 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 1 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 2 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 3 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 4 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 5 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 6 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 7 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 8 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 9 * (INV_SLOT_SIZE_PX + 1), 206 + 1 * (INV_SLOT_SIZE_PX + 1) }, // inv row 2
	{  2 + 0 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 1 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 2 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 3 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 4 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 5 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 6 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 7 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 8 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 9 * (INV_SLOT_SIZE_PX + 1), 206 + 2 * (INV_SLOT_SIZE_PX + 1) }, // inv row 3
	{  2 + 0 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 1 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 2 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 3 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 4 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 5 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 6 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 7 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 8 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{  2 + 9 * (INV_SLOT_SIZE_PX + 1), 206 + 3 * (INV_SLOT_SIZE_PX + 1) }, // inv row 4
	{ 1                   ,  MENUBTN_HEIGHT + 3 + 3 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 1                   ,  MENUBTN_HEIGHT + 3 + 2 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 1                   ,  MENUBTN_HEIGHT + 3 + 1 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 1                   ,  MENUBTN_HEIGHT + 3 + 0 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 2 + INV_SLOT_SIZE_PX,  MENUBTN_HEIGHT + 3 + 3 * (INV_SLOT_SIZE_PX + 1) }, // belt column 2
	{ 2 + INV_SLOT_SIZE_PX,  MENUBTN_HEIGHT + 3 + 2 * (INV_SLOT_SIZE_PX + 1) }, // belt column 2
	{ 2 + INV_SLOT_SIZE_PX,  MENUBTN_HEIGHT + 3 + 1 * (INV_SLOT_SIZE_PX + 1) }, // belt column 2
	{ 2 + INV_SLOT_SIZE_PX,  MENUBTN_HEIGHT + 3 + 0 * (INV_SLOT_SIZE_PX + 1) }  // belt column 2
	// clang-format on
};

/* InvSlotXY to InvSlot map. */
const BYTE InvSlotTbl[] = {
	// clang-format off
	SLOT_HEAD,
	SLOT_HEAD,
	SLOT_HEAD,
	SLOT_HEAD,
	SLOT_RING_LEFT,
	SLOT_RING_RIGHT,
	SLOT_AMULET,
	SLOT_HAND_LEFT,
	SLOT_HAND_LEFT,
	SLOT_HAND_LEFT,
	SLOT_HAND_LEFT,
	SLOT_HAND_LEFT,
	SLOT_HAND_LEFT,
	SLOT_HAND_RIGHT,
	SLOT_HAND_RIGHT,
	SLOT_HAND_RIGHT,
	SLOT_HAND_RIGHT,
	SLOT_HAND_RIGHT,
	SLOT_HAND_RIGHT,
	SLOT_CHEST,
	SLOT_CHEST,
	SLOT_CHEST,
	SLOT_CHEST,
	SLOT_CHEST,
	SLOT_CHEST,
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 1
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 2
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 3
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_STORAGE, // inv row 4
	SLOT_BELT,
	SLOT_BELT,
	SLOT_BELT,
	SLOT_BELT,
	SLOT_BELT,
	SLOT_BELT,
	SLOT_BELT,
	SLOT_BELT 
	// clang-format on
};

/** Specifies the starting inventory slots for placement of 2x2 items. */
const int AP2x2Tbl[10] = { 8, 28, 6, 26, 4, 24, 2, 22, 0, 20 };

void FreeInvGFX()
{
	MemFreeDbg(pInvCels);
	MemFreeDbg(pBeltCels);
}

void InitInv()
{
	pInvCels = LoadFileInMem("Data\\Inv\\Inv.CEL", NULL);
	pBeltCels = LoadFileInMem("Data\\Inv\\Belt.CEL", NULL);
	invflag = FALSE;
}

static void InvDrawSlotBack(int X, int Y, int W, int H)
{
	BYTE *dst;

	assert(gpBuffer != NULL);

	dst = &gpBuffer[X + BUFFER_WIDTH * Y];

	int wdt, hgt;
	BYTE pix;

	for (hgt = H; hgt; hgt--, dst -= BUFFER_WIDTH + W) {
		for (wdt = W; wdt; wdt--, dst++) {
			pix = *dst;
			if (pix >= PAL16_BLUE) {
				if (pix <= PAL16_BLUE + 15)
					*dst -= PAL16_BLUE - PAL16_BEIGE;
				else if (pix >= PAL16_GRAY)
					*dst -= PAL16_GRAY - PAL16_BEIGE;
			}
		}
	}
}

static BYTE InvItemColor(ItemStruct *is)
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
	int frame, frame_width, screen_x, screen_y, i, ii;
	BYTE *cCels;

	CelDraw(RIGHT_PANEL_X, SCREEN_Y + SPANEL_HEIGHT - 1, pInvCels, 1, SPANEL_WIDTH);

	cCels = pCursCels;

	p = &plr[myplr];
	is = &p->InvBody[INVLOC_HEAD];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_HEAD_FIRST].X;
		screen_y = SCREEN_Y + InvRect[SLOTXY_HEAD_FIRST].Y + INV_SLOT_SIZE_PX;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		if (pcursinvitem == INVITEM_HEAD) {
			CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}
	}

	is = &p->InvBody[INVLOC_RING_LEFT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_RING_LEFT].X;
		screen_y = SCREEN_Y + InvRect[SLOTXY_RING_LEFT].Y;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_RING_LEFT) {
			CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}
	}

	is = &p->InvBody[INVLOC_RING_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_RING_RIGHT].X;
		screen_y = SCREEN_Y + InvRect[SLOTXY_RING_RIGHT].Y;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_RING_RIGHT) {
			CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}
	}

	is = &p->InvBody[INVLOC_AMULET];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_AMULET].X;
		screen_y = SCREEN_Y + InvRect[SLOTXY_AMULET].Y;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_AMULET) {
			CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}
	}

	is = &p->InvBody[INVLOC_HAND_LEFT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_HAND_LEFT_FIRST].X;
		screen_y = SCREEN_Y + InvRect[SLOTXY_HAND_LEFT_FIRST].Y + 2 * INV_SLOT_SIZE_PX;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		if (frame_width == INV_SLOT_SIZE_PX)
			screen_x += INV_SLOT_SIZE_PX / 2;
		if (InvItemHeight[frame] != (3 * INV_SLOT_SIZE_PX))
			screen_y -= INV_SLOT_SIZE_PX / 2;

		if (pcursinvitem == INVITEM_HAND_LEFT) {
			CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iLoc == ILOC_TWOHAND) {
#ifdef HELLFIRE
			if (p->_pClass != PC_BARBARIAN
			    || is->_itype != ITYPE_SWORD
			        && is->_itype != ITYPE_MACE)
#endif
			{
				screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_HAND_RIGHT_FIRST].X;
				screen_y = SCREEN_Y + InvRect[SLOTXY_HAND_RIGHT_FIRST].Y + 2 * INV_SLOT_SIZE_PX;
				InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);
				light_table_index = 0;
				cel_transparency_active = TRUE;

				if (frame_width == INV_SLOT_SIZE_PX)
					screen_x += INV_SLOT_SIZE_PX / 2;
				if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
					screen_y -= INV_SLOT_SIZE_PX / 2;
				CelClippedDrawLightTrans(screen_x, screen_y, cCels, frame, frame_width);

				cel_transparency_active = FALSE;
			}
		}
	}

	is = &p->InvBody[INVLOC_HAND_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_HAND_RIGHT_FIRST].X;
		screen_y = SCREEN_Y + InvRect[SLOTXY_HAND_RIGHT_FIRST].Y + 2 * INV_SLOT_SIZE_PX;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		if (frame_width == INV_SLOT_SIZE_PX)
			screen_x += INV_SLOT_SIZE_PX / 2;
		if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
			screen_y -= INV_SLOT_SIZE_PX / 2;

		if (pcursinvitem == INVITEM_HAND_RIGHT) {
			CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}
	}

	is = &p->InvBody[INVLOC_CHEST];
	if (is->_itype != ITYPE_NONE) {
		screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_CHEST_FIRST].X;
		screen_y = SCREEN_Y + InvRect[SLOTXY_CHEST_FIRST].Y + 2 * INV_SLOT_SIZE_PX;
		InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		if (pcursinvitem == INVITEM_CHEST) {
			CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (p->InvGrid[i] != 0) {
			InvDrawSlotBack(
			    InvRect[i + SLOTXY_INV_FIRST].X + RIGHT_PANEL_X,
			    InvRect[i + SLOTXY_INV_FIRST].Y + SCREEN_Y,
			    INV_SLOT_SIZE_PX,
			    INV_SLOT_SIZE_PX);
		}
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		ii = p->InvGrid[i];
		if (ii > 0) { // first slot of an item
			ii--;
			is = &p->InvList[ii];

			screen_x = InvRect[i + SLOTXY_INV_FIRST].X + RIGHT_PANEL_X;
			screen_y = InvRect[i + SLOTXY_INV_FIRST].Y + SCREEN_Y;

			frame = is->_iCurs + CURSOR_FIRSTITEM;
			frame_width = InvItemWidth[frame];

			if (pcursinvitem == ii + INVITEM_INV_FIRST) {
				CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
			}

			if (is->_iStatFlag) {
				CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
			} else {
				CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
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

	CelDraw(SCREEN_X + InvRect[SLOTXY_BELT_FIRST].X - 1, SCREEN_Y + SCREEN_HEIGHT - InvRect[SLOTXY_BELT_LAST].Y + 1, pBeltCels, 1, 60);

	cCels = pCursCels;

	is = plr[myplr].SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, is++) {
		if (is->_itype == ITYPE_NONE) {
			continue;
		}
		screen_x = InvRect[SLOTXY_BELT_FIRST + i].X + SCREEN_X;
		screen_y = SCREEN_Y + SCREEN_HEIGHT - InvRect[SLOTXY_BELT_FIRST + i].Y;
		InvDrawSlotBack(screen_x, screen_y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);
		frame = is->_iCurs + CURSOR_FIRSTITEM;
		assert(InvItemWidth[frame] == INV_SLOT_SIZE_PX);
		frame_width = INV_SLOT_SIZE_PX;

		if (pcursinvitem == i + INVITEM_BELT_FIRST) {
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
			if (!sgbControllerActive || invflag)
#endif
				CelDrawOutline(InvItemColor(is), screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag) {
			CelClippedDraw(screen_x, screen_y, cCels, frame, frame_width);
		} else {
			CelDrawLightRed(screen_x, screen_y, cCels, frame, frame_width);
		}

		if (is->_iStatFlag && AllItemsList[is->_iIdx].iUsable) {
			fi = i + 49; // '1' + i;
			ff = fontframe[gbFontTransTbl[fi]];
			PrintChar(screen_x + INV_SLOT_SIZE_PX - fontkern[ff], screen_y, ff, COL_WHITE);
		}
	}
}

/** 
 * @brief Try to place an item with the given dimensions in the inventory
 *        of the selected player.
 * @param pnum the id of the player
 * @param ii the inventory index to place the item
 * @param sx the width of the item
 * @param sy the height of the item
 * @param is the item to place, if NULL the item wont be added to the inventory
 * @return TRUE if the item fits
*/
BOOL AutoPlace(int pnum, int ii, int sx, int sy, ItemStruct *is)
{
	PlayerStruct *p;
	int i, j, xx, yy;
	BOOL done;

	p = &plr[pnum];
	done = TRUE;
	if (ii < 0)
		ii = 0;
	yy = 10 * (ii / 10);
	for (j = 0; j < sy && done; j++) {
		if (yy >= NUM_INV_GRID_ELEM) {
			done = FALSE;
		}
		xx = ii % 10;
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
	if (done && is != NULL) {
		copy_pod(p->InvList[p->_pNumInv], *is);
		p->_pNumInv++;
		yy = 10 * (ii / 10);
		for (j = 0; j < sy; j++) {
			xx = ii % 10;
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

BOOL GoldAutoPlace(int pnum, ItemStruct *is)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int free, value, ii, done;

	p = &plr[pnum];
	value = is->_ivalue;
	pi = p->InvList;
	done = 0;
	for (int i = p->_pNumInv; i > 0 && done < 2; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			free = GOLD_MAX_LIMIT - pi->_ivalue;
			if (free > 0) {
				value -= free;
				if (value <= 0) {
					SetGoldItemValue(pi, GOLD_MAX_LIMIT + value);
					done |= 2;
				} else {
					SetGoldItemValue(pi, GOLD_MAX_LIMIT);
					done |= 1;
				}			
			}
		}
	}
	for (int i = NUM_INV_GRID_ELEM - 1; i >= 0 && done < 2; i--) {
		if (p->InvGrid[i] == 0) {
			ii = p->_pNumInv;
			p->_pNumInv = ii + 1;
			p->InvGrid[i] = ii + 1;
			pi = &p->InvList[ii];
			copy_pod(*pi, *is);
			value -= GOLD_MAX_LIMIT;
			if (value <= 0) {
				SetGoldItemValue(pi, value + GOLD_MAX_LIMIT);
				done |= 2;
			} else {
				SetGoldItemValue(pi, GOLD_MAX_LIMIT);
				GetItemSeed(is);
				done |= 1;
			}
		}
	}

	if (done == 0)
		return FALSE;

	CalculateGold(pnum);
	if (done == 1) {
		// partial placement
		SetGoldItemValue(is, value);
		NewCursor(is->_iCurs + CURSOR_FIRSTITEM);
		return FALSE;
	} else {
		// complete placement
		is->_itype = ITYPE_NONE;
		NewCursor(CURSOR_HAND);
		return TRUE;
	}
}

BOOL WeaponAutoPlace(int pnum, ItemStruct *is, BOOL saveflag)
{
	PlayerStruct *p;

	if (!is->_iStatFlag || is->_iClass != ICLASS_WEAPON)
		return FALSE;

	p = &plr[pnum];
	if (p->_pmode > PM_WALK3)
		return FALSE;

	if ((p->_pgfxnum & 0xF) != ANIM_ID_UNARMED && (p->_pgfxnum & 0xF) != ANIM_ID_UNARMED_SHIELD)
#ifdef HELLFIRE
		if (p->_pClass != PC_BARD || ((p->_pgfxnum & 0xF) != ANIM_ID_MACE && (p->_pgfxnum & 0xF) != ANIM_ID_SWORD))
#endif
			return FALSE;

#ifdef HELLFIRE
	if (p->_pClass == PC_MONK)
		return FALSE;
#endif
	if (is->_iLoc != ILOC_TWOHAND
#ifdef HELLFIRE
	    || (p->_pClass == PC_BARBARIAN && (is->_itype == ITYPE_SWORD || is->_itype == ITYPE_MACE))
#endif
	) {
#ifdef HELLFIRE
		if (p->_pClass != PC_BARD)
#endif
		{
			if (p->InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE && p->InvBody[INVLOC_HAND_LEFT]._iClass == ICLASS_WEAPON)
				return FALSE;
			if (p->InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE && p->InvBody[INVLOC_HAND_RIGHT]._iClass == ICLASS_WEAPON)
				return FALSE;
		}

		if (p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE) {
			if (saveflag) {
				NetSendCmdChItem(TRUE, is, INVLOC_HAND_LEFT);
				copy_pod(p->InvBody[INVLOC_HAND_LEFT], *is);
				CalcPlrInv(pnum, TRUE);
			}
			return TRUE;
		}
		if (p->InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE && p->InvBody[INVLOC_HAND_LEFT]._iLoc != ILOC_TWOHAND) {
			if (saveflag) {
				NetSendCmdChItem(TRUE, is, INVLOC_HAND_RIGHT);
				copy_pod(p->InvBody[INVLOC_HAND_RIGHT], *is);
				CalcPlrInv(pnum, TRUE);
			}
			return TRUE;
		}
	} else if (p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE && p->InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE) {
		if (saveflag) {
			NetSendCmdChItem(TRUE, is, INVLOC_HAND_LEFT);
			copy_pod(p->InvBody[INVLOC_HAND_LEFT], *is);
			CalcPlrInv(pnum, TRUE);
		}
		return TRUE;
	}

	return FALSE;
}

BOOL AutoPlaceInv(int pnum, ItemStruct *is, BOOL saveflag)
{
	ItemStruct *pi;
	int i, w, h;

	i = is->_iCurs + CURSOR_FIRSTITEM;
	w = InvItemWidth[i] / INV_SLOT_SIZE_PX;
	h = InvItemHeight[i] / INV_SLOT_SIZE_PX;

	pi = saveflag ? is : NULL;
	if (w == 1 && h == 1) {
		if (AllItemsList[is->_iIdx].iUsable && is->_iStatFlag) {
			for (i = 0; i < MAXBELTITEMS; i++) {
				if (plr[pnum].SpdList[i]._itype == ITYPE_NONE) {
					if (pi != NULL) {
						copy_pod(plr[pnum].SpdList[i], *pi);
						CalcPlrScrolls(pnum);
						//gbRedrawFlags |= REDRAW_SPEED_BAR;
					}
					return TRUE;
				}
			}
		}
		for (i = 30; i <= 39; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 20; i <= 29; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 10; i <= 19; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 0; i <= 9; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
	}
	if (w == 1 && h == 2) {
		for (i = 29; i >= 20; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 9; i >= 0; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 19; i >= 10; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
	}
	if (w == 1 && h == 3) {
		for (i = 0; i < 20; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
	}
	if (w == 2 && h == 2) {
		for (i = 0; i < 10; i++) {
			if (AutoPlace(pnum, AP2x2Tbl[i], w, h, pi))
				return TRUE;
		}
		for (i = 21; i < 29; i += 2) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 1; i < 9; i += 2) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 10; i < 19; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
	}
	if (w == 2 && h == 3) {
		for (i = 0; i < 9; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
		for (i = 10; i < 19; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return TRUE;
		}
	}
	return FALSE;
}

static int SwapItem(ItemStruct *a, ItemStruct *b)
{
	ItemStruct h;

	copy_pod(h, *a);
	copy_pod(*a, *b);
	copy_pod(*b, h);

	return h._iCurs + CURSOR_FIRSTITEM;
}

static void CheckInvPaste()
{
	PlayerStruct *p;
	ItemStruct *holditem, *is, *wRight;
	int r, sx, sy;
	int i, j, xx, yy, ii;
	BOOL done;
	int il, cn, it, iv, ig, gt;

	p = &plr[myplr];
	holditem = &p->HoldItem;

	//r = holditem->_iCurs + CURSOR_FIRSTITEM;
	sx = cursW; // InvItemWidth[r];
	sy = cursH; // InvItemHeight[r];
	i = MouseX + (sx >> 1);
	j = MouseY + (sy >> 1);
	sx /= INV_SLOT_SIZE_PX;
	sy /= INV_SLOT_SIZE_PX;

	for (r = 0; r < SLOTXY_BELT_FIRST; r++) {
		if (i >= InvRect[r].X + RIGHT_PANEL
		 && i <= InvRect[r].X + RIGHT_PANEL + INV_SLOT_SIZE_PX
		 && j >= InvRect[r].Y - INV_SLOT_SIZE_PX
		 && j <= InvRect[r].Y) {
			break;
		}
		if (r == SLOTXY_CHEST_LAST) {
			if ((sx & 1) == 0)
				i -= INV_SLOT_SIZE_PX / 2;
			if ((sy & 1) == 0)
				j -= INV_SLOT_SIZE_PX / 2;
		}
	}
	if (r == SLOTXY_BELT_FIRST)
		return;

	switch (InvSlotTbl[r]) {
	case SLOT_HEAD:
		il = ILOC_HELM;
		break;
	case SLOT_RING_LEFT:
	case SLOT_RING_RIGHT:
		il = ILOC_RING;
		break;
	case SLOT_AMULET:
		il = ILOC_AMULET;
		break;
	case SLOT_HAND_LEFT:
	case SLOT_HAND_RIGHT:
		il = ILOC_ONEHAND;
		break;
	case SLOT_CHEST:
		il = ILOC_ARMOR;
		break;
	case SLOT_STORAGE:
		il = ILOC_UNEQUIPABLE;
		break;
	default:
		ASSUME_UNREACHABLE
	}
	done = FALSE;
	if (il == ILOC_UNEQUIPABLE) {
		done = TRUE;
		it = 0;
		ii = r - SLOTXY_INV_FIRST;
		if (holditem->_itype == ITYPE_GOLD) {
			iv = p->InvGrid[ii];
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
	} else if (il == holditem->_iLoc) {
		done = TRUE;
	} else if (il == ILOC_ONEHAND && holditem->_iLoc == ILOC_TWOHAND) {
#ifdef HELLFIRE
		if (p->_pClass != PC_BARBARIAN
			|| (holditem->_itype != ITYPE_SWORD && holditem->_itype != ITYPE_MACE))
#endif
			il = ILOC_TWOHAND;
		done = TRUE;
	}

	if (!done)
		return;

	if (il != ILOC_UNEQUIPABLE && !holditem->_iStatFlag) {
		PlaySFX(sgSFXSets[SFXS_PLR_13][p->_pClass]);
		return;
	}

	PlaySFX(ItemInvSnds[ItemCAnimTbl[holditem->_iCurs]]);

	cn = CURSOR_HAND;
	switch (il) {
	case ILOC_HELM:
		NetSendCmdChItem(FALSE, holditem, INVLOC_HEAD);
		is = &p->InvBody[INVLOC_HEAD];
		if (is->_itype == ITYPE_NONE)
			copy_pod(*is, *holditem);
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_RING:
		if (r == SLOTXY_RING_LEFT) {
			NetSendCmdChItem(FALSE, holditem, INVLOC_RING_LEFT);
			is = &p->InvBody[INVLOC_RING_LEFT];
			if (is->_itype == ITYPE_NONE)
				copy_pod(*is, *holditem);
			else
				cn = SwapItem(is, holditem);
		} else {
			NetSendCmdChItem(FALSE, holditem, INVLOC_RING_RIGHT);
			is = &p->InvBody[INVLOC_RING_RIGHT];
			if (is->_itype == ITYPE_NONE)
				copy_pod(*is, *holditem);
			else
				cn = SwapItem(is, holditem);
		}
		break;
	case ILOC_AMULET:
		NetSendCmdChItem(FALSE, holditem, INVLOC_AMULET);
		is = &p->InvBody[INVLOC_AMULET];
		if (is->_itype == ITYPE_NONE)
			copy_pod(*is, *holditem);
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_ONEHAND:
		is = &p->InvBody[INVLOC_HAND_LEFT];
		wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		if (r <= SLOTXY_HAND_LEFT_LAST) {
			if (is->_itype == ITYPE_NONE) {
				if (wRight->_itype == ITYPE_NONE || wRight->_iClass != holditem->_iClass
#ifdef HELLFIRE
				 || (p->_pClass == PC_BARD && wRight->_iClass == ICLASS_WEAPON)
#endif
				) {
					NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_LEFT);
					copy_pod(*is, *holditem);
				} else {
					NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_RIGHT);
					cn = SwapItem(wRight, holditem);
				}
				break;
			}
			if (wRight->_itype == ITYPE_NONE || wRight->_iClass != holditem->_iClass
#ifdef HELLFIRE
			 || (p->_pClass == PC_BARD && wRight->_iClass == ICLASS_WEAPON)
#endif
			) {
				NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_LEFT);
				cn = SwapItem(is, holditem);
				break;
			}

			NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_RIGHT);
			cn = SwapItem(wRight, holditem);
			break;
		}
		if (wRight->_itype == ITYPE_NONE) {
			if (is->_itype == ITYPE_NONE || is->_iLoc != ILOC_TWOHAND
#ifdef HELLFIRE
			 || (p->_pClass == PC_BARBARIAN && (is->_itype == ITYPE_SWORD || is->_itype == ITYPE_MACE))
#endif
			) {
				if (is->_itype == ITYPE_NONE || is->_iClass != holditem->_iClass
#ifdef HELLFIRE
				 || (p->_pClass == PC_BARD && is->_iClass == ICLASS_WEAPON)
#endif
				) {
					NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_RIGHT);
					copy_pod(*wRight, *holditem);
					break;
				}
				NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_LEFT);
				cn = SwapItem(is, holditem);
				break;
			}
			NetSendCmdDelItem(FALSE, INVLOC_HAND_LEFT);
			NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_RIGHT);
			SwapItem(wRight, is);
			cn = SwapItem(wRight, holditem);
			break;
		}

		if (is->_itype != ITYPE_NONE && is->_iClass == holditem->_iClass
#ifdef HELLFIRE
		 && (p->_pClass != PC_BARD || is->_iClass != ICLASS_WEAPON)
#endif
		) {
			NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_LEFT);
			cn = SwapItem(is, holditem);
			break;
		}
		NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_RIGHT);
		cn = SwapItem(wRight, holditem);
		break;
	case ILOC_TWOHAND:
		is = &p->InvBody[INVLOC_HAND_LEFT];
		wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		NetSendCmdDelItem(FALSE, INVLOC_HAND_RIGHT);
		if (is->_itype != ITYPE_NONE && wRight->_itype != ITYPE_NONE) {
			if (wRight->_itype != ITYPE_SHIELD)
				wRight = is;
			if (!AutoPlaceInv(myplr, wRight, TRUE))
				return;

			wRight->_itype = ITYPE_NONE;
			wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		}

		if (is->_itype != ITYPE_NONE || wRight->_itype != ITYPE_NONE) {
			NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_LEFT);
			if (is->_itype == ITYPE_NONE)
				SwapItem(is, wRight);
			cn = SwapItem(is, holditem);
		} else {
			NetSendCmdChItem(FALSE, holditem, INVLOC_HAND_LEFT);
			copy_pod(*is, *holditem);
		}
		break;
	case ILOC_ARMOR:
		NetSendCmdChItem(FALSE, holditem, INVLOC_CHEST);
		is = &p->InvBody[INVLOC_CHEST];
		if (is->_itype == ITYPE_NONE)
			copy_pod(*is, *holditem);
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_UNEQUIPABLE:
		if (holditem->_itype == ITYPE_GOLD && it == 0) {
			ii = r - SLOTXY_INV_FIRST;
			il = p->InvGrid[ii];
			if (il > 0) {
				il--;
				is = &p->InvList[il];
				gt = is->_ivalue;
				ig = holditem->_ivalue + gt;
				if (ig <= GOLD_MAX_LIMIT) {
					p->_pGold += ig - gt;
					SetGoldItemValue(is, ig);
				} else {
					ig = GOLD_MAX_LIMIT - gt;
					p->_pGold += ig;
					SetGoldItemValue(holditem, holditem->_ivalue - ig);
					SetGoldItemValue(is, GOLD_MAX_LIMIT);
					// BUGFIX: incorrect values here are leftover from beta
					if (holditem->_ivalue >= GOLD_MEDIUM_LIMIT)
						cn = ICURS_GOLD_LARGE + CURSOR_FIRSTITEM;
					else if (holditem->_ivalue <= GOLD_SMALL_LIMIT)
						cn = ICURS_GOLD_SMALL + CURSOR_FIRSTITEM;
					else
						cn = ICURS_GOLD_MEDIUM + CURSOR_FIRSTITEM;
				}
			} else {
				il = p->_pNumInv;
				copy_pod(p->InvList[il], *holditem);
				p->_pNumInv++;
				p->InvGrid[ii] = p->_pNumInv;
				p->_pGold += holditem->_ivalue;
				if (holditem->_ivalue <= GOLD_MAX_LIMIT) {
					SetGoldItemValue(&p->InvList[il], holditem->_ivalue);
#ifdef HELLFIRE
				} else {
					p->InvList[il]._iCurs = ICURS_GOLD_LARGE;
#endif
				}
			}
		} else {
			if (it == 0) {
				copy_pod(p->InvList[p->_pNumInv], *holditem);
				p->_pNumInv++;
				it = p->_pNumInv;
			} else {
				il = it - 1;
				if (holditem->_itype == ITYPE_GOLD)
					p->_pGold += holditem->_ivalue;
				cn = SwapItem(&p->InvList[il], holditem);
				if (holditem->_itype == ITYPE_GOLD)
					CalculateGold(myplr);
				for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
					if (p->InvGrid[i] == it)
						p->InvGrid[i] = 0;
					else if (p->InvGrid[i] == -it)
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
	default:
		ASSUME_UNREACHABLE
	}
	CalcPlrInv(myplr, TRUE);
	if (cn == CURSOR_HAND)
		SetCursorPos(MouseX + (cursW >> 1), MouseY + (cursH >> 1));
	NewCursor(cn);
}

static BOOL CheckBeltPaste()
{
	ItemStruct *holditem, *is;
	int r, i, j, cn;

	i = MouseX + INV_SLOT_SIZE_PX / 2;
	j = MouseY + INV_SLOT_SIZE_PX / 2;

	for (r = SLOTXY_BELT_FIRST; r <= SLOTXY_BELT_LAST; r++) {
		if (i >= InvRect[r].X
		 && i <= InvRect[r].X + INV_SLOT_SIZE_PX
		 && j >= SCREEN_HEIGHT - InvRect[r].Y - INV_SLOT_SIZE_PX
		 && j <= SCREEN_HEIGHT - InvRect[r].Y) {
			break;
		}
	}
	if (r > SLOTXY_BELT_LAST)
		return FALSE;

	holditem = &plr[myplr].HoldItem;

	// BUGFIX: TODO why is _iLoc not set to ILOC_BELT?
	if (holditem->_iLoc != ILOC_BELT
	 && (holditem->_iLoc != ILOC_UNEQUIPABLE
		 || cursW != INV_SLOT_SIZE_PX || cursH != INV_SLOT_SIZE_PX
		 || !AllItemsList[holditem->_iIdx].iUsable)) {
		return TRUE;
	}
	
	PlaySFX(ItemInvSnds[ItemCAnimTbl[holditem->_iCurs]]);

	is = &plr[myplr].SpdList[r - SLOTXY_BELT_FIRST];
	cn = CURSOR_HAND;
	if (is->_itype == ITYPE_NONE) {
		copy_pod(*is, *holditem);
	} else {
		cn = SwapItem(is, holditem);
	}
	//gbRedrawFlags |= REDRAW_SPEED_BAR;
	CalcPlrScrolls(myplr);
	//CalcPlrInv(myplr, TRUE);
	if (cn == CURSOR_HAND)
		SetCursorPos(MouseX + (cursW >> 1), MouseY + (cursH >> 1));
	NewCursor(cn);
	return TRUE;
}

void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, BOOL bId)
{
	PlayerStruct *p;

	RecreateItem(idx, wCI, seed, 0);

	p = &plr[pnum];
	copy_pod(p->HoldItem, item[MAXITEMS]);

	if (bId) {
		p->HoldItem._iIdentified = TRUE;
	}

	if (bLoc < NUM_INVLOC) {
		copy_pod(p->InvBody[bLoc], p->HoldItem);

		if (bLoc == INVLOC_HAND_LEFT && p->HoldItem._iLoc == ILOC_TWOHAND) {
			p->InvBody[INVLOC_HAND_RIGHT]._itype = ITYPE_NONE;
		} else if (bLoc == INVLOC_HAND_RIGHT && p->HoldItem._iLoc == ILOC_TWOHAND) {
			p->InvBody[INVLOC_HAND_LEFT]._itype = ITYPE_NONE;
		}
	}

	CalcPlrInv(pnum, TRUE);
}

static BOOL CheckInvCut()
{
	PlayerStruct *p;
	ItemStruct *pi;
	char ii;
	int r, i, j;

	p = &plr[myplr];
	if (p->_pmode > PM_WALK3) {
		return FALSE;
	}

	r = pcursinvitem;
	if (r == -1)
		return FALSE;

	//p->HoldItem._itype = ITYPE_NONE;

	switch (r) {
	case INVITEM_HEAD:
		pi = &p->InvBody[INVLOC_HEAD];
		assert(pi->_itype != ITYPE_NONE);
		NetSendCmdDelItem(FALSE, INVLOC_HEAD);
		break;
	case INVITEM_RING_LEFT:
		pi = &p->InvBody[INVLOC_RING_LEFT];
		assert(pi->_itype != ITYPE_NONE);
		NetSendCmdDelItem(FALSE, INVLOC_RING_LEFT);
		break;
	case INVITEM_RING_RIGHT:
		pi = &p->InvBody[INVLOC_RING_RIGHT];
		assert(pi->_itype != ITYPE_NONE);
		NetSendCmdDelItem(FALSE, INVLOC_RING_RIGHT);
		break;
	case INVITEM_AMULET:
		pi = &p->InvBody[INVLOC_AMULET];
		assert(pi->_itype != ITYPE_NONE);
		NetSendCmdDelItem(FALSE, INVLOC_AMULET);
		break;
	case INVITEM_HAND_LEFT:
		pi = &p->InvBody[INVLOC_HAND_LEFT];
		assert(pi->_itype != ITYPE_NONE);
		NetSendCmdDelItem(FALSE, INVLOC_HAND_LEFT);
		break;
	case INVITEM_HAND_RIGHT:
		pi = &p->InvBody[INVLOC_HAND_RIGHT];
		assert(pi->_itype != ITYPE_NONE);
		NetSendCmdDelItem(FALSE, INVLOC_HAND_RIGHT);
		break;
	case INVITEM_CHEST:
		pi = &p->InvBody[INVLOC_CHEST];
		assert(pi->_itype != ITYPE_NONE);
		NetSendCmdDelItem(FALSE, INVLOC_CHEST);
		break;
	default:
		if (r >= INVITEM_INV_FIRST && r <= INVITEM_INV_LAST) {
			ii = r - INVITEM_INV_FIRST;
			for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
				if (abs(p->InvGrid[i]) == ii + 1) {
					p->InvGrid[i] = 0;
				}
			}

			pi = &p->InvList[ii];
			p->_pNumInv--;
			i = p->_pNumInv;
			if (i > 0 && i != ii) {
				pi = &p->InvList[i];
				SwapItem(&p->InvList[ii], pi);

				i++;
				ii++;
				for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
					if (p->InvGrid[j] == i) {
						p->InvGrid[j] = ii;
					} else if (p->InvGrid[j] == -i) {
						p->InvGrid[j] = -ii;
					}
				}
			}
		} else { // r >= INVITEM_BELT_FIRST
			pi = &p->SpdList[r - INVITEM_BELT_FIRST];
			assert(pi->_itype != ITYPE_NONE);
			//gbRedrawFlags |= REDRAW_SPEED_BAR;
		}
	}

	copy_pod(p->HoldItem, *pi);
	if (pi->_itype == ITYPE_GOLD) {
		CalculateGold(myplr);
	}
	pi->_itype = ITYPE_NONE;

	CalcPlrInv(myplr, TRUE);
	ItemStatOk(myplr, &p->HoldItem);

	PlaySFX(IS_IGRAB);
	NewCursor(p->HoldItem._iCurs + CURSOR_FIRSTITEM);
	SetCursorPos(MouseX - (cursW >> 1), MouseY - (cursH >> 1));
	return TRUE;
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
		copy_pod(p->InvList[iv], p->InvList[i]);

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

void RemoveSpdBarItem(int pnum, int iv)
{
	plr[pnum].SpdList[iv]._itype = ITYPE_NONE;

	CalcPlrScrolls(pnum);

	//gbRedrawFlags |= REDRAW_SPEED_BAR;
}

void CheckInvClick()
{
	if (pcurs >= CURSOR_FIRSTITEM) {
		CheckInvPaste();
	} else {
		CheckInvCut();
	}
}

/**
 * Check for interactions with belt
 */
BOOL CheckBeltClick()
{
	if (pcurs >= CURSOR_FIRSTITEM) {
		return CheckBeltPaste();
	} else {
		return CheckInvCut();
	}
}

static void CheckQuestItem(int pnum, ItemStruct *is)
{
	PlayerStruct *p;
	int idx;

	p = &plr[pnum];
	idx = is->_iIdx;
	if (idx == IDI_OPTAMULET && quests[Q_BLIND]._qactive == QUEST_ACTIVE)
		quests[Q_BLIND]._qactive = QUEST_DONE;
	else if (idx == IDI_MUSHROOM) {
		if (quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE && quests[Q_MUSHROOM]._qvar1 == QS_MUSHSPAWNED) {
			sfxdelay = 10;
			// BUGFIX: Voice for this quest might be wrong in MP
			sfxdnum = sgSFXSets[SFXS_PLR_95][p->_pClass];
			quests[Q_MUSHROOM]._qvar1 = QS_MUSHPICKED;
		}
	} else if (idx == IDI_ANVIL && quests[Q_ANVIL]._qactive != QUEST_NOTAVAIL) {
		if (quests[Q_ANVIL]._qactive == QUEST_INIT) {
			quests[Q_ANVIL]._qactive = QUEST_ACTIVE;
			quests[Q_ANVIL]._qvar1 = 1;
		}
		if (quests[Q_ANVIL]._qlog) {
			sfxdelay = 10;
			sfxdnum = sgSFXSets[SFXS_PLR_89][p->_pClass];
		}
	} else if (idx == IDI_GLDNELIX && quests[Q_VEIL]._qactive != QUEST_NOTAVAIL) {
		sfxdelay = 30;
		sfxdnum = sgSFXSets[SFXS_PLR_88][p->_pClass];
	} else if (idx == IDI_ROCK && quests[Q_ROCK]._qactive != QUEST_NOTAVAIL) {
		if (quests[Q_ROCK]._qactive == QUEST_INIT) {
			quests[Q_ROCK]._qactive = QUEST_ACTIVE;
			quests[Q_ROCK]._qvar1 = 1;
		}
		if (quests[Q_ROCK]._qlog) {
			sfxdelay = 10;
			sfxdnum = sgSFXSets[SFXS_PLR_87][p->_pClass];
		}
	} else if (idx == IDI_ARMOFVAL && quests[Q_BLOOD]._qactive == QUEST_ACTIVE) {
		quests[Q_BLOOD]._qactive = QUEST_DONE;
		sfxdelay = 20;
		sfxdnum = sgSFXSets[SFXS_PLR_91][p->_pClass];
#ifdef HELLFIRE
	} else if (idx == IDI_MAPOFDOOM) {
		quests[Q_GRAVE]._qlog = FALSE;
		quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
		quests[Q_GRAVE]._qvar1 = 1;
		sfxdelay = 10;
		sfxdnum = sgSFXSets[SFXS_PLR_79][p->_pClass];
	} else if (idx == IDI_NOTE1 || idx == IDI_NOTE2 || idx == IDI_NOTE3) {
		int nn;
		if ((idx == IDI_NOTE1 || PlrHasItem(pnum, IDI_NOTE1, &nn))
		 && (idx == IDI_NOTE2 || PlrHasItem(pnum, IDI_NOTE2, &nn))
		 && (idx == IDI_NOTE3 || PlrHasItem(pnum, IDI_NOTE3, &nn))) {
			sfxdelay = 10;
			sfxdnum = sgSFXSets[SFXS_PLR_46][p->_pClass];
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
			SetItemData(MAXITEMS, IDI_FULLNOTE);
			SetupItem(MAXITEMS);
			copy_pod(*is, item[MAXITEMS]);
			GetItemSeed(is);
		}
#endif
	}
}

void InvGetItem(int pnum, int ii)
{
	PlayerStruct *p;
	ItemStruct *is;
	int i;

	is = &item[ii];
	if (dItem[is->_ix][is->_iy] == 0)
		return;
	dItem[is->_ix][is->_iy] = 0;

	p = &plr[pnum];
	if (myplr == pnum && pcurs >= CURSOR_FIRSTITEM)
		NetSendCmdPItem(TRUE, CMD_SYNCPUTITEM, &p->HoldItem, p->_px, p->_py);
	// always mask CF_PREGEN to make life of RecreateItem easier later on
	// otherwise this should not have an effect, since the item is already in 'delta'
	is->_iCreateInfo &= ~CF_PREGEN;
	CheckQuestItem(pnum, is);
	SetBookLevel(pnum, is);
	ItemStatOk(pnum, is);
	copy_pod(p->HoldItem, *is);
	NewCursor(p->HoldItem._iCurs + CURSOR_FIRSTITEM);
	pcursitem = -1;

	for (i = 0; i < numitems; ) {
		if (itemactive[i] == ii) {
			DeleteItem(ii, i);
		} else {
			i++;
		}
	}
}

void AutoGetItem(int pnum, int ii)
{
	PlayerStruct *p;
	ItemStruct *is;
	int i;
	BOOL done;

	if (pcurs != CURSOR_HAND) {
		return;
	}

	is = &item[ii];
	if (dItem[is->_ix][is->_iy] == 0 && ii != MAXITEMS)
		return;

	p = &plr[pnum];
	// always mask CF_PREGEN to make life of RecreateItem easier later on
	// otherwise this should not have an effect, since the item is already in 'delta'
	is->_iCreateInfo &= ~CF_PREGEN;
	CheckQuestItem(pnum, is);
	SetBookLevel(pnum, is);
	ItemStatOk(pnum, is);
	if (is->_itype == ITYPE_GOLD) {
		done = GoldAutoPlace(pnum, is);
	} else {
		done = WeaponAutoPlace(pnum, is, TRUE)
			|| AutoPlaceInv(pnum, is, TRUE);
	}
	if (done) {
		dItem[is->_ix][is->_iy] = 0;
		for (i = 0; i < numitems; ) {
			if (itemactive[i] == ii) {
				DeleteItem(ii, i);
			} else {
				i++;
			}
		}
	} else {
		if (pnum == myplr) {
			PlaySFX(sgSFXSets[SFXS_PLR_14][p->_pClass], 3);
		}
		RespawnItem(ii, TRUE);
		NetSendCmdPItem(TRUE, CMD_RESPAWNITEM, is, is->_ix, is->_iy);
		NewCursor(CURSOR_HAND);
	}
}

int FindGetItem(WORD idx, WORD ci, int iseed)
{
	int i, ii;

	for (i = 0; i < numitems; i++) {
		ii = itemactive[i];
		if (item[ii]._iIdx == idx && item[ii]._iSeed == iseed && item[ii]._iCreateInfo == ci)
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
		if (item[ii]._iIdx != idx
		 || item[ii]._iSeed != iseed
		 || item[ii]._iCreateInfo != ci) {
			ii = FindGetItem(idx, ci, iseed);
		}
	} else {
		ii = FindGetItem(idx, ci, iseed);
	}

	if (ii != -1) {
		is = &item[ii];
		dItem[is->_ix][is->_iy] = 0;
		for (i = 0; i < numitems; ) {
			if (itemactive[i] == ii) {
				DeleteItem(ii, i);
			} else {
				i++;
			}
		}
	}
}

BOOL CanPut(int x, int y)
{
	int oi, oi2;

	if (x < DBORDERX || x >= DSIZEX + DBORDERX || y < DBORDERY || y >= DSIZEY + DBORDERY)
		return FALSE;

	if ((dItem[x][y] | nSolidTable[dPiece[x][y]]) != 0)
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

	if (currlevel == 0)
		if (dMonster[x][y] != 0 || dMonster[x + 1][y + 1] != 0)
			return FALSE;

	return TRUE;
}

BOOL FindItemLocation(int sx, int sy, int *dx, int *dy, int rad)
{
	int dir;
	int xx, yy, i, j, k;

	if (sx != *dx || sy != *dy) {
		dir = GetDirection(sx, sy, *dx, *dy);
		*dx = sx + offset_x[dir];
		*dy = sy + offset_y[dir];
		if (CanPut(*dx, *dy))
			return TRUE;

		dir = (dir - 1) & 7;
		*dx = sx + offset_x[dir];
		*dy = sy + offset_y[dir];
		if (CanPut(*dx, *dy))
			return TRUE;

		dir = (dir + 2) & 7;
		*dx = sx + offset_x[dir];
		*dy = sy + offset_y[dir];
		if (CanPut(*dx, *dy))
			return TRUE;

		*dx = sx;
		*dy = sy;
	}

	if (CanPut(*dx, *dy))
		return TRUE;

	xx = *dx;
	yy = *dy;
	for (k = 1; k <= rad; k++) {
		for (j = -k; j <= k; j++) {
			yy = j + sy;
			for (i = -k; i <= k; i++) {
				xx = i + sx;
				if (CanPut(xx, yy)) {
					*dx = xx;
					*dy = yy;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL DropItem()
{
	int x, y;

	if (numitems >= MAXITEMS)
		return FALSE;

	x = cursmx;
	y = cursmy;
	if (!FindItemLocation(plr[myplr]._px, plr[myplr]._py, &x, &y, 1))
		return FALSE;

	NetSendCmdPItem(TRUE, CMD_PUTITEM, &plr[myplr].HoldItem, cursmx, cursmy);
	NewCursor(CURSOR_HAND);
	return TRUE;
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
	int ii;

	if (numitems >= MAXITEMS)
		return -1;

	if (FindGetItem(plr[pnum].HoldItem._iIdx, plr[pnum].HoldItem._iCreateInfo, plr[pnum].HoldItem._iSeed) != -1) {
		DrawInvMsg("A duplicate item has been detected.  Destroying duplicate...");
		SyncGetItem(x, y, plr[pnum].HoldItem._iIdx, plr[pnum].HoldItem._iCreateInfo, plr[pnum].HoldItem._iSeed);
	}

	if (!FindItemLocation(plr[pnum]._px, plr[pnum]._py, &x, &y, DSIZEX / 2))
		return -1;

#ifdef HELLFIRE
	if (currlevel == 0) {
		if (plr[pnum].HoldItem._iCurs == ICURS_RUNE_BOMB && cursmx >= DBORDERX + 69 && cursmx <= DBORDERX + 72 && cursmy >= DBORDERY + 51 && cursmy <= DBORDERY + 54) {
			NetSendCmd(FALSE, CMD_OPENHIVE);
			quests[Q_FARMER]._qactive = QUEST_DONE;
			if (gbMaxPlayers != 1) {
				NetSendCmdQuest(TRUE, Q_FARMER);
			}
			return -1;
		}
		if (plr[pnum].HoldItem._iIdx == IDI_MAPOFDOOM && cursmx >= DBORDERX + 25  && cursmx <= DBORDERX + 28 && cursmy >= DBORDERY + 10 && cursmy <= DBORDERY + 14) {
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
	copy_pod(item[ii], plr[pnum].HoldItem);
	item[ii]._ix = x;
	item[ii]._iy = y;
	RespawnItem(ii, TRUE);
	numitems++;
	NewCursor(CURSOR_HAND);
	return ii;
}

int SyncPutItem(int pnum, int x, int y, ItemStruct *is)
{
	int ii;

	if (numitems >= MAXITEMS)
		return -1;

	if (FindGetItem(is->_iIdx, is->_iCreateInfo, is->_iSeed) != -1) {
		DrawInvMsg("A duplicate item has been detected from another player.");
		SyncGetItem(x, y, is->_iIdx, is->_iCreateInfo, is->_iSeed);
	}

	if (!FindItemLocation(plr[pnum]._px, plr[pnum]._py, &x, &y, DSIZEX / 2))
		return -1;

	ii = itemavail[0];
	dItem[x][y] = ii + 1;
	itemavail[0] = itemavail[MAXITEMS - (numitems + 1)];
	itemactive[numitems] = ii;
	copy_pod(item[ii], *is);
	item[ii]._ix = x;
	item[ii]._iy = y;
	RespawnItem(ii, TRUE);
	numitems++;
	return ii;
}

char CheckInvBelt()
{
	int r;

	for (r = SLOTXY_BELT_FIRST; r <= SLOTXY_BELT_LAST; r++) {
		if (MouseX >= InvRect[r].X
		 && MouseX <= InvRect[r].X + INV_SLOT_SIZE_PX
		 && MouseY >= SCREEN_HEIGHT - InvRect[r].Y - INV_SLOT_SIZE_PX
		 && MouseY <= SCREEN_HEIGHT - InvRect[r].Y) {
			break;
		}
	}
	r -= SLOTXY_BELT_FIRST;
	if (r < MAXBELTITEMS && plr[myplr].SpdList[r]._itype != ITYPE_NONE) {
		//gbRedrawFlags |= REDRAW_SPEED_BAR;
		return INVITEM_BELT_FIRST + r;
	}

	return -1;
}

char CheckInvItem()
{
	int r;
	ItemStruct *pi;
	PlayerStruct *p;
	char rv;

	for (r = 0; r < SLOTXY_BELT_FIRST; r++) {
		if (MouseX >= InvRect[r].X + RIGHT_PANEL
		 && MouseX <= InvRect[r].X + RIGHT_PANEL + INV_SLOT_SIZE_PX
		 && MouseY >= InvRect[r].Y - INV_SLOT_SIZE_PX
		 && MouseY <= InvRect[r].Y) {
			break;
		}
	}

	p = &plr[myplr];
	switch (InvSlotTbl[r]) {
	case SLOT_HEAD:
		rv = INVITEM_HEAD;
		static_assert(INVLOC_HEAD == INVITEM_HEAD, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_RING_LEFT:
		rv = INVITEM_RING_LEFT;
		static_assert(INVLOC_RING_LEFT == INVITEM_RING_LEFT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_RING_RIGHT:
		rv = INVITEM_RING_RIGHT;
		static_assert(INVLOC_RING_RIGHT == INVITEM_RING_RIGHT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_AMULET:
		rv = INVITEM_AMULET;
		static_assert(INVLOC_AMULET == INVITEM_AMULET, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_HAND_LEFT:
		rv = INVITEM_HAND_LEFT;
		static_assert(INVLOC_HAND_LEFT == INVITEM_HAND_LEFT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_HAND_RIGHT:
		rv = INVITEM_HAND_LEFT;
		static_assert(INVLOC_HAND_LEFT == INVITEM_HAND_LEFT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		pi = &p->InvBody[rv];
#ifdef HELLFIRE
		if (pi->_itype == ITYPE_NONE || pi->_iLoc != ILOC_TWOHAND
		    || (p->_pClass == PC_BARBARIAN && (pi->_itype == ITYPE_SWORD || pi->_itype == ITYPE_MACE))) {
#else
		if (pi->_itype == ITYPE_NONE || pi->_iLoc != ILOC_TWOHAND) {
#endif
			rv = INVITEM_HAND_RIGHT;
			static_assert(INVLOC_HAND_RIGHT == INVITEM_HAND_RIGHT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		}
		break;
	case SLOT_CHEST:
		rv = INVITEM_CHEST;
		static_assert(INVLOC_CHEST == INVITEM_CHEST, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		pi = &p->InvBody[rv];
		break;
	case SLOT_STORAGE:
		r = p->InvGrid[r - SLOTXY_INV_FIRST];
		if (r == 0)
			return -1;
		r = abs(r) - 1;
		assert(p->InvList[r]._itype != ITYPE_NONE);
		return INVITEM_INV_FIRST + r;
	case SLOT_BELT:
		return -1;
	default:
		ASSUME_UNREACHABLE
	}

	pi = &p->InvBody[rv];
	if (pi->_itype == ITYPE_NONE)
		return -1;

	return rv;
}

static void StartGoldDrop()
{
	if (talkflag || plr[myplr]._pmode != PM_STAND)
		return;
	initialDropGoldIndex = pcursinvitem;
	assert(pcursinvitem >= INVITEM_INV_FIRST && pcursinvitem <= INVITEM_INV_LAST);
	initialDropGoldValue = plr[myplr].InvList[pcursinvitem - INVITEM_INV_FIRST]._ivalue;
	dropGoldFlag = TRUE;
	dropGoldValue = 0;
}

static void InvAddHp()
{
	PlayerStruct *p;
	int hp;

	p = &plr[myplr];
	hp = p->_pMaxHP >> 8;
	hp = ((hp >> 1) + random_(39, hp)) << 6;
	switch (p->_pClass) {
#ifdef HELLFIRE
	case PC_WARRIOR:
	case PC_BARBARIAN: hp <<= 1; break;
	case PC_ROGUE:
	case PC_MONK:
	case PC_BARD: hp += hp >> 1; break;
#else
	case PC_WARRIOR: hp <<= 1;    break;
	case PC_ROGUE: hp += hp >> 1; break;
#endif
	case PC_SORCERER: break;
	default:
		ASSUME_UNREACHABLE
	}
	PlrIncHp(myplr, hp);
}

static void InvAddMana()
{
	PlayerStruct *p;
	int mana;

	p = &plr[myplr];
	mana = p->_pMaxMana >> 8;
	mana = ((mana >> 1) + random_(40, mana)) << 6;
	switch (p->_pClass) {
	case PC_WARRIOR:				break;
	case PC_SORCERER: mana <<= 1;	break;
#ifdef HELLFIRE
	case PC_BARBARIAN:				break;
	case PC_MONK:
	case PC_BARD:
#endif
	case PC_ROGUE:
		mana += mana >> 1;			break;
	default:
		ASSUME_UNREACHABLE
	}
	PlrIncHp(myplr, mana);
}

BOOL UseInvItem(int cii)
{
	int iv;
	ItemStruct *is;
	BOOL speedlist;
	int pnum = myplr;
	PlayerStruct *p;

	if (plr[pnum]._pHitPoints < (1 << 6))
		return TRUE;
	if (pcurs != CURSOR_HAND)
		return TRUE;
	if (stextflag != STORE_NONE)
		return TRUE;

	if (cii <= INVITEM_INV_LAST) {
		if (cii < INVITEM_INV_FIRST)
			return FALSE;
		iv = cii - INVITEM_INV_FIRST;
		is = &plr[pnum].InvList[iv];
		speedlist = FALSE;
	} else {
		if (talkflag)
			return TRUE;
		iv = cii - INVITEM_BELT_FIRST;
		is = &plr[pnum].SpdList[iv];
		if (is->_itype == ITYPE_NONE)
			return FALSE;
		speedlist = TRUE;
	}

	if (is->_iIdx == IDI_GOLD) {
		StartGoldDrop();
		return TRUE;
	}
	if (is->_iIdx == IDI_MUSHROOM) {
		sfxdelay = 10;
		sfxdnum = sgSFXSets[SFXS_PLR_95][plr[pnum]._pClass];
		return TRUE;
	}
	if (is->_iIdx == IDI_FUNGALTM) {
		PlaySFX(IS_IBOOK);
		sfxdelay = 10;
		sfxdnum = sgSFXSets[SFXS_PLR_29][plr[pnum]._pClass];
		return TRUE;
	}

	if (!AllItemsList[is->_iIdx].iUsable) {
		return FALSE;
	}

	if (!is->_iStatFlag) {
		PlaySFX(sgSFXSets[SFXS_PLR_13][plr[pnum]._pClass]);
		return TRUE;
	}

	if (currlevel == 0 && is->_iMiscId == IMISC_SCROLL
	 && !spelldata[is->_iSpell].sTownSpell) {
		return TRUE;
	}

#ifdef HELLFIRE
	if (currlevel == 0 && is->_iMiscId == IMISC_RUNE) {
		return TRUE;
	}
#endif
	// add sfx
	if (is->_iMiscId == IMISC_BOOK)
		PlaySFX(IS_RBOOK);
	else // if (pnum == myplr)
		PlaySFX(ItemInvSnds[ItemCAnimTbl[is->_iCurs]]);

	// use the item
	switch (is->_iMiscId) {
	case IMISC_HEAL:
	case IMISC_FULLHEAL:
		PlrFillHp(pnum);
		break;
	case IMISC_MANA:
		InvAddMana();
		break;
	case IMISC_FULLMANA:
		PlrFillMana(pnum);
		break;
	case IMISC_REJUV:
		InvAddHp();
		InvAddMana();
		break;
	case IMISC_FULLREJUV:
		PlrFillHp(pnum);
		PlrFillMana(pnum);
		break;
	case IMISC_SCROLL:
#ifdef HELLFIRE
	case IMISC_RUNE:
#endif
		if (spelldata[is->_iSpell].scCurs != CURSOR_NONE) {
			//if (pnum == myplr)
				NewCursor(spelldata[is->_iSpell].scCurs);
			plr[pnum]._pTSpell = is->_iSpell;
			plr[pnum]._pSplFrom = cii;
		} else {
			NetSendCmdLocBParam3(TRUE, CMD_SPELLXY,
				cursmx, cursmy, is->_iSpell, cii, GetSpellLevel(pnum, is->_iSpell));
		}
		return TRUE;
	case IMISC_BOOK:
		p = &plr[pnum];
		p->_pMemSpells |= SPELL_MASK(is->_iSpell);
		if (p->_pSplLvl[is->_iSpell] < MAXSPLLEVEL)
			p->_pSplLvl[is->_iSpell]++;
		PlrIncMana(pnum, spelldata[is->_iSpell].sManaCost << 6);
		//if (pnum == myplr)
			CalcPlrBookVals(pnum);
		break;
	case IMISC_MAPOFDOOM:
		doom_init();
		return TRUE;
	case IMISC_OILQLTY:
	case IMISC_OILZEN:
	case IMISC_OILSTR:
	case IMISC_OILDEX:
	case IMISC_OILVIT:
	case IMISC_OILMAG:
	case IMISC_OILRESIST:
	case IMISC_OILCHANCE:
	case IMISC_OILCLEAN:
		plr[pnum]._pOilFrom = cii;
		//if (pnum != myplr)
		//	return;
		NewCursor(CURSOR_OIL);
		return TRUE;
	case IMISC_SPECELIX:
		ModifyPlrStr(pnum, 3);
		ModifyPlrMag(pnum, 3);
		ModifyPlrDex(pnum, 3);
		ModifyPlrVit(pnum, 3);
		break;
#ifdef HELLFIRE
	case IMISC_NOTE:
		InitQTextMsg(TEXT_BOOK9);
		invflag = FALSE;
		return TRUE;
#endif
	}

	// consume the item
	if (speedlist) {
		RemoveSpdBarItem(pnum, iv);
	} else {
		RemoveInvItem(pnum, iv);
	}
	return TRUE;
}

void CalculateGold(int pnum)
{
	ItemStruct *pi;
	int i, gold;

	gold = 0;
	pi = plr[pnum].InvList;
	for (i = plr[pnum]._pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD)
			gold += pi->_ivalue;
	}

	plr[pnum]._pGold = gold;
}

DEVILUTION_END_NAMESPACE
