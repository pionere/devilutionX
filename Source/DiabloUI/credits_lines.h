#pragma once

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HELLFIRE
#define CREDITS_LINE_COUNT 91
#else
#define CREDITS_LINE_COUNT 455
#endif
extern const char* const CREDITS_LINES[CREDITS_LINE_COUNT];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
