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
static int _gnGammaCorrection = 100;
/** Specifies whether colour cycling is enabled. */
bool gbColorCyclingEnabled = true;
/** Specifies whether the game-screen is active with max brightness. */
static bool _gbFadedIn = false;

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
	int value = 100;

	getIniInt("Graphics", "Gamma Correction", &value);

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
	BYTE PalData[NUM_COLORS][3];

	assert(pszFileName != NULL);

	LoadFileWithMem(pszFileName, &PalData[0][0]);

	palette_create_sdl_colors(orig_palette, PalData);
}

void LoadLvlPalette()
{
	int rv;
	char szFileName[DATA_ARCHIVE_MAX_PATH];

	rv = RandRange(1, 4);
	snprintf(szFileName, sizeof(szFileName), AllLevels[currLvl._dLevelNum].dPalName, rv);
	LoadPalette(szFileName);
}

int GetGamma()
{
	return _gnGammaCorrection;
}

void SetGamma(int gamma)
{
	_gnGammaCorrection = gamma;
	setIniInt("Graphics", "Gamma Correction", gamma);
	ApplyGamma(system_palette, logical_palette);
	UpdatePalette();
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

/*void palette_update_caves()
{
	int i;
	SDL_Color col;

	col = system_palette[1];
	for (i = 1; i < 31; i++) {
		system_palette[i] = system_palette[i + 1];
	}
	system_palette[i] = col;

	UpdatePalette();
}*/

#ifdef HELLFIRE
/*static int cryptCycleCounter = 3;
void palette_update_crypt()
{
	int i;
	SDL_Color col;

	if (--cryptCycleCounter == 0) {
		cryptCycleCounter = 3;

		col = system_palette[15];
		for (i = 15; i > 1; i--) {
			system_palette[i] = system_palette[i - 1];
		}
		system_palette[i] = col;

	}
	col = system_palette[31];
	for (i = 31; i > 16; i--) {
		system_palette[i] = system_palette[i - 1];
	}
	system_palette[i] = col;

	UpdatePalette();
}

static int nestCycleCounter = 3;
void palette_update_nest()
{
	int i;
	SDL_Color col;

	if (--nestCycleCounter != 0) {
		return;
	}
	nestCycleCounter = 3;

	col = system_palette[8];
	for (i = 8; i > 1; i--) {
		system_palette[i] = system_palette[i - 1];
	}
	system_palette[i] = col;

	col = system_palette[15];
	for (i = 15; i > 9; i--) {
		system_palette[i] = system_palette[i - 1];
	}
	system_palette[i] = col;

	UpdatePalette();
}*/
#endif

void palette_update_quest_palette(int n)
{
	int i;

	for (i = 32 - n; i >= 0; i--) {
		logical_palette[i] = orig_palette[i];
	}
	ApplyGamma(system_palette, logical_palette); // 32
	UpdatePalette();
}

DEVILUTION_END_NAMESPACE
