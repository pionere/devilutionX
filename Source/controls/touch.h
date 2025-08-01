#pragma once

#if HAS_TOUCHPAD
#ifdef USE_SDL1
static_assert(false, "Touchpad is not supported in SDL1.");
#endif
#include <SDL.h>

#include "../all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void InitTouch();
void InitGamepadGFX();
void FreeGamepadGFX();
void handle_touch(SDL_Event* event);
bool TryActionDirCmd(bool altSkill, RECT_AREA32& vec);
void DrawGamepad();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_TOUCHPAD
