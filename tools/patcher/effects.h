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

#ifndef NOSOUND
bool IsSFXPlaying(int nSFX);
void StopStreamSFX();
void StopSFX();
void CheckStreamSFX();
void PlaySFX(int psfx);
void InitUiSFX();
void FreeUiSFX();
#else
inline bool IsSFXPlaying(int nSFX) { return false; }
inline void StopStreamSFX() { }
inline void StopSFX() { }
inline void CheckStreamSFX() { }
inline void PlaySFX(int psfx) { }
inline void InitUiSFX() { }
inline void FreeUiSFX() { }
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __EFFECTS_H__ */
