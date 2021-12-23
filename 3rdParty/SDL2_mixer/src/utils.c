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

/* misc helper routines */

#include "utils.h"
#include "SDL_cpuinfo.h"
#include <stddef.h>

#ifndef FULL // SELF_CONV
void Mix_Converter_AUDIO16_Mono2Stereo(Mix_BuffOps* buf);
void (*Mix_Convert_AUDIO16_Mono2Stereo)(Mix_BuffOps* buf) = Mix_Converter_AUDIO16_Mono2Stereo;
#endif

#ifdef FULL // META
#if !defined(HAVE_SDL_STRTOKR)
/*
 * Adapted from _PDCLIB_strtok() of PDClib library at
 * https://github.com/DevSolar/pdclib.git
 *
 * The code was under CC0 license:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode :
 *
 *                        No Copyright
 *
 * The person who associated a work with this deed has dedicated the
 * work to the public domain by waiving all of his or her rights to
 * the work worldwide under copyright law, including all related and
 * neighboring rights, to the extent allowed by law.
 *
 * You can copy, modify, distribute and perform the work, even for
 * commercial purposes, all without asking permission. See Other
 * Information below.
 */
char *SDL_strtokr(char *s1, const char *s2, char **ptr)
{
    const char *p = s2;

    if (!s2 || !ptr || (!s1 && !*ptr)) return NULL;

    if (s1 != NULL) {  /* new string */
        *ptr = s1;
    } else { /* old string continued */
        if (*ptr == NULL) {
        /* No old string, no new string, nothing to do */
            return NULL;
        }
        s1 = *ptr;
    }

    /* skip leading s2 characters */
    while (*p && *s1) {
        if (*s1 == *p) {
        /* found separator; skip and start over */
            ++s1;
            p = s2;
            continue;
        }
        ++p;
    }

    if (! *s1) { /* no more to parse */
        *ptr = s1;
        return NULL;
    }

    /* skipping non-s2 characters */
    *ptr = s1;
    while (**ptr) {
        p = s2;
        while (*p) {
            if (**ptr == *p++) {
            /* found separator; overwrite with '\0', position *ptr, return */
                *((*ptr)++) = '\0';
                return s1;
            }
        }
        ++(*ptr);
    }

    /* parsed to end of string */
    return s1;
}
#endif /* HAVE_SDL_STRTOKR */
#endif // FULL - META
#ifdef FULL
/* Is given tag a loop tag? */
SDL_bool _Mix_IsLoopTag(const char *tag)
{
    char buf[5];
    SDL_strlcpy(buf, tag, 5);
    return SDL_strcasecmp(buf, "LOOP") == 0;
}

/* Parse time string of the form HH:MM:SS.mmm and return equivalent sample
 * position */
Sint64 _Mix_ParseTime(char *time, long samplerate_hz)
{
    char *num_start, *p;
    Sint64 result;
    char c;
    int val;

    /* Time is directly expressed as a sample position */
    if (SDL_strchr(time, ':') == NULL) {
        return SDL_strtoll(time, NULL, 10);
    }

    result = 0;
    num_start = time;

    for (p = time; *p != '\0'; ++p) {
        if (*p == '.' || *p == ':') {
            c = *p; *p = '\0';
            if ((val = SDL_atoi(num_start)) < 0)
                return -1;
            result = result * 60 + val;
            num_start = p + 1;
            *p = c;
        }

        if (*p == '.') {
            double val_f = SDL_atof(p);
            if (val_f < 0) return -1;
            return result * samplerate_hz + (Sint64) (val_f * samplerate_hz);
        }
    }

    if ((val = SDL_atoi(num_start)) < 0) return -1;
    return (result * 60 + val) * samplerate_hz;
}
#endif // FULL

#ifndef FULL // WAV_SRC

/* The volume ranges from 0 - 128 */
#define ADJUST_VOLUME(s, v) (s = (s*v)/MIX_MAX_VOLUME)
#define ADJUST_VOLUME_U8(s, v)  (s = (((s-128)*v)/MIX_MAX_VOLUME)+128)

/**
 * Mix audio buffers. Based on SDL_MixAudioFormat of SDL2/SDL_audio.
 */
void Mix_MixAudioFormat(Uint8* dst, const Uint8* src, SDL_AudioFormat format, int len, int volume)
{
    if (volume == 0) {
        return;
    }

    if (format == AUDIO_U8) {
        Uint8 src_sample;
        int dst_sample;

        while (len--) {
            src_sample = *src;
            ADJUST_VOLUME_U8(src_sample, volume);
            dst_sample = *dst + src_sample;
            dst_sample -= 128;
            if (dst_sample < 0)
                dst_sample = 0;
            else if (dst_sample > SDL_MAX_UINT8)
                dst_sample = SDL_MAX_UINT8;
            *dst = dst_sample;
            ++dst;
            ++src;
        }
    } else {
    // assert(format == AUDIO_S16LSB);
        Sint16 src1, src2;
        int dst_sample;
        const int max_audioval = SDL_MAX_SINT16;
        const int min_audioval = SDL_MIN_SINT16;

        len /= 2;
        while (len--) {
            src1 = SDL_SwapLE16(*(Sint16*)src);
            ADJUST_VOLUME(src1, volume);
            src2 = SDL_SwapLE16(*(Sint16*)dst);
            src += 2;
            dst_sample = src1 + src2;
            if (dst_sample > max_audioval) {
                dst_sample = max_audioval;
            } else if (dst_sample < min_audioval) {
                dst_sample = min_audioval;
            }
            *(Sint16*)dst = SDL_SwapLE16(dst_sample);
            dst += 2;
        }
    }
}
#endif // FULL - WAV_SRC

#ifndef FULL // SELF_CONV
#ifdef __AVX__
void Mix_Converter_AUDIO16_Mono2Stereo_AVX(Mix_BuffOps* buf)
{
    Sint16* srcPos = (Sint16*)buf->endPos;
    Sint16* currPos = (Sint16*)buf->currPos;

    Sint16* dstPos = srcPos + (srcPos - currPos);
    buf->endPos = dstPos;

    while (&currPos[16] <= srcPos) {
        srcPos -= 16;
        dstPos -= 16;
        __m256i aa = _mm256_loadu_si256(srcPos);
        __m256i zero = _mm256_setzero_si256();
        __m256i bb0 = _mm256_unpackhi_epi16(aa, zero);
        __m256i bb1 = _mm256_slli_si256(bb0, 2);
        __m256i bb = _mm256_or_si256(bb0, bb1);
        _mm256_storeu_si256(dstPos, bb);

        dstPos -= 16;
        __m256i cc0 = _mm256_unpacklo_epi16(aa, zero);
        __m256i cc1 = _mm256_slli_si256(cc0, 2);
        __m256i cc = _mm256_or_si256(cc0, cc1);
        _mm256_storeu_si256(dstPos, cc);
    }

    while (srcPos != currPos) {
        srcPos--;
        dstPos--;
        *dstPos = *srcPos;
        dstPos--;
        *dstPos = *srcPos;
    }
}
#endif // __AVX__
#ifdef __SSE2__
void Mix_Converter_AUDIO16_Mono2Stereo_SSE2(Mix_BuffOps* buf)
{
    Sint16* srcPos = (Sint16*)buf->endPos;
    Sint16* currPos = (Sint16*)buf->currPos;

    Sint16* dstPos = srcPos + (srcPos - currPos);
    buf->endPos = dstPos;

    while (&currPos[8] <= srcPos) {
        srcPos -= 8;
        dstPos -= 8;
        __m128i aa = _mm_loadu_si128(srcPos);
        __m128i zero = _mm_setzero_si128();
        __m128i bb0 = _mm_unpackhi_epi16(aa, zero);
        __m128i bb1 = _mm_slli_si128(bb0, 2);
        __m128i bb = _mm_or_si128(bb0, bb1);
        _mm_storeu_si128(dstPos, bb);

        dstPos -= 8;
        __m128i cc0 = _mm_unpacklo_epi16(aa, zero);
        __m128i cc1 = _mm_slli_si128(cc0, 2);
        __m128i cc = _mm_or_si128(cc0, cc1);
        _mm_storeu_si128(dstPos, cc);
    }

    while (srcPos != currPos) {
        srcPos--;
        dstPos--;
        *dstPos = *srcPos;
        dstPos--;
        *dstPos = *srcPos;
    }
}
#endif // _SSE2_

void Mix_Converter_AUDIO16_Mono2Stereo(Mix_BuffOps* buf)
{
    Sint16* srcPos = (Sint16*)buf->endPos;
    Sint16* currPos = (Sint16*)buf->currPos;

    Sint16* dstPos = srcPos + (srcPos - currPos);
    buf->endPos = dstPos;

    while (srcPos != currPos) {
        srcPos--;
        dstPos--;
        *dstPos = *srcPos;
        dstPos--;
        *dstPos = *srcPos;
    }
}

void Mix_Utils_Init()
{
#ifdef __SSE2__
    if (SDL_HasSSE2()) {
        Mix_Convert_AUDIO16_Mono2Stereo = Mix_Converter_AUDIO16_Mono2Stereo_SSE2;
    }
#endif
#if defined(__AVX__) && SDL_VERSION_ATLEAST(2, 0, 2)
    if (SDL_HasAVX()) {
        Mix_Convert_AUDIO16_Mono2Stereo = Mix_Converter_AUDIO16_Mono2Stereo_AVX;
    }
#endif
}
#endif // FULL - SELF_CONV
