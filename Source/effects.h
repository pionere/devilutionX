/**
 * @file effects.h
 *
 * Interface of functions for loading and playing sounds.
 */
#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#ifdef NOSOUND
#include "engine.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int sfxdelay;
extern int sfxdnum;
extern const int sgSFXSets[NUM_SFXSets][NUM_CLASSES];

#ifndef NOSOUND
bool effect_is_playing(int nSFX);
void stream_stop();
void InitMonsterSND(int midx);
void FreeMonsterSnd();
void PlayEffect(int mnum, int mode);
void PlaySFX(int psfx, int rndCnt = 1);
void PlaySfxLoc(int psfx, int x, int y, int rndCnt = 1);
void sound_stop();
void sound_pause(bool pause);
void sound_update();
void FreeGameEffects();
void FreeUiEffects();
void InitGameEffects();
void InitUiEffects();
void effects_play_sound(const char* snd_file);
#else
inline bool effect_is_playing(int nSFX) { return false; }
inline void stream_stop() { }
inline void PlaySFX(int psfx, int rndCnt = 1) { if (rndCnt != 1) random_(165, rndCnt); }
inline void PlaySfxLoc(int psfx, int x, int y, int rndCnt = 1) { PlaySFX(-1, rndCnt); }
inline void InitMonsterSND(int midx) { }
inline void FreeMonsterSnd() { }
inline void PlayEffect(int mnum, int mode) { PlaySFX(-1, 2); }
inline void sound_stop() { }
inline void sound_pause(bool pause) { }
inline void sound_update() { }
inline void FreeGameEffects() { }
inline void FreeUiEffects() { }
inline void InitGameEffects() { }
inline void InitUiEffects() { }
inline void effects_play_sound(const char* snd_file) { }
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __EFFECTS_H__ */
