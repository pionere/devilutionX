/**
 * @file palette.cpp
 *
 * Implementation of functions for handling the engines color palette.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

SDL_Color logical_palette[256];
SDL_Color system_palette[256];
SDL_Color orig_palette[256];

/** Specifies the gamma correction level. */
int _gnGammaCorrection = 100;
/** Specifies whether colour cycling is enabled. */
bool gbColorCyclingEnabled = true;
/** Specifies whether the game-screen is active with max brightness. */
bool _gbFadedIn = false;

void palette_update()
{
#ifndef USE_SDL1
	assert(back_palette != NULL);
#endif
	if (SDLC_SetSurfaceAndPaletteColors(back_surface, back_palette, system_palette, 0, 256) < 0) {
		sdl_fatal(ERR_SDL_PALETTE_UPDATE);
	}
	back_surface_palette_version++;
}

void ApplyGamma(SDL_Color* dst, const SDL_Color* src)
{
	int i;
	double g;

	if (_gnGammaCorrection == 100) {
		memcpy(dst, src, sizeof(SDL_Color) * 256);
		return;
	}

	g = _gnGammaCorrection / 100.0;

	for (i = 0; i < 256; i++) {
		dst[i].r = pow(src[i].r / 256.0, g) * 256.0;
		dst[i].g = pow(src[i].g / 256.0, g) * 256.0;
		dst[i].b = pow(src[i].b / 256.0, g) * 256.0;
	}
	gbRedrawFlags = REDRAW_ALL;
}

void palette_init()
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

	gbColorCyclingEnabled = getIniBool("Graphics", "Color Cycling", true);
}

void LoadPalette(const char* pszFileName)
{
	BYTE PalData[256][3];

	assert(pszFileName != NULL);

	LoadFileWithMem(pszFileName, &PalData[0][0]);

	palette_create_sdl_colors(orig_palette, PalData);
}

void LoadRndLvlPal()
{
	int rv;
	char szFileName[MAX_PATH];

	rv = RandRange(1, 4);
	snprintf(szFileName, sizeof(szFileName), AllLevels[currLvl._dLevelIdx].dPalName, rv);
	LoadPalette(szFileName);
}

void IncreaseGamma()
{
	if (_gnGammaCorrection < 100) {
		_gnGammaCorrection += 5;
		if (_gnGammaCorrection > 100)
			_gnGammaCorrection = 100;
		ApplyGamma(system_palette, logical_palette);
		palette_update();
	}
}

void DecreaseGamma()
{
	if (_gnGammaCorrection > 30) {
		_gnGammaCorrection -= 5;
		if (_gnGammaCorrection < 30)
			_gnGammaCorrection = 30;
		ApplyGamma(system_palette, logical_palette);
		palette_update();
	}
}

void UpdateGamma(int gamma)
{
	gamma = 130 - gamma;
	setIniInt("Graphics", "Gamma Correction", gamma);
	_gnGammaCorrection = gamma;
	ApplyGamma(system_palette, logical_palette);
	palette_update();
}

int GetGamma()
{
	return 130 - _gnGammaCorrection;
}

void SetFadeLevel(unsigned fadeval)
{
	int i;

	for (i = 0; i < 256; i++) { // BUGFIX: should be 256 (fixed)
		system_palette[i].r = (fadeval * logical_palette[i].r) >> 8;
		system_palette[i].g = (fadeval * logical_palette[i].g) >> 8;
		system_palette[i].b = (fadeval * logical_palette[i].b) >> 8;
	}
	palette_update();
}

void PaletteFadeIn(bool instant)
{
	int i;

	ApplyGamma(logical_palette, orig_palette);
	if (!instant) {
		Uint32 tc = SDL_GetTicks();
		for (i = 0; i < 256; i = (SDL_GetTicks() - tc) >> 0) { // instead of >> 0 it was /2.083 ... 32 frames @ 60hz
			SetFadeLevel(i);
			BltFast();
			RenderPresent();
		}
	}
	SetFadeLevel(256);
	memcpy(logical_palette, orig_palette, sizeof(orig_palette));
	_gbFadedIn = true;
}

void PaletteFadeOut()
{
	int i;

	if (_gbFadedIn) {
		Uint32 tc = SDL_GetTicks();
		for (i = 256; i > 0; i = 256 - ((SDL_GetTicks() - tc) >> 0)) { // instead of >> 0 it was /2.083 ... 32 frames @ 60hz
			SetFadeLevel(i);
			BltFast();
			RenderPresent();
		}
		SetFadeLevel(0);
		_gbFadedIn = false;
	}
}

void palette_update_caves()
{
	int i;
	SDL_Color col;

	col = system_palette[1];
	for (i = 1; i < 31; i++) {
		system_palette[i] = system_palette[i + 1];
	}
	system_palette[i] = col;

	palette_update();
}

#ifdef HELLFIRE
int dword_6E2D58;
int dword_6E2D54;
void palette_update_crypt()
{
	int i;
	SDL_Color col;

	if (dword_6E2D58 > 1) {
		col = system_palette[15];
		for (i = 15; i > 1; i--) {
			system_palette[i] = system_palette[i - 1];
		}
		system_palette[i] = col;

		dword_6E2D58 = 0;
	} else {
		dword_6E2D58++;
	}
	if (dword_6E2D54 > 0) {
		col = system_palette[31];
		for (i = 31; i > 16; i--) {
			system_palette[i] = system_palette[i - 1];
		}
		system_palette[i] = col;

		palette_update();
		dword_6E2D54++;
	} else {
		dword_6E2D54 = 1;
	}
}

int dword_6E2D5C;
int dword_6E2D60;
void palette_update_hive()
{
	int i;
	SDL_Color col;

	if (dword_6E2D60 == 2) {
		col = system_palette[8];
		for (i = 8; i > 1; i--) {
			system_palette[i] = system_palette[i - 1];
		}
		system_palette[i] = col;

		dword_6E2D60 = 0;
	} else {
		dword_6E2D60++;
	}
	if (dword_6E2D5C == 2) {
		col = system_palette[15];
		for (i = 15; i > 9; i--) {
			system_palette[i] = system_palette[i - 1];
		}
		system_palette[i] = col;

		palette_update();
		dword_6E2D5C = 0;
	} else {
		dword_6E2D5C++;
	}
}
#endif

void palette_update_quest_palette(int n)
{
	int i;

	for (i = 32 - n; i >= 0; i--) {
		logical_palette[i] = orig_palette[i];
	}
	ApplyGamma(system_palette, logical_palette); // 32
	palette_update();
}

DEVILUTION_END_NAMESPACE
