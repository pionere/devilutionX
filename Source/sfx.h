/**
 * @file sfx.h
 *
 * Interface of functions for loading and playing sounds.
 */
#ifndef __SFX_H__
#define __SFX_H__

#ifdef NOSOUND
#include "engine/engine.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

static_assert(NUM_SFXS <= INT_MAX, "SFX_VALID checks only the sign of the SFX_-value I.");
static_assert(SFX_NONE < 0, "SFX_VALID checks only the sign of the SFX_-value II.");
#define SFX_VALID(x) (x >= 0)

#ifdef __cplusplus
extern "C" {
#endif

extern const int sgSFXSets[NUM_SFXSets][NUM_CLASSES];

#ifndef NOSOUND
bool IsSfxStreaming(int nsfx);
// bool IsSfxPlaying(int nsfx);
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
inline bool IsSfxStreaming(int nsfx) { return false; }
// inline bool IsSfxPlaying(int nsfx) { return false; }
inline void StopStreamSFX() { }
inline void StopSFX() { }
inline void CheckStreamSFX() { }
inline void InitMonsterSFX(int midx) { }
inline void FreeMonsterSFX() { }
inline void PlayMonSfx(int mnum, int mode) { }
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

#endif /* __SFX_H__ */
