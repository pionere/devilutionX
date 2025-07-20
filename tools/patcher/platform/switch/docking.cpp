#include "docking.h"

#include <switch.h>
#include <SDL.h>

#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

static int currently_docked = -1; // keep track of docked or handheld mode

/**
 * @brief Do a manual window resize when docking/undocking the Switch
 */
void HandleDocking()
{
	int docked, display_width, display_height;

	switch (appletGetOperationMode()) {
	case AppletOperationMode_Handheld:
		docked = 0;
		break;
	case AppletOperationMode_Console:
		docked = 1;
		break;
	default:
		docked = 0;
	}

	if (docked != currently_docked) {
		currently_docked = docked;
		// docked mode has changed, update window size
		if (docked) {
			display_width = 1920;
			display_height = 1080;
		} else {
			display_width = 1280;
			display_height = 720;
		}
		// remove leftover-garbage on screen. Need to perform three clears to ensure all buffers get cleared, otherwise
		//  the display flickers showing a stale frame at certain refresh rates/dock modes.
		for (int i = 0; i < 3; i++) {
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
		}
		SDL_SetWindowSize(ghMainWnd, display_width, display_height);
	}
}

DEVILUTION_END_NAMESPACE
