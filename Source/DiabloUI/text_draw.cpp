#include "text_draw.h"

#include "DiabloUI/art_draw.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

static int AlignXOffset(int flags, const SDL_Rect &dest, int w)
{
	if (flags & UIS_CENTER)
		return (dest.w - w) / 2;
	if (flags & UIS_RIGHT)
		return dest.w - w;
	return 0;
}

void DrawArtStr(const char* text, const SDL_Rect &rect, int flags, bool drawTextCursor)
{
	unsigned size = (flags & UIS_SIZE) >> 0;
	unsigned color = (flags & UIS_COLOR) >> 7;

	Art* fontArt = &ArtFonts[size][color];

	const int x = rect.x + AlignXOffset(flags, rect, GetArtStrWidth(text, size));
	const int y = rect.y + ((flags & UIS_VCENTER) ? (rect.h - fontArt->frame_height) / 2 : 0);

	int sx = x, sy = y;
	for ( ; *text != '\0'; text++) {
		if (*text == '\n') {
			sx = x;
			sy += fontArt->frame_height;
			continue;
		}
		BYTE w = FontTables[size][*(BYTE*)text];
		DrawArt(sx, sy, fontArt, *(BYTE*)text, w);
		sx += w;
	}
	if (drawTextCursor && GetAnimationFrame(2, 512) != 0) {
		DrawArt(sx, sy, fontArt, '|');
	}
}

DEVILUTION_END_NAMESPACE
