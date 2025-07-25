#pragma once
// Backports for older versions of SDL 2.
#ifndef USE_SDL1
#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2, 0, 4)
inline SDL_bool SDL_PointInRect(const SDL_Point* p, const SDL_Rect* r)
{
	return ((p->x >= r->x) && (p->x < (r->x + r->w))
	           && (p->y >= r->y) && (p->y < (r->y + r->h)))
	    ? SDL_TRUE
	    : SDL_FALSE;
}
#endif

#if !SDL_VERSION_ATLEAST(2, 0, 5)
inline SDL_Surface*
SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth,
    Uint32 format)
{
	int bpp;
	Uint32 rmask, gmask, bmask, amask;
	if (!SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask))
		return NULL;
	return SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
}

inline SDL_Surface*
SDL_CreateRGBSurfaceWithFormatFrom(void* pixels,
    int width, int height, int depth, int pitch,
    Uint32 format)
{
	SDL_Surface* surface;

	surface = SDL_CreateRGBSurfaceWithFormat(0, 0, 0, 0, format);
	if (surface != NULL) {
		surface->flags |= SDL_PREALLOC;
		surface->pixels = pixels;
		surface->w = width;
		surface->h = height;
		surface->pitch = pitch;
		SDL_SetClipRect(surface, NULL);
	}
	return surface;
}
#endif

#if !SDL_VERSION_ATLEAST(2, 0, 10)
typedef enum
{
	SDL_TOUCH_DEVICE_INVALID = -1,
	SDL_TOUCH_DEVICE_DIRECT,            /* touch screen with window-relative coordinates */
	SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE, /* trackpad with absolute device coordinates */
	SDL_TOUCH_DEVICE_INDIRECT_RELATIVE  /* trackpad with screen cursor-relative coordinates */
} SDL_TouchDeviceType;

inline SDL_TouchDeviceType SDL_GetTouchDeviceType(SDL_TouchID touchID)
{
	return SDL_TOUCH_DEVICE_DIRECT;
}
#endif
#endif // !USE_SDL1