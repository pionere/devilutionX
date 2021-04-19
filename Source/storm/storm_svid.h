#pragma once

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

void SVidPlayBegin(const char *filename, int flags, HANDLE *video);
bool SVidPlayContinue();
void SVidPlayEnd(HANDLE video);

DEVILUTION_END_NAMESPACE
