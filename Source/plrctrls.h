/**
 * @file plrctrls.h
 *
 * Interface of functions from the core game to the controllers.
 */
#ifndef __PLRCTRLS_H__
#define __PLRCTRLS_H__

DEVILUTION_BEGIN_NAMESPACE

extern bool sgbControllerActive;

// Runs at the end of CheckCursMove()
// Handles item, object, and monster auto-aim.
void plrctrls_after_check_curs_move();

// Run after every game logic iteration.
// Handles player and menu movement.
void plrctrls_after_game_logic();

// Whether the mouse cursor is being moved with the controller.
bool IsMovingMouseCursorWithController();

void FocusOnInventory();

// defined in misc_msg.cpp
void FocusOnCharInfo();

// defined in touch.cpp
void finish_simulated_mouse_clicks(int current_mouse_x, int current_mouse_y);

// defined in modifier_hints.cpp
void DrawControllerModifierHints();

DEVILUTION_END_NAMESPACE

#endif /* __PLRCTRLS_H__ */
