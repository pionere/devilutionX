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

  This file by Ryan C. Gordon (icculus@icculus.org)

  These are some internally supported special effects that use SDL_mixer's
  effect callback API. They are meant for speed over quality.  :)
*/

#include "SDL_cpuinfo.h"
#include "SDL_endian.h"
#include "SDL_mixer.h"

#include "mixer.h"

#define MIX_INTERNAL_EFFECT__
#include "effects_internal.h"

/* profile code:
    #include <sys/time.h>
    #include <unistd.h>
    struct timeval tv1;
    struct timeval tv2;
    
    gettimeofday(&tv1, NULL);

        ... do your thing here ...

    gettimeofday(&tv2, NULL);
    printf("%ld\n", tv2.tv_usec - tv1.tv_usec);
*/


/*
 * Positional effects...panning, distance attenuation, etc.
 */

#ifdef FULL // FIX_EFF
static _Mix_EffectPosArgs **pos_args_array = NULL;
#ifdef FULL // EFF_CHECK
static _Mix_EffectPosArgs *pos_args_global = NULL;
#endif
static int position_channels = 0;
#endif // FULL - FIX_EFF

/* Function pointers set to a CPU-specific implementation. */
static SDL_bool (*_Eff_do_volume_s16lbs)(void* stream, unsigned len, void* udata) = NULL;
static SDL_bool (*_Eff_do_position_s16lsb)(void* stream, unsigned len, void* udata) = NULL;

#ifdef FULL // FIX_EFF
/* This just frees up the callback-specific data. */
static void SDLCALL _Eff_PositionDone(int channel, void *udata)
{
    (void)udata;
#ifdef FULL // EFF_CHECK
    if (channel < 0) {
        if (pos_args_global != NULL) {
            SDL_free(pos_args_global);
            pos_args_global = NULL;
        }
    }
    else if (pos_args_array[channel] != NULL) {
#else
    if (pos_args_array[channel] != NULL) {
#endif
        SDL_free(pos_args_array[channel]);
        pos_args_array[channel] = NULL;
    }
}
#endif // FULL - FIX_EFF
#ifdef FULL // FIX_OUT
static void SDLCALL _Eff_position_u8(int chan, void *stream, int len, void *udata)
{
    Uint8 *ptr = (Uint8 *) stream;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Uint16) != 0) {
        *ptr = (Uint8) (((float) *ptr) * dist_f);
        ptr++;
        len--;
    }

    if (((_Mix_EffectPosArgs *)udata)->room_angle == 180)
    for (i = 0; i < len; i += sizeof (Uint8) * 2) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * right_f) * dist_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * left_f) * dist_f) + 128);
        ptr++;
    }
    else for (i = 0; i < len; i += sizeof (Uint8) * 2) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * left_f) * dist_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * right_f) * dist_f) + 128);
        ptr++;
    }
}

static void SDLCALL _Eff_position_u8_c4(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Uint16) != 0) {
        *ptr = (Uint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    if (args->room_angle == 0)
    for (i = 0; i < len; i += sizeof (Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
    }
    else if (args->room_angle == 90)
    for (i = 0; i < len; i += sizeof (Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
    }
    else if (args->room_angle == 180)
    for (i = 0; i < len; i += sizeof (Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
    }
    else if (args->room_angle == 270)
    for (i = 0; i < len; i += sizeof (Uint8) * 4) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
    }
}

static void SDLCALL _Eff_position_u8_c6(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    int i;

    (void)chan;
    (void)len;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Uint16) != 0) {
        *ptr = (Uint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    if (args->room_angle == 0)
    for (i = 0; i < len; i += sizeof (Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->center_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
    }
    else if (args->room_angle == 90)
    for (i = 0; i < len; i += sizeof (Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f/2) + 128)
            + (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f/2) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
    }
    else if (args->room_angle == 180)
    for (i = 0; i < len; i += sizeof (Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f/2) + 128)
            + (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f/2) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
    }
    else if (args->room_angle == 270)
    for (i = 0; i < len; i += sizeof (Uint8) * 6) {
        /* must adjust the sample so that 0 is the center */
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_rear_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->right_f) * args->distance_f) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_f) * args->distance_f/2) + 128)
            + (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->left_rear_f) * args->distance_f/2) + 128);
        ptr++;
        *ptr = (Uint8) ((Sint8) ((((float) (Sint8) (*ptr - 128))
            * args->lfe_f) * args->distance_f) + 128);
        ptr++;
    }
}


/*
 * This one runs about 10.1 times faster than the non-table version, with
 *  no loss in quality. It does, however, require 64k of memory for the
 *  lookup table. Also, this will only update position information once per
 *  call; the non-table version always checks the arguments for each sample,
 *  in case the user has called Mix_SetPanning() or whatnot again while this
 *  callback is running.
 */
static void SDLCALL _Eff_position_table_u8(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Uint8 *ptr = (Uint8 *) stream;
    Uint32 *p;
    int i;
    Uint8 *l = ((Uint8 *) _Eff_volume_table) + (256 * args->left_u8);
    Uint8 *r = ((Uint8 *) _Eff_volume_table) + (256 * args->right_u8);
    Uint8 *d = ((Uint8 *) _Eff_volume_table) + (256 * args->distance_u8);

    (void)chan;

    if (args->room_angle == 180) {
        Uint8 *temp = l;
        l = r;
        r = temp;
    }
    /*
     * if there's only a mono channnel, then l[] and r[] are always
     *  volume 255, and are therefore throwaways. Still, we have to
     *  be sure not to overrun the audio buffer...
     */
    while (len % (int)sizeof(Uint32) != 0) {
        *ptr = d[l[*ptr]];
        ptr++;
        if (args->channels > 1) {
            *ptr = d[r[*ptr]];
            ptr++;
        }
        len -= args->channels;
    }

    p = (Uint32 *) ptr;

    for (i = 0; i < len; i += sizeof (Uint32)) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        *p = (d[l[(*p & 0xFF000000) >> 24]] << 24) |
             (d[r[(*p & 0x00FF0000) >> 16]] << 16) |
             (d[l[(*p & 0x0000FF00) >>  8]] <<  8) |
             (d[r[(*p & 0x000000FF)      ]]      ) ;
#else
        *p = (d[r[(*p & 0xFF000000) >> 24]] << 24) |
             (d[l[(*p & 0x00FF0000) >> 16]] << 16) |
             (d[r[(*p & 0x0000FF00) >>  8]] <<  8) |
             (d[l[(*p & 0x000000FF)      ]]      ) ;
#endif
        ++p;
    }
}


static void SDLCALL _Eff_position_s8(int chan, void *stream, int len, void *udata)
{
    Sint8 *ptr = (Sint8 *) stream;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Sint16) != 0) {
        *ptr = (Sint8) (((float) *ptr) * dist_f);
        ptr++;
        len--;
    }

    if (((_Mix_EffectPosArgs *)udata)->room_angle == 180)
    for (i = 0; i < len; i += sizeof (Sint8) * 2) {
        *ptr = (Sint8)((((float) *ptr) * right_f) * dist_f);
        ptr++;
        *ptr = (Sint8)((((float) *ptr) * left_f) * dist_f);
        ptr++;
    }
    else
    for (i = 0; i < len; i += sizeof (Sint8) * 2) {
        *ptr = (Sint8)((((float) *ptr) * left_f) * dist_f);
        ptr++;
        *ptr = (Sint8)((((float) *ptr) * right_f) * dist_f);
        ptr++;
    }
}
static void SDLCALL _Eff_position_s8_c4(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Sint16) != 0) {
        *ptr = (Sint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    for (i = 0; i < len; i += sizeof (Sint8) * 4) {
      switch (args->room_angle) {
       case 0:
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        break;
       case 90:
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        break;
       case 180:
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        break;
       case 270:
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        break;
      }
    }
}

static void SDLCALL _Eff_position_s8_c6(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    int i;

    (void)chan;

    /*
     * if there's only a mono channnel (the only way we wouldn't have
     *  a len divisible by 2 here), then left_f and right_f are always
     *  1.0, and are therefore throwaways.
     */
    if (len % (int)sizeof(Sint16) != 0) {
        *ptr = (Sint8) (((float) *ptr) * args->distance_f);
        ptr++;
        len--;
    }

    for (i = 0; i < len; i += sizeof (Sint8) * 6) {
      switch (args->room_angle) {
       case 0:
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->center_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
       case 90:
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f / 2)
           + (Sint8)((((float) *ptr) * args->right_f) * args->distance_f / 2); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
       case 180:
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f / 2)
           + (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f / 2); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
       case 270:
        *ptr = (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_rear_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->right_f) * args->distance_f); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->left_f) * args->distance_f / 2)
           + (Sint8)((((float) *ptr) * args->left_rear_f) * args->distance_f / 2); ptr++;
        *ptr = (Sint8)((((float) *ptr) * args->lfe_f) * args->distance_f); ptr++;
        break;
      }
    }
}

/*
 * This one runs about 10.1 times faster than the non-table version, with
 *  no loss in quality. It does, however, require 64k of memory for the
 *  lookup table. Also, this will only update position information once per
 *  call; the non-table version always checks the arguments for each sample,
 *  in case the user has called Mix_SetPanning() or whatnot again while this
 *  callback is running.
 */
static void SDLCALL _Eff_position_table_s8(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint8 *ptr = (Sint8 *) stream;
    Uint32 *p;
    int i;
    Sint8 *l = ((Sint8 *) _Eff_volume_table) + (256 * args->left_u8);
    Sint8 *r = ((Sint8 *) _Eff_volume_table) + (256 * args->right_u8);
    Sint8 *d = ((Sint8 *) _Eff_volume_table) + (256 * args->distance_u8);

    (void)chan;

    if (args->room_angle == 180) {
        Sint8 *temp = l;
        l = r;
        r = temp;
    }

    while (len % (int)sizeof(Uint32) != 0) {
        *ptr = d[l[*ptr]];
        ptr++;
        if (args->channels > 1) {
            *ptr = d[r[*ptr]];
            ptr++;
        }
        len -= args->channels;
    }

    p = (Uint32 *) ptr;

    for (i = 0; i < len; i += sizeof (Uint32)) {
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        *p = (d[l[((Sint16)(Sint8)((*p & 0xFF000000) >> 24))+128]] << 24) |
             (d[r[((Sint16)(Sint8)((*p & 0x00FF0000) >> 16))+128]] << 16) |
             (d[l[((Sint16)(Sint8)((*p & 0x0000FF00) >>  8))+128]] <<  8) |
             (d[r[((Sint16)(Sint8)((*p & 0x000000FF)      ))+128]]      ) ;
#else
        *p = (d[r[((Sint16)(Sint8)((*p & 0xFF000000) >> 24))+128]] << 24) |
             (d[l[((Sint16)(Sint8)((*p & 0x00FF0000) >> 16))+128]] << 16) |
             (d[r[((Sint16)(Sint8)((*p & 0x0000FF00) >>  8))+128]] <<  8) |
             (d[l[((Sint16)(Sint8)((*p & 0x000000FF)      ))+128]]      ) ;
#endif
        ++p;
    }
}


/* !!! FIXME : Optimize the code for 16-bit samples? */

static void SDLCALL _Eff_position_u16lsb(int chan, void *stream, int len, void *udata)
{
    Uint16 *ptr = (Uint16 *) stream;
    const SDL_bool opp = ((_Mix_EffectPosArgs *)udata)->room_angle == 180 ? SDL_TRUE : SDL_FALSE;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Uint16) * 2) {
        Sint16 sampl = (Sint16) (SDL_SwapLE16(*(ptr+0)) - 32768);
        Sint16 sampr = (Sint16) (SDL_SwapLE16(*(ptr+1)) - 32768);

        Uint16 swapl = (Uint16) ((Sint16) (((float) sampl * left_f)
                                    * dist_f) + 32768);
        Uint16 swapr = (Uint16) ((Sint16) (((float) sampr * right_f)
                                    * dist_f) + 32768);

        if (opp) {
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
        }
        else {
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
        }
    }
}

static void SDLCALL _Eff_position_u16lsb_c4(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Uint16 *ptr = (Uint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Uint16) * 4) {
        Sint16 sampl = (Sint16) (SDL_SwapLE16(*(ptr+0)) - 32768);
        Sint16 sampr = (Sint16) (SDL_SwapLE16(*(ptr+1)) - 32768);
        Sint16 samplr = (Sint16) (SDL_SwapLE16(*(ptr+2)) - 32768);
        Sint16 samprr = (Sint16) (SDL_SwapLE16(*(ptr+3)) - 32768);

        Uint16 swapl = (Uint16) ((Sint16) (((float) sampl * args->left_f)
                                    * args->distance_f) + 32768);
        Uint16 swapr = (Uint16) ((Sint16) (((float) sampr * args->right_f)
                                    * args->distance_f) + 32768);
        Uint16 swaplr = (Uint16) ((Sint16) (((float) samplr * args->left_rear_f)
                                    * args->distance_f) + 32768);
        Uint16 swaprr = (Uint16) ((Sint16) (((float) samprr * args->right_rear_f)
                                    * args->distance_f) + 32768);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            break;
        case 90:
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            break;
        case 180:
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            break;
        case 270:
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_u16lsb_c6(int chan, void *stream, int len, void *udata)
{
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Uint16 *ptr = (Uint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Uint16) * 6) {
        Sint16 sampl = (Sint16) (SDL_SwapLE16(*(ptr+0)) - 32768);
        Sint16 sampr = (Sint16) (SDL_SwapLE16(*(ptr+1)) - 32768);
        Sint16 samplr = (Sint16) (SDL_SwapLE16(*(ptr+2)) - 32768);
        Sint16 samprr = (Sint16) (SDL_SwapLE16(*(ptr+3)) - 32768);
        Sint16 sampce = (Sint16) (SDL_SwapLE16(*(ptr+4)) - 32768);
        Sint16 sampwf = (Sint16) (SDL_SwapLE16(*(ptr+5)) - 32768);

        Uint16 swapl = (Uint16) ((Sint16) (((float) sampl * args->left_f)
                                    * args->distance_f) + 32768);
        Uint16 swapr = (Uint16) ((Sint16) (((float) sampr * args->right_f)
                                    * args->distance_f) + 32768);
        Uint16 swaplr = (Uint16) ((Sint16) (((float) samplr * args->left_rear_f)
                                    * args->distance_f) + 32768);
        Uint16 swaprr = (Uint16) ((Sint16) (((float) samprr * args->right_rear_f)
                                    * args->distance_f) + 32768);
        Uint16 swapce = (Uint16) ((Sint16) (((float) sampce * args->center_f)
                                    * args->distance_f) + 32768);
        Uint16 swapwf = (Uint16) ((Sint16) (((float) sampwf * args->lfe_f)
                                    * args->distance_f) + 32768);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapce);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapwf);
            break;
        case 90:
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr)/2 + (Uint16) SDL_SwapLE16(swaprr)/2;
            *(ptr++) = (Uint16) SDL_SwapLE16(swapwf);
            break;
        case 180:
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr)/2 + (Uint16) SDL_SwapLE16(swaplr)/2;
            *(ptr++) = (Uint16) SDL_SwapLE16(swapwf);
            break;
        case 270:
            *(ptr++) = (Uint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapLE16(swapl)/2 + (Uint16) SDL_SwapLE16(swaplr)/2;
            *(ptr++) = (Uint16) SDL_SwapLE16(swapwf);
            break;
        }
    }
}
#endif // FULL
#ifdef FULL // FIX_EFF
static void SDLCALL _Eff_position_s16lsb(int chan, void *stream, int len, void *udata)
#else
SDL_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_MIX_MAX_VOLUME, SDL_MAX_SINT16 * MIX_MAX_POS_EFFECT * MIX_MAX_VOLUME <= SDL_MAX_SINT32);
#define ADJUST_SIDE_VOLUME(s, v) (s = (s*v)/(MIX_MAX_VOLUME * MIX_MAX_POS_EFFECT))
static SDL_bool _Eff_position_s16lsb(void* stream, unsigned len, void* udata)
#endif
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16* ptr = (Sint16*)stream;
#ifdef FULL // FIX_OUT
    const SDL_bool opp = ((_Mix_EffectPosArgs *)udata)->room_angle == 180 ? SDL_TRUE : SDL_FALSE;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
#endif
#ifdef FULL // FIX_EFF
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    (void)chan;
#else
    Mix_Channel* channel = (Mix_Channel*)udata;
    //const float left_f = channel->effect.left_f * channel->volume / MIX_MAX_VOLUME;
    //const float right_f = channel->effect.right_f * channel->volume / MIX_MAX_VOLUME;
    //if (left_f < 1.0f / SDL_MAX_SINT16 && right_f < 1.0f / SDL_MAX_SINT16)
    //    return SDL_FALSE;
    int voll, volr;
    const int volume = ((Mix_Channel*)udata)->volume;
    const int left = channel->effect.left_vol * volume;
    const int right = channel->effect.right_vol * volume;
    if (volume == 0)
        return SDL_FALSE;
#endif
#if 0
    if (len % (int)(sizeof(Sint16) * 2)) {
        fprintf(stderr,"Not an even number of frames! len=%d\n", len);
        return SDL_FALSE;
    }
#endif

    len /= sizeof(Sint16) * 2;
    while (len--) {
#ifdef FULL // FIX_OUT
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+1))) *
                                    right_f) * dist_f);
        if (opp) {
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
        }
        else {
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
        }
#else
        /*Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+0))) *
                                    left_f));
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+1))) *
                                    right_f));
        //if (opp) {
        //    *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
        //    *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
        //}
        //else {
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
        //}*/
        voll = SDL_SwapLE16(*ptr);
        ADJUST_SIDE_VOLUME(voll, left);
        *ptr = (Sint16) SDL_SwapLE16(voll);
        ptr++;
        volr = SDL_SwapLE16(*ptr);
        ADJUST_SIDE_VOLUME(volr, right);
        *ptr = (Sint16) SDL_SwapLE16(volr);
        ptr++;
#endif
    }
    return SDL_TRUE;
}
#ifdef SDL_SSE2_INTRINSICS
static SDL_bool SDL_TARGETING("sse2") _Eff_position_s16lsb_SSE2(void* stream, unsigned len, void* udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16* ptr = (Sint16*)stream;
    Mix_Channel* channel = (Mix_Channel*)udata;
    const int volume = ((Mix_Channel*)udata)->volume;
    int left = channel->effect.left_vol * volume;
    int right = channel->effect.right_vol * volume;
    if (volume == 0)
        return SDL_FALSE;

#if 1
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_SSE2_MIX_MAX_VOLUME, (MIX_MAX_VOLUME & (MIX_MAX_VOLUME - 1)) == 0);
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_SSE2_MIX_MAX_POS_EFFECT, (MIX_MAX_POS_EFFECT & (MIX_MAX_POS_EFFECT - 1)) == 0);
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_SSE2_low, (MIX_MAX_VOLUME * MIX_MAX_POS_EFFECT) <= (1 << 15));
    left *= (1 << 15) / (MIX_MAX_VOLUME * MIX_MAX_POS_EFFECT);
    right *= (1 << 15) / (MIX_MAX_VOLUME * MIX_MAX_POS_EFFECT);
    if ((int16_t)left < 0) {
        left = INT16_MAX;
    }
    if ((int16_t)right < 0) {
        right = INT16_MAX;
    }
#endif
    __m128i mm = _mm_set1_epi32(left | (right << 16));

    while (len >= 16) {
        len -= 16;
        __m128i aa = _mm_loadu_si128((const __m128i*)ptr);
#if 1
        __m128i sm = _mm_mulhi_epi16(aa, mm);
        __m128i cc = _mm_slli_epi16(sm, 1);
#else
        __m128i lo = _mm_mullo_epi16(aa, mm);
        __m128i hi = _mm_mulhi_epi16(aa, mm);
        __m128i c0 = _mm_unpacklo_epi16(lo, hi);
        __m128i c1 = _mm_unpackhi_epi16(lo, hi);
        __m128i d0 = _mm_srai_epi32(c0, 10 + 4);
        __m128i d1 = _mm_srai_epi32(c1, 10 + 4);
        __m128i cc = _mm_packs_epi32(d0, d1);
#endif
        _mm_storeu_si128((__m128i*)ptr, cc);
        ptr += 8;
    }

    if (len != 0) {
        _Eff_position_s16lsb(ptr, len, channel);
    }
    return SDL_TRUE;
}
#endif // SDL_SSE2_INTRINSICS
#ifdef SDL_AVX2_INTRINSICS
static SDL_bool SDL_TARGETING("avx2") _Eff_position_s16lsb_AVX2(void* stream, unsigned len, void* udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16* ptr = (Sint16*)stream;
    Mix_Channel* channel = (Mix_Channel*)udata;
    const int volume = ((Mix_Channel*)udata)->volume;
    int left = channel->effect.left_vol * volume;
    int right = channel->effect.right_vol * volume;
    if (volume == 0)
        return SDL_FALSE;

#if 1
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_AVX2_MIX_MAX_VOLUME, (MIX_MAX_VOLUME & (MIX_MAX_VOLUME - 1)) == 0);
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_AVX2_MIX_MAX_POS_EFFECT, (MIX_MAX_POS_EFFECT & (MIX_MAX_POS_EFFECT - 1)) == 0);
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_AVX2_low, (MIX_MAX_VOLUME * MIX_MAX_POS_EFFECT) <= (1 << 15));
    left *= (1 << 15) / (MIX_MAX_VOLUME * MIX_MAX_POS_EFFECT);
    right *= (1 << 15) / (MIX_MAX_VOLUME * MIX_MAX_POS_EFFECT);
    if ((int16_t)left < 0) {
        left = INT16_MAX;
    }
    if ((int16_t)right < 0) {
        right = INT16_MAX;
    }
#else
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_AVX2_MIX_MAX_VOLUME, MIX_MAX_VOLUME == (1 << 10));
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_position_s16lsb_AVX2_MIX_MAX_POS_EFFECT, MIX_MAX_POS_EFFECT == (1 << 4));
#endif

    __m256i mm = _mm256_set1_epi32(left | (right << 16));

    while (len >= 32) {
        len -= 32;
        __m256i aa = _mm256_loadu_si256((const __m256i*)ptr);
#if 1
        __m256i sm = _mm256_mulhi_epi16(aa, mm);
        __m256i cc = _mm256_slli_epi16(sm, 1);
#else
        __m256i lo = _mm256_mullo_epi16(aa, mm);
        __m256i hi = _mm256_mulhi_epi16(aa, mm);
        __m256i c0 = _mm256_unpacklo_epi16(lo, hi);
        __m256i c1 = _mm256_unpackhi_epi16(lo, hi);
        __m256i d0 = _mm256_srai_epi32(c0, 10 + 4);
        __m256i d1 = _mm256_srai_epi32(c1, 10 + 4);
        __m256i cc = _mm256_packs_epi32(d0, d1);
#endif
        _mm256_storeu_si256((__m256i*)ptr, cc);
        ptr += 16;
    }

    if (len != 0) {
        _Eff_position_s16lsb(ptr, len, channel);
    }
    return SDL_TRUE;
}
#endif // SDL_AVX2_INTRINSICS
#define ADJUST_VOLUME(s, v) (s = (s*v)/MIX_MAX_VOLUME)
static SDL_bool _Eff_volume_s16lbs(void* stream, unsigned len, void* udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16* ptr = (Sint16*)stream;
    int vol, volume = ((Mix_Channel*)udata)->volume;
    if (volume == 0)
        return SDL_FALSE;
    if (volume == MIX_MAX_VOLUME)
        return SDL_TRUE;
    len /= sizeof(Sint16);
    while (len--) {
        vol = SDL_SwapLE16(*ptr);
        ADJUST_VOLUME(vol, volume);
        *ptr = SDL_SwapLE16(vol);
        ptr++;
    }
    return SDL_TRUE;
}
#ifdef SDL_SSE2_INTRINSICS
static SDL_bool SDL_TARGETING("sse2") _Eff_volume_s16lbs_SSE2(void* stream, unsigned len, void* udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16* ptr = (Sint16*)stream;
    int vol, volume = ((Mix_Channel*)udata)->volume;
    if (volume == 0)
        return SDL_FALSE;
    if (volume == MIX_MAX_VOLUME)
        return SDL_TRUE;

#if 1
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_volume_s16lbs_SSE2_MIX_MAX_VOLUME, (MIX_MAX_VOLUME & (MIX_MAX_VOLUME - 1)) == 0);
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_volume_s16lbs_SSE2_low, MIX_MAX_VOLUME <= (1 << 15));
    volume *= (1 << 15) / MIX_MAX_VOLUME;
#else
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_volume_s16lbs_SSE2_MIX_MAX_VOLUME, MIX_MAX_VOLUME == (1 << 10));
#endif
    __m128i mm = _mm_set1_epi16(volume);

    while (len >= 16) {
        len -= 16;
        __m128i aa = _mm_loadu_si128((const __m128i*)ptr);
#if 1
        __m128i sm = _mm_mulhi_epi16(aa, mm);
        __m128i cc = _mm_slli_epi16(sm, 1);
#else
        __m128i lo = _mm_mullo_epi16(aa, mm);
        __m128i hi = _mm_mulhi_epi16(aa, mm);
        __m128i c0 = _mm_unpacklo_epi16(lo, hi);
        __m128i c1 = _mm_unpackhi_epi16(lo, hi);
        __m128i d0 = _mm_srai_epi32(c0, 10);
        __m128i d1 = _mm_srai_epi32(c1, 10);
        __m128i cc = _mm_packs_epi32(d0, d1);
#endif
        _mm_storeu_si128((__m128i*)ptr, cc);
        ptr += 8;
    }
#if 1
    volume /= (unsigned)((1 << 15) / MIX_MAX_VOLUME);
#endif
    len /= sizeof(Sint16);
    while (len--) {
        vol = SDL_SwapLE16(*ptr);
        ADJUST_VOLUME(vol, volume);
        *ptr = SDL_SwapLE16(vol);
        ptr++;
    }
    return SDL_TRUE;
}
#endif // SDL_SSE2_INTRINSICS
#ifdef SDL_AVX2_INTRINSICS
static SDL_bool SDL_TARGETING("avx2") _Eff_volume_s16lbs_AVX2(void* stream, unsigned len, void* udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16* ptr = (Sint16*)stream;
    int vol, volume = ((Mix_Channel*)udata)->volume;
    if (volume == 0)
        return SDL_FALSE;
    if (volume == MIX_MAX_VOLUME)
        return SDL_TRUE;

#if 1
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_volume_s16lbs_AVX2_MIX_MAX_VOLUME, (MIX_MAX_VOLUME & (MIX_MAX_VOLUME - 1)) == 0);
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_volume_s16lbs_AVX2_low, MIX_MAX_VOLUME <= (1 << 15));
    volume *= (1 << 15) / MIX_MAX_VOLUME;
#else
    SDL_INLINE_COMPILE_TIME_ASSERT(_Eff_volume_s16lbs_AVX2_MIX_MAX_VOLUME, MIX_MAX_VOLUME == (1 << 10));
#endif
    __m256i mm = _mm256_set1_epi16(volume);

    while (len >= 32) {
        len -= 32;
        __m256i aa = _mm256_loadu_si256((const __m256i*)ptr);
#if 1
        __m256i sm = _mm256_mulhi_epi16(aa, mm);
        __m256i cc = _mm256_slli_epi16(sm, 1);
#else
        __m256i lo = _mm256_mullo_epi16(aa, mm);
        __m256i hi = _mm256_mulhi_epi16(aa, mm);
        __m256i c0 = _mm256_unpacklo_epi16(lo, hi);
        __m256i c1 = _mm256_unpackhi_epi16(lo, hi);
        __m256i d0 = _mm256_srai_epi32(c0, 10);
        __m256i d1 = _mm256_srai_epi32(c1, 10);
        __m256i cc = _mm256_packs_epi32(d0, d1);
#endif
        _mm256_storeu_si256((__m256i*)ptr, cc);
        ptr += 16;
    }
#if 1
    volume /= (unsigned)((1 << 15) / MIX_MAX_VOLUME);
#endif
    len /= sizeof(Sint16);
    while (len--) {
        vol = SDL_SwapLE16(*ptr);
        ADJUST_VOLUME(vol, volume);
        *ptr = SDL_SwapLE16(vol);
        ptr++;
    }
    return SDL_TRUE;
}
#endif // SDL_AVX2_INTRINSICS
#ifdef FULL // FIX_OUT
static void SDLCALL _Eff_position_s16lsb_c4(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 4 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 4) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+1))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+2))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s16lsb_c6(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 6 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 6) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint16 swapce = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint16 swapwf = (Sint16) ((((float) (Sint16) SDL_SwapLE16(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapce);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr)/2 + (Sint16) SDL_SwapLE16(swaprr)/2;
            *(ptr++) = (Sint16) SDL_SwapLE16(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr)/2 + (Sint16) SDL_SwapLE16(swaplr)/2;
            *(ptr++) = (Sint16) SDL_SwapLE16(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_SwapLE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapLE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapLE16(swapl)/2 + (Sint16) SDL_SwapLE16(swaplr)/2;
            *(ptr++) = (Sint16) SDL_SwapLE16(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_u16msb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Uint16 *ptr = (Uint16 *) stream;
    const SDL_bool opp = ((_Mix_EffectPosArgs *)udata)->room_angle == 180 ? SDL_TRUE : SDL_FALSE;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 2) {
        Sint16 sampl = (Sint16) (SDL_SwapBE16(*(ptr+0)) - 32768);
        Sint16 sampr = (Sint16) (SDL_SwapBE16(*(ptr+1)) - 32768);

        Uint16 swapl = (Uint16) ((Sint16) (((float) sampl * left_f)
                                    * dist_f) + 32768);
        Uint16 swapr = (Uint16) ((Sint16) (((float) sampr * right_f)
                                    * dist_f) + 32768);

        if (opp) {
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
        }
        else {
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
        }
    }
}

static void SDLCALL _Eff_position_u16msb_c4(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 4 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Uint16 *ptr = (Uint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 4) {
        Sint16 sampl = (Sint16) (SDL_SwapBE16(*(ptr+0)) - 32768);
        Sint16 sampr = (Sint16) (SDL_SwapBE16(*(ptr+1)) - 32768);
        Sint16 samplr = (Sint16) (SDL_SwapBE16(*(ptr+2)) - 32768);
        Sint16 samprr = (Sint16) (SDL_SwapBE16(*(ptr+3)) - 32768);

        Uint16 swapl = (Uint16) ((Sint16) (((float) sampl * args->left_f)
                                    * args->distance_f) + 32768);
        Uint16 swapr = (Uint16) ((Sint16) (((float) sampr * args->right_f)
                                    * args->distance_f) + 32768);
        Uint16 swaplr = (Uint16) ((Sint16) (((float) samplr * args->left_rear_f)
                                    * args->distance_f) + 32768);
        Uint16 swaprr = (Uint16) ((Sint16) (((float) samprr * args->right_rear_f)
                                    * args->distance_f) + 32768);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            break;
        case 90:
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            break;
        case 180:
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            break;
        case 270:
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_u16msb_c6(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 6 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Uint16 *ptr = (Uint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 6) {
        Sint16 sampl = (Sint16) (SDL_SwapBE16(*(ptr+0)) - 32768);
        Sint16 sampr = (Sint16) (SDL_SwapBE16(*(ptr+1)) - 32768);
        Sint16 samplr = (Sint16) (SDL_SwapBE16(*(ptr+2)) - 32768);
        Sint16 samprr = (Sint16) (SDL_SwapBE16(*(ptr+3)) - 32768);
        Sint16 sampce = (Sint16) (SDL_SwapBE16(*(ptr+4)) - 32768);
        Sint16 sampwf = (Sint16) (SDL_SwapBE16(*(ptr+5)) - 32768);

        Uint16 swapl = (Uint16) ((Sint16) (((float) sampl * args->left_f)
                                    * args->distance_f) + 32768);
        Uint16 swapr = (Uint16) ((Sint16) (((float) sampr * args->right_f)
                                    * args->distance_f) + 32768);
        Uint16 swaplr = (Uint16) ((Sint16) (((float) samplr * args->left_rear_f)
                                    * args->distance_f) + 32768);
        Uint16 swaprr = (Uint16) ((Sint16) (((float) samprr * args->right_rear_f)
                                    * args->distance_f) + 32768);
        Uint16 swapce = (Uint16) ((Sint16) (((float) sampce * args->center_f)
                                    * args->distance_f) + 32768);
        Uint16 swapwf = (Uint16) ((Sint16) (((float) sampwf * args->lfe_f)
                                    * args->distance_f) + 32768);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapce);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapwf);
            break;
        case 90:
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr)/2 + (Uint16) SDL_SwapBE16(swaprr)/2;
            *(ptr++) = (Uint16) SDL_SwapBE16(swapwf);
            break;
        case 180:
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr)/2 + (Uint16) SDL_SwapBE16(swaplr)/2;
            *(ptr++) = (Uint16) SDL_SwapBE16(swapwf);
            break;
        case 270:
            *(ptr++) = (Uint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Uint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Uint16) SDL_SwapBE16(swapl)/2 + (Uint16) SDL_SwapBE16(swaplr)/2;
            *(ptr++) = (Uint16) SDL_SwapBE16(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s16msb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    Sint16 *ptr = (Sint16 *) stream;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 2) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+1))) *
                                    right_f) * dist_f);
        *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
        *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
    }
}

static void SDLCALL _Eff_position_s16msb_c4(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 4 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 4) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s16msb_c6(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 6 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint16) * 6) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint16 swapce = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint16 swapwf = (Sint16) ((((float) (Sint16) SDL_SwapBE16(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapce);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr)/2 + (Sint16) SDL_SwapBE16(swaprr)/2;
            *(ptr++) = (Sint16) SDL_SwapBE16(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr)/2 + (Sint16) SDL_SwapBE16(swaplr)/2;
            *(ptr++) = (Sint16) SDL_SwapBE16(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint16) SDL_SwapBE16(swaplr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl);
            *(ptr++) = (Sint16) SDL_SwapBE16(swaprr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapr);
            *(ptr++) = (Sint16) SDL_SwapBE16(swapl)/2 + (Sint16) SDL_SwapBE16(swaplr)/2;
            *(ptr++) = (Sint16) SDL_SwapBE16(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32lsb(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 2 channels. */
    Sint32 *ptr = (Sint32 *) stream;
    const SDL_bool opp = ((_Mix_EffectPosArgs *)udata)->room_angle == 180 ? SDL_TRUE : SDL_FALSE;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

#if 0
    if (len % (int)(sizeof(Sint32) * 2)) {
        fprintf(stderr,"Not an even number of frames! len=%d\n", len);
        return;
    }
#endif

    for (i = 0; i < len; i += sizeof (Sint32) * 2) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+1))) *
                                    right_f) * dist_f);
        if (opp) {
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
        }
        else {
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
        }
    }
}

static void SDLCALL _Eff_position_s32lsb_c4(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 4 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint32) * 4) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+1))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+2))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32lsb_c6(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 6 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint32) * 6) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint32 swapce = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint32 swapwf = (Sint32) ((((float) (Sint32) SDL_SwapLE32(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapce);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr)/2 + (Sint32) SDL_SwapLE32(swaprr)/2;
            *(ptr++) = (Sint32) SDL_SwapLE32(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr)/2 + (Sint32) SDL_SwapLE32(swaplr)/2;
            *(ptr++) = (Sint32) SDL_SwapLE32(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_SwapLE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapLE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapLE32(swapl)/2 + (Sint32) SDL_SwapLE32(swaplr)/2;
            *(ptr++) = (Sint32) SDL_SwapLE32(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32msb(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 2 channels. */
    Sint32 *ptr = (Sint32 *) stream;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint32) * 2) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+0))) *
                                    left_f) * dist_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+1))) *
                                    right_f) * dist_f);
        *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
        *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
    }
}

static void SDLCALL _Eff_position_s32msb_c4(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 4 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint32) * 4) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            break;
        }
    }
}

static void SDLCALL _Eff_position_s32msb_c6(int chan, void *stream, int len, void *udata)
{
    /* 32 signed bits (lsb) * 6 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    Sint32 *ptr = (Sint32 *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (Sint32) * 6) {
        Sint32 swapl = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint32 swapr = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint32 swaplr = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint32 swaprr = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint32 swapce = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint32 swapwf = (Sint32) ((((float) (Sint32) SDL_SwapBE32(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapce);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapwf);
            break;
        case 90:
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr)/2 + (Sint32) SDL_SwapBE32(swaprr)/2;
            *(ptr++) = (Sint32) SDL_SwapBE32(swapwf);
            break;
        case 180:
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr)/2 + (Sint32) SDL_SwapBE32(swaplr)/2;
            *(ptr++) = (Sint32) SDL_SwapBE32(swapwf);
            break;
        case 270:
            *(ptr++) = (Sint32) SDL_SwapBE32(swaplr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl);
            *(ptr++) = (Sint32) SDL_SwapBE32(swaprr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapr);
            *(ptr++) = (Sint32) SDL_SwapBE32(swapl)/2 + (Sint32) SDL_SwapBE32(swaplr)/2;
            *(ptr++) = (Sint32) SDL_SwapBE32(swapwf);
            break;
        }
    }
}

static void SDLCALL _Eff_position_f32sys(int chan, void *stream, int len, void *udata)
{
    /* float * 2 channels. */
    float *ptr = (float *) stream;
    const float dist_f = ((_Mix_EffectPosArgs *)udata)->distance_f;
    const float left_f = ((_Mix_EffectPosArgs *)udata)->left_f;
    const float right_f = ((_Mix_EffectPosArgs *)udata)->right_f;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (float) * 2) {
        float swapl = ((*(ptr+0) * left_f) * dist_f);
        float swapr = ((*(ptr+1) * right_f) * dist_f);
        *(ptr++) = swapl;
        *(ptr++) = swapr;
    }
}

static void SDLCALL _Eff_position_f32sys_c4(int chan, void *stream, int len, void *udata)
{
    /* float * 4 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    float *ptr = (float *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (float) * 4) {
        float swapl = ((*(ptr+0) * args->left_f) * args->distance_f);
        float swapr = ((*(ptr+1) * args->right_f) * args->distance_f);
        float swaplr = ((*(ptr+2) * args->left_rear_f) * args->distance_f);
        float swaprr = ((*(ptr+3) * args->right_rear_f) * args->distance_f);
        switch (args->room_angle) {
        case 0:
            *(ptr++) = swapl;
            *(ptr++) = swapr;
            *(ptr++) = swaplr;
            *(ptr++) = swaprr;
            break;
        case 90:
            *(ptr++) = swapr;
            *(ptr++) = swaprr;
            *(ptr++) = swapl;
            *(ptr++) = swaplr;
            break;
        case 180:
            *(ptr++) = swaprr;
            *(ptr++) = swaplr;
            *(ptr++) = swapr;
            *(ptr++) = swapl;
            break;
        case 270:
            *(ptr++) = swaplr;
            *(ptr++) = swapl;
            *(ptr++) = swaprr;
            *(ptr++) = swapr;
            break;
        }
    }
}

static void SDLCALL _Eff_position_f32sys_c6(int chan, void *stream, int len, void *udata)
{
    /* float * 6 channels. */
    volatile _Mix_EffectPosArgs *args = (volatile _Mix_EffectPosArgs *) udata;
    float *ptr = (float *) stream;
    int i;

    (void)chan;

    for (i = 0; i < len; i += sizeof (float) * 6) {
        float swapl = ((*(ptr+0) * args->left_f) * args->distance_f);
        float swapr = ((*(ptr+1) * args->right_f) * args->distance_f);
        float swaplr = ((*(ptr+2) * args->left_rear_f) * args->distance_f);
        float swaprr = ((*(ptr+3) * args->right_rear_f) * args->distance_f);
        float swapce = ((*(ptr+4) * args->center_f) * args->distance_f);
        float swapwf = ((*(ptr+5) * args->lfe_f) * args->distance_f);

        switch (args->room_angle) {
        case 0:
            *(ptr++) = swapl;
            *(ptr++) = swapr;
            *(ptr++) = swaplr;
            *(ptr++) = swaprr;
            *(ptr++) = swapce;
            *(ptr++) = swapwf;
            break;
        case 90:
            *(ptr++) = swapr;
            *(ptr++) = swaprr;
            *(ptr++) = swapl;
            *(ptr++) = swaplr;
            *(ptr++) = swapr/2.0f + swaprr/2.0f;
            *(ptr++) = swapwf;
            break;
        case 180:
            *(ptr++) = swaprr;
            *(ptr++) = swaplr;
            *(ptr++) = swapr;
            *(ptr++) = swapl;
            *(ptr++) = swaprr/2.0f + swaplr/2.0f;
            *(ptr++) = swapwf;
            break;
        case 270:
            *(ptr++) = swaplr;
            *(ptr++) = swapl;
            *(ptr++) = swaprr;
            *(ptr++) = swapr;
            *(ptr++) = swapl/2.0f + swaplr/2.0f;
            *(ptr++) = swapwf;
            break;
        }
    }
}
#endif // FULL
#ifdef FULL // FIX_EFF
static void init_position_args(_Mix_EffectPosArgs *args)
{
#ifdef FULL
    SDL_memset(args, '\0', sizeof (_Mix_EffectPosArgs));
#endif
    args->in_use = 0;
#ifdef FULL // FIX_OUT
    static_assert(MIX_MAX_POS_EFFECT <= UCHAR_MAX, "Positional effects use BYTE fields.");
    args->room_angle = 0;
    args->left_u8 = args->right_u8 = args->distance_u8 = MIX_MAX_POS_EFFECT;
    args->left_f  = args->right_f  = args->distance_f  = 1.0f;
    args->left_rear_u8 = args->right_rear_u8 = args->center_u8 = args->lfe_u8 = MIX_MAX_POS_EFFECT;
    args->left_rear_f = args->right_rear_f = args->center_f = args->lfe_f = 1.0f;
    Mix_QuerySpec(NULL, NULL, (int *) &args->channels);
#else
    args->left_f  = args->right_f  = 1.0f;
#endif
}

static _Mix_EffectPosArgs *get_position_arg(int channel)
{
    void *rc;
    int i;
#ifdef FULL // EFF_CHECK
    if (channel < 0) {
        if (pos_args_global == NULL) {
            pos_args_global = SDL_malloc(sizeof (_Mix_EffectPosArgs));
            if (pos_args_global == NULL) {
                Mix_OutOfMemory();
                return(NULL);
            }
            init_position_args(pos_args_global);
        }

        return(pos_args_global);
    }
#endif
    if (channel >= position_channels) {
        rc = SDL_realloc(pos_args_array, (size_t)(channel + 1) * sizeof(_Mix_EffectPosArgs *));
        if (rc == NULL) {
            Mix_OutOfMemory();
            return(NULL);
        }
        pos_args_array = (_Mix_EffectPosArgs **) rc;
        for (i = position_channels; i <= channel; i++) {
            pos_args_array[i] = NULL;
        }
        position_channels = channel + 1;
    }

    if (pos_args_array[channel] == NULL) {
        pos_args_array[channel] = (_Mix_EffectPosArgs *)SDL_malloc(sizeof(_Mix_EffectPosArgs));
        if (pos_args_array[channel] == NULL) {
            Mix_OutOfMemory();
            return(NULL);
        }
        init_position_args(pos_args_array[channel]);
    }

    return(pos_args_array[channel]);
}

static Mix_EffectFunc_t get_position_effect_func(Uint16 format, int channels)
{
    Mix_EffectFunc_t f = NULL;

    switch (format) {
#ifdef FULL // FIX_OUT
        case AUDIO_U8:
            switch (channels) {
            case 1:
            case 2:
                f = (_Eff_build_volume_table_u8()) ? _Eff_position_table_u8 :
                                                     _Eff_position_u8;
                break;
            case 4:
                f = _Eff_position_u8_c4;
                break;
            case 6:
                f = _Eff_position_u8_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case AUDIO_S8:
            switch (channels) {
            case 1:
            case 2:
                f = (_Eff_build_volume_table_s8()) ? _Eff_position_table_s8 :
                                                     _Eff_position_s8;
                break;
            case 4:
                f = _Eff_position_s8_c4;
                break;
            case 6:
                f = _Eff_position_s8_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case AUDIO_U16LSB:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_u16lsb;
                break;
            case 4:
                f = _Eff_position_u16lsb_c4;
                break;
            case 6:
                f = _Eff_position_u16lsb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;
#endif // FIX_OUT
        case AUDIO_S16LSB:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s16lsb;
                break;
#ifdef FULL // FIX_OUT
            case 4:
                f = _Eff_position_s16lsb_c4;
                break;
            case 6:
                f = _Eff_position_s16lsb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;
        case AUDIO_U16MSB:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_u16msb;
                break;
            case 4:
                f = _Eff_position_u16msb_c4;
                break;
            case 6:
                f = _Eff_position_u16msb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case AUDIO_S16MSB:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s16msb;
                break;
            case 4:
                f = _Eff_position_s16msb_c4;
                break;
            case 6:
                f = _Eff_position_s16msb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case AUDIO_S32MSB:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s32msb;
                break;
            case 4:
                f = _Eff_position_s32msb_c4;
                break;
            case 6:
                f = _Eff_position_s32msb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case AUDIO_S32LSB:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_s32lsb;
                break;
            case 4:
                f = _Eff_position_s32lsb_c4;
                break;
            case 6:
                f = _Eff_position_s32lsb_c6;
                break;
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;

        case AUDIO_F32SYS:
            switch (channels) {
            case 1:
            case 2:
                f = _Eff_position_f32sys;
                break;
            case 4:
                f = _Eff_position_f32sys_c4;
                break;
            case 6:
                f = _Eff_position_f32sys_c6;
                break;
#endif // FIX_OUT
            default:
                Mix_SetError("Unsupported audio channels");
                break;
            }
            break;
        default:
            Mix_SetError("Unsupported audio format");
            break;
    }

    return(f);
}
#else // FIX_EFF
SDL_bool _Mix_DoEffects(void* buf, unsigned len, Mix_Channel* channel)
{
    // FIX_OUT
    // assert(MIX_DEFAULT_CHANNELS == 2, "_Mix_DoEffects does not pick its function dynamically I.");
    // assert(MIX_DEFAULT_FORMAT == AUDIO_S16LSB, "_Mix_DoEffects does not pick its function dynamically II."); // FIX_OUT
    if (channel->has_effect)
        return _Eff_do_position_s16lsb(buf, len, channel);
    return _Eff_do_volume_s16lbs(buf, len, channel);
}
#endif // FULL - FIX_EFF

#ifdef FULL
static Uint8 speaker_amplitude[6];

static void set_amplitudes(int channels, int angle, int room_angle)
{
    int left = MIX_MAX_POS_EFFECT, right = MIX_MAX_POS_EFFECT;
    int left_rear = MIX_MAX_POS_EFFECT, right_rear = MIX_MAX_POS_EFFECT, center = MIX_MAX_POS_EFFECT;

    /* our only caller Mix_SetPosition() already makes angle between 0 and 359. */

    if (channels == 2)
    {
        /*
         * We only attenuate by position if the angle falls on the far side
         *  of center; That is, an angle that's due north would not attenuate
         *  either channel. Due west attenuates the right channel to 0.0, and
         *  due east attenuates the left channel to 0.0. Slightly east of
         *  center attenuates the left channel a little, and the right channel
         *  not at all. I think of this as occlusion by one's own head.  :)
         *
         *   ...so, we split our angle circle into four quadrants...
         */
        if (angle < 90) {
            left = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) angle) / 89.0f)));
        } else if (angle < 180) {
            left = (int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 90)) / 89.0f));
        } else if (angle < 270) {
            right = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 180)) / 89.0f)));
        } else {
            right = (int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 270)) / 89.0f));
        }
    }

    if (channels == 4 || channels == 6)
    {
        /*
         *  An angle that's due north does not attenuate the center channel.
         *  An angle in the first quadrant, 0-90, does not attenuate the RF.
         *
         *   ...so, we split our angle circle into 8 ...
         *
         *             CE
         *             0
         *     LF      |         RF
         *             |
         *  270<-------|----------->90
         *             |
         *     LR      |         RR
         *            180
         *
         */
        if (angle < 45) {
            left = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (180 - angle)) / 179.0f)));
            left_rear = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle + 45)) / 89.0f)));
            right_rear = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (90 - angle)) / 179.0f)));
        } else if (angle < 90) {
            center = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (225 - angle)) / 179.0f)));
            left = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (180 - angle)) / 179.0f)));
            left_rear = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (135 - angle)) / 89.0f)));
            right_rear = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (90 + angle)) / 179.0f)));
        } else if (angle < 135) {
            center = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (225 - angle)) / 179.0f)));
            left = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 45)) / 89.0f)));
            right = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (270 - angle)) / 179.0f)));
            left_rear = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle)) / 179.0f)));
        } else if (angle < 180) {
            center = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 90)) / 89.0f)));
            left = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (225 - angle)) / 89.0f)));
            right = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (270 - angle)) / 179.0f)));
            left_rear = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle)) / 179.0f)));
        } else if (angle < 225) {
            center = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (270 - angle)) / 89.0f)));
            left = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 90)) / 179.0f)));
            right = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 135)) / 89.0f)));
            right_rear = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (360 - angle)) / 179.0f)));
        } else if (angle < 270) {
            center = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 135)) / 179.0f)));
            left = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 90)) / 179.0f)));
            right = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (315 - angle)) / 89.0f)));
            right_rear = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (360 - angle)) / 179.0f)));
        } else if (angle < 315) {
            center = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 135)) / 179.0f)));
            right = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 180)) / 179.0f)));
            left_rear = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (450 - angle)) / 179.0f)));
            right_rear = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 225)) / 89.0f)));
        } else {
            right = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (angle - 180)) / 179.0f)));
            left_rear = ((int) (MIX_MAX_POS_EFFECT_F * (((float) (450 - angle)) / 179.0f)));
            right_rear = MIX_MAX_POS_EFFECT - ((int) (MIX_MAX_POS_EFFECT_F * (((float) (405 - angle)) / 89.0f)));
        }
    }

    if (left < 0) left = 0;
    if (left > MIX_MAX_POS_EFFECT) left = MIX_MAX_POS_EFFECT;
    if (right < 0) right = 0;
    if (right > MIX_MAX_POS_EFFECT) right = MIX_MAX_POS_EFFECT;
    if (left_rear < 0) left_rear = 0;
    if (left_rear > MIX_MAX_POS_EFFECT) left_rear = MIX_MAX_POS_EFFECT;
    if (right_rear < 0) right_rear = 0;
    if (right_rear > MIX_MAX_POS_EFFECT) right_rear = MIX_MAX_POS_EFFECT;
    if (center < 0) center = 0;
    if (center > MIX_MAX_POS_EFFECT) center = MIX_MAX_POS_EFFECT;

    if (room_angle == 90) {
        speaker_amplitude[0] = (Uint8)left_rear;
        speaker_amplitude[1] = (Uint8)left;
        speaker_amplitude[2] = (Uint8)right_rear;
        speaker_amplitude[3] = (Uint8)right;
    }
    else if (room_angle == 180) {
        if (channels == 2) {
            speaker_amplitude[0] = (Uint8)right;
            speaker_amplitude[1] = (Uint8)left;
        }
        else {
            speaker_amplitude[0] = (Uint8)right_rear;
            speaker_amplitude[1] = (Uint8)left_rear;
            speaker_amplitude[2] = (Uint8)right;
            speaker_amplitude[3] = (Uint8)left;
        }
    }
    else if (room_angle == 270) {
        speaker_amplitude[0] = (Uint8)right;
        speaker_amplitude[1] = (Uint8)right_rear;
        speaker_amplitude[2] = (Uint8)left;
        speaker_amplitude[3] = (Uint8)left_rear;
    }
    else {
        speaker_amplitude[0] = (Uint8)left;
        speaker_amplitude[1] = (Uint8)right;
        speaker_amplitude[2] = (Uint8)left_rear;
        speaker_amplitude[3] = (Uint8)right_rear;
    }
    speaker_amplitude[4] = (Uint8)center;
    speaker_amplitude[5] = MIX_MAX_POS_EFFECT;
}

int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance);
#endif // FULL
void Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
#ifdef FULL
    Mix_EffectFunc_t f = NULL;
    int channels;
    Uint16 format;
#endif
    _Mix_EffectPosArgs *args = NULL;
    //int retval = 1;

#ifdef FULL // FIX_OUT
    Mix_QuerySpec(NULL, &format, &channels);
    if (channels != 2 && channels != 4 && channels != 6)    /* it's a no-op; we call that successful. */
        return(1);
    if (channels > 2) {
        /* left = right = MIX_MAX_POS_EFFECT => angle = 0, to unregister effect as when channels = 2 */
        /* left = MIX_MAX_POS_EFFECT =>  angle = -90;  left = 0 => angle = +89 */
        int angle = 0;
        if ((left != MIX_MAX_POS_EFFECT) || (right != MIX_MAX_POS_EFFECT)) {
            angle = (int)left;
            angle = 127 - angle;
            angle = -angle;
            angle = angle * 90 / 128; /* Make it larger for more effect? */
        }
        return Mix_SetPosition(channel, angle, 0);
    }
    f = get_position_effect_func(format, channels);
#endif // FULL
#ifdef FULL // FIX_EFF
    if (f == NULL)
        return(0);

    Mix_LockAudio();
    args = get_position_arg(channel);
    if (!args) {
        Mix_UnlockAudio();
        return(0);
    }

        /* it's a no-op; unregister the effect, if it's registered. */
#ifdef FULL // FIX_OUT
    if ((args->distance_u8 == MIX_MAX_POS_EFFECT) && (left == MIX_MAX_POS_EFFECT) && (right == MIX_MAX_POS_EFFECT)) {
#else
    if ((left == MIX_MAX_POS_EFFECT) && (right == MIX_MAX_POS_EFFECT)) {
#endif
        if (args->in_use) {
            retval = _Mix_UnregisterEffect_locked(channel, f);
            Mix_UnlockAudio();
            return(retval);
        } else {
            Mix_UnlockAudio();
            return(1);
        }
    }
#ifdef FULL
    args->left_u8 = left;
#endif
    args->left_f = ((float) left) / MIX_MAX_POS_EFFECT_F;
#ifdef FULL
    args->right_u8 = right;
#endif
    args->right_f = ((float) right) / MIX_MAX_POS_EFFECT_F;
#ifdef FULL
    args->room_angle = 0;
#endif
    if (!args->in_use) {
        args->in_use = 1;
        retval=_Mix_RegisterEffect_locked(channel, f, _Eff_PositionDone, (void*)args);
    }
#else // FIX_EFF
    Mix_LockAudio();
    if ((left == MIX_MAX_POS_EFFECT) && (right == MIX_MAX_POS_EFFECT)) {
        mix_channel[channel].has_effect = SDL_FALSE;
    } else {
        mix_channel[channel].has_effect = SDL_TRUE;
        args = &mix_channel[channel].effect;
        //args->left_f = ((float) left) / MIX_MAX_POS_EFFECT_F;
        args->left_vol = left;
        //args->right_f = ((float) right) / MIX_MAX_POS_EFFECT_F;
        args->right_vol = right;
    }
#endif // FULL - FIX_EFF
    Mix_UnlockAudio();
    //return(retval);
}

#ifdef FULL
int Mix_SetDistance(int channel, Uint8 distance)
{
    Mix_EffectFunc_t f = NULL;
    Uint16 format;
    _Mix_EffectPosArgs *args = NULL;
    int channels;
    int retval = 1;

    Mix_QuerySpec(NULL, &format, &channels);
    f = get_position_effect_func(format, channels);
    if (f == NULL)
        return(0);

    Mix_LockAudio();
    args = get_position_arg(channel);
    if (!args) {
        Mix_UnlockAudio();
        return(0);
    }

    distance = MIX_MAX_POS_EFFECT - distance;  /* flip it to our scale. */

    /* it's a no-op; unregister the effect, if it's registered. */
    if ((distance == MIX_MAX_POS_EFFECT) && (args->left_u8 == MIX_MAX_POS_EFFECT) && (args->right_u8 == MIX_MAX_POS_EFFECT)) {
        if (args->in_use) {
            retval = _Mix_UnregisterEffect_locked(channel, f);
            Mix_UnlockAudio();
            return(retval);
        } else {
            Mix_UnlockAudio();
            return(1);
        }
    }

    args->distance_u8 = distance;
    args->distance_f = ((float) distance) / MIX_MAX_POS_EFFECT_F;
    if (!args->in_use) {
        args->in_use = 1;
        retval = _Mix_RegisterEffect_locked(channel, f, _Eff_PositionDone, (void *) args);
    }

    Mix_UnlockAudio();
    return(retval);
}

int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance)
{
    Mix_EffectFunc_t f = NULL;
    Uint16 format;
    int channels;
    _Mix_EffectPosArgs *args = NULL;
    Sint16 room_angle = 0;
    int retval = 1;

    Mix_QuerySpec(NULL, &format, &channels);
    f = get_position_effect_func(format, channels);
    if (f == NULL)
        return(0);

    /* make angle between 0 and 359. */
    angle %= 360;
    if (angle < 0) angle += 360;

    Mix_LockAudio();
    args = get_position_arg(channel);
    if (!args) {
        Mix_UnlockAudio();
        return(0);
    }

    /* it's a no-op; unregister the effect, if it's registered. */
    if ((!distance) && (!angle)) {
        if (args->in_use) {
            retval = _Mix_UnregisterEffect_locked(channel, f);
            Mix_UnlockAudio();
            return(retval);
        } else {
            Mix_UnlockAudio();
            return(1);
        }
    }

    if (channels == 2)
    {
        if (angle > 180)
            room_angle = 180; /* exchange left and right channels */
        else room_angle = 0;
    }

    if (channels == 4 || channels == 6)
    {
        if (angle > 315) room_angle = 0;
        else if (angle > 225) room_angle = 270;
        else if (angle > 135) room_angle = 180;
        else if (angle > 45) room_angle = 90;
        else room_angle = 0;
    }

    distance = MIX_MAX_POS_EFFECT - distance;  /* flip it to scale Mix_SetDistance() uses. */

    set_amplitudes(channels, angle, room_angle);

    args->left_u8 = speaker_amplitude[0];
    args->left_f = ((float) speaker_amplitude[0]) / MIX_MAX_POS_EFFECT_F;
    args->right_u8 = speaker_amplitude[1];
    args->right_f = ((float) speaker_amplitude[1]) / MIX_MAX_POS_EFFECT_F;
    args->left_rear_u8 = speaker_amplitude[2];
    args->left_rear_f = ((float) speaker_amplitude[2]) / MIX_MAX_POS_EFFECT_F;
    args->right_rear_u8 = speaker_amplitude[3];
    args->right_rear_f = ((float) speaker_amplitude[3]) / MIX_MAX_POS_EFFECT_F;
    args->center_u8 = speaker_amplitude[4];
    args->center_f = ((float) speaker_amplitude[4]) / MIX_MAX_POS_EFFECT_F;
    args->lfe_u8 = speaker_amplitude[5];
    args->lfe_f = ((float) speaker_amplitude[5]) / MIX_MAX_POS_EFFECT_F;
    args->distance_u8 = distance;
    args->distance_f = ((float) distance) / MIX_MAX_POS_EFFECT_F;
    args->room_angle = room_angle;
    if (!args->in_use) {
        args->in_use = 1;
        retval = _Mix_RegisterEffect_locked(channel, f, _Eff_PositionDone, (void *) args);
    }

    Mix_UnlockAudio();
    return(retval);
}
#endif // FULL

void _Eff_PositionInit(void)
{
#if defined(SDL_AVX2_INTRINSICS) && SDL_VERSION_ATLEAST(2, 0, 2)
    if (SDL_HasAVX2()) {
        _Eff_do_volume_s16lbs = _Eff_volume_s16lbs_AVX2;
        _Eff_do_position_s16lsb = _Eff_position_s16lsb_AVX2;
        return;
    }
#endif
#ifdef SDL_SSE2_INTRINSICS
#ifdef SDL_HAVE_SSE2_SUPPORT
    // SDL_assert(SDL_HasSSE2());
    if (1) {
#else
    if (SDL_HasSSE2()) {
#endif
        _Eff_do_volume_s16lbs = _Eff_volume_s16lbs_SSE2;
        _Eff_do_position_s16lsb = _Eff_position_s16lsb_SSE2;
        return;
    }
#endif
    _Eff_do_volume_s16lbs = _Eff_volume_s16lbs;
    _Eff_do_position_s16lsb = _Eff_position_s16lsb;
}

void _Eff_PositionDeinit(void)
{
#ifdef FULL // FIX_EFF
    int i;
    for (i = 0; i < position_channels; i++) {
        SDL_free(pos_args_array[i]);
    }

    position_channels = 0;
#ifdef FULL // EFF_CHECK
    SDL_free(pos_args_global);
    pos_args_global = NULL;
#endif
    SDL_free(pos_args_array);
    pos_args_array = NULL;
#endif // FULL - FIX_EFF
}

/* end of effects_position.c ... */

/* vi: set ts=4 sw=4 expandtab: */
