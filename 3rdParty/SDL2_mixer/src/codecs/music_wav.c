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

#ifdef MUSIC_WAV

/* This file supports streaming WAV files */

#include "music_wav.h"
#define MIX_INTERNAL_EFFECT__
#include "../effects_internal.h"

/*
    Taken with permission from SDL_wave.h, part of the SDL library,
    available at: http://www.libsdl.org/
    and placed under the same license as this mixer library.
*/

/* WAVE files are little-endian */

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF        0x46464952      /* "RIFF" */
#define WAVE        0x45564157      /* "WAVE" */
#define FMT         0x20746D66      /* "fmt " */
#define DATA        0x61746164      /* "data" */
#define SMPL        0x6c706d73      /* "smpl" */
#define LIST        0x5453494c      /* "LIST" */
#define ID3_        0x20336469      /* "id3 " */
#define PCM_CODE    1               /* WAVE_FORMAT_PCM */
#define ADPCM_CODE  2               /* WAVE_FORMAT_ADPCM */
#define FLOAT_CODE  3               /* WAVE_FORMAT_IEEE_FLOAT */
#define ALAW_CODE   6               /* WAVE_FORMAT_ALAW */
#define uLAW_CODE   7               /* WAVE_FORMAT_MULAW */
#define EXT_CODE    0xFFFE          /* WAVE_FORMAT_EXTENSIBLE */
#define WAVE_MONO   1
#define WAVE_STEREO 2

#pragma pack(push, 1)
typedef struct {
/* Not saved in the chunk we read:
    Uint32  chunkID;
    Uint32  chunkLen;
*/
    Uint16  encoding;
    Uint16  channels;       /* 1 = mono, 2 = stereo */
    Uint32  frequency;      /* One of 11025, 22050, or 44100 Hz */
    Uint32  byterate;       /* Average bytes per second */
    Uint16  blockalign;     /* Bytes per sample block */
    Uint16  bitspersample;      /* One of 8, 12, 16, or 4 for ADPCM */
} WaveFMT;

typedef struct {
    WaveFMT format;
    Uint16  cbSize;
    union {
        Uint16 validbitspersample; /* bits of precision */
        Uint16 samplesperblock;   /* valid if wBitsPerSample==0 */
        Uint16 reserved;         /* If neither applies, set to zero. */
    } Samples;
    Uint32 channelsmask;
    /* GUID subFormat 16 bytes */
    Uint32 subencoding;
    Uint16 sub_data2;
    Uint16 sub_data3;
#ifdef FULL // MUS_ENC
    Uint8  sub_data[8];
#endif
} WaveFMTEx;

#pragma pack(pop)

typedef struct {
    Uint32 identifier;
    Uint32 type;
    Uint32 start;
    Uint32 end;
    Uint32 fraction;
    Uint32 play_count;
} SampleLoop;

typedef struct {
/* Not saved in the chunk we read:
    Uint32  chunkID;
    Uint32  chunkLen;
*/
    Uint32  manufacturer;
    Uint32  product;
    Uint32  sample_period;
    Uint32  MIDI_unity_note;
    Uint32  MIDI_pitch_fraction;
    Uint32  SMTPE_format;
    Uint32  SMTPE_offset;
    Uint32  sample_loops;
    Uint32  sampler_data;
    SampleLoop loops[1];
} SamplerChunk;

/*********************************************/
/* Define values for AIFF (IFF audio) format */
/*********************************************/
#define FORM        0x4d524f46      /* "FORM" */
#define AIFF        0x46464941      /* "AIFF" */
#define AIFC        0x43464941      /* "AIFС" */
#define FVER        0x52455646      /* "FVER" */
#define SSND        0x444e5353      /* "SSND" */
#define COMM        0x4d4d4f43      /* "COMM" */
#define AIFF_ID3_   0x20334449      /* "ID3 " */
#define MARK        0x4B52414D      /* "MARK" */
#define INST        0x54534E49      /* "INST" */
#define AUTH        0x48545541      /* "AUTH" */
#define NAME        0x454D414E      /* "NAME" */
#define _c__        0x20296328      /* "(c) " */

/* Supported compression types */
#define NONE        0x454E4F4E      /* "NONE" */
#define sowt        0x74776F73      /* "sowt" */
#define raw_        0x20776172      /* "raw " */
#define ulaw        0x77616C75      /* "ulaw" */
#define alaw        0x77616C61      /* "alaw" */
#define ULAW        0x57414C55      /* "ULAW" */
#define ALAW        0x57414C41      /* "ALAW" */
#define fl32        0x32336C66      /* "fl32" */
#define fl64        0x34366C66      /* "fl64" */
#define FL32        0x32334C46      /* "FL32" */

/* Function to load the WAV/AIFF stream */
static SDL_bool LoadWAVMusic(WAV_Music *wave);
#ifdef FULL // WAV_SRC
static SDL_bool LoadAIFFMusic(WAV_Music *wave);
#endif
static void WAV_Delete(Mix_Audio* audio);
#ifdef FULL // MUS_ENC
static int fetch_pcm(WAV_Music* wave, int length);
#endif
/* Load a WAV stream from the given RWops object */
#ifdef FULL // FIX_MUS, FREE_SRC
static void *WAV_CreateFromRW(Mix_RWops *src, int freesrc)
#else
static void* WAV_CreateFromRW(Mix_RWops* src, Mix_Audio* dst)
#endif
{
    WAV_Music *wave;
    Uint32 magic;
    SDL_bool loaded = SDL_FALSE;
#ifdef FULL // FIX_MUS
    wave = (WAV_Music *)SDL_calloc(1, sizeof(*wave));
    if (wave == NULL) {
        Mix_OutOfMemory();
        return NULL;
    }
#else
    wave = &dst->asWAV;
#endif
#ifdef FULL // SRC_PTR
    wave->src = src;
#else
    Mix_RWFromMem(&wave->src, src->basePos, (size_t)src->endPos - (size_t)src->basePos);
    src = &wave->src;
#endif // SRC_PTR
#ifdef FULL // FIX_MUS
    wave->volume = MIX_MAX_VOLUME;
#endif
#ifdef FULL // MUS_ENC
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
    /* Default decoder is PCM */
    wave->decode = fetch_pcm;
#endif
#endif
#ifdef FULL // WAV_ENC
    wave->encoding = PCM_CODE;
#endif

    magic = Mix_ReadLE32(src);
    if (magic == RIFF || magic == WAVE) {
        loaded = LoadWAVMusic(wave);
#ifdef FULL // WAV_SRC
    } else if (magic == FORM) {
        loaded = LoadAIFFMusic(wave);
#endif
    } else {
        Mix_SetError("Unknown WAVE data format");
    }
    if (!loaded) {
        WAV_Delete(dst);
        return NULL;
    }
#ifdef FULL // SELF_CONV
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
    wave->buffer = (Uint8*)SDL_malloc(wave->spec.size);
    if (wave->buffer == NULL) {
        Mix_OutOfMemory();
        WAV_Delete(wave);
        return NULL;
    }
    wave->stream = SDL_NewAudioStream(
        wave->spec.format, wave->spec.channels, wave->spec.freq,
#ifdef FULL // FIX_OUT
        music_spec.format, music_spec.channels, music_spec.freq);
#else
        MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, MIX_DEFAULT_FREQUENCY);
#endif
    if (wave->stream == NULL) {
        WAV_Delete(wave);
        return NULL;
    }
#else
    SDL_BuildAudioCVT(&wave->cvt,
            wave->spec.format, wave->spec.channels, wave->spec.freq,
#ifdef FULL // FIX_OUT
            music_spec.format, music_spec.channels, music_spec.freq);
#else
        MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, MIX_DEFAULT_FREQUENCY);
#endif
#endif // SDL_VERSION_ATLEAST(2, 0, 7)
#endif // FULL - SELF_CONV
#ifdef FULL // SRC_PTR
#ifdef FULL // FREE_SRC
    wave->freesrc = freesrc;
#else
    wave->freesrc = SDL_TRUE;
#endif
#endif // SRC_PTR
    return wave;
}
#ifdef FULL // FIX_MUS
static void WAV_SetVolume(void *context, int volume)
{
    WAV_Music *wave = (WAV_Music *)context;
    wave->volume = volume;
}
#ifdef FULL
static int WAV_GetVolume(void *context)
{
    WAV_Music *wave = (WAV_Music *)context;
    return wave->volume;
}
#endif
#endif // FULL - FIX_MUS
/* Start playback of a given WAV stream */
#ifdef FULL // MEM_OPS
static int WAV_Play(Mix_Audio* audio, int loop_count)
{
    WAV_Music* wave = &audio->asWAV;
#ifdef FULL // WAV_LOOP
    unsigned int i;
    for (i = 0; i < wave->numloops; ++i) {
        WAVLoopPoint *loop = &wave->loops[i];
        loop->active = SDL_TRUE;
        loop->current_play_count = loop->initial_play_count;
    }
#endif
    wave->loop_count = loop_count;
    if (Mix_RWseek(wave->src, wave->start, RW_SEEK_SET) < 0) {
        return -1;
    }
    return 0;
}
#else // MEM_OPS
static int WAV_Play(Mix_Channel* channel, int loop_count)
{
#ifdef FULL // WAV_LOOP
    WAV_Music* wave = &channel->chunk->asWAV;

    unsigned int i;
    for (i = 0; i < wave->numloops; ++i) {
        WAVLoopPoint *loop = &wave->loops[i];
        loop->active = SDL_TRUE;
        loop->current_play_count = loop->initial_play_count;
    }
    channel->loop_count = loop_count;
#endif
    if (Mix_RWseek(&channel->playOps, 0, RW_SEEK_SET) < 0) {
        return -1;
    }
    return 0;
}
#endif // FULL - MEM_OPS

#ifdef FULL // SELF_CONV
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
static int fetch_pcm(WAV_Music* wave, int length)
{
    return (int)Mix_RWread(wave->src, wave->buffer, 1, (size_t)length);
}
#ifdef FULL // WAV_SRC
static Uint32 PCM_S24_to_S32_BE(Uint8 *x) {
    const Uint32 bits = 24;
    Uint32 in = (((Uint32)x[0] << 0)  & 0x0000FF) |
                (((Uint32)x[1] << 8)  & 0x00FF00) |
                (((Uint32)x[2] << 16) & 0xFF0000);
    Uint32 m = 1u << (bits - 1);
    return (in ^ m) - m;
}

static Uint32 PCM_S24_to_S32_LE(Uint8 *x) {
    const Uint32 bits = 24;
    Uint32 in = (((Uint32)x[2] << 0)  & 0x0000FF) |
                (((Uint32)x[1] << 8)  & 0x00FF00) |
                (((Uint32)x[0] << 16) & 0xFF0000);
    Uint32 m = 1u << (bits - 1);
    return (in ^ m) - m;
}

static int fetch_pcm24be(WAV_Music* wave, int length)
{
    int i = 0, o = 0;
    length = (int)Mix_RWread(wave->src, wave->buffer, 1, (size_t)((length / 4) * 3));
    if (length % wave->samplesize != 0) {
        length -= length % wave->samplesize;
    }
    for (i = length - 3, o = ((length - 3) / 3) * 4; i >= 0; i -= 3, o -= 4) {
        Uint32 decoded = PCM_S24_to_S32_BE(wave->buffer + i);
        wave->buffer[o + 0] = (decoded >> 0) & 0xFF;
        wave->buffer[o + 1] = (decoded >> 8) & 0xFF;
        wave->buffer[o + 2] = (decoded >> 16) & 0xFF;
        wave->buffer[o + 3] = (decoded >> 24) & 0xFF;
    }
    return (length / 3) * 4;
}

static int fetch_pcm24le(WAV_Music* wave, int length)
{
    int i = 0, o = 0;
    length = (int)Mix_RWread(wave->src, wave->buffer, 1, (size_t)((length / 4) * 3));
    if (length % wave->samplesize != 0) {
        length -= length % wave->samplesize;
    }
    for (i = length - 3, o = ((length - 3) / 3) * 4; i >= 0; i -= 3, o -= 4) {
        Uint32 decoded = PCM_S24_to_S32_LE(wave->buffer + i);
        wave->buffer[o + 3] = (decoded >> 0) & 0xFF;
        wave->buffer[o + 2] = (decoded >> 8) & 0xFF;
        wave->buffer[o + 1] = (decoded >> 16) & 0xFF;
        wave->buffer[o + 0] = (decoded >> 24) & 0xFF;
    }
    return (length / 3) * 4;
}

SDL_FORCE_INLINE double
Mix_SwapDouble(double x)
{
    union
    {
        double f;
        Uint64 ui64;
    } swapper;
    swapper.f = x;
    swapper.ui64 = SDL_Swap64(swapper.ui64);
    return swapper.f;
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define Mix_SwapDoubleLE(X)  (X)
#define Mix_SwapDoubleBE(X)  Mix_SwapDouble(X)
#else
#define Mix_SwapDoubleLE(X)  Mix_SwapDouble(X)
#define Mix_SwapDoubleBE(X)  (X)
#endif

static int fetch_float64be(WAV_Music* wave, int length)
{
    int i = 0, o = 0;
    length = (int)Mix_RWread(wave->src, wave->buffer, 1, (size_t)(length));
    if (length % wave->samplesize != 0) {
        length -= length % wave->samplesize;
    }
    for (i = 0, o = 0; i <= length; i += 8, o += 4) {
        union
        {
            float f;
            Uint32 ui32;
        } sample;
        sample.f = (float)Mix_SwapDoubleBE(*(double*)(wave->buffer + i));
        wave->buffer[o + 0] = (sample.ui32 >> 0) & 0xFF;
        wave->buffer[o + 1] = (sample.ui32 >> 8) & 0xFF;
        wave->buffer[o + 2] = (sample.ui32 >> 16) & 0xFF;
        wave->buffer[o + 3] = (sample.ui32 >> 24) & 0xFF;
    }
    return length / 2;
}

static int fetch_float64le(WAV_Music* wave, int length)
{
    int i = 0, o = 0;
    length = (int)Mix_RWread(wave->src, wave->buffer, 1, (size_t)(length));
    if (length % wave->samplesize != 0) {
        length -= length % wave->samplesize;
    }
    for (i = 0, o = 0; i <= length; i += 8, o += 4) {
        union
        {
            float f;
            Uint32 ui32;
        } sample;
        sample.f = (float)Mix_SwapDoubleLE(*(double*)(wave->buffer + i));
        wave->buffer[o + 0] = (sample.ui32 >> 0) & 0xFF;
        wave->buffer[o + 1] = (sample.ui32 >> 8) & 0xFF;
        wave->buffer[o + 2] = (sample.ui32 >> 16) & 0xFF;
        wave->buffer[o + 3] = (sample.ui32 >> 24) & 0xFF;
    }
    return length / 2;
}
#endif // FULL
#ifdef FULL // MUS_ENC
/*
    G711 decode tables taken from SDL2 (src/audio/SDL_wave.c)
*/
#ifdef SDL_WAVE_LAW_LUT
static const Sint16 alaw_lut[256] = {
    -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736, -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784, -2752,
    -2624, -3008, -2880, -2240, -2112, -2496, -2368, -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392, -22016,
    -20992, -24064, -23040, -17920, -16896, -19968, -18944, -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136, -11008,
    -10496, -12032, -11520, -8960, -8448, -9984, -9472, -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568, -344,
    -328, -376, -360, -280, -264, -312, -296, -472, -456, -504, -488, -408, -392, -440, -424, -88,
    -72, -120, -104, -24, -8, -56, -40, -216, -200, -248, -232, -152, -136, -184, -168, -1376,
    -1312, -1504, -1440, -1120, -1056, -1248, -1184, -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696, -688,
    -656, -752, -720, -560, -528, -624, -592, -944, -912, -1008, -976, -816, -784, -880, -848, 5504,
    5248, 6016, 5760, 4480, 4224, 4992, 4736, 7552, 7296, 8064, 7808, 6528, 6272, 7040, 6784, 2752,
    2624, 3008, 2880, 2240, 2112, 2496, 2368, 3776, 3648, 4032, 3904, 3264, 3136, 3520, 3392, 22016,
    20992, 24064, 23040, 17920, 16896, 19968, 18944, 30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136, 11008,
    10496, 12032, 11520, 8960, 8448, 9984, 9472, 15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568, 344,
    328, 376, 360, 280, 264, 312, 296, 472, 456, 504, 488, 408, 392, 440, 424, 88,
    72, 120, 104, 24, 8, 56, 40, 216, 200, 248, 232, 152, 136, 184, 168, 1376,
    1312, 1504, 1440, 1120, 1056, 1248, 1184, 1888, 1824, 2016, 1952, 1632, 1568, 1760, 1696, 688,
    656, 752, 720, 560, 528, 624, 592, 944, 912, 1008, 976, 816, 784, 880, 848
};

static const Sint16 mulaw_lut[256] = {
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956, -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764, -15996,
    -15484, -14972, -14460, -13948, -13436, -12924, -12412, -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316, -7932,
    -7676, -7420, -7164, -6908, -6652, -6396, -6140, -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092, -3900,
    -3772, -3644, -3516, -3388, -3260, -3132, -3004, -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980, -1884,
    -1820, -1756, -1692, -1628, -1564, -1500, -1436, -1372, -1308, -1244, -1180, -1116, -1052, -988, -924, -876,
    -844, -812, -780, -748, -716, -684, -652, -620, -588, -556, -524, -492, -460, -428, -396, -372,
    -356, -340, -324, -308, -292, -276, -260, -244, -228, -212, -196, -180, -164, -148, -132, -120,
    -112, -104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 0, 32124,
    31100, 30076, 29052, 28028, 27004, 25980, 24956, 23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764, 15996,
    15484, 14972, 14460, 13948, 13436, 12924, 12412, 11900, 11388, 10876, 10364, 9852, 9340, 8828, 8316, 7932,
    7676, 7420, 7164, 6908, 6652, 6396, 6140, 5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092, 3900,
    3772, 3644, 3516, 3388, 3260, 3132, 3004, 2876, 2748, 2620, 2492, 2364, 2236, 2108, 1980, 1884,
    1820, 1756, 1692, 1628, 1564, 1500, 1436, 1372, 1308, 1244, 1180, 1116, 1052, 988, 924, 876,
    844, 812, 780, 748, 716, 684, 652, 620, 588, 556, 524, 492, 460, 428, 396, 372,
    356, 340, 324, 308, 292, 276, 260, 244, 228, 212, 196, 180, 164, 148, 132, 120,
    112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0
};
#endif

static Sint16 uLAW_To_PCM16(Uint8 u_val)
{
#ifdef SDL_WAVE_LAW_LUT
    return mulaw_lut[u_val];
#else
    Uint8 nibble = ~u_val;
    Sint16 mantissa = nibble & 0xf;
    Uint8 exponent = (nibble >> 4) & 0x7;
    Sint16 step = (Sint16)(4 << (exponent + 1));

    mantissa = (Sint16)(0x80 << exponent) + step * mantissa + step / 2 - 132;

    return nibble & 0x80 ? -mantissa : mantissa;
#endif
}

static Sint16 ALAW_To_PCM16(Uint8 a_val)
{
#ifdef SDL_WAVE_LAW_LUT
    return alaw_lut[a_val];
#else
    Uint8 nibble = a_val;
    Uint8 exponent = (nibble & 0x7f) ^ 0x55;
    Sint16 mantissa = exponent & 0xf;

    exponent >>= 4;
    if (exponent > 0) {
        mantissa |= 0x10;
    }
    mantissa = (Sint16)(mantissa << 4) | 0x8;
    if (exponent > 1) {
        mantissa <<= exponent - 1;
    }

    return nibble & 0x80 ? mantissa : -mantissa;
#endif
}

static int fetch_xlaw(Sint16 (*decode_sample)(Uint8), WAV_Music* wave, int length)
{
    int i = 0, o = 0;
    length = (int)Mix_RWread(wave->src, wave->buffer, 1, (size_t)(length / 2));
    if (length % wave->samplesize != 0) {
        length -= length % wave->samplesize;
    }
    for (i = length - 1, o = (length - 1) * 2; i >= 0; i--, o -= 2) {
        Uint16 decoded = (Uint16)decode_sample(wave->buffer[i]);
        wave->buffer[o] = decoded & 0xFF;
        wave->buffer[o + 1] = (decoded >> 8) & 0xFF;
    }
    return length * 2;
}

static int fetch_ulaw(WAV_Music* wave, int length)
{
    return fetch_xlaw(uLAW_To_PCM16, wave, length);
}

static int fetch_alaw(WAV_Music* wave, int length)
{
    return fetch_xlaw(ALAW_To_PCM16, wave, length);
}
#endif // FULL - MUS_ENC
#endif // SDL_VERSION_ATLEAST(2, 0, 7) - USE_SDL1
#else // FULL - SELF_CONV
#ifdef FULL // MEM_OPS
static int fetch_pcm(WAV_Music* wave, Mix_BuffOps* buffOps, int length)
{
    int result = Mix_RWread(wave->src, buffOps->basePos, 1, (size_t)length);
    buffOps->endPos = (Uint8*)buffOps->basePos + result;
    return result;
}
#else
static unsigned fetch_pcm(Mix_RWops* src, Mix_BuffOps* buffOps, int length)
{
    unsigned result = (unsigned)Mix_RWread(src, buffOps->basePos, (size_t)length);
    buffOps->endPos = (Uint8*)buffOps->basePos + result;
    return result;
}
#endif // MEM_OPS
#endif // FULL - SELF_CONV

/* Play some of a stream previously started with WAV_Play() */
#ifdef FULL // FIX_MUS, SOME_VOL
static int WAV_GetSome(void *context, void *data, int bytes, SDL_bool *done)
#else
static int WAV_GetSome(Mix_Channel* channel, void* stream, int bytes)
#endif
{
#ifdef FULL // SELF_CONV
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
    WAV_Music *wave = (WAV_Music *)context;
#ifdef FULL // FILE_INT
    Sint64 pos, stop;
#ifdef FULL // WAV_LOOP
    WAVLoopPoint *loop;
    Sint64 loop_start; // = wave->start;
    Sint64 loop_stop; // = wave->stop;
#endif
#else
    int pos, stop;
#ifdef FULL // WAV_LOOP
    WAVLoopPoint *loop;
    int loop_start; // = wave->start;
    int loop_stop; // = wave->stop;
#endif
#endif
#ifdef FULL // WAV_LOOP
    SDL_bool looped = SDL_FALSE;
#endif
    SDL_bool at_end = SDL_FALSE;
#ifdef FULL // WAV_LOOP
    unsigned i;
#endif
    int filled, amount, result;

    filled = SDL_AudioStreamGet(wave->stream, data, bytes);
    if (filled != 0) {
        return filled;
    }
    pos = Mix_RWtell(wave->src);
    stop = wave->stop;
#ifdef FULL // WAV_LOOP
    loop = NULL;
    for (i = 0; i < wave->numloops; ++i) {
        loop = &wave->loops[i];
        if (loop->active) {
            const int bytes_per_sample = (SDL_AUDIO_BITSIZE(wave->spec.format) / 8) * wave->spec.channels;
            loop_start = wave->start + loop->start * (Uint32)bytes_per_sample;
            loop_stop = wave->start + (loop->stop + 1) * (Uint32)bytes_per_sample;
            if (pos >= loop_start && pos < loop_stop) {
                stop = loop_stop;
                break;
            }
        }
        loop = NULL;
    }
#endif
    amount = (int)wave->spec.size;
    if ((stop - pos) < amount) {
        amount = (int)(stop - pos);
    }
#ifdef FULL // MUS_ENC
    amount = wave->decode(wave, amount);
#else
    amount = fetch_pcm(wave, amount);
#endif
    if (amount > 0) {
        result = SDL_AudioStreamPut(wave->stream, wave->buffer, amount);
        if (result < 0) {
            return -1;
        }
    } else {
        /* We might be looping, continue */
        at_end = SDL_TRUE;
    }
#ifdef FULL // WAV_LOOP
    if (loop && Mix_RWtell(wave->src) >= stop) {
        if (loop->current_play_count == 1) {
            loop->active = SDL_FALSE;
        } else {
            if (loop->current_play_count > 0) {
                --loop->current_play_count;
            }
            if (Mix_RWseek(wave->src, loop_start, RW_SEEK_SET) < 0)
                return -1;
            looped = SDL_TRUE;
        }
    }

    if (!looped && (at_end || Mix_RWtell(wave->src) >= wave->stop)) {
#else
    if (at_end || Mix_RWtell(wave->src) >= wave->stop) {
#endif
        int loop_count = wave->loop_count;
        if (loop_count == 0) {
            if (amount == 0) {
                *done = SDL_TRUE;
                SDL_AudioStreamFlush(wave->stream);
            }
        } else {
            if (loop_count > 0) {
                loop_count = loop_count - 1;
            }
            if (WAV_Play(wave, loop_count) < 0) {
                return -1;
            }
        }
    }

    /* We'll get called again in the case where we looped or have more data */
    return 0;
#else // USE_SDL1
    WAV_Music *wave = (WAV_Music *)context;
#ifdef FULL // FILE_INT
    Sint64 pos, stop;
#ifdef FULL // WAV_LOOP
    WAVLoopPoint *loop;
    Sint64 loop_start;
    Sint64 loop_stop;
#endif
#else
    int pos, stop;
#ifdef FULL // WAV_LOOP
    WAVLoopPoint *loop;
    int loop_start;
    int loop_stop;
#endif
#endif // FULL
#ifdef FULL // WAV_LOOP
    SDL_bool looped = SDL_FALSE;
    unsigned i;
#endif
    int consumed;
    void *stream = data;

    pos = Mix_RWtell(wave->src);
    stop = wave->stop;
#ifdef FULL // WAV_LOOP
    loop = NULL;
    for (i = 0; i < wave->numloops; ++i) {
        loop = &wave->loops[i];
        if (loop->active) {
            const int bytes_per_sample = (SDL_AUDIO_BITSIZE(wave->spec.format) / 8) * wave->spec.channels;
            loop_start = wave->start + loop->start * bytes_per_sample;
            loop_stop = wave->start + (loop->stop + 1) * bytes_per_sample;
            if (pos >= loop_start && pos < loop_stop)
            {
                stop = loop_stop;
                break;
            }
        }
        loop = NULL;
    }
#endif // FULL - WAV_LOOP
    if (wave->cvt.needed) {
        int original_len = (int)((double)bytes/wave->cvt.len_ratio); 
        /* Make sure the length is a multiple of the sample size */
        {
            const int bytes_per_sample = (SDL_AUDIO_BITSIZE(wave->spec.format) / 8) * wave->spec.channels;
            const int alignment_mask = (bytes_per_sample - 1);
            original_len &= ~alignment_mask;
        }
        if (wave->cvt.len != original_len) {
            int worksize;
            if (wave->cvt.buf != NULL) {
                SDL_free(wave->cvt.buf);
            }
            worksize = original_len*wave->cvt.len_mult;
            wave->cvt.buf=(Uint8 *)SDL_malloc(worksize);
            if (wave->cvt.buf == NULL) {
                Mix_OutOfMemory();
                return -1;
            }
            wave->cvt.len = original_len;
        }
        if ((stop - pos) < original_len) {
            original_len = (int)(stop - pos);
        }
        original_len = (int)Mix_RWread(wave->src, wave->cvt.buf, 1, original_len);
        wave->cvt.len = original_len;
        SDL_ConvertAudio(&wave->cvt);
        //SDL_MixAudioFormat(stream, wave->cvt.buf, music_spec.format, wave->cvt.len_cvt, wave->volume);
        SDL_MixAudio(stream, wave->cvt.buf, wave->cvt.len_cvt, wave->volume);
        consumed = wave->cvt.len_cvt;
    } else {
        Uint8 *data;
        if ((stop - pos) < bytes) {
            bytes = (int)(stop - pos);
        }
        data = SDL_stack_alloc(Uint8, bytes);
        if (data) {
            bytes = (int)Mix_RWread(wave->src, data, 1, bytes); // MUS_ENC
            //SDL_MixAudioFormat(stream, data, music_spec.format, bytes, wave->volume);
            SDL_MixAudio(stream, data, bytes, wave->volume);
            SDL_stack_free(data);
        }
        consumed = bytes;
    }
#ifdef FULL // WAV_LOOP
    if (loop && Mix_RWtell(wave->src) >= stop) {
        if (loop->current_play_count == 1) {
            loop->active = SDL_FALSE;
        } else {
            if (loop->current_play_count > 0) {
                --loop->current_play_count;
            }
            if (Mix_RWseek(wave->src, loop_start, RW_SEEK_SET) < 0)
                return -1;
            looped = SDL_TRUE;
        }
    }
    if (!looped && (consumed == 0 || Mix_RWtell(wave->src) >= wave->stop)) {
#else // FULL - WAV_LOOP
    if (consumed == 0 || Mix_RWtell(wave->src) >= wave->stop) {
#endif // FULL - WAV_LOOP
        int loop_count = wave->loop_count;
        if (loop_count == 0) {
            if (consumed == 0) {
                *done = SDL_TRUE;
            }
        } else {
            if (loop_count > 0) {
                loop_count = (loop_count - 1);
            }
            if (WAV_Play(wave, loop_count) < 0) {
                return -1;
            }
        }
    }
    return consumed;
#endif // SDL_VERSION_ATLEAST(2, 0, 7)
#else // SELF CONV
    Mix_AudioSpec* audioSpec;
#ifdef FULL // MEM_OPS
    WAV_Music* wave;
#else
    Mix_RWops* playOps;
#endif
#ifdef FULL // FILE_INT
    Sint64 pos, stop;
#ifdef FULL // WAV_LOOP
    WAVLoopPoint* loop;
    Sint64 loop_start; // = wave->start;
    Sint64 loop_stop; // = wave->stop;
#endif
#else // FILE_INT
    size_t pos, stop;
    int len;
#ifdef FULL // WAV_LOOP
    WAVLoopPoint* loop;
    int loop_start; // = wave->start;
    int loop_stop; // = wave->stop;
#endif
#endif // FILE_INT
#ifdef FULL // WAV_LOOP
    SDL_bool looped = SDL_FALSE;
#endif
    SDL_bool at_end;
#ifdef FULL // WAV_LOOP
    unsigned i;
#endif
    int filled, amount;
    Uint8* cursor;
    Mix_BuffOps* buffOps = &channel->buffOps;

    cursor = (Uint8*)buffOps->currPos;
    filled = (int)((Uint8*)buffOps->endPos - cursor);
    if (filled != 0) {
        if (filled > bytes)
            filled = bytes;
        buffOps->currPos = cursor + filled;
#ifdef FULL // SOME_VOL
        SDL_memcpy(data, cursor, filled);
#else
        if (_Mix_DoEffects(cursor, filled, channel)) {
#ifdef FULL // SELF_MIX
#if SDL_VERSION_ATLEAST(2, 0, 0) // USE_SDL1
#ifdef FULL // FIX_OUT
            SDL_MixAudioFormat(snd, dst, music_spec.format, (Uint32)consumed, volume);
#else
            SDL_MixAudioFormat(snd, dst, MIX_DEFAULT_FORMAT, (Uint32)consumed, volume);
#endif
#else
            SDL_MixAudio(snd, dst, (Uint32)consumed, volume);
#endif
#else // SELF_MIX
#ifdef FULL // FIX_OUT
            Mix_MixAudioFormat(stream, cursor, MIX_DEFAULT_FORMAT, filled);
#else
            Mix_MixAudioFormat(stream, cursor, filled);
#endif
#endif // SELF_MIX
#endif // SOME_VOL
        }
        return filled;
    }

#ifdef FULL // MEM_OPS
    wave = &channel->chunk->asWAV;
    pos = Mix_RWtell(wave->src);
    stop = wave->stop;
#else
    playOps = &channel->playOps;
    pos = (size_t)playOps->currPos;
    stop = (size_t)playOps->endPos;
#endif // MEM_OPS
#ifdef FULL // WAV_LOOP
    loop = NULL;
    for (i = 0; i < wave->numloops; ++i) {
        loop = &wave->loops[i];
        if (loop->active) {
            const int bytes_per_sample = (SDL_AUDIO_BITSIZE(wave->spec.format) / 8) * wave->spec.channels;
            loop_start = wave->start + loop->start * (Uint32)bytes_per_sample;
            loop_stop = wave->start + (loop->stop + 1) * (Uint32)bytes_per_sample;
            if (pos >= loop_start && pos < loop_stop) {
                stop = loop_stop;
                break;
            }
        }
        loop = NULL;
    }
#endif
#ifdef FULL // MEM_OPS
    audioSpec = &wave->spec;
#else
    audioSpec = &channel->chunk->asWAV.spec;
#endif
    amount = (int)audioSpec->sampleSize;
    len = (int)(stop - pos);
    at_end = len <= amount;
    if (at_end) {
        amount = len;
    }
#ifdef FULL // MUS_ENC
    amount = wave->decode(wave, amount);
#else
    buffOps->currPos = buffOps->basePos;
#ifdef FULL // MEM_OPS
    amount = fetch_pcm(wave, buffOps, amount);
#else
    amount = fetch_pcm(playOps, buffOps, amount);
#endif
#endif
    if (amount > 0) {
        Mix_ConvertAudio(channel->chunk, buffOps);
    } else {
        /* We might be looping, continue */
        //at_end = SDL_TRUE;
    }
#ifdef FULL // MEM_OPS
    pos = Mix_RWtell(wave->src);
#else
    //pos = (int)playOps->currPos;
#endif
#ifdef FULL // WAV_LOOP
    if (loop && pos >= stop) {
        if (loop->current_play_count == 1) {
            loop->active = SDL_FALSE;
        } else {
            if (loop->current_play_count > 0) {
                --loop->current_play_count;
            }
            if (Mix_RWseek(wave->src, loop_start, RW_SEEK_SET) < 0)
                return -1;
            looped = SDL_TRUE;
        }
    }

    if (!looped && (at_end || pos >= wave->stop)) {
#else
#ifdef FULL // MEM_OPS
    if (at_end/* || pos >= wave->stop*/) {
#else
    if (at_end/* || pos >= (int)playOps->endPos*/) {
#endif // MEM_OPS
#endif // WAV_LOOP
#ifdef FULL // MEM_OPS
        int loop_count = wave->loop_count;
        if (loop_count == 0) {
            if (amount == 0) {
                /* All done */
                return -1;
            }
        } else {
            if (loop_count > 0) {
                loop_count = (loop_count - 1);
            }
            if (WAV_Play(wave, loop_count) < 0) {
                return -1;
            }
        }
#else
        int loop_count = channel->loop_count;
        if (loop_count == 0) {
            if (amount == 0) {
                /* All done */
                return -1;
            }
        } else {
#ifdef FULL // MUS_LOOP, LOOP
            if (loop_count > 0) {
                loop_count = (loop_count - 1);
            }
#endif
            return WAV_Play(channel, loop_count);
        }
#endif // MEM_OPS
    }

    /* We'll get called again in the case where we looped or have more data */
    return 0;
#endif // SELF_CONV
}
#ifdef FULL // FIX_MUS
static int WAV_GetAudio(void *context, void *data, int bytes)
{
    WAV_Music *wave = (WAV_Music *)context;
    return music_pcm_getaudio(context, data, bytes, wave->volume, WAV_GetSome);
}
#else
static int WAV_GetAudio(Mix_Channel* channel, void* stream, int bytes)
{
    return music_pcm_getaudio(channel, stream, bytes, WAV_GetSome);
}
#endif
#ifdef FULL // SEEK
static int WAV_Seek(void *context, double position)
{
    WAV_Music *wave = (WAV_Music *)context;
    Sint64 sample_size = wave->spec.freq * wave->samplesize;
    Sint64 dest_offset = (Sint64)(position * (double)wave->spec.freq * wave->samplesize);
    Sint64 destpos = wave->start + dest_offset;
    destpos -= dest_offset % sample_size;
    if (Mix_RWseek(wave->src, destpos, RW_SEEK_SET) < 0)
        return -1;
    return 0;
}

static double WAV_Tell(void *context)
{
    WAV_Music *wave = (WAV_Music *)context;
    Sint64 phys_pos = Mix_RWtell(wave->src);
    return (double)(phys_pos - wave->start) / (double)(wave->spec.freq * wave->samplesize);
}

/* Return music duration in seconds */
static double WAV_Duration(void *context)
{
    WAV_Music *wave = (WAV_Music *)context;
    Sint64 sample_size = wave->spec.freq * wave->samplesize;
    return (double)(wave->stop - wave->start) / sample_size;
}

static const char* WAV_GetMetaTag(void *context, Mix_MusicMetaTag tag_type)
{
    WAV_Music *wave = (WAV_Music *)context;
    return meta_tags_get(&wave->tags, tag_type);
}
#endif // FULL
/* Close the given WAV stream */
static void WAV_Delete(Mix_Audio* audio)
{
    WAV_Music* wave = &audio->asWAV;
#ifdef FULL // META
    /* Clean up associated data */
    meta_tags_clear(&wave->tags);
#endif
#ifdef FULL // WAV_LOOP
    if (wave->loops) {
        SDL_free(wave->loops);
#ifndef FULL // FIX_MUS
        wave->loops = NULL;
#endif
    }
#endif
#ifdef FULL // SELF_CONV
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
    if (wave->stream) {
        SDL_FreeAudioStream(wave->stream);
#ifndef FULL // FIX_MUS
        wave->stream = NULL;
#endif
    }
    if (wave->buffer) {
        SDL_free(wave->buffer);
#ifndef FULL // FIX_MUS
        wave->buffer = NULL;
#endif
    }
#else
    if (wave->cvt.buf != NULL) {
        SDL_free(wave->cvt.buf);
#ifndef FULL // FIX_MUS
        wave->cvt.buf = NULL;
#endif
    }
#endif
#endif // FULL - SELF_CONV
#ifdef FULL // SRC_PTR
    if (wave->freesrc) {
        Mix_RWclose(wave->src);
#ifndef FULL // FIX_MUS
        wave->freesrc = SDL_FALSE;
#endif
    }
#endif // SRC_PTR
#ifndef FULL // MEM_OPS
    SDL_free(wave->src.basePos);
#endif
#ifdef FULL // FIX_MUS
    SDL_free(wave);
#endif
}

static int ParseFMT(WAV_Music *wave, Uint32 chunk_length)
{
#ifdef FULL // SELF_CONV
    SDL_AudioSpec *spec = &wave->spec;
#else
    Mix_AudioSpec* spec = &wave->spec;
    int freq;
#endif
    WaveFMTEx fmt;
    unsigned size;
    Uint16 bits, encoding;

    if (chunk_length < sizeof(fmt.format)) {
        return Mix_SetError("Wave format chunk too small");
    }

    size = (chunk_length >= sizeof(fmt)) ? sizeof(fmt) : sizeof(fmt.format);
#ifdef FULL // MEM_OPS
    if (!Mix_RWread(wave->src, &fmt, size, 1)) {
#else
    if (!Mix_RWread(&wave->src, &fmt, size)) {
#endif
        return Mix_SetError("Couldn't read %d bytes from WAV file", chunk_length);
    }
    chunk_length -= size;
    if (chunk_length != 0 && Mix_RWseek(&wave->src, chunk_length, RW_SEEK_CUR) < 0) {
        return Mix_SetError("Couldn't read %d bytes from WAV file", chunk_length);
    }

    encoding = SDL_SwapLE16(fmt.format.encoding);

    if (encoding == EXT_CODE) {
        if (size < sizeof(fmt)) {
            return Mix_SetError("Wave format chunk too small");
        }
        encoding = (Uint16)SDL_SwapLE32(fmt.subencoding);
    }
#ifdef FULL // MUS_ENC
#if SDL_VERSION_ATLEAST(2, 0, 7) // USE_SDL1
    /* Decode the audio data format */
    switch (encoding) {
        case PCM_CODE:
        case FLOAT_CODE:
            /* We can understand this */
            wave->decode = fetch_pcm;
            break;
        case uLAW_CODE:
            /* , this */
            wave->decode = fetch_ulaw;
            break;
        case ALAW_CODE:
            /* , and this */
            wave->decode = fetch_alaw;
            break;
        default:
            /* but NOT this */
            Mix_SetError("Unknown WAVE data format");
            return SDL_FALSE;
    }
#endif // SDL_VERSION_ATLEAST(2, 0, 7)
#else
    if (encoding != PCM_CODE) {
        return Mix_SetError("Unknown WAVE data format");
    }
#endif // FULL - MUS_ENC
#ifdef FULL // SELF_CONV
    spec->freq = (int)SDL_SwapLE32(fmt.format.frequency);
#else
    freq = (int)SDL_SwapLE32(fmt.format.frequency);
    if (freq == MIX_DEFAULT_FREQUENCY) {
        spec->freqMpl = 1;
    } else if (freq == 2 * MIX_DEFAULT_FREQUENCY) {
        spec->freqMpl = 2;
    } else {
        return Mix_SetError("Unknown WAVE data format");
    }
#endif // FULL - SELF_CONV
    bits = SDL_SwapLE16(fmt.format.bitspersample);
    switch (bits) {
        case 8:
#ifdef FULL // MUS_ENC
            switch(encoding) {
            case PCM_CODE:  spec->format = AUDIO_U8; break;
            case ALAW_CODE: spec->format = AUDIO_S16; break;
            case uLAW_CODE: spec->format = AUDIO_S16; break;
            default: goto unknown_bits;
            }
#else
            spec->format = AUDIO_U8;
#endif
            break;
        case 16:
#ifdef FULL // MUS_ENC
            switch(encoding) {
            case PCM_CODE: spec->format = AUDIO_S16; break;
            default: goto unknown_bits;
            }
#else
            spec->format = AUDIO_S16;
#endif
            break;
#ifdef FULL // WAV_SRC
        case 24:
            switch(encoding) {
            case PCM_CODE:
                wave->decode = fetch_pcm24le;
                spec->format = AUDIO_S32;
                break;
            default: goto unknown_bits;
            }
            break;
        case 32:
            switch(encoding) {
            case PCM_CODE:   spec->format = AUDIO_S32; break;
            case FLOAT_CODE: spec->format = AUDIO_F32; break;
            default: goto unknown_bits;
            }
            break;
        case 64:
            switch(encoding) {
            case FLOAT_CODE:
                wave->decode = fetch_float64le;
                spec->format = AUDIO_F32;
                break;
            default: goto unknown_bits;
            }
            break;
#endif // FULL - WAV_SRC
        default:
#ifdef FULL // WAV_SRC, MUS_ENC
            unknown_bits:
            return Mix_SetError("Unknown PCM format with %d bits", bits);
#else
            return Mix_SetError("Unknown WAVE data format");
#endif
    }
#ifdef FULL // WAV_ENC
    wave->encoding = encoding;
#endif
    spec->channels = (Uint8) SDL_SwapLE16(fmt.format.channels);
#ifdef FULL // SELF_CONV
    spec->samples = MIX_STREAM_SAMPLE_COUNT;
#ifdef FULL // MUS_ENC, SEEK
    wave->samplesize = spec->channels * (bits / 8);
#endif
    /* SDL_CalculateAudioSpec */
    spec->size = SDL_AUDIO_BITSIZE(spec->format) / 8;
    spec->size *= spec->channels;
    spec->size *= spec->samples;
#else
    /* Calculate Mix_AudioSpec */
    Mix_CalculateSampleSize(spec);
#endif // FULL - SELF_CONV
    return 0;
}

static SDL_bool ParseDATA(WAV_Music *wave, Uint32 chunk_length)
{
    wave->start = (unsigned)Mix_RWtell(&wave->src);
    wave->stop = wave->start + chunk_length;
    return Mix_RWseek(&wave->src, chunk_length, RW_SEEK_CUR) >= 0 ? SDL_TRUE : SDL_FALSE;
}
#ifdef FULL // WAV_LOOP
static SDL_bool AddLoopPoint(WAV_Music *wave, Uint32 play_count, Uint32 start, Uint32 stop)
{
    WAVLoopPoint *loop;
    WAVLoopPoint *loops = SDL_realloc(wave->loops, (wave->numloops + 1) * sizeof(*wave->loops));
    if (!loops) {
        Mix_OutOfMemory();
        return SDL_FALSE;
    }

    loop = &loops[ wave->numloops ];
    loop->start = start;
    loop->stop = stop;
    loop->initial_play_count = play_count;
    loop->current_play_count = play_count;

    wave->loops = loops;
    ++wave->numloops;
    return SDL_TRUE;
}

static SDL_bool ParseSMPL(WAV_Music *wave, Uint32 chunk_length)
{
    SamplerChunk *chunk;
    Uint8 *data;
    Uint32 i;
    SDL_bool loaded = SDL_FALSE;

    data = (Uint8 *)SDL_malloc(chunk_length);
    if (!data) {
        Mix_OutOfMemory();
        return SDL_FALSE;
    }
    if (!Mix_RWread(wave->src, data, chunk_length, 1)) {
        Mix_SetError("Couldn't read %d bytes from WAV file", chunk_length);
        SDL_free(data);
        return SDL_FALSE;
    }
    chunk = (SamplerChunk *)data;

    for (i = 0; i < SDL_SwapLE32(chunk->sample_loops); ++i) {
        const Uint32 LOOP_TYPE_FORWARD = 0;
        Uint32 loop_type = SDL_SwapLE32(chunk->loops[i].type);
        if (loop_type == LOOP_TYPE_FORWARD) {
            AddLoopPoint(wave, SDL_SwapLE32(chunk->loops[i].play_count), SDL_SwapLE32(chunk->loops[i].start), SDL_SwapLE32(chunk->loops[i].end));
        }
    }

    loaded = SDL_TRUE;
    SDL_free(data);
    return loaded;
}
#endif
#ifdef FULL // META
static void read_meta_field(Mix_MusicMetaTags *tags, Mix_MusicMetaTag tag_type, size_t *i, Uint32 chunk_length, Uint8 *data, size_t fieldOffset)
{
    Uint32 len = 0;
    int isID3 = fieldOffset == 7;
    char *field = NULL;
    *i += 4;
    len = isID3 ?
          SDL_SwapBE32(*((Uint32 *)(data + *i))) : /* ID3  */
          SDL_SwapLE32(*((Uint32 *)(data + *i))); /* LIST */
    if (len > chunk_length) {
        return; /* Do nothing due to broken lenght */
    }
    *i += fieldOffset;
    field = (char *)SDL_malloc(len + 1);
    SDL_memset(field, 0, (len + 1));
    SDL_strlcpy(field, (char *)(data + *i), isID3 ? len - 1 : len);
    *i += len;
    meta_tags_set(tags, tag_type, field);
    SDL_free(field);
}

static SDL_bool ParseLIST(WAV_Music *wave, Uint32 chunk_length)
{
    Uint8 *data;

    data = (Uint8 *)SDL_malloc(chunk_length);
    if (!data) {
        Mix_OutOfMemory();
        return SDL_FALSE;
    }

    if (!Mix_RWread(wave->src, data, chunk_length, 1)) {
        Mix_SetError("Couldn't read %d bytes from WAV file", chunk_length);
        SDL_free(data);
        return SDL_FALSE;
    }

    if (SDL_strncmp((char *)data, "INFO", 4) == 0) {
        size_t i = 4;
        for (i = 4; i < chunk_length - 4;) {
            if(SDL_strncmp((char *)(data + i), "INAM", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_TITLE, &i, chunk_length, data, 4);
                continue;
            } else if(SDL_strncmp((char *)(data + i), "IART", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_ARTIST, &i, chunk_length, data, 4);
                continue;
            } else if(SDL_strncmp((char *)(data + i), "IALB", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_ALBUM, &i, chunk_length, data, 4);
                continue;
            } else if (SDL_strncmp((char *)(data + i), "BCPR", 4) == 0) {
                read_meta_field(&wave->tags, MIX_META_COPYRIGHT, &i, chunk_length, data, 4);
                continue;
            }
            i++;
        }
    }

    /* done: */
    SDL_free(data);

    return SDL_TRUE;
}
#endif // FULL
static SDL_bool LoadWAVMusic(WAV_Music *wave)
{
    Mix_RWops *src = &wave->src;
    Uint32 chunk_type;
    Uint32 chunk_length;
    SDL_bool found_FMT = SDL_FALSE;
#ifdef FULL // META, WAV_LOOP
    SDL_bool found_DATA = SDL_FALSE;
#endif
    /* WAV magic header */
    Uint32 wavelen;
    Uint32 WAVEmagic;
#ifdef FULL // META
    meta_tags_init(&wave->tags);
#endif
    /* Check the magic header */
    wavelen = Mix_ReadLE32(src);
    WAVEmagic = Mix_ReadLE32(src);

    (void)wavelen;   /* unused */
    (void)WAVEmagic; /* unused */

    /* Read the chunks */
    for (; ;) {
        chunk_type = Mix_ReadLE32(src);
        chunk_length = Mix_ReadLE32(src);

        if (chunk_length == 0)
            break;

        switch (chunk_type)
        {
        case FMT:
            found_FMT = SDL_TRUE;
            if (ParseFMT(wave, chunk_length) < 0)
                return SDL_FALSE;
            break;
        case DATA:
#ifdef FULL // META, WAV_LOOP
            found_DATA = SDL_TRUE;
            if (!ParseDATA(wave, chunk_length))
                return SDL_FALSE;
#else
            if (ParseDATA(wave, chunk_length) && found_FMT)
                return SDL_TRUE;
#endif
            break;
#ifdef FULL // WAV_LOOP
        case SMPL:
            if (!ParseSMPL(wave, chunk_length))
                return SDL_FALSE;
            break;
#endif
#ifdef FULL // META
        case LIST:
            if (!ParseLIST(wave, chunk_length))
                return SDL_FALSE;
            break;
#endif
        default:
            if (Mix_RWseek(src, chunk_length, RW_SEEK_CUR) < 0)
                return SDL_FALSE;
            break;
        }
    }
#ifdef FULL // META, WAV_LOOP
    if (!found_FMT) {
        Mix_SetError("Bad WAV file (no FMT chunk)");
        return SDL_FALSE;
    }

    if (!found_DATA) {
        Mix_SetError("Bad WAV file (no DATA chunk)");
        return SDL_FALSE;
    }
    return SDL_TRUE;
#else
    Mix_SetError("Unknown WAVE data format");
    return SDL_FALSE;
#endif
}

/* I couldn't get SANE_to_double() to work, so I stole this from libsndfile.
 * I don't pretend to fully understand it.
 */
#ifdef FULL // WAV_SRC
static Uint32 SANE_to_Uint32 (Uint8 *sanebuf)
{
    /* Negative number? */
    if (sanebuf[0] & 0x80)
        return 0;

    /* Less than 1? */
    if (sanebuf[0] <= 0x3F)
        return 1;

    /* Way too big? */
    if (sanebuf[0] > 0x40)
        return 0x4000000;

    /* Still too big? */
    if (sanebuf[0] == 0x40 && sanebuf[1] > 0x1C)
        return 800000000;

    return (Uint32)(((sanebuf[2] << 23) | (sanebuf[3] << 15) | (sanebuf[4] << 7) |
            (sanebuf[5] >> 1)) >> (29 - sanebuf[1]));
}

static SDL_bool LoadAIFFMusic(WAV_Music *wave)
{
    Mix_RWops *src = wave->src;
    SDL_AudioSpec *spec = &wave->spec;
    SDL_bool found_SSND = SDL_FALSE;
    SDL_bool found_COMM = SDL_FALSE;
    SDL_bool found_FVER = SDL_FALSE;
    SDL_bool is_AIFC = SDL_FALSE;

    Uint32 chunk_type;
    Uint32 chunk_length;
    Sint64 next_chunk = 0;
    Sint64 file_length;

    /* AIFF magic header */
    Uint32 AIFFmagic;
    /* SSND chunk        */
    Uint32 offset;
    Uint32 blocksize;
    /* COMM format chunk */
    Uint16 channels = 0;
    Uint32 numsamples = 0;
    Uint16 samplesize = 0;
    Uint8 sane_freq[10];
    Uint32 frequency = 0;
    Uint32 AIFCVersion1 = 0;
    Uint32 compressionType = 0;
    char *chunk_buffer;

    file_length = Mix_RWsize(src);

    /* Check the magic header */
    chunk_length = Mix_ReadBE32(src);
    AIFFmagic = Mix_ReadLE32(src);
    if (AIFFmagic != AIFF && AIFFmagic != AIFC) {
        Mix_SetError("Unrecognized file type (not AIFF or AIFC)");
        return SDL_FALSE;
    }
    if (AIFFmagic == AIFC) {
        is_AIFC = SDL_TRUE;
    }

    /* From what I understand of the specification, chunks may appear in
     * any order, and we should just ignore unknown ones.
     *
     * TODO: Better sanity-checking. E.g. what happens if the AIFF file
     *       contains compressed sound data?
     */
    do {
        chunk_type      = Mix_ReadLE32(src);
        chunk_length    = Mix_ReadBE32(src);
        next_chunk      = Mix_RWtell(src) + chunk_length;

        if (chunk_length % 2) {
            next_chunk++;
        }

        switch (chunk_type) {
        case SSND:
            found_SSND = SDL_TRUE;
            offset = Mix_ReadBE32(src);
            blocksize = Mix_ReadBE32(src);
            wave->start = Mix_RWtell(src) + offset;
            (void)blocksize; /* unused */
            break;

        case FVER:
            found_FVER = SDL_TRUE;
            AIFCVersion1 = Mix_ReadBE32(src);
            (void)AIFCVersion1; /* unused */
            break;

        case MARK:
        case INST:
            /* Just skip those chunks */
            break;

        case NAME:
        case AUTH:
        case _c__:
            chunk_buffer = (char*)SDL_calloc(1, chunk_length + 1);
            if (Mix_RWread(src, chunk_buffer, 1, chunk_length) != chunk_length) {
                SDL_free(chunk_buffer);
                return SDL_FALSE;
            }
            meta_tags_set(&wave->tags,
                          chunk_type == NAME ? MIX_META_TITLE :
                          chunk_type == AUTH ? MIX_META_ARTIST :
                          chunk_type == _c__ ? MIX_META_COPYRIGHT : 0,
                          chunk_buffer);
            SDL_free(chunk_buffer);
            break;

        case COMM:
            found_COMM = SDL_TRUE;

            /* Read the audio data format chunk */
            channels = Mix_ReadBE16(src);
            numsamples = Mix_ReadBE32(src);
            samplesize = Mix_ReadBE16(src);
            Mix_RWread(src, sane_freq, sizeof(sane_freq), 1);
            frequency = SANE_to_Uint32(sane_freq);
            if (is_AIFC) {
                compressionType = Mix_ReadLE32(src);
                /* here must be a "compressionName" which is a padded string */
            }
            break;

        default:
            /* Unknown/unsupported chunk: we just skip over */
            break;
        }
    } while (next_chunk < file_length && Mix_RWseek(src, next_chunk, RW_SEEK_SET) >= 0);

    if (!found_SSND) {
        Mix_SetError("Bad AIFF/AIFF-C file (no SSND chunk)");
        return SDL_FALSE;
    }

    if (!found_COMM) {
        Mix_SetError("Bad AIFF/AIFF-C file (no COMM chunk)");
        return SDL_FALSE;
    }

    if (is_AIFC && !found_FVER) {
        Mix_SetError("Bad AIFF-C file (no FVER chunk)");
        return SDL_FALSE;
    }


    wave->samplesize = channels * (samplesize / 8);
    wave->stop = wave->start + channels * numsamples * (samplesize / 8);

    /* Decode the audio data format */
    SDL_memset(spec, 0, (sizeof *spec));
    spec->freq = (int)frequency;
    switch (samplesize) {
    case 8:
        if (!is_AIFC)
            spec->format = AUDIO_S8;
        else switch (compressionType) {
        case raw_: spec->format = AUDIO_U8; break;
        case sowt: spec->format = AUDIO_S8; break;
        case ulaw:
            spec->format = AUDIO_S16LSB;
            wave->encoding = uLAW_CODE;
            wave->decode = fetch_ulaw;
            break;
        case alaw:
            spec->format = AUDIO_S16LSB;
            wave->encoding = ALAW_CODE;
            wave->decode = fetch_alaw;
            break;
        default: goto unsupported_format;
        }
        break;
    case 16:
        if (!is_AIFC)
            spec->format = AUDIO_S16MSB;
        else switch (compressionType) {
        case sowt: spec->format = AUDIO_S16LSB; break;
        case NONE: spec->format = AUDIO_S16MSB; break;
        case ULAW:
            spec->format = AUDIO_S16LSB;
            wave->encoding = uLAW_CODE;
            wave->decode = fetch_ulaw;
            break;
        case ALAW:
            spec->format = AUDIO_S16LSB;
            wave->encoding = ALAW_CODE;
            wave->decode = fetch_alaw;
            break;
        default: goto unsupported_format;
        }
        break;
    case 24:
        wave->encoding = PCM_CODE;
        wave->decode = fetch_pcm24be;
        if (!is_AIFC)
            spec->format = AUDIO_S32MSB;
        else switch (compressionType) {
        case sowt: spec->format = AUDIO_S32LSB; break;
        case NONE: spec->format = AUDIO_S32MSB; break;
        default: goto unsupported_format;
        }
        break;
    case 32:
        if (!is_AIFC)
            spec->format = AUDIO_S32MSB;
        else switch (compressionType) {
        case sowt: spec->format = AUDIO_S32LSB; break;
        case NONE: spec->format = AUDIO_S32MSB; break;
        case fl32:
        case FL32: spec->format = AUDIO_F32MSB; break;
        default: goto unsupported_format;
        }
        break;
    case 64:
        wave->encoding = FLOAT_CODE;
        wave->decode = fetch_float64be;
        if (!is_AIFC)
            spec->format = AUDIO_F32;
        else switch (compressionType) {
        case fl64:
            spec->format = AUDIO_F32;
            break;
        default: goto unsupported_format;
        }
        break;
    default:
    unsupported_format:
        Mix_SetError("Unknown samplesize in data format");
        return SDL_FALSE;
    }
    spec->channels = (Uint8) channels;
    spec->samples = 4096;       /* Good default buffer size */
    spec->size = SDL_AUDIO_BITSIZE(spec->format) / 8;
    spec->size *= spec->channels;
    spec->size *= spec->samples;

    return SDL_TRUE;
}
#endif // FULL
const Mix_MusicInterface Mix_MusicInterface_WAV =
{
#ifdef FULL // WAV_SRC
    "WAVE",
    MIX_MUSIC_WAVE,
    MUS_WAV,
    SDL_FALSE,
    SDL_FALSE,

    NULL,   /* Load */
    NULL,   /* Open */
#endif
    WAV_CreateFromRW,
#ifdef FULL // WAV_SRC
    NULL,   /* CreateFromFile */
#endif
#ifdef FULL // FIX_MUS
    WAV_SetVolume,
#ifdef FULL
    WAV_GetVolume,
#endif
    WAV_Play,
#endif // FULL - FIX_MUS
#ifdef FULL
    NULL,   /* IsPlaying */
#endif
    WAV_GetAudio,
#ifdef FULL
    NULL,       /* Jump */
    WAV_Seek,   /* Seek */
    WAV_Tell,   /* Tell */
    WAV_Duration,
    NULL,   /* LoopStart */
    NULL,   /* LoopEnd */
    NULL,   /* LoopLength */
    WAV_GetMetaTag,   /* GetMetaTag */
    NULL,   /* Pause */
    NULL,   /* Resume */
    NULL,   /* Stop */
#endif
    WAV_Delete,
#ifdef FULL // WAV_SRC
    NULL,   /* Close */
    NULL    /* Unload */
#endif
};

#endif /* MUSIC_WAV */

/* vi: set ts=4 sw=4 expandtab: */
