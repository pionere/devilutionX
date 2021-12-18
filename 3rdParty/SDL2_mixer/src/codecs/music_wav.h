/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* This file supports streaming WAV files */
#ifdef FULL
#include "music.h"
#else
#include "../music.h"
#endif
#ifndef FULL // SELF_CONV
#include "../utils.h"
#endif

typedef struct {
    SDL_bool active;
    Uint32 start;
    Uint32 stop;
    Uint32 initial_play_count;
    Uint32 current_play_count;
} WAVLoopPoint;

typedef struct {
    SDL_RWops *src;
    int freesrc;
    SDL_AudioSpec spec;
    int volume;
#ifdef FULL // MUS_LOOP
    int play_count;
#endif
#ifdef FULL // FILE_INT
    Sint64 start;
    Sint64 stop;
#ifdef FULL // MUS_ENC, SEEK
    Sint64 samplesize;
#endif
#else
    int start;
    int stop;
#ifdef FULL // MUS_ENC, SEEK
    int samplesize;
#endif
#endif
#ifdef FULL // SELF_CONV
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
    Uint8 *buffer;
    SDL_AudioStream *stream;
#else
    SDL_AudioCVT cvt;
#endif
#else
    Mix_BuffOps buffer;
#endif // SELF_CONV
#ifdef FULL // WAV_LOOP
    unsigned int numloops;
    WAVLoopPoint *loops;
#endif
#ifdef FULL
    Mix_MusicMetaTags tags;
#endif
#ifdef FULL // WAV_ENC
    Uint16 encoding;
#endif
#ifdef FULL // MUS_ENC
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
    int (*decode)(void *music, int length);
#endif
#endif
} WAV_Music;

extern Mix_MusicInterface Mix_MusicInterface_WAV;

/* vi: set ts=4 sw=4 expandtab: */
