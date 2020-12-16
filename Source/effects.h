/**
 * @file effects.h
 *
 * Interface of functions for loading and playing sounds.
 */
#ifndef __EFFECTS_H__
#define __EFFECTS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int sfxdelay;
extern int sfxdnum;
extern const int sgSFXSets[NUM_SFXSets][NUM_CLASSES];

BOOL effect_is_playing(int nSFX);
void stream_stop();
void InitMonsterSND(int midx);
void FreeMonsterSnd();
BOOL calc_snd_position(int x, int y, int *plVolume, int *plPan);
void PlayEffect(int mnum, int mode);
void PlaySFX(int psfx, int rndCnt = 1);
void PlaySfxLoc(int psfx, int x, int y, int rndCnt = 1);
void sound_stop();
void sound_update();
void effects_cleanup_sfx();
void sound_init();
void ui_sound_init();
void effects_play_sound(const char *snd_file);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __EFFECTS_H__ */
