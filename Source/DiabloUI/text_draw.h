#pragma once

#include <SDL.h>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

struct TtfSurfaceCache {

	TtfSurfaceCache()
	{
		text = NULL;
		shadow = NULL;
	}

	~TtfSurfaceCache()
	{
		SDL_FreeSurface(text);
		SDL_FreeSurface(shadow);
	}

	SDL_Surface *text;
	SDL_Surface *shadow;
};

void DrawTTF(const char *text, const SDL_Rect &rect, int flags,
    const SDL_Color text_color, const SDL_Color shadow_color,
    TtfSurfaceCache &render_cache);

void DrawArtStr(const char *text, const SDL_Rect &rect, int flags, bool drawTextCursor = false);

DEVILUTION_END_NAMESPACE
