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

#ifndef INCLUDE_EFFECTS_INTERNAL_H_
#define INCLUDE_EFFECTS_INTERNAL_H_

#ifndef MIX_INTERNAL_EFFECT__
#error You should not include this file or use these functions.
#endif

#include "SDL_mixer.h"
#include "types_internal.h"

#ifdef FULL
extern int _Mix_effects_max_speed;
extern void *_Eff_volume_table;
void *_Eff_build_volume_table_u8(void);
void *_Eff_build_volume_table_s8(void);
#endif
void _Mix_InitEffects(void);
void _Mix_DeinitEffects(void);
void _Eff_PositionInit(void);
void _Eff_PositionDeinit(void);
#ifdef FULL // FIX_EFF
int _Mix_RegisterEffect_locked(int channel, Mix_EffectFunc_t f,
                               Mix_EffectDone_t d, void *arg);
int _Mix_UnregisterEffect_locked(int channel, Mix_EffectFunc_t f);
int _Mix_UnregisterAllEffects_locked(int channel);
#else
SDL_bool _Mix_DoEffects(void* buf, unsigned len, Mix_Channel* channel);
// unregister an effect on a channel (effect_position/mixer -> mixer)
void _Mix_UnregisterChanEffect(int channel);
#endif

#endif /* _INCLUDE_EFFECTS_INTERNAL_H_ */

/* vi: set ts=4 sw=4 expandtab: */
