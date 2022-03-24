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
//void Mix_Converter_AUDIO8_Mono2Stereo(Mix_BuffOps* buf);
//void (*Mix_Convert_AUDIO8_Mono2Stereo)(Mix_BuffOps* buf) = Mix_Converter_AUDIO8_Mono2Stereo;
static void Mix_Converter_AUDIO16_Mono2Stereo(Mix_BuffOps* buf);
static void (*Mix_Convert_AUDIO16_Mono2Stereo)(Mix_BuffOps* buf) = Mix_Converter_AUDIO16_Mono2Stereo;
static void Mix_Converter_AUDIO8_Resample_Half(Mix_BuffOps* buf);
static void (*Mix_Convert_AUDIO8_Resample_Half)(Mix_BuffOps* buf) = Mix_Converter_AUDIO8_Resample_Half;
static void Mix_Converter_AUDIO16_Resample_Half(Mix_BuffOps* buf);
static void (*Mix_Convert_AUDIO16_Resample_Half)(Mix_BuffOps* buf) = Mix_Converter_AUDIO16_Resample_Half;
static void Mix_Converter_U8_S16LSB(Mix_BuffOps* buf);
static void (*Mix_Convert_U8_S16LSB)(Mix_BuffOps* buf) = Mix_Converter_U8_S16LSB;
void Mix_Mixer_AUDIOS16(void* dst, const void* src, unsigned len);
void (*Mix_MixAudioFormat)(void* dst, const void* src, unsigned len) = Mix_Mixer_AUDIOS16;

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

#ifndef FULL // MEM_OPS
size_t Mix_RWread(Mix_RWops* src, void* dst, size_t len)
{
    if ((Uint8*)src->currPos + len <= (Uint8*)src->endPos) {
        memcpy(dst, src->currPos, len);
        src->currPos = (Uint8*)src->currPos + len;
        return len;
    }
    return 0;
}

size_t Mix_RWtell(Mix_RWops* src)
{
    return (size_t)src->currPos - (size_t)src->basePos;
}

void Mix_RWclose(Mix_RWops* src)
{
    SDL_free(src);
}

int Mix_RWseek(Mix_RWops* src, int offset, int whence)
{
    void* newPos;

    switch (whence) {
    case RW_SEEK_SET:
        newPos = src->basePos;
        break;
    case RW_SEEK_CUR:
        newPos = src->currPos;
        break;
    case RW_SEEK_END:
        newPos = src->endPos;
        break;
    default:
        ASSUME_UNREACHABLE;
        break;
    }

    newPos = (Uint8*)newPos + offset;
    if (newPos >= src->basePos && newPos <= src->endPos) {
        src->currPos = newPos;
        return 1;
    }
    return -1;
}

Uint32 Mix_ReadLE32(Mix_RWops* src)
{
    Uint32 result = 0;

    if ((Uint8*)src->endPos >= (Uint8*)src->currPos + sizeof(Uint32)) {
        result = SDL_SwapLE32(*(Uint32*)src->currPos);
        src->currPos = (Uint8*)src->currPos + sizeof(Uint32);
    }

    return result;
}
#ifdef FULL // SRC_PTR
Mix_RWops* Mix_RWFromConstMem(const void* mem, size_t size)
{
    Mix_RWops* rwOps = (Mix_RWops*)malloc(sizeof(Mix_RWops));

    if (rwOps != NULL) {
        rwOps->basePos = (void*)mem;
        rwOps->currPos = (void*)mem;
        rwOps->endPos = (Uint8*)mem + size;
    }

    return rwOps;
}
#endif
void Mix_RWFromMem(Mix_RWops* rwOps, const void* mem, size_t size)
{
    rwOps->basePos = (void*)mem;
    rwOps->currPos = (void*)mem;
    rwOps->endPos = (Uint8*)mem + size;
}
#endif // FULL - MEM_OPS

#ifndef FULL // WAV_SRC

/* The volume ranges from 0 - 128 */
#define ADJUST_VOLUME(s, v) (s = (s*v)/MIX_MAX_VOLUME)
#define ADJUST_VOLUME_U8(s, v)  (s = (((s-128)*v)/MIX_MAX_VOLUME)+128)

/**
 * Mix audio buffers. Based on SDL_MixAudioFormat of SDL2/SDL_audio.
 */
void Mix_Mixer_AUDIOS16(void* dst, const void* src, unsigned len)
{
    // assert(format == AUDIO_S16LSB);
    const Sint16* srcPos = (const Sint16*)src;
    Sint16* currPos = (Sint16*)dst;

    Sint16 src1, src2;
    int dst_sample;
    const int max_audioval = SDL_MAX_SINT16;
    const int min_audioval = SDL_MIN_SINT16;

    len /= 2;
    while (len--) {
        src1 = SDL_SwapLE16(*srcPos);
        src2 = SDL_SwapLE16(*currPos);
        dst_sample = src1 + src2;
        if (dst_sample > max_audioval) {
            dst_sample = max_audioval;
        } else if (dst_sample < min_audioval) {
            dst_sample = min_audioval;
        }
        *currPos = SDL_SwapLE16(dst_sample);
        srcPos++;
        currPos++;
    }
}
#ifdef __SSE2__
void Mix_Mixer_AUDIOS16_SSE2(void* dst, const void* src, unsigned len)
{
    const Sint16* srcPos = (const Sint16*)src;
    Sint16* currPos = (Sint16*)dst;

    while (len >= 16) {
        len -= 16;
        __m128i aa = _mm_loadu_si128((const __m128i*)srcPos);
        __m128i bb = _mm_loadu_si128((const __m128i*)currPos);

        __m128i cc = _mm_adds_epi16(aa, bb);
        _mm_storeu_si128(currPos, cc);

        currPos += 8;
        srcPos += 8;
    }

    if (len != 0) {
        Mix_Mixer_AUDIOS16(currPos, srcPos, len);
    }
}
#endif // __SSE2__
#ifdef __AVX__
void Mix_Mixer_AUDIOS16_AVX(void* dst, const void* src, unsigned len)
{
    const Sint16* srcPos = (const Sint16*)src;
    Sint16* currPos = (Sint16*)dst;

    while (len >= 32) {
        len -= 32;
        __m256i aa = _mm256_loadu_si256((const __m256i*)srcPos);
        __m256i bb = _mm256_loadu_si256((const __m256i*)currPos);

        __m256i cc = _mm256_adds_epi16(aa, bb);
        _mm_storeu_si256(currPos, cc);

        currPos += 16;
        srcPos += 16;
    }

    if (len != 0) {
        Mix_Mixer_AUDIOS16(currPos, srcPos, len);
    }
}
#endif // __AVX__
#endif // FULL - SELF_MIX

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
        __m256i aa = _mm256_loadu_si256((const __m256i*)srcPos);
        __m256i bb = _mm256_unpackhi_epi16(aa, aa);
        _mm256_storeu_si256(dstPos, bb);

        dstPos -= 16;
        __m256i cc = _mm256_unpacklo_epi16(aa, aa);
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
        __m128i aa = _mm_loadu_si128((const __m128i*)srcPos);
        __m128i bb = _mm_unpackhi_epi16(aa, aa);
        _mm_storeu_si128(dstPos, bb);

        dstPos -= 8;
        __m128i cc = _mm_unpacklo_epi16(aa, aa);
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

#ifdef __SSE2__
void Mix_Converter_AUDIO8_Resample_Half_SSE2(Mix_BuffOps* buf)
{
    Uint8* srcPos = (Uint8*)buf->currPos;
    Uint8* dstPos = srcPos;

    Uint8* endPos = (Uint8*)buf->endPos;

    endPos = srcPos + (endPos - srcPos) / 2;
    buf->endPos = endPos;

    while (dstPos <= endPos - 16) {
        __m128i aa = _mm_loadu_si128((const __m128i*)srcPos);
        __m128i bb = _mm_loadu_si128((const __m128i*)&srcPos[16]);
        //aa = _mm_slli_epi16(aa, 8);
        aa = _mm_srli_epi16(aa, 8);
        //bb = _mm_slli_epi16(bb, 8);
        bb = _mm_srli_epi16(bb, 8);
        __m128i cc = _mm_packus_epi16(aa, bb);
        _mm_storeu_si128((__m128i*)dstPos, cc);

        dstPos += 16;
        srcPos += 32;
    }

    while (dstPos != endPos) {
        *dstPos = *srcPos;
        dstPos++;
        srcPos += 2;
    }
}
#endif
void Mix_Converter_AUDIO8_Resample_Half(Mix_BuffOps* buf)
{
    Uint8* srcPos = (Uint8*)buf->currPos;
    Uint8* dstPos = srcPos;

    Uint8* endPos = (Uint8*)buf->endPos;

    endPos = srcPos + (endPos - srcPos) / 2;
    buf->endPos = endPos;

    while (dstPos != endPos) {
        *dstPos = *srcPos;
        dstPos++;
        srcPos += 2;
    }
}

#ifdef __SSE2__
void Mix_Converter_AUDIO16_Resample_Half_SSE2(Mix_BuffOps* buf)
{
    Sint16* srcPos = (Sint16*)buf->currPos;
    Sint16* dstPos = srcPos;

    Sint16* endPos = (Sint16*)buf->endPos;

    endPos = srcPos + (endPos - srcPos) / 2;
    buf->endPos = endPos;

    while (dstPos <= endPos - 8) {
        __m128i aa = _mm_loadu_si128((const __m128i*)srcPos);
        __m128i bb = _mm_loadu_si128((const __m128i*)&srcPos[8]);

        aa = _mm_shufflehi_epi16(aa, (0 << 0) | (2 << 2) | (1 << 4) | (3 << 6));
        aa = _mm_shufflelo_epi16(aa, (0 << 0) | (2 << 2) | (1 << 4) | (3 << 6));
        aa = _mm_shuffle_epi32(aa, (0 << 0) | (2 << 2) | (1 << 4) | (3 << 6));

        bb = _mm_shufflehi_epi16(bb, (0 << 0) | (2 << 2) | (1 << 4) | (3 << 6));
        bb = _mm_shufflelo_epi16(bb, (0 << 0) | (2 << 2) | (1 << 4) | (3 << 6));
        bb = _mm_shuffle_epi32(bb, (0 << 0) | (2 << 2) | (1 << 4) | (3 << 6));

        __m128i cc = _mm_unpacklo_epi64(aa, bb);
        _mm_storeu_si128((__m128i*)dstPos, cc);

        dstPos += 8;
        srcPos += 16;
    }

    while (dstPos != endPos) {
        *dstPos = *srcPos;
        dstPos++;
        srcPos += 2;
    }
}
#endif
void Mix_Converter_AUDIO16_Resample_Half(Mix_BuffOps* buf)
{
    Sint16* srcPos = (Sint16*)buf->currPos;
    Sint16* dstPos = srcPos;

    Sint16* endPos = (Sint16*)buf->endPos;

    endPos = srcPos + (endPos - srcPos) / 2;
    buf->endPos = endPos;

    while (dstPos != endPos) {
        *dstPos = *srcPos;
        dstPos++;
        srcPos += 2;
    }
}

#ifdef __AVX__
void Mix_Converter_U8_S16LSB_AVX(Mix_BuffOps* buf)
{
    Uint8* srcPos = (Uint8*)buf->endPos;
    Uint8* currPos = (Uint8*)buf->currPos;

    Uint8* dstPos = srcPos + (srcPos - currPos);
    buf->endPos = dstPos;

    const __m256i zero = _mm256_setzero_si256();
    const __m256i sub = _mm256_set1_epi16(128);

    while (&currPos[32] <= srcPos) {
        srcPos -= 32;
        dstPos -= 32;

        __m256i aa = _mm256_loadu_si256((const __m256i*)srcPos);

        __m256i bb = _mm256_unpackhi_epi8(aa, zero);// zfill(a[hi], 16)
        bb = _mm256_sub_epi16(bb, sub);				// subtract 128

        bb = _mm256_slli_epi16(bb, 8);				// shl 8
        _mm256_storeu_si256((__m256i*)dstPos, bb);

        dstPos -= 32;
        __m256i cc = _mm256_unpacklo_epi8(aa, zero);// zfill(a[lo], 16)
        cc = _mm256_sub_epi16(cc, sub);				// subtract 128

        cc = _mm256_slli_epi16(cc, 8);				// shl 8
        _mm256_storeu_si256((__m256i*)dstPos, cc);
    }

    while (srcPos != currPos) {
        srcPos--;
        dstPos -= 2;
        *(Sint16*)dstPos = SDL_SwapLE16((*srcPos - 128) << 8);
    }
}
#endif // __AVX__
#ifdef __SSE2__
void Mix_Converter_U8_S16LSB_SSE2(Mix_BuffOps* buf)
{
    Uint8* srcPos = (Uint8*)buf->endPos;
    Uint8* currPos = (Uint8*)buf->currPos;

    Uint8* dstPos = srcPos + (srcPos - currPos);
    buf->endPos = dstPos;

    const __m128i zero = _mm_setzero_si128();
    const __m128i sub = _mm_set1_epi16(128);

    while (&currPos[16] <= srcPos) {
        srcPos -= 16;
        dstPos -= 16;

        __m128i aa = _mm_loadu_si128((const __m128i*)srcPos);

        __m128i bb = _mm_unpackhi_epi8(aa, zero);	// zfill(a[hi], 16)
        bb = _mm_sub_epi16(bb, sub);				// subtract 128

        bb = _mm_slli_epi16(bb, 8);					// shl 8
        _mm_storeu_si128((__m128i*)dstPos, bb);

        dstPos -= 16;
        __m128i cc = _mm_unpacklo_epi8(aa, zero);	// zfill(a[lo], 16)
        cc = _mm_sub_epi16(cc, sub);				// subtract 128

        cc = _mm_slli_epi16(cc, 8);					// shl 8
        _mm_storeu_si128((__m128i*)dstPos, cc);
    }

    while (srcPos != currPos) {
        srcPos--;
        dstPos -= 2;
        *(Sint16*)dstPos = SDL_SwapLE16((*srcPos - 128) << 8);
    }
}
#endif
void Mix_Converter_U8_S16LSB(Mix_BuffOps* buf)
{
    Uint8* srcPos = (Uint8*)buf->endPos;
    Uint8* currPos = (Uint8*)buf->currPos;

    Uint8* dstPos = srcPos + (srcPos - currPos);
    buf->endPos = dstPos;

    while (srcPos != currPos) {
        srcPos--;
        dstPos -= 2;
        *(Sint16*)dstPos = SDL_SwapLE16((*srcPos - 128) << 8);
    }
}

void Mix_Utils_Init()
{
#ifdef __SSE2__
    if (SDL_HasSSE2()) {
        //Mix_Convert_AUDIO8_Mono2Stereo = Mix_Converter_AUDIO8_Mono2Stereo_SSE2;
        Mix_Convert_AUDIO16_Mono2Stereo = Mix_Converter_AUDIO16_Mono2Stereo_SSE2;
        Mix_Convert_AUDIO8_Resample_Half = Mix_Converter_AUDIO8_Resample_Half_SSE2;
        Mix_Convert_AUDIO16_Resample_Half = Mix_Converter_AUDIO16_Resample_Half_SSE2;
        Mix_MixAudioFormat = Mix_Mixer_AUDIOS16_SSE2;
        Mix_Convert_U8_S16LSB = Mix_Converter_U8_S16LSB_SSE2;
    }
#endif
#if defined(__AVX__) && SDL_VERSION_ATLEAST(2, 0, 2)
    if (SDL_HasAVX()) {
        Mix_Convert_AUDIO16_Mono2Stereo = Mix_Converter_AUDIO16_Mono2Stereo_AVX;
        Mix_MixAudioFormat = Mix_Mixer_AUDIOS16_AVX;
        Mix_Convert_U8_S16LSB = Mix_Converter_U8_S16LSB_AVX;
    }
#endif
}

void Mix_BuildAudioCVT(Mix_Audio* audio)
{
    Mix_AudioSpec* audioSpec = &audio->asWAV.spec; // WAV_SRC
    int index = 0;
    unsigned mpl = 2;

    if (audioSpec->freqMpl == 2) {
        // assert(audioSpec->channels == 1);
        if (audioSpec->format == AUDIO_U8) {
            audio->converters[index] = Mix_Convert_AUDIO8_Resample_Half;
        } else {
            // assert(audioSpec->format == AUDIO_S16LSB);
            audio->converters[index] = Mix_Convert_AUDIO16_Resample_Half;
        }
        index++;
        mpl = 1;
    }
    if (audioSpec->format == AUDIO_U8) {
        audio->converters[index] = Mix_Convert_U8_S16LSB;
        index++;
        mpl *= 2;
    }
    if (audioSpec->channels == 1) {
        // assert(SDL_AUDIO_BITSIZE(audioSpec.format) == 16);
        audio->converters[index] = Mix_Convert_AUDIO16_Mono2Stereo;
        index++;
        mpl *= 2;
    }
    if (index < SDL_arraysize(audio->converters))
        audio->converters[index] = NULL;
    // assert(mpl >= 2 && mpl % 2 == 0);
    // assert(audioSpec->freqMpl == 1 || audioSpec->freqMpl == 2);
    // assert(audioSpec->format == AUDIO_U8 || audioSpec->freqMpl == AUDIO_16);
    // assert(audioSpec->channels == 1 || audioSpec->channels == 2);
    audio->convMpl = mpl / 2;
}

void Mix_ConvertAudio(Mix_Audio* audio, Mix_BuffOps* buffOps)
{
    void (**converters)(Mix_BuffOps* buf) = &audio->converters[0];

    if (converters[0] == NULL)
        return;
    converters[0](buffOps);

    if (converters[1] == NULL)
        return;
    converters[1](buffOps);

    if (converters[2] == NULL)
        return;
    converters[2](buffOps);
}

#endif // FULL - SELF_CONV

#if DEBUG_MODE && !SDL_VERSION_ATLEAST(2, 0, 0) // USE_SDL1
int Mix_SetError(const char* msg, ...)
{
	va_list va;

	va_start(va, msg);

	SDL_SetError(msg, va);

	va_end(va);

	return -1;
}
#endif
