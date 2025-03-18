// Compatibility wrappers for SDL 1 & 2.
#pragma once
#include <SDL.h>

#ifndef USE_SDL1
#define SDLC_KEYSTATE(x)         SDL_SCANCODE_##x
#define SDLC_KEYSTATE_LEFTCTRL   SDL_SCANCODE_LCTRL
#define SDLC_KEYSTATE_RIGHTCTRL  SDL_SCANCODE_RCTRL
#define SDLC_KEYSTATE_LEFTSHIFT  SDL_SCANCODE_LSHIFT
#define SDLC_KEYSTATE_RIGHTSHIFT SDL_SCANCODE_RSHIFT
#define SDLC_KEYSTATE_LALT       SDL_SCANCODE_LALT
#define SDLC_KEYSTATE_RALT       SDL_SCANCODE_RALT
#define SDLC_KEYSTATE_UP         SDL_SCANCODE_UP
#define SDLC_KEYSTATE_DOWN       SDL_SCANCODE_DOWN
#define SDLC_KEYSTATE_LEFT       SDL_SCANCODE_LEFT
#define SDLC_KEYSTATE_RIGHT      SDL_SCANCODE_RIGHT

#define vkcode                   key.keysym.sym
#else
//#define SDLC_KEYSTATE(x) SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_##x)
#define SDLC_KEYSTATE(x)         SDLK_##x
#define SDLC_KEYSTATE_LEFTCTRL   SDLK_LCTRL
#define SDLC_KEYSTATE_RIGHTCTRL  SDLK_RCTRL
#define SDLC_KEYSTATE_LEFTSHIFT  SDLK_LSHIFT
#define SDLC_KEYSTATE_RIGHTSHIFT SDLK_LSHIFT
#define SDLC_KEYSTATE_LALT       SDLK_LALT
#define SDLC_KEYSTATE_RALT       SDLK_RALT
#define SDLC_KEYSTATE_UP         SDLK_UP
#define SDLC_KEYSTATE_DOWN       SDLK_DOWN
#define SDLC_KEYSTATE_LEFT       SDLK_LEFT
#define SDLC_KEYSTATE_RIGHT      SDLK_RIGHT

#define vkcode                   key.keysym.scancode
#endif

inline const Uint8* SDLC_GetKeyState()
{
#ifdef USE_SDL1
	return SDL_GetKeyState(NULL);
#else
	return SDL_GetKeyboardState(NULL);
#endif
}

inline int SDLC_SetColorKey(SDL_Surface* surface, Uint32 key)
{
#ifdef USE_SDL1
	return SDL_SetColorKey(surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, key);
#else
	SDL_SetSurfaceRLE(surface, 1);
	return SDL_SetColorKey(surface, SDL_TRUE, key);
#endif
}

// Copies the colors into the surface's palette.
inline int SDLC_SetSurfaceColors(SDL_Surface* surface, SDL_Color* colors, int firstcolor, int ncolors)
{
#ifdef USE_SDL1
	return SDL_SetPalette(surface, SDL_LOGPAL, colors, firstcolor, ncolors) - 1;
#else
	return SDL_SetPaletteColors(surface->format->palette, colors, firstcolor, ncolors);
#endif
}

/*
 * SDL1: Copies the colors into the surface's palette.
 * SDL2: Sets the surface's palette to the input palette.
 */
inline int SDLC_SetSurfaceColors(SDL_Surface* surface, SDL_Palette* palette)
{
#ifdef USE_SDL1
	return SDLC_SetSurfaceColors(surface, palette->colors, 0, palette->ncolors);
#else
	return SDL_SetPixelFormatPalette(surface->format, palette);
#endif
}

#ifndef SDL_TICKS_AFTER
#define SDL_TICKS_AFTER(A, B, C) ((Uint32)((A) - (B)) >= (Uint32)(C))
#endif
