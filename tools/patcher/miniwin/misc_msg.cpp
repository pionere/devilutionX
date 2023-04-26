/**
 * @file misc_msg.cpp
 *
 * Windows message handling and keyboard event conversion for SDL.
 */
#include <SDL.h>

#include "all.h"
#include "utils/display.h"
#include "utils/sdl_compat.h"

DEVILUTION_BEGIN_NAMESPACE

/** The current input handler function */
WNDPROC CurrentWndProc;

void SetCursorPos(int x, int y)
{
	LogicalToOutput(&x, &y);
	SDL_WarpMouseInWindow(ghMainWnd, x, y);
}

WNDPROC SetWindowProc(WNDPROC newWndProc)
{
	WNDPROC oldWndProc;

	oldWndProc = CurrentWndProc;
	CurrentWndProc = newWndProc;
	return oldWndProc;
}

DEVILUTION_END_NAMESPACE
