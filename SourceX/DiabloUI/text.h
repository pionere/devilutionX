#pragma once

#include <cstddef>

#include "DiabloUI/fonts.h"

DEVILUTION_BEGIN_NAMESPACE

unsigned GetArtStrWidth(const char *str, unsigned size);
void WordWrapArtStr(char *text, unsigned width);

DEVILUTION_END_NAMESPACE
