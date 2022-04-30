#include "scrollbar.h"

#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE* scrollBarBackCel;
BYTE* scrollBarThumbCel;
BYTE* scrollBarArrowCel;

void LoadScrollBar()
{
	assert(scrollBarBackCel == NULL);
	assert(scrollBarThumbCel == NULL);
	assert(scrollBarArrowCel == NULL);
	scrollBarBackCel = LoadFileInMem("ui_art\\sb_bg.CEL");
	scrollBarThumbCel = LoadFileInMem("ui_art\\sb_thumb.CEL");
	scrollBarArrowCel = LoadFileInMem("ui_art\\sb_arrow.CEL");
}

void UnloadScrollBar()
{
	MemFreeDbg(scrollBarBackCel);
	MemFreeDbg(scrollBarThumbCel);
	MemFreeDbg(scrollBarArrowCel);
}

DEVILUTION_END_NAMESPACE
