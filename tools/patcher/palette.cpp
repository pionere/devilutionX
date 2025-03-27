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
/** Specifies the current fade level. */
int gnFadeValue;

void UpdatePalette()
{
	SetSurfaceAndPaletteColors(system_palette, 0, NUM_COLORS);
}

void ApplyGamma(SDL_Color* dst, const SDL_Color* src)
{
	if (_gnGammaCorrection == 100) {
		if (dst != src)
			memcpy(dst, src, sizeof(SDL_Color) * NUM_COLORS);
	} else {
		int i;
		double g = _gnGammaCorrection / 100.0;

		for (i = 0; i < NUM_COLORS; i++) {
			dst[i].r = (Uint8)(pow(src[i].r / 255.0, g) * 255.0);
			dst[i].g = (Uint8)(pow(src[i].g / 255.0, g) * 255.0);
			dst[i].b = (Uint8)(pow(src[i].b / 255.0, g) * 255.0);
		}
	}
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
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

	gnFadeValue = fadeval;

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
		int fv = 0; // gnFadeValue;
		Uint32 tc = SDL_GetTicks();
		for (i = fv; i < FADE_LEVELS; i = fv + ((SDL_GetTicks() - tc) >> 0)) { // instead of >> 0 it was /2.083 ... 32 frames @ 60hz
			SetFadeLevel(i);
			BltFast();
			RenderPresent();
		}
	}
	SetFadeLevel(FADE_LEVELS);
}

void PaletteFadeOut()
{
	int i;

	if (gnFadeValue != 0) {
		int fv = FADE_LEVELS; // gnFadeValue;
		Uint32 tc = SDL_GetTicks();
		for (i = fv; i > 0; i = fv - ((SDL_GetTicks() - tc) >> 0)) { // instead of >> 0 it was /2.083 ... 32 frames @ 60hz
			SetFadeLevel(i);
			BltFast();
			RenderPresent();
		}
		SetFadeLevel(0);
	}
}

DEVILUTION_END_NAMESPACE
