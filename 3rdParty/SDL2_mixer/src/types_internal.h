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

#ifndef INCLUDE_TYPES_INTERNAL_H_
#define INCLUDE_TYPES_INTERNAL_H_

#include "SDL_mixer.h"

#if !SDL_VERSION_ATLEAST(2, 0, 0)
/**
 * \brief A signed 8-bit integer type.
 */
#define SDL_MAX_SINT8   ((Sint8)0x7F)           /* 127 */
#define SDL_MIN_SINT8   ((Sint8)(~0x7F))        /* -128 */
typedef int8_t Sint8;
/**
 * \brief An unsigned 8-bit integer type.
 */
#define SDL_MAX_UINT8   ((Uint8)0xFF)           /* 255 */
#define SDL_MIN_UINT8   ((Uint8)0x00)           /* 0 */
typedef uint8_t Uint8;
/**
 * \brief A signed 16-bit integer type.
 */
#define SDL_MAX_SINT16  ((Sint16)0x7FFF)        /* 32767 */
#define SDL_MIN_SINT16  ((Sint16)(~0x7FFF))     /* -32768 */
typedef int16_t Sint16;
/**
 * \brief An unsigned 16-bit integer type.
 */
#define SDL_MAX_UINT16  ((Uint16)0xFFFF)        /* 65535 */
#define SDL_MIN_UINT16  ((Uint16)0x0000)        /* 0 */
typedef uint16_t Uint16;
/**
 * \brief A signed 32-bit integer type.
 */
#define SDL_MAX_SINT32  ((Sint32)0x7FFFFFFF)    /* 2147483647 */
#define SDL_MIN_SINT32  ((Sint32)(~0x7FFFFFFF)) /* -2147483648 */
typedef int32_t Sint32;
/**
 * \brief An unsigned 32-bit integer type.
 */
#define SDL_MAX_UINT32  ((Uint32)0xFFFFFFFFu)   /* 4294967295 */
#define SDL_MIN_UINT32  ((Uint32)0x00000000)    /* 0 */
typedef uint32_t Uint32;
/**
 * \brief A signed 64-bit integer type.
 */
#define SDL_MAX_SINT64  ((Sint64)0x7FFFFFFFFFFFFFFFll)      /* 9223372036854775807 */
#define SDL_MIN_SINT64  ((Sint64)(~0x7FFFFFFFFFFFFFFFll))   /* -9223372036854775808 */
typedef int64_t Sint64;
/**
 * \brief An unsigned 64-bit integer type.
 */
#define SDL_MAX_UINT64  ((Uint64)0xFFFFFFFFFFFFFFFFull)     /* 18446744073709551615 */
#define SDL_MIN_UINT64  ((Uint64)(0x0000000000000000ull))   /* 0 */
typedef uint64_t Uint64;
#endif // !SDL_VERSION_ATLEAST(2, 0, 0)
#if !SDL_VERSION_ATLEAST(2, 0, 7)
/**
 * \brief A signed 8-bit integer type.
 */
#define SDL_MAX_SINT8   ((Sint8)0x7F)           /* 127 */
#define SDL_MIN_SINT8   ((Sint8)(~0x7F))        /* -128 */
/**
 * \brief An unsigned 8-bit integer type.
 */
#define SDL_MAX_UINT8   ((Uint8)0xFF)           /* 255 */
#define SDL_MIN_UINT8   ((Uint8)0x00)           /* 0 */
/**
 * \brief A signed 16-bit integer type.
 */
#define SDL_MAX_SINT16  ((Sint16)0x7FFF)        /* 32767 */
#define SDL_MIN_SINT16  ((Sint16)(~0x7FFF))     /* -32768 */
/**
 * \brief An unsigned 16-bit integer type.
 */
#define SDL_MAX_UINT16  ((Uint16)0xFFFF)        /* 65535 */
#define SDL_MIN_UINT16  ((Uint16)0x0000)        /* 0 */
/**
 * \brief A signed 32-bit integer type.
 */
#define SDL_MAX_SINT32  ((Sint32)0x7FFFFFFF)    /* 2147483647 */
#define SDL_MIN_SINT32  ((Sint32)(~0x7FFFFFFF)) /* -2147483648 */
 /**
 * \brief An unsigned 32-bit integer type.
 */
#define SDL_MAX_UINT32  ((Uint32)0xFFFFFFFFu)   /* 4294967295 */
#define SDL_MIN_UINT32  ((Uint32)0x00000000)    /* 0 */
/**
 * \brief A signed 64-bit integer type.
 */
#define SDL_MAX_SINT64  ((Sint64)0x7FFFFFFFFFFFFFFFll)      /* 9223372036854775807 */
#define SDL_MIN_SINT64  ((Sint64)(~0x7FFFFFFFFFFFFFFFll))   /* -9223372036854775808 */
/**
 * \brief An unsigned 64-bit integer type.
 */
#define SDL_MAX_UINT64  ((Uint64)0xFFFFFFFFFFFFFFFFull)     /* 18446744073709551615 */
#define SDL_MIN_UINT64  ((Uint64)(0x0000000000000000ull))   /* 0 */
#endif // !SDL_VERSION_ATLEAST(2, 0, 7)

/* Utils */

typedef struct Mix_BuffOps {
    void* basePos;
    void* currPos;
    void* endPos;
} Mix_BuffOps;

/* Music */

typedef struct _Mix_Audio Mix_Audio;

typedef struct
{
#ifdef FULL // WAV_SRC
    const char *tag;
    Mix_MusicAPI api;
    Mix_MusicType type;
    SDL_bool loaded;
    SDL_bool opened;

    /* Load the library */
    int (*Load)(void);

    /* Initialize for the audio output */
    int (*Open)(const SDL_AudioSpec *spec);
#endif
    /* Create a music object from an SDL_RWops stream
     * If the function returns NULL, 'src' will be freed if needed by the caller.
     */
#ifdef FULL // FIX_MUS, FREE_SRC
    void *(*CreateFromRW)(SDL_RWops *src, int freesrc);
#else
    void *(*CreateFromRW)(SDL_RWops* src, Mix_Audio* dst, Uint8* buffer);
#endif
#ifdef FULL // WAV_SRC
    /* Create a music object from a file, if SDL_RWops are not supported */
    void *(*CreateFromFile)(const char *file);
#endif
#ifdef FULL // FIX_MUS
    /* Set the volume */
    void (*SetVolume)(void *music, int volume);
#endif
#ifdef FULL
    /* Get the volume */
    int (*GetVolume)(void *music);
#endif
    /* Start playing music from the beginning with an optional loop count */
    int (*Play)(Mix_Audio* audio, int play_count);
#ifdef FULL
    /* Returns SDL_TRUE if music is still playing */
    SDL_bool (*IsPlaying)(void *music);
#endif
    /* Get music data, returns the number of bytes left */
    int (*GetAudio)(Mix_Audio* audio, void *data, int bytes);
#ifdef FULL
    /* Jump to a given order in mod music */
    int (*Jump)(void *music, int order);

    /* Seek to a play position (in seconds) */
    int (*Seek)(void *music, double position);

    /* Tell a play position (in seconds) */
    double (*Tell)(void *music);

    /* Get Music duration (in seconds) */
    double (*Duration)(void *music);

    /* Tell a loop start position (in seconds) */
    double (*LoopStart)(void *music);

    /* Tell a loop end position (in seconds) */
    double (*LoopEnd)(void *music);

    /* Tell a loop length position (in seconds) */
    double (*LoopLength)(void *music);

    /* Get a meta-tag string if available */
    const char* (*GetMetaTag)(void *music, Mix_MusicMetaTag tag_type);

    /* Pause playing music */
    void (*Pause)(void *music);

    /* Resume playing music */
    void (*Resume)(void *music);

    /* Stop playing music */
    void (*Stop)(void *music);
#endif
    /* Delete a music object */
    void (*Delete)(Mix_Audio* audio);
#ifdef FULL // WAV_SRC
    /* Close the library and clean up */
    void (*Close)(void);

    /* Unload the library */
    void (*Unload)(void);
#endif
} Mix_MusicInterface;

/* Music-WAV */

typedef struct {
    SDL_bool active;
    Uint32 start;
    Uint32 stop;
    Uint32 initial_play_count;
    Uint32 current_play_count;
} WAVLoopPoint;

typedef struct {
    Uint32 sampleSize;          /**< Data size if MIX_STREAM_SAMPLE_COUNT samples are read. */
    SDL_AudioFormat format;     /**< Audio data format */
    Uint8 channels;             /**< Number of channels: 1 mono, 2 stereo */
    Uint8 freqMpl;              /**< Frequency multiplier of MIX_DEFAULT_FREQUENCY */
} Mix_AudioSpec;

typedef struct {
    SDL_RWops *src;
    int freesrc;
#ifdef FULL // SELF_CONV
    SDL_AudioSpec spec;
#else
    Mix_AudioSpec spec;
#endif
#ifdef FULL // FIX_MUS
    int volume;
#endif
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
    int (*decode)(struct WAV_Music* wave, int length);
#endif
#endif
} WAV_Music;

/* Mixer */

#ifdef FULL // FIX_MUS
struct _Mix_Music {
#ifdef FULL // WAV_SRC
    Mix_MusicInterface *interface;
#endif
    void *context;

    SDL_bool playing;
#ifdef FULL // FADING
    Mix_Fading fading;
    int fade_step;
    int fade_steps;
    char filename[1024];
#endif
};
#else
typedef struct _Mix_Audio {
    union {
        WAV_Music asWAV;
    };
} _Mix_Audio;
struct _Mix_Music {
    int volume;
    SDL_bool playing;
    Mix_Audio audio;
};
#endif // FULL - FIX_MUS

#endif /* INCLUDE_TYPES_INTERNAL_H_ */

/* vi: set ts=4 sw=4 expandtab: */
