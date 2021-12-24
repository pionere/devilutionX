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
#ifndef MUSIC_H_
#define MUSIC_H_

#include "SDL_mixer.h"
#include "types_internal.h"

/* Supported music APIs, in order of preference */

typedef enum
{
    MIX_MUSIC_CMD,
    MIX_MUSIC_WAVE,
    MIX_MUSIC_MODPLUG,
    MIX_MUSIC_MIKMOD,
    MIX_MUSIC_FLUIDSYNTH,
    MIX_MUSIC_TIMIDITY,
    MIX_MUSIC_NATIVEMIDI,
    MIX_MUSIC_OGG,
    MIX_MUSIC_MPG123,
    MIX_MUSIC_MAD,
    MIX_MUSIC_FLAC,
    MIX_MUSIC_OPUS,
    MIX_MUSIC_LIBXMP,
    MIX_MUSIC_LAST
} Mix_MusicAPI;


/* Supported meta-tags */

typedef enum
{
    MIX_META_TITLE,
    MIX_META_ARTIST,
    MIX_META_ALBUM,
    MIX_META_COPYRIGHT,
    MIX_META_LAST
} Mix_MusicMetaTag;


/* MIXER-X: Meta-tags utility structure */

typedef struct {
    char *tags[4];
} Mix_MusicMetaTags;

#ifdef FULL
extern void meta_tags_init(Mix_MusicMetaTags *tags);
extern void meta_tags_clear(Mix_MusicMetaTags *tags);
extern void meta_tags_set(Mix_MusicMetaTags *tags, Mix_MusicMetaTag type, const char *value);
extern const char* meta_tags_get(Mix_MusicMetaTags *tags, Mix_MusicMetaTag type);
#endif

/* Music API implementation */

#ifdef FULL
extern int get_num_music_interfaces(void);
extern Mix_MusicInterface *get_music_interface(int index);
#endif
#ifdef FULL // WAV_SRC
extern Mix_MusicType detect_music_type(Mix_RWops *src);
extern SDL_bool load_music_type(Mix_MusicType type);
extern SDL_bool open_music_type(Mix_MusicType type);
#endif
#ifdef FULL
extern SDL_bool has_music(Mix_MusicType type);
#endif
extern void open_music(const SDL_AudioSpec *spec);
#ifdef FULL // FIX_MUS, SOME_VOL
extern int music_pcm_getaudio(void *context, void *data, int bytes, int volume,
                              int (*GetSome)(void *context, void *data, int bytes, SDL_bool *done));
#else
extern int music_pcm_getaudio(Mix_Audio* audio, void* data, int bytes,
                              int (*GetSome)(Mix_Audio* audio, Mix_BuffOps* buffer, void* data, int bytes, int volume));
#endif
extern void SDLCALL music_mixer(void *udata, Uint8 *stream, int len);
extern void close_music(void);
#ifdef FULL // WAV_SRC
extern void unload_music(void);
#endif

#ifdef FULL
extern char *music_cmd;
extern SDL_AudioSpec music_spec;
#endif
SDL_bool Mix_LoadAudio_RW(Mix_RWops* src, Mix_Audio* dst);
void Mix_UnloadAudio(Mix_Audio* audio);

#endif /* MUSIC_H_ */

/* vi: set ts=4 sw=4 expandtab: */
