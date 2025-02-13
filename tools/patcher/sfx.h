/**
 * @file sfx.h
 *
 * Interface of functions for loading and playing sounds.
 */
#ifndef __SFX_H__
#define __SFX_H__

#ifdef NOSOUND
#include "engine.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NOSOUND
void StopStreamSFX();
void StopSFX();
void PlaySfx(int nsfx);
void InitUiSFX();
void FreeUiSFX();
#else
inline void StopStreamSFX() { }
inline void StopSFX() { }
inline void PlaySfx(int nsfx) { }
inline void InitUiSFX() { }
inline void FreeUiSFX() { }
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SFX_H__ */
