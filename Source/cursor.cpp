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
/** Cursor images CEL */
BYTE *pCursCels;
#ifdef HELLFIRE
BYTE *pCursCels2;
#endif

/** inv_item value */
char pcursinvitem;
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

/*  Maps from objcurs.cel frame number to frame width.
 **If the values are modified, make sure validateCursorAreas does not fail.**
 */
/*constexpr*/ const int InvItemWidth[] = {
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
/*constexpr*/ const int InvItemHeight[] = {
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
constexpr BOOL validateCursorAreas()
{
	static_assert(lengthof(InvItemHeight) == lengthof(InvItemWidth), "Mismatching InvItem tables.");
	for (int i = 0; i < lengthof(InvItemHeight); i++) {
		if (InvItemHeight[i] * InvItemWidth[i] > MAX_CURSOR_AREA) {
			return FALSE;
		}
	}
	return TRUE;
}
static_assert(validateCursorAreas(), "One of the cursor area does not fit to the defined maximum.");*/

void InitCursor()
{
	assert(pCursCels == NULL);
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

void NewCursor(int i)
{
	pcurs = i;
	cursW = InvItemWidth[i];
	cursH = InvItemHeight[i];
}

void InitLevelCursor()
{
	NewCursor(CURSOR_HAND);
	cursmx = ViewX;
	cursmy = ViewY;
	pcurstemp = -1;
	pcursmonst = -1;
	pcursobj = -1;
	pcursitem = -1;
	pcursplr = -1;
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
				trigflag = TRUE;
				ClearPanel();
				if (mis->_miType == MIS_TOWN) {
					copy_cstr(infostr, "Town Portal");
					snprintf(tempstr, sizeof(tempstr), "from %s", plr[mis->_miSource]._pName);
				} else {
					copy_cstr(infostr, "Portal to");
					if (!setlevel)
						copy_cstr(tempstr, "The Unholy Altar");
					else
						copy_cstr(tempstr, "level 15");
				}
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
	//if (sy > PANEL_TOP - 1 && MouseX >= PANEL_LEFT && MouseX < PANEL_LEFT + PANEL_WIDTH && track_isscrolling()) {
	//	sy = PANEL_TOP - 1;
	//}

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

	if (mx < DBORDERX)
		mx = DBORDERX;
	else if (mx > MAXDUNX - 1 - DBORDERX)
		mx = MAXDUNX - 1 - DBORDERX;
	if (my < DBORDERY)
		my = DBORDERY;
	else if (my > MAXDUNY - 1 - DBORDERY)
		my = MAXDUNY - 1 - DBORDERY;

	flipflag = flipy && flipx || (flipy || flipx) && px < TILE_WIDTH / 2;

	pcurstemp = pcursmonst;
	pcursmonst = -1;
	pcursobj = -1;
	pcursitem = -1;
	if (pcursinvitem != -1) {
		gbRedrawFlags |= REDRAW_SPEED_BAR;
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

	if (pcurs == CURSOR_IDENTIFY) {
		cursmx = mx;
		cursmy = my;
		return;
	}

	static_assert(DBORDERX >= 2 && DBORDERY >= 2, "Borders are too small to skip the OOB checks.");
	if (leveltype != DTYPE_TOWN) {
		if (pcurstemp != -1) {
			if (!flipflag) {
				mi = dMonster[mx + 2][my + 1];
				if (mi != 0 && dFlags[mx + 2][my + 1] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
						cursmx = mx + 2; /// BUGFIX: 'mx + 2' (fixed)
						cursmy = my + 1; /// BUGFIX: 'my + 1' (fixed)
						pcursmonst = mi;
					}
				}
			} else {
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
			mi = dMonster[mx + 2][my + 2];
			if (mi != 0 && dFlags[mx + 2][my + 2] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
					cursmx = mx + 2;
					cursmy = my + 2;
					pcursmonst = mi;
				}
			}
			if (!flipflag) {
				mi = dMonster[mx + 1][my];
				if (mi != 0 && dFlags[mx + 1][my] & BFLAG_LIT) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
						cursmx = mx + 1;
						cursmy = my;
						pcursmonst = mi;
					}
				}
			} else {
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
			mi = dMonster[mx + 1][my + 1];
			if (mi != 0 && dFlags[mx + 1][my + 1] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (mi == pcurstemp && monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
					cursmx = mx + 1;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
			if (pcursmonst != -1) {
				if (monster[pcursmonst]._mFlags & (MFLAG_HIDDEN | MFLAG_GOLEM))
					pcursmonst = -1;
				else
					return;
			}
		}
		if (!flipflag) {
			mi = dMonster[mx + 2][my + 1];
			if (mi != 0 && dFlags[mx + 2][my + 1] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
					cursmx = mx + 2;
					cursmy = my + 1;
					pcursmonst = mi;
				}
			}
		} else {
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
		mi = dMonster[mx + 2][my + 2];
		if (mi != 0 && dFlags[mx + 2][my + 2] & BFLAG_LIT) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 4) {
				cursmx = mx + 2;
				cursmy = my + 2;
				pcursmonst = mi;
			}
		}
		if (!flipflag) {
			mi = dMonster[mx + 1][my];
			if (mi != 0 && dFlags[mx + 1][my] & BFLAG_LIT) {
				mi = mi >= 0 ? mi - 1 : -(mi + 1);
				if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
					cursmx = mx + 1;
					cursmy = my;
					pcursmonst = mi;
				}
			}
		} else {
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
		mi = dMonster[mx + 1][my + 1];
		if (mi != 0 && dFlags[mx + 1][my + 1] & BFLAG_LIT) {
			mi = mi >= 0 ? mi - 1 : -(mi + 1);
			if (monster[mi]._mhitpoints >> 6 > 0 && monster[mi].MData->mSelFlag & 2) {
				cursmx = mx + 1;
				cursmy = my + 1;
				pcursmonst = mi;
			}
		}
		if (pcursmonst != -1) {
			if (monster[pcursmonst]._mFlags & (MFLAG_HIDDEN | MFLAG_GOLEM))
				pcursmonst = -1;
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
		if (pcursmonst != -1) {
			if (!towner[pcursmonst]._tSelFlag
			 || (monster[pcursmonst]._mFlags & MFLAG_GOLEM))
				pcursmonst = -1;
			else
				return;
		}
	}

	if (!flipflag) {
		bv = dPlayer[mx + 1][my];
		if (bv != 0) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (bv != myplr && plr[bv]._pHitPoints != 0) {
				cursmx = mx + 1;
				cursmy = my;
				pcursplr = bv;
			}
		}
	} else {
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
		for (xx = -1; xx <= 1; xx++) {
			for (yy = -1; yy <= 1; yy++) {
				if (dFlags[mx + xx][my + yy] & BFLAG_DEAD_PLAYER) {
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
	bv = dPlayer[mx + 1][my + 1];
	if (bv != 0) {
		bv = bv >= 0 ? bv - 1 : -(bv + 1);
		if (bv != myplr && plr[bv]._pHitPoints != 0) {
			cursmx = mx + 1;
			cursmy = my + 1;
			pcursplr = bv;
		}
	}

	if (pcursplr != -1) {
		return;
	}

	if (!flipflag) {
		bv = dObject[mx + 1][my];
		if (bv != 0) {
			bv = bv >= 0 ? bv - 1 : -(bv + 1);
			if (object[bv]._oSelFlag >= 2) {
				cursmx = mx + 1;
				cursmy = my;
				pcursobj = bv;
			}
		}
	} else {
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
	bv = dObject[mx + 1][my + 1];
	if (bv != 0) {
		bv = bv >= 0 ? bv - 1 : -(bv + 1);
		if (object[bv]._oSelFlag >= 2) {
			cursmx = mx + 1;
			cursmy = my + 1;
			pcursobj = bv;
		}
	}

	if (pcursobj != -1)
		return;

	if (!flipflag) {
		bv = dItem[mx + 1][my];
		if (bv > 0) {
			bv--;
			if (item[bv]._iSelFlag >= 2) {
				cursmx = mx + 1;
				cursmy = my;
				pcursitem = bv;
			}
		}
	} else {
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
	bv = dItem[mx + 1][my + 1];
	if (bv > 0) {
		bv--;
		if (item[bv]._iSelFlag >= 2) {
			cursmx = mx + 1;
			cursmy = my + 1;
			pcursitem = bv;
		}
	}
	if (pcursitem == -1) {
		cursmx = mx;
		cursmy = my;
		CheckTrigForce();
		CheckTownPortal();
	}
}

DEVILUTION_END_NAMESPACE
