#pragma once

#include <string>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

const std::string &GetBasePath();
const std::string &GetPrefPath();
const std::string &GetConfigPath();

void SetBasePath(const char *path);
void SetPrefPath(const char *path);
void SetConfigPath(const char *path);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
