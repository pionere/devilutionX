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
int pcursicon;

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
	pcursicon = i;
	cursW = 33; // InvItemWidth[i];
	cursH = 29; // InvItemHeight[i];
}

DEVILUTION_END_NAMESPACE
