#include "art_draw.h"

#include "all.h"

#include "DiabloUI/diabloui.h"
#include "utils/display.h"
#include "utils/sdl_compat.h"

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

	if (art->surface->format->BitsPerPixel == 8 && art->palette_version != back_surface_palette_version) {
		if (SDLC_SetSurfaceColors(art->surface, back_surface->format->palette) < 0)
			sdl_fatal(ERR_SDL_ART_COLOR);
		art->palette_version = back_surface_palette_version;
	}

	if (SDL_BlitSurface(art->surface, &srcRect, DiabloUiSurface(), &dstRect) < 0)
		sdl_fatal(ERR_SDL_ART_BLIT);
}

void DrawAnimatedArt(Art *art, int screenX, int screenY)
{
	DrawArt(screenX, screenY, art, GetAnimationFrame(art->frames));
}

int GetAnimationFrame(int frames, int fps)
{
	//assert(frames > 0 && fps > 0);
	return (SDL_GetTicks() / fps) % frames;
}

DEVILUTION_END_NAMESPACE
