#pragma once

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

unsigned GetArtStrWidth(const char* str, unsigned size);
void WordWrapArtStr(char* text, unsigned width, unsigned size);

DEVILUTION_END_NAMESPACE
