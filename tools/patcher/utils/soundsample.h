#pragma once

#ifndef NOSOUND

#include <SDL_mixer.h>
#include "../../defs.h"

DEVILUTION_BEGIN_NAMESPACE

static_assert(VOLUME_MAX == MIX_MAX_VOLUME, "Set VOLUME_MAX to MIX_MAX_VOLUME for better performance.");

#define SND_DEFAULT_FREQUENCY 22050
#define SND_DEFAULT_FORMAT    AUDIO_S16LSB
#define SND_DEFAULT_CHANNELS  2

static_assert(((VOLUME_MAX - VOLUME_MIN) & (VOLUME_MAX - VOLUME_MIN - 1)) == 0, "Set VOLUME_MAX - VOLUME_MIN to power of 2 for better performance.");
#define ADJUST_VOLUME(raw_volume, min_vol, volume) \
	(((VOLUME_MAX - VOLUME_MIN) * min_vol + (raw_volume - min_vol) * (volume - VOLUME_MIN)) / (VOLUME_MAX - VOLUME_MIN))

#define MIX_VOLUME(volume) ADJUST_VOLUME(MIX_MAX_VOLUME, 0, volume)

DEVILUTION_END_NAMESPACE

#endif // !NOSOUND
