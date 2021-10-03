#pragma once

#include <SDL.h>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

struct Art {
	SDL_Surface *surface;
	int frames;
	int logical_width;
	int frame_height; // logical frame height (before scaling)
	unsigned int palette_version;

	void Unload()
	{
		SDL_FreeSurface(surface);
		surface = NULL;
	}
};

bool LoadArt(const char* pszFile, Art* art, int frames = 1, SDL_Color* pPalette = NULL);
void LoadMaskedArt(const char* pszFile, Art* art, int frames = 1, int mask = 250);
void LoadArt(Art* art, const BYTE* artData, int w, int h, int frames = 1);

DEVILUTION_END_NAMESPACE
