#pragma once

#include <cstddef>

#include "DiabloUI/fonts.h"

DEVILUTION_BEGIN_NAMESPACE

std::size_t GetArtStrWidth(const char *str, std::size_t size);
void WordWrapArtStr(char *text, std::size_t width);

DEVILUTION_END_NAMESPACE
