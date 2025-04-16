#include "text_draw.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "all.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

static int AlignXOffset(int flags, int rw, int sw)
{
	if (flags & UIS_HCENTER)
		return (rw - sw) >> 1;
	if (flags & UIS_RIGHT)
		return rw - sw;
	return 0;
}

void DrawArtStr(const char* text, const SDL_Rect& rect, int flags)
{
	unsigned size = (flags & UIS_SIZE) >> 0, color;
	int sw, lh, dy, sx, sy, cx, cy;
	static int (*pChar)(int sx, int sy, BYTE text, BYTE col);

	switch (size) {
	case AFT_SMALL:
		sw = GetSmallStringWidth(text);
		dy = 1;
		lh = SMALL_FONT_HEIGHT - dy;
		//lh = SMALL_FONT_HEIGHT;
		pChar = PrintSmallChar;
		break;
	case AFT_MED:
		static_assert(MED_FONT_HEIGHT == BIG_FONT_HEIGHT, "DrawArtStr handles medium and big characters the same way.");
	case AFT_BIG:
		sw = GetBigStringWidth(text);
		dy = 5 - 2;
		lh = BIG_FONT_HEIGHT - dy;
		//lh = BIG_FONT_HEIGHT;
		pChar = PrintBigChar;
		break;
	case AFT_HUGE:
		sw = GetHugeStringWidth(text);
		dy = 10 - 4;
		lh = HUGE_FONT_HEIGHT - dy;
		//lh = HUGE_FONT_HEIGHT;
		pChar = PrintHugeChar;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	sx = rect.x + AlignXOffset(flags, rect.w, sw) + SCREEN_X;
	sy = rect.y + ((flags & UIS_VCENTER) ? ((rect.h - lh) >> 1) : 0) + SCREEN_Y + lh;

	sy += dy;
	lh += dy;
	color = (flags & UIS_COLOR) >> 7;

	cx = sx;
	cy = sy;
	for ( ; *text != '\0'; text++) {
		if (*text == '\n') {
			cx = sx;
			cy += lh;
			continue;
		}
		cx += pChar(cx, cy, (BYTE)*text, color);
	}
}

DEVILUTION_END_NAMESPACE
