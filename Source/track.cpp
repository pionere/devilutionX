/**
 * @file track.cpp
 *
 * Implementation of functionality tracking what the mouse cursor is pointing at.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static bool _gbIsScrolling;
static Uint32 sgdwLastWalk;
static bool _gbIsWalking;

void track_process()
{
	if (!_gbIsWalking)
		return;

	if (pcurspos.subtile.x < 0 || pcurspos.subtile.x >= MAXDUNX - 1 || pcurspos.subtile.y < 0 || pcurspos.subtile.y >= MAXDUNY - 1)
		return;
	// WALK_TICK
	if (myplr._pVar8 <= 6 && myplr._pmode != PM_STAND)
		return;

	if (pcurspos.subtile.x != myplr._ptargx || pcurspos.subtile.y != myplr._ptargy) {
		Uint32 now = SDL_GetTicks();
		if (SDL_TICKS_PASSED(now, sgdwLastWalk + tick_delay * 6)) {
			sgdwLastWalk = now;
			NetSendCmdLoc(CMD_WALKXY, pcurspos.subtile.x, pcurspos.subtile.y);
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
		NetSendCmdLoc(CMD_WALKXY, pcurspos.subtile.x, pcurspos.subtile.y);
	} else if (_gbIsScrolling) {
		_gbIsScrolling = false;
	}
}

bool track_isscrolling()
{
	return _gbIsScrolling;
}

DEVILUTION_END_NAMESPACE
