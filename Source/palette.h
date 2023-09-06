/**
 * @file palette.h
 *
 * Interface of functions for handling the engines color palette.
 */
#ifndef __PALETTE_H__
#define __PALETTE_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define FADE_LEVELS 256

extern bool gbColorCyclingEnabled;

extern SDL_Color logical_palette[NUM_COLORS];
extern SDL_Color system_palette[NUM_COLORS];
extern SDL_Color orig_palette[NUM_COLORS];

void palette_update();
void palette_init();
void LoadPalette(const char* pszFileName);
void LoadLvlPalette();
void IncreaseGamma();
void ApplyGamma(SDL_Color* dst, const SDL_Color* src);
void DecreaseGamma();
void UpdateGamma(int gamma);
int GetGamma();
void SetFadeLevel(unsigned fadeval);
void PaletteFadeIn(bool instant);
void PaletteFadeOut();
void palette_update_caves();
#ifdef HELLFIRE
void palette_update_crypt();
// void palette_update_nest();
#endif
void palette_update_quest_palette(int n);

// Create a palette of SDL_Colors.
inline void palette_create_sdl_colors(SDL_Color (&DVL_RESTRICT colors)[NUM_COLORS], BYTE (&DVL_RESTRICT paldata)[NUM_COLORS][3])
{
	for (int i = 0; i < NUM_COLORS; i++) {
		colors[i].r = paldata[i][0];
		colors[i].g = paldata[i][1];
		colors[i].b = paldata[i][2];
#ifndef USE_SDL1
		colors[i].a = SDL_ALPHA_OPAQUE;
#endif
	}
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PALETTE_H__ */
