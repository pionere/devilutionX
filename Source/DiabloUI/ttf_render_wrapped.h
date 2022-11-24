#pragma once

#include "../types.h"
#include <SDL_ttf.h>

DEVILUTION_BEGIN_NAMESPACE

/**
 * Renders UTF-8, wrapping lines to avoid exceeding wrapLength, and aligning
 * according to the `x_align` argument.
 *
 * This method is slow. Caching the result is recommended.
 */
SDL_Surface* RenderUTF8_Solid_Wrapped(TTF_Font* font, const char* text, SDL_Color fg, Uint32 wrapLength, const int x_align);

DEVILUTION_END_NAMESPACE
