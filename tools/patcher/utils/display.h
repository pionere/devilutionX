#pragma once

#include <type_traits>

//#include <SDL.h>

//#include "../defs.h"
//#include "../gameui.h"
#include "../all.h"

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
//extern int viewportHeight;

#ifdef USE_SDL1
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
	float scaleX;
	SDL_RenderGetScale(renderer, &scaleX, NULL);
	*x = (T)(*x / scaleX);
	*y = (T)(*y / scaleX);

	SDL_Rect view;
	SDL_RenderGetViewport(renderer, &view);
	*x -= view.x;
	*y -= view.y;
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
	//SDL_Rect view;
	//SDL_RenderGetViewport(renderer, &view);
	//assert(view.x == 0 && view.y == 0);
	//*x += view.x;
	//*y += view.y;

	float scaleX, scaleY;
	SDL_RenderGetScale(renderer, &scaleX, &scaleY);
	T xx = (T)(*x * scaleX);
	if ((T)(xx / scaleX) != *x) {
		xx++;
	}
	*x = xx;
	T yy = (T)(*y * scaleY);
	if ((T)(yy / scaleY) != *y) {
		yy++;
	}
	*y = yy;
	//*x = (T)(*x * scaleX);
	//*y = (T)(*y * scaleX);
#else
	const SDL_Surface* surface = OutputSurfaceToScale();
	if (surface == NULL) return;
	*x = *x * surface->w / SCREEN_WIDTH;
	*y = *y * surface->h / SCREEN_HEIGHT;
#endif
}

DEVILUTION_END_NAMESPACE
