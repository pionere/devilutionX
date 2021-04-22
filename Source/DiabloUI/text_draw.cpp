#include "text_draw.h"

#include "DiabloUI/art_draw.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/text.h"
#include "DiabloUI/ttf_render_wrapped.h"
#include "DiabloUI/ui_item.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

extern SDL_Surface *pal_surface;

namespace {

TextAlignment XAlignmentFromFlags(int flags)
{
	if (flags & UIS_CENTER)
		return TextAlignment_CENTER;
	if (flags & UIS_RIGHT)
		return TextAlignment_END;
	return TextAlignment_BEGIN;
}

int AlignXOffset(int flags, const SDL_Rect &dest, int w)
{
	if (flags & UIS_CENTER)
		return (dest.w - w) / 2;
	if (flags & UIS_RIGHT)
		return dest.w - w;
	return 0;
}

} // namespace

void DrawTTF(const char *text, const SDL_Rect &rectIn, int flags,
    const SDL_Color &textColor, const SDL_Color &shadowColor,
    TtfSurfaceCache &renderCache)
{
	SDL_Rect rect(rectIn);
	if (font == NULL || text == NULL || *text == '\0')
		return;
	const auto xAlign = XAlignmentFromFlags(flags);
	if (renderCache.text == NULL) {
		renderCache.text = RenderUTF8_Solid_Wrapped(font, text, textColor, rect.w, xAlign);
		ScaleSurfaceToOutput(&renderCache.text);
	}
	if (renderCache.shadow == NULL) {
		renderCache.shadow = RenderUTF8_Solid_Wrapped(font, text, shadowColor, rect.w, xAlign);
		ScaleSurfaceToOutput(&renderCache.shadow);
	}
	SDL_Surface *textSurface = renderCache.text;
	SDL_Surface *shadowSurface = renderCache.shadow;
	if (textSurface == NULL)
		return;

	SDL_Rect destRect = rect;
	ScaleOutputRect(&destRect);
	destRect.x += AlignXOffset(flags, destRect, textSurface->w);
	destRect.y += (flags & UIS_VCENTER) ? (destRect.h - textSurface->h) / 2 : 0;

	SDL_Rect shadowRect = destRect;
	++shadowRect.x;
	++shadowRect.y;
	if (SDL_BlitSurface(shadowSurface, nullptr, DiabloUiSurface(), &shadowRect) < 0)
		ErrSdl();
	if (SDL_BlitSurface(textSurface, nullptr, DiabloUiSurface(), &destRect) < 0)
		ErrSdl();
}

void DrawArtStr(const char *text, const SDL_Rect &rect, int flags, bool drawTextCursor)
{
	_artFontTables size = AFT_SMALL;
	_artFontColors color = flags & UIS_GOLD ? AFC_GOLD : AFC_SILVER;

	if (flags & UIS_MED)
		size = AFT_MED;
	else if (flags & UIS_BIG)
		size = AFT_BIG;
	else if (flags & UIS_HUGE)
		size = AFT_HUGE;

	const int x = rect.x + AlignXOffset(flags, rect, GetArtStrWidth(text, size));
	const int y = rect.y + ((flags & UIS_VCENTER) ? (rect.h - ArtFonts[size][color].h()) / 2 : 0);

	int sx = x, sy = y;
	for (size_t i = 0, n = strlen(text); i < n; i++) {
		if (text[i] == '\n') {
			sx = x;
			sy += ArtFonts[size][color].h();
			continue;
		}
		BYTE w = FontTables[size][*(BYTE *)&text[i] + 2] ? FontTables[size][*(BYTE *)&text[i] + 2] : FontTables[size][0];
		DrawArt(sx, sy, &ArtFonts[size][color], *(BYTE *)&text[i], w);
		sx += w;
	}
	if (drawTextCursor && GetAnimationFrame(2, 500)) {
		DrawArt(sx, sy, &ArtFonts[size][color], '|');
	}
}

DEVILUTION_END_NAMESPACE
