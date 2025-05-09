#pragma once

#include <string>
#include "../../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

const char* GetBasePath();
const char* GetPrefPath();
const std::string* GetBasePathStr();

void InitPaths();
/** Set paths explicitly before calling InitPaths. */
void SetBasePath(const char* path);
void SetPrefPath(const char* path);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
