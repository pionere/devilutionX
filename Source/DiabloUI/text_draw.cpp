#include "text_draw.h"

#include "all.h"

#include "DiabloUI/art_draw.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/text.h"
#include "DiabloUI/ttf_render_wrapped.h"
#include "DiabloUI/ui_item.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

static int XAlignmentFromFlags(int flags)
{
	return (flags & UIS_XALIGN) >> 4;
}

static int AlignXOffset(int flags, const SDL_Rect &dest, int w)
{
	if (flags & UIS_CENTER)
		return (dest.w - w) / 2;
	if (flags & UIS_RIGHT)
		return dest.w - w;
	return 0;
}

void DrawTTF(const char *text, const SDL_Rect &rectIn, int flags,
    const SDL_Color textColor, const SDL_Color shadowColor,
    TtfSurfaceCache &renderCache)
{
	if (font == NULL || text == NULL || *text == '\0')
		return;
	const auto xAlign = XAlignmentFromFlags(flags);
	SDL_Surface* textSurface = renderCache.text;
	if (textSurface == NULL) {
		textSurface = RenderUTF8_Solid_Wrapped(font, text, textColor, rectIn.w, xAlign);
		if (textSurface == NULL)
			return;
		ScaleSurfaceToOutput(&textSurface);
		renderCache.text = textSurface;
	}
	SDL_Surface* shadowSurface = renderCache.shadow;
	if (shadowSurface == NULL) {
		shadowSurface = RenderUTF8_Solid_Wrapped(font, text, shadowColor, rectIn.w, xAlign);
		ScaleSurfaceToOutput(&shadowSurface);
		renderCache.shadow = shadowSurface;
	}

	SDL_Rect destRect = rectIn;
	ScaleOutputRect(&destRect);
	destRect.x += AlignXOffset(flags, destRect, textSurface->w);
	destRect.y += (flags & UIS_VCENTER) ? (destRect.h - textSurface->h) / 2 : 0;

	++destRect.x;
	++destRect.y;

	SDL_Surface* uiSurface = DiabloUiSurface();
	if (SDL_BlitSurface(shadowSurface, NULL, uiSurface, &destRect) < 0)
		sdl_fatal(ERR_SDL_TDRAW_SHADOW);
	--destRect.x;
	--destRect.y;
	if (SDL_BlitSurface(textSurface, NULL, uiSurface, &destRect) < 0)
		sdl_fatal(ERR_SDL_TDRAW_TEXT);
}

void DrawArtStr(const char *text, const SDL_Rect &rect, int flags, bool drawTextCursor)
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
		BYTE w = FontTables[size][*(BYTE*)text + 2] != 0 ? FontTables[size][*(BYTE*)text + 2] : FontTables[size][0];
		DrawArt(sx, sy, fontArt, *(BYTE*)text, w);
		sx += w;
	}
	if (drawTextCursor && GetAnimationFrame(2, 500) != 0) {
		DrawArt(sx, sy, fontArt, '|');
	}
}

DEVILUTION_END_NAMESPACE
