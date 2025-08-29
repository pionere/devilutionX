#pragma once

#include <type_traits>

//#include <SDL.h>

//#include "../defs.h"
//#include "../gameui.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool gbWndActive;
#if !FULLSCREEN_ONLY
extern bool gbFullscreen;
#endif
extern int gbFrameRateControl;
extern unsigned gnRefreshDelay;
extern SDL_Window* ghMainWnd;
extern SDL_Renderer* renderer;
extern SDL_Texture* renderer_texture;
extern SDL_Surface* renderer_surface;

extern int screenWidth;
extern int screenHeight;

#ifdef USE_SDL1
void SetVideoMode(int width, int height, int bpp, uint32_t flags);
void SetVideoModeToPrimary(int width, int height);
SDL_Surface* OutputSurfaceToScale();
#endif

// Returns:
// SDL1: Video surface.
// SDL2, no upscale: Window surface.
// SDL2, upscale: Renderer texture surface.
SDL_Surface* GetOutputSurface();

#ifdef __cplusplus
}
#endif

// Convert from output coordinates to logical (resolution-independent) coordinates.
template <
    typename T,
    typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
void OutputToLogical(T* x, T* y)
{
#ifndef USE_SDL1
	if (!renderer)
		return;
#if !SDL_VERSION_ATLEAST(2, 0, 18)
	// TODO: dpi_scale?
	float scaleX, scaleY;
	SDL_RenderGetScale(renderer, &scaleX, &scaleY);
	*x = (T)(*x / scaleX);
	*y = (T)(*y / scaleY);

	SDL_Rect view;
	SDL_RenderGetViewport(renderer, &view);
	*x -= view.x;
	*y -= view.y;
#else
	float wx, wy;
	SDL_RenderWindowToLogical(renderer, x, y, &wx, &wy);
	*x = (T)wx;
	*y = (T)wy;
#endif
#else
	const SDL_Surface* surface = OutputSurfaceToScale();
	if (surface == NULL) return;
	*x = *x * SCREEN_WIDTH / surface->w;
	*y = *y * SCREEN_HEIGHT / surface->h;
#endif
}

template <
    typename T,
    typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
void LogicalToOutput(T* x, T* y)
{
#ifndef USE_SDL1
	if (renderer == NULL)
		return;
#if !SDL_VERSION_ATLEAST(2, 0, 18)
	SDL_Rect view;
	SDL_RenderGetViewport(renderer, &view);
	*x += view.x;
	*y += view.y;
	float scaleX, scaleY;
	SDL_RenderGetScale(renderer, &scaleX, &scaleY);
#if 0
	*x = (T)SDL_ceilf(*x * scaleX);
	*y = (T)SDL_ceilf(*y * scaleY);
#else
	*x = (T)(*x * scaleX);
	*y = (T)(*y * scaleY);
#endif
	// TODO: dpi_scale?
#else
	SDL_RenderLogicalToWindow(renderer, (float)*x, (float)*y, x, y);
#endif
#else
	const SDL_Surface* surface = OutputSurfaceToScale();
	if (surface == NULL) return;
	*x = *x * surface->w / SCREEN_WIDTH;
	*y = *y * surface->h / SCREEN_HEIGHT;
#endif
}

#define SCALE_AREA(sw, sh, dw, dh, rw, rh) \
{ \
	unsigned long srcW = sw, srcH = sh, dstW = dw, dstH = dh; \
	unsigned long mul0 = srcW * dstH; \
	unsigned long mul1 = srcH * dstW; \
	if (mul0 > mul1) { \
		rw = dstW; \
		rh = mul1 / srcW; \
	} else { \
		rw = mul0 / srcH; \
		rh = dstH; \
	} \
}

DEVILUTION_END_NAMESPACE
