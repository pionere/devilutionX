#include "display.h"

#include <algorithm>

#ifdef __3DS__
#include "platform/ctr/display.hpp"
#endif

#include "all.h"
#include <config.h>

#include "DiabloUI/diabloui.h"
#include "control.h"
#include "controls/controller.h"
#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"
#include "controls/touch.h"
#include "storm/storm_cfg.h"

#ifdef USE_SDL1
#ifndef SDL1_VIDEO_MODE_BPP
#define SDL1_VIDEO_MODE_BPP 0
#endif
#ifndef SDL1_VIDEO_MODE_FLAGS
#define SDL1_VIDEO_MODE_FLAGS SDL_SWSURFACE
#endif
#endif

DEVILUTION_BEGIN_NAMESPACE

/**
 * Specfies whether the game is the current active window
 */
bool gbWndActive;
/**
 * Specifies whether to give the game exclusive access to the
 * screen, as needed for efficient rendering in fullscreen mode.
 */
bool gbFullscreen = true;
/**
 * Specfies whether vertical sync or FPS limiter is used (or neither).
 */
#ifdef USE_SDL1
int gbFrameRateControl = FRC_CPUSLEEP; // use the FPS limiter
#else
int gbFrameRateControl = FRC_VSYNC;    // use vsync
#endif
/*
 * Target (screen-)refresh delay in milliseconds when
 * VSync is inactive (disabled or not available).
 * TODO: ensure gnRefreshDelay < gnTickDelay
 */
int gnRefreshDelay;
SDL_Window* ghMainWnd;
SDL_Renderer* renderer;
SDL_Texture* renderer_texture;
/** 24-bit renderer texture surface */
SDL_Surface* renderer_surface = NULL;

/** Currently active palette */
SDL_Palette* back_palette;

/** 8-bit surface wrapper around #gpBuffer */
SDL_Surface* back_surface;

int screenWidth;
int screenHeight;
//int viewportHeight;

#ifdef USE_SDL1
void SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
	DoLog("Setting video mode %dx%d bpp=%d flags=0x%08X", width, height, bpp, flags);
	ghMainWnd = SDL_SetVideoMode(width, height, bpp, flags);
	if (ghMainWnd == NULL) {
		sdl_error(ERR_SDL_DISPLAY_MODE_SET);
	}
#if DEBUG_MODE
	const SDL_VideoInfo* current = SDL_GetVideoInfo();
	DoLog("Video mode is now %dx%d bpp=%d flags=0x%08X",
	    current->current_w, current->current_h, current->vfmt->BitsPerPixel, SDL_GetVideoSurface()->flags);
#endif
}

void SetVideoModeToPrimary(int width, int height)
{
	int flags = SDL1_VIDEO_MODE_FLAGS | SDL_HWPALETTE;
	if (gbFullscreen)
		flags |= SDL_FULLSCREEN;
#ifdef __3DS__
	flags &= ~SDL_FULLSCREEN;
	bool fitToScreen = getIniBool("Graphics", "Fit to Screen", true);
	flags |= Get3DSScalingFlag(fitToScreen, width, height);
#endif
	SetVideoMode(width, height, SDL1_VIDEO_MODE_BPP, flags);
	// gbFullscreen = (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) != 0;
}
#else
void RecreateDisplay(int width, int height)
{
	if (renderer_texture != NULL)
		SDL_DestroyTexture(renderer_texture);

	renderer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_UNKNOWN, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (renderer_texture == NULL) {
		sdl_error(ERR_SDL_RENDERER_TEXTURE);
	}

	if (SDL_RenderSetLogicalSize(renderer, width, height) < 0) {
		sdl_error(ERR_SDL_RENDERER_SIZE);
	}
}
#endif // USE_SDL1

static void AdjustToScreenGeometry(int width, int height)
{
	screenWidth = width;
	screenHeight = height;
#ifdef USE_SDL1
	if (OutputRequiresScaling()) {
		DoLog("Using software scaling");
	}
#endif
}

#ifndef USE_SDL1
static void CalculatePreferredWindowSize(int& width, int& height, bool useIntegerScaling)
{
	SDL_DisplayMode mode;
	if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
		sdl_error(ERR_SDL_DISPLAY_MODE_GET);
	}

	if (mode.w < mode.h) {
		std::swap(mode.w, mode.h);
	}

	if (useIntegerScaling) {
		int factor = std::min(mode.w / width, mode.h / height);
		width = mode.w / factor;
		height = mode.h / factor;
		return;
	}

	float wFactor = (float)mode.w / width;
	float hFactor = (float)mode.h / height;

	if (wFactor > hFactor) {
		width = mode.w * height / mode.h;
	} else {
		height = mode.h * width / mode.w;
	}
}
#endif

void SpawnWindow()
{
	const char* lpWindowName = PROJECT_NAME;

#if !defined(USE_SDL1) && (__WINRT__ || __ANDROID__ || __IPHONEOS__)
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 2) && __ANDROID__ && (HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD)
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
	SDL_SetHint(SDL_HINT_IME_INTERNAL_EDITING, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 6) && defined(__vita__)
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 10)
	SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
#endif

	int initFlags = SDL_INIT_VIDEO;
#ifndef NOSOUND
	initFlags |= SDL_INIT_AUDIO;
#endif
#if HAS_JOYSTICK
	initFlags |= SDL_INIT_JOYSTICK;
#endif
#if !defined(USE_SDL1) && (HAS_GAMECTRL || HAS_KBCTRL || HAS_DPAD)
	initFlags |= SDL_INIT_GAMECONTROLLER;
#endif
	if (SDL_Init(initFlags) < 0) {
		sdl_error(ERR_SDL_INIT);
	}

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#ifndef USE_SDL1
	char mapping[1024];
	if (getIniValue("Controller", "sdl2_controller_mapping", mapping, 1024)) {
		SDL_GameControllerAddMapping(mapping);
	}
#endif
#endif

#ifdef USE_SDL1
#if HAS_JOYSTICK
	// On SDL 1, there are no ADDED/REMOVED events.
	// Always try to initialize the first joystick.
	Joystick::Add(0);
#endif
#ifdef __SWITCH__
#if HAS_GAMECTRL
	// TODO: There is a bug in SDL2 on Switch where it does not report controllers on startup (Jan 1, 2020)
	GameController::Add(0);
#endif
#endif
#endif
	int width = DEFAULT_WIDTH;
	int height = DEFAULT_HEIGHT;
#if 0
	int smode = 0; 
	switch (smode) {
	case 0: // oe
		width = DEFAULT_WIDTH + 64;			// 704
		height = DEFAULT_HEIGHT + 48;		// 528
		break;
	case 1: // oo
		width = DEFAULT_WIDTH + 64 + 32;	// 736
		height = DEFAULT_HEIGHT;			// 552
		break;
	case 2: // EE
		width = DEFAULT_WIDTH + (32 * 4 + 2) / 3 + 1;	// 684
		height = DEFAULT_HEIGHT;						// 513
		break;
	case 3:// EO
		width = DEFAULT_WIDTH;
		height = DEFAULT_HEIGHT;
		break;
	}
#else
	getIniInt("Graphics", "Width", &width);
	getIniInt("Graphics", "Height", &height);
#endif
#if !FULLSCREEN_ONLY
	if (gbFullscreen)
		gbFullscreen = getIniBool("Graphics", "Fullscreen", true);
#endif
	getIniInt("Graphics", "Frame Rate Control", &gbFrameRateControl);

	bool grabInput = getIniBool("Diablo", "Grab Input", false);

#ifdef USE_SDL1
	SDL_WM_SetCaption(lpWindowName, WINDOW_ICON_NAME);
	SetVideoModeToPrimary(width, height);
	if (grabInput)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	atexit(SDL_VideoQuit); // Without this video mode is not restored after fullscreen.
	const SDL_VideoInfo* current = SDL_GetVideoInfo();
	width = current->current_w;
	height = current->current_h;
#else
	bool integerScalingEnabled = getIniBool("Graphics", "Integer Scaling", false);
	bool upscale = getIniBool("Graphics", "Upscale", true);
	bool fitToScreen = getIniBool("Graphics", "Fit to Screen", true);

	if (upscale && fitToScreen) {
		CalculatePreferredWindowSize(width, height, integerScalingEnabled);
	}

	int flags = SDL_WINDOW_ALLOW_HIGHDPI;
	if (grabInput) {
		flags |= SDL_WINDOW_INPUT_GRABBED;
	}
	if (upscale) {
		if (gbFullscreen) {
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		flags |= SDL_WINDOW_RESIZABLE;

		char scaleQuality[2] = "2";
		getIniValue("Graphics", "Scaling Quality", scaleQuality, 2);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality);
	} else if (gbFullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	ghMainWnd = SDL_CreateWindow(lpWindowName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (ghMainWnd == NULL) {
		sdl_error(ERR_SDL_WINDOW_CREATE);
	}
	// gbFullscreen = (SDL_GetWindowFlags(ghMainWnd) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
	if (upscale) {
		Uint32 rendererFlags = 0;

		if (gbFrameRateControl == FRC_VSYNC) {
			rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
		}

		renderer = SDL_CreateRenderer(ghMainWnd, -1, rendererFlags);
		if (renderer == NULL) {
			sdl_error(ERR_SDL_RENDERER_CREATE);
		}

		if (integerScalingEnabled && SDL_RenderSetIntegerScale(renderer, SDL_TRUE) < 0) {
			sdl_error(ERR_SDL_RENDERER_SCALE);
		}

		RecreateDisplay(width, height);
	} else {
		SDL_GetWindowSize(ghMainWnd, &width, &height);
	}
#endif

	AdjustToScreenGeometry(width, height);

#if HAS_TOUCHPAD
	InitTouch();
#endif
	int refreshRate = 60;
#ifndef USE_SDL1
	SDL_DisplayMode mode;
	// TODO: use SDL_GetCurrentDisplayMode after window is shown?
	if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
		refreshRate = mode.refresh_rate;
	}
#endif
	gnRefreshDelay = 1000 / refreshRate;

	// return ghMainWnd != NULL;
}

SDL_Surface* GetOutputSurface()
{
#ifdef USE_SDL1
	return SDL_GetVideoSurface();
#else
	if (renderer != NULL)
		return renderer_surface;
	return SDL_GetWindowSurface(ghMainWnd);
#endif
}

#ifdef USE_SDL1
bool OutputRequiresScaling()
{
	return SCREEN_WIDTH != GetOutputSurface()->w || SCREEN_HEIGHT != GetOutputSurface()->h;
}

void ScaleOutputRect(SDL_Rect* rect)
{
	if (!OutputRequiresScaling())
		return;
	const SDL_Surface* surface = GetOutputSurface();
	rect->x = rect->x * surface->w / SCREEN_WIDTH;
	rect->y = rect->y * surface->h / SCREEN_HEIGHT;
	rect->w = rect->w * surface->w / SCREEN_WIDTH;
	rect->h = rect->h * surface->h / SCREEN_HEIGHT;
}

static SDL_Surface* CreateScaledSurface(SDL_Surface* src)
{
	SDL_Rect stretched_rect = { 0, 0, static_cast<Uint16>(src->w), static_cast<Uint16>(src->h) };
	ScaleOutputRect(&stretched_rect);
	SDL_Surface* stretched = SDL_CreateRGBSurface(
	    SDL_SWSURFACE, stretched_rect.w, stretched_rect.h, src->format->BitsPerPixel,
	    src->format->Rmask, src->format->Gmask, src->format->Bmask, src->format->Amask);
	if (SDL_HasColorKey(src)) {
		SDL_SetColorKey(stretched, SDL_SRCCOLORKEY, src->format->colorkey);
		if (src->format->palette != NULL)
			SDL_SetPalette(stretched, SDL_LOGPAL, src->format->palette->colors, 0, src->format->palette->ncolors);
	}
	if (SDL_SoftStretch((src), NULL, stretched, &stretched_rect) < 0) {
		SDL_FreeSurface(stretched);
		sdl_error(ERR_SDL_WINDOW_STRETCH);
	}
	return stretched;
}

void ScaleSurfaceToOutput(SDL_Surface** surface)
{
	if (!OutputRequiresScaling())
		return;
	SDL_Surface* stretched = CreateScaledSurface(*surface);
	SDL_FreeSurface((*surface));
	*surface = stretched;
}
#endif // USE_SDL1

DEVILUTION_END_NAMESPACE
