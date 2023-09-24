#pragma once

#include <SDL.h>

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MenuAction {
	MenuAction_NONE = 0,
	MenuAction_SELECT,
	MenuAction_BACK,
	MenuAction_DELETE,

	MenuAction_UP,
	MenuAction_DOWN,
	MenuAction_LEFT,
	MenuAction_RIGHT,

	MenuAction_PAGE_UP,
	MenuAction_PAGE_DOWN,
} MenuAction;

MenuAction GetMenuAction(const SDL_Event& event);

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
/** Menu action from holding the left stick or DPad. */
MenuAction GetMenuHeldUpDownAction();
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
