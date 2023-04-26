#pragma once

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HELLFIRE
#define CREDITS_LINES_SIZE 91
#else
#define CREDITS_LINES_SIZE 455
#endif
extern const char* const CREDITS_LINES[CREDITS_LINES_SIZE];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
