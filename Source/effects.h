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
bool IsSfxPlaying(int nsfx);
void StopStreamSFX();
void StopSFX();
void CheckStreamSFX();
void InitMonsterSFX(int midx);
void FreeMonsterSFX();
void PlayMonSfx(int mnum, int mode);
void PlaySfx(int nsfx);
void PlaySfxN(int nsfx, int rndCnt);
void PlaySfxLoc(int nsfx, int x, int y);
void PlaySfxLocN(int nsfx, int x, int y, int rndCnt);
void PlayWalkSfx(int pnum);
void InitGameSFX();
void InitUiSFX();
void FreeGameSFX();
void FreeUiSFX();
#else
inline bool IsSfxPlaying(int nsfx) { return false; }
inline void StopStreamSFX() { }
inline void StopSFX() { }
inline void CheckStreamSFX() { }
inline void InitMonsterSFX(int midx) { }
inline void FreeMonsterSFX() { }
inline void PlayMonSfx(int mnum, int mode) { PlaySfxN(-1, 2); }
inline void PlaySfx(int nsfx) { }
inline void PlaySfxN(int nsfx, int rndCnt) { if (rndCnt > 1) random_low(165, rndCnt); }
inline void PlaySfxLoc(int nsfx, int x, int y) { }
inline void PlaySfxLocN(int nsfx, int x, int y, int rndCnt) { PlaySfxN(-1, rndCnt); }
inline void PlayWalkSfx(int pnum) { }
inline void InitGameSFX() { }
inline void InitUiSFX() { }
inline void FreeGameSFX() { }
inline void FreeUiSFX() { }
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __EFFECTS_H__ */
