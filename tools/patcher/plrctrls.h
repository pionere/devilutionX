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

// Whether the mouse cursor is being moved with the controller. (defined in controller_motion.cpp)
bool IsMovingMouseCursorWithController();

// defined in touch.cpp
void finish_simulated_mouse_clicks();

// defined in modifier_hints.cpp
void DrawControllerModifierHints();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PLRCTRLS_H__ */
