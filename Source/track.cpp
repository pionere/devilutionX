/**
 * @file track.cpp
 *
 * Implementation of functionality tracking what the mouse cursor is pointing at.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbIsScrolling;
static DWORD sgdwLastWalk;
static bool _gbIsWalking;

void track_process()
{
	if (!_gbIsWalking)
		return;

	if (cursmx < 0 || cursmx >= MAXDUNX - 1 || cursmy < 0 || cursmy >= MAXDUNY - 1)
		return;
	// WALK_TICK
	if (myplr._pVar8 <= 6 && myplr._pmode != PM_STAND)
		return;

	if (cursmx != myplr._ptargx || cursmy != myplr._ptargy) {
		DWORD tick = SDL_GetTicks();
		if ((int)(tick - sgdwLastWalk) >= tick_delay * 6) {
			sgdwLastWalk = tick;
			NetSendCmdLoc(true, CMD_WALKXY, cursmx, cursmy);
			if (!_gbIsScrolling)
				_gbIsScrolling = true;
		}
	}
}

void track_repeat_walk(bool rep)
{
	if (_gbIsWalking == rep)
		return;

	_gbIsWalking = rep;
	if (rep) {
		_gbIsScrolling = false;
		sgdwLastWalk = SDL_GetTicks() - tick_delay;
		NetSendCmdLoc(true, CMD_WALKXY, cursmx, cursmy);
	} else if (_gbIsScrolling) {
		_gbIsScrolling = false;
	}
}

bool track_isscrolling()
{
	return _gbIsScrolling;
}

DEVILUTION_END_NAMESPACE
