#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

enum MenuAction {
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
};

MenuAction GetMenuAction(const SDL_Event &event);

/** Menu action from holding the left stick or DPad. */
MenuAction GetMenuHeldUpDownAction();

DEVILUTION_END_NAMESPACE
