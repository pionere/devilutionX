#include "axis_direction.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include "../utils/sdl_compat.h"

DEVILUTION_BEGIN_NAMESPACE

AxisDirectionRepeater axisDirRepeater;

AxisDirection AxisDirectionRepeater::Get(AxisDirection axisDirection)
{
	const Uint32 now = SDL_GetTicks();
	switch (axisDirection.x) {
	case AxisDirectionX_LEFT:
		last_right_ = 0;
		if (last_left_ != 0 && !SDL_TICKS_AFTER(now, last_left_, min_interval_ms_)) {
			axisDirection.x = AxisDirectionX_NONE;
		} else {
			last_left_ = now;
		}
		break;
	case AxisDirectionX_RIGHT:
		last_left_ = 0;
		if (last_right_ != 0 && !SDL_TICKS_AFTER(now, last_right_, min_interval_ms_)) {
			axisDirection.x = AxisDirectionX_NONE;
		} else {
			last_right_ = now;
		}
		break;
	case AxisDirectionX_NONE:
		last_left_ = last_right_ = 0;
		break;
	}
	switch (axisDirection.y) {
	case AxisDirectionY_UP:
		last_down_ = 0;
		if (last_up_ != 0 && !SDL_TICKS_AFTER(now, last_up_, min_interval_ms_)) {
			axisDirection.y = AxisDirectionY_NONE;
		} else {
			last_up_ = now;
		}
		break;
	case AxisDirectionY_DOWN:
		last_up_ = 0;
		if (last_down_ != 0 && !SDL_TICKS_AFTER(now, last_down_, min_interval_ms_)) {
			axisDirection.y = AxisDirectionY_NONE;
		} else {
			last_down_ = now;
		}
		break;
	case AxisDirectionY_NONE:
		last_up_ = last_down_ = 0;
		break;
	}
	return axisDirection;
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
