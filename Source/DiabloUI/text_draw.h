#pragma once

#include <SDL.h>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void DrawArtStr(const char* text, const SDL_Rect &rect, int flags, bool drawTextCursor = false);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
