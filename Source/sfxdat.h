/**
 * @file sfxdat.h
 *
 * Interface of data related to sounds.
 */
#ifndef __SFXDAT_H__
#define __SFXDAT_H__

#ifdef NOSOUND
#include "engine.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern const int sgSFXSets[NUM_SFXSets][NUM_CLASSES];

#ifndef NOSOUND
extern SFXStruct sgSFX[NUM_SFXS];
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SFXDAT_H__ */
