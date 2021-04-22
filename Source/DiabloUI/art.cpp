#include "art.h"

#include "all.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

void LoadArt(const char *pszFile, Art *art, int frames, SDL_Color *pPalette)
{
	assert(art != NULL);
	if (art->surface != NULL)
		return;

	art->frames = frames;

	unsigned width, height, bpp;
	if (!SBmpLoadImage(pszFile, NULL, NULL, 0, &width, &height, &bpp)) {
		SDL_Log("Failed to load image meta");
		return;
	}

	Uint32 format;
	switch (bpp) {
	case 8:
		format = SDL_PIXELFORMAT_INDEX8;
		break;
	case 24:
		format = SDL_PIXELFORMAT_RGB888;
		break;
	case 32:
		format = SDL_PIXELFORMAT_RGBA8888;
		break;
	default:
		format = SDL_PIXELFORMAT_UNKNOWN;
		break;
	}
	SDL_Surface *artSurface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, width, height, bpp, format);

	if (!SBmpLoadImage(pszFile, pPalette, static_cast<BYTE *>(artSurface->pixels),
	        artSurface->pitch * artSurface->format->BytesPerPixel * height, NULL, NULL, NULL)) {
		SDL_Log("Failed to load image");
		SDL_FreeSurface(artSurface);
		//art->surface = NULL;
		return;
	}

	art->surface = artSurface;
	art->logical_width = artSurface->w;
	art->frame_height = height / frames;
	//art->palette_version = pal_surface_palette_version - 1;
	ScaleSurfaceToOutput(&art->surface);
}

void LoadMaskedArt(const char *pszFile, Art *art, int frames, int mask)
{
	LoadArt(pszFile, art, frames);
	if (art->surface != NULL)
		SDLC_SetColorKey(art->surface, mask);
}

void LoadArt(Art *art, const BYTE *artData, int w, int h, int frames)
{
	art->frames = frames;
	art->surface = SDL_CreateRGBSurfaceWithFormatFrom(
	    const_cast<BYTE *>(artData), w, h, 8, w, SDL_PIXELFORMAT_INDEX8);
	art->logical_width = w;
	art->frame_height = h / frames;
	ScaleSurfaceToOutput(&art->surface);
}

DEVILUTION_END_NAMESPACE
