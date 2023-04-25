#pragma once

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void DrawControllerModifierHints();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif
