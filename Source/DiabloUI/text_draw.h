#pragma once

#include <SDL.h>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

void DrawArtStr(const char* text, const SDL_Rect &rect, int flags, bool drawTextCursor = false);

DEVILUTION_END_NAMESPACE
