#pragma once

#include "../../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

unsigned GetArtStrWidth(const char* str, unsigned size);
void WordWrapArtStr(char* text, unsigned width, unsigned size);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
