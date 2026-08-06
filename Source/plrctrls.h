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

// defined in controller.cpp
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
// return whether the ingame-cursor should be drawn
bool plrctrls_draw_cursor();

void FocusOnInventory();

void FocusOnCharInfo();

// defined in touch.cpp
void DrawGamepad();

// defined in modifier_hints.cpp
void DrawControllerModifierHints();

void UseBeltItem(bool manaItem);

// Open chests, doors, pickup items.
void PerformSecondaryAction();
void PerformSpellAction();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLRCTRLS_H__ */
