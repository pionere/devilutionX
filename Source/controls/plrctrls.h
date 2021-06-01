#pragma once
// Controller actions implementation

#include "all.h"
#include "controller.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1

DEVILUTION_BEGIN_NAMESPACE

typedef enum belt_item_type {
	BLT_HEALING,
	BLT_MANA,
} belt_item_type;

// Runs every frame.
// Handles menu movement.
void plrctrls_every_frame();

// Run after every game logic iteration.
// Handles player movement.
void plrctrls_after_game_logic();

// Runs at the end of CheckCursMove()
// Handles item, object, and monster auto-aim.
void plrctrls_after_check_curs_move();

// Moves the map if active, the cursor otherwise.
void HandleRightStickMotion();

// Whether we're in a dialog menu that the game handles natively with keyboard controls.
bool InGameMenu();

// Whether the automap is being displayed.
bool IsAutomapActive();

// Whether the mouse cursor is being moved with the controller.
bool IsMovingMouseCursorWithController();

void UseBeltItem(int type);

// Talk to towners, click on inv items, attack, etc.
void PerformPrimaryAction();

// Open chests, doors, pickup items.
void PerformSecondaryAction();
bool TryDropItem();
void FocusOnInventory();
void PerformSpellAction();
void StoreSpellCoords();

extern int speedspellcount;
extern bool sgbControllerActive;

DEVILUTION_END_NAMESPACE

#endif
