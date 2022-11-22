#pragma once

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

HANDLE SVidPlayBegin(const char* filename, int flags);
bool SVidPlayContinue();
void SVidPlayEnd();

DEVILUTION_END_NAMESPACE
