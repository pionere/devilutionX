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

extern bool gbColorCyclingEnabled;

extern SDL_Color logical_palette[256];
extern SDL_Color system_palette[256];
extern SDL_Color orig_palette[256];

void palette_update();
void palette_init();
void LoadPalette(const char *pszFileName);
void LoadRndLvlPal();
void IncreaseGamma();
void ApplyGamma(SDL_Color* dst, const SDL_Color* src);
void DecreaseGamma();
void UpdateGamma(int gamma);
int GetGamma();
void SetFadeLevel(unsigned fadeval);
void PaletteFadeIn();
void PaletteFadeOut();
void palette_update_caves();
#ifdef HELLFIRE
void palette_update_crypt();
void palette_update_hive();
#endif
void palette_update_quest_palette(int n);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PALETTE_H__ */
