#pragma once

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include <SDL.h>

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

enum AxisDirectionX {
	AxisDirectionX_NONE = 0,
	AxisDirectionX_LEFT,
	AxisDirectionX_RIGHT
};
enum AxisDirectionY {
	AxisDirectionY_NONE = 0,
	AxisDirectionY_UP,
	AxisDirectionY_DOWN
};

/**
 * @brief 8-way direction of a D-Pad or a thumb stick.
 */
struct AxisDirection {
	AxisDirectionX x;
	AxisDirectionY y;
};

/**
 * @brief Returns a non-empty AxisDirection at most once per the given time interval.
 */
class AxisDirectionRepeater {
public:
	AxisDirectionRepeater(Uint32 min_interval_ms = 200)
	    : last_left_(0)
	    , last_right_(0)
	    , last_up_(0)
	    , last_down_(0)
	    , min_interval_ms_(min_interval_ms)
	{
	}

	AxisDirection Get(AxisDirection axis_direction);

private:
	Uint32 last_left_;
	Uint32 last_right_;
	Uint32 last_up_;
	Uint32 last_down_;
	Uint32 min_interval_ms_;
};

extern AxisDirectionRepeater axisDirRepeater;

DEVILUTION_END_NAMESPACE

#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
