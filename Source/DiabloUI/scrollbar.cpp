#include "scrollbar.h"

DEVILUTION_BEGIN_NAMESPACE

Art ArtScrollBarBackground;
Art ArtScrollBarThumb;
Art ArtScrollBarArrow;

void LoadScrollBar()
{
	LoadArt("ui_art\\sb_bg.pcx", &ArtScrollBarBackground);
	assert(ArtScrollBarBackground.logical_width == SCROLLBAR_BG_WIDTH);
	assert(ArtScrollBarBackground.frame_height == SCROLLBAR_BG_HEIGHT);
	LoadArt("ui_art\\sb_thumb.pcx", &ArtScrollBarThumb);
	assert(ArtScrollBarThumb.frame_height == SCROLLBAR_THUMB_HEIGHT);
	LoadArt("ui_art\\sb_arrow.pcx", &ArtScrollBarArrow, 4);
	assert(ArtScrollBarArrow.logical_width == SCROLLBAR_ARROW_WIDTH);
	assert(ArtScrollBarArrow.frame_height == SCROLLBAR_ARROW_HEIGHT);
}

void UnloadScrollBar()
{
	ArtScrollBarArrow.Unload();
	ArtScrollBarThumb.Unload();
	ArtScrollBarBackground.Unload();
}

DEVILUTION_END_NAMESPACE
