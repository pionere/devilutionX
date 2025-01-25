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
void StopStreamSFX();
void StopSFX();
void CheckStreamSFX();
void InitMonsterSFX(int midx);
void FreeMonsterSFX();
void PlayMonSFX(int mnum, int mode);
void PlaySFX(int psfx);
void PlaySfxN(int psfx, int rndCnt);
void PlaySfxLoc(int psfx, int x, int y);
void PlaySfxLocN(int psfx, int x, int y, int rndCnt);
void PlayWalkSfx(int pnum);
void InitGameSFX();
void InitUiSFX();
void FreeGameSFX();
void FreeUiSFX();
#else
inline bool IsSFXPlaying(int nSFX) { return false; }
inline void StopStreamSFX() { }
inline void StopSFX() { }
inline void CheckStreamSFX() { }
inline void InitMonsterSFX(int midx) { }
inline void FreeMonsterSFX() { }
inline void PlayMonSFX(int mnum, int mode) { PlaySfxN(-1, 2); }
inline void PlaySFX(int psfx) { }
inline void PlaySfxN(int psfx, int rndCnt) { if (rndCnt > 1) random_low(165, rndCnt); }
inline void PlaySfxLoc(int psfx, int x, int y) { }
inline void PlaySfxLocN(int psfx, int x, int y, int rndCnt) { PlaySfxN(-1, rndCnt); }
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
