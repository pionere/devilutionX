#pragma once

#include <cstddef>

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#endif

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

// Re-maps a keyboard key as per the REMAP_KEYBOARD_KEYS define.
inline void remap_keyboard_key(SDL_Keycode* sym)
{
#ifdef REMAP_KEYBOARD_KEYS

	struct Mapping {
		SDL_Keycode from;
		SDL_Keycode to;
	};
	constexpr Mapping kMappings[] = { REMAP_KEYBOARD_KEYS };
	for (unsigned i = 0; i < sizeof(kMappings) / sizeof(kMappings[0]); ++i) {
		if (*sym == kMappings[i].from) {
			*sym = kMappings[i].to;
			return;
		}
	}
#endif
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
