#include "axis_direction.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include "../utils/sdl_compat.h"

DEVILUTION_BEGIN_NAMESPACE

AxisDirectionRepeater axisDirRepeater;

int AxisDirectionRepeater::Get(int axisDirection)
{
	const Uint32 now = SDL_GetTicks();
	if (axisDirection == last_dir && !SDL_TICKS_AFTER(now, last_dir_ts, min_interval_ms_)) {
		axisDirection = DIR_NONE;
	} else {
		last_dir = axisDirection;
		last_dir_ts = now;
	}
	return axisDirection;
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
