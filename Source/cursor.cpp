/**
 * @file cursor.cpp
 *
 * Implementation of cursor tracking functionality.
 */
#include "all.h"
#include "plrctrls.h"

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
BYTE* pCursCels;

/** Current active window */
BYTE pcurswnd;
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
/** Current highlighted tile row/column */
POS32 pcurspos;
/** Index of current cursor image */
int pcursicon;
/** The targeting mode (TGT_*) */
int pcurstgt;

/*  Maps from objcurs.cel frame number to frame width.
 **If the values are modified, make sure validateCursorAreas does not fail.**
 */
/*constexpr*/ const int InvItemWidth[(int)CURSOR_FIRSTITEM + (int)NUM_ICURS] = {
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
/*constexpr*/ const int InvItemHeight[(int)CURSOR_FIRSTITEM + (int)NUM_ICURS] = {
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

#if USE_PATCH
	pCursCels = LoadFileInMem("Data\\Inv\\Objcurs.CEL");
	if (SwapLE32(*(DWORD*)pCursCels) != lengthof(InvItemWidth) - 1) {
		app_fatal("Invalid file %s in the mpq.", "Data\\Inv\\Objcurs.CEL");
	}
#else
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
#endif // HELLFIRE
#endif // USE_PATCH
	ClearCursor();
}

void FreeCursorGFX()
{
	MemFreeDbg(pCursCels);
	//ClearCursor();
}

void NewCursor(int i)
{
	pcursicon = i;
	cursW = InvItemWidth[i];
	cursH = InvItemHeight[i];
	pcurstgt = TGT_NORMAL;
	switch (i) {
	case CURSOR_NONE:
	case CURSOR_HAND:
		break;
	case CURSOR_IDENTIFY:
	case CURSOR_REPAIR:
	case CURSOR_RECHARGE:
	case CURSOR_OIL:
		// skip monster/player/object/etc targeting if the player is using a skill/spell to target an item
		pcurstgt = TGT_NONE;
		break;
	case CURSOR_DISARM:
		pcurstgt = TGT_OBJECT;
		break;
	case CURSOR_TELEKINESIS:
		break;
	case CURSOR_RESURRECT:
		pcurstgt = TGT_DEAD;
		break;
	case CURSOR_TELEPORT:
		break;
	case CURSOR_HEALOTHER:
		pcurstgt = TGT_PLAYER;
		break;
	case CURSOR_HOURGLASS:
		break;
	default:
		// pcursicon >= CURSOR_FIRSTITEM
		// skip monster/player/object/etc targeting if the player is holding an item
		pcurstgt = TGT_NONE;
		break;
	}
}

void ChangeTarget()
{
	int cursor = pcursicon;
	if (cursor == CURSOR_HOURGLASS) {
		cursor = gnTimeoutCurs;
	}
	if (cursor == CURSOR_HAND || cursor == CURSOR_TELEKINESIS || cursor == CURSOR_TELEPORT) {
		if (pcurstgt == TGT_NORMAL) {
			pcurstgt = TGT_ITEM;
		} else if (pcurstgt == TGT_ITEM) {
			pcurstgt = TGT_OBJECT;
		} else {
			pcurstgt = TGT_NORMAL;
		}
	}
}

void InitLevelCursor()
{
	NewCursor(myplr._pHoldItem._itype == ITYPE_NONE ? CURSOR_HAND : myplr._pHoldItem._iCurs + CURSOR_FIRSTITEM);
	ClearCursor();
	pcursmonst = MON_NONE;
	CheckCursMove();
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	plrctrls_after_check_curs_move();
#endif
}

void CheckTownPortal()
{
	MissileStruct* mis;
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
			int dx = pcurspos.x - (mis->_mix - 1);
			int dy = pcurspos.y - (mis->_miy - 1);
			if (abs(dx) <= 1 && abs(dy) <= 1 // select the 3x3 square around (-1;-1)
			 && abs(dx - dy) < 2) {          // exclude the top left and bottom right positions
				pcurstrig = MAXTRIGGERS + missileactive[i] + 1;
				pcurspos.x = mis->_mix;
				pcurspos.y = mis->_miy;
			}
		}
	}
}

void CheckCursMove()
{
	int pnum, sx, sy, /*fx, fy,*/ mx, my, tx, ty, px, py, mi;
	int pcurstemp; // Previously highlighted monster
	bool flipflag, flipx, flipy;

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
	pcurswnd = WND_NONE;

	static_assert(MDM_ALIVE == 0, "BitOr optimization of CheckCursMove expects MDM_ALIVE to be zero.");
	static_assert(STORE_NONE == 0, "BitOr optimization of CheckCursMove expects STORE_NONE to be zero.");
	if (gbDeathflag /*| gbDoomflag*/ | gbSkillListFlag | gbQtextflag | stextflag)
		return;

	sx = MousePos.x;
	sy = MousePos.y;

	if (POS_IN_RECT(sx, sy, gnWndBeltX, gnWndBeltY, BELT_WIDTH, BELT_HEIGHT))
		pcurswnd = WND_BELT;
	for (int i = 0; i < gnNumActiveWindows; i++) {
		switch (gaActiveWindows[i]) {
		case WND_INV:
			if (POS_IN_RECT(sx, sy, gnWndInvX, gnWndInvY, SPANEL_WIDTH, SPANEL_HEIGHT))
				pcurswnd = WND_INV;
			break;
		case WND_CHAR:
			if (POS_IN_RECT(sx, sy, gnWndCharX, gnWndCharY, SPANEL_WIDTH, SPANEL_HEIGHT))
				pcurswnd = WND_CHAR;
			break;
		case WND_BOOK:
			if (POS_IN_RECT(sx, sy, gnWndBookX, gnWndBookY, SPANEL_WIDTH, SPANEL_HEIGHT))
				pcurswnd = WND_BOOK;
			break;
		case WND_TEAM:
			if (POS_IN_RECT(sx, sy, gnWndTeamX, gnWndTeamY, SPANEL_WIDTH, SPANEL_HEIGHT))
				pcurswnd = WND_TEAM;
			break;
		case WND_QUEST:
			if (POS_IN_RECT(sx, sy, gnWndQuestX, gnWndQuestY, SPANEL_WIDTH, SPANEL_HEIGHT))
				pcurswnd = WND_QUEST;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}
	// skip monster/player/object/etc targeting if hovering over a window.
	if (pcurswnd != WND_NONE) {
		// skip item targeting if the cursor can not target an item (in inventory)
		if (pcursicon == CURSOR_HAND || pcursicon == CURSOR_IDENTIFY || pcursicon == CURSOR_REPAIR || pcursicon == CURSOR_RECHARGE || pcursicon == CURSOR_OIL) {
			if (pcurswnd == WND_INV)
				pcursinvitem = CheckInvItem();
			else if (pcurswnd == WND_BELT)
				pcursinvitem = CheckInvBelt();
		}
		return;
	}

	if (gbZoomInFlag) {
		sx >>= 1;
		sy >>= 1;
	}

	sx += gsMouseVp._vOffsetX;
	sy += gsMouseVp._vOffsetY;

	//if (ScrollInfo._sdir != SDIR_NONE) {
		sx -= ScrollInfo._sxoff;
		sy -= ScrollInfo._syoff;

	//	// Predict the next frame when walking to avoid input jitter
	//	fx = myplr._pVar6 >> PLR_WALK_SHIFT; // WALK_XOFF
	//	fx -= (myplr._pVar6 + myplr._pVar4) >> PLR_WALK_SHIFT; // WALK_XOFF + WALK_XVEL
	//	fy = myplr._pVar7 >> PLR_WALK_SHIFT; // WALK_YOFF
	//	fy -= (myplr._pVar7 + myplr._pVar5) >> PLR_WALK_SHIFT; // WALK_YOFF + WALK_YVEL
	//	sx -= fx;
	//	sy -= fy;
	//}

	// Center player tile on screen
	mx = ViewX + gsMouseVp._vShiftX;
	my = ViewY + gsMouseVp._vShiftY;

	// ensure sx/y are positive
	sx += TILE_WIDTH;
	// assert(sx >= 0);
	sy += TILE_HEIGHT;
	// assert(sy >= 0);

	tx = ((unsigned)sx) / TILE_WIDTH;
	ty = ((unsigned)sy) / TILE_HEIGHT;
	tx--; // revert added value
	ty--;
	SHIFT_GRID(mx, my, tx, ty);

	// Shift position to match diamond grid aligment
	px = ((unsigned)sx) % TILE_WIDTH;
	py = ((unsigned)sy) % TILE_HEIGHT;

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

	pcurspos.x = mx;
	pcurspos.y = my;

	int curmon[5] = { 0 };
	int curobj[5] = { 0 };
	int curitem[3] = { 0 };
	int curplr[3] = { 0 };
	int deadplr[1] = { 0 };
	const int selFlag[5] = { 4, 4, 2, 1, 2 };
	int offx[5] = { 2, 2, 1, 0, 1 };
	int offy[5] = { 1, 2, 0, 0, 1 };
	static_assert(DBORDERX >= 2 && DBORDERY >= 2, "Borders are too small to skip the OOB checks.");
	if (dFlags[mx][my] & BFLAG_VISIBLE) {
		curmon[3] = dMonster[mx][my];
		curobj[3] = dObject[mx][my];
		curplr[1] = dPlayer[mx][my];
		curitem[1] = dItem[mx][my];
		if (dFlags[mx][my] & BFLAG_DEAD_PLAYER) {
			for (pnum = 0; pnum < MAX_PLRS; pnum++) {
				if (/*pnum != mypnum && plr._pmode == PM_DEATH &&*/ plr._px == mx && plr._py == my && plr._pActive && plr._pDunLevel == currLvl._dLevelIdx) 			{
					deadplr[0] = pnum;
				}
			}

		}
	}
	if (dFlags[mx + 1][my + 1] & BFLAG_VISIBLE) {
		curmon[4] = dMonster[mx + 1][my + 1];
		curobj[4] = dObject[mx + 1][my + 1];
		curplr[2] = dPlayer[mx + 1][my + 1];
		curitem[2] = dItem[mx + 1][my + 1];
	}
	if (dFlags[mx + 2][my + 2] & BFLAG_VISIBLE) {
		curmon[1] = dMonster[mx + 2][my + 2];
		curobj[1] = dObject[mx + 2][my + 2];
	}
	if (flipflag) {
		if (dFlags[mx + 0][my + 1] & BFLAG_VISIBLE) {
			curmon[2] = dMonster[mx + 0][my + 1];
			curobj[2] = dObject[mx + 0][my + 1];
			curplr[0] = dPlayer[mx + 0][my + 1];
			curitem[0] = dItem[mx + 0][my + 1];
		}
		if (dFlags[mx + 1][my + 2] & BFLAG_VISIBLE) {
			curmon[0] = dMonster[mx + 1][my + 2];
			curobj[0] = dObject[mx + 1][my + 2];
		}
	} else {
		if (dFlags[mx + 1][my + 0] & BFLAG_VISIBLE) {
			curmon[2] = dMonster[mx + 1][my + 0];
			curobj[2] = dObject[mx + 1][my + 0];
			curplr[0] = dPlayer[mx + 1][my + 0];
			curitem[0] = dItem[mx + 1][my + 0];
		}
		if (dFlags[mx + 2][my + 1] & BFLAG_VISIBLE) {
			curmon[0] = dMonster[mx + 2][my + 1];
			curobj[0] = dObject[mx + 2][my + 1];
		}
	}
	offx[0] = flipflag ? 1 : 2; offy[0] = flipflag ? 2 : 1;
	offx[2] = flipflag ? 0 : 1; offy[2] = flipflag ? 1 : 0;

	switch (pcurstgt) {
	case TGT_NORMAL:
		break;
	case TGT_ITEM:
		// select an item
		for (int i = 2; i >= 0; i--) {
			mi = curitem[i];
			if (mi > 0) {
				mi = mi - 1;
				if (!(items[mi]._iSelFlag & selFlag[i + 2])) {
					continue;
				}
				pcursitem = mi;
				pcurspos.x = mx + offx[i + 2];
				pcurspos.y = my + offy[i + 2];
				return;
			}
		}
		return;
	case TGT_OBJECT:
		// select an object
		for (int i = 4; i >= 0; i--) {
			mi = curobj[i];
			if (mi != 0) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (!(objects[mi]._oSelFlag & selFlag[i])) {
					continue;
				}
				pcursobj = mi;
				pcurspos.x = mx + offx[i];
				pcurspos.y = my + offy[i];
				return;
			}
		}
		return;
	case TGT_PLAYER:
		// select a live player
		for (int i = 2; i >= 0; i--) {
			mi = curplr[i];
			if (mi != 0) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi == mypnum || plx(mi)._pHitPoints < (1 << 6)) {
					continue;
				}
				pcursplr = mi;
				pcurspos.x = mx + offx[i + 2];
				pcurspos.y = my + offy[i + 2];
				return;
			}
		}
		return;
	case TGT_DEAD:
		// select a dead player
		if (deadplr[0] != 0) {
			pcursplr = deadplr[0];
			pcurspos.x = mx;
			pcurspos.y = my;
			return;
		}
		return;
	case TGT_NONE:
		return;
	default:
		ASSUME_UNREACHABLE
	}
	// select the previous monster/npc
	if (pcursmonst != MON_NONE) {
		for (int i = 4; i >= 0; i--) {
			mi = curmon[i];
			if (mi != 0) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi != pcursmonst) {
					continue;
				}
				if (!(monsters[mi]._mSelFlag & selFlag[i])) {
					continue;
				}
				pcursmonst = mi;
				pcurspos.x = mx + offx[i];
				pcurspos.y = my + offy[i];
				return;
			}
		}
	}
	// select a monster/npc
	for (int i = 4; i >= 0; i--) {
		mi = curmon[i];
		if (mi != 0) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (monsters[mi]._mhitpoints < (1 << 6) || (monsters[mi]._mFlags & MFLAG_HIDDEN)) {
				continue;
			}
			if (!(monsters[mi]._mSelFlag & selFlag[i])) {
				continue;
			}
			if (mi < MAX_MINIONS) {
				break;
			}
			pcursmonst = mi;
			pcurspos.x = mx + offx[i];
			pcurspos.y = my + offy[i];
			return;
		}
	}
	// select a live player
	for (int i = 2; i >= 0; i--) {
		mi = curplr[i];
		if (mi != 0) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (mi == mypnum || plx(mi)._pHitPoints < (1 << 6)) {
				continue;
			}
			pcursplr = mi;
			pcurspos.x = mx + offx[i + 2];
			pcurspos.y = my + offy[i + 2];
			return;
		}
	}
	// select a dead player
	if (deadplr[0] != 0) {
		pcursplr = deadplr[0];
		pcurspos.x = mx;
		pcurspos.y = my;
		return;
	}
	// select an object
	for (int i = 4; i >= 0; i--) {
		mi = curobj[i];
		if (mi != 0) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (!(objects[mi]._oSelFlag & selFlag[i])) {
				continue;
			}
			pcursobj = mi;
			pcurspos.x = mx + offx[i];
			pcurspos.y = my + offy[i];
			return;
		}
	}
	// select an item
	for (int i = 2; i >= 0; i--) {
		mi = curitem[i];
		if (mi > 0) {
			mi = mi - 1;
			if (!(items[mi]._iSelFlag & selFlag[i + 2])) {
				continue;
			}
			pcursitem = mi;
			pcurspos.x = mx + offx[i + 2];
			pcurspos.y = my + offy[i + 2];
			return;
		}
	}

	pcurspos.x = mx;
	pcurspos.y = my;
	CheckTrigForce();
	CheckTownPortal();
}

DEVILUTION_END_NAMESPACE
