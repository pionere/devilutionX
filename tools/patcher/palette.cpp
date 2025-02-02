/**
 * @file palette.cpp
 *
 * Implementation of functions for handling the engines color palette.
 */
#include "all.h"
#include "storm/storm_cfg.h"

DEVILUTION_BEGIN_NAMESPACE

static SDL_Color logical_palette[NUM_COLORS];
SDL_Color system_palette[NUM_COLORS];
static SDL_Color orig_palette[NUM_COLORS];

/** Specifies the gamma correction level. */
int _gnGammaCorrection = 100;
/** Specifies whether the game-screen is active with max brightness. */
bool _gbFadedIn = false;

void UpdatePalette()
{
#ifndef USE_SDL1
	assert(back_palette != NULL);
#endif
	if (SDLC_SetSurfaceAndPaletteColors(back_surface, back_palette, system_palette, 0, NUM_COLORS) < 0) {
		sdl_error(ERR_SDL_PALETTE_UPDATE);
	}
}

void ApplyGamma(SDL_Color* dst, const SDL_Color* src)
{
	int i;
	double g;

	if (_gnGammaCorrection == 100) {
		if (dst != src)
			memcpy(dst, src, sizeof(SDL_Color) * NUM_COLORS);
		return;
	}

	g = _gnGammaCorrection / 100.0;

	for (i = 0; i < NUM_COLORS; i++) {
		dst[i].r = (Uint8)(pow(src[i].r / 256.0, g) * 256.0);
		dst[i].g = (Uint8)(pow(src[i].g / 256.0, g) * 256.0);
		dst[i].b = (Uint8)(pow(src[i].b / 256.0, g) * 256.0);
	}
	// gbRedrawFlags = REDRAW_ALL;
}

void InitPalette()
{
	int value;

	if (!getIniInt("Graphics", "Gamma Correction", &value))
		value = 100;
	if (value < 30) {
		value = 30;
	} else if (value > 100) {
		value = 100;
	}
	_gnGammaCorrection = value;
}

void LoadPalette(const char* pszFileName)
{
	BYTE PalData[NUM_COLORS][3];

	assert(pszFileName != NULL);

	LoadFileWithMem(pszFileName, &PalData[0][0]);

	palette_create_sdl_colors(orig_palette, PalData);
}

void SetFadeLevel(unsigned fadeval)
{
	int i;

	for (i = 0; i < NUM_COLORS; i++) { // BUGFIX: should be 256 (fixed)
		system_palette[i].r = (fadeval * logical_palette[i].r) / FADE_LEVELS;
		system_palette[i].g = (fadeval * logical_palette[i].g) / FADE_LEVELS;
		system_palette[i].b = (fadeval * logical_palette[i].b) / FADE_LEVELS;
	}
	ApplyGamma(system_palette, system_palette);
	UpdatePalette();
}

void PaletteFadeIn(bool instant)
{
	int i;

	memcpy(logical_palette, orig_palette, sizeof(orig_palette));
	if (!instant) {
		Uint32 tc = SDL_GetTicks();
		for (i = 0; i < FADE_LEVELS; i = (SDL_GetTicks() - tc) >> 0) { // instead of >> 0 it was /2.083 ... 32 frames @ 60hz
			SetFadeLevel(i);
			BltFast();
			RenderPresent();
		}
	}
	SetFadeLevel(FADE_LEVELS);
	_gbFadedIn = true;
}

void PaletteFadeOut()
{
	int i;

	if (_gbFadedIn) {
		Uint32 tc = SDL_GetTicks();
		for (i = FADE_LEVELS; i > 0; i = FADE_LEVELS - ((SDL_GetTicks() - tc) >> 0)) { // instead of >> 0 it was /2.083 ... 32 frames @ 60hz
			SetFadeLevel(i);
			BltFast();
			RenderPresent();
		}
		SetFadeLevel(0);
		_gbFadedIn = false;
	}
}

DEVILUTION_END_NAMESPACE
