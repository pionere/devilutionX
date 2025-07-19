#include <3ds.h>
#include <SDL.h>
#include "utils/sdl2_to_1_2_backports.h"
#include "utils/log.h"

int SDL_ShowSimpleMessageBox(Uint32 flags,
    const char *title,
    const char *message,
    SDL_Surface *window)
{
	SDL_ShowCursor(SDL_DISABLE);

	bool init = !gspHasGpuRight();

	char text[1024];
	snprintf(text, sizeof(text), "%s\n\n%s", title, message);

	if (init)
		gfxInitDefault();

	errorConf error;
	errorInit(&error, ERROR_TEXT, CFG_LANGUAGE_EN);
	errorText(&error, text);
	errorDisp(&error);

	if (init)
		gfxExit();

	return 0;
}
