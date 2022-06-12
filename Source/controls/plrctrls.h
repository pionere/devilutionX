#pragma once
// Controller actions implementation

#include "all.h"
#include "controller.h"
#include "../plrctrls.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD

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

// Whether the automap is being displayed.
bool IsAutomapActive();

extern bool sgbControllerActive;

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif
