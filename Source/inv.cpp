/**
 * @file inv.cpp
 *
 * Implementation of player inventory.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

bool gbInvflag;
CmdSkillUse gbTSkillUse; // the spell to cast and its source after the target is selected

CelImageBuf* pInvCels;
CelImageBuf* pBeltCels;

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
 */
const InvXY InvRect[NUM_XY_SLOTS] = {
	// clang-format off
	//  X,   Y
	{ 120,                    30 },                       // helmet
	{ 120 + INV_SLOT_SIZE_PX, 30 },                       // helmet
	{ 120,                    30 +    INV_SLOT_SIZE_PX }, // helmet
	{ 120 + INV_SLOT_SIZE_PX, 30 +    INV_SLOT_SIZE_PX }, // helmet
	{  60, 172 }, // left ring
	{ 205, 172 }, // right ring
	{ 186,  44 }, // amulet
	{  47,                    82 },                      // left hand
	{  47 + INV_SLOT_SIZE_PX, 82 },                      // left hand
	{  47,                    82 +   INV_SLOT_SIZE_PX }, // left hand
	{  47 + INV_SLOT_SIZE_PX, 82 +   INV_SLOT_SIZE_PX }, // left hand
	{  47,                    82 + 2*INV_SLOT_SIZE_PX }, // left hand
	{  47 + INV_SLOT_SIZE_PX, 82 + 2*INV_SLOT_SIZE_PX }, // left hand
	{ 192,                    82 },                      // right hand
	{ 192 + INV_SLOT_SIZE_PX, 82 },                      // right hand
	{ 192,                    82 +   INV_SLOT_SIZE_PX }, // right hand
	{ 192 + INV_SLOT_SIZE_PX, 82 +   INV_SLOT_SIZE_PX }, // right hand
	{ 192,                    82 + 2*INV_SLOT_SIZE_PX }, // right hand
	{ 192 + INV_SLOT_SIZE_PX, 82 + 2*INV_SLOT_SIZE_PX }, // right hand
	{ 120,                    94 },                      // chest
	{ 120 + INV_SLOT_SIZE_PX, 94 },                      // chest
	{ 120,                    94 +   INV_SLOT_SIZE_PX }, // chest
	{ 120 + INV_SLOT_SIZE_PX, 94 +   INV_SLOT_SIZE_PX }, // chest
	{ 120,                    94 + 2*INV_SLOT_SIZE_PX }, // chest
	{ 120 + INV_SLOT_SIZE_PX, 94 + 2*INV_SLOT_SIZE_PX }, // chest
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
	{ 1                   ,  1 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 1                   ,  2 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 1                   ,  3 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 1                   ,  4 * (INV_SLOT_SIZE_PX + 1) }, // belt column 1
	{ 2 + INV_SLOT_SIZE_PX,  1 * (INV_SLOT_SIZE_PX + 1) }, // belt column 2
	{ 2 + INV_SLOT_SIZE_PX,  2 * (INV_SLOT_SIZE_PX + 1) }, // belt column 2
	{ 2 + INV_SLOT_SIZE_PX,  3 * (INV_SLOT_SIZE_PX + 1) }, // belt column 2
	{ 2 + INV_SLOT_SIZE_PX,  4 * (INV_SLOT_SIZE_PX + 1) }  // belt column 2
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

void FreeInvGFX()
{
	MemFreeDbg(pInvCels);
	MemFreeDbg(pBeltCels);
}

void InitInv()
{
	assert(pInvCels == NULL);
	pInvCels = CelLoadImage("Data\\Inv\\Inv.CEL", SPANEL_WIDTH);
	assert(pBeltCels == NULL);
	pBeltCels = CelLoadImage("Data\\Inv\\Belt.CEL", BELT_WIDTH);
	//gbInvflag = false;
	//gbTSkillUse = { SPL_NULL, 0 };
}

static void InvDrawSlotBack(int X, int Y, int W, int H)
{
	BYTE* dst;

	// assert(gpBuffer != NULL);

	dst = &gpBuffer[BUFFERXY(X, Y)];

	int wdt, hgt;
	BYTE pix;

	for (hgt = H; hgt; hgt--, dst -= BUFFER_WIDTH + W) {
		for (wdt = W; wdt; wdt--, dst++) {
			pix = *dst;
			if (pix >= PAL16_BLUE) {
				if (pix <= PAL16_BLUE + 15)
					; // *dst -= PAL16_BLUE - PAL16_BEIGE;
				else if (pix >= PAL16_GRAY)
					*dst -= PAL16_GRAY - PAL16_BEIGE;
			}
		}
	}
}

/**
 * @brief Render the inventory panel to the back buffer
 */
void DrawInv()
{
	ItemStruct *is, *pi;
	int pnum, frame, frame_width, screen_x, screen_y, dx, dy, i;
	BYTE* cCels;

	screen_x = SCREEN_X + gnWndInvX;
	screen_y = SCREEN_Y + gnWndInvY;
	CelDraw(screen_x, screen_y + SPANEL_HEIGHT - 1, pInvCels, 1);

	cCels = pCursCels;

	pnum = mypnum;
	pi = /*!INVIDX_VALID(pcursinvitem) ? NULL :*/ PlrItem(pnum, pcursinvitem);
	is = &plr._pInvBody[INVLOC_HEAD];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_HEAD_FIRST].X, screen_y + InvRect[SLOTXY_HEAD_LAST].Y, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_HEAD_FIRST].X, screen_y + InvRect[SLOTXY_HEAD_LAST].Y, cCels, frame, frame_width);
	}

	is = &plr._pInvBody[INVLOC_RING_LEFT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_RING_LEFT].X, screen_y + InvRect[SLOTXY_RING_LEFT].Y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_RING_LEFT].X, screen_y + InvRect[SLOTXY_RING_LEFT].Y, cCels, frame, frame_width);
	}

	is = &plr._pInvBody[INVLOC_RING_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_RING_RIGHT].X, screen_y + InvRect[SLOTXY_RING_RIGHT].Y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_RING_RIGHT].X, screen_y + InvRect[SLOTXY_RING_RIGHT].Y, cCels, frame, frame_width);
	}

	is = &plr._pInvBody[INVLOC_AMULET];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_AMULET].X, screen_y + InvRect[SLOTXY_AMULET].Y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_AMULET].X, screen_y + InvRect[SLOTXY_AMULET].Y, cCels, frame, frame_width);
	}

	is = &plr._pInvBody[INVLOC_HAND_LEFT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_HAND_LEFT_FIRST].X, screen_y + InvRect[SLOTXY_HAND_LEFT_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		dx = 0;
		dy = 0;
		if (frame_width == INV_SLOT_SIZE_PX)
			dx += INV_SLOT_SIZE_PX / 2;
		if (InvItemHeight[frame] != (3 * INV_SLOT_SIZE_PX))
			dy -= INV_SLOT_SIZE_PX / 2;

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_HAND_LEFT_FIRST].X + dx, screen_y + InvRect[SLOTXY_HAND_LEFT_LAST].Y + dy, cCels, frame, frame_width);

		if (TWOHAND_WIELD(&plr, is)) {
				InvDrawSlotBack(screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);
				light_trn_index = 0;
				gbCelTransparencyActive = true;

				dx = 0;
				dy = 0;
				if (frame_width == INV_SLOT_SIZE_PX)
					dx += INV_SLOT_SIZE_PX / 2;
				if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
					dy -= INV_SLOT_SIZE_PX / 2;
				CelClippedDrawLightTrans(screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X + dx, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y + dy, cCels, frame, frame_width);
		}
	}

	is = &plr._pInvBody[INVLOC_HAND_RIGHT];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];
		// calc item offsets for weapons smaller than 2x3 slots
		dx = 0;
		dy = 0;
		if (frame_width == INV_SLOT_SIZE_PX)
			dx += INV_SLOT_SIZE_PX / 2;
		if (InvItemHeight[frame] != 3 * INV_SLOT_SIZE_PX)
			dy -= INV_SLOT_SIZE_PX / 2;

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_HAND_RIGHT_FIRST].X + dx, screen_y + InvRect[SLOTXY_HAND_RIGHT_LAST].Y + dy, cCels, frame, frame_width);
	}

	is = &plr._pInvBody[INVLOC_CHEST];
	if (is->_itype != ITYPE_NONE) {
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_CHEST_FIRST].X, screen_y + InvRect[SLOTXY_CHEST_LAST].Y, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX);

		frame = is->_iCurs + CURSOR_FIRSTITEM;
		frame_width = InvItemWidth[frame];

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_CHEST_FIRST].X, screen_y + InvRect[SLOTXY_CHEST_LAST].Y, cCels, frame, frame_width);
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (plr._pInvList[i]._itype != ITYPE_NONE) {
			InvDrawSlotBack(
			    screen_x + InvRect[i + SLOTXY_INV_FIRST].X,
			    screen_y + InvRect[i + SLOTXY_INV_FIRST].Y,
			    INV_SLOT_SIZE_PX,
			    INV_SLOT_SIZE_PX);
		}
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		is = &plr._pInvList[i];
		if (is->_itype != ITYPE_NONE && is->_itype != ITYPE_PLACEHOLDER) {
			// first (bottom left) slot of an item
			frame = is->_iCurs + CURSOR_FIRSTITEM;
			frame_width = InvItemWidth[frame];

			scrollrt_draw_item(is, pi == is, screen_x + InvRect[i + SLOTXY_INV_FIRST].X, screen_y + InvRect[i + SLOTXY_INV_FIRST].Y, cCels, frame, frame_width);
		}
	}
}

void DrawInvBelt()
{
	ItemStruct *is, *pi;
	int pnum, i, frame, frame_width, screen_x, screen_y;
	BYTE fi, ff;
	BYTE* cCels;

	screen_x = SCREEN_X + gnWndBeltX;
	screen_y = SCREEN_Y + gnWndBeltY;
	CelDraw(screen_x, screen_y + BELT_HEIGHT - 1, pBeltCels, 1);

	pnum = mypnum;
	pi = NULL;
	if (INVIDX_VALID(pcursinvitem))
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		if (!sgbControllerActive || gbInvflag)
#endif
			pi = PlrItem(pnum, pcursinvitem);
	is = plr._pSpdList;
	cCels = pCursCels;
	for (i = 0; i < MAXBELTITEMS; i++, is++) {
		if (is->_itype == ITYPE_NONE) {
			continue;
		}
		InvDrawSlotBack(screen_x + InvRect[SLOTXY_BELT_FIRST + i].X, screen_y + InvRect[SLOTXY_BELT_FIRST + i].Y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);
		frame = is->_iCurs + CURSOR_FIRSTITEM;
		assert(InvItemWidth[frame] == INV_SLOT_SIZE_PX);
		frame_width = INV_SLOT_SIZE_PX;

		scrollrt_draw_item(is, pi == is, screen_x + InvRect[SLOTXY_BELT_FIRST + i].X, screen_y + InvRect[SLOTXY_BELT_FIRST + i].Y, cCels, frame, frame_width);

		if (is->_iStatFlag && is->_iUsable) {
			fi = i + 49; // '1' + i;
			// PrintSmallChar right adjusted
			ff = gbStdFontFrame[fi];
			// PrintSmallChar(screen_x + InvRect[SLOTXY_BELT_FIRST + i].X + INV_SLOT_SIZE_PX - smallFontWidth[ff], screen_y + InvRect[SLOTXY_BELT_FIRST + i].Y, fi, COL_WHITE);
			PrintSmallColorChar(screen_x + InvRect[SLOTXY_BELT_FIRST + i].X + INV_SLOT_SIZE_PX - smallFontWidth[ff], screen_y + InvRect[SLOTXY_BELT_FIRST + i].Y, ff, COL_WHITE);
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
static bool AutoPlace(int pnum, int ii, int sx, int sy, ItemStruct* is)
{
	ItemStruct* pi;
	int i, j, xx, it;
	bool done;

	done = true;
	if (ii < 0)
		ii = 0;
	static_assert(NUM_INV_GRID_ELEM % 10 == 0, "AutoPlace expects 10 slot per row.");
	if ((ii % 10) + sx > 10
	 || (ii / 10) + sy > NUM_INV_GRID_ELEM / 10)
		done = false;

	xx = ii;
	for (j = 0; j < sy && done; j++) {
		for (i = 0; i < sx && done; i++, xx++) {
			done = plr._pInvList[xx]._itype == ITYPE_NONE;
		}
		xx += 10 - sx;
	}
	if (done && is != NULL) {
		it = ii + (sy - 1) * 10; // start from the bottom-left corner
		xx = it;
		copy_pod(plr._pInvList[xx], *is);
		for (j = 0; j < sy; j++) {
			for (i = 0; i < sx; i++, xx++) {
				pi = &plr._pInvList[xx];
				if (i != 0 || j != 0) {
					pi->_itype = ITYPE_PLACEHOLDER;
					pi->_iPHolder = it;
				}
			}
			xx -= 10 + sx;
		}
		CalcPlrScrolls(pnum);
	}
	return done;
}

/*
 * Create a new item-seed based on seed of the item and the current turn-id
 */
static int GetNewItemSeed(ItemStruct* is)
{
	int seed = (gdwGameLogicTurn >> 8) | (gdwGameLogicTurn << 24); // _rotr(gdwGameLogicTurn, 8)

	seed ^= is->_iSeed;
	SetRndSeed(seed);
	return NextRndSeed();
}

static bool GoldAutoPlace(int pnum, ItemStruct* is)
{
	ItemStruct* pi;
	int free, value, done;

	value = is->_ivalue;
	pi = plr._pInvList;
	done = 0;
	for (int i = NUM_INV_GRID_ELEM; i > 0 && done < 2; i--, pi++) {
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
	pi = plr._pInvList;
	for (int i = NUM_INV_GRID_ELEM; i > 0 && done < 2; i--, pi++) {
		if (pi->_itype == ITYPE_NONE) {
			copy_pod(*pi, *is);
			value -= GOLD_MAX_LIMIT;
			if (value <= 0) {
				SetGoldItemValue(pi, value + GOLD_MAX_LIMIT);
				done |= 2;
			} else {
				SetGoldItemValue(pi, GOLD_MAX_LIMIT);
				is->_iSeed = GetNewItemSeed(pi);
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
		return false;
	} else {
		// complete placement
		is->_itype = ITYPE_NONE;
		return true;
	}
}

/* commented out because _pmode might be out of sync in multiplayer games
bool WeaponAutoPlace(int pnum, ItemStruct* is, bool saveflag)
{
	if (!is->_iStatFlag || is->_iClass != ICLASS_WEAPON)
		return false;

	if (plr._pmode > PM_WALK2)
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
			if (plr._pInvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE && plr._pInvBody[INVLOC_HAND_LEFT]._iClass == ICLASS_WEAPON)
				return false;
			if (plr._pInvBody[INVLOC_HAND_RIGHT]._itype != ITYPE_NONE && plr._pInvBody[INVLOC_HAND_RIGHT]._iClass == ICLASS_WEAPON)
				return false;
		}

		if (plr._pInvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE) {
			if (saveflag) {
				copy_pod(plr._pInvBody[INVLOC_HAND_LEFT], *is);
				CalcPlrInv(pnum, true);
			}
			return true;
		}
		if (plr._pInvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE && plr._pInvBody[INVLOC_HAND_LEFT]._iLoc != ILOC_TWOHAND) {
			if (saveflag) {
				copy_pod(plr._pInvBody[INVLOC_HAND_RIGHT], *is);
				CalcPlrInv(pnum, true);
			}
			return true;
		}
	} else if (plr._pInvBody[INVLOC_HAND_LEFT]._itype == ITYPE_NONE && plr._pInvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE) {
		if (saveflag) {
			copy_pod(plr._pInvBody[INVLOC_HAND_LEFT], *is);
			CalcPlrInv(pnum, true);
		}
		return true;
	}

	return false;
}*/

bool AutoPlaceBelt(int pnum, ItemStruct* is, bool saveflag)
{
	ItemStruct* pi;
	int i, n;

	if (is->_iLoc != ILOC_BELT || !is->_iStatFlag)
		return false;

	// assert(is->_iUsable);
	n = is->_iDurability;
	// assert(n != 0); -- otherwise the item is discarded
	// assert(n == 1 || saveflag); -- otherwise subsequent AutoPlaceBelt/AutoPlaceInv calls might fail
	for (i = 0; i < MAXBELTITEMS; i++) {
		pi = &plr._pSpdList[i];
		if (pi->_itype != ITYPE_NONE
		 && pi->_iMiscId == is->_iMiscId
		 && pi->_iSpell == is->_iSpell
		 && pi->_iDurability < pi->_iMaxDur) { // STACK
			n -= pi->_iMaxDur - pi->_iDurability;
			if (saveflag) {
				//pi->_ivalue /= pi->_iDurability;
				//is->_ivalue /= is->_iDurability;
				if (n <= 0)
					pi->_iDurability = pi->_iMaxDur + n;
				else
					pi->_iDurability = pi->_iMaxDur;
				is->_iDurability = n;
				//pi->_ivalue *= pi->_iDurability;
				//is->_ivalue *= is->_iDurability;
			}
			if (n <= 0)
				return true;
		}
	}

	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr._pSpdList[i]._itype == ITYPE_NONE) {
			if (saveflag) {
				copy_pod(plr._pSpdList[i], *is);
				CalcPlrScrolls(pnum);
				//gbRedrawFlags |= REDRAW_SPEED_BAR;
			}
			return true;
		}
	}
	return false;
}

bool AutoPlaceInv(int pnum, ItemStruct* is, bool saveflag)
{
	ItemStruct* pi;
	int i, n, w, h;

	if (is->_iUsable) {
		// assert(is->_itype == ITYPE_MISC);
		n = is->_iDurability;
		// assert(n != 0); -- otherwise the item is discarded
		for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
			pi = &plr._pInvList[i];
			if (pi->_itype != ITYPE_NONE
			 && pi->_iMiscId == is->_iMiscId
			 && pi->_iSpell == is->_iSpell
			 && pi->_iDurability < pi->_iMaxDur) { // STACK
				n -= pi->_iMaxDur - pi->_iDurability;
				if (saveflag) {
					//pi->_ivalue /= pi->_iDurability;
					//is->_ivalue /= is->_iDurability;
					if (n <= 0)
						pi->_iDurability = pi->_iMaxDur + n;
					else
						pi->_iDurability = pi->_iMaxDur;
					is->_iDurability = n;
					//pi->_ivalue *= pi->_iDurability;
					//is->_ivalue *= is->_iDurability;
				}
				if (n <= 0)
					return true;
			}
		}
	}

	i = is->_iCurs + CURSOR_FIRSTITEM;
	w = InvItemWidth[i] / INV_SLOT_SIZE_PX;
	h = InvItemHeight[i] / INV_SLOT_SIZE_PX;

	static_assert(NUM_INV_GRID_ELEM % 10 == 0, "AutoPlaceInv expects 10 slot per row.");
	pi = saveflag ? is : NULL;
	if (h == 1) {
		for (i = 0; i < NUM_INV_GRID_ELEM; i++)
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
	} else if (h == 2) {
		for (i = 9; i >= 0; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
			if (AutoPlace(pnum, i + 20, w, h, pi))
				return true;
		}
		for (i = 19; i >= 10; i--) {
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		}
	} else if (h == 3) {
		for (i = 10; i < 20; i++)
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
		for (i = 0; i < 10; i++)
			if (AutoPlace(pnum, i, w, h, pi))
				return true;
	}
	return false;
}

static int SwapItem(ItemStruct* a, ItemStruct* b)
{
	ItemStruct h;

	copy_pod(h, *a);
	copy_pod(*a, *b);
	copy_pod(*b, h);

	return h._iCurs + CURSOR_FIRSTITEM;
}

static int SwapHoldBodyItem(PlayerStruct* p, ItemStruct* holditem, int invLoc)
{
	ItemStruct* is;
	int cn;

	is = &p->_pInvBody[invLoc];

	cn = CURSOR_HAND;
	if (is->_itype == ITYPE_NONE)
		copy_pod(*is, *holditem);
	else
		cn = SwapItem(is, holditem);
	return cn;
}

static int MergeStackableItem(ItemStruct* a, ItemStruct* b)
{
	int gt, ig, cn;

	// assert(a->_iUsable || b->_iUsable);
	if (a->_iMiscId == IMISC_MAP // TODO: use a->_iMaxDur != 1 instead?
	 || a->_iMiscId != b->_iMiscId
	 || a->_iSpell != b->_iSpell) {
		return CURSOR_NONE;
	}
	// assert(a->_iUsable && b->_iUsable);
	gt = a->_iDurability;
	ig = b->_iDurability + gt; // STACK
	//a->_ivalue /= a->_iDurability;
	//b->_ivalue /= b->_iDurability;
	if (ig <= a->_iMaxDur) {
		a->_iDurability = ig;
		cn = CURSOR_HAND;
	} else {
		ig = a->_iMaxDur - gt;
		a->_iDurability = a->_iMaxDur;
		b->_iDurability -= ig;
		cn = b->_iCurs + CURSOR_FIRSTITEM;
	}
	//a->_ivalue *= a->_iDurability;
	//b->_ivalue *= b->_iDurability;
	return cn;
}

static void CheckInvPaste()
{
	int r, sx, sy, i, j;

	sx = cursW;
	sy = cursH;
	i = MousePos.x; // + (sx >> 1); -- CURSOR_HOTSPOT
	j = MousePos.y; // + (sy >> 1);
	sx /= INV_SLOT_SIZE_PX;
	sy /= INV_SLOT_SIZE_PX;

	for (r = 0; r < SLOTXY_BELT_FIRST; r++) {
		if (POS_IN_RECT(i, j,
			gnWndInvX + InvRect[r].X, gnWndInvY + InvRect[r].Y - INV_SLOT_SIZE_PX,
			INV_SLOT_SIZE_PX + 1, INV_SLOT_SIZE_PX + 1)) {
			NetSendCmdBParam1(CMD_PASTEPLRITEM, r);
			break;
		}
		if (r == SLOTXY_CHEST_LAST) {
			if ((sx & 1) == 0)
				i -= INV_SLOT_SIZE_PX / 2;
			if ((sy & 1) == 0)
				j -= INV_SLOT_SIZE_PX / 2;
		}
	}
}

void InvPasteItem(int pnum, BYTE r)
{
	PlayerStruct* p;
	ItemStruct *holditem, *is, *wRight;
	int sx, sy;
	int i, j, xx, ii;
	bool done;
	int il, cn, it, iv, ig, gt;

	// assert(plr._pmode != PM_DEATH);

	p = &plr;

	holditem = &p->_pHoldItem;
	if (holditem->_itype == ITYPE_NONE)
		return;

	i = holditem->_iCurs + CURSOR_FIRSTITEM;
	sx = InvItemWidth[i] / INV_SLOT_SIZE_PX;
	sy = InvItemHeight[i] / INV_SLOT_SIZE_PX;

	switch (InvSlotTbl[r]) {
	case SLOT_HEAD:
		il = ILOC_HELM;
		r = INVLOC_HEAD;
		break;
	case SLOT_RING_LEFT:
		il = ILOC_RING;
		r = INVLOC_RING_LEFT;
		break;
	case SLOT_RING_RIGHT:
		il = ILOC_RING;
		r = INVLOC_RING_RIGHT;
		break;
	case SLOT_AMULET:
		il = ILOC_AMULET;
		r = INVLOC_AMULET;
		break;
	case SLOT_HAND_LEFT:
	case SLOT_HAND_RIGHT:
		il = ILOC_ONEHAND;
		break;
	case SLOT_CHEST:
		il = ILOC_ARMOR;
		r = INVLOC_CHEST;
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
			is = &p->_pInvList[ii];
			if (is->_itype != ITYPE_NONE && is->_itype != ITYPE_GOLD) {
				if (is->_itype == ITYPE_PLACEHOLDER)
					it = is->_iPHolder;
				else
					it = ii;
				it++;
			}
		} else {
			xx = ((sy - 1) >> 1) * 10;
			if (ii >= xx)
				ii = ii - xx;
			xx = ((sx - 1) >> 1);
			if ((ii % 10) >= xx)
				ii = ii - xx;
			static_assert(NUM_INV_GRID_ELEM % 10 == 0, "InvPasteItem expects 10 slot per row.");
			if ((ii % 10) + sx > 10
			 || (ii / 10) + sy > NUM_INV_GRID_ELEM / 10)
				done = false;

			xx = ii;
			for (j = 0; j < sy && done; j++) {
				for (i = 0; i < sx && done; i++, xx++) {
					is = &p->_pInvList[xx];
					if (is->_itype != ITYPE_NONE) {
						iv = xx;
						if (is->_itype == ITYPE_PLACEHOLDER)
							iv = is->_iPHolder;
						iv++;
						if (it == 0)
							it = iv;
						if (it != iv)
							done = false;
					}
				}
				xx += 10 - sx;
			}
		}
	} else if (il == holditem->_iLoc) {
		done = true;
	} else if (il == ILOC_ONEHAND && holditem->_iLoc == ILOC_TWOHAND) {
		if (TWOHAND_WIELD(p, holditem))
			il = ILOC_TWOHAND;
		done = true;
	}

	if (!done)
		return;

	if (il != ILOC_UNEQUIPABLE && !holditem->_iStatFlag) {
		if (pnum == mypnum)
			PlaySfx(sgSFXSets[SFXS_PLR_13][p->_pClass]);
		return;
	}

	cn = CURSOR_HAND;
	switch (il) {
	case ILOC_HELM:
	case ILOC_RING:
	case ILOC_AMULET:
	case ILOC_ARMOR:
		cn = SwapHoldBodyItem(p, holditem, r);
		break;
	case ILOC_ONEHAND:
		is = &p->_pInvBody[INVLOC_HAND_LEFT];
		wRight = &p->_pInvBody[INVLOC_HAND_RIGHT];
		if (holditem->_iClass == ICLASS_WEAPON) {
			// place a weapon
			if (is->_itype == ITYPE_NONE) {
				// always place the weapon in the left hand if possible
				copy_pod(*is, *holditem);
			} else {
				if (r <= SLOTXY_HAND_LEFT_LAST) {
					// in the left hand - replace the current item
					cn = SwapItem(is, holditem);
				} else {
					// in the right hand
					if (/*is->_itype != ITYPE_NONE &&*/ TWOHAND_WIELD(p, is)) {
						// replace two-handed weapons and place the weapon in the left hand
						cn = SwapItem(is, holditem);
					} else if (wRight->_itype != ITYPE_NONE) {
						// replace item in the right hand
						cn = SwapItem(wRight, holditem);
					} else {
						// place weapon in the right hand
						copy_pod(*wRight, *holditem);
					}
				}
			}
		} else {
			// place a shield
			if (wRight->_itype != ITYPE_NONE) {
				// replace if there was something
				cn = SwapItem(wRight, holditem);
			} else if (is->_itype != ITYPE_NONE && TWOHAND_WIELD(p, is)) {
				// pick two-handed weapons and place the shield in the right hand
				SwapItem(wRight, is);
				cn = SwapItem(wRight, holditem);
			} else {
				// place the shield in the right hand
				copy_pod(*wRight, *holditem);
			}
		}
		break;
	case ILOC_TWOHAND:
		is = &p->_pInvBody[INVLOC_HAND_LEFT];
		wRight = &p->_pInvBody[INVLOC_HAND_RIGHT];
		if (is->_itype != ITYPE_NONE && wRight->_itype != ITYPE_NONE) {
			if (wRight->_itype != ITYPE_SHIELD)
				wRight = is;
			if (!AutoPlaceInv(pnum, wRight, true))
				return;

			wRight->_itype = ITYPE_NONE;
			wRight = &p->_pInvBody[INVLOC_HAND_RIGHT];
		}

		/*if (is->_itype != ITYPE_NONE || wRight->_itype != ITYPE_NONE) {
			if (is->_itype == ITYPE_NONE)
				SwapItem(is, wRight);
			cn = SwapItem(is, holditem);
		} else {
			copy_pod(*is, *holditem);
		}*/
		if (/*is->_itype == ITYPE_NONE &&*/ wRight->_itype != ITYPE_NONE) {
			SwapItem(is, wRight);
			cn = SwapItem(is, holditem);
		} else {
			cn = SwapHoldBodyItem(p, holditem, INVLOC_HAND_LEFT);
		}
		break;
	case ILOC_UNEQUIPABLE:
		//ii = r - SLOTXY_INV_FIRST;
		is = &p->_pInvList[ii];
		if (holditem->_itype == ITYPE_GOLD && it == 0) {
			// empty or gold target
			if (is->_itype == ITYPE_GOLD) {
				// gold target
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
					cn = holditem->_iCurs + CURSOR_FIRSTITEM;
				}
			} else {
				// empty target
				copy_pod(*is, *holditem);
				p->_pGold += holditem->_ivalue;
			}
		} else {
			if (it == 0) {
				// empty target
				copy_pod(*is, *holditem);
			} else {
				if (holditem->_iUsable) {
					cn = MergeStackableItem(is, holditem);
					if (cn != CURSOR_NONE)
						break;
				}

				it--;
				for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
					if (p->_pInvList[i]._itype == ITYPE_PLACEHOLDER
					 && p->_pInvList[i]._iPHolder == it)
						p->_pInvList[i]._itype = ITYPE_NONE;
				}
				if (holditem->_itype == ITYPE_GOLD)
					p->_pGold += holditem->_ivalue;
				if (it == ii) {
					cn = SwapItem(is, holditem);
				} else {
					copy_pod(*is, *holditem);
					copy_pod(*holditem, p->_pInvList[it]);
					p->_pInvList[it]._itype = ITYPE_NONE;
					cn = holditem->_iCurs + CURSOR_FIRSTITEM;
				}
				if (holditem->_itype == ITYPE_GOLD)
					p->_pGold -= holditem->_ivalue;
			}
			it = ii + (sy - 1) * 10; // start from the bottom-left corner
			if (it != ii)
				copy_pod(p->_pInvList[it], *is);

			xx = it;
			for (j = 0; j < sy; j++) {
				for (i = 0; i < sx; i++, xx++) {
					if (i != 0 || j != 0) {
						p->_pInvList[xx]._itype = ITYPE_PLACEHOLDER;
						p->_pInvList[xx]._iPHolder = it;
					}
				}
				xx -= 10 + sx;
			}
		}
		break;
	default:
		ASSUME_UNREACHABLE
	}
	CalcPlrInv(pnum, true);
	if (cn == CURSOR_HAND)
		holditem->_itype = ITYPE_NONE;
	if (pnum == mypnum) {
		PlaySfx(itemfiledata[ItemCAnimTbl[pcursicon - CURSOR_FIRSTITEM]].iiSFX);
		// if (cn == CURSOR_HAND) {
		//	SetCursorPos(MousePos.x + (cursW >> 1), MousePos.y + (cursH >> 1));
		// }
		NewCursor(cn);
	}
}

static void CheckBeltPaste()
{
	int r, i, j;
	i = MousePos.x; // + INV_SLOT_SIZE_PX / 2; -- CURSOR_HOTSPOT
	j = MousePos.y; // + INV_SLOT_SIZE_PX / 2;

	for (r = 0; r < MAXBELTITEMS; r++) {
		if (POS_IN_RECT(i, j,
			gnWndBeltX + InvRect[SLOTXY_BELT_FIRST + r].X,  gnWndBeltY + InvRect[SLOTXY_BELT_FIRST + r].Y - INV_SLOT_SIZE_PX,
			INV_SLOT_SIZE_PX + 1, INV_SLOT_SIZE_PX + 1)) {
			NetSendCmdBParam1(CMD_PASTEPLRBELTITEM, r);
			break;
		}
	}
}

void InvPasteBeltItem(int pnum, BYTE r)
{
	ItemStruct *holditem, *is;
	int cn;

	// assert(plr._pmode != PM_DEATH);
	// assert(r < MAXBELTITEMS);

	holditem = &plr._pHoldItem;

	if (holditem->_iLoc != ILOC_BELT || holditem->_itype == ITYPE_NONE)
		return;
	// assert(holditem->_iUsable);
	is = &plr._pSpdList[r];
	cn = CURSOR_NONE;
	if (is->_itype != ITYPE_NONE) {
		// matching stackable items
		cn = MergeStackableItem(is, holditem);
	}
	if (cn == CURSOR_NONE) {
		cn = SwapItem(is, holditem);
		if (holditem->_itype == ITYPE_NONE)
			cn = CURSOR_HAND;
	}

	CalcPlrScrolls(pnum);
	if (pnum == mypnum) {
		PlaySfx(itemfiledata[ItemCAnimTbl[pcursicon - CURSOR_FIRSTITEM]].iiSFX);
		//gbRedrawFlags |= REDRAW_SPEED_BAR;
		// if (cn == CURSOR_HAND)
		//	SetCursorPos(MousePos.x + (cursW >> 1), MousePos.y + (cursH >> 1));
		NewCursor(cn);
	}
}

static bool CheckInvCut()
{
	BYTE cii;

	if (myplr._pmode != PM_STAND) {
		return false;
	}

	cii = pcursinvitem;
	if (!INVIDX_VALID(cii))
		return false;

	static_assert(KMOD_SHIFT <= UCHAR_MAX, "CheckInvCut send the state of the shift in a byte field.");
	NetSendCmdBParam2(CMD_CUTPLRITEM, cii, (gbModBtnDown & ACTBTN_MASK(ACT_MODACT)) != 0);
	return true;
}

/*
 * @brief Handle a click in the inventory/belt.
 * @param pnum: the id of the player
 * @param cii: the location where the item is in the inventory/belt (inv_item)
 * @param bShift: if it is set the item is moved between the inventory and the belt (if there is enough space)
 *                   otherwise the item is added to the player's hand
 */
void InvCutItem(int pnum, BYTE cii, bool bShift)
{
	ItemStruct* pi;
	int i, ii;

	// assert(plr._pmode != PM_DEATH);

	if (plr._pHoldItem._itype != ITYPE_NONE)
		return;

	switch (cii) {
	case INVITEM_HEAD:
	case INVITEM_RING_LEFT:
	case INVITEM_RING_RIGHT:
	case INVITEM_AMULET:
	case INVITEM_HAND_RIGHT:
	case INVITEM_CHEST:
		static_assert((int)INVITEM_HEAD == (int)INVLOC_HEAD, "Switch of InvCutItem expects matching enum values I.");
		static_assert((int)INVITEM_RING_LEFT == (int)INVLOC_RING_LEFT, "Switch of InvCutItem expects matching enum values II.");
		static_assert((int)INVITEM_RING_RIGHT == (int)INVLOC_RING_RIGHT, "Switch of InvCutItem expects matching enum values III.");
		static_assert((int)INVITEM_AMULET == (int)INVLOC_AMULET, "Switch of InvCutItem expects matching enum values IV.");
		static_assert((int)INVITEM_HAND_RIGHT == (int)INVLOC_HAND_RIGHT, "Switch of InvCutItem expects matching enum values VI.");
		static_assert((int)INVITEM_CHEST == (int)INVLOC_CHEST, "Switch of InvCutItem expects matching enum values VII.");
		pi = &plr._pInvBody[cii];
		break;
	case INVITEM_HAND_LEFT:
		static_assert((int)INVITEM_HAND_LEFT == (int)INVLOC_HAND_LEFT, "Switch of InvCutItem expects matching enum values V.");
		pi = &plr._pInvBody[INVITEM_HAND_RIGHT];
		if (pi->_itype != ITYPE_NONE && pi->_itype != ITYPE_SHIELD) {
			// move right hand weapon to left hand
			SwapItem(pi, &plr._pInvBody[INVITEM_HAND_LEFT]);
		} else {
			pi = &plr._pInvBody[INVITEM_HAND_LEFT];
		}
		break;
	default:
		static_assert(INVITEM_CHEST + 1 == INVITEM_INV_FIRST, "InvCutItem expects the storage items after the normal items.");
		static_assert(INVITEM_INV_LAST + 1 == INVITEM_BELT_FIRST, "InvCutItem expects the storage items before the belt items.");
		if (/*cii >= INVITEM_INV_FIRST &&*/ cii <= INVITEM_INV_LAST) {
			ii = cii - INVITEM_INV_FIRST;
			pi = &plr._pInvList[ii];
			if (pi->_itype == ITYPE_PLACEHOLDER) {
				ii = pi->_iPHolder;
				pi = &plr._pInvList[ii];
			}

			for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
				if (plr._pInvList[i]._itype == ITYPE_PLACEHOLDER
				 && plr._pInvList[i]._iPHolder == ii) {
					plr._pInvList[i]._itype = ITYPE_NONE;
				}
			}
			if (bShift && pi->_itype != ITYPE_NONE && AutoPlaceBelt(pnum, pi, true)) {
				pi->_itype = ITYPE_NONE;
			}
		} else { // cii >= INVITEM_BELT_FIRST
			ii = cii - INVITEM_BELT_FIRST;
			// assert(ii < MAXBELTITEMS);
			pi = &plr._pSpdList[ii];
			if (bShift && pi->_itype != ITYPE_NONE && AutoPlaceInv(pnum, pi, true)) {
				//gbRedrawFlags |= REDRAW_SPEED_BAR;
				pi->_itype = ITYPE_NONE;
			}
		}
		break;
	}

	if (pi->_itype == ITYPE_NONE)
		return;
	copy_pod(plr._pHoldItem, *pi);
	pi->_itype = ITYPE_NONE;
	if (plr._pHoldItem._itype == ITYPE_GOLD)
		CalculateGold(pnum);
	else
		CalcPlrInv(pnum, true);

	if (pnum == mypnum) {
		PlaySfx(IS_IGRAB);
		NewCursor(plr._pHoldItem._iCurs + CURSOR_FIRSTITEM);
		// SetCursorPos(MousePos.x - (cursW >> 1), MousePos.y - (cursH >> 1));
	}
}

/*
 * Remove an item from the player's inventory, equipment or belt.
 * Does not maintain the _pInvGrid of the player.
 * @param pnum the id of the player
 * @param bLoc inv_item enum value to identify the item
 */
void SyncPlrItemRemove(int pnum, BYTE bLoc)
{
	static_assert(INVITEM_BODY_FIRST == 0, "SyncPlrItemRemove expects the body location as first.");
	if (bLoc < INVITEM_INV_FIRST) {
		// body item
		plr._pInvBody[bLoc]._itype = ITYPE_NONE;
		// move right hand weapon to left hand
		if (bLoc == INVITEM_HAND_LEFT
		 && plr._pInvBody[INVITEM_HAND_RIGHT]._itype != ITYPE_NONE
		 && plr._pInvBody[INVITEM_HAND_RIGHT]._itype != ITYPE_SHIELD)
			SwapItem(&plr._pInvBody[INVITEM_HAND_LEFT], &plr._pInvBody[INVITEM_HAND_RIGHT]);
		CalcPlrInv(pnum, true/*plr._pmode != PM_DEATH*/);
	} else if (bLoc < INVITEM_BELT_FIRST) {
		// inv item
		bLoc -= INVITEM_INV_FIRST;
		SyncPlrStorageRemove(pnum, bLoc);
	} else {
		// belt item
		bLoc -= INVITEM_BELT_FIRST;
		// assert(bLoc < MAXBELTITEMS);
		plr._pSpdList[bLoc]._itype = ITYPE_NONE;
		CalcPlrScrolls(pnum);
		//gbRedrawFlags |= REDRAW_SPEED_BAR;
	}
}

void SyncPlrStorageRemove(int pnum, int iv)
{
	ItemStruct* pi;
	int frame, sx, sy, i, j, xx;

	pi = &plr._pInvList[iv];
	if (pi->_itype == ITYPE_PLACEHOLDER) {
		iv = pi->_iPHolder;
		pi = &plr._pInvList[iv];
	}

	frame = pi->_iCurs + CURSOR_FIRSTITEM;
	sx = InvItemWidth[frame] / INV_SLOT_SIZE_PX;
	sy = InvItemHeight[frame] / INV_SLOT_SIZE_PX;

	static_assert(NUM_INV_GRID_ELEM % 10 == 0, "SyncPlrStorageRemove expects 10 slot per row.");
	xx = iv;
	for (j = 0; j < sy; j++) {
		for (i = 0; i < sx; i++, xx++) {
			plr._pInvList[xx]._itype = ITYPE_NONE;
		}
		xx -= 10 + sx;
	}

	CalcPlrScrolls(pnum);
}

void CheckInvClick(bool altAction)
{
	if (altAction) {
		if (INVIDX_VALID(pcursinvitem))
			InvUseItem(pcursinvitem);
	} else if (pcursicon >= CURSOR_FIRSTITEM) {
		CheckInvPaste();
	} else {
		if (!CheckInvCut()) {
			StartWndDrag(WND_INV);
		}
	}
}

/**
 * Check for interactions with belt
 */
void CheckBeltClick(bool altAction)
{
	if (altAction) {
		if (INVIDX_VALID(pcursinvitem))
			InvUseItem(pcursinvitem);
	} else if (pcursicon >= CURSOR_FIRSTITEM) {
		/*return*/ CheckBeltPaste();
	} else {
		if (!CheckInvCut()) {
			StartWndDrag(WND_BELT);
		}
	}
}

static void CheckQuestItem(int pnum, ItemStruct* is)
{
	int idx, delay = 0;

	idx = is->_iIdx;
	if (idx == IDI_OPTAMULET) {
		if (quests[Q_BLIND]._qactive == QUEST_ACTIVE)
			quests[Q_BLIND]._qactive = QUEST_DONE;
	} else if (idx == IDI_MUSHROOM) {
		if (pnum == mypnum
		 && quests[Q_MUSHROOM]._qactive == QUEST_ACTIVE && quests[Q_MUSHROOM]._qvar1 < QV_MUSHROOM_MUSHGIVEN
		 && quests[Q_MUSHROOM]._qvar2 != SFXS_PLR_95) {
			quests[Q_MUSHROOM]._qvar2 = SFXS_PLR_95;
			delay = 10;
			idx = TEXT_IM_MUSHROOM;
		}
	} else if (idx == IDI_ANVIL) {
		if (quests[Q_ANVIL]._qactive == QUEST_ACTIVE) {
			delay = 10;
			idx = TEXT_IM_ANVIL;
		}
	} else if (idx == IDI_GLDNELIX) {
		if (quests[Q_VEIL]._qactive == QUEST_ACTIVE) {
			delay = 30;
			idx = TEXT_IM_GLDNELIX;
		}
	} else if (idx == IDI_ROCK) {
		if (quests[Q_ROCK]._qactive == QUEST_ACTIVE) {
			delay = 10;
			idx = TEXT_IM_ROCK;
		}
	} else if (idx == IDI_ARMOFVAL) {
		if (quests[Q_BLOOD]._qactive == QUEST_ACTIVE) {
			quests[Q_BLOOD]._qactive = QUEST_DONE;
			delay = 20;
			idx = TEXT_IM_ARMOFVAL;
		}
#ifdef HELLFIRE
	} else if (idx == IDI_FANG) {
		if (quests[Q_GRAVE]._qactive == QUEST_INIT) {
			delay = 10;
			idx = TEXT_IM_FANG;
		}
	} else if (idx == IDI_NOTE1 || idx == IDI_NOTE2 || idx == IDI_NOTE3) {
		int nn, i, x, y;
		if ((idx == IDI_NOTE1 || PlrHasStorageItem(pnum, IDI_NOTE1, &nn))
		 && (idx == IDI_NOTE2 || PlrHasStorageItem(pnum, IDI_NOTE2, &nn))
		 && (idx == IDI_NOTE3 || PlrHasStorageItem(pnum, IDI_NOTE3, &nn))) {
			static_assert((int)IDI_NOTE1 + 1 == (int)IDI_NOTE2, "CheckQuestItem requires an ordered IDI_NOTE enum I.");
			static_assert((int)IDI_NOTE2 + 1 == (int)IDI_NOTE3, "CheckQuestItem requires an ordered IDI_NOTE enum II.");
			for (i = IDI_NOTE1; i <= IDI_NOTE3; i++) {
				if (idx != i) {
					PlrHasStorageItem(pnum, i, &nn);
					SyncPlrStorageRemove(pnum, nn);
				}
			}
			// preserve seed and location of the last item (required by DeleteItem[AutoGetItem, InvGetItem])
			idx = is->_iSeed;
			x = is->_ix;
			y = is->_iy;
			SetItemSData(is, IDI_FULLNOTE);
			is->_iSeed = idx;
			is->_ix = x;
			is->_iy = y;
			delay = 10;
			idx = TEXT_IM_FULLNOTE;
		}
#endif
	}
	if (delay != 0 && pnum == mypnum) {
		gnSfxDelay = delay;
		gnSfxNum = idx;
	}
}

void SyncInvGetItem(int pnum, int ii)
{
	ItemStruct* is;

	is = &items[ii];

	// always mask CF_PREGEN to make life of RecreateItem easier later on
	// otherwise this should not have an effect, since the item is already in 'delta'
	//is->_iCreateInfo &= ~CF_PREGEN;
	is->_iFloorFlag = FALSE;
	CheckQuestItem(pnum, is);
	ItemStatOk(pnum, is);
	copy_pod(plr._pHoldItem, *is);
}

void InvGetItem(int pnum, int ii)
{
	assert(dItem[items[ii]._ix][items[ii]._iy] == ii + 1);

	SyncInvGetItem(pnum, ii);

	if (pnum == mypnum) {
		PlaySfx(IS_IGRAB);
		NewCursor(plr._pHoldItem._iCurs + CURSOR_FIRSTITEM);
		// SetCursorPos(MousePos.x - (cursW >> 1), MousePos.y - (cursH >> 1));
		pcursitem = ITEM_NONE;
	}

	DeleteItem(ii);
}

bool SyncAutoGetItem(int pnum, int ii)
{
	ItemStruct* is;
	bool done;

	is = &items[ii];
	// always mask CF_PREGEN to make life of RecreateItem easier later on
	// otherwise this should not have an effect, since the item is already in 'delta'
	//is->_iCreateInfo &= ~CF_PREGEN;
	is->_iFloorFlag = FALSE;
	CheckQuestItem(pnum, is);
	ItemStatOk(pnum, is);
	if (is->_itype == ITYPE_GOLD) {
		done = GoldAutoPlace(pnum, is);
	} else {
		done = /*WeaponAutoPlace(pnum, is, true)
			|| */AutoPlaceBelt(pnum, is, true)
			|| AutoPlaceInv(pnum, is, true);
	}
	return done;
}

bool AutoGetItem(int pnum, int ii)
{
	bool done;

	assert(dItem[items[ii]._ix][items[ii]._iy] == ii + 1);

	done = SyncAutoGetItem(pnum, ii);

	if (done) {
		DeleteItem(ii);
	} else {
		if (pnum == mypnum) {
			PlaySfxN(sgSFXSets[SFXS_PLR_14][plr._pClass], 3);
		}
		RespawnItem(ii, true);
	}
	return done;
}

void SyncSplitGold(int pnum, int cii, int value)
{
	ItemStruct* pi;
	int seed, val;

	// assert(cii < NUM_INV_GRID_ELEM);

	pi = &plr._pInvList[cii];
	if (pi->_itype != ITYPE_GOLD)
		return;
	if (plr._pHoldItem._itype != ITYPE_NONE)
		return;
	val = pi->_ivalue - value;
	if (val > 0) {
		seed = GetNewItemSeed(pi);
		SetGoldItemValue(pi, val);
	} else {
		seed = pi->_iSeed; // preserve the original seed
		value += val;
		SyncPlrStorageRemove(pnum, cii);
	}
	plr._pGold -= value;
	pi = &plr._pHoldItem;
	CreateBaseItem(pi, IDI_GOLD);
	pi->_iSeed = seed;
	pi->_iStatFlag = TRUE;
	SetGoldItemValue(pi, value);
	if (pnum == mypnum) {
		PlaySfx(IS_IGRAB);
		NewCursor(pi->_iCurs + CURSOR_FIRSTITEM);
		// SetCursorPos(MousePos.x - (cursW >> 1), MousePos.y - (cursH >> 1));
	}
}

BYTE CheckInvBelt()
{
	int r;

	for (r = SLOTXY_BELT_FIRST; r <= SLOTXY_BELT_LAST; r++) {
		if (POS_IN_RECT(MousePos.x, MousePos.y,
			gnWndBeltX + InvRect[r].X, gnWndBeltY + InvRect[r].Y - INV_SLOT_SIZE_PX,
			INV_SLOT_SIZE_PX + 1, INV_SLOT_SIZE_PX + 1)) {
			break;
		}
	}
	r -= SLOTXY_BELT_FIRST;
	if (r < MAXBELTITEMS && myplr._pSpdList[r]._itype != ITYPE_NONE) {
		//gbRedrawFlags |= REDRAW_SPEED_BAR;
		return INVITEM_BELT_FIRST + r;
	}

	return INVITEM_NONE;
}

BYTE CheckInvItem()
{
	int r;
	ItemStruct* pi;
	PlayerStruct* p;
	BYTE rv;

	for (r = 0; r < SLOTXY_BELT_FIRST; r++) {
		if (POS_IN_RECT(MousePos.x, MousePos.y,
			gnWndInvX + InvRect[r].X, gnWndInvY + InvRect[r].Y - INV_SLOT_SIZE_PX,
			INV_SLOT_SIZE_PX + 1, INV_SLOT_SIZE_PX + 1)) {
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
		pi = &p->_pInvBody[rv];
		if (pi->_itype == ITYPE_NONE || !TWOHAND_WIELD(p, pi)) {
			rv = INVITEM_HAND_RIGHT;
			static_assert((int)INVLOC_HAND_RIGHT == (int)INVITEM_HAND_RIGHT, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		}
		break;
	case SLOT_CHEST:
		rv = INVITEM_CHEST;
		static_assert((int)INVLOC_CHEST == (int)INVITEM_CHEST, "INVLOC - INVITEM match is necessary in CheckInvBody 1.");
		break;
	case SLOT_STORAGE:
		rv = r - SLOTXY_INV_FIRST;
		pi = &p->_pInvList[rv];
		if (pi->_itype == ITYPE_NONE)
			return INVITEM_NONE;
		return rv + INVITEM_INV_FIRST;
	case SLOT_BELT:
		return INVITEM_NONE;
	default:
		ASSUME_UNREACHABLE
	}

	pi = &p->_pInvBody[rv];
	if (pi->_itype == ITYPE_NONE)
		return INVITEM_NONE;

	return rv;
}

static void StartGoldDrop(int cii)
{
	if (gbTalkflag /*|| myplr._pmode != PM_STAND*/)
		return;
	gbDropGoldIndex = cii;
	gnDropGoldValue = 0;
}

static void InvAddHp(int pnum)
{
	PlayerStruct* p;
	int hp;

	p = &plr;
	// commented out because iSeed of the STACK is not regenerated
	// hp = p->_pMaxHP >> 8;
	// hp = ((hp >> 1) + random_(39, hp)) << 6;
	hp = p->_pMaxHP >> (8 - 6);
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
	PlrIncHp(pnum, hp);
}

static void InvAddMana(int pnum)
{
	PlayerStruct* p;
	int mana;

	p = &plr;
	// commented out because iSeed of the STACK is not regenerated
	// mana = p->_pMaxMana >> 8;
	// mana = ((mana >> 1) + random_(40, mana)) << 6;
	mana = p->_pMaxMana >> (8 - 6);
	switch (p->_pClass) {
	case PC_WARRIOR:              break;
	case PC_SORCERER: mana <<= 1; break;
#ifdef HELLFIRE
	case PC_BARBARIAN:            break;
	case PC_MONK:
	case PC_BARD:
#endif
	case PC_ROGUE:
		mana += mana >> 1;        break;
	default:
		ASSUME_UNREACHABLE
	}
	PlrIncMana(pnum, mana);
}

void InvUseItem(int cii)
{
	ItemStruct* is;
	int pnum = mypnum;
	// assert(INVIDX_VALID(cii));
	// assert(plr._pHitPoints != 0);

	is = PlrItem(pnum, cii);

	if (is->_itype == ITYPE_NONE)
		return;
	if (is->_iIdx == IDI_GOLD) {
		StartGoldDrop(cii);
		return;
	}
	if (is->_iIdx == IDI_MUSHROOM) {
		gnSfxDelay = 10;
		gnSfxNum = TEXT_IM_MUSHROOM;
		return;
	}
	if (is->_iIdx == IDI_FUNGALTM) {
		PlaySfx(IS_IBOOK);
		gnSfxDelay = 10;
		gnSfxNum = TEXT_IM_FUNGALTM;
		return;
	}

	if (!is->_iUsable) {
		return;
	}

	if (!is->_iStatFlag) {
		PlaySfx(sgSFXSets[SFXS_PLR_13][plr._pClass]);
		return;
	}

	if (currLvl._dType == DTYPE_TOWN
#ifdef HELLFIRE
	 && (is->_iMiscId == IMISC_SCROLL || is->_iMiscId == IMISC_RUNE)
#else
	 && is->_iMiscId == IMISC_SCROLL
#endif
	 && (spelldata[is->_iSpell].sUseFlags & SFLAG_DUNGEON) == SFLAG_DUNGEON) {
		return;
	}
	if (currLvl._dType != DTYPE_TOWN && is->_iMiscId == IMISC_MAP) {
		return;
	}

	// add sfx
	PlaySfx(is->_iMiscId == IMISC_BOOK ? IS_RBOOK : itemfiledata[ItemCAnimTbl[is->_iCurs]].iiSFX);

	// use the item
	switch (is->_iMiscId) {
	case IMISC_HEAL:
	case IMISC_FULLHEAL:
	case IMISC_MANA:
	case IMISC_FULLMANA:
	case IMISC_REJUV:
	case IMISC_FULLREJUV:
	case IMISC_BOOK:
	case IMISC_SPECELIX:
		NetSendCmdBParam1(CMD_USEPLRITEM, cii);
		break;
	case IMISC_SCROLL:
#ifdef HELLFIRE
	case IMISC_RUNE:
#endif
	{
		BYTE sn = is->_iSpell;
		const CmdSkillUse itmSkill = { sn, static_cast<int8_t>(cii) }; 
		if (spelldata[sn].scCurs != CURSOR_NONE) {
			gbTSkillUse = itmSkill;
			NewCursor(spelldata[sn].scCurs);
		} else {
			NetSendCmdLocSkill(pcurspos.x, pcurspos.y, itmSkill);
		}
	} break;
	//case IMISC_MAPOFDOOM:
	//	doom_init();
	//	break;
	case IMISC_OILQLTY:
	case IMISC_OILZEN:
	case IMISC_OILSTR:
	case IMISC_OILDEX:
	case IMISC_OILVIT:
	case IMISC_OILMAG:
	case IMISC_OILRESIST:
	case IMISC_OILCHANCE:
	case IMISC_OILCLEAN:
		gbTSkillUse = { SPL_OIL, static_cast<int8_t>(cii) };
		NewCursor(CURSOR_OIL);
		break;
	case IMISC_MAP:
		InitCampaignMap(cii);
		break;
#ifdef HELLFIRE
	case IMISC_NOTE:
		StartQTextMsg(TEXT_BOOK9);
		break;
#endif
	default:
		ASSUME_UNREACHABLE
	}
}

bool SyncUseItem(int pnum, BYTE cii, BYTE sn)
{
	ItemStruct* is;

	// assert(plr._pmode != PM_DEATH);
	// assert(cii < NUM_INVELEM);

	is = PlrItem(pnum, cii);

	if (is->_itype == ITYPE_NONE || !is->_iStatFlag)
		return false;

	if (cii < INVITEM_INV_FIRST) {
		if (is->_iSpell != sn || is->_iCharges <= 0)
			return false;
		is->_iCharges--;
		CalcPlrCharges(pnum);
		return true;
	}

	if (!is->_iUsable)
		return false;

	// use the item
	// commented out because iSeed of the STACK is not regenerated
	// SetRndSeed(is->_iSeed);
	switch (is->_iMiscId) {
	case IMISC_HEAL:
		InvAddHp(pnum);
		break;
	case IMISC_FULLHEAL:
		PlrFillHp(pnum);
		break;
	case IMISC_MANA:
		InvAddMana(pnum);
		break;
	case IMISC_FULLMANA:
		PlrFillMana(pnum);
		break;
	case IMISC_REJUV:
		InvAddHp(pnum);
		InvAddMana(pnum);
		break;
	case IMISC_FULLREJUV:
		PlrFillHp(pnum);
		PlrFillMana(pnum);
		break;
	case IMISC_SCROLL:
#ifdef HELLFIRE
	case IMISC_RUNE:
#endif
		if (is->_iSpell != sn)
			return false;
		sn = SPL_INVALID;
		break;
	case IMISC_BOOK:
		sn = is->_iSpell;
		PlrIncMana(pnum, spelldata[sn].sManaCost << 6);
		plr._pSkillExp[sn] += SkillExpLvlsTbl[0];
		if (plr._pSkillExp[sn] > SkillExpLvlsTbl[MAXSPLLEVEL] - 1) {
			plr._pSkillExp[sn] = SkillExpLvlsTbl[MAXSPLLEVEL] - 1;
		}

		if (plr._pSkillExp[sn] >= SkillExpLvlsTbl[plr._pSkillLvlBase[sn]]) {
			IncreasePlrSkillLvl(pnum, sn);
		}
		// CalcPlrSpells(pnum);
		sn = SPL_INVALID;
		break;
	case IMISC_SPECELIX:
		RestorePlrHpVit(pnum);
		break;
	//case IMISC_MAPOFDOOM:
	case IMISC_NOTE:
	case IMISC_OILQLTY:
	case IMISC_OILZEN:
	case IMISC_OILSTR:
	case IMISC_OILDEX:
	case IMISC_OILVIT:
	case IMISC_OILMAG:
	case IMISC_OILRESIST:
	case IMISC_OILCHANCE:
	case IMISC_OILCLEAN:
	case IMISC_MAP:
		// should not happen, only if the player is reckless...
		return false;
	default:
		ASSUME_UNREACHABLE
	}
	// consume the item
	if (--is->_iDurability <= 0) // STACK
		SyncPlrItemRemove(pnum, cii);
	return sn == SPL_INVALID;
}

bool SyncUseMapItem(int pnum, BYTE cii, BYTE mIdx)
{
	ItemStruct* is;

	// assert(plr._pmode != PM_DEATH);
	// assert(cii < NUM_INVELEM);
	// assert(mIdx < MAXCAMPAIGNSIZE);

	is = PlrItem(pnum, cii);

	if (is->_itype == ITYPE_NONE || !is->_iStatFlag)
		return false;

	// if (!is->_iUsable)
	//	return false;

	// use the item
	if (is->_iMiscId != IMISC_MAP) {
		return false;
	}

	int available = is->_ivalue;
	int mask = 1 << mIdx;
	if (!(available & mask)) {
		return false;
	}

	if (pnum == mypnum && camItemIndex == cii) {
		SetRndSeed(is->_iSeed);
		// skip the seeds used during map-generation
		for (int i = 0; i < 2 * MAXCAMPAIGNSIZE; i++) {
			NextRndSeed();
		}
		// calculate the seed of the map
		for (int i = 0; i < mIdx; i++) {
			DWORD seed = NextRndSeed();
			seed = (seed >> 8) | (seed << 24); // _rotr(seed, 8)
			SetRndSeed(seed);
		}
		NetSendCmdCreateLvl(GetRndSeed(), selCamEntry.ceLevel, selCamEntry.ceDunType);
	}

	available &= ~mask;
	if (available == 0) {
		SyncPlrItemRemove(pnum, cii);
	} else {
		is->_ivalue = available;
	}

	return true;
}

void CalculateGold(int pnum)
{
	ItemStruct* pi;
	int i, gold;

	gold = 0;
	pi = plr._pInvList;
	for (i = NUM_INV_GRID_ELEM; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD)
			gold += pi->_ivalue;
	}

	plr._pGold = gold;
}

DEVILUTION_END_NAMESPACE
