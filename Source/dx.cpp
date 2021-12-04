/**
 * @file dx.cpp
 *
 * Implementation of functions setting up the graphics pipeline.
 */
#include "all.h"
#include "utils/display.h"
#include <SDL.h>
#include <vector>

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
	back_surface = SDL_CreateRGBSurfaceWithFormat(0, BUFFER_WIDTH, BUFFER_HEIGHT, 8, SDL_PIXELFORMAT_INDEX8);
	if (back_surface == NULL) {
		sdl_fatal(ERR_SDL_BACK_PALETTE_CREATE);
	}

	gpBuffer = (BYTE *)back_surface->pixels;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * SCREEN_Y];
	//gpBufEnd = (BYTE *)(BUFFER_WIDTH * (SCREEN_Y + SCREEN_HEIGHT));
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + SCREEN_HEIGHT)];

#ifndef USE_SDL1
	// In SDL2, `back_surface` points to the global `back_palette`.
	back_palette = SDL_AllocPalette(256);
	if (back_palette == NULL)
		sdl_fatal(ERR_SDL_BACK_PALETTE_ALLOC);
	if (SDL_SetSurfacePalette(back_surface, back_palette) < 0)
		sdl_fatal(ERR_SDL_BACK_PALETTE_SET);
#else
	// In SDL1, `back_surface` owns its palette and we must update it every
	// time the global `back_palette` is changed. No need to do anything here as
	// the global `back_palette` doesn't have any colors set yet.
#endif

	back_surface_palette_version = 1;
}

static void dx_create_primary_surface()
{
#ifndef USE_SDL1
	if (renderer != NULL) {
		int width, height;
		SDL_RenderGetLogicalSize(renderer, &width, &height);
		Uint32 format;
		if (SDL_QueryTexture(renderer_texture, &format, NULL, NULL, NULL) < 0)
			sdl_fatal(ERR_SDL_TEXTURE_CREATE);
		renderer_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, SDL_BITSPERPIXEL(format), format);
	}
#endif
	if (GetOutputSurface() == NULL) {
		sdl_fatal(ERR_SDL_SURFACE_CHECK);
	}
}

void dx_init()
{
#ifndef USE_SDL1
	SDL_RaiseWindow(ghMainWnd);
	SDL_ShowWindow(ghMainWnd);
#endif

	dx_create_primary_surface();
	dx_create_back_buffer();
	palette_init();
}
static void lock_buf_priv()
{
	sgMemCrit.Enter();
	if (_guLockCount != 0) {
		_guLockCount++;
		return;
	}
	assert(gpBuffer == back_surface->pixels);
	//gpBuffer = (BYTE *)back_surface->pixels;
	//gpBufEnd += (uintptr_t)gpBuffer;
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
#ifdef _DEBUG
	if (_guLockCount == 0)
		app_fatal("draw main unlock error");
	if (gpBuffer == NULL)
		app_fatal("draw consistency error");
#endif
	_guLockCount--;
	//if (_guLockCount == 0) {
	//	gpBufEnd -= (uintptr_t)gpBuffer;
	//}
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
#if HAS_GAMECTRL == 1
	std::vector<GameController> & ctrls = GameController::All();
	while (!ctrls.empty()) {
		GameController::Remove(ctrls.front().instance_id_);
	}
#endif
#ifndef USE_SDL1
	if (ghMainWnd != NULL)
		SDL_HideWindow(ghMainWnd);
#endif
	sgMemCrit.Enter();
	_guLockCount = 0;
	gpBuffer = NULL;
	sgMemCrit.Leave();

	if (back_surface == NULL)
		return;
	SDL_FreeSurface(back_surface);
	back_surface = NULL;
#ifndef USE_SDL1
	SDL_FreePalette(back_palette);
	if (renderer != NULL) {
		SDL_FreeSurface(renderer_surface);
		renderer_surface = NULL;
		SDL_DestroyRenderer(renderer); // destroys renderer_texture as well
		renderer = NULL;
		renderer_texture = NULL;
	}
#endif
	SDL_DestroyWindow(ghMainWnd);
}

void ToggleFullscreen()
{
#ifdef USE_SDL1
	ghMainWnd = SDL_SetVideoMode(0, 0, 0, ghMainWnd->flags ^ SDL_FULLSCREEN);
	if (ghMainWnd == NULL) {
		sdl_fatal(ERR_SDL_FULLSCREEN_SDL1);
	}
#else
	Uint32 flags = 0;
	if (!gbFullscreen) {
		flags = renderer != NULL ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
	}
	if (SDL_SetWindowFullscreen(ghMainWnd, flags) < 0) {
		sdl_fatal(ERR_SDL_FULLSCREEN_SDL2);
	}
#endif
	gbFullscreen = !gbFullscreen;
	gbRedrawFlags = REDRAW_ALL;
}

/**
 * @brief Render the whole screen black
 */
void ClearScreenBuffer()
{
	lock_buf(3);

	assert(back_surface != NULL);

	SDL_FillRect(back_surface, NULL, 0x000000);

	unlock_buf(3);
}

void RedBack()
{
	assert(gpBuffer != NULL);

	int w, h;
	BYTE *dst, *tbl;
	bool inHell = currLvl._dType == DTYPE_HELL;

	dst = &gpBuffer[SCREENXY(0, 0)];
	tbl = ColorTrns[COLOR_TRN_CORAL];
	for (h = VIEWPORT_HEIGHT; h > 0; h--, dst += BUFFER_WIDTH - SCREEN_WIDTH) {
		for (w = SCREEN_WIDTH; w > 0; w--) {
			if (!inHell || *dst >= 32)
				*dst = tbl[*dst];
			dst++;
		}
	}
}

/**
 * Draws a half-transparent rectangle by blacking out odd pixels on odd lines,
 * even pixels on even lines.
 * @brief Render a transparent black rectangle
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 */
void trans_rect(int sx, int sy, int width, int height)
{
	int row, col;
	BYTE *pix = &gpBuffer[sx + BUFFER_WIDTH * sy];
	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			if (((row ^ col) & 1) == 0)
				*pix = 0;
			pix++;
		}
		pix += BUFFER_WIDTH - width;
	}
}

void BltFast(const SDL_Rect *src_rect, SDL_Rect *dst_rect)
{
	Blit(back_surface, src_rect, dst_rect);
}

void Blit(SDL_Surface *src, const SDL_Rect *src_rect, SDL_Rect *dst_rect)
{
	SDL_Surface *dst = GetOutputSurface();
#ifndef USE_SDL1
	if (SDL_BlitSurface(src, src_rect, dst, dst_rect) < 0)
		sdl_fatal(ERR_SDL_DX_BLIT_SDL2);
#else
	if (!OutputRequiresScaling()) {
		if (SDL_BlitSurface(src, const_cast<SDL_Rect*>(src_rect), dst, dst_rect) < 0)
			sdl_fatal(ERR_SDL_DX_BLIT_SDL1);
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
		if (SDL_BlitScaled(src, const_cast<SDL_Rect*>(src_rect), dst, dst_rect) < 0)
			sdl_fatal(ERR_SDL_DX_BLIT_SCALE);
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
			sdl_fatal(ERR_SDL_DX_BLIT_STRETCH);
		}
		SDL_FreeSurface(stretched);
		return;
	}

	// A surface with a non-output pixel format but without a color key needs scaling.
	// We can convert the format and then call BlitScaled.
	SDL_Surface *converted = SDL_ConvertSurface(src, dst->format, 0);
	if (SDL_BlitScaled(converted, const_cast<SDL_Rect*>(src_rect), dst, dst_rect) < 0) {
		SDL_FreeSurface(converted);
		sdl_fatal(ERR_SDL_DX_BLIT_CONVERTED);
	}
	SDL_FreeSurface(converted);
#endif
}

/**
 * @brief Limit FPS to avoid high CPU load, use when v-sync isn't available
 */
static void LimitFrameRate()
{
	static Uint32 frameDeadline;
	Uint32 v = 0, tc = SDL_GetTicks() * 1000;

	if (frameDeadline > tc) {
		v = tc % gnRefreshDelay;
		SDL_Delay(v / 1000 + 1); // ceil
	}
	frameDeadline = tc + v + gnRefreshDelay;
}

void RenderPresent()
{
	SDL_Surface *surface = GetOutputSurface();

	if (gbWndActive) {
#ifndef USE_SDL1
		if (renderer != NULL) {
			if (SDL_UpdateTexture(renderer_texture, NULL, surface->pixels, surface->pitch) < 0) { //pitch is 2560
				sdl_fatal(ERR_SDL_DX_UPDATE_TEXTURE);
			}

			// Clear buffer to avoid artifacts in case the window was resized
			if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0) { // TODO only do this if window was resized
				sdl_fatal(ERR_SDL_DX_DRAW_COLOR);
			}

			if (SDL_RenderClear(renderer) < 0) {
				sdl_fatal(ERR_SDL_DX_RENDER_CLEAR);
			}
			if (SDL_RenderCopy(renderer, renderer_texture, NULL, NULL) < 0) {
				sdl_fatal(ERR_SDL_DX_RENDER_COPY);
			}
			SDL_RenderPresent(renderer);

			if (gbVsyncEnabled)
				return;
		} else {
			if (SDL_UpdateWindowSurface(ghMainWnd) < 0) {
				sdl_fatal(ERR_SDL_DX_RENDER_SURFACE);
			}
		}
#else
		if (SDL_Flip(surface) < 0) {
			sdl_fatal(ERR_SDL_DX_FLIP);
		}
#endif
	}
	if (gbFPSLimit)
		LimitFrameRate();
}

DEVILUTION_END_NAMESPACE
