#include "scrollbar.h"

DEVILUTION_BEGIN_NAMESPACE

Art ArtScrollBarBackground;
Art ArtScrollBarThumb;
Art ArtScrollBarArrow;

void LoadScrollBar()
{
	LoadArt("ui_art\\sb_bg.pcx", &ArtScrollBarBackground);
	assert(ArtScrollBarBackground.w() == SCROLLBAR_BG_WIDTH);
	assert(ArtScrollBarBackground.h() == SCROLLBAR_BG_HEIGHT);
	LoadArt("ui_art\\sb_thumb.pcx", &ArtScrollBarThumb);
	assert(ArtScrollBarThumb.h() == SCROLLBAR_THUMB_HEIGHT);
	LoadArt("ui_art\\sb_arrow.pcx", &ArtScrollBarArrow, 4);
	assert(ArtScrollBarArrow.w() == SCROLLBAR_ARROW_WIDTH);
	assert(ArtScrollBarArrow.h() == SCROLLBAR_ARROW_HEIGHT);
}

void UnloadScrollBar()
{
	ArtScrollBarArrow.Unload();
	ArtScrollBarThumb.Unload();
	ArtScrollBarBackground.Unload();
}

DEVILUTION_END_NAMESPACE
