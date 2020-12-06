#include "scrollbar.h"

DEVILUTION_BEGIN_NAMESPACE

Art ArtScrollBarBackground;
Art ArtScrollBarThumb;
Art ArtScrollBarArrow;

void LoadScrollBar() {
	LoadArt("ui_art\\sb_bg.pcx", &ArtScrollBarBackground);
	LoadArt("ui_art\\sb_thumb.pcx", &ArtScrollBarThumb);
	LoadArt("ui_art\\sb_arrow.pcx", &ArtScrollBarArrow, 4);
}

void UnloadScrollBar() {
	ArtScrollBarArrow.Unload();
	ArtScrollBarThumb.Unload();
	ArtScrollBarBackground.Unload();
}

DEVILUTION_END_NAMESPACE
