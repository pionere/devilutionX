#pragma once

#include "../types.h"
#include "controller.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1

DEVILUTION_BEGIN_NAMESPACE

void DrawControllerModifierHints();

DEVILUTION_END_NAMESPACE

#endif
