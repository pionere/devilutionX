/**
 * @file dx.cpp
 *
 * Implementation of functions setting up the graphics pipeline.
 */
#include "all.h"
#include "utils/display.h"
#include <SDL.h>

#ifdef __3DS__
#include <3ds.h>
#endif

DEVILUTION_BEGIN_NAMESPACE

unsigned _guLockCount;
/** Back buffer */
BYTE *gpBuffer;
/** Upper bound of back buffer. */
BYTE *gpBufStart;
/** Lower bound of back buffer. */
BYTE *gpBufEnd;

#ifdef _DEBUG
int locktbl[256];
#endif
static CCritSect sgMemCrit;

static void dx_create_back_buffer()
{
	pal_surface = SDL_CreateRGBSurfaceWithFormat(0, BUFFER_WIDTH, BUFFER_HEIGHT, 8, SDL_PIXELFORMAT_INDEX8);
	if (pal_surface == NULL) {
		ErrSdl();
	}

	gpBuffer = (BYTE *)pal_surface->pixels;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * SCREEN_Y];
	gpBufEnd = (BYTE *)(BUFFER_WIDTH * (SCREEN_HEIGHT + SCREEN_Y));

#ifndef USE_SDL1
	// In SDL2, `pal_surface` points to the global `palette`.
	if (SDL_SetSurfacePalette(pal_surface, palette) < 0)
		ErrSdl();
#else
	// In SDL1, `pal_surface` owns its palette and we must update it every
	// time the global `palette` is changed. No need to do anything here as
	// the global `palette` doesn't have any colors set yet.
#endif

	pal_surface_palette_version = 1;
}

static void dx_create_primary_surface()
{
#ifndef USE_SDL1
	if (renderer != NULL) {
		int width, height;
		SDL_RenderGetLogicalSize(renderer, &width, &height);
		Uint32 format;
		if (SDL_QueryTexture(texture, &format, NULL, NULL, NULL) < 0)
			ErrSdl();
		renderer_texture_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, SDL_BITSPERPIXEL(format), format);
	}
#endif
	if (GetOutputSurface() == NULL) {
		ErrSdl();
	}
}

void dx_init()
{
#ifndef USE_SDL1
	SDL_RaiseWindow(ghMainWnd);
	SDL_ShowWindow(ghMainWnd);
#endif

	dx_create_primary_surface();
	palette_init();
	dx_create_back_buffer();
}
static void lock_buf_priv()
{
	sgMemCrit.Enter();
	if (_guLockCount != 0) {
		_guLockCount++;
		return;
	}

	gpBuffer = (BYTE *)pal_surface->pixels;
	gpBufEnd += (uintptr_t)gpBuffer; // (BYTE *)pal_surface->pixels;
	// gpBufEnd = gpBuffer + pal_surface->pitch * pal_surface->h;
	_guLockCount++;
}

void lock_buf(BYTE idx)
{
#ifdef _DEBUG
	++locktbl[idx];
#endif
	lock_buf_priv();
}

static void unlock_buf_priv()
{
	if (_guLockCount == 0)
		app_fatal("draw main unlock error");
	if (gpBuffer == NULL)
		app_fatal("draw consistency error");

	_guLockCount--;
	if (_guLockCount == 0) {
		gpBufEnd -= (uintptr_t)gpBuffer;
	}
	sgMemCrit.Leave();
}

void unlock_buf(BYTE idx)
{
#ifdef _DEBUG
	if (locktbl[idx] == 0)
		app_fatal("Draw lock underflow: 0x%x", idx);
	--locktbl[idx];
#endif
	unlock_buf_priv();
}

void dx_cleanup()
{
#ifndef USE_SDL1
	if (ghMainWnd != NULL)
		SDL_HideWindow(ghMainWnd);
#endif
	sgMemCrit.Enter();
	_guLockCount = 0;
	gpBuffer = NULL;
	sgMemCrit.Leave();

	if (pal_surface == NULL)
		return;
	SDL_FreeSurface(pal_surface);
	pal_surface = NULL;
	SDL_FreePalette(palette);
	SDL_FreeSurface(renderer_texture_surface);
#ifndef USE_SDL1
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
#endif
	SDL_DestroyWindow(ghMainWnd);
}

void dx_reinit()
{
#ifdef USE_SDL1
	ghMainWnd = SDL_SetVideoMode(0, 0, 0, ghMainWnd->flags ^ SDL_FULLSCREEN);
	if (ghMainWnd == NULL) {
		ErrSdl();
	}
#else
	Uint32 flags = 0;
	if (!gbFullscreen) {
		flags = renderer != NULL ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
	}
	if (SDL_SetWindowFullscreen(ghMainWnd, flags) != 0) {
		ErrSdl();
	}
#endif
	gbFullscreen = !gbFullscreen;
	gbRedrawFlags = REDRAW_ALL;
}

void InitPalette()
{
	palette = SDL_AllocPalette(256);
	if (palette == NULL) {
		ErrSdl();
	}
}

void BltFast(const SDL_Rect *src_rect, SDL_Rect *dst_rect)
{
	Blit(pal_surface, src_rect, dst_rect);
}

void Blit(SDL_Surface *src, const SDL_Rect *src_rect, SDL_Rect *dst_rect)
{
	SDL_Surface *dst = GetOutputSurface();
#ifndef USE_SDL1
	if (SDL_BlitSurface(src, src_rect, dst, dst_rect) < 0)
		ErrSdl();
	return;
#else
	if (!OutputRequiresScaling()) {
		if (SDL_BlitSurface(src, src_rect, dst, dst_rect) < 0)
			ErrSdl();
		return;
	}

	SDL_Rect scaled_dst_rect;
	if (dst_rect != NULL) {
		scaled_dst_rect = *dst_rect;
		ScaleOutputRect(&scaled_dst_rect);
		dst_rect = &scaled_dst_rect;
	}

	// Same pixel format: We can call BlitScaled directly.
	if (SDLBackport_PixelFormatFormatEq(src->format, dst->format)) {
		if (SDL_BlitScaled(src, src_rect, dst, dst_rect) < 0)
			ErrSdl();
		return;
	}

	// If the surface has a color key, we must stretch first and can then call BlitSurface.
	if (SDL_HasColorKey(src)) {
		SDL_Surface *stretched = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_rect->w, dst_rect->h, src->format->BitsPerPixel,
		    src->format->Rmask, src->format->Gmask, src->format->BitsPerPixel, src->format->Amask);
		SDL_SetColorKey(stretched, SDL_SRCCOLORKEY, src->format->colorkey);
		if (src->format->palette != NULL)
			SDL_SetPalette(stretched, SDL_LOGPAL, src->format->palette->colors, 0, src->format->palette->ncolors);
		SDL_Rect stretched_rect = { 0, 0, dst_rect->w, dst_rect->h };
		if (SDL_SoftStretch(src, src_rect, stretched, &stretched_rect) < 0
		    || SDL_BlitSurface(stretched, &stretched_rect, dst, dst_rect) < 0) {
			SDL_FreeSurface(stretched);
			ErrSdl();
		}
		SDL_FreeSurface(stretched);
		return;
	}

	// A surface with a non-output pixel format but without a color key needs scaling.
	// We can convert the format and then call BlitScaled.
	SDL_Surface *converted = SDL_ConvertSurface(src, dst->format, 0);
	if (SDL_BlitScaled(converted, src_rect, dst, dst_rect) < 0) {
		SDL_FreeSurface(converted);
		ErrSdl();
	}
	SDL_FreeSurface(converted);
#endif
}

/**
 * @brief Limit FPS to avoid high CPU load, use when v-sync isn't available
 */
void LimitFrameRate()
{
	if (!gbFPSLimit)
		return;
	static uint32_t frameDeadline;
	uint32_t tc = SDL_GetTicks() * 1000;
	uint32_t v = 0;
	if (frameDeadline > tc) {
		v = tc % gnRefreshDelay;
		SDL_Delay(v / 1000 + 1); // ceil
	}
	frameDeadline = tc + v + gnRefreshDelay;
}

void RenderPresent()
{
	SDL_Surface *surface = GetOutputSurface();

	if (!gbActive) {
		LimitFrameRate();
		return;
	}

#ifndef USE_SDL1
	if (renderer != NULL) {
		if (SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch) <= -1) { //pitch is 2560
			ErrSdl();
		}

		// Clear buffer to avoid artifacts in case the window was resized
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) <= -1) { // TODO only do this if window was resized
			ErrSdl();
		}

		if (SDL_RenderClear(renderer) <= -1) {
			ErrSdl();
		}
		if (SDL_RenderCopy(renderer, texture, NULL, NULL) <= -1) {
			ErrSdl();
		}
		SDL_RenderPresent(renderer);

		if (!gbVsyncEnabled) {
			LimitFrameRate();
		}
	} else {
		if (SDL_UpdateWindowSurface(ghMainWnd) <= -1) {
			ErrSdl();
		}
		LimitFrameRate();
	}
#else
	if (SDL_Flip(surface) <= -1) {
		ErrSdl();
	}
	LimitFrameRate();
#endif
}

void PaletteGetEntries(unsigned dwNumEntries, SDL_Color *lpEntries)
{
	for (unsigned i = 0; i < dwNumEntries; i++) {
		lpEntries[i] = system_palette[i];
	}
}
DEVILUTION_END_NAMESPACE
