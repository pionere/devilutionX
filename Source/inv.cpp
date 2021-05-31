/**
 * @file inv.cpp
 *
 * Implementation of player inventory.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

bool gbInvflag;
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
	pInvCels = LoadFileInMem("Data\\Inv\\Inv.CEL");
	pBeltCels = LoadFileInMem("Data\\Inv\\Belt.CEL");
	gbInvflag = false;
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
	return ICOL_YELLOW;
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

	p = &myplr;
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
			    || (is->_itype != ITYPE_SWORD && is->_itype != ITYPE_MACE))
#endif
			{
				screen_x = RIGHT_PANEL_X + InvRect[SLOTXY_HAND_RIGHT_FIRST].X;
				screen_y = SCREEN_Y + InvRect[SLOTXY_HAND_RIGHT_FIRST].Y + 2 * INV_SLOT_SIZE_PX;
				InvDrawSlotBack(screen_x, screen_y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);
				light_table_index = 0;
				gbCelTransparencyActive = true;

				if (frame_width == INV_SLOT_SIZE_PX)
					screen_x += INV_SLOT_SIZE_PX / 2;
				if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
					screen_y -= INV_SLOT_SIZE_PX / 2;
				CelClippedDrawLightTrans(screen_x, screen_y, cCels, frame, frame_width);
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
		if (ii > 0) { // first (bottom left) slot of an item
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

	is = myplr.SpdList;
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
			if (!sgbControllerActive || gbInvflag)
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
			ff = sfontframe[gbFontTransTbl[fi]];
			PrintChar(screen_x + INV_SLOT_SIZE_PX - sfontkern[ff], screen_y, ff, COL_WHITE);
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
static bool AutoPlace(int pnum, int ii, int sx, int sy, ItemStruct *is)
{
	int i, j, xx, yy;
	bool done;

	done = true;
	if (ii < 0)
		ii = 0;
	yy = 10 * (ii / 10);
	for (j = 0; j < sy && done; j++) {
		if (yy >= NUM_INV_GRID_ELEM) {
			done = false;
		}
		xx = ii % 10;
		for (i = 0; i < sx && done; i++) {
			if (xx >= 10) {
				done = false;
			} else {
				done = plr.InvGrid[xx + yy] == 0;
			}
			xx++;
		}
		yy += 10;
	}
	if (done && is != NULL) {
		NetSendCmdChItem(is, INVITEM_INV_FIRST + plr._pNumInv);
		copy_pod(plr.InvList[plr._pNumInv], *is);
		plr._pNumInv++;
		yy = 10 * (ii / 10);
		for (j = 0; j < sy; j++) {
			xx = ii % 10;
			for (i = 0; i < sx; i++) {
				if (i != 0 || j != sy - 1) {
					plr.InvGrid[xx + yy] = -plr._pNumInv;
				} else {
					plr.InvGrid[xx + yy] = plr._pNumInv;
				}
				xx++;
			}
			yy += 10;
		}
		CalcPlrScrolls(pnum);
	}
	return done;
}

static bool GoldAutoPlace(int pnum, ItemStruct *is)
{
	ItemStruct *pi;
	int free, value, ii, done;

	value = is->_ivalue;
	pi = plr.InvList;
	done = 0;
	for (int i = plr._pNumInv; i > 0 && done < 2; i--, pi++) {
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
		if (plr.InvGrid[i] == 0) {
			ii = plr._pNumInv;
			NetSendCmdChItem(is, INVITEM_INV_FIRST + ii);
			plr._pNumInv = ii + 1;
			plr.InvGrid[i] = ii + 1;
			pi = &plr.InvList[ii];
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
		return false;

	CalculateGold(pnum);
	if (done == 1) {
		// partial placement
		SetGoldItemValue(is, value);
		NewCursor(is->_iCurs + CURSOR_FIRSTITEM);
		return false;
	} else {
		// complete placement
		is->_itype = ITYPE_NONE;
		NewCursor(CURSOR_HAND);
		return true;
	}
}

bool WeaponAutoPlace(int pnum, ItemStruct *is, bool saveflag)
{
	if (!is->_iStatFlag || is->_iClass != ICLASS_WEAPON)
		return false;

	if (plr._pmode > PM_WALK3)
		return false;

	if ((plr._pgfxnum & 0xF) != ANIM_ID_UNARMED && (plr._pgfxnum & 0xF) != ANIM_ID_UNARMED_SHIELD)
#ifdef HELLFIRE
		if (plr._pClass != PC_BARD || ((plr._pgfxnum & 0xF) != ANIM_ID_MACE && (plr._pgfxnum & 0xF) != ANIM_ID_SWORD))
#endif
			return false;

#ifdef HELLFIRE
	if (plr._pClass == PC_MONK)
		return false;
#endif
	if (is->_iLoc != ILOC_TWOHAND
#ifdef HELLFIRE
	    || (plr._pClass == PC_BARBARIAN && (is->_itype == ITYPE_SWORD || is->_itype == ITYPE_MACE))
#endif
	) {
#ifdef HELLFIRE
		if (plr._pClass != PC_BARD)
#endif
		{
			if (plr.InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE && plr.InvBody[INVLOC_HAND_LEFT]._iClass == ICLASS_WEAPON)
				return false;
			if (plr.InvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE && plr.InvBody[INVLOC_HAND_RIGHT]._iClass == ICLASS_WEAPON)
				return false;
		}

		if (plr.InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE) {
			if (saveflag) {
				NetSendCmdChItem(is, INVLOC_HAND_LEFT);
				copy_pod(plr.InvBody[INVLOC_HAND_LEFT], *is);
				CalcPlrInv(pnum, true);
			}
			return true;
		}
		if (plr.InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE && plr.InvBody[INVLOC_HAND_LEFT]._iLoc != ILOC_TWOHAND) {
			if (saveflag) {
				NetSendCmdChItem(is, INVLOC_HAND_RIGHT);
				copy_pod(plr.InvBody[INVLOC_HAND_RIGHT], *is);
				CalcPlrInv(pnum, true);
			}
			return true;
		}
	} else if (plr.InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE && plr.InvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE) {
		if (saveflag) {
			NetSendCmdChItem(is, INVLOC_HAND_LEFT);
			copy_pod(plr.InvBody[INVLOC_HAND_LEFT], *is);
			CalcPlrInv(pnum, true);
		}
		return true;
	}

	return false;
}

bool AutoPlaceBelt(int pnum, ItemStruct *is, bool saveflag)
{
	int i, w, h;

	if (!AllItemsList[is->_iIdx].iUsable || !is->_iStatFlag)
		return false;

	i = is->_iCurs + CURSOR_FIRSTITEM;
	w = InvItemWidth[i] / INV_SLOT_SIZE_PX;
	h = InvItemHeight[i] / INV_SLOT_SIZE_PX;

	if (w != 1 || h != 1)
		return false;

	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr.SpdList[i]._itype == ITYPE_NONE) {
			if (saveflag) {
				NetSendCmdChItem(is, INVITEM_BELT_FIRST + i);
				copy_pod(plr.SpdList[i], *is);
				CalcPlrScrolls(pnum);
				//gbRedrawFlags |= REDRAW_SPEED_BAR;
			}
			return true;
		}
	}
	return false;
}

bool AutoPlaceInv(int pnum, ItemStruct *is, bool saveflag)
{
	ItemStruct *pi;
	int i, w, h;

	i = is->_iCurs + CURSOR_FIRSTITEM;
	w = InvItemWidth[i] / INV_SLOT_SIZE_PX;
	h = InvItemHeight[i] / INV_SLOT_SIZE_PX;

	pi = saveflag ? is : NULL;
	if (w == 1 && h == 1) {
		for (i = 30; i <= 39; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 20; i <= 29; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 10; i <= 19; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 0; i <= 9; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
	}
	if (w == 1 && h == 2) {
		for (i = 29; i >= 20; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 9; i >= 0; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 19; i >= 10; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
	}
	if (w == 1 && h == 3) {
		for (i = 0; i < 20; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
	}
	if (w == 2 && h == 2) {
		for (i = 0; i < 10; i++) {
			if (AutoPlace(pnum, AP2x2Tbl[i], w, h, pi))
				return true;
		}
		for (i = 21; i < 29; i += 2) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 1; i < 9; i += 2) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 10; i < 19; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
	}
	if (w == 2 && h == 3) {
		for (i = 0; i < 9; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
		for (i = 10; i < 19; i++) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
	}
	return false;
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
	bool done;
	int il, cn, it, iv, ig, gt;

	p = &myplr;
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
	done = false;
	if (il == ILOC_UNEQUIPABLE) {
		done = true;
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
					done = false;
				xx = (ii % 10) - ((sx - 1) >> 1);
				if (xx < 0)
					xx = 0;
				for (i = 0; i < sx && done; i++) {
					if (xx >= 10) {
						done = false;
					} else {
						iv = p->InvGrid[xx + yy];
						if (iv != 0) {
							if (iv < 0)
								iv = -iv;
							if (it != 0) {
								if (it != iv)
									done = false;
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
		done = true;
	} else if (il == ILOC_ONEHAND && holditem->_iLoc == ILOC_TWOHAND) {
		il = ILOC_TWOHAND;
		done = true;
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
		NetSendCmdChItem(holditem, INVLOC_HEAD);
		is = &p->InvBody[INVLOC_HEAD];
		if (is->_itype == ITYPE_NONE)
			copy_pod(*is, *holditem);
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_RING:
		if (r == SLOTXY_RING_LEFT) {
			NetSendCmdChItem(holditem, INVLOC_RING_LEFT);
			is = &p->InvBody[INVLOC_RING_LEFT];
			if (is->_itype == ITYPE_NONE)
				copy_pod(*is, *holditem);
			else
				cn = SwapItem(is, holditem);
		} else {
			NetSendCmdChItem(holditem, INVLOC_RING_RIGHT);
			is = &p->InvBody[INVLOC_RING_RIGHT];
			if (is->_itype == ITYPE_NONE)
				copy_pod(*is, *holditem);
			else
				cn = SwapItem(is, holditem);
		}
		break;
	case ILOC_AMULET:
		NetSendCmdChItem(holditem, INVLOC_AMULET);
		is = &p->InvBody[INVLOC_AMULET];
		if (is->_itype == ITYPE_NONE)
			copy_pod(*is, *holditem);
		else
			cn = SwapItem(is, holditem);
		break;
	case ILOC_ONEHAND:
		is = &p->InvBody[INVLOC_HAND_LEFT];
		wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		if (holditem->_iClass == ICLASS_WEAPON) {
			// place a weapon
			if (is->_itype == ITYPE_NONE) {
				// always place the weapon in the left hand if possible
				NetSendCmdChItem(holditem, INVLOC_HAND_LEFT);
				copy_pod(*is, *holditem);
			} else {
				if (r <= SLOTXY_HAND_LEFT_LAST) {
					// in the left hand - replace the current item
					NetSendCmdChItem(holditem, INVLOC_HAND_LEFT);
					cn = SwapItem(is, holditem);
				} else {
					// in the right hand
					if (/*is->_itype != ITYPE_NONE &&*/ is->_iLoc == ILOC_TWOHAND) {
						// replace two-handed weapons and place the weapon in the left hand
						NetSendCmdChItem(holditem, INVLOC_HAND_LEFT);
						cn = SwapItem(is, holditem);
					} else if (wRight->_itype != ITYPE_NONE) {
						// replace item in the right hand
						NetSendCmdChItem(holditem, INVLOC_HAND_RIGHT);
						cn = SwapItem(wRight, holditem);
					} else {
						// place weapon in the right hand
						NetSendCmdChItem(holditem, INVLOC_HAND_RIGHT);
						copy_pod(*wRight, *holditem);
					}
				}
			}
		} else {
			// place a shield
			if (wRight->_itype != ITYPE_NONE) {
				// replace if there was something
				NetSendCmdChItem(holditem, INVLOC_HAND_RIGHT);
				cn = SwapItem(wRight, holditem);
			} else if (is->_itype != ITYPE_NONE && is->_iLoc == ILOC_TWOHAND) {
				// pick two-handed weapons and place the shield in the right hand
				NetSendCmdDelItem(INVLOC_HAND_LEFT);
				NetSendCmdChItem(holditem, INVLOC_HAND_RIGHT);
				SwapItem(wRight, is);
				cn = SwapItem(wRight, holditem);
			} else {
				// place the shield in the right hand
				NetSendCmdChItem(holditem, INVLOC_HAND_RIGHT);
				copy_pod(*wRight, *holditem);
			}
		}
		break;
	case ILOC_TWOHAND:
		is = &p->InvBody[INVLOC_HAND_LEFT];
		wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		if (is->_itype != ITYPE_NONE && wRight->_itype != ITYPE_NONE) {
			if (wRight->_itype != ITYPE_SHIELD)
				wRight = is;
			if (!AutoPlaceInv(mypnum, wRight, true))
				return;

			wRight->_itype = ITYPE_NONE;
			wRight = &p->InvBody[INVLOC_HAND_RIGHT];
		}
		NetSendCmdDelItem(INVLOC_HAND_RIGHT);

		if (is->_itype != ITYPE_NONE || wRight->_itype != ITYPE_NONE) {
			NetSendCmdChItem(holditem, INVLOC_HAND_LEFT);
			if (is->_itype == ITYPE_NONE)
				SwapItem(is, wRight);
			cn = SwapItem(is, holditem);
		} else {
			NetSendCmdChItem(holditem, INVLOC_HAND_LEFT);
			copy_pod(*is, *holditem);
		}
		break;
	case ILOC_ARMOR:
		NetSendCmdChItem(holditem, INVLOC_CHEST);
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
				NetSendCmdChItem(holditem, INVITEM_INV_FIRST + il);
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
				il = p->_pNumInv;
				NetSendCmdChItem(holditem, INVITEM_INV_FIRST + il);
				copy_pod(p->InvList[il], *holditem);
				p->_pNumInv++;
				it = p->_pNumInv;
			} else {
				il = it - 1;
				// NetSendCmdDelItem(INVITEM_INV_FIRST + il);
				NetSendCmdChItem(holditem, INVITEM_INV_FIRST + il);
				if (holditem->_itype == ITYPE_GOLD)
					p->_pGold += holditem->_ivalue;
				cn = SwapItem(&p->InvList[il], holditem);
				if (holditem->_itype == ITYPE_GOLD)
					CalculateGold(mypnum);
				for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
					if (abs(p->InvGrid[i]) == it)
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
	CalcPlrInv(mypnum, true);
	if (cn == CURSOR_HAND)
		SetCursorPos(MouseX + (cursW >> 1), MouseY + (cursH >> 1));
	NewCursor(cn);
}

static void CheckBeltPaste()
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
		return; // FALSE;

	holditem = &myplr.HoldItem;

	// BUGFIX: TODO why is _iLoc not set to ILOC_BELT?
	if (holditem->_iLoc != ILOC_BELT
	 && (holditem->_iLoc != ILOC_UNEQUIPABLE
		 || cursW != INV_SLOT_SIZE_PX || cursH != INV_SLOT_SIZE_PX
		 || !AllItemsList[holditem->_iIdx].iUsable)) {
		return; // TRUE;
	}
	
	PlaySFX(ItemInvSnds[ItemCAnimTbl[holditem->_iCurs]]);

	r -= SLOTXY_BELT_FIRST;
	NetSendCmdChItem(holditem, INVITEM_BELT_FIRST + r);

	is = &myplr.SpdList[r];
	cn = CURSOR_HAND;
	if (is->_itype == ITYPE_NONE) {
		copy_pod(*is, *holditem);
	} else {
		cn = SwapItem(is, holditem);
	}
	//gbRedrawFlags |= REDRAW_SPEED_BAR;
	CalcPlrScrolls(mypnum);
	//CalcPlrInv(mypnum, true);
	if (cn == CURSOR_HAND)
		SetCursorPos(MouseX + (cursW >> 1), MouseY + (cursH >> 1));
	NewCursor(cn);
	//return TRUE;
}

void SyncPlrItemChange(int pnum, BYTE bLoc, int ii)
{
	ItemStruct *is;

	is = &items[ii];
	if (bLoc < INVITEM_INV_FIRST) {
		// body item
		copy_pod(plr.InvBody[bLoc], *is);
		CalcPlrInv(pnum, true);
		return;
	}
	ItemStatOk(pnum, is);
	if (bLoc < INVITEM_BELT_FIRST) {
		// inv item
		bLoc -= INVITEM_INV_FIRST;
		copy_pod(plr.InvList[bLoc], *is);
		if (bLoc >= plr._pNumInv) {
			// place a new item
			assert(bLoc == plr._pNumInv);
			plr._pNumInv++;
		}
		CalcPlrScrolls(pnum);
	} else {
		// belt item
		bLoc -= INVITEM_BELT_FIRST;
		copy_pod(plr.SpdList[bLoc], *is);
		CalcPlrScrolls(pnum);
	}
}

static void CheckInvCut(bool bShift)
{
	PlayerStruct *p;
	ItemStruct *pi;
	char ii;
	int r, i, j;

	p = &myplr;
	if (p->_pmode > PM_WALK3) {
		return; // FALSE;
	}

	r = pcursinvitem;
	if (r == INVITEM_NONE)
		return; // FALSE;

	//p->HoldItem._itype = ITYPE_NONE;

	NetSendCmdDelItem(r);

	switch (r) {
	case INVITEM_HEAD:
		pi = &p->InvBody[INVLOC_HEAD];
		assert(pi->_itype != ITYPE_NONE);
		break;
	case INVITEM_RING_LEFT:
		pi = &p->InvBody[INVLOC_RING_LEFT];
		assert(pi->_itype != ITYPE_NONE);
		break;
	case INVITEM_RING_RIGHT:
		pi = &p->InvBody[INVLOC_RING_RIGHT];
		assert(pi->_itype != ITYPE_NONE);
		break;
	case INVITEM_AMULET:
		pi = &p->InvBody[INVLOC_AMULET];
		assert(pi->_itype != ITYPE_NONE);
		break;
	case INVITEM_HAND_LEFT:
		pi = &p->InvBody[INVLOC_HAND_LEFT];
		assert(pi->_itype != ITYPE_NONE);
		break;
	case INVITEM_HAND_RIGHT:
		pi = &p->InvBody[INVLOC_HAND_RIGHT];
		assert(pi->_itype != ITYPE_NONE);
		break;
	case INVITEM_CHEST:
		pi = &p->InvBody[INVLOC_CHEST];
		assert(pi->_itype != ITYPE_NONE);
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
			if (bShift && AutoPlaceBelt(mypnum, pi, true)) {
				pi->_itype = ITYPE_NONE;
				return; // TRUE;
			}
		} else { // r >= INVITEM_BELT_FIRST
			pi = &p->SpdList[r - INVITEM_BELT_FIRST];
			assert(pi->_itype != ITYPE_NONE);
			//gbRedrawFlags |= REDRAW_SPEED_BAR;
			if (bShift && AutoPlaceInv(mypnum, pi, true)) {
				pi->_itype = ITYPE_NONE;
				return; // TRUE;
			}
		}
		break;
	}

	copy_pod(p->HoldItem, *pi);
	if (pi->_itype == ITYPE_GOLD) {
		CalculateGold(mypnum);
	}
	pi->_itype = ITYPE_NONE;

	CalcPlrInv(mypnum, true);
	ItemStatOk(mypnum, &p->HoldItem);

	PlaySFX(IS_IGRAB);
	NewCursor(p->HoldItem._iCurs + CURSOR_FIRSTITEM);
	SetCursorPos(MouseX - (cursW >> 1), MouseY - (cursH >> 1));
	//return TRUE;
}

/*
 * Remove an item from the player's inventory, equipment or belt.
 * Does not maintain the InvGrid of the player.
 * @param pnum the id of the player
 * @param bLoc inv_item enum value to identify the item
 */
void SyncPlrItemRemove(int pnum, BYTE bLoc)
{
	if (bLoc < INVITEM_INV_FIRST) {
		// body item
		plr.InvBody[bLoc]._itype = ITYPE_NONE;
		CalcPlrInv(pnum, plr._pmode != PM_DEATH);
	} else if (bLoc < INVITEM_BELT_FIRST) {
		// inv item
		bLoc -= INVITEM_INV_FIRST;
		plr._pNumInv--;
		if (bLoc != plr._pNumInv)
			copy_pod(plr.InvList[bLoc], plr.InvList[plr._pNumInv]);
		CalcPlrScrolls(pnum);
	} else {
		// belt item
		bLoc -= INVITEM_BELT_FIRST;
		plr.SpdList[bLoc]._itype = ITYPE_NONE;
		CalcPlrScrolls(pnum);
	}
}

void RemoveInvItem(int pnum, int iv)
{
	int i, j;

	iv++;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (abs(plr.InvGrid[i]) == iv) {
			plr.InvGrid[i] = 0;
		}
	}

	iv--;
	plr._pNumInv--;

	i = plr._pNumInv;
	if (i > 0 && i != iv) {
		copy_pod(plr.InvList[iv], plr.InvList[i]);

		i++;
		iv++;
		for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
			if (plr.InvGrid[j] == i) {
				plr.InvGrid[j] = iv;
			} else if (plr.InvGrid[j] == -i) {
				plr.InvGrid[j] = -iv;
			}
		}
	}

	CalcPlrScrolls(pnum);
}

void RemoveSpdBarItem(int pnum, int iv)
{
	plr.SpdList[iv]._itype = ITYPE_NONE;

	CalcPlrScrolls(pnum);

	//gbRedrawFlags |= REDRAW_SPEED_BAR;
}

/*
 * Remove an item from the player's inventory (box).
 * @param pnum the id of the player
 * @param iv the index of the item
 */
void PlrInvItemRemove(int pnum, int iv)
{
	RemoveInvItem(pnum, iv);
	NetSendCmdDelItem(INVITEM_INV_FIRST + iv);
}

void CheckInvClick(bool bShift)
{
	if (pcurs >= CURSOR_FIRSTITEM) {
		CheckInvPaste();
	} else {
		CheckInvCut(bShift);
	}
}

/**
 * Check for interactions with belt
 */
void CheckBeltClick(bool bShift)
{
	if (pcurs >= CURSOR_FIRSTITEM) {
		/*return*/ CheckBeltPaste();
	} else {
		/*return*/ CheckInvCut(bShift);
	}
}

static void CheckQuestItem(int pnum, ItemStruct *is)
{
	int idx, delay;

	idx = is->_iIdx;
	if (idx == IDI_OPTAMULET) {
		if (quests[Q_BLIND]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_BLIND]._qactive = QUEST_DONE;
		if (pnum == mypnum) {
			NetSendCmdQuest(Q_BLIND, false); // recipient should not matter
		}
		return;
	}
	if (idx == IDI_MUSHROOM) {
		if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 >= QS_MUSHGIVEN)
			return;
		if (quests[Q_MUSHROOM]._qvar2 == SFXS_PLR_95)
			return;
		quests[Q_MUSHROOM]._qvar2 = SFXS_PLR_95;
		delay = 10;
		idx = TEXT_IM_MUSHROOM;
	} else if (idx == IDI_ANVIL) {
		if (quests[Q_ANVIL]._qactive == QUEST_NOTAVAIL)
			return;
		if (quests[Q_ANVIL]._qactive == QUEST_INIT) {
			quests[Q_ANVIL]._qactive = QUEST_ACTIVE;
			quests[Q_ANVIL]._qvar1 = 1;
			if (pnum == mypnum) {
				NetSendCmdQuest(Q_ANVIL, false); // recipient should not matter
			}
		}
		if (!quests[Q_ANVIL]._qlog)
			return;
		delay = 10;
		idx =TEXT_IM_ANVIL;
	} else if (idx == IDI_GLDNELIX) {
		if (quests[Q_VEIL]._qactive != QUEST_ACTIVE)
			return;
		delay = 30;
		idx = TEXT_IM_GLDNELIX;
	} else if (idx == IDI_ROCK) {
		if (quests[Q_ROCK]._qactive == QUEST_NOTAVAIL)
			return;
		if (quests[Q_ROCK]._qactive == QUEST_INIT) {
			quests[Q_ROCK]._qactive = QUEST_ACTIVE;
			quests[Q_ROCK]._qvar1 = 1;
			if (pnum == mypnum) {
				NetSendCmdQuest(Q_ROCK, false); // recipient should not matter
			}
		}
		if (!quests[Q_ROCK]._qlog)
			return;
		delay = 10;
		idx = TEXT_IM_ROCK;
	} else if (idx == IDI_ARMOFVAL) {
		if (quests[Q_BLOOD]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_BLOOD]._qactive = QUEST_DONE;
		delay = 20;
		idx = TEXT_IM_ARMOFVAL;
#ifdef HELLFIRE
	} else if (idx == IDI_MAPOFDOOM) {
		if (quests[Q_GRAVE]._qactive == QUEST_NOTAVAIL)
			return;
		if (quests[Q_GRAVE]._qactive == QUEST_INIT) {
			// quests[Q_GRAVE]._qlog = FALSE;
			quests[Q_GRAVE]._qactive = QUEST_ACTIVE;
			quests[Q_GRAVE]._qvar1 = 1;
			if (pnum == mypnum) {
				NetSendCmdQuest(Q_GRAVE, false); // recipient should not matter
			}
		}
		delay = 10;
		idx = TEXT_IM_MAPOFDOOM;
	} else if (idx == IDI_NOTE1 || idx == IDI_NOTE2 || idx == IDI_NOTE3) {
		int nn, i;
		if ((idx == IDI_NOTE1 || PlrHasItem(pnum, IDI_NOTE1, &nn))
		 && (idx == IDI_NOTE2 || PlrHasItem(pnum, IDI_NOTE2, &nn))
		 && (idx == IDI_NOTE3 || PlrHasItem(pnum, IDI_NOTE3, &nn))) {
			static_assert((int)IDI_NOTE1 + 1 == (int)IDI_NOTE2, "CheckQuestItem requires an ordered IDI_NOTE enum I.");
			static_assert((int)IDI_NOTE2 + 1 == (int)IDI_NOTE3, "CheckQuestItem requires an ordered IDI_NOTE enum II.");
			for (i = IDI_NOTE1; i <= IDI_NOTE3; i++) {
				if (idx != i) {
					PlrHasItem(pnum, i, &nn);
					PlrInvItemRemove(pnum, nn);
				}
			}
			SetItemData(MAXITEMS, IDI_FULLNOTE);
			SetupItem(MAXITEMS);
			copy_pod(*is, items[MAXITEMS]);
			GetItemSeed(is);
			delay = 10;
			idx = TEXT_IM_FULLNOTE;
		} else {
			return;
		}
#endif
	} else {
		return;
	}
	if (pnum == mypnum) {
		sfxdelay = delay;
		sfxdnum = idx;
	}
}

void InvGetItem(int pnum, int ii)
{
	ItemStruct *is;

	is = &items[ii];
	if (dItem[is->_ix][is->_iy] == 0)
		return;
	dItem[is->_ix][is->_iy] = 0;

	if (mypnum == pnum && pcurs >= CURSOR_FIRSTITEM)
		NetSendCmdPItem(true, CMD_SYNCPUTITEM, &plr.HoldItem, plr._px, plr._py);
	// always mask CF_PREGEN to make life of RecreateItem easier later on
	// otherwise this should not have an effect, since the item is already in 'delta'
	is->_iCreateInfo &= ~CF_PREGEN;
	CheckQuestItem(pnum, is);
	ItemStatOk(pnum, is);
	copy_pod(plr.HoldItem, *is);
	NewCursor(plr.HoldItem._iCurs + CURSOR_FIRSTITEM);
	pcursitem = ITEM_NONE;

	DeleteItems(ii);
}

void AutoGetItem(int pnum, int ii)
{
	ItemStruct *is;
	bool done;

	if (pcurs != CURSOR_HAND) {
		return;
	}

	is = &items[ii];
	if (dItem[is->_ix][is->_iy] == 0)
		return;

	// always mask CF_PREGEN to make life of RecreateItem easier later on
	// otherwise this should not have an effect, since the item is already in 'delta'
	is->_iCreateInfo &= ~CF_PREGEN;
	CheckQuestItem(pnum, is);
	ItemStatOk(pnum, is);
	if (is->_itype == ITYPE_GOLD) {
		done = GoldAutoPlace(pnum, is);
	} else {
		done = WeaponAutoPlace(pnum, is, true)
			|| AutoPlaceBelt(pnum, is, true)
			|| AutoPlaceInv(pnum, is, true);
	}
	if (done) {
		dItem[is->_ix][is->_iy] = 0;
		DeleteItems(ii);
	} else {
		if (pnum == mypnum) {
			PlaySFX(sgSFXSets[SFXS_PLR_14][plr._pClass], 3);
		}
		RespawnItem(ii, true);
		NetSendCmdPItem(true, CMD_RESPAWNITEM, is, is->_ix, is->_iy);
		NewCursor(CURSOR_HAND);
	}
}

int FindGetItem(int iseed, WORD wIndex, WORD wCI)
{
	int i, ii;

	for (i = 0; i < numitems; i++) {
		ii = itemactive[i];
		if (items[ii]._iSeed == iseed && items[ii]._iIdx == wIndex && items[ii]._iCreateInfo == wCI)
			return ii;
	}

	return -1;
}

void SyncGetItemIdx(int ii)
{
	ItemStruct *is;

	is = &items[ii];
	dItem[is->_ix][is->_iy] = 0;
	DeleteItems(ii);
}

void SyncGetItemAt(int x, int y, int iseed, WORD wIndex, WORD wCI)
{
	int ii;

	ii = dItem[x][y];
	if (ii != 0) {
		ii--;
		if (items[ii]._iSeed != iseed
		 || items[ii]._iIdx != wIndex
		 || items[ii]._iCreateInfo != wCI) {
			ii = FindGetItem(iseed, wIndex, wCI);
		}
	} else {
		ii = FindGetItem(iseed, wIndex, wCI);
	}

	if (ii != -1) {
		SyncGetItemIdx(ii);
	}
}

bool CanPut(int x, int y)
{
	int oi, oi2;

	if (x < DBORDERX || x >= DBORDERX + DSIZEX || y < DBORDERY || y >= DBORDERY + DSIZEY)
		return false;

	if ((dItem[x][y] | nSolidTable[dPiece[x][y]]) != 0)
		return false;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return false;
	}

	oi = dObject[x + 1][y + 1];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSelFlag != 0)
			return false;
	}

	oi = dObject[x + 1][y];
	if (oi > 0) {
		oi2 = dObject[x][y + 1];
		if (oi2 > 0 && object[oi - 1]._oSelFlag != 0 && object[oi2 - 1]._oSelFlag != 0)
			return false;
	}

	if (currLvl._dType == DTYPE_TOWN)
		if ((dMonster[x][y] | dMonster[x + 1][y + 1]) != 0)
			return false;

	return true;
}

bool FindItemLocation(int sx, int sy, int *dx, int *dy, int rad)
{
	int dir;
	int xx, yy, i, j, k;

	if (sx != *dx || sy != *dy) {
		dir = GetDirection(sx, sy, *dx, *dy);
		*dx = sx + offset_x[dir];
		*dy = sy + offset_y[dir];
		if (CanPut(*dx, *dy))
			return true;

		dir = (dir - 1) & 7;
		*dx = sx + offset_x[dir];
		*dy = sy + offset_y[dir];
		if (CanPut(*dx, *dy))
			return true;

		dir = (dir + 2) & 7;
		*dx = sx + offset_x[dir];
		*dy = sy + offset_y[dir];
		if (CanPut(*dx, *dy))
			return true;

		*dx = sx;
		*dy = sy;
	}

	if (CanPut(*dx, *dy))
		return true;

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
					return true;
				}
			}
		}
	}
	return false;
}

bool DropItem()
{
	int x, y;

	if (numitems >= MAXITEMS)
		return false;

	x = cursmx;
	y = cursmy;
	if (!FindItemLocation(myplr._px, myplr._py, &x, &y, 1))
		return false;

	NetSendCmdPItem(true, CMD_PUTITEM, &myplr.HoldItem, cursmx, cursmy);
	NewCursor(CURSOR_HAND);
	return true;
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

int InvPutItem(int pnum, int x, int y, int ii)
{
	ii = SyncPutItem(pnum, x, y, ii, true);

	if (ii != -1 && pnum == mypnum) {
		NewCursor(CURSOR_HAND);
	}
	return ii;
}

/**
 * Place an item around the given position.
 *
 * @param pnum the id of the player who places the item / initiated the item placement
 * @param x tile coordinate to place the item
 * @param y tile coordinate to place the item
 * @param ii the index of the item to place
 * @param plrAround true: the item should be placed around the player
 *                 false: the item should be placed around x:y
 */
int SyncPutItem(int pnum, int x, int y, int ii, bool plrAround)
{
	int xx, yy;
	ItemStruct *is;

	// assert(plr.plrlevel == currLvl._dLevelIdx);
	if (numitems >= MAXITEMS)
		return -1;

	if (plrAround) {
		xx = plr._px;
		yy = plr._py;
	} else {
		xx = x;
		yy = y;
	}
	if (!FindItemLocation(xx, yy, &x, &y, DSIZEX / 2))
		return -1;

	is = &items[ii];

	ii = itemavail[0];
	dItem[x][y] = ii + 1;
	itemavail[0] = itemavail[MAXITEMS - (numitems + 1)];
	itemactive[numitems] = ii;
	copy_pod(items[ii], *is);
	items[ii]._ix = x;
	items[ii]._iy = y;
	RespawnItem(ii, true);
	numitems++;
	return ii;
}

BYTE CheckInvBelt()
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
	if (r < MAXBELTITEMS && myplr.SpdList[r]._itype != ITYPE_NONE) {
		//gbRedrawFlags |= REDRAW_SPEED_BAR;
		return INVITEM_BELT_FIRST + r;
	}

	return INVITEM_NONE;
}

BYTE CheckInvItem()
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

	p = &myplr;
	switch (InvSlotTbl[r]) {
	case SLOT_HEAD:
		rv = INVITEM_HEAD;
		static_assert((int)INVLOC_HEAD == (int)INVITEM_HEAD, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_RING_LEFT:
		rv = INVITEM_RING_LEFT;
		static_assert((int)INVLOC_RING_LEFT == (int)INVITEM_RING_LEFT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_RING_RIGHT:
		rv = INVITEM_RING_RIGHT;
		static_assert((int)INVLOC_RING_RIGHT == (int)INVITEM_RING_RIGHT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_AMULET:
		rv = INVITEM_AMULET;
		static_assert((int)INVLOC_AMULET == (int)INVITEM_AMULET, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_HAND_LEFT:
		rv = INVITEM_HAND_LEFT;
		static_assert((int)INVLOC_HAND_LEFT == (int)INVITEM_HAND_LEFT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_HAND_RIGHT:
		rv = INVITEM_HAND_LEFT;
		static_assert((int)INVLOC_HAND_LEFT == (int)INVITEM_HAND_LEFT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		pi = &p->InvBody[rv];
#ifdef HELLFIRE
		if (pi->_itype == ITYPE_NONE || pi->_iLoc != ILOC_TWOHAND
		    || (p->_pClass == PC_BARBARIAN && (pi->_itype == ITYPE_SWORD || pi->_itype == ITYPE_MACE))) {
#else
		if (pi->_itype == ITYPE_NONE || pi->_iLoc != ILOC_TWOHAND) {
#endif
			rv = INVITEM_HAND_RIGHT;
			static_assert((int)INVLOC_HAND_RIGHT == (int)INVITEM_HAND_RIGHT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		}
		break;
	case SLOT_CHEST:
		rv = INVITEM_CHEST;
		static_assert((int)INVLOC_CHEST == (int)INVITEM_CHEST, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		pi = &p->InvBody[rv];
		break;
	case SLOT_STORAGE:
		r = p->InvGrid[r - SLOTXY_INV_FIRST];
		if (r == 0)
			return INVITEM_NONE;
		r = abs(r) - 1;
		assert(p->InvList[r]._itype != ITYPE_NONE);
		return INVITEM_INV_FIRST + r;
	case SLOT_BELT:
		return INVITEM_NONE;
	default:
		ASSUME_UNREACHABLE
	}

	pi = &p->InvBody[rv];
	if (pi->_itype == ITYPE_NONE)
		return INVITEM_NONE;

	return rv;
}

static void StartGoldDrop()
{
	if (gbTalkflag || myplr._pmode != PM_STAND)
		return;
	initialDropGoldIndex = pcursinvitem;
	assert(pcursinvitem >= INVITEM_INV_FIRST && pcursinvitem <= INVITEM_INV_LAST);
	initialDropGoldValue = myplr.InvList[pcursinvitem - INVITEM_INV_FIRST]._ivalue;
	gbDropGoldFlag = true;
	dropGoldValue = 0;
}

static void InvAddHp()
{
	PlayerStruct *p;
	int hp;

	p = &myplr;
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
	PlrIncHp(mypnum, hp);
}

static void InvAddMana()
{
	PlayerStruct *p;
	int mana;

	p = &myplr;
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
	PlrIncMana(mypnum, mana);
}

bool InvUseItem(int cii)
{
	int iv, sn;
	ItemStruct *is;
	bool speedlist;
	int pnum = mypnum;

	if (plr._pHitPoints < (1 << 6))
		return true;
	if (pcurs != CURSOR_HAND)
		return true;
	if (stextflag != STORE_NONE)
		return true;

	if (cii < INVITEM_INV_FIRST)
		return false;
	if (cii <= INVITEM_INV_LAST) {
		iv = cii - INVITEM_INV_FIRST;
		is = &plr.InvList[iv];
		speedlist = false;
	} else {
		iv = cii - INVITEM_BELT_FIRST;
		is = &plr.SpdList[iv];
		speedlist = true;
	}

	if (is->_itype == ITYPE_NONE)
		return false;
	if (is->_iIdx == IDI_GOLD) {
		StartGoldDrop();
		return true;
	}
	if (is->_iIdx == IDI_MUSHROOM) {
		sfxdelay = 10;
		sfxdnum = TEXT_IM_MUSHROOM;
		return true;
	}
	if (is->_iIdx == IDI_FUNGALTM) {
		PlaySFX(IS_IBOOK);
		sfxdelay = 10;
		sfxdnum = TEXT_IM_FUNGALTM;
		return true;
	}

	if (!AllItemsList[is->_iIdx].iUsable) {
		return false;
	}

	if (!is->_iStatFlag) {
		PlaySFX(sgSFXSets[SFXS_PLR_13][plr._pClass]);
		return true;
	}

	if (currLvl._dType == DTYPE_TOWN
#ifdef HELLFIRE
	 && (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE)
#else
	 && is->_iMiscId == IMISC_SCROLL
#endif
		&& (spelldata[is->_iSpell].sFlags & SFLAG_DUNGEON) == SFLAG_DUNGEON) {
		return true;
	}

	// add sfx
	if (is->_iMiscId == IMISC_BOOK)
		PlaySFX(IS_RBOOK);
	else
		PlaySFX(ItemInvSnds[ItemCAnimTbl[is->_iCurs]]);

	// use the item
	switch (is->_iMiscId) {
	case IMISC_HEAL:
	case IMISC_FULLHEAL:
	case IMISC_MANA:
	case IMISC_FULLMANA:
	case IMISC_REJUV:
	case IMISC_FULLREJUV:
	case IMISC_SPECELIX:
		break;
	case IMISC_SCROLL:
#ifdef HELLFIRE
	case IMISC_RUNE:
#endif
		sn = is->_iSpell;
		if (spelldata[sn].scCurs != CURSOR_NONE) {
			NewCursor(spelldata[sn].scCurs);
			plr._pTSpell = sn;
			plr._pTSplFrom = cii;
		} else {
			NetSendCmdLocSkill(cursmx, cursmy, sn, cii, GetSpellLevel(pnum, sn));
		}
		return true;
	case IMISC_BOOK:
		sn = is->_iSpell;
		//plr._pMemSkills |= SPELL_MASK(sn);
		plr._pSkillExp[sn] += SkillExpLvlsTbl[0];
		if (plr._pSkillExp[sn] > SkillExpLvlsTbl[MAXSPLLEVEL] - 1) {
			plr._pSkillExp[sn] = SkillExpLvlsTbl[MAXSPLLEVEL] - 1;
		}

		if (plr._pSkillExp[sn] >= SkillExpLvlsTbl[plr._pSkillLvl[sn]]) {
			plr._pSkillLvl[sn]++;
			NetSendCmdBParam2(false, CMD_PLRSKILLLVL, sn, plr._pSkillLvl[sn]);
		}
		// PlrIncMana(pnum, spelldata[sn].sManaCost << 6);
		//CalcPlrSpells(pnum);
		break;
	case IMISC_MAPOFDOOM:
		doom_init();
		return true;
	case IMISC_OILQLTY:
	case IMISC_OILZEN:
	case IMISC_OILSTR:
	case IMISC_OILDEX:
	case IMISC_OILVIT:
	case IMISC_OILMAG:
	case IMISC_OILRESIST:
	case IMISC_OILCHANCE:
	case IMISC_OILCLEAN:
		plr._pOilFrom = cii;
		NewCursor(CURSOR_OIL);
		return true;
#ifdef HELLFIRE
	case IMISC_NOTE:
		InitQTextMsg(TEXT_BOOK9);
		gbInvflag = false;
		return true;
#endif
	default:
		ASSUME_UNREACHABLE
	}

	if (speedlist) {
		iv += INVITEM_BELT_FIRST;
	} else {
		iv += INVITEM_INV_FIRST;
	}
	NetSendCmdBParam1(true, CMD_USEPLRITEM, iv);
	return true;
}

void SyncUseItem(int pnum, int cii)
{
	ItemStruct *is;
	int iv, sn;
	bool speedlist;

	if (plr._pHitPoints < (1 << 6))
		return;

	if (cii < INVITEM_INV_FIRST) {
		is = &plr.InvBody[cii];
		assert(is->_itype != ITYPE_NONE);
		// assert(is->_iSpell == sn); TODO validate sn?
		assert(is->_iStatFlag);
		assert(is->_iCharges > 0);
		is->_iCharges--;
		CalcPlrStaff(pnum);
		return;
	}
	if (cii <= INVITEM_INV_LAST) {
		iv = cii - INVITEM_INV_FIRST;
		is = &plr.InvList[iv];
		speedlist = false;
	} else {
		iv = cii - INVITEM_BELT_FIRST;
		is = &plr.SpdList[iv];
		speedlist = true;
	}

	if (is->_itype == ITYPE_NONE)
		return;

	if (!AllItemsList[is->_iIdx].iUsable)
		return;

	if (!is->_iStatFlag)
		return;

	/* TODO validate packages from internet
	if (AllLevels[plr.plrlevel].dType == DTYPE_TOWN
#ifdef HELLFIRE
	 && (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE)
#else
	 && is->_iMiscId == IMISC_SCROLL
#endif
		&& (spelldata[is->_iSpell].sFlags & SFLAG_DUNGEON) == SFLAG_DUNGEON) {
		return;
	}*/

	// use the item
	SetRndSeed(is->_iSeed);
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
		break;
	case IMISC_BOOK:
		sn = is->_iSpell;
		plr._pMemSkills |= SPELL_MASK(sn);
		PlrIncMana(pnum, spelldata[sn].sManaCost << 6);
		// CalcPlrSpells(pnum);
		break;
	case IMISC_SPECELIX:
		RestorePlrHpVit(pnum);
		break;
	default:
		// should not happen under normal circumstances, but safer to just return
		// to avoid further desync of items
		// ASSUME_UNREACHABLE
		return;
	}

	// consume the item
	if (speedlist) {
		RemoveSpdBarItem(pnum, iv);
	} else {
		RemoveInvItem(pnum, iv);
	}
}

void CalculateGold(int pnum)
{
	ItemStruct *pi;
	int i, gold;

	gold = 0;
	pi = plr.InvList;
	for (i = plr._pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD)
			gold += pi->_ivalue;
	}

	plr._pGold = gold;
}

DEVILUTION_END_NAMESPACE
