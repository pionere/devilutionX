/**
 * @file cursor.cpp
 *
 * Implementation of cursor tracking functionality.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
  * Container to hold the cached properties of the mouse-viewport.
  *
  * _vColumns: the number of columns on the screen 
  * _vRows: the number of rows on the screen.
  * _vOffsetX: the base X-offset to draw the tiles in the back buffer.
  * _vOffsetY: the base Y-offset to draw the tiles in the back buffer.
  * _vShiftX: the base offset to ViewX.
  * _vShiftY: the base offset to ViewY.
*/
ViewportStruct gsMouseVp;

/** Pixel width of the current cursor image */
int cursW;
/** Pixel height of the current cursor image */
int cursH;
/** Current highlighted monster */
int pcursmonst = MON_NONE;
/** Cursor images CEL */
BYTE *pCursCels;

/** inv_item value */
BYTE pcursinvitem;
/** Current highlighted item */
BYTE pcursitem;
/** Current highlighted object */
BYTE pcursobj;
/** Current highlighted player */
BYTE pcursplr;
/* Current highlighted trigger
 * 0 - MAXTRIGGERS : standard triggers
 * MAXTRIGGERS+1...: missile-id of townportals
 */
int pcurstrig;
/** Current highlighted tile row */
int cursmx;
/** Current highlighted tile column */
int cursmy;
/** Index of current cursor image */
int pcurs;

/*  Maps from objcurs.cel frame number to frame width.
 **If the values are modified, make sure validateCursorAreas does not fail.**
 */
/*constexpr*/ const int InvItemWidth[CURSOR_FIRSTITEM + NUM_ICURS] = {
	// clang-format off
	// Cursors
	0, 33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 23,
	// Items
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
#ifdef HELLFIRE
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX
#endif
	// clang-format on
};

/*  Maps from objcurs.cel frame number to frame height.
 **If the values are modified, make sure validateCursorAreas does not fail.**
 */
/*constexpr*/ const int InvItemHeight[CURSOR_FIRSTITEM + NUM_ICURS] = {
	// clang-format off
	// Cursors
	0, 29, 32, 32, 32, 32, 32, 32, 32, 32, 32, 35,
	// Items
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
#ifdef HELLFIRE
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX, 1 * INV_SLOT_SIZE_PX,
	2 * INV_SLOT_SIZE_PX, 2 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX,
	3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX, 3 * INV_SLOT_SIZE_PX
#endif
	// clang-format on
};

/* commented out because even the latest MSVC compiler requires a special flag (/Zc:externConstexpr) to handle this
   the drawing logic extends the cursor to whole DWORDs (and adds a border)
   This means the check is not exact!
constexpr bool validateCursorAreas()
{
	static_assert(lengthof(InvItemHeight) == lengthof(InvItemWidth), "Mismatching InvItem tables.");
	for (int i = 0; i < lengthof(InvItemHeight); i++) {
		if (InvItemHeight[i] * InvItemWidth[i] > MAX_CURSOR_AREA) {
			return false;
		}
	}
	return true;
}
static_assert(validateCursorAreas(), "One of the cursor area does not fit to the defined maximum.");*/

void InitCursorGFX()
{
	assert(pCursCels == NULL);
	
#ifdef HELLFIRE
	size_t sizeA, sizeB;
	BYTE *aCursCels, *bCursCels;

	aCursCels = LoadFileInMem("Data\\Inv\\Objcurs.CEL", &sizeA);
	bCursCels = LoadFileInMem("Data\\Inv\\Objcurs2.CEL", &sizeB);

	pCursCels = CelMerge(aCursCels, sizeA, bCursCels, sizeB);

	mem_free_dbg(aCursCels);
	mem_free_dbg(bCursCels);
#else
	pCursCels = LoadFileInMem("Data\\Inv\\Objcurs.CEL");
#endif
	ClearCursor();
}

void FreeCursorGFX()
{
	MemFreeDbg(pCursCels);
	//ClearCursor();
}

void NewCursor(int i)
{
	pcurs = i;
	cursW = InvItemWidth[i];
	cursH = InvItemHeight[i];
}

void InitLevelCursor()
{
	NewCursor(myplr._pHoldItem._itype == ITYPE_NONE ? CURSOR_HAND : myplr._pHoldItem._iCurs + CURSOR_FIRSTITEM);
	cursmx = ViewX;
	cursmy = ViewY;
	pcursmonst = MON_NONE;
	pcursobj = OBJ_NONE;
	pcursitem = ITEM_NONE;
	pcursplr = PLR_NONE;
	ClearCursor();
}

void CheckTownPortal()
{
	MissileStruct *mis;
	int i;

	for (i = 0; i < nummissiles; i++) {
		mis = &missile[missileactive[i]];
		if (mis->_miType == MIS_TOWN || mis->_miType == MIS_RPORTAL) {
			/*      ^
			 *      |
			 * ----++------>
			 *    +++
			 *    ++|
			 *      |
			 */
			int dx = cursmx - (mis->_mix - 1);
			int dy = cursmy - (mis->_miy - 1);
			if (abs(dx) <= 1 && abs(dy) <= 1 &&	// select the 3x3 square around (-1;-1)
				abs(dx - dy) < 2) {				// exclude the top left and bottom right positions
				pcurstrig = MAXTRIGGERS + missileactive[i] + 1;
				cursmx = mis->_mix;
				cursmy = mis->_miy;
			}
		}
	}
}

void CheckCursMove()
{
	int pnum, sx, sy, fx, fy, mx, my, tx, ty, px, py, xx, yy, mi;
	int pcurstemp; // Previously highlighted monster
	char bv;
	bool flipflag, flipx, flipy;

	sx = MouseX;
	sy = MouseY;

	//if (sy > PANEL_TOP - 1 && MouseX >= PANEL_LEFT && MouseX < PANEL_LEFT + PANEL_WIDTH && track_isscrolling()) {
	//	sy = PANEL_TOP - 1;
	//}

	if (gbZoomInFlag) {
		sx >>= 1;
		sy >>= 1;
	}

	sx -= ScrollInfo._sxoff - gsMouseVp._vOffsetX;
	sy -= ScrollInfo._syoff - gsMouseVp._vOffsetY;

	// Predict the next frame when walking to avoid input jitter
	if (ScrollInfo._sdir != SDIR_NONE) {
		fx = myplr._pVar6 / 256; // WALK_XOFF
		fx -= (myplr._pVar6 + myplr._pVar4) / 256; // WALK_XOFF + WALK_XVEL
		fy = myplr._pVar7 / 256; // WALK_YOFF
		fy -= (myplr._pVar7 + myplr._pVar5) / 256; // WALK_YOFF + WALK_YVEL
		sx -= fx;
		sy -= fy;
	}

	// Center player tile on screen
	mx = ViewX + gsMouseVp._vShiftX;
	my = ViewY + gsMouseVp._vShiftY;

	tx = sx / TILE_WIDTH;
	ty = sy / TILE_HEIGHT;
	SHIFT_GRID(mx, my, tx, ty);

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

	flipflag = (flipy && flipx) || ((flipy || flipx) && px < TILE_WIDTH / 2);

	// limit the position to the 'live' dungeon
	if (mx < DBORDERX)
		mx = DBORDERX;
	else if (mx > MAXDUNX - 1 - DBORDERX)
		mx = MAXDUNX - 1 - DBORDERX;
	if (my < DBORDERY)
		my = DBORDERY;
	else if (my > MAXDUNY - 1 - DBORDERY)
		my = MAXDUNY - 1 - DBORDERY;

	pcurstemp = pcursmonst;
	pcursmonst = MON_NONE;
	pcursobj = OBJ_NONE;
	pcursitem = ITEM_NONE;
	//if (pcursinvitem != INVITEM_NONE) {
	//	gbRedrawFlags |= REDRAW_SPEED_BAR;
	//}
	pcursinvitem = INVITEM_NONE;
	pcursplr = PLR_NONE;
	pcurstrig = -1;

	static_assert(MDM_ALIVE == 0, "BitOr optimization of CheckCursMove expects MDM_ALIVE to be zero.");	
	static_assert(STORE_NONE == 0, "BitOr optimization of CheckCursMove expects STORE_NONE to be zero.");
	if (gbDeathflag | gbDoomflag | gbSkillListFlag | gbQtextflag | stextflag)
		return;
	if (pcurs >= CURSOR_FIRSTITEM) {
		cursmx = mx;
		cursmy = my;
		return;
	}
	if ((gbInvflag | gbSbookflag | gbTeamFlag) && MouseX > RIGHT_PANEL && MouseY <= SPANEL_HEIGHT) {
		if (gbInvflag)
			pcursinvitem = CheckInvItem();
		return;
	}
	if ((gbChrflag | gbQuestlog) && MouseX < SPANEL_WIDTH && MouseY <= SPANEL_HEIGHT) {
		return;
	}

	if (MouseX >= InvRect[SLOTXY_BELT_FIRST].X
	 && MouseX <= InvRect[SLOTXY_BELT_LAST].X + INV_SLOT_SIZE_PX
	 && MouseY >= SCREEN_HEIGHT - InvRect[SLOTXY_BELT_FIRST].Y  - INV_SLOT_SIZE_PX
	 && MouseY <= SCREEN_HEIGHT - InvRect[SLOTXY_BELT_LAST].Y) {
		pcursinvitem = CheckInvBelt();
		return;
	}

	if (pcurs == CURSOR_IDENTIFY) {
		cursmx = mx;
		cursmy = my;
		return;
	}

	static_assert(DBORDERX >= 2 && DBORDERY >= 2, "Borders are too small to skip the OOB checks.");
	if (currLvl._dType != DTYPE_TOWN) {
		if (pcurstemp != MON_NONE /*&& pcurstemp >= MAX_MINIONS*/
		 && monsters[pcurstemp]._mhitpoints >= (1 << 6)
		 && !(monsters[pcurstemp]._mFlags & MFLAG_HIDDEN)) {
			if (!flipflag) {
				mi = dMonster[mx + 2][my + 1];
				if (mi != 0 && (dFlags[mx + 2][my + 1] & BFLAG_VISIBLE)) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monsters[mi]._mSelFlag & 4) {
						cursmx = mx + 2; /// BUGFIX: 'mx + 2' (fixed)
						cursmy = my + 1; /// BUGFIX: 'my + 1' (fixed)
						pcursmonst = mi;
					}
				}
			} else {
				mi = dMonster[mx + 1][my + 2];
				if (mi != 0 && (dFlags[mx + 1][my + 2] & BFLAG_VISIBLE)) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monsters[mi]._mSelFlag & 4) {
						cursmx = mx + 1;
						cursmy = my + 2;
						pcursmonst = mi;
					}
				}
			}
			mi = dMonster[mx + 2][my + 2];
			if (mi != 0 && (dFlags[mx + 2][my + 2] & BFLAG_VISIBLE)) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi == pcurstemp && monsters[mi]._mSelFlag & 4) {
					cursmx = mx + 2;
					cursmy = my + 2;
					pcursmonst = mi;
				}
			}
			if (!flipflag) {
				mi = dMonster[mx + 1][my];
				if (mi != 0 && (dFlags[mx + 1][my] & BFLAG_VISIBLE)) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monsters[mi]._mSelFlag & 2) {
						cursmx = mx + 1;
						cursmy = my;
						pcursmonst = mi;
					}
				}
			} else {
				mi = dMonster[mx][my + 1];
				if (mi != 0 && (dFlags[mx][my + 1] & BFLAG_VISIBLE)) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monsters[mi]._mSelFlag & 2) {
						cursmx = mx;
						cursmy = my + 1;
						pcursmonst = mi;
					}
				}
			}
			mi = dMonster[mx][my];
			if (mi != 0 && (dFlags[mx][my] & BFLAG_VISIBLE)) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi == pcurstemp && monsters[mi]._mSelFlag & 1) {
					cursmx = mx;
					cursmy = my;
					pcursmonst = mi;
				}
			}
			mi = dMonster[mx + 1][my + 1];
			if (mi != 0 && (dFlags[mx + 1][my + 1] & BFLAG_VISIBLE)) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi == pcurstemp && monsters[mi]._mSelFlag & 2) {
					cursmx = mx + 1;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
			if (pcursmonst != MON_NONE) {
				return;
			}
		}
		if (!flipflag) {
			mi = dMonster[mx + 2][my + 1];
			if (mi != 0 && (dFlags[mx + 2][my + 1] & BFLAG_VISIBLE)) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monsters[mi]._mhitpoints >= (1 << 6) && monsters[mi]._mSelFlag & 4) {
					cursmx = mx + 2;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
		} else {
			mi = dMonster[mx + 1][my + 2];
			if (mi != 0 && (dFlags[mx + 1][my + 2] & BFLAG_VISIBLE)) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monsters[mi]._mhitpoints >= (1 << 6) && monsters[mi]._mSelFlag & 4) {
					cursmx = mx + 1;
					cursmy = my + 2;
					pcursmonst = mi;
				}
			}
		}
		mi = dMonster[mx + 2][my + 2];
		if (mi != 0 && (dFlags[mx + 2][my + 2] & BFLAG_VISIBLE)) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (monsters[mi]._mhitpoints >= (1 << 6) && monsters[mi]._mSelFlag & 4) {
				cursmx = mx + 2;
				cursmy = my + 2;
				pcursmonst = mi;
			}
		}
		if (!flipflag) {
			mi = dMonster[mx + 1][my];
			if (mi != 0 && (dFlags[mx + 1][my] & BFLAG_VISIBLE)) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monsters[mi]._mhitpoints >= (1 << 6) && monsters[mi]._mSelFlag & 2) {
					cursmx = mx + 1;
					cursmy = my;
					pcursmonst = mi;
				}
			}
		} else {
			mi = dMonster[mx][my + 1];
			if (mi != 0 && (dFlags[mx][my + 1] & BFLAG_VISIBLE)) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monsters[mi]._mhitpoints >= (1 << 6) && monsters[mi]._mSelFlag & 2) {
					cursmx = mx;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
		}
		mi = dMonster[mx][my];
		if (mi != 0 && (dFlags[mx][my] & BFLAG_VISIBLE)) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (monsters[mi]._mhitpoints >= (1 << 6) && monsters[mi]._mSelFlag & 1) {
				cursmx = mx;
				cursmy = my;
				pcursmonst = mi;
			}
		}
		mi = dMonster[mx + 1][my + 1];
		if (mi != 0 && (dFlags[mx + 1][my + 1] & BFLAG_VISIBLE)) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (monsters[mi]._mhitpoints >= (1 << 6) && monsters[mi]._mSelFlag & 2) {
				cursmx = mx + 1;
				cursmy = my + 1;
				pcursmonst = mi;
			}
		}
		if (pcursmonst != MON_NONE) {
			if (pcursmonst < MAX_MINIONS || monsters[pcursmonst]._mFlags & MFLAG_HIDDEN)
				pcursmonst = MON_NONE;
			else
				return;
		}
	} else {
		if (!flipflag) {
			mi = dMonster[mx + 1][my];
			if (mi > 0) {
				pcursmonst = mi - 1;
				cursmx = mx + 1;
				cursmy = my;
			}
		} else {
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
		mi = dMonster[mx + 1][my + 1];
		if (mi > 0) {
			pcursmonst = mi - 1;
			cursmx = mx + 1;
			cursmy = my + 1;
		}
		if (pcursmonst != MON_NONE) {
			//if (/*pcursmonst < MAX_MINIONS ||*/!towners[pcursmonst]._tSelFlag)
			//	pcursmonst = MON_NONE;
			//else
				return;
		}
	}

	if (!flipflag) {
		bv = dPlayer[mx + 1][my];
		if (bv != 0 && (dFlags[mx + 1][my] & BFLAG_VISIBLE)) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (bv != mypnum && plx(bv)._pHitPoints >= (1 << 6)) {
				cursmx = mx + 1;
				cursmy = my;
				pcursplr = bv;
			}
		}
	} else {
		bv = dPlayer[mx][my + 1];
		if (bv != 0 && (dFlags[mx][my + 1] & BFLAG_VISIBLE)) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (bv != mypnum && plx(bv)._pHitPoints >= (1 << 6)) {
				cursmx = mx;
				cursmy = my + 1;
				pcursplr = bv;
			}
		}
	}
	bv = dPlayer[mx][my];
	if (bv != 0 && (dFlags[mx][my] & BFLAG_VISIBLE)) {
		bv = bv >= 0 ? bv - 1 : -(bv + 1);
		if (bv != mypnum) {
			cursmx = mx;
			cursmy = my;
			pcursplr = bv;
		}
	}
	if ((dFlags[mx][my] & (BFLAG_DEAD_PLAYER | BFLAG_VISIBLE)) == (BFLAG_DEAD_PLAYER | BFLAG_VISIBLE)) {
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (plr._px == mx && plr._py == my && pnum != mypnum) {
				cursmx = mx;
				cursmy = my;
				pcursplr = pnum;
			}
		}
	}
	if (pcurs == CURSOR_RESURRECT) {
		for (xx = -1; xx <= 1; xx++) {
			for (yy = -1; yy <= 1; yy++) {
				if ((dFlags[mx + xx][my + yy] & (BFLAG_DEAD_PLAYER | BFLAG_VISIBLE)) == (BFLAG_DEAD_PLAYER | BFLAG_VISIBLE)) {
					for (pnum = 0; pnum < MAX_PLRS; pnum++) {
						if (plr._px == mx + xx && plr._py == my + yy && pnum != mypnum) {
							cursmx = mx + xx;
							cursmy = my + yy;
							pcursplr = pnum;
						}
					}
				}
			}
		}
	}
	bv = dPlayer[mx + 1][my + 1];
	if (bv != 0 && (dFlags[mx + 1][my + 1] & BFLAG_VISIBLE)) {
		bv = bv >= 0 ? bv - 1 : -(bv + 1);
		if (bv != mypnum && plx(bv)._pHitPoints >= (1 << 6)) {
			cursmx = mx + 1;
			cursmy = my + 1;
			pcursplr = bv;
		}
	}

	if (pcursplr != PLR_NONE) {
		return;
	}

	if (!flipflag) {
		bv = dObject[mx + 1][my];
		if (bv != 0) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (objects[bv]._oSelFlag >= 2) {
				cursmx = mx + 1;
				cursmy = my;
				pcursobj = bv;
			}
		}
	} else {
		bv = dObject[mx][my + 1];
		if (bv != 0) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (objects[bv]._oSelFlag >= 2) {
				cursmx = mx;
				cursmy = my + 1;
				pcursobj = bv;
			}
		}
	}
	bv = dObject[mx][my];
	if (bv != 0) {
		bv = bv >= 0 ? bv - 1 : -(bv + 1);
		if (objects[bv]._oSelFlag == 1 || objects[bv]._oSelFlag == 3) {
			cursmx = mx;
			cursmy = my;
			pcursobj = bv;
		}
	}
	bv = dObject[mx + 1][my + 1];
	if (bv != 0) {
		bv = bv >= 0 ? bv - 1 : -(bv + 1);
		if (objects[bv]._oSelFlag >= 2) {
			cursmx = mx + 1;
			cursmy = my + 1;
			pcursobj = bv;
		}
	}

	if (pcursobj != OBJ_NONE)
		return;

	if (!flipflag) {
		bv = dItem[mx + 1][my];
		if (bv > 0) {
			bv--;
			if (items[bv]._iSelFlag >= 2) {
				cursmx = mx + 1;
				cursmy = my;
				pcursitem = bv;
			}
		}
	} else {
		bv = dItem[mx][my + 1];
		if (bv > 0) {
			bv--;
			if (items[bv]._iSelFlag >= 2) {
				cursmx = mx;
				cursmy = my + 1;
				pcursitem = bv;
			}
		}
	}
	bv = dItem[mx][my];
	if (bv > 0) {
		bv--;
		if (items[bv]._iSelFlag == 1 || items[bv]._iSelFlag == 3) {
			cursmx = mx;
			cursmy = my;
			pcursitem = bv;
		}
	}
	bv = dItem[mx + 1][my + 1];
	if (bv > 0) {
		bv--;
		if (items[bv]._iSelFlag >= 2) {
			cursmx = mx + 1;
			cursmy = my + 1;
			pcursitem = bv;
		}
	}
	if (pcursitem == ITEM_NONE) {
		cursmx = mx;
		cursmy = my;
		CheckTrigForce();
		CheckTownPortal();
	}
}

DEVILUTION_END_NAMESPACE
