#include "platform/ctr/display.hpp"
#include <SDL.h>

uint32_t Get3DSScalingFlag(int width, int height)
{
	if (width * 3 < height * 5)
		return SDL_FITHEIGHT;
	return SDL_FITWIDTH;
}
