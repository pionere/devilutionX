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

extern const int sgSFXSets[NUM_SFXSets][NUM_CLASSES];

#ifndef NOSOUND
bool IsSFXPlaying(int nSFX);
void CheckStreamSFX();
void StopStreamSFX();
void InitMonsterSFX(int midx);
void FreeMonsterSFX();
void PlayMonSFX(int mnum, int mode);
void PlaySFX(int psfx, int rndCnt = 1);
void PlaySfxLoc(int psfx, int x, int y, int rndCnt = 1);
void FreeGameSFX();
void FreeUiSFX();
void InitGameSFX();
void InitUiSFX();
#else
inline bool IsSFXPlaying(int nSFX) { return false; }
inline void CheckStreamSFX() { }
inline void StopStreamSFX() { }
inline void PlaySFX(int psfx, int rndCnt = 1) { if (rndCnt > 1) random_low(165, rndCnt); }
inline void PlaySfxLoc(int psfx, int x, int y, int rndCnt = 1) { PlaySFX(-1, rndCnt); }
inline void InitMonsterSFX(int midx) { }
inline void FreeMonsterSFX() { }
inline void PlayMonSFX(int mnum, int mode) { PlaySFX(-1, 2); }
inline void FreeGameSFX() { }
inline void FreeUiSFX() { }
inline void InitGameSFX() { }
inline void InitUiSFX() { }
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __EFFECTS_H__ */
