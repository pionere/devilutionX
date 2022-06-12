#pragma once

#include "../types.h"

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

MenuAction GetMenuAction(const SDL_Event &event);

/** Menu action from holding the left stick or DPad. */
MenuAction GetMenuHeldUpDownAction();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
