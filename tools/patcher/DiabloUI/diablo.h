/**
 * @file diablo.h
 *
 * Interface of functions from the DiabloUI to the core game.
 */
#pragma once

#include <string>

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/* Defined in init.cpp */
extern std::string diabdat_paths[NUM_MPQS + 1];

/* Defined in effects.cpp */
/*#ifndef NOSOUND
void PlaySfx(int nsfx);
#else
inline void PlaySfx(int nsfx) { }
#endif*/

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
