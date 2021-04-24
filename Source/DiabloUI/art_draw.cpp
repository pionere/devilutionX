#include "DiabloUI/art_draw.h"

#include "DiabloUI/diabloui.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

void DrawArt(int screenX, int screenY, Art *art, int nFrame, int srcW, int srcH)
{
	if (screenY >= SCREEN_HEIGHT || screenX >= SCREEN_WIDTH || art->surface == NULL)
		return;

	SDL_Rect srcRect;
	srcRect.x = 0;
	srcRect.y = nFrame * art->h();
	srcRect.w = art->w();
	srcRect.h = art->h();

	ScaleOutputRect(&srcRect);

	if (srcW != 0 && srcW < srcRect.w)
		srcRect.w = srcW;
	if (srcH != 0 && srcH < srcRect.h)
		srcRect.h = srcH;
	SDL_Rect dstRect = { screenX, screenY, srcRect.w, srcRect.h };
	ScaleOutputRect(&dstRect);

	if (art->surface->format->BitsPerPixel == 8 && art->palette_version != pal_surface_palette_version) {
		if (SDLC_SetSurfaceColors(art->surface, pal_surface->format->palette) <= -1)
			ErrSdl();
		art->palette_version = pal_surface_palette_version;
	}

	if (SDL_BlitSurface(art->surface, &srcRect, DiabloUiSurface(), &dstRect) < 0)
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
