#pragma once

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include <SDL.h>

#include "../defs.h"
#include "../enums.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Returns a non-empty AxisDirection at most once per the given time interval.
 */
class AxisDirectionRepeater {
public:
	AxisDirectionRepeater(Uint32 min_interval_ms = 200)
	    : last_dir(DIR_NONE)
	    , last_dir_ts(0)
	    , min_interval_ms_(min_interval_ms)
	{
	}

	int Get(int axis_direction);

private:
	int last_dir;
	Uint32 last_dir_ts;
	Uint32 min_interval_ms_;
};

extern AxisDirectionRepeater axisDirRepeater;

DEVILUTION_END_NAMESPACE

#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
