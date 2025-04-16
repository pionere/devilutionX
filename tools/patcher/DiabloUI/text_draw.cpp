#include "text_draw.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "all.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

static int AlignXOffset(int flags, const SDL_Rect& dest, int w)
{
	if (flags & UIS_HCENTER)
		return (dest.w - w) >> 1;
	if (flags & UIS_RIGHT)
		return dest.w - w;
	return 0;
}

void DrawArtStr(const char* text, const SDL_Rect& rect, int flags)
{
	unsigned size = (flags & UIS_SIZE) >> 0;
	unsigned color = (flags & UIS_COLOR) >> 7;
	int w, h, dy;
	static int (*pChar)(int sx, int sy, BYTE text, BYTE col);

	switch (size) {
	case AFT_SMALL:
		w = GetSmallStringWidth(text);
		dy = 1;
		h = SMALL_FONT_HEIGHT - dy;
		//h = SMALL_FONT_HEIGHT;
		pChar = PrintSmallChar;
		break;
	case AFT_MED:
		static_assert(MED_FONT_HEIGHT == BIG_FONT_HEIGHT, "DrawArtStr handles medium and big characters the same way.");
	case AFT_BIG:
		w = GetBigStringWidth(text);
		dy = 5 - 2;
		h = BIG_FONT_HEIGHT - dy;
		//h = BIG_FONT_HEIGHT;
		pChar = PrintBigChar;
		break;
	case AFT_HUGE:
		w = GetHugeStringWidth(text);
		dy = 10 - 4;
		h = HUGE_FONT_HEIGHT - dy;
		//h = HUGE_FONT_HEIGHT;
		pChar = PrintHugeChar;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	int x = rect.x + AlignXOffset(flags, rect, w) + SCREEN_X;
	int y = rect.y + ((flags & UIS_VCENTER) ? ((rect.h - h) >> 1) : 0) + SCREEN_Y + h;

	y += dy;
	h += dy;

	int sx = x, sy = y;
	for ( ; *text != '\0'; text++) {
		if (*text == '\n') {
			sx = x;
			sy += h;
			continue;
		}
		sx += pChar(sx, sy, (BYTE)*text, color);
	}
}

DEVILUTION_END_NAMESPACE
