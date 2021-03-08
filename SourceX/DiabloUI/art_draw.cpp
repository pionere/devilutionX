#include "DiabloUI/art_draw.h"
#include "display.h"

DEVILUTION_BEGIN_NAMESPACE

void DrawArt(int screenX, int screenY, Art *art, int nFrame, int srcW, int srcH)
{
	if (screenY >= SCREEN_HEIGHT || screenX >= SCREEN_WIDTH || art->surface == NULL)
		return;

	SDL_Rect src_rect;
	src_rect.x = 0;
	src_rect.y = nFrame * art->h();
	src_rect.w = art->w();
	src_rect.h = art->h();

	ScaleOutputRect(&src_rect);

	if (srcW && srcW < src_rect.w)
		src_rect.w = srcW;
	if (srcH && srcH < src_rect.h)
		src_rect.h = srcH;
	SDL_Rect dst_rect = { screenX, screenY, src_rect.w, src_rect.h };
	ScaleOutputRect(&dst_rect);

	if (art->surface->format->BitsPerPixel == 8 && art->palette_version != pal_surface_palette_version) {
		if (SDLC_SetSurfaceColors(art->surface, pal_surface->format->palette) <= -1)
			ErrSdl();
		art->palette_version = pal_surface_palette_version;
	}

	if (SDL_BlitSurface(art->surface, &src_rect, GetOutputSurface(), &dst_rect) < 0)
		ErrSdl();
}

void DrawAnimatedArt(Art *art, int screenX, int screenY)
{
	DrawArt(screenX, screenY, art, GetAnimationFrame(art->frames));
}

int GetAnimationFrame(int frames, int fps)
{
	int frame = (SDL_GetTicks() / fps) % frames;

	return frame > frames ? 0 : frame;
}

DEVILUTION_END_NAMESPACE
