#pragma once

#ifdef USE_SDL1
#include <SDL.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <cmath>
#include <cstddef>

// SDL1.dll was compiled assuming an strdup function available.
// Some compilers might have removed it in the meantime -> provide SDL_strdup for backwards compatibility
#if defined(_WIN32) && (!defined(_WIN32_WINNT) || _WIN32_WINNT <= 0x0500)
#ifndef SDL_strdup
#define SDL_strdup _strdup
#endif
#endif

#define WINDOW_ICON_NAME 0

//== Utility

#define SDL_zero(x)                  SDL_memset(&(x), 0, sizeof((x)))
#define SDL_InvalidParamError(param) SDL_SetError("Parameter '%s' is invalid", (param))
#define SDL_floor                    floor

//== Events handling

#define SDL_threadID Uint32
#ifndef SDLCALL
#define SDLCALL
#endif
typedef int(SDLCALL* SDL_ThreadFunction)(void* data);

#define SDL_TICKS_PASSED(A, B)  ((Sint32)((B) - (A)) <= 0)

#define SDL_Keysym  SDL_keysym
#define SDL_Keycode SDLKey
#define SDL_Keymod  SDLMod

#define SDLK_PRINTSCREEN  SDLK_PRINT
#define SDLK_SCROLLLOCK   SDLK_SCROLLOCK
#define SDLK_NUMLOCKCLEAR SDLK_NUMLOCK
#define SDLK_KP_1         SDLK_KP1
#define SDLK_KP_2         SDLK_KP2
#define SDLK_KP_3         SDLK_KP3
#define SDLK_KP_4         SDLK_KP4
#define SDLK_KP_5         SDLK_KP5
#define SDLK_KP_6         SDLK_KP6
#define SDLK_KP_7         SDLK_KP7
#define SDLK_KP_8         SDLK_KP8
#define SDLK_KP_9         SDLK_KP9
#define SDLK_KP_0         SDLK_KP0
#define SDLK_KP_COMMA     SDLK_COMMA
#define SDLK_LGUI         SDLK_LSUPER
#define SDLK_RGUI         SDLK_RSUPER

// Haptic events are not supported in SDL1.
#define SDL_INIT_HAPTIC 0

// For now we only process ASCII input when using SDL1.
#define SDL_TEXTINPUTEVENT_TEXT_SIZE 2

#define SDL_JoystickID           Sint32
#define SDL_JoystickNameForIndex SDL_JoystickName

typedef enum SDL_LogPriority {
	SDL_LOG_PRIORITY_VERBOSE = 1,
	SDL_LOG_PRIORITY_DEBUG,
	SDL_LOG_PRIORITY_INFO,
	SDL_LOG_PRIORITY_WARN,
	SDL_LOG_PRIORITY_ERROR,
	SDL_LOG_PRIORITY_CRITICAL,
	SDL_NUM_LOG_PRIORITIES
} SDL_LogPriority;

typedef enum SDL_LogCategory {
	SDL_LOG_CATEGORY_APPLICATION,
	SDL_LOG_CATEGORY_ERROR,
	SDL_LOG_CATEGORY_ASSERT,
	SDL_LOG_CATEGORY_SYSTEM,
	SDL_LOG_CATEGORY_AUDIO,
	SDL_LOG_CATEGORY_VIDEO,
	SDL_LOG_CATEGORY_RENDER,
	SDL_LOG_CATEGORY_INPUT,
	SDL_LOG_CATEGORY_TEST,
} SDL_LogCategory;

void SDL_Log(const char* fmt, ...);
//{
//	va_list ap;
//	va_start(ap, fmt);
//	vprintf(fmt, ap);
//	va_end(ap);
//	puts("");
//}
//void SDL_Log(const char* fmt, ...); DVL_PRINTF_ATTRIBUTE(1, 2);
void SDL_LogVerbose(SDL_LogCategory category, const char* fmt, ...);                                   // DVL_PRINTF_ATTRIBUTE(2, 3);
void SDL_LogDebug(SDL_LogCategory category, const char* fmt, ...);                                     // DVL_PRINTF_ATTRIBUTE(2, 3);
void SDL_LogInfo(SDL_LogCategory category, const char* fmt, ...);                                      // DVL_PRINTF_ATTRIBUTE(2, 3);
void SDL_LogWarn(SDL_LogCategory category, const char* fmt, ...);                                      // DVL_PRINTF_ATTRIBUTE(2, 3);
void SDL_LogError(SDL_LogCategory category, const char* fmt, ...);                                     // DVL_PRINTF_ATTRIBUTE(2, 3);
void SDL_LogCritical(SDL_LogCategory category, const char* fmt, ...);                                  // DVL_PRINTF_ATTRIBUTE(2, 3);
void SDL_LogMessageV(SDL_LogCategory category, SDL_LogPriority priority, const char* fmt, va_list ap); // DVL_PRINTF_ATTRIBUTE(3, 0);

void SDL_LogSetAllPriority(SDL_LogPriority priority);
void SDL_LogSetPriority(int category, SDL_LogPriority priority);
SDL_LogPriority SDL_LogGetPriority(SDL_LogCategory category);

inline void SDL_StartTextInput()
{
	SDL_EnableUNICODE(1);
}

inline void SDL_StopTextInput()
{
	SDL_EnableUNICODE(0);
}

inline void SDL_SetTextInputRect(const SDL_Rect* r)
{
}

//== Graphics helpers

typedef struct SDL_Point {
	int x;
	int y;
} SDL_Point;

inline SDL_bool SDL_PointInRect(const SDL_Point* p, const SDL_Rect* r)
{
	return ((p->x >= r->x) && (p->x < (r->x + r->w)) && (p->y >= r->y) && (p->y < (r->y + r->h))) ? SDL_TRUE : SDL_FALSE;
}

//= Messagebox (simply logged to stderr for now)

typedef enum {
	SDL_MESSAGEBOX_ERROR = 0x00000010,      /**< error dialog */
	SDL_MESSAGEBOX_WARNING = 0x00000020,    /**< warning dialog */
	SDL_MESSAGEBOX_INFORMATION = 0x00000040 /**< informational dialog */
} SDL_MessageBoxFlags;

#ifdef __3DS__
/** Defined in Source/platform/ctr/messagebox.cpp */
int SDL_ShowSimpleMessageBox(Uint32 flags,
    const char* title,
    const char* message,
    SDL_Surface* window);
#else
inline int SDL_ShowSimpleMessageBox(Uint32 flags,
    const char* title,
    const char* message,
    SDL_Surface* window)
{
	SDL_Log("MSGBOX: %s\n%s", title, message);
	return 0;
}
#endif

//= Window handling

#define SDL_Window SDL_Surface

inline void SDL_GetWindowPosition(SDL_Window* window, int* x, int* y)
{
	*x = window->clip_rect.x;
	*y = window->clip_rect.x;
	SDL_Log("SDL_GetWindowPosition %d %d", *x, *y);
}

inline void SDL_GetWindowSize(SDL_Window* window, int* w, int* h)
{
	*w = window->clip_rect.w;
	*h = window->clip_rect.h;
	SDL_Log("SDL_GetWindowSize %d %d", *w, *h);
}

inline void SDL_DestroyWindow(SDL_Window* window)
{
}

inline void
SDL_WarpMouseInWindow(SDL_Window* window, int x, int y)
{
	SDL_WarpMouse(x, y);
}

//= Renderer stubs

#define SDL_Renderer void

//= Texture stubs

#define SDL_Texture void

//= Palette handling

inline SDL_Palette*
SDL_AllocPalette(int ncolors)
{
	SDL_Palette* palette;

	/* Input validation */
	if (ncolors < 1) {
		SDL_InvalidParamError("ncolors");
		return NULL;
	}

	palette = (SDL_Palette*)SDL_malloc(sizeof(*palette));
	if (!palette) {
		SDL_OutOfMemory();
		return NULL;
	}
	palette->colors = (SDL_Color*)SDL_malloc(ncolors * sizeof(*palette->colors));
	if (!palette->colors) {
		SDL_free(palette);
		return NULL;
	}
	palette->ncolors = ncolors;
	SDL_memset(palette->colors, 0xFF, ncolors * sizeof(*palette->colors));
	return palette;
}

inline void
SDL_FreePalette(SDL_Palette* palette)
{
	if (!palette) {
		SDL_InvalidParamError("palette");
		return;
	}
	SDL_free(palette->colors);
	SDL_free(palette);
}

inline bool SDL_HasColorKey(SDL_Surface* surface)
{
	return (surface->flags & SDL_SRCCOLORKEY) != 0;
}

//= Pixel formats

#define SDL_PIXELFORMAT_INDEX8   1
#define SDL_PIXELFORMAT_RGB888   2
#define SDL_PIXELFORMAT_RGBA8888 3
#define SDL_BITSPERPIXEL(X)      (((X) >> 8) & 0xFF)

inline void SDLBackport_PixelformatToMask(int pixelformat, Uint32* rmask,
    Uint32* gmask,
    Uint32* bmask,
    Uint32* amask)
{
	if (pixelformat == SDL_PIXELFORMAT_RGBA8888) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		*rmask = 0xff000000;
		*gmask = 0x00ff0000;
		*bmask = 0x0000ff00;
		*amask = 0x000000ff;
#else
		*rmask = 0x000000ff;
		*gmask = 0x0000ff00;
		*bmask = 0x00ff0000;
		*amask = 0xff000000;
#endif
	} else if (pixelformat == SDL_PIXELFORMAT_RGB888) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		*rmask = 0xff000000;
		*gmask = 0x00ff0000;
		*bmask = 0x0000ff00;
#else
		*rmask = 0x000000ff;
		*gmask = 0x0000ff00;
		*bmask = 0x00ff0000;
#endif
		*amask = 0;
	} else {
		*rmask = *gmask = *bmask = *amask = 0;
	}
}

/**
 * A limited implementation of `a.format` == `b.format` from SDL2.
 */
inline bool SDLBackport_PixelFormatFormatEq(const SDL_PixelFormat* a, const SDL_PixelFormat* b)
{
	return a->BitsPerPixel == b->BitsPerPixel && (a->palette != NULL) == (b->palette != NULL)
	    && a->Rmask == b->Rmask && a->Gmask == b->Gmask && a->Bmask == b->Bmask;
}

/**
 * Similar to `SDL_ISPIXELFORMAT_INDEXED` from SDL2.
 */
inline bool SDLBackport_IsPixelFormatIndexed(const SDL_PixelFormat* pf)
{
	return pf->BitsPerPixel == 8 && pf->palette != NULL;
}

//= Surface creation

inline SDL_Surface*
SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth,
    Uint32 format)
{
	Uint32 rmask, gmask, bmask, amask;
	depth = SDL_BITSPERPIXEL(format);
	SDLBackport_PixelformatToMask(format, &rmask, &gmask, &bmask, &amask);
	return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, depth, rmask, gmask, bmask, amask);
}

inline SDL_Surface*
SDL_CreateRGBSurfaceWithFormatFrom(void* pixels,
    int width, int height, int depth, int pitch,
    Uint32 format)
{
	Uint32 rmask, gmask, bmask, amask;
	depth = SDL_BITSPERPIXEL(format);
	SDLBackport_PixelformatToMask(format, &rmask, &gmask, &bmask, &amask);
	return SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, rmask, gmask, bmask, amask);
}

//= BlitScaled backport from SDL 2.0.9.

// NOTE: Not thread-safe
int SDL_SoftStretch(SDL_Surface* src, const SDL_Rect* srcrect,
    SDL_Surface* dst, const SDL_Rect* dstrect);

// NOTE: The second argument is const in SDL2 but not here.
int SDL_BlitScaled(SDL_Surface* src, SDL_Rect* srcrect,
    SDL_Surface* dst, SDL_Rect* dstrect);

//== Filesystem

char* SDL_GetBasePath();
char* SDL_GetPrefPath(const char* org, const char* app);

//== Audio

// Audio flags are not supported in SDL1.
#define SDL_AUDIO_ALLOW_SAMPLES_CHANGE 0

#endif // USE_SDL1
