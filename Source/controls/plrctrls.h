#pragma once
// Controller actions implementation

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

#include "all.h"
#include "controller.h"
#include "../plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

typedef enum belt_item_type {
	BLT_HEALING,
	BLT_MANA,
} belt_item_type;

// Whether we're in a dialog menu that the game handles natively with keyboard controls.
bool InGameMenu();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
