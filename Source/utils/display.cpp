#include "display.h"

#include <algorithm>

#ifdef __vita__
#include <psp2/power.h>
#endif

#ifdef __3DS__
#include "platform/ctr/display.hpp"
#endif

#include "all.h"

#include "DiabloUI/diabloui.h"
#include "control.h"
#include "controls/controller.h"
#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"
#include "controls/game_controls.h"

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
 * Specfies whether vertical sync is enabled.
 */
bool gbVsyncEnabled;
/**
 * Specfies whether the FPS limiter is enabled to reduce CPU load.
 */
bool gbFPSLimit;
/**
 * Specfies whether the FPS counter is shown.
 */
bool gbShowFPS;
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
unsigned int back_surface_palette_version = 0;

/** 8-bit surface wrapper around #gpBuffer */
SDL_Surface* back_surface;

int screenWidth;
int screenHeight;
//int viewportHeight;

#ifdef USE_SDL1
void SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
	SDL_Log("Setting video mode %dx%d bpp=%u flags=0x%08X", width, height, bpp, flags);
	ghMainWnd = SDL_SetVideoMode(width, height, bpp, flags);
	if (ghMainWnd == NULL) {
		sdl_fatal(ERR_SDL_DISPLAY_MODE_SET);
	}
	const SDL_VideoInfo &current = *SDL_GetVideoInfo();
	SDL_Log("Video mode is now %dx%d bpp=%u flags=0x%08X",
	    current.current_w, current.current_h, current.vfmt->BitsPerPixel, SDL_GetVideoSurface()->flags);
}

void SetVideoModeToPrimary(bool fullscreen, int width, int height)
{
	int flags = SDL1_VIDEO_MODE_FLAGS | SDL_HWPALETTE;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;
#ifdef __3DS__
	flags &= ~SDL_FULLSCREEN;
	bool fitToScreen = getIniBool("Graphics", "Fit to Screen", true);
	flags |= Get3DSScalingFlag(fitToScreen, width, height);
#endif
	SetVideoMode(width, height, SDL1_VIDEO_MODE_BPP, flags);
}

bool IsFullScreen() {
	return (SDL_GetVideoSurface()->flags & SDL_FULLSCREEN) != 0;
	// ifndef USE_SDL1:
	//   return (SDL_GetWindowFlags(ghMainWnd) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}
#else
void RecreateDisplay(int width, int height)
{
	if (renderer_texture != NULL)
		SDL_DestroyTexture(renderer_texture);

	renderer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (renderer_texture == NULL) {
		sdl_fatal(ERR_SDL_RENDERER_TEXTURE);
	}

	if (SDL_RenderSetLogicalSize(renderer, width, height) < 0) {
		sdl_fatal(ERR_SDL_RENDERER_SIZE);
	}
}
#endif // USE_SDL1

static void AdjustToScreenGeometry(int width, int height)
{
	screenWidth = width;
	screenHeight = height;
#ifdef USE_SDL1
	if (OutputRequiresScaling())
		SDL_Log("Using software scaling");
#endif
	//viewportHeight = screenHeight;
	/*if (screenWidth <= PANEL_WIDTH) {
		// Part of the screen is fully obscured by the UI
		viewportHeight -= PANEL_HEIGHT;
	}*/
}

#ifndef USE_SDL1
static void CalculatePreferredWindowSize(int &width, int &height, bool useIntegerScaling)
{
	SDL_DisplayMode mode;
	if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
		sdl_fatal(ERR_SDL_DISPLAY_MODE_GET);
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

void SpawnWindow(const char* lpWindowName)
{
#ifdef __vita__
	scePowerSetArmClockFrequency(444);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 6) && defined(__vita__)
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 10)
	SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
#endif
#if (__WINRT__ || __ANDROID__ || __IPHONEOS__) && !USE_SDL1
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

//#ifdef _WIN32
//	// The default WASAPI backend causes distortions
//	// https://github.com/diasurgical/devilutionX/issues/1434
//	SDL_setenv("SDL_AUDIODRIVER", "winmm", /*overwrite=*/false);
//#endif

	int initFlags = SDL_INIT_VIDEO;
#ifndef NOSOUND
	initFlags |= SDL_INIT_AUDIO;
#endif
#if HAS_JOYSTICK == 1
	initFlags |= SDL_INIT_JOYSTICK;
#endif
#if HAS_GAMECTRL == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	initFlags |= SDL_INIT_GAMECONTROLLER;
#endif
	if (SDL_Init(initFlags) < 0) {
		sdl_fatal(ERR_SDL_INIT);
	}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	dpad_hotkeys = getIniBool("Controller", "dpad_hotkeys", false);
	switch_potions_and_clicks = getIniBool("Controller", "switch_potions_and_clicks", false);

#ifndef USE_SDL1
	char mapping[1024];
	if (getIniValue("Controller", "sdl2_controller_mapping", mapping, 1024)) {
		SDL_GameControllerAddMapping(mapping);
	}
#endif
#endif

#ifdef USE_SDL1
	SDL_EnableUNICODE(1);
#if HAS_JOYSTICK == 1
	// On SDL 1, there are no ADDED/REMOVED events.
	// Always try to initialize the first joystick.
	Joystick::Add(0);
#endif
#ifdef __SWITCH__
#if HAS_GAMECTRL == 1
	// TODO: There is a bug in SDL2 on Switch where it does not report controllers on startup (Jan 1, 2020)
	GameController::Add(0);
#endif
#endif
#endif

	int width = DEFAULT_WIDTH;
	int height = DEFAULT_HEIGHT;
	getIniInt("Graphics", "Width", &width);
	getIniInt("Graphics", "Height", &height);
	bool integerScalingEnabled = getIniBool("Graphics", "Integer Scaling", false);

#ifndef __vita__
	if (gbFullscreen)
		gbFullscreen = getIniBool("Graphics", "Fullscreen", true);
#endif

	bool grabInput = getIniBool("Diablo", "Grab Input", false);

#ifdef USE_SDL1
	SDL_WM_SetCaption(lpWindowName, WINDOW_ICON_NAME);
	SetVideoModeToPrimary(gbFullscreen, width, height);
	if (grabInput)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	atexit(SDL_VideoQuit); // Without this video mode is not restored after fullscreen.
	const SDL_VideoInfo &current = *SDL_GetVideoInfo();
	width = current.current_w;
	height = current.current_h;
#else
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
		sdl_fatal(ERR_SDL_WINDOW_CREATE);
	}

	if (upscale) {
		Uint32 rendererFlags = 0;

		gbVsyncEnabled = getIniBool("Graphics", "Vertical Sync", true);
		if (gbVsyncEnabled) {
			rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
		}

		renderer = SDL_CreateRenderer(ghMainWnd, -1, rendererFlags);
		if (renderer == NULL) {
			sdl_fatal(ERR_SDL_RENDERER_CREATE);
		}

		if (integerScalingEnabled && SDL_RenderSetIntegerScale(renderer, SDL_TRUE) < 0) {
			sdl_fatal(ERR_SDL_RENDERER_SCALE);
		}

		RecreateDisplay(width, height);
	} else {
		SDL_GetWindowSize(ghMainWnd, &width, &height);
	}
#endif

	AdjustToScreenGeometry(width, height);

	int refreshRate = 60;
#ifndef USE_SDL1
	SDL_DisplayMode mode;
	// TODO: use SDL_GetCurrentDisplayMode after window is shown?
	if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
		refreshRate = mode.refresh_rate;
	}
#endif
	gnRefreshDelay = 1000 / refreshRate;

	gbFPSLimit = getIniBool("Graphics", "FPS Limiter", true);
	gbShowFPS = getIniBool("Graphics", "Show FPS", false);

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
		sdl_fatal(ERR_SDL_WINDOW_STRETCH);
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
