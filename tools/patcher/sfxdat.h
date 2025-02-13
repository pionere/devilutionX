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

#ifndef NOSOUND
extern const SFXData sfxdata[61];
extern const SFXFileData sfxfiledata[61];
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SFXDAT_H__ */
