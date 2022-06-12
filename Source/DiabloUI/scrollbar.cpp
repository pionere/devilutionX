#include "scrollbar.h"

#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

CelImageBuf* scrollBarBackCel;
CelImageBuf* scrollBarThumbCel;
CelImageBuf* scrollBarArrowCel;

void LoadScrollBar()
{
	assert(scrollBarBackCel == NULL);
	assert(scrollBarThumbCel == NULL);
	assert(scrollBarArrowCel == NULL);
	scrollBarBackCel = CelLoadImage("ui_art\\sb_bg.CEL", SCROLLBAR_BG_WIDTH);
	scrollBarThumbCel = CelLoadImage("ui_art\\sb_thumb.CEL", SCROLLBAR_THUMB_WIDTH);
	scrollBarArrowCel = CelLoadImage("ui_art\\sb_arrow.CEL", SCROLLBAR_ARROW_WIDTH);
}

void UnloadScrollBar()
{
	MemFreeDbg(scrollBarBackCel);
	MemFreeDbg(scrollBarThumbCel);
	MemFreeDbg(scrollBarArrowCel);
}

DEVILUTION_END_NAMESPACE
