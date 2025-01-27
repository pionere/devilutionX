/**
 * @file movie.cpp
 *
 * Implementation of video playback.
 */
#include "all.h"
#include "plrctrls.h"
#include "storm/storm_svid.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Start playback of a given video.
 * @param pszMovie The file name of the video
 * @param movieFlags flags to control the playback, see movie_flags enum.
 * @return the reason why playback ended (movie_playback_result)
 */
int play_movie(const char* pszMovie, int movieFlags)
{
	int result = MPR_DONE;
	HANDLE video_stream;

	sound_disable_music();
	StopSFX();

	//video_stream = SVidPlayBegin(pszMovie, (movieFlags & MOV_LOOP) ? 0x100C0808 : 0x10280808);
	video_stream = SVidPlayBegin(pszMovie, movieFlags);
	Dvl_Event e;
	while (video_stream != NULL) {
		while (PeekMessage(e)) {
			switch (e.type) {
			case DVL_WM_KEYDOWN:
				if (e.vkcode == DVL_VK_ESCAPE) {
					result = MPR_CANCEL;
					break;
				}
#if !FULLSCREEN_ONLY
				if (SDL_GetModState() & KMOD_ALT) {
					if (e.vkcode == DVL_VK_RETURN)
						ToggleFullscreen();
					continue;
				}
#endif
			case DVL_WM_LBUTTONDOWN:
			case DVL_WM_RBUTTONDOWN:
				if (movieFlags & MOV_SKIP) {
					result = MPR_CANCEL;
					break;
				}
				continue;
			case DVL_WM_QUIT:
				if (gbRunGame) {
					NetSendCmd(CMD_DISCONNECT);
					gbRunGameResult = false;
				}
				result = MPR_QUIT;
				break;
			default:
				continue;
			}
			break;
		}
#if HAS_TOUCHPAD
		finish_simulated_mouse_clicks();
#endif
		if (!SVidPlayContinue() || result != MPR_DONE) {
			SVidPlayEnd();
			break;
		}
	}

	sound_restart_music();

	return result;
}

DEVILUTION_END_NAMESPACE
