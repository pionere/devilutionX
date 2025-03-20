/**
 * @file dx.cpp
 *
 * Implementation of functions setting up the graphics pipeline.
 *
 * game_logic -- DrawView --> back_surface (INDEX8, SCREEN + BORDER)
 *
 * SDL2 - upscale:
 *   back_surface     -- Blit --> renderer_surface ( ?, SCREEN)
 *
 *   renderer_surface -- RenderPresent/SDL_UpdateTexture --> renderer_texture ( ?, SCREEN)
 *
 *   renderer_texture -- RenderPresent/SDL_RenderCopy --> renderer ( ?, SCREEN)
 *
 * SDL2 - standard:
 *   back_surface     -- Blit --> window_surface
 *
 *   window_surface   -- RenderPresent/SDL_UpdateWindowSurface --> window
 *
 * SDL1:
 *   back_surface     -- Blit --> video_surface
 *
 *   video_surface    -- RenderPresent/SDL_Flip --> window
 */
#include "all.h"
#include "utils/display.h"
#include <SDL.h>
#if HAS_GAMECTRL
#include "controls/controller.h"
#endif

#ifdef __3DS__
#include <3ds.h>
#endif

DEVILUTION_BEGIN_NAMESPACE
#if DEBUG_MODE || DEV_MODE
unsigned _guLockCount;
#endif
/** 8-bit surface wrapper around #gpBuffer */
static SDL_Surface* back_surface;
/** Back buffer */
BYTE* gpBuffer;
/** Upper bound of back buffer. */
BYTE* gpBufStart;
/** Lower bound of back buffer. */
BYTE* gpBufEnd;
/** The width of the back buffer. */
int gnBufferWidth;
#ifndef USE_SDL1
/** Currently active palette */
static SDL_Palette* back_palette;
#endif

#if DEBUG_MODE
int locktbl[256];
static CCritSect sgMemCrit;
#endif

static void dx_create_back_buffer()
{
	gnBufferWidth = BORDER_LEFT + SCREEN_WIDTH + BORDER_LEFT;
	// The buffer needs to be divisible by 4 for the engine to blit correctly
	if (gnBufferWidth & 3) {
		gnBufferWidth += 4 - (gnBufferWidth & 3);
	}
	back_surface = SDL_CreateRGBSurfaceWithFormat(0, BUFFER_WIDTH, BUFFER_HEIGHT, 0, SDL_PIXELFORMAT_INDEX8);
	if (back_surface == NULL) {
		sdl_error(ERR_SDL_BACK_PALETTE_CREATE);
	}
	assert(back_surface->pitch == gnBufferWidth);
	gpBuffer = (BYTE*)back_surface->pixels;
	gpBufStart = &gpBuffer[BUFFER_WIDTH * SCREEN_Y]; // SCREENXY(0, 0)
	//gpBufEnd = (BYTE )(BUFFER_WIDTH * (SCREEN_Y + SCREEN_HEIGHT));
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_Y + SCREEN_HEIGHT)]; // SCREENXY(SCREEN_WIDTH, SCREEN_HEIGHT - 1)

#ifndef USE_SDL1
	// In SDL2, `back_surface` points to the global `back_palette`.
	back_palette = back_surface->format->palette;
	/*back_palette = SDL_AllocPalette(NUM_COLORS);
	if (back_palette == NULL)
		sdl_error(ERR_SDL_BACK_PALETTE_ALLOC);
	if (SDL_SetSurfacePalette(back_surface, back_palette) < 0)
		sdl_error(ERR_SDL_BACK_PALETTE_SET);*/
#else
	// In SDL1, `back_surface` owns its palette and we must update it every
	// time the global `back_palette` is changed. No need to do anything here as
	// the global `back_palette` doesn't have any colors set yet.
#endif
}

static void dx_create_primary_surface()
{
#ifndef USE_SDL1
	if (renderer != NULL) {
		int width, height;
		SDL_RenderGetLogicalSize(renderer, &width, &height);
		Uint32 format;
		if (SDL_QueryTexture(renderer_texture, &format, NULL, NULL, NULL) < 0)
			sdl_error(ERR_SDL_TEXTURE_CREATE);
		renderer_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 0, format);
	}
#endif
	if (GetOutputSurface() == NULL) {
		sdl_error(ERR_SDL_SURFACE_CHECK);
	}
}

void dx_init()
{
	SpawnWindow();
#ifndef USE_SDL1
	SDL_RaiseWindow(ghMainWnd);
	SDL_ShowWindow(ghMainWnd);
	SDL_DisableScreenSaver();
#endif

	dx_create_primary_surface();
	dx_create_back_buffer();
	InitPalette();
#ifndef USE_SDL1
	UpdatePalette();
#endif
	gbWndActive = true;
}

static void lock_buf_priv()
{
#if DEBUG_MODE
	sgMemCrit.Enter();
	if (_guLockCount != 0) {
		_guLockCount++;
		return;
	}
	assert(gpBuffer == back_surface->pixels);
	//gpBuffer = (BYTE*)back_surface->pixels;
	//gpBufEnd += (uintptr_t)gpBuffer;
	_guLockCount++;
#elif DEV_MODE
	assert(_guLockCount == 0);
	_guLockCount++;
	assert(gpBuffer == back_surface->pixels);
#endif
}

void lock_buf(BYTE idx)
{
#if DEBUG_MODE
	++locktbl[idx];
#endif
	lock_buf_priv();
}

static void unlock_buf_priv()
{
#if DEBUG_MODE
	if (_guLockCount == 0)
		app_fatal("draw main unlock error");
	if (gpBuffer == NULL)
		app_fatal("draw consistency error");
	_guLockCount--;
	//if (_guLockCount == 0) {
	//	gpBufEnd -= (uintptr_t)gpBuffer;
	//}
	sgMemCrit.Leave();
#elif DEV_MODE
	assert(_guLockCount == 1);
	_guLockCount--;
#endif
}

void unlock_buf(BYTE idx)
{
#if DEBUG_MODE
	if (locktbl[idx] == 0)
		app_fatal("Draw lock underflow: 0x%x", idx);
	--locktbl[idx];
#endif
	unlock_buf_priv();
}

void dx_cleanup()
{
#ifndef USE_SDL1
	SDL_HideWindow(ghMainWnd);
#endif
#if DEBUG_MODE
	sgMemCrit.Enter();
	_guLockCount = 0;
	gpBuffer = NULL;
	sgMemCrit.Leave();
#elif DEV_MODE
	_guLockCount = 0;
	gpBuffer = NULL;
#else
	gpBuffer = NULL;
#endif

	SDL_FreeSurface(back_surface);
	back_surface = NULL;
#ifndef USE_SDL1
	// SDL_FreePalette(back_palette);
	// back_palette = NULL;
	if (renderer != NULL) {
		SDL_FreeSurface(renderer_surface);
		renderer_surface = NULL;
		SDL_DestroyRenderer(renderer); // destroys renderer_texture as well
		renderer = NULL;
		renderer_texture = NULL;
	}
#endif
	SDL_DestroyWindow(ghMainWnd);

	/* commented out, because SDL_Quit should do this
#if HAS_GAMECTRL
	GameController::ReleaseAll();
#endif
#if HAS_JOYSTICK
	Joystick::ReleaseAll();
#endif*/

	SDL_Quit();
}
#if !FULLSCREEN_ONLY
void ToggleFullscreen()
{
#ifdef USE_SDL1
	ghMainWnd = SDL_SetVideoMode(0, 0, 0, ghMainWnd->flags ^ SDL_FULLSCREEN);
	if (ghMainWnd == NULL) {
		sdl_error(ERR_SDL_FULLSCREEN_SDL1);
	}
#else
	Uint32 flags = 0;
	if (!gbFullscreen) {
		flags = renderer != NULL ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
	}
	if (SDL_SetWindowFullscreen(ghMainWnd, flags) < 0) {
		sdl_issue(ERR_SDL_FULLSCREEN_SDL2);
		return;
	}
#endif
	gbFullscreen = !gbFullscreen;
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
}
#endif
/**
 * @brief Render the whole screen black
 */
void ClearScreenBuffer()
{
	//lock_buf(3);

	//assert(back_surface != NULL);

	//SDL_FillRect(back_surface, NULL, 0x000000);
	// BYTE *dst = &gpBuffer[SCREENXY(0, 0)];
	// BYTE *dstEnd = &gpBuffer[SCREENXY(SCREEN_WIDTH, SCREEN_HEIGHT - 1)];
	BYTE* dst = gpBufStart;
	BYTE* dstEnd = gpBufEnd;
	memset(dst, 0, (size_t)dstEnd - (size_t)dst);

	//unlock_buf(3);
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
#ifdef USE_SDL1
/*static void ScaleOutputRect(const SDL_Surface* dst, SDL_Rect* rect)
{
	rect->x = rect->x * dst->w / SCREEN_WIDTH;
	rect->y = rect->y * dst->h / SCREEN_HEIGHT;
	rect->w = rect->w * dst->w / SCREEN_WIDTH;
	rect->h = rect->h * dst->h / SCREEN_HEIGHT;
}*/
#endif
void BltFast()
{
	SDL_Surface* src = back_surface;
	SDL_Rect src_rect = {
		SCREEN_X,
		SCREEN_Y,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};

	SDL_Surface* dst = GetOutputSurface();
#ifndef USE_SDL1
	SDL_Rect dst_rect = {
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};

	if (SDL_LowerBlit(src, &src_rect, dst, &dst_rect) < 0)
		sdl_issue(ERR_SDL_DX_BLIT_SDL2);
#else
	int result;
	// ScaleOutputRect(dst, &dst_rect);
	SDL_Rect dst_rect = {
		0,
		0,
		dst->w,
		dst->h,
	};

	if (src_rect.w == dst_rect.w && src_rect.h == dst_rect.h) {
		if (SDL_BlitSurface(src, &src_rect, dst, &dst_rect) < 0)
			sdl_issue(ERR_SDL_DX_BLIT_SDL1);
		return;
	}

	// Same pixel format: We can call BlitScaled directly.
	if (SDLBackport_PixelFormatFormatEq(src->format, dst->format)) {
		if (SDL_BlitScaled(src, &src_rect, dst, &dst_rect) < 0)
			sdl_issue(ERR_SDL_DX_BLIT_SCALE);
		return;
	}

	// If the surface has a color key, we must stretch first and can then call BlitSurface.
	if (SDL_HasColorKey(src)) {
		SDL_Surface* stretched = SDL_CreateRGBSurface(SDL_SWSURFACE, dst_rect.w, dst_rect.h, src->format->BitsPerPixel,
		    src->format->Rmask, src->format->Gmask, src->format->BitsPerPixel, src->format->Amask);
		SDL_SetColorKey(stretched, SDL_SRCCOLORKEY, src->format->colorkey);
		if (src->format->palette != NULL)
			SDL_SetPalette(stretched, SDL_LOGPAL, src->format->palette->colors, 0, src->format->palette->ncolors);
		SDL_Rect stretched_rect = dst_rect;
		result = SDL_SoftStretch(src, &src_rect, stretched, &stretched_rect);
		if (result >= 0) {
			result = SDL_BlitSurface(stretched, &stretched_rect, dst, &dst_rect);
		}
		SDL_FreeSurface(stretched);
		if (result < 0)
			sdl_issue(ERR_SDL_DX_BLIT_STRETCH);
		return;
	}

	// A surface with a non-output pixel format but without a color key needs scaling.
	// We can convert the format and then call BlitScaled.
	SDL_Surface* converted = SDL_ConvertSurface(src, dst->format, 0);
	result = SDL_BlitScaled(converted, &src_rect, dst, &dst_rect);
	SDL_FreeSurface(converted);
	if (result < 0)
		sdl_issue(ERR_SDL_DX_BLIT_CONVERTED);
#endif
}

void BltStretched(SDL_Surface* surface)
{
	SDL_Rect outputRect;
	SCALE_AREA(surface->w, surface->h, SCREEN_WIDTH, SCREEN_HEIGHT, outputRect.w, outputRect.h)
	outputRect.x = SCREEN_X + ((SCREEN_WIDTH - outputRect.w) >> 1);
	outputRect.y = SCREEN_Y + ((SCREEN_HEIGHT - outputRect.h) >> 1);

	SDL_SoftStretch(surface, NULL, back_surface, &outputRect);

	BltFast();
}

/**
 * @brief Limit FPS to avoid high CPU load, use when v-sync isn't available
 */
static void LimitFrameRate()
{
	static Uint32 frameDeadline;
	Uint32 tc = SDL_GetTicks();

	if (frameDeadline > tc) {
		SDL_Delay(frameDeadline - tc);
		tc = frameDeadline;
	}
	frameDeadline = tc + gnRefreshDelay;
}

void RenderPresent()
{
	if (gbWndActive) {
		SDL_Surface* surface = GetOutputSurface();
#ifndef USE_SDL1
		if (renderer != NULL) {
			if (SDL_UpdateTexture(renderer_texture, NULL, surface->pixels, surface->pitch) < 0) {
				sdl_issue(ERR_SDL_DX_UPDATE_TEXTURE);
			}

			// Clear buffer to avoid artifacts in case the window was resized
			/* skip SDL_RenderClear since the whole screen is redrawn anyway
			if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0) { // TODO only do this if window was resized
				sdl_issue(ERR_SDL_DX_DRAW_COLOR);
			}

			if (SDL_RenderClear(renderer) < 0) {
				sdl_issue(ERR_SDL_DX_RENDER_CLEAR);
			}*/
			if (SDL_RenderCopy(renderer, renderer_texture, NULL, NULL) < 0) {
				sdl_issue(ERR_SDL_DX_RENDER_COPY);
			}
			SDL_RenderPresent(renderer);
		} else {
			if (SDL_UpdateWindowSurface(ghMainWnd) < 0) {
				sdl_issue(ERR_SDL_DX_RENDER_SURFACE);
			}
		}
#else
		if (SDL_Flip(surface) < 0) {
			sdl_issue(ERR_SDL_DX_FLIP);
		}
#endif
		if (gbFrameRateControl != FRC_CPUSLEEP)
			return;
	}
	LimitFrameRate();
}

/*
 * SDL1: Sets the colors of the video-surface and the palette of the back_surface.
 * SDL2: Sets the palette's colors.
 */
void SetSurfaceAndPaletteColors(SDL_Color* colors, int firstcolor, int ncolors)
{
	int result;
#ifdef USE_SDL1
	SDL_Surface* surface = back_surface;
#if SDL1_VIDEO_MODE_BPP == 8
	// When the video surface is 8bit, we need to set the output palette as well.
	SDL_Surface *videoSurface = SDL_GetVideoSurface();
	result = SDL_SetColors(videoSurface, colors, firstcolor, ncolors);
	if (result == 0)
		sdl_issue(ERR_SDL_PALETTE_UPDATE);
	if (videoSurface == surface) return;
#endif
	// In SDL1, the surface always has its own distinct palette, so we need to
	// update it as well.
	result = SDL_SetPalette(surface, SDL_LOGPAL, colors, firstcolor, ncolors);
	if (result == 0)
		sdl_issue(ERR_SDL_PALETTE_UPDATE);
#else // !USE_SDL1
	SDL_Palette* palette = back_palette;
	result = SDL_SetPaletteColors(palette, colors, firstcolor, ncolors);
	if (result < 0)
		sdl_issue(ERR_SDL_PALETTE_UPDATE);
#endif
}

DEVILUTION_END_NAMESPACE
