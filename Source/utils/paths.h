#pragma once

#include <string>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

const char *GetBasePath();
const char *GetPrefPath();
const char *GetConfigPath();

void SetBasePath(const char *path);
void SetPrefPath(const char *path);
void SetConfigPath(const char *path);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
