/**
 * @file plrctrls.h
 *
 * Interface of functions from the core game to the controllers.
 */
#ifndef __PLRCTRLS_H__
#define __PLRCTRLS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool sgbControllerActive;

// Runs at the end of CheckCursMove()
// Handles item, object, and monster auto-aim.
void plrctrls_after_check_curs_move();

// Run after every game logic iteration.
// Handles player movement.
void plrctrls_after_game_logic();
// Runs every frame.
// Handles menu movement.
void plrctrls_every_frame();

// Whether the mouse cursor is being moved with the controller.
bool IsMovingMouseCursorWithController();

void FocusOnInventory();

void FocusOnCharInfo();

// defined in touch.cpp
void finish_simulated_mouse_clicks(int current_mouse_x, int current_mouse_y);

// defined in modifier_hints.cpp
void DrawControllerModifierHints();

void UseBeltItem(int type);

// Talk to towners, click on inv items, attack, etc.
void PerformPrimaryAction();

// Open chests, doors, pickup items.
void PerformSecondaryAction();
void PerformSpellAction();
void StoreSpellCoords();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLRCTRLS_H__ */
