#pragma once

#include "../types.h"
#include "controller.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

DEVILUTION_BEGIN_NAMESPACE

void DrawControllerModifierHints();

DEVILUTION_END_NAMESPACE

#endif
