#include "text_draw.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "all.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

static int AlignXOffset(int flags, const SDL_Rect& dest, int w)
{
	if (flags & UIS_CENTER)
		return (dest.w - w) / 2;
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

	color = color == AFC_SILVER ? COL_WHITE : COL_GOLD;
	switch (size) {
	case AFT_SMALL:
		w = GetSmallStringWidth(text);
		dy = 1;
		h = 11 - dy;
		//h = 11;
		pChar = PrintSmallChar;
		break;
	case AFT_MED:
	case AFT_BIG:
		w = GetBigStringWidth(text);
		dy = 5 - 2;
		h = 22 - dy;
		//h = 22;
		pChar = PrintBigChar;
		break;
	case AFT_HUGE:
		w = GetHugeStringWidth(text);
		dy = 10 - 4;
		h = 46 - dy;
		//h = 46;
		pChar = PrintHugeChar;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	int x = rect.x + AlignXOffset(flags, rect, w) + SCREEN_X;
	int y = rect.y + ((flags & UIS_VCENTER) ? (rect.h - h) / 2 : 0) + SCREEN_Y + h;

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
