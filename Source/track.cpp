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

	// WALK_TICK
	if (myplr._pVar8 <= 6 && myplr._pmode != PM_STAND)
		return;

	assert(IN_ACTIVE_DUN(pcurspos.dun.x, pcurspos.dun.y));
	if (pcurspos.dun.x != myplr._ptargx || pcurspos.dun.y != myplr._ptargy) {
		Uint32 now = SDL_GetTicks();
		if (SDL_TICKS_PASSED(now, sgdwLastWalk + tick_delay * 6)) {
			sgdwLastWalk = now;
			NetSendCmdLoc(CMD_WALKXY, pcurspos.dun);
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
		NetSendCmdLoc(CMD_WALKXY, pcurspos.dun);
	} else if (_gbIsScrolling) {
		_gbIsScrolling = false;
	}
}

bool track_isscrolling()
{
	return _gbIsScrolling;
}

DEVILUTION_END_NAMESPACE
