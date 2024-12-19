/**
 * @file cursor.cpp
 *
 * Implementation of cursor tracking functionality.
 */
#include "all.h"
//#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

/** Pixel width of the current cursor image */
int cursW;
/** Pixel height of the current cursor image */
int cursH;
/** Cursor images CEL */
BYTE* pCursCels;

/** Index of current cursor image */
int pcursicon = CURSOR_NONE;

/*  Maps from objcurs.cel frame number to frame width.
 **If the values are modified, make sure validateCursorAreas does not fail.**
 */
static const int InvItemWidth[] = {
	// clang-format off
	// Cursors
	0, 33
	// clang-format on
};

/*  Maps from objcurs.cel frame number to frame height.
 **If the values are modified, make sure validateCursorAreas does not fail.**
 */
static const int InvItemHeight[] = {
	// clang-format off
	// Cursors
	0, 29
	// clang-format on
};

void InitCursorGFX()
{
	assert(pCursCels == NULL);

	pCursCels = LoadFileInMem("Data\\Inv\\Objcurs.CEL");

	ClearCursor();
}

void FreeCursorGFX()
{
	MemFreeDbg(pCursCels);
	//ClearCursor();
}

void NewCursor(int i)
{
	assert(i == CURSOR_HAND);
	i = CURSOR_HAND;
	pcursicon = i;
	cursW = InvItemWidth[i];
	cursH = InvItemHeight[i];
}

DEVILUTION_END_NAMESPACE
